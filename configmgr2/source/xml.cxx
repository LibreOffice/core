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

#include <algorithm>
#include <stack>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "comphelper/sequenceasvector.hxx"
#include "libxml/parser.h"
#include "libxml/xmlreader.h"
#include "libxml/xmlschemastypes.h"
#include "libxml/xmlstring.h"
#include "libxml/xmlwriter.h"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/thread.hxx"
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

#include "data.hxx"
#include "groupnode.hxx"
#include "layer.hxx"
#include "localizedpropertynode.hxx"
#include "localizedpropertyvaluenode.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "propertynode.hxx"
#include "setnode.hxx"
#include "type.hxx"
#include "xml.hxx"

namespace configmgr {

namespace xml {

namespace {

namespace css = com::sun::star;

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
                    buf.append(char(static_cast< unsigned char >(0xEF)));
                    buf.append(char(static_cast< unsigned char >(0xBF)));
                    buf.append(
                        static_cast< char >(
                            0xBE +
                            (text[3] == 'F' || text[3] == 'f' ? 1 : 0)));
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

bool equal(
    xmlChar const * begin1, xmlChar const * end1, char const * begin2,
    sal_Int32 length2)
{
    OSL_ASSERT(begin1 != 0 && end1 != 0);
    return
        rtl_str_compare_WithLength(
            reinterpret_cast< char const * >(begin1), end1 - begin1, begin2,
            length2) ==
        0;
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
    OSL_ASSERT(str == 0 || end != 0);
    return str == 0
        ? rtl::OUString()
        : rtl::OUString(
            reinterpret_cast< char const * >(str), end - str,
            RTL_TEXTENCODING_UTF8);
}

struct XmlString: private boost::noncopyable {
    xmlChar * str;

    explicit XmlString(xmlChar * theStr = 0): str(theStr) {}

    ~XmlString() { xmlFree(str); }

    void clear();

    XmlString & operator =(xmlChar * theStr);
};

void XmlString::clear() {
    xmlFree(str);
    str = 0;
}

XmlString & XmlString::operator =(xmlChar * theStr) {
    OSL_ASSERT(str == 0);
    str = theStr;
    return *this;
}

rtl::OUString fullTemplateName(
    rtl::OUString const & component, rtl::OUString const & name)
{
    OSL_ASSERT(component.indexOf(':') == -1);
    rtl::OUStringBuffer buf(component);
    buf.append(sal_Unicode(':'));
    buf.append(name);
    return buf.makeStringAndClear();
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

enum Operation {
    OPERATION_MODIFY, OPERATION_REPLACE, OPERATION_FUSE, OPERATION_REMOVE };

Operation parseOperation(xmlChar const * text) {
    if (text == 0 || xmlStrEqual(text, xmlString("modify"))) {
        return OPERATION_MODIFY;
    } else if (xmlStrEqual(text, xmlString("replace"))) {
        return OPERATION_REPLACE;
    } else if (xmlStrEqual(text, xmlString("fuse"))) {
        return OPERATION_FUSE;
    } else if (xmlStrEqual(text, xmlString("remove"))) {
        return OPERATION_REMOVE;
    } else {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid op ")) +
             fromXmlString(text)),
            css::uno::Reference< css::uno::XInterface >());
    }
}

Type parseType(Reader const * reader, xmlChar const * text) {
    if (text == 0) {
        return TYPE_ERROR;
    }
    xmlChar const * p = xmlStrchr(text, ':');
    if (p != 0) {
        XmlString uri(
            xmlTextReaderLookupNamespace(
                reader->getReader(),
                reinterpret_cast< xmlChar const * >(
                    rtl::OString(
                        reinterpret_cast< char const * >(text), p - text).
                    getStr())));
        if (uri.str != 0) {
            if (xmlStrEqual(
                    uri.str, xmlString("http://www.w3.org/2001/XMLSchema")))
            {
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
                           uri.str,
                           xmlString("http://openoffice.org/2001/registry")))
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
        }
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid type ")) +
         fromXmlString(text)),
        css::uno::Reference< css::uno::XInterface >());
}

bool parseBoolean(xmlChar const * text, bool deflt) {
    if (text == 0) {
        return deflt;
    }
    if (xmlStrEqual(text, xmlString("true"))) {
        return true;
    }
    if (xmlStrEqual(text, xmlString("false"))) {
        return false;
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid boolean ")) +
         fromXmlString(text)),
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
        if (equal(text, end, RTL_CONSTASCII_STRINGPARAM("true")) ||
            equal(text, end, RTL_CONSTASCII_STRINGPARAM("1")))
        {
            *value = true;
            return true;
        }
        if (equal(text, end, RTL_CONSTASCII_STRINGPARAM("false")) ||
            equal(text, end, RTL_CONSTASCII_STRINGPARAM("0")))
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

template< typename T > char const * parseEscapedValue(
    char const * text, bool (* parse)(xmlChar const *, xmlChar const *, T *),
    T * value)
{
    rtl::OString unesc;
    text = reinterpret_cast< char const * >(
        unescapeText(reinterpret_cast< xmlChar const * >(text), true, &unesc));
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
    xmlChar const * separator, bool escaped, rtl::OStringBuffer const & text,
    bool (* parse)(xmlChar const *, xmlChar const *, T *),
    css::uno::Sequence< T > * value)
{
    OSL_ASSERT(parse != 0 && value != 0);
    comphelper::SequenceAsVector< T > seq;
    if (escaped) {
        if (text.getLength() != 0) {
            for (char const * p = text.getStr();;) {
                T val;
                p = parseEscapedValue(p, parse, &val);
                if (p == 0) {
                    return false;
                }
                seq.push_back(val);
                if (*p == 0) {
                    break;
                }
                if (separator == 0) {
                    char const * q = p;
                    while (*q == ' ' || *q == '\t' || *q == '\x0A' ||
                           *q == '\x0D')
                    {
                        ++q;
                    }
                    if (q == p) {
                        return false;
                    }
                    p = q;
                } else {
                    int sepLen = xmlStrlen(separator);
                    if (xmlStrncmp(
                            reinterpret_cast< xmlChar const * >(p), separator,
                            sepLen) !=
                        0)
                    {
                        return false;
                    }
                    p += sepLen;
                }
            }
        }
    } else {
        XmlString col;
        xmlChar const * p = reinterpret_cast< xmlChar const * >(text.getStr());
        xmlChar const * sep;
        int sepLen;
        if (separator == 0) {
            col = xmlSchemaCollapseString(p);
            if (col.str != 0) {
                p = col.str;
            }
            sep = xmlString(" ");
            sepLen = RTL_CONSTASCII_LENGTH(" ");
        } else {
            sep = separator;
            sepLen = xmlStrlen(separator);
        }
        if (*p != '\0') {
            for (;;) {
                sal_Int32 i = rtl_str_indexOfStr_WithLength(
                    reinterpret_cast< char const * >(p), xmlStrlen(p),
                    reinterpret_cast< char const * >(sep), sepLen);
                T val;
                if (!(*parse)(p, i == -1 ? 0 : p + i, &val)) {
                    return false;
                }
                seq.push_back(val);
                if (i == -1) {
                    break;
                }
                p += i + sepLen;
            }
        }
    }
    *value = seq.getAsConstList();
    return true;
}

css::uno::Any parseValue(
    xmlChar const * separator, bool escaped, rtl::OStringBuffer const & text,
    Type type)
{
    switch (type) {
    case TYPE_ANY:
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid value of type any")),
            css::uno::Reference< css::uno::XInterface >());
    case TYPE_BOOLEAN:
        {
            XmlString col(
                xmlSchemaCollapseString(
                    reinterpret_cast< xmlChar const * >(text.getStr())));
            sal_Bool val;
            if (parseBooleanValue(
                    (col.str == 0
                     ? reinterpret_cast< xmlChar const * >(text.getStr())
                     : col.str),
                    0, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_SHORT:
        {
            XmlString col(
                xmlSchemaCollapseString(
                    reinterpret_cast< xmlChar const * >(text.getStr())));
            sal_Int16 val;
            if (parseShortValue(
                    (col.str == 0
                     ? reinterpret_cast< xmlChar const * >(text.getStr())
                     : col.str),
                    0, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_INT:
        {
            XmlString col(
                xmlSchemaCollapseString(
                    reinterpret_cast< xmlChar const * >(text.getStr())));
            sal_Int32 val;
            if (parseIntValue(
                    (col.str == 0
                     ? reinterpret_cast< xmlChar const * >(text.getStr())
                     : col.str),
                    0, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_LONG:
        {
            XmlString col(
                xmlSchemaCollapseString(
                    reinterpret_cast< xmlChar const * >(text.getStr())));
            sal_Int64 val;
            if (parseLongValue(
                    (col.str == 0
                     ? reinterpret_cast< xmlChar const * >(text.getStr())
                     : col.str),
                    0, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_DOUBLE:
        {
            XmlString col(
                xmlSchemaCollapseString(
                    reinterpret_cast< xmlChar const * >(text.getStr())));
            double val;
            if (parseDoubleValue(
                    (col.str == 0
                     ? reinterpret_cast< xmlChar const * >(text.getStr())
                     : col.str),
                    0, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_STRING:
        if (escaped) {
            rtl::OUString val;
            char const * p = parseEscapedValue(
                text.getStr(), &parseStringValue, &val);
            if (p != 0 && *p == '\0') {
                return css::uno::makeAny(val);
            }
        } else {
            rtl::OUString val;
            if (parseStringValue(
                    reinterpret_cast< xmlChar const * >(text.getStr()), 0,
                    &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_HEXBINARY:
        {
            XmlString col(
                xmlSchemaCollapseString(
                    reinterpret_cast< xmlChar const * >(text.getStr())));
            css::uno::Sequence< sal_Int8 > val;
            if (parseHexbinaryValue(
                    (col.str == 0
                     ? reinterpret_cast< xmlChar const * >(text.getStr())
                     : col.str),
                    0, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_BOOLEAN_LIST:
        {
            css::uno::Sequence< sal_Bool > val;
            if (parseListValue(
                    separator, escaped, text, &parseBooleanValue, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_SHORT_LIST:
        {
            css::uno::Sequence< sal_Int16 > val;
            if (parseListValue(
                    separator, escaped, text, &parseShortValue, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_INT_LIST:
        {
            css::uno::Sequence< sal_Int32 > val;
            if (parseListValue(separator, escaped, text, &parseIntValue, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_LONG_LIST:
        {
            css::uno::Sequence< sal_Int64 > val;
            if (parseListValue(separator, escaped, text, &parseLongValue, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_DOUBLE_LIST:
        {
            css::uno::Sequence< double > val;
            if (parseListValue(
                    separator, escaped, text, &parseDoubleValue, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_STRING_LIST:
        {
            css::uno::Sequence< rtl::OUString > val;
            if (parseListValue(
                    separator, escaped, text, &parseStringValue, &val))
            {
                return css::uno::makeAny(val);
            }
        }
        break;
    case TYPE_HEXBINARY_LIST:
        {
            css::uno::Sequence< css::uno::Sequence< sal_Int8 > > val;
            if (parseListValue(
                    separator, escaped, text, &parseHexbinaryValue, &val))
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
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid value")),
        css::uno::Reference< css::uno::XInterface >());
}

rtl::OUString parseTemplateReference(
    xmlChar const * component, xmlChar const * nodeType,
    rtl::OUString const & componentName,
    rtl::OUString const * defaultTemplateName)
{
    if (nodeType == 0) {
        if (defaultTemplateName != 0) {
            return *defaultTemplateName;
        }
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("missing node-type attribute")),
            css::uno::Reference< css::uno::XInterface >());
    }
    return fullTemplateName(
        component == 0 ? componentName : fromXmlString(component),
        fromXmlString(nodeType));
}

struct XmlTextWriter: private boost::noncopyable {
    xmlTextWriterPtr writer;

    explicit XmlTextWriter(xmlTextWriterPtr theWriter): writer(theWriter) {}

    ~XmlTextWriter() { xmlFreeTextWriter(writer); }
};

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

void writeValue(xmlTextWriterPtr writer, Type type, css::uno::Any const & value)
{
    switch (type) {
    case TYPE_NIL:
        xmlTextWriterWriteAttribute(
            writer, xmlString("xsi:nil"), xmlString("true"));
        break;
    case TYPE_BOOLEAN:
        {
            bool val = bool();
            value >>= val;
            writeBooleanValue(writer, val);
        }
        break;
    case TYPE_SHORT:
        {
            sal_Int16 val;
            value >>= val;
            writeShortValue(writer, val);
        }
        break;
    case TYPE_INT:
        {
            sal_Int32 val;
            value >>= val;
            writeIntValue(writer, val);
        }
        break;
    case TYPE_LONG:
        {
            sal_Int64 val;
            value >>= val;
            writeLongValue(writer, val);
        }
        break;
    case TYPE_DOUBLE:
        {
            double val;
            value >>= val;
            writeDoubleValue(writer, val);
        }
        break;
    case TYPE_STRING:
        {
            xmlTextWriterWriteAttribute(
                writer, xmlString("oor:escaped"), xmlString("true"));
            rtl::OUString val;
            value >>= val;
            writeStringValue(writer, val);
        }
        break;
    case TYPE_HEXBINARY:
        {
            // Written in escaped form to be able to share writeHexbinaryValue
            // with the TYPE_HEXBINARY_LIST case (see there):
            xmlTextWriterWriteAttribute(
                writer, xmlString("oor:escaped"), xmlString("true"));
            css::uno::Sequence< sal_Int8 > val;
            value >>= val;
            writeHexbinaryValue(writer, val);
        }
        break;
    case TYPE_BOOLEAN_LIST:
        writeListValue(writer, &writeBooleanValue, value);
        break;
    case TYPE_SHORT_LIST:
        writeListValue(writer, &writeShortValue, value);
        break;
    case TYPE_INT_LIST:
        writeListValue(writer, &writeIntValue, value);
        break;
    case TYPE_LONG_LIST:
        writeListValue(writer, &writeLongValue, value);
        break;
    case TYPE_DOUBLE_LIST:
        writeListValue(writer, &writeDoubleValue, value);
        break;
    case TYPE_STRING_LIST:
        xmlTextWriterWriteAttribute(
            writer, xmlString("oor:escaped"), xmlString("true"));
        writeListValue(writer, &writeStringValue, value);
        break;
    case TYPE_HEXBINARY_LIST:
        // Written in escaped form to distinguish an empty list from a list with
        // one empty hexbinary element:
        xmlTextWriterWriteAttribute(
            writer, xmlString("oor:escaped"), xmlString("true"));
        writeListValue(writer, &writeHexbinaryValue, value);
        break;
    default: // TYPE_ERROR, TYPE_ANY
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void writeNode(
    xmlTextWriterPtr writer, rtl::Reference< Node > const & parent,
    rtl::OUString const & name, rtl::Reference< Node > const & node,
    bool topLevel)
{
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
    if (PropertyNode * prop = dynamic_cast< PropertyNode * >(node.get())) {
        xmlTextWriterStartElement(writer, xmlString("prop"));
        xmlTextWriterWriteAttribute(
            writer, xmlString("oor:name"),
            xmlString(convertToUtf8(name).getStr()));
        xmlTextWriterWriteAttribute(
            writer, xmlString("oor:op"), xmlString("fuse"));
        Type type = prop->getType();
        if (type == TYPE_ANY) {
            type = mapType(prop->getValue());
            if (type != TYPE_ERROR) { //TODO
                xmlTextWriterWriteAttribute(
                    writer, xmlString("oor:type"), xmlString(typeNames[type]));
            }
        }
        xmlTextWriterStartElement(writer, xmlString("value"));
        writeValue(writer, type, prop->getValue());
        xmlTextWriterEndElement(writer);
        xmlTextWriterEndElement(writer);
    } else if (LocalizedPropertyNode * locprop =
               dynamic_cast< LocalizedPropertyNode * >(node.get()))
    {
        xmlTextWriterStartElement(writer, xmlString("prop"));
        xmlTextWriterWriteAttribute(
            writer, xmlString("oor:name"),
            xmlString(convertToUtf8(name).getStr()));
        xmlTextWriterWriteAttribute(
            writer, xmlString("oor:op"), xmlString("fuse"));
        for (NodeMap::iterator i(locprop->getMembers().begin());
             i != locprop->getMembers().end(); ++i)
        {
            if (!i->second->isRemoved()) {
                writeNode(
                    writer, node, i->first, i->second,
                    topLevel && locprop->getLayer() != NO_LAYER);
            }
        }
        xmlTextWriterEndElement(writer);
    } else if (LocalizedPropertyValueNode * locval =
               dynamic_cast< LocalizedPropertyValueNode * >(node.get()))
    {
        if (locval->isRemoved()
            ? topLevel && locval->getLayer() == NO_LAYER
            : !topLevel || locval->getLayer() == NO_LAYER)
        {
            xmlTextWriterStartElement(writer, xmlString("value"));
            if (name.getLength() != 0) {
                xmlTextWriterWriteAttribute(
                    writer, xmlString("xml:lang"),
                    xmlString(convertToUtf8(name).getStr()));
            }
            if (locval->isRemoved()) {
                xmlTextWriterWriteAttribute(
                    writer, xmlString("oor:op"), xmlString("remove"));
            } else {
                Type type =
                    dynamic_cast< LocalizedPropertyNode * >(parent.get())->
                    getType();
                if (type == TYPE_ANY) {
                    type = mapType(locval->getValue());
                    if (type != TYPE_ERROR) { // TODO
                        xmlTextWriterWriteAttribute(
                            writer, xmlString("oor:type"),
                            xmlString(typeNames[type]));
                    }
                }
                writeValue(writer, type, locval->getValue());
            }
            xmlTextWriterEndElement(writer);
        }
    } else if (GroupNode * group = dynamic_cast< GroupNode * >(node.get())) {
        xmlTextWriterStartElement(writer, xmlString("node"));
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
            if (!i->second->isRemoved()) {
                writeNode(writer, node, i->first, i->second, false);
            }
        }
        xmlTextWriterEndElement(writer);
    } else if (SetNode * set = dynamic_cast< SetNode * >(node.get())) {
        xmlTextWriterStartElement(writer, xmlString("node"));
        xmlTextWriterWriteAttribute(
            writer, xmlString("oor:name"),
            xmlString(convertToUtf8(name).getStr()));
        if (set->getTemplateName().getLength() != 0) { // set member
            xmlTextWriterWriteAttribute(
                writer, xmlString("oor:op"), xmlString("replace"));
        }
        for (NodeMap::iterator i(set->getMembers().begin());
             i != set->getMembers().end(); ++i)
        {
            if (!i->second->isRemoved()) {
                writeNode(writer, node, i->first, i->second, false);
            }
        }
        xmlTextWriterEndElement(writer);
    } else {
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            css::uno::Reference< css::uno::XInterface >());
    }
}

class XcsParser: public Parser {
public:
    XcsParser(int layer, Data * data):
        layer_(layer), data_(data), state_(STATE_START) {}

private:
    virtual ~XcsParser() {}

    virtual bool startElement(
        Reader const * reader, xmlChar const * name, xmlChar const * nsUri);

    virtual void endElement(Reader const * reader);

    virtual void characters(Reader const * reader);

    void handleComponentSchema(Reader const * reader);

    void handleNodeRef(Reader const * reader);

    void handleProp(Reader const * reader);

    void handlePropValue(
        Reader const * reader, rtl::Reference< Node > const & property);

    void handleGroup(Reader const * reader, bool isTemplate);

    void handleSet(Reader const * reader, bool isTemplate);

    void handleSetItem(Reader const * reader, SetNode * set);

    enum State {
        STATE_START, STATE_COMPONENT_SCHEMA, STATE_TEMPLATES,
        STATE_TEMPLATES_DONE, STATE_COMPONENT, STATE_COMPONENT_DONE };

    struct Element {
        rtl::Reference< Node > node;
        rtl::OUString name;

        Element(
            rtl::Reference< Node > const & theNode,
            rtl::OUString const & theName):
            node(theNode), name(theName) {}
    };

    typedef std::stack< Element > ElementStack;

    int layer_;
    Data * data_;
    rtl::OUString componentName_;
    State state_;
    long ignoring_;
    ElementStack elements_;
    rtl::Reference< Node > valueNode_;
    XmlString valueSeparator_;
    bool valueEscaped_;
    Type valueType_;
    rtl::OStringBuffer value_;
};

bool XcsParser::startElement(
    Reader const * reader, xmlChar const * name, xmlChar const * nsUri)
{
    if (state_ == STATE_START) {
        if (xmlStrEqual(name, xmlString("component-schema")) && nsUri != 0 &&
            xmlStrEqual(
                nsUri, xmlString("http://openoffice.org/2001/registry")))
        {
            handleComponentSchema(reader);
            state_ = STATE_COMPONENT_SCHEMA;
            ignoring_ = 0;
            return true;
        }
    } else {
        //TODO: ignoring component-schema import, component-schema uses, and
        // prop constraints; accepting all four at illegal places (and with
        // illegal content):
        if (ignoring_ > 0 ||
            ((xmlStrEqual(name, xmlString("info")) ||
              xmlStrEqual(name, xmlString("import")) ||
              xmlStrEqual(name, xmlString("uses")) ||
              xmlStrEqual(name, xmlString("constraints"))) &&
             nsUri == 0))
        {
            OSL_ASSERT(ignoring_ < LONG_MAX);
            ++ignoring_;
            return true;
        }
        switch (state_) {
        case STATE_COMPONENT_SCHEMA:
            if (xmlStrEqual(name, xmlString("templates")) && nsUri == 0) {
                state_ = STATE_TEMPLATES;
                return true;
            }
            // fall through
        case STATE_TEMPLATES_DONE:
            if (xmlStrEqual(name, xmlString("component")) && nsUri == 0) {
                state_ = STATE_COMPONENT;
                OSL_ASSERT(elements_.empty());
                elements_.push(
                    Element(
                        new GroupNode(layer_, false, rtl::OUString()),
                        componentName_));
                return true;
            }
            break;
        case STATE_TEMPLATES:
            if (elements_.empty()) {
                if (xmlStrEqual(name, xmlString("group")) && nsUri == 0) {
                    handleGroup(reader, true);
                    return true;
                }
                if (xmlStrEqual(name, xmlString("set")) && nsUri == 0) {
                    handleSet(reader, true);
                    return true;
                }
                break;
            }
            // fall through
        case STATE_COMPONENT:
            OSL_ASSERT(!elements_.empty());
            if ((dynamic_cast< PropertyNode * >(elements_.top().node.get())
                 != 0) ||
                (dynamic_cast< LocalizedPropertyNode * >(
                    elements_.top().node.get())
                 != 0))
            {
                if (xmlStrEqual(name, xmlString("value")) && nsUri == 0) {
                    handlePropValue(reader, elements_.top().node);
                    return true;
                }
            } else if (dynamic_cast< GroupNode * >(elements_.top().node.get())
                       != 0)
            {
                if (xmlStrEqual(name, xmlString("prop")) && nsUri == 0) {
                    handleProp(reader);
                    return true;
                }
                if (xmlStrEqual(name, xmlString("node-ref")) && nsUri == 0) {
                    handleNodeRef(reader);
                    return true;
                }
                if (xmlStrEqual(name, xmlString("group")) && nsUri == 0) {
                    handleGroup(reader, false);
                    return true;
                }
                if (xmlStrEqual(name, xmlString("set")) && nsUri == 0) {
                    handleSet(reader, false);
                    return true;
                }
            } else if (SetNode * set = dynamic_cast< SetNode * >(
                           elements_.top().node.get()))
            {
                if (xmlStrEqual(name, xmlString("item")) && nsUri == 0) {
                    handleSetItem(reader, set);
                    return true;
                }
            }
            break;
        case STATE_COMPONENT_DONE:
            break;
        default: // STATE_START
            OSL_ASSERT(false); // this cannot happen
            break;
        }
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad member <")) +
         fromXmlString(name) +
         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
         reader->getUrl()),
        css::uno::Reference< css::uno::XInterface >());
}

void XcsParser::endElement(Reader const * reader) {
    if (ignoring_ > 0) {
        --ignoring_;
    } else if (valueNode_.is()) {
        css::uno::Any val(
            parseValue(valueSeparator_.str, valueEscaped_, value_, valueType_));
        if (PropertyNode * prop = dynamic_cast< PropertyNode * >(
                valueNode_.get()))
        {
            prop->setValue(layer_, val);
        } else if (LocalizedPropertyNode * locprop =
                   dynamic_cast< LocalizedPropertyNode * >(valueNode_.get()))
        {
            locprop->getMembers()[rtl::OUString()] =
                new LocalizedPropertyValueNode(layer_, val);
        } else {
            OSL_ASSERT(false); // this cannot happen
        }
        valueNode_.clear();
        valueSeparator_.clear();
        value_.setLength(0);
    } else if (!elements_.empty()) {
        rtl::Reference< Node > node(elements_.top().node);
        rtl::OUString name(elements_.top().name);
        elements_.pop();
        NodeMap * map;
        if (elements_.empty()) {
            switch (state_) {
            case STATE_TEMPLATES:
                map = &data_->templates;
                break;
            case STATE_COMPONENT:
                map = &data_->components;
                state_ = STATE_COMPONENT_DONE;
                break;
            default:
                OSL_ASSERT(false);
                throw css::uno::RuntimeException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
                    css::uno::Reference< css::uno::XInterface >());
            }
        } else if (GroupNode * group = dynamic_cast< GroupNode * >(
                       elements_.top().node.get()))
        {
            map = &group->getMembers();
        } else {
            OSL_ASSERT(false);
            throw css::uno::RuntimeException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
                css::uno::Reference< css::uno::XInterface >());
        }
        if (!map->insert(NodeMap::value_type(name, node)).second) {
            throw css::uno::RuntimeException(
                (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("duplicate ")) +
                 name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else {
        switch (state_) {
        case STATE_COMPONENT_SCHEMA:
        case STATE_TEMPLATES_DONE:
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("no component element in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        case STATE_TEMPLATES:
            state_ = STATE_TEMPLATES_DONE;
            break;
        case STATE_COMPONENT_DONE:
            break;
        default:
            OSL_ASSERT(false); // this cannot happen
        }
    }
}

void XcsParser::characters(Reader const * reader) {
    if (valueNode_.is()) {
        value_.append(
            reinterpret_cast< char const * >(
                xmlTextReaderConstValue(reader->getReader())));
    }
}

void XcsParser::handleComponentSchema(Reader const * reader) {
    //TODO: oor:version, xml:lang attributes
    XmlString attrPackage(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("package"),
            xmlString("http://openoffice.org/2001/registry")));
    if (attrPackage.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-schema package attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString attrName(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("name"),
            xmlString("http://openoffice.org/2001/registry")));
    if (attrName.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-schema name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OStringBuffer buf;
    buf.append(reinterpret_cast< char const * >(attrPackage.str));
    buf.append('.');
    buf.append(reinterpret_cast< char const * >(attrName.str));
    componentName_ = rtl::OUString(
        buf.getStr(), buf.getLength(), RTL_TEXTENCODING_UTF8);
}

void XcsParser::handleNodeRef(Reader const * reader) {
    XmlString attrName(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("name"),
            xmlString("http://openoffice.org/2001/registry")));
    if (attrName.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no node-ref name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString attrComponent(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("component"),
            xmlString("http://openoffice.org/2001/registry")));
    XmlString attrNodeType(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("node-type"),
            xmlString("http://openoffice.org/2001/registry")));
    rtl::Reference< Node > tmpl(
        data_->getTemplate(
            layer_,
            parseTemplateReference(
                attrComponent.str, attrNodeType.str, componentName_, 0)));
    if (!tmpl.is()) {
        //TODO: this can erroneously happen as long as import/uses attributes
        // are not correctly processed
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("unknown node-ref ")) +
             fromXmlString(attrName.str) +
             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::Reference< Node > node(tmpl->clone());
    node->setLayer(layer_);
    elements_.push(Element(node, fromXmlString(attrName.str)));
}

void XcsParser::handleProp(Reader const * reader) {
    XmlString attrName(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("name"),
            xmlString("http://openoffice.org/2001/registry")));
    if (attrName.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no prop name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString attrType(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("type"),
            xmlString("http://openoffice.org/2001/registry")));
    valueType_ = parseType(reader, attrType.str);
    if (valueType_ == TYPE_ERROR) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no prop type attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString attrLocalized(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("localized"),
            xmlString("http://openoffice.org/2001/registry")));
    bool localized = parseBoolean(attrLocalized.str, false);
    XmlString attrNillable(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("nillable"),
            xmlString("http://openoffice.org/2001/registry")));
    bool nillable = parseBoolean(attrNillable.str, true);
    elements_.push(
        Element(
            (localized
             ? rtl::Reference< Node >(
                 new LocalizedPropertyNode(layer_, valueType_, nillable))
             : rtl::Reference< Node >(
                 new PropertyNode(
                     layer_, valueType_, nillable, css::uno::Any(), false))),
            fromXmlString(attrName.str)));
}

void XcsParser::handlePropValue(
    Reader const * reader, rtl::Reference< Node > const & property)
{
    valueNode_ = property;
    valueSeparator_ = xmlTextReaderGetAttributeNs(
        reader->getReader(), xmlString("separator"),
        xmlString("http://openoffice.org/2001/registry"));
    XmlString attrEscaped(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("escaped"),
            xmlString("http://openoffice.org/2001/registry")));
    valueEscaped_ = parseBoolean(attrEscaped.str, false);
}

void XcsParser::handleGroup(Reader const * reader, bool isTemplate) {
    XmlString attrName(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("name"),
            xmlString("http://openoffice.org/2001/registry")));
    if (attrName.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no group name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(fromXmlString(attrName.str));
    if (isTemplate) {
        name = fullTemplateName(componentName_, name);
    }
    XmlString attrExtensible(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("extensible"),
            xmlString("http://openoffice.org/2001/registry")));
    elements_.push(
        Element(
            new GroupNode(
                layer_, parseBoolean(attrExtensible.str, false),
                isTemplate ? name : rtl::OUString()),
            name));
}

void XcsParser::handleSet(Reader const * reader, bool isTemplate) {
    XmlString attrName(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("name"),
            xmlString("http://openoffice.org/2001/registry")));
    if (attrName.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no set name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(fromXmlString(attrName.str));
    if (isTemplate) {
        name = fullTemplateName(componentName_, name);
    }
    XmlString attrComponent(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("component"),
            xmlString("http://openoffice.org/2001/registry")));
    XmlString attrNodeType(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("node-type"),
            xmlString("http://openoffice.org/2001/registry")));
    elements_.push(
        Element(
            new SetNode(
                layer_,
                parseTemplateReference(
                    attrComponent.str, attrNodeType.str, componentName_, 0),
                isTemplate ? name : rtl::OUString()),
            name));
}

void XcsParser::handleSetItem(Reader const * reader, SetNode * set) {
    XmlString attrComponent(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("component"),
            xmlString("http://openoffice.org/2001/registry")));
    XmlString attrNodeType(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("node-type"),
            xmlString("http://openoffice.org/2001/registry")));
    set->getAdditionalTemplateNames().push_back(
        parseTemplateReference(
            attrComponent.str, attrNodeType.str, componentName_, 0));
}

class XcuParser: public Parser {
public:
    XcuParser(int layer, Data * data): layer_(layer), data_(data) {}

private:
    virtual ~XcuParser() {}

    virtual bool startElement(
        Reader const * reader, xmlChar const * name, xmlChar const * nsUri);

    virtual void endElement(Reader const * reader);

    virtual void characters(Reader const * reader);

    void handleComponentData(Reader const * reader);

    void handleItem(Reader const * reader);

    void handlePropValue(Reader const * reader, PropertyNode * prop);

    void handleLocpropValue(
        Reader const * reader, LocalizedPropertyNode * locprop);

    void handleGroupProp(Reader const * reader, GroupNode * group);

    void handleUnknownGroupProp(
        Reader const * reader, GroupNode * group, rtl::OUString const & name,
        Type type, Operation operation, bool finalized);

    void handlePlainGroupProp(
        Reader const * reader, PropertyNode * property,
        rtl::OUString const & name, Type type, Operation operation,
        bool finalized);

    void handleLocalizedGroupProp(
        Reader const * reader, LocalizedPropertyNode * property,
        rtl::OUString const & name, Type type, Operation operation,
        bool finalized);

    void handleGroupNode(Reader const * reader, GroupNode * group);

    void handleSetNode(Reader const * reader, SetNode * set);

    struct State {
        rtl::Reference< Node > node; // empty iff ignore or <items>
        rtl::OUString name; // empty and ignored if !insert
        bool ignore;
        bool insert;
        bool locked;
        bool record;

        State(): ignore(true), insert(false), locked(false), record(false) {}

        State(
            rtl::Reference< Node > const & theNode, bool theLocked,
            bool theRecord):
            node(theNode), ignore(false), insert(false), locked(theLocked),
            record(theRecord) {}

        State(
            rtl::Reference< Node > const & theNode,
            rtl::OUString const & theName, bool theLocked, bool theRecord):
            node(theNode), name(theName), ignore(false), insert(true),
            locked(theLocked), record(theRecord) {}
    };

    typedef std::stack< State > StateStack;

    int layer_;
    Data * data_;
    rtl::OUString componentName_;
    StateStack state_;
    rtl::Reference< Node > valueNode_;
    XmlString valueSeparator_;
    bool valueEscaped_;
    Type valueType_;
    rtl::OStringBuffer value_;
    rtl::OUString pathPrefix_;
    rtl::OUString path_;
};

bool XcuParser::startElement(
    Reader const * reader, xmlChar const * name, xmlChar const * nsUri)
{
#if OSL_DEBUG_LEVEL > 0
    StateStack::size_type oldStateSize = state_.size();
#endif
    if (state_.empty()) {
        if (xmlStrEqual(name, xmlString("component-data")) && nsUri != 0 &&
            xmlStrEqual(
                nsUri, xmlString("http://openoffice.org/2001/registry")))
        {
            handleComponentData(reader);
        } else if (xmlStrEqual(name, xmlString("items")) &&
                   nsUri != 0 &&
                   xmlStrEqual(
                       nsUri, xmlString("http://openoffice.org/2001/registry")))
        {
            state_.push(State(rtl::Reference< Node >(), false, false));
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad root element <")) +
                 fromXmlString(name) +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else if (state_.top().ignore) {
        state_.push(state_.top());
    } else if (!state_.top().node.is()) {
        if (xmlStrEqual(name, xmlString("item")) && nsUri == 0) {
            handleItem(reader);
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad items node member <")) +
                 fromXmlString(name) +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else if (PropertyNode * prop = dynamic_cast< PropertyNode * >(
                   state_.top().node.get()))
    {
        if (xmlStrEqual(name, xmlString("value")) && nsUri == 0) {
            handlePropValue(reader, prop);
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad property node member <")) +
                 fromXmlString(name) +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else if (LocalizedPropertyNode * locprop =
               dynamic_cast< LocalizedPropertyNode * >(state_.top().node.get()))
    {
        if (xmlStrEqual(name, xmlString("value")) && nsUri == 0) {
            handleLocpropValue(reader, locprop);
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "bad localized property node member <")) +
                 fromXmlString(name) +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else if (GroupNode * group = dynamic_cast< GroupNode * >(
                   state_.top().node.get()))
    {
        if (xmlStrEqual(name, xmlString("prop")) && nsUri == 0) {
            handleGroupProp(reader, group);
        } else if (xmlStrEqual(name, xmlString("node")) && nsUri == 0) {
            handleGroupNode(reader, group);
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad group node member <")) +
                 fromXmlString(name) +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else if (SetNode * set = dynamic_cast< SetNode * >(
                   state_.top().node.get()))
    {
        if (xmlStrEqual(name, xmlString("node")) && nsUri == 0) {
            handleSetNode(reader, set);
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad set node member <")) +
                 fromXmlString(name) +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad member <")) +
             fromXmlString(name) +
             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    OSL_ASSERT(!state_.empty() && state_.size() - 1 == oldStateSize);
    return true;
}

void XcuParser::endElement(Reader const *) {
    OSL_ASSERT(!state_.empty());
    if (valueNode_.is()) {
        css::uno::Any val(
            parseValue(valueSeparator_.str, valueEscaped_, value_, valueType_));
        if (PropertyNode * prop = dynamic_cast< PropertyNode * >(
                valueNode_.get()))
        {
            prop->setValue(layer_, val);
        } else if (LocalizedPropertyValueNode * locval =
                   dynamic_cast< LocalizedPropertyValueNode * >(
                       valueNode_.get()))
        {
            locval->setValue(layer_, val);
        } else {
            OSL_ASSERT(false); // this cannot happen
        }
        valueNode_.clear();
        valueSeparator_.clear();
        value_.setLength(0);
    }
    rtl::Reference< Node > insert;
    rtl::OUString name;
    if (state_.top().insert) {
        insert = state_.top().node;
        OSL_ASSERT(insert.is());
        name = state_.top().name;
    }
    state_.pop();
    if (insert.is()) {
        OSL_ASSERT(!state_.empty() && state_.top().node.is());
        if (LocalizedPropertyNode * locprop =
            dynamic_cast< LocalizedPropertyNode * >(state_.top().node.get()))
        {
            locprop->getMembers()[name] = insert;
        } else if (GroupNode * group =
                   dynamic_cast< GroupNode * >(state_.top().node.get()))
        {
            group->getMembers()[name] = insert;
        } else if (SetNode * set =
                   dynamic_cast< SetNode * >(state_.top().node.get()))
        {
            set->getMembers()[name] = insert;
        } else {
            OSL_ASSERT(false); // this cannot happen
        }
    }
}

void XcuParser::characters(Reader const * reader) {
    if (valueNode_.is()) {
        value_.append(
            reinterpret_cast< char const * >(
                xmlTextReaderConstValue(reader->getReader())));
    }
}

void XcuParser::handleComponentData(Reader const * reader) {
    XmlString attrPackage(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("package"),
            xmlString("http://openoffice.org/2001/registry")));
    if (attrPackage.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-data package attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString attrName(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("name"),
            xmlString("http://openoffice.org/2001/registry")));
    if (attrName.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-data name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OStringBuffer buf;
    buf.append(reinterpret_cast< char const * >(attrPackage.str));
    buf.append('.');
    buf.append(reinterpret_cast< char const * >(attrName.str));
    componentName_ = rtl::OUString(
        buf.getStr(), buf.getLength(), RTL_TEXTENCODING_UTF8);
    rtl::Reference< Node > node(
        Data::findNode(layer_, data_->components, componentName_));
    if (!node.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unknown component ")) +
             componentName_ +
             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString attrOp(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("op"),
            xmlString("http://openoffice.org/2001/registry")));
    switch (parseOperation(attrOp.str)) {
    case OPERATION_MODIFY:
    case OPERATION_FUSE:
        break;
    default:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "invalid operation on root node in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString attrFinalized(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("finalized"),
            xmlString("http://openoffice.org/2001/registry")));
    int finalizedLayer = std::min(
        (parseBoolean(attrFinalized.str, false) ? layer_ : NO_LAYER),
        node->getFinalized());
    node->setFinalized(finalizedLayer);
    state_.push(State(node, finalizedLayer < layer_, false));
}

void XcuParser::handleItem(Reader const * reader) {
    XmlString attrPath(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("path"),
            xmlString("http://openoffice.org/2001/registry")));
    if (attrPath.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("missing path attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OString unescPath;
    if (unescapeText(attrPath.str, false, &unescPath) == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "badly escaped path attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    int finalizedLayer;
    rtl::Reference< Node > node(
        data_->resolvePath(
            rtl::OStringToOUString(unescPath, RTL_TEXTENCODING_UTF8),
            &componentName_, 0, &pathPrefix_, 0, &finalizedLayer));
    pathPrefix_ += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    if (!node.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("nonexisting path attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    state_.push(State(node, finalizedLayer < layer_, layer_ == NO_LAYER));
}

void XcuParser::handlePropValue(Reader const * reader, PropertyNode * prop) {
    OSL_ASSERT(!state_.top().record);
    XmlString attrNil(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("nil"),
            xmlString("http://www.w3.org/2001/XMLSchema-instance")));
    if (parseBoolean(attrNil.str, false)) {
        if (!prop->isNillable()) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xsi:nil attribute for non-nillable prop in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
        prop->setValue(layer_, css::uno::Any());
    } else {
        valueNode_ = prop;
        valueSeparator_ = xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("separator"),
            xmlString("http://openoffice.org/2001/registry"));
        XmlString attrEscaped(
            xmlTextReaderGetAttributeNs(
                reader->getReader(), xmlString("escaped"),
                xmlString("http://openoffice.org/2001/registry")));
        valueEscaped_ = parseBoolean(attrEscaped.str, false);
    }
    state_.push(State());
}

void XcuParser::handleLocpropValue(
    Reader const * reader, LocalizedPropertyNode * locprop)
{
    OSL_ASSERT(!state_.top().record);
    rtl::OUString name(
        fromXmlString(xmlTextReaderConstXmlLang(reader->getReader())));
    NodeMap::iterator i(locprop->getMembers().find(name));
    if (i != locprop->getMembers().end() && i->second->getLayer() > layer_) {
        state_.push(State()); // ignored
        return;
    }
    XmlString attrNil(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("nil"),
            xmlString("http://www.w3.org/2001/XMLSchema-instance")));
    bool nil = parseBoolean(attrNil.str, false);
    if (nil && !locprop->isNillable()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xsi:nil attribute for non-nillable prop in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString attrOp(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("op"),
            xmlString("http://openoffice.org/2001/registry")));
    switch (parseOperation(attrOp.str)) {
    case OPERATION_MODIFY:
        if (nil) {
            if (i == locprop->getMembers().end()) {
                locprop->getMembers()[name] = new LocalizedPropertyValueNode(
                    layer_, css::uno::Any());
            } else {
                dynamic_cast< LocalizedPropertyValueNode * >(i->second.get())->
                    setValue(layer_, css::uno::Any());
            }
            state_.push(State());
        } else {
            valueSeparator_ = xmlTextReaderGetAttributeNs(
                reader->getReader(), xmlString("separator"),
                xmlString("http://openoffice.org/2001/registry"));
            XmlString attrEscaped(
                xmlTextReaderGetAttributeNs(
                    reader->getReader(), xmlString("escaped"),
                    xmlString("http://openoffice.org/2001/registry")));
            valueEscaped_ = parseBoolean(attrEscaped.str, false);
            if (i == locprop->getMembers().end()) {
                valueNode_ = new LocalizedPropertyValueNode(
                    layer_, css::uno::Any());
                state_.push(State(valueNode_, name, false, false));
            } else {
                valueNode_ = i->second;
                state_.push(State());
            }
        }
        break;
    case OPERATION_REMOVE:
        //TODO: only allow if parent.op == OPERATION_FUSE
        //TODO: disallow removing when e.g. lang=""?
        if (i != locprop->getMembers().end()) {
            i->second->remove(layer_);
        }
        state_.push(State());
        break;
    default:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "bad op attribute for value element in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void XcuParser::handleGroupProp(Reader const * reader, GroupNode * group) {
    XmlString attrName(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("name"),
            xmlString("http://openoffice.org/2001/registry")));
    if (attrName.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no prop name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(fromXmlString(attrName.str));
    if (state_.top().record) {
        data_->addModification(pathPrefix_ + name);
    }
    XmlString attrType(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("type"),
            xmlString("http://openoffice.org/2001/registry")));
    Type type(parseType(reader, attrType.str));
    if (type == TYPE_ANY) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "invalid prop type attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString attrOp(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("op"),
            xmlString("http://openoffice.org/2001/registry")));
    Operation op(parseOperation(attrOp.str));
    XmlString attrFinalized(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("finalized"),
            xmlString("http://openoffice.org/2001/registry")));
    bool finalized = parseBoolean(attrFinalized.str, false);
    NodeMap::iterator i(group->getMembers().find(name));
    if (i == group->getMembers().end()) {
        handleUnknownGroupProp(reader, group, name, type, op, finalized);
    } else if (PropertyNode * prop = dynamic_cast< PropertyNode * >(
                   i->second.get()))
    {
        handlePlainGroupProp(reader, prop, name, type, op, finalized);
    } else if (LocalizedPropertyNode * locprop =
               dynamic_cast< LocalizedPropertyNode * >(i->second.get()))
    {
        handleLocalizedGroupProp(reader, locprop, name, type, op, finalized);
    } else {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("inappropriate prop ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void XcuParser::handleUnknownGroupProp(
    Reader const * reader, GroupNode * group, rtl::OUString const & name,
    Type type, Operation operation, bool finalized)
{
    if (!group->isExtensible()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unknown prop ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    switch (operation) {
    case OPERATION_MODIFY:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "invalid modify of extension prop ")) +
             name +
             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    case OPERATION_REPLACE:
    case OPERATION_FUSE:
        {
            if (type == TYPE_ERROR) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "missing type attribute for prop ")) +
                 name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
            }
            valueType_ = type;
            rtl::Reference< Node > prop(
                new PropertyNode(
                    layer_, TYPE_ANY, true, css::uno::Any(), true));
            if (finalized) {
                prop->setFinalized(layer_);
            }
            state_.push(State(prop, name, state_.top().locked, false));
        }
        break;
    case OPERATION_REMOVE:
        // ignore unknown (presumably extension) properties
        state_.push(State());
        break;
    }
}

void XcuParser::handlePlainGroupProp(
    Reader const * reader, PropertyNode * property, rtl::OUString const & name,
    Type type, Operation operation, bool finalized)
{
    if (property->getLayer() > layer_) {
        state_.push(State()); // ignored
        return;
    }
    int finalizedLayer = std::min(
        finalized ? layer_ : NO_LAYER, property->getFinalized());
    property->setFinalized(finalizedLayer);
    if (type != TYPE_ERROR && property->getType() != TYPE_ANY &&
        type != property->getType())
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid type for prop ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    valueType_ = type == TYPE_ERROR ? property->getType() : type;
    switch (operation) {
    case OPERATION_MODIFY:
    case OPERATION_REPLACE:
    case OPERATION_FUSE:
        state_.push(
            State(
                property, state_.top().locked || finalizedLayer < layer_,
                false));
        break;
    case OPERATION_REMOVE:
        if (!property->isExtension()) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "invalid remove of non-extension prop ")) +
                 name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
        property->remove(layer_);
        state_.push(State()); // ignore children
        break;
    }
}

void XcuParser::handleLocalizedGroupProp(
    Reader const * reader, LocalizedPropertyNode * property,
    rtl::OUString const & name, Type type, Operation operation, bool finalized)
{
    if (property->getLayer() > layer_) {
        state_.push(State()); // ignored
        return;
    }
    int finalizedLayer = std::min(
        finalized ? layer_ : NO_LAYER, property->getFinalized());
    property->setFinalized(finalizedLayer);
    if (type != TYPE_ERROR && property->getType() != TYPE_ANY &&
        type != property->getType())
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid type for prop ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    valueType_ = type == TYPE_ERROR ? property->getType() : type;
    switch (operation) {
    case OPERATION_MODIFY:
    case OPERATION_FUSE:
        state_.push(
            State(
                property, state_.top().locked || finalizedLayer < layer_,
                false));
        break;
    case OPERATION_REPLACE:
        {
            rtl::Reference< Node > replacement(
                new LocalizedPropertyNode(
                    layer_, property->getType(), property->isNillable()));
            replacement->setFinalized(property->getFinalized());
            state_.push(
                State(
                    replacement, name,
                    state_.top().locked || finalizedLayer < layer_, false));
        }
        break;
    case OPERATION_REMOVE:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "invalid remove of non-extension prop ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void XcuParser::handleGroupNode(Reader const * reader, GroupNode * group) {
    OSL_ASSERT(!state_.top().record);
    XmlString attrName(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("name"),
            xmlString("http://openoffice.org/2001/registry")));
    if (attrName.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no node name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(fromXmlString(attrName.str));
    rtl::Reference< Node > subgroup(
        Data::findNode(layer_, group->getMembers(), name));
    if (!subgroup.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unknown node ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString attrOp(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("op"),
            xmlString("http://openoffice.org/2001/registry")));
    Operation op(parseOperation(attrOp.str));
    if ((op != OPERATION_MODIFY && op != OPERATION_FUSE) || state_.top().record)
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "invalid operation on group node in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString attrFinalized(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("finalized"),
            xmlString("http://openoffice.org/2001/registry")));
    int finalizedLayer = std::min(
        parseBoolean(attrFinalized.str, false) ? layer_ : NO_LAYER,
        subgroup->getFinalized());
    subgroup->setFinalized(finalizedLayer);
    state_.push(
        State(subgroup, state_.top().locked || finalizedLayer < layer_, false));
}

void XcuParser::handleSetNode(Reader const * reader, SetNode * set) {
    XmlString attrName(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("name"),
            xmlString("http://openoffice.org/2001/registry")));
    if (attrName.str == 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no node name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(fromXmlString(attrName.str));
    XmlString attrComponent(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("component"),
            xmlString("http://openoffice.org/2001/registry")));
    XmlString attrNodeType(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("node-type"),
            xmlString("http://openoffice.org/2001/registry")));
    rtl::OUString templateName(
        parseTemplateReference(
            attrComponent.str, attrNodeType.str, componentName_,
            &set->getDefaultTemplateName()));
    if (state_.top().record) {
        data_->addModification(
            pathPrefix_ + Data::createSegment(templateName, name));
    }
    if (!set->isValidTemplate(templateName)) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("set member node ")) +
             name +
             rtl::OUString(
                 RTL_CONSTASCII_USTRINGPARAM(" references invalid template ")) +
             templateName + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::Reference< Node > tmpl(data_->getTemplate(layer_, templateName));
    if (!tmpl.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("set member node ")) +
             name +
             rtl::OUString(
                 RTL_CONSTASCII_USTRINGPARAM(
                     " references undefined template ")) +
             templateName + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    XmlString attrOp(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("op"),
            xmlString("http://openoffice.org/2001/registry")));
    Operation op(parseOperation(attrOp.str));
    XmlString attrFinalized(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("finalized"),
            xmlString("http://openoffice.org/2001/registry")));
    int finalizedLayer = parseBoolean(attrFinalized.str, false)
        ? layer_ : NO_LAYER;
    XmlString attrMandatory(
        xmlTextReaderGetAttributeNs(
            reader->getReader(), xmlString("mandatory"),
            xmlString("http://openoffice.org/2001/registry")));
    int mandatoryLayer = parseBoolean(attrMandatory.str, false)
        ? layer_ : NO_LAYER;
    NodeMap::iterator i(set->getMembers().find(name));
    if (i != set->getMembers().end()) {
        finalizedLayer = std::min(finalizedLayer, i->second->getFinalized());
        i->second->setFinalized(finalizedLayer);
        mandatoryLayer = std::min(mandatoryLayer, i->second->getMandatory());
        i->second->setMandatory(mandatoryLayer);
        if (i->second->getLayer() > layer_) {
            state_.push(State()); // ignored
            return;
        }
    }
    switch (op) {
    case OPERATION_MODIFY:
        if (i == set->getMembers().end() || i->second->isRemoved()) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "invalid modify of unknown set member node ")) +
                 name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
        state_.push(
            State(
                i->second, state_.top().locked || finalizedLayer < layer_,
                false));
        break;
    case OPERATION_REPLACE:
        if (state_.top().locked || finalizedLayer < layer_) {
            state_.push(State()); // ignored
        } else {
            rtl::Reference< Node > member(tmpl->clone());
            member->setLayer(layer_);
            member->setFinalized(finalizedLayer);
            member->setMandatory(mandatoryLayer);
            state_.push(State(member, name, false, false));
        }
        break;
    case OPERATION_FUSE:
        if (i == set->getMembers().end() || i->second->isRemoved()) {
            if (state_.top().locked || finalizedLayer < layer_) {
                state_.push(State()); // ignored
            } else {
                rtl::Reference< Node > member(tmpl->clone());
                member->setLayer(layer_);
                member->setFinalized(finalizedLayer);
                member->setMandatory(mandatoryLayer);
                state_.push(State(member, name, false, false));
            }
        } else {
            state_.push(
                State(
                    i->second, state_.top().locked || finalizedLayer < layer_,
                    false));
        }
        break;
    case OPERATION_REMOVE:
        // Ignore removal of unknown members, members finalized in a lower
        // layer, and members made mandatory in this or a lower layer:
        if (i != set->getMembers().end() && !state_.top().locked &&
            finalizedLayer >= layer_ && mandatoryLayer > layer_)
        {
            i->second->remove(layer_);
        }
        state_.push(State());
        break;
    }
}

}

Reader::Reader(
    rtl::OUString const & url, rtl::Reference< Parser > const & parser):
    url_(url), parser_(parser), reader_(0)
{
    OSL_ASSERT(parser.is());
}

bool Reader::parse() {
    bool skip = false;
    if (reader_ == 0) {
        reader_ = xmlReaderForFile(
            convertToFilepath(url_), 0, XML_PARSE_NONET | XML_PARSE_NOCDATA);
            //TODO: pass (external) file URL instead of filepath?
        if (reader_ == 0) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xmlReaderForFile failed for ")) +
                 url_),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else {
        skip = true;
    }
    for (;; skip = false) {
        if (!skip) {
            switch (xmlTextReaderRead(reader_)) {
            case -1:
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "xmlTextReaderRead failed for ")) +
                     url_),
                    css::uno::Reference< css::uno::XInterface >());
            case 0:
                return true;
            }
        }
        switch (xmlTextReaderNodeType(reader_)) {
        case -1:
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xmlTextReaderNodeType failed for ")) +
                 url_),
                css::uno::Reference< css::uno::XInterface >());
        case XML_READER_TYPE_ELEMENT:
            if (!parser_->startElement(
                    this, xmlTextReaderConstLocalName(reader_),
                    xmlTextReaderConstNamespaceUri(reader_)))
            {
                return false;
            }
            if (!xmlTextReaderIsEmptyElement(reader_)) {
                break;
            }
            // fall through:
        case XML_READER_TYPE_END_ELEMENT:
            parser_->endElement(this);
            break;
        case XML_READER_TYPE_TEXT:
        case XML_READER_TYPE_SIGNIFICANT_WHITESPACE:
            {
                parser_->characters(this);
            }
            break;
        }
    }
}

rtl::OUString Reader::getUrl() const {
    return url_;
}

xmlTextReaderPtr Reader::getReader() const {
    return reader_;
}

Reader::~Reader() {
    xmlFreeTextReader(reader_);
}

Parser::Parser() {}

Parser::~Parser() {}

XcdParser::XcdParser(
    int layer, Dependencies const & dependencies, Data * data):
    layer_(layer), dependencies_(dependencies), data_(data), state_(STATE_START)
{}

XcdParser::~XcdParser() {}

bool XcdParser::startElement(
    Reader const * reader, xmlChar const * name, xmlChar const * nsUri)
{
    if (nestedParser_.is()) {
        OSL_ASSERT(nesting_ != LONG_MAX);
        ++nesting_;
        return nestedParser_->startElement(reader, name, nsUri);
    }
    switch (state_) {
    case STATE_START:
        if (xmlStrEqual(name, xmlString("data")) && nsUri != 0 &&
            xmlStrEqual(
                nsUri, xmlString("http://openoffice.org/2001/registry")))
        {
            state_ = STATE_DEPENDENCIES;
            return true;
        }
        break;
    case STATE_DEPENDENCIES:
        if (xmlStrEqual(name, xmlString("dependency")) && nsUri == 0) {
            XmlString file(
                xmlTextReaderGetAttribute(
                    reader->getReader(), xmlString("file")));
            if (file.str == 0) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "no dependency file attribute in ")) +
                     reader->getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            if (dependencies_.find(fromXmlString(file.str)) ==
                dependencies_.end())
            {
                return false;
            }
            state_ = STATE_DEPENDENCY;
            return true;
        }
        state_ = STATE_COMPONENTS;
        // fall through
    case STATE_COMPONENTS:
        if (xmlStrEqual(name, xmlString("component-schema")) && nsUri != 0 &&
            xmlStrEqual(
                nsUri, xmlString("http://openoffice.org/2001/registry")))
        {
            nestedParser_ = new XcsParser(layer_, data_);
            nesting_ = 1;
            return nestedParser_->startElement(reader, name, nsUri);
        }
        if (xmlStrEqual(name, xmlString("component-data")) && nsUri != 0 &&
            xmlStrEqual(
                nsUri, xmlString("http://openoffice.org/2001/registry")))
        {
            nestedParser_ = new XcuParser(layer_ + 1, data_);
            nesting_ = 1;
            return nestedParser_->startElement(reader, name, nsUri);
        }
        break;
    default: // STATE_DEPENDENCY
        OSL_ASSERT(false); // this cannot happen
        break;
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad member <")) +
         fromXmlString(name) +
         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
         reader->getUrl()),
        css::uno::Reference< css::uno::XInterface >());
}

void XcdParser::endElement(Reader const * reader) {
    if (nestedParser_.is()) {
        nestedParser_->endElement(reader);
        if (--nesting_ == 0) {
            nestedParser_.clear();
        }
    } else {
        switch (state_) {
        case STATE_DEPENDENCY:
            state_ = STATE_DEPENDENCIES;
            break;
        case STATE_DEPENDENCIES:
        case STATE_COMPONENTS:
            break;
        default:
            OSL_ASSERT(false); // this cannot happen
            break;
        }
    }
}

void XcdParser::characters(Reader const * reader) {
    if (nestedParser_.is()) {
        nestedParser_->characters(reader);
    }
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

void parseXcsFile(rtl::OUString const & url, int layer, Data * data) {
    OSL_VERIFY(
        rtl::Reference< Reader >(new Reader(url, new XcsParser(layer, data)))->
        parse());
}

void parseXcuFile(rtl::OUString const & url, int layer, Data * data) {
    OSL_VERIFY(
        rtl::Reference< Reader >(new Reader(url, new XcuParser(layer, data)))->
        parse());
}

void parseModFile(rtl::OUString const & url, Data * data) {
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
    parseXcuFile(url, NO_LAYER, data); //TODO: atomic check for existence
}

void writeModFile(rtl::OUString const & url, Data const & data) {
    XmlTextWriter writer(
        xmlNewTextWriterFilename(convertToFilepath(url).getStr(), 0));
    if (writer.writer == 0) {
        //TODO: As long as there is no UserInstallation directory yet,
        // xmlNewTextWriterFilename will legitimately fail (but also print noise
        // to stderr):
        return;
    }
    xmlTextWriterSetIndent(writer.writer, 1);
        //TODO: more readable, but potentially slower?
    xmlTextWriterStartDocument(writer.writer, 0, 0, 0);
    xmlTextWriterStartElement(writer.writer, xmlString("oor:items"));
    xmlTextWriterWriteAttribute(
        writer.writer, xmlString("xmlns:oor"),
        xmlString("http://openoffice.org/2001/registry"));
    xmlTextWriterWriteAttribute(
        writer.writer, xmlString("xmlns:xs"),
        xmlString("http://www.w3.org/2001/XMLSchema"));
    xmlTextWriterWriteAttribute(
        writer.writer, xmlString("xmlns:xsi"),
        xmlString("http://www.w3.org/2001/XMLSchema-instance"));
    //TODO: Do not write back information about those removed items that did not
    // come from the .xcs/.xcu files, anyway (but had been added dynamically
    // instead):
    for (Data::Modifications::const_iterator i(data.modifications.begin());
         i != data.modifications.end(); ++i)
    {
        rtl::OUString name;
        rtl::OUString parentPath(parseLastSegment(*i, &name));
        rtl::Reference< Node > parent;
        rtl::Reference< Node > node(data.resolvePath(*i, 0, 0, 0, &parent, 0));
        if (node.is()) {
            xmlTextWriterStartElement(writer.writer, xmlString("item"));
            xmlTextWriterWriteAttribute(
                writer.writer, xmlString("oor:path"),
                xmlString(
                    convertToUtf8(escapeText(parentPath, false)).getStr()));
            writeNode(writer.writer, parent, name, node, true);
            xmlTextWriterEndElement(writer.writer);
            // It is never necessary to write the oor:mandatory attribute, as it
            // cannot be set via the UNO API.
        } else {
            parent = data.resolvePath(parentPath, 0, 0, 0, 0, 0);
            if (LocalizedPropertyNode * locprop =
                dynamic_cast< LocalizedPropertyNode * >(parent.get()))
            {
                NodeMap::iterator j(locprop->getMembers().find(name));
                if (j != locprop->getMembers().end() &&
                    j->second->getLayer() == NO_LAYER)
                {
                    OSL_ASSERT(j->second->isRemoved());
                    rtl::OUString parentName;
                    rtl::OUString grandparentPath(
                        parseLastSegment(parentPath, &parentName));
                    xmlTextWriterStartElement(writer.writer, xmlString("item"));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:path"),
                        xmlString(
                            convertToUtf8(escapeText(grandparentPath, false)).
                            getStr()));
                    xmlTextWriterStartElement(writer.writer, xmlString("prop"));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:name"),
                        xmlString(convertToUtf8(parentName).getStr()));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:op"), xmlString("fuse"));
                    xmlTextWriterStartElement(
                        writer.writer, xmlString("value"));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("xml:lang"),
                        xmlString(convertToUtf8(name).getStr()));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:op"),
                        xmlString("remove"));
                    xmlTextWriterEndElement(writer.writer);
                    xmlTextWriterEndElement(writer.writer);
                    xmlTextWriterEndElement(writer.writer);
                }
            } else if (GroupNode * group = dynamic_cast< GroupNode * >(
                           parent.get()))
            {
                OSL_ASSERT(dynamic_cast< GroupNode * >(group)->isExtensible());
                NodeMap::iterator j(group->getMembers().find(name));
                if (j != group->getMembers().end() &&
                    j->second->getLayer() == NO_LAYER)
                {
                    OSL_ASSERT(j->second->isRemoved());
                    xmlTextWriterStartElement(writer.writer, xmlString("item"));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:path"),
                        xmlString(
                            convertToUtf8(escapeText(parentPath, false)).
                            getStr()));
                    xmlTextWriterStartElement(writer.writer, xmlString("prop"));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:name"),
                        xmlString(convertToUtf8(name).getStr()));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:op"),
                        xmlString("remove"));
                    xmlTextWriterEndElement(writer.writer);
                    xmlTextWriterEndElement(writer.writer);
                }
            } else if (SetNode * set = dynamic_cast< SetNode * >(parent.get()))
            {
                NodeMap::iterator j(set->getMembers().find(name));
                if (j != set->getMembers().end() &&
                    j->second->getLayer() == NO_LAYER)
                {
                    OSL_ASSERT(j->second->isRemoved());
                    xmlTextWriterStartElement(writer.writer, xmlString("item"));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:path"),
                        xmlString(
                            convertToUtf8(escapeText(parentPath, false)).
                            getStr()));
                    xmlTextWriterStartElement(writer.writer, xmlString("node"));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:name"),
                        xmlString(convertToUtf8(name).getStr()));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:op"),
                        xmlString("remove"));
                    xmlTextWriterEndElement(writer.writer);
                    xmlTextWriterEndElement(writer.writer);
                }
            } else {
                OSL_ASSERT(false); // this cannot happen
            }
        }
    }
    if (xmlTextWriterEndDocument(writer.writer) == -1) { //TODO: check all -1?
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cannot write ")) + url,
            css::uno::Reference< css::uno::XInterface >());
    }
}

}

}
