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
#include <climits>
#include <stack>
#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "comphelper/sequenceasvector.hxx"
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
#include "pad.hxx"
#include "propertynode.hxx"
#include "setnode.hxx"
#include "span.hxx"
#include "type.hxx"
#include "xml.hxx"
#include "xmlreader.hxx"

namespace configmgr {

namespace xml {

namespace {

namespace css = com::sun::star;

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

rtl::OUString convertUtf8String(Span const & text) {
    OSL_ASSERT(text.is());
    return rtl::OUString(text.begin, text.length, RTL_TEXTENCODING_UTF8);
        // conversion cannot fail as XML parser guarantees that text is legal
        // UTF-8
        //TODO
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

rtl::OString convertToUtf8(
    rtl::OUString const & text, sal_Int32 offset, sal_Int32 length)
{
    OSL_ASSERT(
        offset <= text.getLength() && text.getLength() - offset >= length);
    rtl::OString utf8;
    if (!rtl_convertUStringToString(
            &utf8.pData, text.pData->buffer + offset, length,
            RTL_TEXTENCODING_UTF8,
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

rtl::OString convertToUtf8(rtl::OUString const & text) {
    return convertToUtf8(text, 0, text.getLength());
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

Operation parseOperation(Span const & text) {
    if (!text.is() || text.equals(RTL_CONSTASCII_STRINGPARAM("modify"))) {
        return OPERATION_MODIFY;
    } else if (text.equals(RTL_CONSTASCII_STRINGPARAM("replace"))) {
        return OPERATION_REPLACE;
    } else if (text.equals(RTL_CONSTASCII_STRINGPARAM("fuse"))) {
        return OPERATION_FUSE;
    } else if (text.equals(RTL_CONSTASCII_STRINGPARAM("remove"))) {
        return OPERATION_REMOVE;
    } else {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid op ")) +
             convertUtf8String(text)),
            css::uno::Reference< css::uno::XInterface >());
    }
}

Type parseType(XmlReader const * reader, Span const & text) {
    if (!text.is()) {
        return TYPE_ERROR;
    }
    sal_Int32 i = rtl_str_indexOfChar_WithLength(text.begin, text.length, ':');
    if (i >= 0) {
        switch (reader->getNamespace(Span(text.begin, i))) {
        case XmlReader::NAMESPACE_OOR:
            if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                    RTL_CONSTASCII_STRINGPARAM("any")))
            {
                return TYPE_ANY;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("boolean-list")))
            {
                return TYPE_BOOLEAN_LIST;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("short-list")))
            {
                return TYPE_SHORT_LIST;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("int-list")))
            {
                return TYPE_INT_LIST;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("long-list")))
            {
                return TYPE_LONG_LIST;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("double-list")))
            {
                return TYPE_DOUBLE_LIST;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("string-list")))
            {
                return TYPE_STRING_LIST;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("hexBinary-list")))
            {
                return TYPE_HEXBINARY_LIST;
            }
            break;
        case XmlReader::NAMESPACE_XS:
            if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                    RTL_CONSTASCII_STRINGPARAM("boolean")))
            {
                return TYPE_BOOLEAN;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("short")))
            {
                return TYPE_SHORT;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("int")))
            {
                return TYPE_INT;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("long")))
            {
                return TYPE_LONG;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("double")))
            {
                return TYPE_DOUBLE;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("string")))
            {
                return TYPE_STRING;
            } else if (Span(text.begin + i + 1, text.length - (i + 1)).equals(
                           RTL_CONSTASCII_STRINGPARAM("hexBinary")))
            {
                return TYPE_HEXBINARY;
            }
            break;
        default:
            break;
        }
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid type ")) +
         convertUtf8String(text)),
        css::uno::Reference< css::uno::XInterface >());
}

bool parseBoolean(Span const & text, bool deflt) {
    if (!text.is()) {
        return deflt;
    }
    if (text.equals(RTL_CONSTASCII_STRINGPARAM("true"))) {
        return true;
    }
    if (text.equals(RTL_CONSTASCII_STRINGPARAM("false"))) {
        return false;
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid boolean ")) +
         convertUtf8String(text)),
        css::uno::Reference< css::uno::XInterface >());
}

bool parseBooleanValue(Span const & text, sal_Bool * value) {
    OSL_ASSERT(text.is() && value != 0);
    if (text.equals(RTL_CONSTASCII_STRINGPARAM("true")) ||
        text.equals(RTL_CONSTASCII_STRINGPARAM("1")))
    {
        *value = true;
        return true;
    }
    if (text.equals(RTL_CONSTASCII_STRINGPARAM("false")) ||
        text.equals(RTL_CONSTASCII_STRINGPARAM("0")))
    {
        *value = false;
        return true;
    }
    return false;
}

bool parseShortValue(Span const & text, sal_Int16 * value) {
    OSL_ASSERT(text.is() && value != 0);
    sal_Int32 n = rtl::OString(text.begin, text.length).toInt32();
        //TODO: check valid lexical representation
    if (n >= SAL_MIN_INT16 && n <= SAL_MAX_INT16) {
        *value = static_cast< sal_Int16 >(n);
        return true;
    }
    return false;
}

bool parseIntValue(Span const & text, sal_Int32 * value) {
    OSL_ASSERT(text.is() && value != 0);
    *value = rtl::OString(text.begin, text.length).toInt32();
        //TODO: check valid lexical representation
    return true;
}

bool parseLongValue(Span const & text, sal_Int64 * value) {
    OSL_ASSERT(text.is() && value != 0);
    *value = rtl::OString(text.begin, text.length).toInt64();
        //TODO: check valid lexical representation
    return true;
}

bool parseDoubleValue(Span const & text, double * value) {
    OSL_ASSERT(text.is() && value != 0);
    *value = rtl::OString(text.begin, text.length).toDouble();
        //TODO: check valid lexical representation
    return true;
}

bool parseStringValue(Span const & text, rtl::OUString * value) {
    OSL_ASSERT(text.is() && value != 0);
    *value = convertUtf8String(text);
    return true;
}

bool parseHexDigit(char c, int * value) {
    OSL_ASSERT(value != 0);
    if (c >= '0' && c <= '9') {
        *value = c - '0';
        return true;
    }
    if (c >= 'A' && c <= 'F') {
        *value = c - 'A' + 10;
        return true;
    }
    if (c >= 'a' && c <= 'f') {
        *value = c - 'a' + 10;
        return true;
    }
    return false;
}

bool parseHexbinaryValue(
    Span const & text, css::uno::Sequence< sal_Int8 > * value)
{
    OSL_ASSERT(text.is() && value != 0);
    if ((text.length & 1) != 0) {
        return false;
    }
    comphelper::SequenceAsVector< sal_Int8 > seq;
    for (sal_Int32 i = 0; i != text.length;) {
        int n1;
        int n2;
        if (!parseHexDigit(text.begin[i++], &n1) ||
            !parseHexDigit(text.begin[i++], &n2))
        {
            return false;
        }
        seq.push_back(static_cast< sal_Int8 >((n1 << 4) | n2));
    }
    *value = seq.getAsConstList();
    return true;
}

template< typename T > css::uno::Any parseValue(
    Span const & text, bool (* parse)(Span const &, T *))
{
    T val;
    if (!(*parse)(text, &val)) {
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid value")),
            css::uno::Reference< css::uno::XInterface >());
    }
    return css::uno::makeAny(val);
}

template< typename T > css::uno::Any parseListValue(
    Span const & separator, Span const & text,
    bool (* parse)(Span const &, T *))
{
    comphelper::SequenceAsVector< T > seq;
    Span sep;
    if (separator.is()) {
        sep = separator;
    } else {
        sep = Span(RTL_CONSTASCII_STRINGPARAM(" "));
    }
    if (text.length != 0) {
        for (Span t(text);;) {
            sal_Int32 i = rtl_str_indexOfStr_WithLength(
                t.begin, t.length, sep.begin, sep.length);
            T val;
            if (!(*parse)(Span(t.begin, i == -1 ? t.length : i), &val)) {
                throw css::uno::RuntimeException(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid value")),
                    css::uno::Reference< css::uno::XInterface >());
            }
            seq.push_back(val);
            if (i < 0) {
                break;
            }
            t.begin += i + sep.length;
            t.length -= i + sep.length;
        }
    }
    return css::uno::makeAny(seq.getAsConstList());
}

css::uno::Any parseValue(Span const & separator, Span const & text, Type type) {
    switch (type) {
    case TYPE_ANY:
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid value of type any")),
            css::uno::Reference< css::uno::XInterface >());
    case TYPE_BOOLEAN:
        return parseValue(text, &parseBooleanValue);
    case TYPE_SHORT:
        return parseValue(text, &parseShortValue);
    case TYPE_INT:
        return parseValue(text, &parseIntValue);
    case TYPE_LONG:
        return parseValue(text, &parseLongValue);
    case TYPE_DOUBLE:
        return parseValue(text, &parseDoubleValue);
    case TYPE_STRING:
        return parseValue(text, &parseStringValue);
    case TYPE_HEXBINARY:
        return parseValue(text, &parseHexbinaryValue);
    case TYPE_BOOLEAN_LIST:
        return parseListValue(separator, text, &parseBooleanValue);
    case TYPE_SHORT_LIST:
        return parseListValue(separator, text, &parseShortValue);
    case TYPE_INT_LIST:
        return parseListValue(separator, text, &parseIntValue);
    case TYPE_LONG_LIST:
        return parseListValue(separator, text, &parseLongValue);
    case TYPE_DOUBLE_LIST:
        return parseListValue(separator, text, &parseDoubleValue);
    case TYPE_STRING_LIST:
        return parseListValue(separator, text, &parseStringValue);
    case TYPE_HEXBINARY_LIST:
        return parseListValue(separator, text, &parseHexbinaryValue);
    default:
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            css::uno::Reference< css::uno::XInterface >());
    }
}

rtl::OUString parseTemplateReference(
    Span const & component, Span const & nodeType,
    rtl::OUString const & componentName,
    rtl::OUString const * defaultTemplateName)
{
    if (!nodeType.is()) {
        if (defaultTemplateName != 0) {
            return *defaultTemplateName;
        }
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("missing node-type attribute")),
            css::uno::Reference< css::uno::XInterface >());
    }
    return fullTemplateName(
        component.is() ? convertUtf8String(component) : componentName,
        convertUtf8String(nodeType));
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
    sal_Int32 i = 0;
    sal_Int32 j = i;
    for (; j < value.getLength(); ++j) {
        sal_Unicode c = value[j];
        if ((c < 0x0020 && c != 0x0009 && c != 0x000A && c != 0x000D) ||
            c == 0xFFFE || c == 0xFFFF)
        {
            if (j > i) {
                xmlTextWriterWriteString(
                    writer,
                    xmlString(convertToUtf8(value, i, j - i).getStr()));
            }
            xmlTextWriterStartElement(writer, xmlString("unicode"));
            xmlTextWriterWriteAttribute(
                writer, xmlString("oor:scalar"),
                xmlString(
                    convertToUtf8(
                        rtl::OUString::valueOf(static_cast< sal_Int32 >(c))).
                    getStr()));
            xmlTextWriterEndElement(writer);
            i = j + 1;
        }
    }
    if (j > i) {
        xmlTextWriterWriteString(
            writer, xmlString(convertToUtf8(value, i, j - i).getStr()));
    }
}

void writeHexbinaryValue(
    xmlTextWriterPtr writer, css::uno::Sequence< sal_Int8 > const & value)
{
    rtl::OStringBuffer buf;
    for (sal_Int32 i = 0; i < value.getLength(); ++i) {
        static char const hexDigit[16] = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
            'D', 'E', 'F' };
        buf.append(hexDigit[(value[i] >> 4) & 0xF]);
        buf.append(hexDigit[value[i] & 0xF]);
    }
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

template< typename T > void writeItemListValue(
    xmlTextWriterPtr writer, void (* write)(xmlTextWriterPtr, T const &),
    css::uno::Any const & value)
{
    css::uno::Sequence< T > val;
    value >>= val;
    for (sal_Int32 i = 0; i < val.getLength(); ++i) {
        xmlTextWriterStartElement(writer, xmlString("it"));
        (*write)(writer, val[i]);
        xmlTextWriterEndElement(writer);
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
            rtl::OUString val;
            value >>= val;
            writeStringValue(writer, val);
        }
        break;
    case TYPE_HEXBINARY:
        {
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
        writeItemListValue(writer, &writeStringValue, value);
        break;
    case TYPE_HEXBINARY_LIST:
        writeItemListValue(writer, &writeHexbinaryValue, value);
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
    switch (node->kind()) {
    case Node::KIND_PROPERTY:
        {
            PropertyNode * prop = dynamic_cast< PropertyNode * >(node.get());
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
                        writer, xmlString("oor:type"),
                        xmlString(typeNames[type]));
                }
            }
            xmlTextWriterStartElement(writer, xmlString("value"));
            writeValue(writer, type, prop->getValue());
            xmlTextWriterEndElement(writer);
            xmlTextWriterEndElement(writer);
        }
        break;
    case Node::KIND_LOCALIZED_PROPERTY:
        {
            xmlTextWriterStartElement(writer, xmlString("prop"));
            xmlTextWriterWriteAttribute(
                writer, xmlString("oor:name"),
                xmlString(convertToUtf8(name).getStr()));
            xmlTextWriterWriteAttribute(
                writer, xmlString("oor:op"), xmlString("fuse"));
            for (NodeMap::iterator i(node->getMembers().begin());
                 i != node->getMembers().end(); ++i)
            {
                if (!i->second->isRemoved()) {
                    writeNode(
                        writer, node, i->first, i->second,
                        topLevel && node->getLayer() != NO_LAYER);
                }
            }
            xmlTextWriterEndElement(writer);
        }
        break;
    case Node::KIND_LOCALIZED_VALUE:
        {
            LocalizedPropertyValueNode * locval =
                dynamic_cast< LocalizedPropertyValueNode * >(node.get());
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
                    Type type = dynamic_cast< LocalizedPropertyNode * >(
                        parent.get())->getType();
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
        }
        break;
    case Node::KIND_GROUP:
        {
            xmlTextWriterStartElement(writer, xmlString("node"));
            xmlTextWriterWriteAttribute(
                writer, xmlString("oor:name"),
                xmlString(convertToUtf8(name).getStr()));
            if (node->getTemplateName().getLength() != 0) { // set member
                xmlTextWriterWriteAttribute(
                    writer, xmlString("oor:op"), xmlString("replace"));
            }
            for (NodeMap::iterator i(node->getMembers().begin());
                 i != node->getMembers().end(); ++i)
            {
                if (!i->second->isRemoved()) {
                    writeNode(writer, node, i->first, i->second, false);
                }
            }
            xmlTextWriterEndElement(writer);
        }
        break;
    case Node::KIND_SET:
        {
            xmlTextWriterStartElement(writer, xmlString("node"));
            xmlTextWriterWriteAttribute(
                writer, xmlString("oor:name"),
                xmlString(convertToUtf8(name).getStr()));
            if (node->getTemplateName().getLength() != 0) { // set member
                xmlTextWriterWriteAttribute(
                    writer, xmlString("oor:op"), xmlString("replace"));
            }
            for (NodeMap::iterator i(node->getMembers().begin());
                 i != node->getMembers().end(); ++i)
            {
                if (!i->second->isRemoved()) {
                    writeNode(writer, node, i->first, i->second, false);
                }
            }
            xmlTextWriterEndElement(writer);
        }
        break;
    }
}

class ValueParser: private boost::noncopyable {
public:
    ValueParser(int layer): layer_(layer) {}

    ~ValueParser() {}

    XmlReader::Text getTextMode() const;

    bool startElement(
        XmlReader * reader, XmlReader::Namespace ns, Span const & name);

    bool endElement(XmlReader const * reader);

    void characters(Span const & text);

    void start(
        rtl::Reference< Node > const & property,
        rtl::OUString const & localizedName = rtl::OUString());

    int getLayer() const { return layer_; }

    Type type_;
    Span separator_;

private:
    void checkEmptyPad(XmlReader const * reader) const;

    template< typename T > css::uno::Any convertItems();

    enum State { STATE_TEXT, STATE_TEXT_UNICODE, STATE_IT, STATE_IT_UNICODE };

    int layer_;
    rtl::Reference< Node > node_;
    rtl::OUString localizedName_;
    State state_;
    Pad pad_;
    std::vector< css::uno::Any > items_;
};

XmlReader::Text ValueParser::getTextMode() const {
    if (node_.is()) {
        switch (state_) {
        case STATE_TEXT:
        case STATE_IT:
            return
                (type_ == TYPE_STRING || type_ == TYPE_STRING_LIST ||
                 separator_.is())
                ? XmlReader::TEXT_RAW : XmlReader::TEXT_NORMALIZED;
        default:
            break;
        }
    }
    return XmlReader::TEXT_NONE;
}

bool ValueParser::startElement(
    XmlReader * reader, XmlReader::Namespace ns, Span const & name)
{
    if (!node_.is()) {
        return false;
    }
    switch (state_) {
    case STATE_TEXT:
        if (ns == XmlReader::NAMESPACE_NONE &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("it")) &&
            isListType(type_) && !separator_.is())
        {
            checkEmptyPad(reader);
            state_ = STATE_IT;
            return true;
        }
        // fall through
    case STATE_IT:
        if (ns == XmlReader::NAMESPACE_NONE &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("unicode")) &&
            (type_ == TYPE_STRING || type_ == TYPE_STRING_LIST))
        {
            sal_Int32 scalar = -1;
            for (;;) {
                XmlReader::Namespace attrNs;
                Span attrLn;
                if (!reader->nextAttribute(&attrNs, &attrLn)) {
                    break;
                }
                if (attrNs == XmlReader::NAMESPACE_OOR &&
                    attrLn.equals(RTL_CONSTASCII_STRINGPARAM("scalar")))
                {
                    if (!parseIntValue(
                            reader->getAttributeValue(true), &scalar))
                    {
                        scalar = -1;
                    }
                    break;
                }
            }
            if (scalar >= 0 && scalar < 0x20 && scalar != 0x09 &&
                scalar != 0x0A && scalar != 0x0D)
            {
                char c = static_cast< char >(scalar);
                pad_.add(&c, 1);
            } else if (scalar == 0xFFFE) {
                pad_.add(RTL_CONSTASCII_STRINGPARAM("\xEF\xBF\xBE"));
            } else if (scalar == 0xFFFF) {
                pad_.add(RTL_CONSTASCII_STRINGPARAM("\xEF\xBF\xBF"));
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad unicode scalar attribute in ")) +
                     reader->getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            state_ = State(state_ + 1);
            return true;
        }
        break;
    default:
        break;
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad member <")) +
         convertUtf8String(name) +
         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
         reader->getUrl()),
        css::uno::Reference< css::uno::XInterface >());
}

bool ValueParser::endElement(XmlReader const * reader) {
    if (!node_.is()) {
        return false;
    }
    switch (state_) {
    case STATE_TEXT:
        {
            css::uno::Any value;
            if (items_.empty()) {
                value = parseValue(separator_, pad_.get(), type_);
                pad_.clear();
            } else {
                checkEmptyPad(reader);
                switch (type_) {
                case TYPE_BOOLEAN_LIST:
                    value = convertItems< sal_Bool >();
                    break;
                case TYPE_SHORT_LIST:
                    value = convertItems< sal_Int16 >();
                    break;
                case TYPE_INT_LIST:
                    value = convertItems< sal_Int32 >();
                    break;
                case TYPE_LONG_LIST:
                    value = convertItems< sal_Int64 >();
                    break;
                case TYPE_DOUBLE_LIST:
                    value = convertItems< double >();
                    break;
                case TYPE_STRING_LIST:
                    value = convertItems< rtl::OUString >();
                    break;
                case TYPE_HEXBINARY_LIST:
                    value = convertItems< css::uno::Sequence< sal_Int8 > >();
                    break;
                default:
                    OSL_ASSERT(false); // this cannot happen
                    break;
                }
                items_.clear();
            }
            switch (node_->kind()) {
            case Node::KIND_PROPERTY:
                dynamic_cast< PropertyNode * >(node_.get())->setValue(
                    layer_, value);
                break;
            case Node::KIND_LOCALIZED_PROPERTY:
                {
                    NodeMap::iterator i(
                        node_->getMembers().find(localizedName_));
                    if (i == node_->getMembers().end()) {
                        node_->getMembers().insert(
                            NodeMap::value_type(
                                localizedName_,
                                new LocalizedPropertyValueNode(layer_, value)));
                    } else {
                        dynamic_cast< LocalizedPropertyValueNode * >(
                            i->second.get())->setValue(layer_, value);
                    }
                }
                break;
            default:
                OSL_ASSERT(false); // this cannot happen
            }
            separator_.clear();
            node_.clear();
        }
        break;
    case STATE_TEXT_UNICODE:
    case STATE_IT_UNICODE:
        state_ = State(state_ - 1);
        break;
    case STATE_IT:
        items_.push_back(parseValue(Span(), pad_.get(), elementType(type_)));
        pad_.clear();
        state_ = STATE_TEXT;
        break;
    }
    return true;
}

void ValueParser::characters(Span const & text) {
    if (node_.is()) {
        OSL_ASSERT(state_ == STATE_TEXT || state_ == STATE_IT);
        pad_.add(text.begin, text.length);
    }
}

void ValueParser::start(
    rtl::Reference< Node > const & node, rtl::OUString const & localizedName)
{
    OSL_ASSERT(node.is() && !node_.is());
    node_ = node;
    localizedName_ = localizedName;
    state_ = STATE_TEXT;
}

void ValueParser::checkEmptyPad(XmlReader const * reader) const {
    if (pad_.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "mixed text and <it> elements in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
}

template< typename T > css::uno::Any ValueParser::convertItems() {
    css::uno::Sequence< T > seq(items_.size());
    for (sal_Int32 i = 0; i < seq.getLength(); ++i) {
        OSL_VERIFY(items_[i] >>= seq[i]);
    }
    return css::uno::makeAny(seq);
}

class XcsParser: public Parser {
public:
    XcsParser(int layer, Data * data):
        valueParser_(layer), data_(data), state_(STATE_START) {}

private:
    virtual ~XcsParser() {}

    virtual XmlReader::Text getTextMode();

    virtual bool startElement(
        XmlReader * reader, XmlReader::Namespace ns, Span const & name);

    virtual void endElement(XmlReader const * reader);

    virtual void characters(Span const & text);

    void handleComponentSchema(XmlReader * reader);

    void handleNodeRef(XmlReader * reader);

    void handleProp(XmlReader * reader);

    void handlePropValue(
        XmlReader * reader, rtl::Reference< Node > const & property);

    void handleGroup(XmlReader * reader, bool isTemplate);

    void handleSet(XmlReader * reader, bool isTemplate);

    void handleSetItem(XmlReader * reader, SetNode * set);

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

    ValueParser valueParser_;
    Data * data_;
    rtl::OUString componentName_;
    State state_;
    long ignoring_;
    ElementStack elements_;
};

XmlReader::Text XcsParser::getTextMode() {
    return valueParser_.getTextMode();
}

bool XcsParser::startElement(
    XmlReader * reader, XmlReader::Namespace ns, Span const & name)
{
    if (valueParser_.startElement(reader, ns, name)) {
        return true;
    }
    if (state_ == STATE_START) {
        if (ns == XmlReader::NAMESPACE_OOR &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("component-schema"))) {
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
            (ns == XmlReader::NAMESPACE_NONE &&
             (name.equals(RTL_CONSTASCII_STRINGPARAM("info")) ||
              name.equals(RTL_CONSTASCII_STRINGPARAM("import")) ||
              name.equals(RTL_CONSTASCII_STRINGPARAM("uses")) ||
              name.equals(RTL_CONSTASCII_STRINGPARAM("constraints")))))
        {
            OSL_ASSERT(ignoring_ < LONG_MAX);
            ++ignoring_;
            return true;
        }
        switch (state_) {
        case STATE_COMPONENT_SCHEMA:
            if (ns == XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("templates")))
            {
                state_ = STATE_TEMPLATES;
                return true;
            }
            // fall through
        case STATE_TEMPLATES_DONE:
            if (ns == XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("component")))
            {
                state_ = STATE_COMPONENT;
                OSL_ASSERT(elements_.empty());
                elements_.push(
                    Element(
                        new GroupNode(
                            valueParser_.getLayer(), false, rtl::OUString()),
                        componentName_));
                return true;
            }
            break;
        case STATE_TEMPLATES:
            if (elements_.empty()) {
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("group")))
                {
                    handleGroup(reader, true);
                    return true;
                }
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("set")))
                {
                    handleSet(reader, true);
                    return true;
                }
                break;
            }
            // fall through
        case STATE_COMPONENT:
            OSL_ASSERT(!elements_.empty());
            switch (elements_.top().node->kind()) {
            case Node::KIND_PROPERTY:
            case Node::KIND_LOCALIZED_PROPERTY:
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("value")))
                {
                    handlePropValue(reader, elements_.top().node);
                    return true;
                }
                break;
            case Node::KIND_GROUP:
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("prop")))
                {
                    handleProp(reader);
                    return true;
                }
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("node-ref")))
                {
                    handleNodeRef(reader);
                    return true;
                }
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("group")))
                {
                    handleGroup(reader, false);
                    return true;
                }
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("set")))
                {
                    handleSet(reader, false);
                    return true;
                }
                break;
            case Node::KIND_SET:
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("item")))
                {
                    handleSetItem(
                        reader,
                        dynamic_cast< SetNode * >(elements_.top().node.get()));
                    return true;
                }
                break;
            default: // Node::KIND_LOCALIZED_VALUE
                OSL_ASSERT(false); // this cannot happen
                break;
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
         convertUtf8String(name) +
         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
         reader->getUrl()),
        css::uno::Reference< css::uno::XInterface >());
}

void XcsParser::endElement(XmlReader const * reader) {
    if (valueParser_.endElement(reader)) {
        return;
    }
    if (ignoring_ > 0) {
        --ignoring_;
    } else if (!elements_.empty()) {
        Element top(elements_.top());
        elements_.pop();
        if (top.node.is()) {
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
            } else {
                map = &elements_.top().node->getMembers();
            }
            if (!map->insert(NodeMap::value_type(top.name, top.node)).second) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("duplicate ")) +
                     top.name +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
                     reader->getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
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

void XcsParser::characters(Span const & text) {
    valueParser_.characters(text);
}

void XcsParser::handleComponentSchema(XmlReader * reader) {
    //TODO: oor:version, xml:lang attributes
    Span attrPackage;
    Span attrName;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("package")))
        {
            attrPackage = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader->getAttributeValue(false);
        }
    }
    if (!attrPackage.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-schema package attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-schema name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OStringBuffer buf;
    buf.append(attrPackage.begin, attrPackage.length);
    buf.append('.');
    buf.append(attrName.begin, attrName.length);
    componentName_ = convertUtf8String(Span(buf.getStr(), buf.getLength()));
}

void XcsParser::handleNodeRef(XmlReader * reader) {
    Span attrName;
    Span attrComponent;
    Span attrNodeType;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("component")))
        {
            attrComponent = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("node-type")))
        {
            attrNodeType = reader->getAttributeValue(false);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no node-ref name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::Reference< Node > tmpl(
        data_->getTemplate(
            valueParser_.getLayer(),
            parseTemplateReference(
                attrComponent, attrNodeType, componentName_, 0)));
    if (!tmpl.is()) {
        //TODO: this can erroneously happen as long as import/uses attributes
        // are not correctly processed
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("unknown node-ref ")) +
             convertUtf8String(attrName) +
             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::Reference< Node > node(tmpl->clone());
    node->setLayer(valueParser_.getLayer());
    elements_.push(Element(node, convertUtf8String(attrName)));
}

void XcsParser::handleProp(XmlReader * reader) {
    Span attrName;
    Span attrType;
    Span attrLocalized;
    Span attrNillable;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("type")))
        {
            attrType = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("localized")))
        {
            attrLocalized = reader->getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("nillable")))
        {
            attrNillable = reader->getAttributeValue(true);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no prop name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    valueParser_.type_ = parseType(reader, attrType);
    if (valueParser_.type_ == TYPE_ERROR) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no prop type attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    bool localized = parseBoolean(attrLocalized, false);
    bool nillable = parseBoolean(attrNillable, true);
    elements_.push(
        Element(
            (localized
             ? rtl::Reference< Node >(
                 new LocalizedPropertyNode(
                     valueParser_.getLayer(), valueParser_.type_, nillable))
             : rtl::Reference< Node >(
                 new PropertyNode(
                     valueParser_.getLayer(), valueParser_.type_, nillable,
                     css::uno::Any(), false))),
            convertUtf8String(attrName)));
}

void XcsParser::handlePropValue(
    XmlReader * reader, rtl::Reference< Node > const & property)
{
    Span attrSeparator;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("separator")))
        {
            attrSeparator = reader->getAttributeValue(false);
        }
    }
    valueParser_.separator_ = attrSeparator;
    valueParser_.start(property);
}

void XcsParser::handleGroup(XmlReader * reader, bool isTemplate) {
    Span attrName;
    Span attrExtensible;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("extensible")))
        {
            attrExtensible = reader->getAttributeValue(true);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no group name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(convertUtf8String(attrName));
    if (isTemplate) {
        name = fullTemplateName(componentName_, name);
    }
    elements_.push(
        Element(
            new GroupNode(
                valueParser_.getLayer(), parseBoolean(attrExtensible, false),
                isTemplate ? name : rtl::OUString()),
            name));
}

void XcsParser::handleSet(XmlReader * reader, bool isTemplate) {
    Span attrName;
    Span attrComponent;
    Span attrNodeType;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("component")))
        {
            attrComponent = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("node-type")))
        {
            attrNodeType = reader->getAttributeValue(false);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no set name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(convertUtf8String(attrName));
    if (isTemplate) {
        name = fullTemplateName(componentName_, name);
    }
    elements_.push(
        Element(
            new SetNode(
                valueParser_.getLayer(),
                parseTemplateReference(
                    attrComponent, attrNodeType, componentName_, 0),
                isTemplate ? name : rtl::OUString()),
            name));
}

void XcsParser::handleSetItem(XmlReader * reader, SetNode * set) {
    Span attrComponent;
    Span attrNodeType;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("component")))
        {
            attrComponent = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("node-type")))
        {
            attrNodeType = reader->getAttributeValue(false);
        }
    }
    set->getAdditionalTemplateNames().push_back(
        parseTemplateReference(attrComponent, attrNodeType, componentName_, 0));
    elements_.push(Element(rtl::Reference< Node >(), rtl::OUString()));
}

class XcuParser: public Parser {
public:
    XcuParser(int layer, Data * data): valueParser_(layer), data_(data) {}

private:
    virtual ~XcuParser() {}

    virtual XmlReader::Text getTextMode();

    virtual bool startElement(
        XmlReader * reader, XmlReader::Namespace ns, Span const & name);

    virtual void endElement(XmlReader const * reader);

    virtual void characters(Span const & span);

    void handleComponentData(XmlReader * reader);

    void handleItem(XmlReader * reader);

    void handlePropValue(XmlReader * reader, PropertyNode * prop);

    void handleLocpropValue(
        XmlReader * reader, LocalizedPropertyNode * locprop);

    void handleGroupProp(XmlReader * reader, GroupNode * group);

    void handleUnknownGroupProp(
        XmlReader const * reader, GroupNode * group, rtl::OUString const & name,
        Type type, Operation operation, bool finalized);

    void handlePlainGroupProp(
        XmlReader const * reader, PropertyNode * property,
        rtl::OUString const & name, Type type, Operation operation,
        bool finalized);

    void handleLocalizedGroupProp(
        XmlReader const * reader, LocalizedPropertyNode * property,
        rtl::OUString const & name, Type type, Operation operation,
        bool finalized);

    void handleGroupNode(
        XmlReader * reader, rtl::Reference< Node > const & group);

    void handleSetNode(XmlReader * reader, SetNode * set);

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

    ValueParser valueParser_;
    Data * data_;
    rtl::OUString componentName_;
    StateStack state_;
    rtl::OUString pathPrefix_;
    rtl::OUString path_;
};

XmlReader::Text XcuParser::getTextMode() {
    return valueParser_.getTextMode();
}

bool XcuParser::startElement(
    XmlReader * reader, XmlReader::Namespace ns, Span const & name)
{
    if (valueParser_.startElement(reader, ns, name)) {
        return true;
    }
    if (state_.empty()) {
        if (ns == XmlReader::NAMESPACE_OOR &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("component-data")))
        {
            handleComponentData(reader);
        } else if (ns == XmlReader::NAMESPACE_OOR &&
                   name.equals(RTL_CONSTASCII_STRINGPARAM("items")))
        {
            state_.push(State(rtl::Reference< Node >(), false, false));
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad root element <")) +
                 convertUtf8String(name) +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else if (state_.top().ignore) {
        state_.push(state_.top());
    } else if (!state_.top().node.is()) {
        if (ns == XmlReader::NAMESPACE_NONE &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("item")))
        {
            handleItem(reader);
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad items node member <")) +
                 convertUtf8String(name) +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else {
        switch (state_.top().node->kind()) {
        case Node::KIND_PROPERTY:
            if (ns == XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("value")))
            {
                handlePropValue(
                    reader,
                    dynamic_cast< PropertyNode * >(state_.top().node.get()));
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad property node member <")) +
                     convertUtf8String(name) +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                     reader->getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case Node::KIND_LOCALIZED_PROPERTY:
            if (ns == XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("value")))
            {
                handleLocpropValue(
                    reader,
                    dynamic_cast< LocalizedPropertyNode * >(
                        state_.top().node.get()));
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad localized property node member <")) +
                     convertUtf8String(name) +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                     reader->getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case Node::KIND_LOCALIZED_VALUE:
            throw css::uno::RuntimeException(
                (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad member <")) +
                 convertUtf8String(name) +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        case Node::KIND_GROUP:
            if (ns == XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("prop")))
            {
                handleGroupProp(
                    reader,
                    dynamic_cast< GroupNode * >(state_.top().node.get()));
            } else if (ns == XmlReader::NAMESPACE_NONE &&
                       name.equals(RTL_CONSTASCII_STRINGPARAM("node")))
            {
                handleGroupNode(reader, state_.top().node);
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad group node member <")) +
                     convertUtf8String(name) +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                     reader->getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case Node::KIND_SET:
            if (ns == XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("node")))
            {
                handleSetNode(
                    reader, dynamic_cast< SetNode * >(state_.top().node.get()));
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM("bad set node member <")) +
                     convertUtf8String(name) +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                     reader->getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        }
    }
    return true;
}

void XcuParser::endElement(XmlReader const * reader) {
    if (valueParser_.endElement(reader)) {
        return;
    }
    OSL_ASSERT(!state_.empty());
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
        state_.top().node->getMembers()[name] = insert;
    }
}

void XcuParser::characters(Span const & text) {
    valueParser_.characters(text);
}

void XcuParser::handleComponentData(XmlReader * reader) {
    Span attrPackage;
    Span attrName;
    Span attrOp;
    Span attrFinalized;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("package")))
        {
            attrPackage = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            attrOp = reader->getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("finalized")))
        {
            attrFinalized = reader->getAttributeValue(true);
        }
    }
    if (!attrPackage.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-data package attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-data name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OStringBuffer buf;
    buf.append(attrPackage.begin, attrPackage.length);
    buf.append('.');
    buf.append(attrName.begin, attrName.length);
    componentName_ = convertUtf8String(Span(buf.getStr(), buf.getLength()));
    rtl::Reference< Node > node(
        Data::findNode(
            valueParser_.getLayer(), data_->components, componentName_));
    if (!node.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unknown component ")) +
             componentName_ +
             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    switch (parseOperation(attrOp)) {
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
    int finalizedLayer = std::min(
        parseBoolean(attrFinalized, false) ? valueParser_.getLayer() : NO_LAYER,
        node->getFinalized());
    node->setFinalized(finalizedLayer);
    state_.push(State(node, finalizedLayer < valueParser_.getLayer(), false));
}

void XcuParser::handleItem(XmlReader * reader) {
    Span attrPath;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("path")))
        {
            attrPath = reader->getAttributeValue(false);
        }
    }
    if (!attrPath.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("missing path attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    int finalizedLayer;
    rtl::Reference< Node > node(
        data_->resolvePath(
            convertUtf8String(attrPath), &componentName_, 0, &pathPrefix_, 0,
            &finalizedLayer));
    pathPrefix_ += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    if (!node.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("nonexisting path attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    state_.push(
        State(
            node, finalizedLayer < valueParser_.getLayer(),
            valueParser_.getLayer() == NO_LAYER));
}

void XcuParser::handlePropValue(XmlReader * reader, PropertyNode * prop) {
    OSL_ASSERT(!state_.top().record);
    Span attrNil;
    Span attrSeparator;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_XSI &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("nil")))
        {
            attrNil = reader->getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("separator")))
        {
            attrSeparator = reader->getAttributeValue(false);
        }
    }
    if (parseBoolean(attrNil, false)) {
        if (!prop->isNillable()) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xsi:nil attribute for non-nillable prop in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
        prop->setValue(valueParser_.getLayer(), css::uno::Any());
        state_.push(State());
    } else {
        valueParser_.separator_ = attrSeparator;
        valueParser_.start(prop);
    }
}

void XcuParser::handleLocpropValue(
    XmlReader * reader, LocalizedPropertyNode * locprop)
{
    OSL_ASSERT(!state_.top().record);
    Span attrLang;
    Span attrNil;
    Span attrSeparator;
    Span attrOp;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_XML &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("lang")))
        {
            attrLang = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_XSI &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("nil")))
        {
            attrNil = reader->getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("separator")))
        {
            attrSeparator = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            attrOp = reader->getAttributeValue(true);
        }
    }
    rtl::OUString name;
    if (attrLang.is()) {
        name = convertUtf8String(attrLang);
    }
    NodeMap::iterator i(locprop->getMembers().find(name));
    if (i != locprop->getMembers().end() &&
        i->second->getLayer() > valueParser_.getLayer())
    {
        state_.push(State()); // ignored
        return;
    }
    bool nil = parseBoolean(attrNil, false);
    if (nil && !locprop->isNillable()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xsi:nil attribute for non-nillable prop in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    switch (parseOperation(attrOp)) {
    case OPERATION_MODIFY:
        if (nil) {
            if (i == locprop->getMembers().end()) {
                locprop->getMembers()[name] = new LocalizedPropertyValueNode(
                    valueParser_.getLayer(), css::uno::Any());
            } else {
                dynamic_cast< LocalizedPropertyValueNode * >(i->second.get())->
                    setValue(valueParser_.getLayer(), css::uno::Any());
            }
            state_.push(State());
        } else {
            valueParser_.separator_ = attrSeparator;
            valueParser_.start(locprop, name);
        }
        break;
    case OPERATION_REMOVE:
        //TODO: only allow if parent.op == OPERATION_FUSE
        //TODO: disallow removing when e.g. lang=""?
        if (i != locprop->getMembers().end()) {
            i->second->remove(valueParser_.getLayer());
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

void XcuParser::handleGroupProp(XmlReader * reader, GroupNode * group) {
    Span attrName;
    Span attrType;
    Span attrOp;
    Span attrFinalized;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("type")))
        {
            attrType = reader->getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            attrOp = reader->getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("finalized")))
        {
            attrFinalized = reader->getAttributeValue(true);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no prop name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(convertUtf8String(attrName));
    if (state_.top().record) {
        data_->addModification(pathPrefix_ + name);
    }
    Type type = parseType(reader, attrType);
    if (type == TYPE_ANY) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "invalid prop type attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    Operation op = parseOperation(attrOp);
    bool finalized = parseBoolean(attrFinalized, false);
    NodeMap::iterator i(group->getMembers().find(name));
    if (i == group->getMembers().end()) {
        handleUnknownGroupProp(reader, group, name, type, op, finalized);
    } else {
        switch (i->second->kind()) {
        case Node::KIND_PROPERTY:
            handlePlainGroupProp(
                reader, dynamic_cast< PropertyNode * >(i->second.get()), name,
                type, op, finalized);
            break;
        case Node::KIND_LOCALIZED_PROPERTY:
            handleLocalizedGroupProp(
                reader,
                dynamic_cast< LocalizedPropertyNode * >(i->second.get()), name,
                type, op, finalized);
            break;
        default:
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("inappropriate prop ")) +
                 name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
                 reader->getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
}

void XcuParser::handleUnknownGroupProp(
    XmlReader const * reader, GroupNode * group, rtl::OUString const & name,
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
            valueParser_.type_ = type;
            rtl::Reference< Node > prop(
                new PropertyNode(
                    valueParser_.getLayer(), TYPE_ANY, true, css::uno::Any(),
                    true));
            if (finalized) {
                prop->setFinalized(valueParser_.getLayer());
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
    XmlReader const * reader, PropertyNode * property,
    rtl::OUString const & name, Type type, Operation operation, bool finalized)
{
    if (property->getLayer() > valueParser_.getLayer()) {
        state_.push(State()); // ignored
        return;
    }
    int finalizedLayer = std::min(
        finalized ? valueParser_.getLayer() : NO_LAYER,
        property->getFinalized());
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
    valueParser_.type_ = type == TYPE_ERROR ? property->getType() : type;
    switch (operation) {
    case OPERATION_MODIFY:
    case OPERATION_REPLACE:
    case OPERATION_FUSE:
        state_.push(
            State(
                property,
                state_.top().locked || finalizedLayer < valueParser_.getLayer(),
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
        property->remove(valueParser_.getLayer());
        state_.push(State()); // ignore children
        break;
    }
}

void XcuParser::handleLocalizedGroupProp(
    XmlReader const * reader, LocalizedPropertyNode * property,
    rtl::OUString const & name, Type type, Operation operation, bool finalized)
{
    if (property->getLayer() > valueParser_.getLayer()) {
        state_.push(State()); // ignored
        return;
    }
    int finalizedLayer = std::min(
        finalized ? valueParser_.getLayer() : NO_LAYER,
        property->getFinalized());
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
    valueParser_.type_ = type == TYPE_ERROR ? property->getType() : type;
    switch (operation) {
    case OPERATION_MODIFY:
    case OPERATION_FUSE:
        state_.push(
            State(
                property,
                state_.top().locked || finalizedLayer < valueParser_.getLayer(),
                false));
        break;
    case OPERATION_REPLACE:
        {
            rtl::Reference< Node > replacement(
                new LocalizedPropertyNode(
                    valueParser_.getLayer(), property->getType(),
                    property->isNillable()));
            replacement->setFinalized(property->getFinalized());
            state_.push(
                State(
                    replacement, name,
                    (state_.top().locked ||
                     finalizedLayer < valueParser_.getLayer()),
                    false));
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

void XcuParser::handleGroupNode(
    XmlReader * reader, rtl::Reference< Node > const & group)
{
    OSL_ASSERT(!state_.top().record);
    Span attrName;
    Span attrOp;
    Span attrFinalized;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            attrOp = reader->getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("finalized")))
        {
            attrFinalized = reader->getAttributeValue(true);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no node name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(convertUtf8String(attrName));
    rtl::Reference< Node > subgroup(
        Data::findNode(valueParser_.getLayer(), group->getMembers(), name));
    if (!subgroup.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unknown node ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    Operation op(parseOperation(attrOp));
    if ((op != OPERATION_MODIFY && op != OPERATION_FUSE) || state_.top().record)
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "invalid operation on group node in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    int finalizedLayer = std::min(
        parseBoolean(attrFinalized, false) ? valueParser_.getLayer() : NO_LAYER,
        subgroup->getFinalized());
    subgroup->setFinalized(finalizedLayer);
    state_.push(
        State(
            subgroup,
            state_.top().locked || finalizedLayer < valueParser_.getLayer(),
            false));
}

void XcuParser::handleSetNode(XmlReader * reader, SetNode * set) {
    Span attrName;
    Span attrComponent;
    Span attrNodeType;
    Span attrOp;
    Span attrFinalized;
    Span attrMandatory;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader->nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("component")))
        {
            attrComponent = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("node-type")))
        {
            attrNodeType = reader->getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            attrOp = reader->getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("finalized")))
        {
            attrFinalized = reader->getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("mandatory")))
        {
            attrMandatory = reader->getAttributeValue(true);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no node name attribute in ")) +
             reader->getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(convertUtf8String(attrName));
    rtl::OUString templateName(
        parseTemplateReference(
            attrComponent, attrNodeType, componentName_,
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
    rtl::Reference< Node > tmpl(
        data_->getTemplate(valueParser_.getLayer(), templateName));
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
    Operation op(parseOperation(attrOp));
    int finalizedLayer = parseBoolean(attrFinalized, false)
        ? valueParser_.getLayer() : NO_LAYER;
    int mandatoryLayer = parseBoolean(attrMandatory, false)
        ? valueParser_.getLayer() : NO_LAYER;
    NodeMap::iterator i(set->getMembers().find(name));
    if (i != set->getMembers().end()) {
        finalizedLayer = std::min(finalizedLayer, i->second->getFinalized());
        i->second->setFinalized(finalizedLayer);
        mandatoryLayer = std::min(mandatoryLayer, i->second->getMandatory());
        i->second->setMandatory(mandatoryLayer);
        if (i->second->getLayer() > valueParser_.getLayer()) {
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
                i->second,
                state_.top().locked || finalizedLayer < valueParser_.getLayer(),
                false));
        break;
    case OPERATION_REPLACE:
        if (state_.top().locked || finalizedLayer < valueParser_.getLayer()) {
            state_.push(State()); // ignored
        } else {
            rtl::Reference< Node > member(tmpl->clone());
            member->setLayer(valueParser_.getLayer());
            member->setFinalized(finalizedLayer);
            member->setMandatory(mandatoryLayer);
            state_.push(State(member, name, false, false));
        }
        break;
    case OPERATION_FUSE:
        if (i == set->getMembers().end() || i->second->isRemoved()) {
            if (state_.top().locked || finalizedLayer < valueParser_.getLayer())
            {
                state_.push(State()); // ignored
            } else {
                rtl::Reference< Node > member(tmpl->clone());
                member->setLayer(valueParser_.getLayer());
                member->setFinalized(finalizedLayer);
                member->setMandatory(mandatoryLayer);
                state_.push(State(member, name, false, false));
            }
        } else {
            state_.push(
                State(
                    i->second,
                    (state_.top().locked ||
                     finalizedLayer < valueParser_.getLayer()),
                    false));
        }
        break;
    case OPERATION_REMOVE:
        // Ignore removal of unknown members, members finalized in a lower
        // layer, and members made mandatory in this or a lower layer:
        if (i != set->getMembers().end() && !state_.top().locked &&
            finalizedLayer >= valueParser_.getLayer() &&
            mandatoryLayer > valueParser_.getLayer())
        {
            i->second->remove(valueParser_.getLayer());
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
    if (reader_.get() == 0) {
        reader_.reset(new XmlReader(url_));
    }
    for (;;) {
        switch (itemData_.is()
                ? XmlReader::RESULT_BEGIN
                : reader_->nextItem(
                    parser_->getTextMode(), &itemData_, &itemNamespace_))
        {
        case XmlReader::RESULT_BEGIN:
            if (!parser_->startElement(
                    reader_.get(), itemNamespace_, itemData_))
            {
                return false;
            }
            break;
        case XmlReader::RESULT_END:
            parser_->endElement(reader_.get());
            break;
        case XmlReader::RESULT_TEXT:
            parser_->characters(itemData_);
            break;
        case XmlReader::RESULT_DONE:
            return true;
        }
        itemData_.clear();
    }
}

Reader::~Reader() {}

Parser::Parser() {}

Parser::~Parser() {}

XcdParser::XcdParser(
    int layer, Dependencies const & dependencies, Data * data):
    layer_(layer), dependencies_(dependencies), data_(data), state_(STATE_START)
{}

XcdParser::~XcdParser() {}

XmlReader::Text XcdParser::getTextMode() {
    return nestedParser_.is()
        ? nestedParser_->getTextMode() : XmlReader::TEXT_NONE;
}

bool XcdParser::startElement(
    XmlReader * reader, XmlReader::Namespace ns, Span const & name)
{
    if (nestedParser_.is()) {
        OSL_ASSERT(nesting_ != LONG_MAX);
        ++nesting_;
        return nestedParser_->startElement(reader, ns, name);
    }
    switch (state_) {
    case STATE_START:
        if (ns == XmlReader::NAMESPACE_OOR &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("data")))
        {
            state_ = STATE_DEPENDENCIES;
            return true;
        }
        break;
    case STATE_DEPENDENCIES:
        if (ns == XmlReader::NAMESPACE_NONE &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("dependency")))
        {
            if (dependency_.getLength() == 0) {
                Span attrFile;
                for (;;) {
                    XmlReader::Namespace attrNs;
                    Span attrLn;
                    if (!reader->nextAttribute(&attrNs, &attrLn)) {
                        break;
                    }
                    if (attrNs == XmlReader::NAMESPACE_NONE && //TODO: _OOR
                        attrLn.equals(RTL_CONSTASCII_STRINGPARAM("file")))
                    {
                        attrFile = reader->getAttributeValue(false);
                    }
                }
                if (!attrFile.is()) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "no dependency file attribute in ")) +
                         reader->getUrl()),
                        css::uno::Reference< css::uno::XInterface >());
                }
                dependency_ = convertUtf8String(attrFile);
                if (dependency_.getLength() == 0) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "bad dependency file attribute in ")) +
                         reader->getUrl()),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
            if (dependencies_.find(dependency_) == dependencies_.end()) {
                return false;
            }
            state_ = STATE_DEPENDENCY;
            dependency_ = rtl::OUString();
            return true;
        }
        state_ = STATE_COMPONENTS;
        // fall through
    case STATE_COMPONENTS:
        if (ns == XmlReader::NAMESPACE_OOR &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("component-schema")))
        {
            nestedParser_ = new XcsParser(layer_, data_);
            nesting_ = 1;
            return nestedParser_->startElement(reader, ns, name);
        }
        if (ns == XmlReader::NAMESPACE_OOR &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("component-data")))
        {
            nestedParser_ = new XcuParser(layer_ + 1, data_);
            nesting_ = 1;
            return nestedParser_->startElement(reader, ns, name);
        }
        break;
    default: // STATE_DEPENDENCY
        OSL_ASSERT(false); // this cannot happen
        break;
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad member <")) +
         convertUtf8String(name) +
         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
         reader->getUrl()),
        css::uno::Reference< css::uno::XInterface >());
}

void XcdParser::endElement(XmlReader const * reader) {
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

void XcdParser::characters(Span const & text) {
    if (nestedParser_.is()) {
        nestedParser_->characters(text);
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
                xmlString(convertToUtf8(parentPath).getStr()));
            writeNode(writer.writer, parent, name, node, true);
            xmlTextWriterEndElement(writer.writer);
            // It is never necessary to write the oor:mandatory attribute, as it
            // cannot be set via the UNO API.
        } else {
            parent = data.resolvePath(parentPath, 0, 0, 0, 0, 0);
            NodeMap::iterator j(parent->getMembers().find(name));
            if (j != parent->getMembers().end() &&
                j->second->getLayer() == NO_LAYER)
            {
                OSL_ASSERT(j->second->isRemoved());
                xmlTextWriterStartElement(writer.writer, xmlString("item"));
                switch (parent->kind()) {
                case Node::KIND_LOCALIZED_PROPERTY:
                    {
                        rtl::OUString parentName;
                        rtl::OUString grandparentPath(
                            parseLastSegment(parentPath, &parentName));
                        xmlTextWriterWriteAttribute(
                            writer.writer, xmlString("oor:path"),
                            xmlString(convertToUtf8(grandparentPath).getStr()));
                        xmlTextWriterStartElement(
                            writer.writer, xmlString("prop"));
                        xmlTextWriterWriteAttribute(
                            writer.writer, xmlString("oor:name"),
                            xmlString(convertToUtf8(parentName).getStr()));
                        xmlTextWriterWriteAttribute(
                            writer.writer, xmlString("oor:op"),
                            xmlString("fuse"));
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
                    }
                    break;
                case Node::KIND_GROUP:
                    OSL_ASSERT(
                        dynamic_cast< GroupNode * >(parent.get())->
                        isExtensible());
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:path"),
                        xmlString(convertToUtf8(parentPath).getStr()));
                    xmlTextWriterStartElement(writer.writer, xmlString("prop"));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:name"),
                        xmlString(convertToUtf8(name).getStr()));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:op"),
                        xmlString("remove"));
                    xmlTextWriterEndElement(writer.writer);
                    break;
                case Node::KIND_SET:
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:path"),
                        xmlString(convertToUtf8(parentPath).getStr()));
                    xmlTextWriterStartElement(writer.writer, xmlString("node"));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:name"),
                        xmlString(convertToUtf8(name).getStr()));
                    xmlTextWriterWriteAttribute(
                        writer.writer, xmlString("oor:op"),
                        xmlString("remove"));
                    xmlTextWriterEndElement(writer.writer);
                    break;
                default:
                    OSL_ASSERT(false); // this cannot happen
                    break;
                }
                xmlTextWriterEndElement(writer.writer);
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
