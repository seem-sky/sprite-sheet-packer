#include "TPSParser.h"
#include <QDomElement>
#include <QDomNode>
#include <QDomDocument>

QVariant TPSParser::parse(QIODevice *device) {
	QVariantMap result;
	QDomDocument doc;
	QString errorMessage;
	int errorLine;
	int errorColumn;
	bool success = doc.setContent(device, false, &errorMessage, &errorLine, &errorColumn);
	if (!success) {
        qDebug() << "TPSParser Warning: Could not parse tps file!";
		qDebug() << "Error message: " << errorMessage;
		qDebug() << "Error line: " << errorLine;
		qDebug() << "Error column: " << errorColumn;
		return result;
	}
	QDomElement root = doc.documentElement();
	if (root.attribute("version", "1.0") != "1.0") {
        qDebug() << "TPSParser Warning: tps is using an unknown format version, parsing might fail unexpectedly";
	}
	return parseElement(root.firstChild().toElement());
}

QVariant TPSParser::parseElement(const QDomElement &e) {
	QString tagName = e.tagName();
    QVariant result;
    if ((tagName == "dict")||(tagName == "map")||(tagName == "struct")||(tagName == "QSize")) {
		result = parseDictElement(e);
    } else if (tagName == "array") {
		result = parseArrayElement(e);
    } else if ((tagName == "string")||(tagName == "filename")||(tagName == "enum")) {
		result = e.text();
//    } else if (tagName == "data") {
//		result = QByteArray::fromBase64(e.text().toUtf8());
    } else if ((tagName == "integer")||(tagName == "uint")||(tagName == "int")) {
		result = e.text().toInt();
    } else if ((tagName == "real")||(tagName == "double")) {
		result = e.text().toFloat();
    } else if (tagName == "true") {
		result = true;
    } else if (tagName == "false") {
		result = false;
    } else if (tagName == "date") {
		result = QDateTime::fromString(e.text(), Qt::ISODate);
    } else {
        qDebug() << "TPSParser Warning: Invalid tag found: " << e.tagName() << e.text();
	}
	return result;
}


QVariantList TPSParser::parseArrayElement(const QDomElement& element) {
	QVariantList result;
	QDomNodeList children = element.childNodes();
	for (int i = 0; i < children.count(); i++) {
		QDomNode child = children.at(i);
		QDomElement e = child.toElement();
		if (!e.isNull()) {
			result.append(parseElement(e));
		}
	}
	return result;
}

QVariantMap TPSParser::parseDictElement(const QDomElement& element) {
	QVariantMap result;
	QDomNodeList children = element.childNodes();
	QString currentKey = "";
	for (int i = 0; i < children.count(); i++) {
		QDomNode child = children.at(i);
		QDomElement e = child.toElement();
		if (!e.isNull()) {
			QString tagName = e.tagName();
			if (tagName == "key") {
				currentKey = e.text();
			}
			else if (currentKey != "") {
				result[currentKey] = parseElement(e);
			}
		}
	}
	return result;
}
