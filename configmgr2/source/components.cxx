/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
************************************************************************/

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "comphelper/sequenceasvector.hxx"
#include "libxml/parser.h"
#include "libxml/xmlschemastypes.h"
#include "libxml/xmlwriter.h"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/mutex.hxx"
#include "osl/thread.hxx"
#include "rtl/bootstrap.hxx"
#include "rtl/ref.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textcvt.h"
#include "rtl/textenc.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "stl/hash_map"

#include "components.hxx"
#include "groupnode.hxx"
#include "localizedpropertynode.hxx"
#include "localizedpropertyvaluenode.hxx"
#include "lock.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "propertynode.hxx"
#include "setnode.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

typedef std::hash_map< rtl::OUString, css::uno::Any, rtl::OUStringHash >
    LocalizedValues;

bool isRemovedValue(css::uno::Any const & value) {
    // An ANY of type TYPE whose value is VOID is used within a LocalizedValues
    // map as a marker for a removed localized value (in a fused xcu property
    // within the registrymodifications file):
    return value.getValueType().getTypeClass() == css::uno::TypeClass_TYPE;
}

void setValues(LocalizedPropertyNode * node, LocalizedValues const & values) {
    for (LocalizedValues::const_iterator i(values.begin()); i != values.end();
         ++i)
    {
        OSL_ASSERT(!isRemovedValue(i->second));
        node->getMembers().insert(
            NodeMap::value_type(
                i->first, new LocalizedPropertyValueNode(i->second)));
    }
}

template< typename T > sal_Int32 findFirst(
    rtl::OUString const & string, T sub, sal_Int32 fromIndex)
{
    sal_Int32 i = string.indexOf(sub, fromIndex);
    return i == -1 ? string.getLength() : i;
}

bool isPrefix(rtl::OUString const & prefix, rtl::OUString const & path) {
    return prefix.getLength() < path.getLength() && path.match(prefix) &&
        path[prefix.getLength()] == '/';
}

bool decodeXml(
    rtl::OUString const & encoded, sal_Int32 begin, sal_Int32 end,
    rtl::OUString * decoded)
{
    OSL_ASSERT(
        begin >= 0 && begin <= end && end <= encoded.getLength() &&
        decoded != 0);
    rtl::OUStringBuffer buf;
    while (begin != end) {
        sal_Unicode c = encoded[begin++];
        if (c == '&') {
            if (encoded.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("amp;"), begin))
            {
                buf.append(sal_Unicode('&'));
                begin += RTL_CONSTASCII_LENGTH("amp;");
            } else if (encoded.matchAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("quot;"), begin))
            {
                buf.append(sal_Unicode('"'));
                begin += RTL_CONSTASCII_LENGTH("quot;");
            } else if (encoded.matchAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("apos;"), begin))
            {
                buf.append(sal_Unicode('\''));
                begin += RTL_CONSTASCII_LENGTH("apos;");
            } else {
                return false;
            }
            OSL_ASSERT(begin <= end);
        } else {
            buf.append(c);
        }
    }
    *decoded = buf.makeStringAndClear();
    return true;
}

rtl::OUString escapeText(rtl::OUString const & text, bool escapeClosingBracket)
{
    rtl::OUStringBuffer buf;
    for (sal_Int32 i = 0; i < text.getLength(); ++i) {
        sal_Unicode c = text[i];
        if (c < 0x0009 || c == 0x000B || c == 0x000C ||
            (c > 0x000D && c <= 0x001F) || c == 0xFFFE || c == 0xFFFF)
        {
            static sal_Unicode const hexDigit[16] = {
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
                'D', 'E', 'F' };
            buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("\\u"));
            buf.append(hexDigit[(c >> 12) & 0xF]);
            buf.append(hexDigit[(c >> 8) & 0xF]);
            buf.append(hexDigit[(c >> 4) & 0xF]);
            buf.append(hexDigit[c & 0xF]);
        } else {
            if (c == '\\' || (c == ']' && escapeClosingBracket)) {
                buf.append(sal_Unicode('\\'));
            }
            buf.append(c);
        }
    }
    return buf.makeStringAndClear();
}

xmlChar const * unescapeText(
    xmlChar const * text, bool enclosingBrackets, rtl::OString * unescaped)
{
    OSL_ASSERT(text != 0 && unescaped != 0);
    if (enclosingBrackets && *text++ != '[') {
        return 0;
    }
    rtl::OStringBuffer buf;
    for (;;) {
        xmlChar c = *text++;
        switch (c) {
        case '\0':
            if (enclosingBrackets) {
                return 0;
            }
            *unescaped = buf.makeStringAndClear();
            return text;
        case '\\':
            c = *text++;
            switch (c) {
            case ']':
                if (!enclosingBrackets) {
                    return 0;
                }
                // fall through
            case '\\':
                buf.append(static_cast< char >(c));
                break;
            case 'u':
                if (text[0] == '0' && text[1] == '0' &&
                    (text[2] == '0' || text[2] == '1') &&
                    ((text[3] >= '0' && text[3] <= '9') ||
                     (text[3] >= 'A' && text[3] <= 'F') ||
                     (text[3] >= 'a' && text[3] <= 'f')))
                {
                    char n = static_cast< char >(
                        ((text[2] - '0') << 4) |
                        (text[3] <= '9'
                         ? text[3] - '0'
                         : (text[3] - (text[3] <= 'F' ? 'A' : 'a') + 10)));
                    switch (n) {
                    case '\x09':
                    case '\x0A':
                    case '\x0D':
                        return 0;
                    default:
                        break;
                    }
                    buf.append(n);
                    text += 4;
                    break;
                }
                if ((text[0] == 'F' || text[0] == 'f') &&
                    (text[1] == 'F' || text[1] == 'f') &&
                    (text[2] == 'F' || text[2] == 'f') &&
                    (text[3] == 'E' || text[3] == 'e' ||
                     text[3] == 'F' || text[3] == 'f'))
                {
                    buf.append(static_cast< char >(0xEF));
                    buf.append(static_cast< char >(0xBF));
                    buf.append(
                        static_cast< char >(
                            0xBE + (text[3] == 'F' || text[3] == 'f' ? 1 : 0)));
                    text += 4;
                    break;
                }
                // fall through
            default:
                return 0;
            }
            break;
        case ']':
            if (enclosingBrackets) {
                *unescaped = buf.makeStringAndClear();
                return text;
            }
            // fall through
        default:
            buf.append(static_cast< char >(c));
            break;
        }
    }
}

rtl::OUString parseLastSegment(
    rtl::OUString const & path, rtl::OUString * name)
{
    OSL_ASSERT(name != 0);
    sal_Int32 i = path.getLength();
    OSL_ASSERT(i > 0 && path[i - 1] != '/');
    if (path[i - 1] == ']') {
        OSL_ASSERT(i > 2 && (path[i - 2] == '\'' || (path[i - 2] == '"')));
        sal_Int32 j = path.lastIndexOf(path[i - 2], i - 2);
        OSL_ASSERT(j > 0);
        decodeXml(path, j + 1, i - 2, name);
        i = path.lastIndexOf('/', j);
    } else {
        i = path.lastIndexOf('/');
        *name = path.copy(i + 1);
    }
    OSL_ASSERT(i != -1);
    return path.copy(0, i);
}

xmlChar const * xmlString(char const * str) {
    return reinterpret_cast< xmlChar const * >(str);
}

rtl::OUString fromXmlString(xmlChar const * str) {
    char const * s = reinterpret_cast< char const * >(str);
    return s == 0
        ? rtl::OUString()
        : rtl::OUString(s, rtl_str_getLength(s), RTL_TEXTENCODING_UTF8);
}

rtl::OUString fromXmlString(xmlChar const * str, xmlChar const * end) {
    OSL_ASSERT(str != 0 && end != 0);
    return rtl::OUString(
        reinterpret_cast< char const * >(str), end - str,
        RTL_TEXTENCODING_UTF8);
}

struct XmlString: private boost::noncopyable {
    xmlChar * str;

    explicit XmlString(xmlChar * theStr = 0): str(theStr) {}

    ~XmlString() { xmlFree(str); }

    XmlString & operator =(xmlChar * theStr);
};

XmlString & XmlString::operator =(xmlChar * theStr) {
    OSL_ASSERT(str == 0);
    str = theStr;
    return *this;
}

struct XmlParserContext: private boost::noncopyable {
    xmlParserCtxtPtr context;

    XmlParserContext();

    ~XmlParserContext() { xmlFreeParserCtxt(context); }
};

XmlParserContext::XmlParserContext(): context(xmlNewParserCtxt()) {
    if (context == 0) {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("xmlNewParserCtxt failed")),
            css::uno::Reference< css::uno::XInterface >());
    }
}

struct XmlDoc: private boost::noncopyable {
    xmlDocPtr doc;

    explicit XmlDoc(xmlDocPtr theDoc): doc(theDoc) {}

    ~XmlDoc() { xmlFreeDoc(doc); }
};

struct XmlTextWriter: private boost::noncopyable {
    xmlTextWriterPtr writer;

    explicit XmlTextWriter(xmlTextWriterPtr theWriter): writer(theWriter) {}

    ~XmlTextWriter() { xmlFreeTextWriter(writer); }
};

rtl::OUString fullTemplateName(
    rtl::OUString const & component, rtl::OUString const & name)
{
    OSL_ASSERT(component.indexOf(':') == -1);
    rtl::OUStringBuffer buf(component);
    buf.append(sal_Unicode(':'));
    buf.append(name);
    return buf.makeStringAndClear();
}

class NodeRef: public Node {
public:
    NodeRef(
        rtl::OUString const & templateName,
        Components::TemplateMap const & templates):
        templateName_(templateName), templates_(templates)
    {}

    virtual rtl::Reference< Node > clone() const
    { return new NodeRef(templateName_, templates_); }

    virtual rtl::Reference< Node > getMember(rtl::OUString const &);

    rtl::OUString getTemplateName() const { return templateName_; }

    Components::TemplateMap const & getTemplates() const { return templates_; }

private:
    virtual ~NodeRef() {}

    rtl::OUString templateName_;
    Components::TemplateMap const & templates_;
};

rtl::Reference< Node > NodeRef::getMember(rtl::OUString const &) {
    throw css::uno::RuntimeException(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unresolved node-ref")),
        css::uno::Reference< css::uno::XInterface >());
}

rtl::OString convertToUtf8(rtl::OUString const & text) {
    rtl::OString utf8;
    if (!text.convertToString(
            &utf8, RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("cannot translate text to UTF-8")),
            css::uno::Reference< css::uno::XInterface >());
    }
    return utf8;
}

rtl::OString convertToFilepath(rtl::OUString const & url) {
    rtl::OUString path1;
    if (osl::FileBase::getSystemPathFromFileURL(url, path1) !=
        osl::FileBase::E_None)
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("cannot get system path for ")) +
             url),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OString path2;
    if (!path1.convertToString(
            &path2, osl_getThreadTextEncoding(),
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("cannot translate system path ")) +
             path1),
            css::uno::Reference< css::uno::XInterface >());
    }
    return path2;
}

xmlDocPtr parseXmlFile(rtl::OUString const & url) {
    XmlParserContext context;
    xmlDocPtr doc(
        xmlCtxtReadFile(
            context.context, convertToFilepath(url).getStr(), 0,
            XML_PARSE_NOERROR));
        //TODO: pass (external) file URL instead of filepath?
    if (doc == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("xmlCtxtReadFile failed for ")) +
             url),
            css::uno::Reference< css::uno::XInterface >());
    }
    return doc;
}

xmlNodePtr skipBlank(xmlNodePtr node) {
    while (node != 0 && xmlIsBlankNode(node)) {
        node = node->next;
    }
    return node;
}

bool isOorElement(xmlNodePtr node, char const * name) {
    return node != 0 && xmlStrEqual(node->name, xmlString(name)) &&
        (node->ns == 0 ||
         xmlStrEqual(
             node->ns->href, xmlString("http://openoffice.org/2001/registry")));
}

rtl::OUString getNameAttribute(xmlDocPtr doc, xmlNodePtr node) {
    XmlString name(
        xmlGetNsProp(
            node, xmlString("name"),
            xmlString("http://openoffice.org/2001/registry")));
    if (name.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("missing name attribute in ")) +
             fromXmlString(doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    return fromXmlString(name.str);
}

enum Operation {
    OPERATION_MODIFY, OPERATION_REPLACE, OPERATION_FUSE, OPERATION_REMOVE };

Operation getOperationAttribute(xmlDocPtr doc, xmlNodePtr node) {
    XmlString op(
        xmlGetNsProp(
            node, xmlString("op"),
            xmlString("http://openoffice.org/2001/registry")));
    if (op.str == 0 || xmlStrEqual(op.str, xmlString("modify"))) {
        return OPERATION_MODIFY;
    } else if (xmlStrEqual(op.str, xmlString("replace"))) {
        return OPERATION_REPLACE;
    } else if (xmlStrEqual(op.str, xmlString("fuse"))) {
        return OPERATION_FUSE;
    } else if (xmlStrEqual(op.str, xmlString("remove"))) {
        return OPERATION_REMOVE;
    } else {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid op attribute in ")) +
             fromXmlString(doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
}

Type getTypeAttribute(xmlDocPtr doc, xmlNodePtr node) {
    XmlString type(
        xmlGetNsProp(
            node, xmlString("type"),
            xmlString("http://openoffice.org/2001/registry")));
    if (type.str == 0) {
        return TYPE_ERROR;
    }
    xmlChar const * p = xmlStrchr(type.str, ':');
    if (p == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid type attribute ")) +
             fromXmlString(doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString prefix(xmlStrsub(type.str, 0, p - type.str));
    xmlNsPtr ns(xmlSearchNs(doc, node, prefix.str));
    if (ns == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid type attribute ")) +
             fromXmlString(doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (xmlStrEqual(ns->href, xmlString("http://www.w3.org/2001/XMLSchema"))) {
        if (xmlStrEqual(p + 1, xmlString("boolean"))) {
            return TYPE_BOOLEAN;
        } else if (xmlStrEqual(p + 1, xmlString("short"))) {
            return TYPE_SHORT;
        } else if (xmlStrEqual(p + 1, xmlString("int"))) {
            return TYPE_INT;
        } else if (xmlStrEqual(p + 1, xmlString("long"))) {
            return TYPE_LONG;
        } else if (xmlStrEqual(p + 1, xmlString("double"))) {
            return TYPE_DOUBLE;
        } else if (xmlStrEqual(p + 1, xmlString("string"))) {
            return TYPE_STRING;
        } else if (xmlStrEqual(p + 1, xmlString("hexBinary"))) {
            return TYPE_HEXBINARY;
        }
    } else if (xmlStrEqual(
                   ns->href, xmlString("http://openoffice.org/2001/registry")))
    {
        if (xmlStrEqual(p + 1, xmlString("any"))) {
            return TYPE_ANY;
        } else if (xmlStrEqual(p + 1, xmlString("boolean-list"))) {
            return TYPE_BOOLEAN_LIST;
        } else if (xmlStrEqual(p + 1, xmlString("short-list"))) {
            return TYPE_SHORT_LIST;
        } else if (xmlStrEqual(p + 1, xmlString("int-list"))) {
            return TYPE_INT_LIST;
        } else if (xmlStrEqual(p + 1, xmlString("long-list"))) {
            return TYPE_LONG_LIST;
        } else if (xmlStrEqual(p + 1, xmlString("double-list"))) {
            return TYPE_DOUBLE_LIST;
        } else if (xmlStrEqual(p + 1, xmlString("string-list"))) {
            return TYPE_STRING_LIST;
        } else if (xmlStrEqual(p + 1, xmlString("hexBinary-list"))) {
            return TYPE_HEXBINARY_LIST;
        }
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("invalid type attribute ")) +
         fromXmlString(doc->URL)),
        css::uno::Reference< css::uno::XInterface >());
}

bool getBooleanAttribute(
    xmlDocPtr doc, xmlNodePtr node, char const * ns, char const * name,
    bool deflt)
{
    XmlString attr(xmlGetNsProp(node, xmlString(name), xmlString(ns)));
    if (attr.str == 0) {
        return deflt;
    }
    if (xmlStrEqual(attr.str, xmlString("true"))) {
        return true;
    }
    if (xmlStrEqual(attr.str, xmlString("false"))) {
        return false;
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("bad boolean attribute value in ")) +
         fromXmlString(doc->URL)),
        css::uno::Reference< css::uno::XInterface >());
}

bool parseBooleanValue(
    xmlChar const * text, xmlChar const * end, sal_Bool * value)
{
    OSL_ASSERT(text != 0 && value != 0);
    if (end == 0) {
        if (xmlStrEqual(text, xmlString("true")) ||
            xmlStrEqual(text, xmlString("1")))
        {
            *value = true;
            return true;
        }
        if (xmlStrEqual(text, xmlString("false")) ||
            xmlStrEqual(text, xmlString("0")))
        {
            *value = false;
            return true;
        }
    } else {
        int len = xmlStrlen(text);
        if ((len == RTL_CONSTASCII_LENGTH("true") &&
             (xmlStrncmp(text, xmlString("true"), RTL_CONSTASCII_LENGTH("true"))
              == 0)) ||
            (len == RTL_CONSTASCII_LENGTH("1") &&
             xmlStrncmp(text, xmlString("1"), RTL_CONSTASCII_LENGTH("1")) == 0))
        {
            *value = true;
            return true;
        }
        if ((len == RTL_CONSTASCII_LENGTH("false") &&
             (xmlStrncmp(
                 text, xmlString("false"), RTL_CONSTASCII_LENGTH("false"))
              == 0)) ||
            (len == RTL_CONSTASCII_LENGTH("0") &&
             xmlStrncmp(text, xmlString("0"), RTL_CONSTASCII_LENGTH("0")) == 0))
        {
            *value = false;
            return true;
        }
    }
    return false;
}

bool parseShortValue(
    xmlChar const * text, xmlChar const * end, sal_Int16 * value)
{
    OSL_ASSERT(text != 0 && value != 0);
    sal_Int32 n;
    if (end == 0) {
        n = rtl_str_toInt32(reinterpret_cast< char const * >(text), 10);
            //TODO: check valid lexical representation
    } else {
        n = fromXmlString(text, end).toInt32();
            //TODO: check valid lexical representation
    }
    if (n >= SAL_MIN_INT16 && n <= SAL_MAX_INT16) {
        *value = static_cast< sal_Int16 >(n);
        return true;
    }
    return false;
}

bool parseIntValue(
    xmlChar const * text, xmlChar const * end, sal_Int32 * value)
{
    OSL_ASSERT(text != 0 && value != 0);
    if (end == 0) {
        *value = rtl_str_toInt32(reinterpret_cast< char const * >(text), 10);
            //TODO: check valid lexical representation
        return true;
    }
    *value = fromXmlString(text, end).toInt32();
        //TODO: check valid lexical representation
    return true;
}

bool parseLongValue(
    xmlChar const * text, xmlChar const * end, sal_Int64 * value)
{
    OSL_ASSERT(text != 0 && value != 0);
    if (end == 0) {
        *value = rtl_str_toInt64(reinterpret_cast< char const * >(text), 10);
            //TODO: check valid lexical representation
        return true;
    }
    *value = fromXmlString(text, end).toInt64();
        //TODO: check valid lexical representation
    return true;
}

bool parseDoubleValue(xmlChar const * text, xmlChar const * end, double * value)
{
    OSL_ASSERT(text != 0 && value != 0);
    if (end == 0) {
        *value = rtl_str_toDouble(reinterpret_cast< char const * >(text));
            //TODO: check valid lexical representation
        return true;
    }
    *value = fromXmlString(text, end).toDouble();
        //TODO: check valid lexical representation
    return true;
}

bool parseStringValue(
    xmlChar const * text, xmlChar const * end, rtl::OUString * value)
{
    OSL_ASSERT((text != 0 || end == 0) && value != 0);
    if (end == 0) {
        *value = fromXmlString(text);
        return true;
    }
    *value = fromXmlString(text, end);
    return true;
}

enum ParseResult { PARSE_END, PARSE_GOOD, PARSE_BAD };

ParseResult parseHexDigit(
    xmlChar const * text, xmlChar const * end, int * value)
{
    OSL_ASSERT(text != 0 && value != 0);
    if (end == 0 ? *text == '\0' : text == end) {
        return PARSE_END;
    }
    xmlChar c = *text;
    if (c >= '0' && c <= '9') {
        *value = c - '0';
        return PARSE_GOOD;
    }
    if (c >= 'A' && c <= 'F') {
        *value = c - 'A' + 10;
        return PARSE_GOOD;
    }
    if (c >= 'a' && c <= 'f') {
        *value = c - 'a' + 10;
        return PARSE_GOOD;
    }
    return PARSE_BAD;
}

bool parseHexbinaryValue(
    xmlChar const * text, xmlChar const * end,
    css::uno::Sequence< sal_Int8 > * value)
{
    OSL_ASSERT((text != 0 || end == 0) && value != 0);
    comphelper::SequenceAsVector< sal_Int8 > seq;
    if (text != 0) {
        for (xmlChar const * p = text;;) {
            int n1;
            switch (parseHexDigit(p++, end, &n1)) {
            case PARSE_END:
                goto done;
            case PARSE_GOOD:
                break;
            case PARSE_BAD:
                return false;
            }
            int n2;
            if (parseHexDigit(p++, end, &n2) != PARSE_GOOD) {
                return false;
            }
            seq.push_back(static_cast< sal_Int8 >((n1 << 4) | n2));
        }
    done:;
    }
    *value = seq.getAsConstList();
    return true;
}

template< typename T > xmlChar const * parseEscapedValue(
    xmlChar const * text, bool (* parse)(xmlChar const *, xmlChar const *, T *),
    T * value)
{
    rtl::OString unesc;
    text = unescapeText(text, true, &unesc);
    return
        (text != 0 &&
         (*parse)(
             reinterpret_cast< xmlChar const * >(unesc.getStr()),
             (reinterpret_cast< xmlChar const * >(unesc.getStr()) +
              unesc.getLength()),
             value))
        ? text : 0;
}

template< typename T > bool parseListValue(
    xmlDocPtr doc, xmlNodePtr node, xmlChar const * text,
    bool (* parse)(xmlChar const *, xmlChar const *, T *),
    css::uno::Sequence< T > * value)
{
    OSL_ASSERT(parse != 0 && value != 0);
    comphelper::SequenceAsVector< T > seq;
    if (text != 0) {
        XmlString sepAttr(
            xmlGetNsProp(
                node, xmlString("separator"),
                xmlString("http://openoffice.org/2001/registry")));
        if (getBooleanAttribute(
                doc, node, "http://openoffice.org/2001/registry", "escaped",
                false))
        {
            for (;;) {
                T val;
                text = parseEscapedValue(text, parse, &val);
                if (text == 0) {
                    return false;
                }
                seq.push_back(val);
                if (*text == 0) {
                    break;
                }
                if (sepAttr.str == 0) {
                    xmlChar const * p = text;
                    while (*p == ' ' || *p == '\t' || *p == '\0x0A' ||
                           *p == '\0x0D')
                    {
                        ++p;
                    }
                    if (p == text) {
                        return false;
                    }
                    text = p;
                } else {
                    int sepLen = xmlStrlen(sepAttr.str);
                    if (xmlStrncmp(text, sepAttr.str, sepLen) != 0) {
                        return false;
                    }
                    text += sepLen;
                }
            }
        } else {
            XmlString col;
            xmlChar const * p;
            xmlChar const * sep;
            int sepLen;
            if (sepAttr.str == 0) {
                col = xmlSchemaCollapseString(text);
                p = col.str == 0 ? text : col.str;
                sep = xmlString(" ");
                sepLen = RTL_CONSTASCII_LENGTH(" ");
            } else {
                p = text;
                sep = sepAttr.str;
                sepLen = xmlStrlen(sep);
            }
            if (*p != '\0') {
                for (;;) {
                    xmlChar const * q = xmlStrstr(p, sep);
                    T val;
                    if (!(*parse)(p, q, &val)) {
                        return false;
                    }
                    seq.push_back(val);
                    if (q == 0) {
                        break;
                    }
                    p = q + sepLen;
                }
            }
        }
    }
    *value = seq.getAsConstList();
    return true;
}

css::uno::Any parseValue(xmlDocPtr doc, xmlNodePtr node, Type type) {
    XmlString text(xmlNodeListGetString(doc, node->xmlChildrenNode, 1));
    switch (type) {
    case TYPE_ANY:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid value of type any in ")) +
             fromXmlString(doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    case TYPE_BOOLEAN:
        if (text.str != 0) {
            XmlString col(xmlSchemaCollapseString(text.str));
            sal_Bool val;
            if (parseBooleanValue(col.str == 0 ? text.str : col.str, 0, &val)) {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_SHORT:
        if (text.str != 0) {
            XmlString col(xmlSchemaCollapseString(text.str));
            sal_Int16 val;
            if (parseShortValue(col.str == 0 ? text.str : col.str, 0, &val)) {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_INT:
        if (text.str != 0) {
            XmlString col(xmlSchemaCollapseString(text.str));
            sal_Int32 val;
            if (parseIntValue(col.str == 0 ? text.str : col.str, 0, &val)) {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_LONG:
        if (text.str != 0) {
            XmlString col(xmlSchemaCollapseString(text.str));
            sal_Int64 val;
            if (parseLongValue(col.str == 0 ? text.str : col.str, 0, &val)) {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_DOUBLE:
        if (text.str != 0) {
            XmlString col(xmlSchemaCollapseString(text.str));
            double val;
            if (parseDoubleValue(col.str == 0 ? text.str : col.str, 0, &val)) {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_STRING:
        if (getBooleanAttribute(
                doc, node, "http://openoffice.org/2001/registry", "escaped",
                false))
        {
            rtl::OUString val;
            xmlChar const * p = parseEscapedValue(
                text.str, &parseStringValue, &val);
            if (p != 0 && *p == '\0') {
                return css::uno::makeAny(val);
            }
        } else {
            rtl::OUString val;
            if (parseStringValue(text.str, 0, &val)) {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_HEXBINARY:
        {
            XmlString col;
            if (text.str != 0) {
                col = xmlSchemaCollapseString(text.str);
            }
            css::uno::Sequence< sal_Int8 > val;
            if (parseHexbinaryValue(col.str == 0 ? text.str : col.str, 0, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_BOOLEAN_LIST:
        {
            css::uno::Sequence< sal_Bool > val;
            if (parseListValue(doc, node, text.str, &parseBooleanValue, &val)) {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_SHORT_LIST:
        {
            css::uno::Sequence< sal_Int16 > val;
            if (parseListValue(doc, node, text.str, &parseShortValue, &val)) {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_INT_LIST:
        {
            css::uno::Sequence< sal_Int32 > val;
            if (parseListValue(doc, node, text.str, &parseIntValue, &val)) {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_LONG_LIST:
        {
            css::uno::Sequence< sal_Int64 > val;
            if (parseListValue(doc, node, text.str, &parseLongValue, &val)) {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_DOUBLE_LIST:
        {
            css::uno::Sequence< double > val;
            if (parseListValue(doc, node, text.str, &parseDoubleValue, &val)) {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_STRING_LIST:
        {
            css::uno::Sequence< rtl::OUString > val;
            if (parseListValue(doc, node, text.str, &parseStringValue, &val)) {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_HEXBINARY_LIST:
        {
            css::uno::Sequence< css::uno::Sequence< sal_Int8 > > val;
            if (parseListValue(doc, node, text.str, &parseHexbinaryValue, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    default:
        OSL_ASSERT(false);
        break;
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid value in ")) +
         fromXmlString(doc->URL)),
        css::uno::Reference< css::uno::XInterface >());
}

rtl::OUString parseTemplateReference(
    rtl::OUString const & componentName, xmlDocPtr doc, xmlNodePtr node,
    rtl::OUString const * defaultTemplateName)
{
    XmlString nodeType(
        xmlGetNsProp(
            node, xmlString("node-type"),
            xmlString("http://openoffice.org/2001/registry")));
    if (nodeType.str == 0) {
        if (defaultTemplateName != 0) {
            return *defaultTemplateName;
        }
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: missing node-type attribute in ")) +
             fromXmlString(doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString component(
        xmlGetNsProp(
            node, xmlString("component"),
            xmlString("http://openoffice.org/2001/registry")));
    return fullTemplateName(
        (component.str == 0 ? componentName : fromXmlString(component.str)),
        fromXmlString(nodeType.str));
}

void parseXcsGroupContent(
    rtl::OUString const & componentName, xmlDocPtr doc, xmlNodePtr node,
    rtl::Reference< GroupNode > const & group,
    Components::TemplateMap const & templates);

rtl::Reference< Node > parseXcsGroup(
    rtl::OUString const & componentName, xmlDocPtr doc, xmlNodePtr node,
    rtl::OUString const & templateName,
    Components::TemplateMap const & templates)
{
    rtl::Reference< GroupNode > group(
        new GroupNode(
            getBooleanAttribute(
                doc, node, "http://openoffice.org/2001/registry", "extensible",
                false),
            templateName));
    parseXcsGroupContent(componentName, doc, node, group, templates);
    return group.get();
}

Node * parseXcsSet(
    rtl::OUString const & componentName, xmlDocPtr doc, xmlNodePtr node,
    rtl::OUString const & templateName)
{
    xmlNodePtr p(skipBlank(node->xmlChildrenNode));
    if (isOorElement(p, "info")) {
        p = skipBlank(p->next);
    }
    std::vector< rtl::OUString > additional;
    while (isOorElement(p, "item")) {
        additional.push_back(parseTemplateReference(componentName, doc, p, 0));
        p = skipBlank(p->next);
    }
    if (p != 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: bad set content in ")) +
             fromXmlString(doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    return new SetNode(
        parseTemplateReference(componentName, doc, node, 0), additional,
        templateName);
}

void parseXcsGroupContent(
    rtl::OUString const & componentName, xmlDocPtr doc, xmlNodePtr node,
    rtl::Reference< GroupNode > const & group,
    Components::TemplateMap const & templates)
{
    xmlNodePtr p(skipBlank(node->xmlChildrenNode));
    if (isOorElement(p, "info")) {
        p = skipBlank(p->next);
    }
    for (; p != 0; p = skipBlank(p->next)) {
        rtl::Reference< Node > member;
        if (isOorElement(p, "prop")) {
            Type type(getTypeAttribute(doc, p));
            if (type == TYPE_ERROR) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcs: missing type attribute in ")) +
                     fromXmlString(doc->URL)),
                    css::uno::Reference< css::uno::XInterface >());
            }
            xmlNodePtr q(skipBlank(p->xmlChildrenNode));
            if (isOorElement(q, "info")) {
                q = skipBlank(q->next);
            }
            if (isOorElement(q, "constraints")) { //TODO
                q = skipBlank(q->next);
            }
            LocalizedValues values;
            if (isOorElement(q, "value")) {
                values.insert(
                    LocalizedValues::value_type(
                        rtl::OUString(), parseValue(doc, q, type)));
                q = skipBlank(q->next);
            }
            if (q != 0) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcs: bad prop content in ")) +
                     fromXmlString(doc->URL)),
                    css::uno::Reference< css::uno::XInterface >());
            }
            if (getBooleanAttribute(
                    doc, p, "http://openoffice.org/2001/registry", "localized",
                    false))
            {
                rtl::Reference< LocalizedPropertyNode > locprop(
                    new LocalizedPropertyNode(
                        type,
                        getBooleanAttribute(
                            doc, p, "http://openoffice.org/2001/registry",
                            "nillable", true)));
                setValues(locprop.get(), values);
                member = locprop.get();
            } else {
                member = new PropertyNode(
                    type,
                    getBooleanAttribute(
                        doc, p, "http://openoffice.org/2001/registry",
                        "nillable", true),
                    values[rtl::OUString()], false);
            }
        } else if (isOorElement(p, "node-ref")) {
            xmlNodePtr q(skipBlank(p->xmlChildrenNode));
            if (isOorElement(q, "info")) {
                q = skipBlank(q->next);
            }
            if (q != 0) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcs: bad set content in ")) +
                     fromXmlString(doc->URL)),
                    css::uno::Reference< css::uno::XInterface >());
            }
            member = new NodeRef(
                parseTemplateReference(componentName, doc, p, 0), templates);
        } else if (isOorElement(p, "group")) {
            member = parseXcsGroup(
                componentName, doc, p, rtl::OUString(), templates);
        } else if (isOorElement(p, "set")) {
            member = parseXcsSet(componentName, doc, p, rtl::OUString());
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xcs: bad component-schema/component content in ")) +
                 fromXmlString(doc->URL)),
                css::uno::Reference< css::uno::XInterface >());
        }
        if (!group->getMembers().insert(
                NodeMap::value_type(getNameAttribute(doc, p), member)).
            second)
        {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xcs: multiple members with same name in ")) +
                 fromXmlString(doc->URL)),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
}

xmlNodePtr parseXcsTemplates(
    rtl::OUString const & componentName, xmlDocPtr doc, xmlNodePtr node,
    Components::TemplateMap * templates)
{
    if (!isOorElement(node, "templates")) {
        return node;
    }
    xmlNodePtr p(skipBlank(node->xmlChildrenNode));
    if (isOorElement(p, "info")) {
        p = skipBlank(p->next);
    }
    for (; p != 0; p = skipBlank(p->next)) {
        rtl::OUString tmplName;
        rtl::Reference< Node > tmpl;
        if (isOorElement(p, "group")) {
            tmplName = fullTemplateName(
                componentName, getNameAttribute(doc, p));
            tmpl = parseXcsGroup(componentName, doc, p, tmplName, *templates);
        } else if (isOorElement(p, "set")) {
            tmplName = fullTemplateName(
                componentName, getNameAttribute(doc, p));
            tmpl = parseXcsSet(componentName, doc, p, tmplName);
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xcs: bad component-schema/templates content in ")) +
                 fromXmlString(doc->URL)),
                css::uno::Reference< css::uno::XInterface >());
        }
        if (!templates->insert(NodeMap::value_type(tmplName, tmpl)).second) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xcs: multiple templates with same name in ")) +
                 fromXmlString(doc->URL)),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
    return node->next;
}

xmlNodePtr parseXcsComponent(
    xmlDocPtr doc, rtl::OUString const & component, xmlNodePtr node,
    NodeMap * components, Components::TemplateMap const & templates)
{
    if (!isOorElement(node, "component")) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: bad component-schema content in ")) +
             fromXmlString(doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::Reference< GroupNode > comp(new GroupNode(false, rtl::OUString()));
    parseXcsGroupContent(component, doc, node, comp, templates);
    if (!components->insert(NodeMap::value_type(component, comp.get())).second)
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: multiply defined component-schema in ")) +
             fromXmlString(doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    return node->next;
}

void parseXcsFile(
    rtl::OUString const & url, Components::TemplateMap * templates,
    NodeMap * components)
{
    XmlDoc doc(parseXmlFile(url));
    xmlNodePtr root(xmlDocGetRootElement(doc.doc));
    if (root == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("xcs: no root element in ")) +
             fromXmlString(doc.doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (!xmlStrEqual(root->name, xmlString("component-schema")) ||
        root->ns == 0 ||
        !xmlStrEqual(
            root->ns->href, xmlString("http://openoffice.org/2001/registry")))
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: non component-schema root element in ")) +
             fromXmlString(doc.doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString package(
        xmlGetNsProp(
            root, xmlString("package"),
            xmlString("http://openoffice.org/2001/registry")));
    if (package.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: no root package attribute in ")) +
             fromXmlString(doc.doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUStringBuffer buf(fromXmlString(package.str));
    buf.append(sal_Unicode('.'));
    buf.append(getNameAttribute(doc.doc, root));
    rtl::OUString comp(buf.makeStringAndClear());
    //TODO: root xml:lang attribute
    xmlNodePtr p(skipBlank(root->xmlChildrenNode));
    if (isOorElement(p, "info")) {
        p = skipBlank(p->next);
    }
    while (isOorElement(p, "import")) { //TODO: process?
        p = skipBlank(p->next);
    }
    while (isOorElement(p, "uses")) { //TODO: process?
        p = skipBlank(p->next);
    }
    p = skipBlank(parseXcsTemplates(comp, doc.doc, p, templates));
    p = skipBlank(parseXcsComponent(doc.doc, comp, p, components, *templates));
    if (p != 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcs: bad component-schema content in ")) +
             fromXmlString(doc.doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void parseXcuNode(
    rtl::OUString const & componentName,
    Components::TemplateMap const & templates, xmlDocPtr doc,
    xmlNodePtr xmlNode, rtl::Reference< Node > const & node,
    Components * modifications, rtl::OUString const & pathPrefix)
{
    if (GroupNode * group = dynamic_cast< GroupNode * >(node.get())) {
        for (xmlNodePtr p(skipBlank(xmlNode->xmlChildrenNode)); p != 0;
             p = skipBlank(p->next))
        {
            if (isOorElement(p, "prop")) {
                rtl::OUString name(getNameAttribute(doc, p));
                PropertyNode * property = 0;
                LocalizedPropertyNode * localized = 0;
                NodeMap::iterator i(
                    Components::resolveNode(name, &group->getMembers()));
                if (i != group->getMembers().end()) {
                    property = dynamic_cast< PropertyNode * >(i->second.get());
                    localized = dynamic_cast< LocalizedPropertyNode * >(
                        i->second.get());
                    if (property == 0 && localized == 0) {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: inappropriate prop in ")) +
                             fromXmlString(doc->URL)),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                }
                Type type(getTypeAttribute(doc, p));
                if (type == TYPE_ANY) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "xcu: invalid type attribute in ")) +
                         fromXmlString(doc->URL)),
                        css::uno::Reference< css::uno::XInterface >());
                }
                Operation op(getOperationAttribute(doc, p));
                //TODO: oor:finalized attributes
                xmlNodePtr q(skipBlank(p->xmlChildrenNode));
                LocalizedValues values;
                while (isOorElement(q, "value")) {
                    if (type == TYPE_ERROR) {
                        if (property != 0) {
                            type = property->getType();
                        } else if (localized != 0) {
                            type = localized->getType();
                        } else {
                            throw css::uno::RuntimeException(
                                (rtl::OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "xcu: missing type attribute in ")) +
                                 fromXmlString(doc->URL)),
                                css::uno::Reference< css::uno::XInterface >());
                        }
                        if (type == TYPE_ANY) {
                            throw css::uno::RuntimeException(
                                (rtl::OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "xcu: missing type attribute in ")) +
                                 fromXmlString(doc->URL)),
                                css::uno::Reference< css::uno::XInterface >());
                        }
                    }
                    bool nil = getBooleanAttribute(
                        doc, q, "http://www.w3.org/2001/XMLSchema-instance",
                        "nil", false);
                    if (nil &&
                        ((property != 0 && !property->isNillable()) ||
                         (localized != 0 && !localized->isNillable())))
                    {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: xsi:nil attribute for non-nillable"
                                    " prop in ")) +
                             fromXmlString(doc->URL)),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    XmlString lang(xmlNodeGetLang(q));
                    if (lang.str != 0 && localized == 0) {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: xml:lang attribute for non-localized"
                                    " prop in ")) +
                             fromXmlString(doc->URL)),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    bool remove;
                    switch (getOperationAttribute(doc, q)) {
                    case OPERATION_MODIFY:
                        remove = false;
                        break;
                    case OPERATION_REMOVE:
                        //TODO: disallow removing when e.g. lang=""?
                        if (localized != 0 && op == OPERATION_FUSE) {
                            remove = true;
                            break;
                        }
                        // fall through
                    default:
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: bad op attribute for value element"
                                    " in ")) +
                             fromXmlString(doc->URL)),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    // For nil and removed values, any actually provided value
                    // is simply ignored for now:
                    if (!values.insert(
                            LocalizedValues::value_type(
                                (lang.str == 0
                                 ? rtl::OUString() : fromXmlString(lang.str)),
                                (remove
                                 ? css::uno::makeAny(
                                     cppu::UnoType< cppu::UnoVoidType >::get())
                                 : (nil
                                    ? css::uno::Any()
                                    : parseValue(doc, q, type))))).
                        second)
                    {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: multiple values (for same xml:lang)"
                                    " in ")) +
                             fromXmlString(doc->URL)),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    q = skipBlank(q->next);
                }
                if (q != 0) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "xcu: bad prop content in ")) +
                         fromXmlString(doc->URL)),
                        css::uno::Reference< css::uno::XInterface >());
                }
                if (i == group->getMembers().end()) {
                    if (!group->isExtensible()) {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: unknown prop name in ")) +
                             fromXmlString(doc->URL)),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    switch (op) {
                    case OPERATION_MODIFY:
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: invalid modify of extension property"
                                    " in ")) +
                             fromXmlString(doc->URL)),
                            css::uno::Reference< css::uno::XInterface >());
                    case OPERATION_REPLACE:
                    case OPERATION_FUSE:
                        group->getMembers().insert(
                            NodeMap::value_type(
                                name,
                                new PropertyNode(
                                    TYPE_ANY, true, values[rtl::OUString()],
                                    true)));
                        break;
                    case OPERATION_REMOVE:
                        // ignore unknown (presumably extension) properties
                        break;
                    }
                } else {
                    switch (op) {
                    case OPERATION_MODIFY:
                    case OPERATION_FUSE:
                        if (property != 0) {
                            property->setValue(values[rtl::OUString()]);
                        } else {
                            for (LocalizedValues::iterator j(values.begin());
                                 j != values.end(); ++j)
                            {
                                if (isRemovedValue(j->second)) {
                                    localized->getMembers().erase(j->first);
                                } else {
                                    NodeMap::iterator k(
                                        Components::resolveNode(
                                            j->first,
                                            &localized->getMembers()));
                                    if (k == localized->getMembers().end()) {
                                        localized->getMembers().insert(
                                            NodeMap::value_type(
                                                j->first,
                                                new LocalizedPropertyValueNode(
                                                    j->second)));
                                    } else {
                                        dynamic_cast<
                                            LocalizedPropertyValueNode * >(
                                                k->second.get())->
                                            setValue(j->second);
                                    }
                                }
                            }
                        }
                        break;
                    case OPERATION_REPLACE:
                        if (property != 0) {
                            property->setValue(values[rtl::OUString()]);
                        } else {
                            localized->getMembers().clear();
                            setValues(localized, values);
                        }
                        break;
                    case OPERATION_REMOVE:
                        {
                            if (!property->isExtension()) {
                                throw css::uno::RuntimeException(
                                    (rtl::OUString(
                                        RTL_CONSTASCII_USTRINGPARAM(
                                            "xcu: invalid remove of"
                                            " non-extension property in ")) +
                                     fromXmlString(doc->URL)),
                                    css::uno::Reference<
                                        css::uno::XInterface >());
                            }
                            group->getMembers().erase(i);
                        }
                        break;
                    }
                }
                if (modifications != 0) {
                    if (localized == 0) {
                        modifications->addModification(pathPrefix + name);
                    } else {
                        rtl::OUString path(
                            pathPrefix + name +
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")));
                        for (LocalizedValues::iterator j(values.begin());
                             j != values.end(); ++j)
                        {
                            modifications->addModification(
                                path +
                                Components::createSegment(
                                    rtl::OUString(
                                        RTL_CONSTASCII_USTRINGPARAM("*")),
                                    j->first));
                        }
                    }
                }
            } else if (isOorElement(p, "node")) {
                Operation op = getOperationAttribute(doc, p);
                if ((op != OPERATION_MODIFY && op != OPERATION_FUSE) ||
                    modifications != 0)
                {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "xcu: invalid operation on group node in ")) +
                         fromXmlString(doc->URL)),
                        css::uno::Reference< css::uno::XInterface >());
                }
                //TODO: oor:component, oor:finalized, oor:mandatory, oor:node-type attributes
                NodeMap::iterator i(
                    Components::resolveNode(
                        getNameAttribute(doc, p), &group->getMembers()));
                if (i == group->getMembers().end()) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "xcu: unknown node name in ")) +
                         fromXmlString(doc->URL)),
                        css::uno::Reference< css::uno::XInterface >());
                }
                parseXcuNode(
                    componentName, templates, doc, p, i->second, 0,
                    rtl::OUString());
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcu: bad component-data or node content in ")) +
                     fromXmlString(doc->URL)),
                    css::uno::Reference< css::uno::XInterface >());
            }
        }
    } else if (SetNode * set = dynamic_cast< SetNode * >(node.get())) {
        for (xmlNodePtr p(skipBlank(xmlNode->xmlChildrenNode)); p != 0;
             p = skipBlank(p->next))
        {
            if (!isOorElement(p, "node")) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcu: non-node element within set node element"
                            " in ")) +
                     fromXmlString(doc->URL)),
                    css::uno::Reference< css::uno::XInterface >());
            }
            rtl::OUString name(getNameAttribute(doc, p));
            rtl::OUString templateName(
                parseTemplateReference(
                    componentName, doc, p, &set->getDefaultTemplateName()));
            if (!set->isValidTemplate(templateName)) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcu: set member node references invalid template"
                            " in ")) +
                     fromXmlString(doc->URL)),
                    css::uno::Reference< css::uno::XInterface >());
            }
            Components::TemplateMap::const_iterator i(
                templates.find(templateName));
            if (i == templates.end()) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xcu: set member node references undefined template"
                            " in ")) +
                     fromXmlString(doc->URL)),
                    css::uno::Reference< css::uno::XInterface >());
            }
            switch (getOperationAttribute(doc, p)) {
            case OPERATION_MODIFY:
                {
                    NodeMap::iterator j(
                        Components::resolveNode(name, &set->getMembers()));
                    if (j == set->getMembers().end()) {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "xcu: invalid modify of unknown set member"
                                    " node in ")) +
                             fromXmlString(doc->URL)),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    parseXcuNode(
                        componentName, templates, doc, p, j->second, 0,
                        rtl::OUString());
                }
                break;
            case OPERATION_REPLACE:
                {
                    rtl::Reference< Node > member(i->second->clone());
                    parseXcuNode(
                        componentName, templates, doc, p, member, 0,
                        rtl::OUString());
                    NodeMap::iterator j(
                        Components::resolveNode(name, &set->getMembers()));
                    if (j == set->getMembers().end()) {
                        set->getMembers().insert(
                            NodeMap::value_type(name, member));
                    } else {
                        j->second = member;
                    };
                }
                break;
            case OPERATION_FUSE:
                {
                    NodeMap::iterator j(
                        Components::resolveNode(name, &set->getMembers()));
                    if (j == set->getMembers().end()) {
                        rtl::Reference< Node > member(i->second->clone());
                        parseXcuNode(
                            componentName, templates, doc, p, member, 0,
                            rtl::OUString());
                        set->getMembers().insert(
                            NodeMap::value_type(name, member));
                    } else {
                        parseXcuNode(
                            componentName, templates, doc, p, j->second, 0,
                            rtl::OUString());
                    }
                }
                break;
            case OPERATION_REMOVE:
                {
                    NodeMap::iterator j(
                        Components::resolveNode(name, &set->getMembers()));
                    // Ignore unknown members:
                    if (j != set->getMembers().end()) {
                        set->getMembers().erase(j);
                    }
                }
                break;
            }
            if (modifications != 0) {
                modifications->addModification(
                    pathPrefix + Components::createSegment(templateName, name));
            }
        }
    } else {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("xcu: inappropriate node in ")) +
             fromXmlString(doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void parseXcuFile(
    rtl::OUString const & url, Components::TemplateMap * templates,
    NodeMap * components)
{
    XmlDoc doc(parseXmlFile(url));
    xmlNodePtr root(xmlDocGetRootElement(doc.doc));
    if (root == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("xcu: no root element in ")) +
             fromXmlString(doc.doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (!xmlStrEqual(root->name, xmlString("component-data")) ||
        root->ns == 0 ||
        !xmlStrEqual(
            root->ns->href, xmlString("http://openoffice.org/2001/registry")))
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcu: non component-data root element in ")) +
             fromXmlString(doc.doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString package(
        xmlGetNsProp(
            root, xmlString("package"),
            xmlString("http://openoffice.org/2001/registry")));
    if (package.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcu: no root package attribute in ")) +
             fromXmlString(doc.doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUStringBuffer buf(fromXmlString(package.str));
    buf.append(sal_Unicode('.'));
    buf.append(getNameAttribute(doc.doc, root));
    rtl::OUString comp(buf.makeStringAndClear());
    //TODO: root oor:finalized, oor:op attributes
    NodeMap::iterator i(Components::resolveNode(comp, components));
    if (i == components->end()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xcu: unknown component-data name in ")) +
             fromXmlString(doc.doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    parseXcuNode(
        comp, *templates, doc.doc, root, i->second, 0, rtl::OUString());
}

void parseFiles(
    rtl::OUString const & extension,
    void (* parseFile)(
        rtl::OUString const &, Components::TemplateMap *, NodeMap *),
    rtl::OUString const & url, Components::TemplateMap * templates,
    NodeMap * components, bool recursive)
{
    osl::Directory dir(url);
    switch (dir.open()) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        if (!recursive) {
            return;
        }
        // fall through
    default:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("cannot open directory ")) +
             url),
            css::uno::Reference< css::uno::XInterface >());
    }
    for (;;) {
        osl::DirectoryItem i;
        osl::FileBase::RC rc = dir.getNextItem(i, SAL_MAX_UINT32);
        if (rc == osl::FileBase::E_NOENT) {
            break;
        }
        if (rc != osl::FileBase::E_None) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("cannot iterate directory ")) +
                 url),
                css::uno::Reference< css::uno::XInterface >());
        }
        osl::FileStatus stat(
            FileStatusMask_Type | FileStatusMask_FileName |
            FileStatusMask_FileURL);
        if (i.getFileStatus(stat) != osl::FileBase::E_None) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("cannot stat in directory ")) +
                 url),
                css::uno::Reference< css::uno::XInterface >());
        }
        if (stat.getFileType() == osl::FileStatus::Directory) { //TODO: symlinks
            parseFiles(
                extension, parseFile, stat.getFileURL(), templates, components,
                true);
        } else {
            rtl::OUString file(stat.getFileName());
            if (file.getLength() >= extension.getLength() &&
                file.match(extension, file.getLength() - extension.getLength()))
            {
                (*parseFile)(stat.getFileURL(), templates, components);
            }
        }
    }
}

void parseXcsXcuLayer(
    rtl::OUString const & url, Components::TemplateMap * templates,
    NodeMap * components)
{
    parseFiles(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".xcs")), &parseXcsFile,
        url + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/schema")),
        templates, components, false);
    parseFiles(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".xcu")), &parseXcuFile,
        url + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/data")), templates,
        components, false);
}

void parseModuleLayer(
    rtl::OUString const & url, Components::TemplateMap * templates,
    NodeMap * components)
{
    parseFiles(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".xcu")), &parseXcuFile, url,
        templates, components, false);
}

void parseResLayer(
    rtl::OUString const & url, Components::TemplateMap * templates,
    NodeMap * components)
{
    parseFiles(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".xcu")), &parseXcuFile, url,
        templates, components, false);
}

void parseSystemLayer() {
    //TODO
}

void writeBooleanValue(xmlTextWriterPtr writer, sal_Bool const & value) {
    xmlTextWriterWriteString(
        writer, xmlString(value ? "true" : "false"));
}

void writeShortValue(xmlTextWriterPtr writer, sal_Int16 const & value) {
    xmlTextWriterWriteString(
        writer,
        xmlString(
            convertToUtf8(rtl::OUString::valueOf(sal_Int32(value))).getStr()));
}

void writeIntValue(xmlTextWriterPtr writer, sal_Int32 const & value) {
    xmlTextWriterWriteString(
        writer,
        xmlString(convertToUtf8(rtl::OUString::valueOf(value)).getStr()));
}

void writeLongValue(xmlTextWriterPtr writer, sal_Int64 const & value) {
    xmlTextWriterWriteString(
        writer,
        xmlString(convertToUtf8(rtl::OUString::valueOf(value)).getStr()));
}

void writeDoubleValue(xmlTextWriterPtr writer, double const & value) {
    xmlTextWriterWriteString(
        writer,
        xmlString(convertToUtf8(rtl::OUString::valueOf(value)).getStr()));
}

void writeStringValue(xmlTextWriterPtr writer, rtl::OUString const & value) {
    rtl::OUStringBuffer buf;
    buf.append(sal_Unicode('['));
    buf.append(escapeText(value, true));
    buf.append(sal_Unicode(']'));
    xmlTextWriterWriteString(
        writer, xmlString(convertToUtf8(buf.makeStringAndClear()).getStr()));
}

void writeHexbinaryValue(
    xmlTextWriterPtr writer, css::uno::Sequence< sal_Int8 > const & value)
{
    rtl::OStringBuffer buf;
    buf.append('[');
    for (sal_Int32 i = 0; i < value.getLength(); ++i) {
        static char const hexDigit[16] = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
            'D', 'E', 'F' };
        buf.append(hexDigit[(value[i] >> 4) & 0xF]);
        buf.append(hexDigit[value[i] & 0xF]);
    }
    buf.append(']');
    xmlTextWriterWriteString(
        writer, xmlString(buf.makeStringAndClear().getStr()));
}

template< typename T > void writeListValue(
    xmlTextWriterPtr writer, void (* write)(xmlTextWriterPtr, T const &),
    css::uno::Any const & value)
{
    css::uno::Sequence< T > val;
    value >>= val;
    for (sal_Int32 i = 0; i < val.getLength(); ++i) {
        if (i != 0) {
            xmlTextWriterWriteString(writer, xmlString(" "));
        }
        (*write)(writer, val[i]);
    }
}

void writeNode(
    xmlTextWriterPtr writer, rtl::OUString const & name,
    rtl::Reference< Node > const & node)
{
    if (PropertyNode * prop = dynamic_cast< PropertyNode * >(node.get())) {
        xmlTextWriterStartElement(writer, xmlString("oor:prop"));
        xmlTextWriterWriteAttribute(
            writer, xmlString("oor:name"),
            xmlString(convertToUtf8(name).getStr()));
        xmlTextWriterWriteAttribute(
            writer, xmlString("oor:op"), xmlString("fuse"));
        Type type = prop->getType();
        if (type == TYPE_ANY) {
            type = mapType(prop->getValue());
            static char const * const typeNames[] = {
                0, 0, 0, // TYPE_ERROR, TYPE_NIL, TYPE_ANY
                "xs:boolean", // TYPE_BOOLEAN
                "xs:short", // TYPE_SHORT
                "xs:int", // TYPE_INT
                "xs:long", // TYPE_LONG
                "xs:double", // TYPE_DOUBLE
                "xs:string", // TYPE_STRING
                "xs:hexBinary", // TYPE_HEXBINARY
                "oor:boolean-list", // TYPE_BOOLEAN_LIST
                "oor:short-list", // TYPE_SHORT_LIST
                "oor:int-list", // TYPE_INT_LIST
                "oor:long-list", // TYPE_LONG_LIST
                "oor:double-list", // TYPE_DOUBLE_LIST
                "oor:string-list", // TYPE_STRING_LIST
                "oor:hexBinary-list" }; // TYPE_HEXBINARY_LIST
            xmlTextWriterWriteAttribute(
                writer, xmlString("oor:type"), xmlString(typeNames[type]));
        }
        xmlTextWriterStartElement(writer, xmlString("oor:value"));
        switch (type) {
        case TYPE_NIL:
            xmlTextWriterWriteAttribute(
                writer, xmlString("xsi:nil"), xmlString("true"));
            break;
        case TYPE_BOOLEAN:
            {
                bool val;
                prop->getValue() >>= val;
                writeBooleanValue(writer, val);
            }
            break;
        case TYPE_SHORT:
            {
                sal_Int16 val;
                prop->getValue() >>= val;
                writeShortValue(writer, val);
            }
            break;
        case TYPE_INT:
            {
                sal_Int32 val;
                prop->getValue() >>= val;
                writeIntValue(writer, val);
            }
            break;
        case TYPE_LONG:
            {
                sal_Int64 val;
                prop->getValue() >>= val;
                writeLongValue(writer, val);
            }
            break;
        case TYPE_DOUBLE:
            {
                double val;
                prop->getValue() >>= val;
                writeDoubleValue(writer, val);
            }
            break;
        case TYPE_STRING:
            {
                xmlTextWriterWriteAttribute(
                    writer, xmlString("oor:escaped"), xmlString("true"));
                rtl::OUString val;
                prop->getValue() >>= val;
                writeStringValue(writer, val);
            }
            break;
        case TYPE_HEXBINARY:
            {
                // Written in escaped form to be able to share
                // writeHexbinaryValue with the TYPE_HEXBINARY_LIST case (see
                // there):
                xmlTextWriterWriteAttribute(
                    writer, xmlString("oor:escaped"), xmlString("true"));
                css::uno::Sequence< sal_Int8 > val;
                prop->getValue() >>= val;
                writeHexbinaryValue(writer, val);
            }
            break;
        case TYPE_BOOLEAN_LIST:
            writeListValue(writer, &writeBooleanValue, prop->getValue());
            break;
        case TYPE_SHORT_LIST:
            writeListValue(writer, &writeShortValue, prop->getValue());
            break;
        case TYPE_INT_LIST:
            writeListValue(writer, &writeIntValue, prop->getValue());
            break;
        case TYPE_LONG_LIST:
            writeListValue(writer, &writeLongValue, prop->getValue());
            break;
        case TYPE_DOUBLE_LIST:
            writeListValue(writer, &writeDoubleValue, prop->getValue());
            break;
        case TYPE_STRING_LIST:
            xmlTextWriterWriteAttribute(
                writer, xmlString("oor:escaped"), xmlString("true"));
            writeListValue(writer, &writeStringValue, prop->getValue());
            break;
        case TYPE_HEXBINARY_LIST:
            // Written in escaped form to distinguish an empty list from a list
            // with one empty hexbinary element:
            xmlTextWriterWriteAttribute(
                writer, xmlString("oor:escaped"), xmlString("true"));
            writeListValue(writer, &writeHexbinaryValue, prop->getValue());
            break;
        default: // TYPE_ERROR, TYPE_ANY
            OSL_ASSERT(false);
            throw css::uno::RuntimeException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
                css::uno::Reference< css::uno::XInterface >());
        }
        xmlTextWriterEndElement(writer);
        xmlTextWriterEndElement(writer);
    } else if (GroupNode * group = dynamic_cast< GroupNode * >(node.get())) {
        xmlTextWriterStartElement(writer, xmlString("oor:node"));
        xmlTextWriterWriteAttribute(
            writer, xmlString("oor:name"),
            xmlString(convertToUtf8(name).getStr()));
        if (group->getTemplateName().getLength() != 0) { // set member
            xmlTextWriterWriteAttribute(
                writer, xmlString("oor:op"), xmlString("replace"));
        }
        for (NodeMap::iterator i(group->getMembers().begin());
             i != group->getMembers().end(); ++i)
        {
            writeNode(writer, i->first, i->second);
        }
        xmlTextWriterEndElement(writer);
    } else {
        if(true)abort();*(char*)0=0;throw 0;//TODO
    }
}

}

Components & Components::singleton() {
    static Components * c = new Components(); // leaks
    return *c;
}

bool Components::allLocales(rtl::OUString const & locale) {
    return locale.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("*"));
}

rtl::OUString Components::createSegment(
    rtl::OUString const & templateName, rtl::OUString const & name)
{
    if (templateName.getLength() == 0) {
        return name;
    }
    rtl::OUStringBuffer buf(templateName);
        //TODO: verify template name contains no bad chars?
    buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("['"));
    for (sal_Int32 i = 0; i < name.getLength(); ++i) {
        sal_Unicode c = name[i];
        switch (c) {
        case '&':
            buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("&amp;"));
            break;
        case '"':
            buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("&quot;"));
            break;
        case '\'':
            buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("&apos;"));
            break;
        default:
            buf.append(c);
            break;
        }
    }
    buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("']"));
    return buf.makeStringAndClear();
}

sal_Int32 Components::parseSegment(
    rtl::OUString const & path, sal_Int32 index, rtl::OUString * name,
    bool * setElement, rtl::OUString * templateName)
{
    OSL_ASSERT(
        index >= 0 && index <= path.getLength() && name != 0 &&
        setElement != 0);
    sal_Int32 i = index;
    while (i < path.getLength() && path[i] != '/' && path[i] != '[') {
        ++i;
    }
    if (i == path.getLength() || path[i] == '/') {
        *name = path.copy(index, i - index);
        *setElement = false;
        return i;
    }
    if (templateName != 0) {
        if (i - index == 1 && path[index] == '*') {
            *templateName = rtl::OUString();
        } else {
            *templateName = path.copy(index, i - index);
        }
    }
    if (++i == path.getLength()) {
        return -1;
    }
    sal_Unicode del = path[i++];
    if (del != '\'' && del != '"') {
        return -1;
    }
    sal_Int32 j = path.indexOf(del, i);
    if (j == -1 || j + 1 == path.getLength() || path[j + 1] != ']' ||
        !decodeXml(path, i, j, name))
    {
        return -1;
    }
    *setElement = true;
    return j + 2;
}

NodeMap::iterator Components::resolveNode(
    rtl::OUString const & name, NodeMap * map)
{
    OSL_ASSERT(map != 0);
    NodeMap::iterator i(map->find(name));
    if (i != map->end()) {
        if (NodeRef * ref = dynamic_cast< NodeRef * >(i->second.get())) {
            NodeMap::const_iterator j(
                ref->getTemplates().find(ref->getTemplateName()));
            if (j == ref->getTemplates().end()) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM("unknown node-ref ")) +
                     ref->getTemplateName()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            i->second = j->second->clone();
        }
    }
    return i;
}

rtl::Reference< Node > Components::resolvePath(
    rtl::OUString const & path, rtl::OUString * firstSegment,
    rtl::OUString * lastSegment, rtl::OUString * canonicalPath)
{
    if (path.getLength() == 0 || path[0] != '/') {
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) + path, 0);
    }
    rtl::OUString seg;
    bool setElement;
    sal_Int32 n = parseSegment(path, 1, &seg, &setElement, 0);
    if (n == -1 || setElement)
    {
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) + path, 0);
    }
    if (firstSegment != 0) {
        *firstSegment = seg;
    }
    rtl::OUStringBuffer canonic;
    if (canonicalPath != 0) {
        canonic.append(sal_Unicode('/'));
        canonic.append(seg);
    }
    NodeMap::iterator i(resolveNode(seg, &components_));
    rtl::Reference< Node > p(i == components_.end() ? 0 : i->second);
    while (p != 0 && n != path.getLength()) {
        if (path[n++] != '/') {
            throw css::uno::RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) + path,
                css::uno::Reference< css::uno::XInterface >());
        }
        if (n == path.getLength()) {
            break; // for backwards compatibility, ignore a final slash
        }
        rtl::OUString templateName;
        n = parseSegment(path, n, &seg, &setElement, &templateName);
        if (n == -1) {
            throw css::uno::RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) + path,
                css::uno::Reference< css::uno::XInterface >());
        }
        if (setElement) {
            SetNode * set = dynamic_cast< SetNode * >(p.get());
            if (set == 0 ||
                (templateName.getLength() != 0 &&
                 !set->isValidTemplate(templateName)))
            {
                throw css::uno::RuntimeException(
                    (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) +
                     path),
                    css::uno::Reference< css::uno::XInterface >());
            }
            p = set->getMember(seg);
            if (templateName.getLength() != 0 && p != 0) {
                rtl::OUString name;
                if (GroupNode * mg = dynamic_cast< GroupNode * >(p.get())) {
                    name = mg->getTemplateName();
                } else if (SetNode * ms = dynamic_cast< SetNode * >(p.get())) {
                    name = ms->getTemplateName();
                } else {
                    OSL_ASSERT(false);
                    throw css::uno::RuntimeException(
                        rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
                        css::uno::Reference< css::uno::XInterface >());
                }
                OSL_ASSERT(name.getLength() != 0);
                if (templateName != name) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM("bad path ")) +
                         path),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
        } else {
            // For backwards compatibility, allow set members to be accessed
            // with simple path segments, like group members:
            p = p->getMember(seg);
        }
        if (p != 0 && canonicalPath != 0) {
            canonic.append(sal_Unicode('/'));
            canonic.append(createSegment(p->getTemplateName(), seg));
        }
    }
    if (p != 0) {
        if (lastSegment != 0) {
            *lastSegment = seg;
        }
        if (canonicalPath != 0) {
            *canonicalPath = canonic.makeStringAndClear();
        }
    }
    return p;
}

rtl::Reference< Node > Components::getTemplate(rtl::OUString const & fullName)
    const
{
    NodeMap::const_iterator i(templates_.find(fullName));
    return i == templates_.end() ? 0 : i->second;
}

void Components::addModification(rtl::OUString const & path) {
    //TODO
    for (Modifications::iterator i(modifications_.begin());
         i != modifications_.end();)
    {
        if (path == *i || isPrefix(*i, path)) {
            return;
        }
        if (isPrefix(path, *i)) {
            modifications_.erase(i++);
        } else {
            ++i;
        }
    }
    modifications_.push_back(path);
}

void Components::writeModifications() {
    rtl::OUString url(getModificationFileUrl());
    XmlTextWriter writer(
        xmlNewTextWriterFilename(convertToFilepath(url).getStr(), 0));
    if (writer.writer == 0) {
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cannot write ")) + url,
            css::uno::Reference< css::uno::XInterface >());
    }
    xmlTextWriterSetIndent(writer.writer, 1);
        //TODO: more readable, but potentially slower?
    xmlTextWriterStartDocument(writer.writer, 0, 0, 0);
    xmlTextWriterStartElement(writer.writer, xmlString("modifications"));
    xmlTextWriterWriteAttribute(
        writer.writer, xmlString("xmlns:oor"),
        xmlString("http://openoffice.org/2001/registry"));
    xmlTextWriterWriteAttribute(
        writer.writer, xmlString("xmlns:xs"),
        xmlString("http://www.w3.org/2001/XMLSchema"));
    //TODO: Do not write back information about those removed items that did not
    // come from the .xcs/.xcu files, anyway (but had been added dynamically
    // instead):
    for (Modifications::iterator i(modifications_.begin());
         i != modifications_.end(); ++i)
    {
        xmlTextWriterStartElement(writer.writer, xmlString("item"));
        rtl::OUString name;
        rtl::OUString parentPath(parseLastSegment(*i, &name));
        rtl::Reference< Node > node(resolvePath(*i, 0, 0, 0));
        if (node.is()) {
            xmlTextWriterWriteAttribute(
                writer.writer, xmlString("path"),
                xmlString(
                    convertToUtf8(escapeText(parentPath, false)).getStr()));
            writeNode(writer.writer, name, node);
        } else {
            rtl::Reference< Node > parent(resolvePath(parentPath, 0, 0, 0));
            if (dynamic_cast< LocalizedPropertyNode * >(parent.get()) != 0) {
                rtl::OUString parentName;
                rtl::OUString grandparentPath(
                    parseLastSegment(parentPath, &parentName));
                xmlTextWriterWriteAttribute(
                    writer.writer, xmlString("path"),
                    xmlString(
                        convertToUtf8(escapeText(grandparentPath, false)).
                        getStr()));
                xmlTextWriterStartElement(writer.writer, xmlString("oor:prop"));
                xmlTextWriterWriteAttribute(
                    writer.writer, xmlString("oor:name"),
                    xmlString(convertToUtf8(parentName).getStr()));
                xmlTextWriterWriteAttribute(
                    writer.writer, xmlString("oor:op"), xmlString("fuse"));
                xmlTextWriterStartElement(
                    writer.writer, xmlString("oor:value"));
                xmlTextWriterWriteAttribute(
                    writer.writer, xmlString("xml:lang"),
                    xmlString(convertToUtf8(name).getStr()));
                xmlTextWriterWriteAttribute(
                    writer.writer, xmlString("oor:op"), xmlString("remove"));
                xmlTextWriterEndElement(writer.writer);
                xmlTextWriterEndElement(writer.writer);
            } else if (dynamic_cast< GroupNode * >(parent.get()) != 0) {
                OSL_ASSERT(
                    dynamic_cast< GroupNode * >(parent.get())->isExtensible());
                xmlTextWriterWriteAttribute(
                    writer.writer, xmlString("path"),
                    xmlString(
                        convertToUtf8(escapeText(parentPath, false)).getStr()));
                xmlTextWriterStartElement(writer.writer, xmlString("oor:prop"));
                xmlTextWriterWriteAttribute(
                    writer.writer, xmlString("oor:name"),
                    xmlString(convertToUtf8(name).getStr()));
                xmlTextWriterWriteAttribute(
                    writer.writer, xmlString("oor:op"), xmlString("remove"));
                xmlTextWriterEndElement(writer.writer);
            } else {
                OSL_ASSERT(dynamic_cast< SetNode * >(parent.get()) != 0);
                xmlTextWriterWriteAttribute(
                    writer.writer, xmlString("path"),
                    xmlString(
                        convertToUtf8(escapeText(parentPath, false)).getStr()));
                xmlTextWriterStartElement(writer.writer, xmlString("oor:node"));
                xmlTextWriterWriteAttribute(
                    writer.writer, xmlString("oor:name"),
                    xmlString(convertToUtf8(name).getStr()));
                xmlTextWriterWriteAttribute(
                    writer.writer, xmlString("oor:op"), xmlString("remove"));
                xmlTextWriterEndElement(writer.writer);
            }
        }
        xmlTextWriterEndElement(writer.writer);
    }
    if (xmlTextWriterEndDocument(writer.writer) == -1) { //TODO: check all -1?
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cannot write ")) + url,
            css::uno::Reference< css::uno::XInterface >());
    }
}

namespace {

rtl::OUString expand(rtl::OUString const & str) {
    rtl::OUString s(str);
    rtl::Bootstrap::expandMacros(s); //TODO: detect failure
    return s;
}

}

Components::Components() {
    parseXcsXcuLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("$OOO_BASE_DIR/share/registry"))),
        &templates_, &components_);
    parseModuleLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "$OOO_BASE_DIR/share/registry/modules"))),
        &templates_, &components_);
    parseResLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("$OOO_BASE_DIR/share/registry"))),
        &templates_, &components_);
    parseXcsXcuLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("$BRAND_BASE_DIR/share/registry"))),
        &templates_, &components_);
    parseModuleLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "$BRAND_BASE_DIR/share/registry/modules"))),
        &templates_, &components_);
    parseXcsXcuLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$OOO_BASE_DIR/program/unorc:UNO_SHARED_PACKAGES_CACHE}/"
                    "registry/com.sun.star.comp.deployment.configuration."
                    "PackageRegistryBackend/registry"))),
        &templates_, &components_);
    parseSystemLayer();
    parseXcsXcuLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$OOO_BASE_DIR/program/unorc:UNO_USER_PACKAGES_CACHE}/"
                    "registry/com.sun.star.comp.deployment.configuration."
                    "PackageRegistryBackend/registry"))),
        &templates_, &components_);
    parseXcsXcuLayer(
        expand(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "${$BRAND_BASE_DIR/program/bootstraprc:UserInstallation}/"
                    "user/registry"))),
        &templates_, &components_);
    parseModificationLayer();
}

Components::~Components() {}

rtl::OUString Components::getModificationFileUrl() const {
    return expand(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "${$BRAND_BASE_DIR/program/bootstraprc:UserInstallation}/user/"
                "registrymodifications")));
}

void Components::parseModificationLayer() {
    rtl::OUString url(getModificationFileUrl());
    osl::DirectoryItem di;
    switch (osl::DirectoryItem::get(url, di)) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        return;
    default:
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cannot stat ")) + url,
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlDoc doc(parseXmlFile(url)); //TODO: atomic check for existence
    xmlNodePtr root(xmlDocGetRootElement(doc.doc));
    if (root == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("no root element in ")) +
             fromXmlString(doc.doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (!xmlStrEqual(root->name, xmlString("modifications")) || root->ns != 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "non modifications root element in ")) +
             fromXmlString(doc.doc->URL)),
            css::uno::Reference< css::uno::XInterface >());
    }
    for (xmlNodePtr p(skipBlank(root->xmlChildrenNode)); p != 0;
         p = skipBlank(p->next))
    {
        if (xmlStrEqual(p->name, xmlString("item")) && p->ns == 0) {
            XmlString path(xmlGetNoNsProp(p, xmlString("path")));
            if (path.str == 0) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "missing path attribute in ")) +
                     fromXmlString(doc.doc->URL)),
                    css::uno::Reference< css::uno::XInterface >());
            }
            rtl::OString unescPath;
            if (unescapeText(path.str, false, &unescPath) == 0) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "badly escaped path attribute in ")) +
                     fromXmlString(doc.doc->URL)),
                    css::uno::Reference< css::uno::XInterface >());
            }
            rtl::OUString componentName;
            rtl::OUString canonicalPath;
            rtl::Reference< Node > node(
                resolvePath(
                    rtl::OStringToOUString(unescPath, RTL_TEXTENCODING_UTF8),
                    &componentName, 0, &canonicalPath));
            if (!node.is()) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "nonexisting path attribute in ")) +
                     fromXmlString(doc.doc->URL)),
                    css::uno::Reference< css::uno::XInterface >());
            }
            parseXcuNode(
                componentName, templates_, doc.doc, p, node, this,
                (canonicalPath +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"))));
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad content in ")) +
                 fromXmlString(doc.doc->URL)),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
}

}
