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

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "osl/file.h"
#include "osl/file.hxx"
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
#include "localizedpropertynode.hxx"
#include "localizedvaluenode.hxx"
#include "modifications.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "propertynode.hxx"
#include "span.hxx"
#include "type.hxx"
#include "writemodfile.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

rtl::OString convertToUtf8(
    rtl::OUString const & text, sal_Int32 offset, sal_Int32 length)
{
    OSL_ASSERT(
        offset <= text.getLength() && text.getLength() - offset >= length);
    rtl::OString s;
    if (!rtl_convertUStringToString(
            &s.pData, text.pData->buffer + offset, length,
            RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("cannot convert to UTF-8")),
            css::uno::Reference< css::uno::XInterface >());
    }
    return s;
}

struct TempFile: public boost::noncopyable {
    rtl::OUString url;
    oslFileHandle handle;
    bool closed;

    TempFile(): handle(0), closed(false) {}

    ~TempFile();
};

TempFile::~TempFile() {
    if (handle != 0) {
        if (!closed) {
            oslFileError e = osl_closeFile(handle);
            if (e != osl_File_E_None) {
                OSL_TRACE(
                    "osl_closeFile failed with %ld", static_cast< long >(e));
            }
        }
        osl::FileBase::RC e = osl::File::remove(url);
        if (e != osl::FileBase::E_None) {
            OSL_TRACE("osl_removeFile failed with %ld", static_cast< long >(e));
        }
    }
}

void writeData(oslFileHandle handle, char const * begin, sal_Int32 length) {
    OSL_ASSERT(length >= 0);
    sal_uInt64 n;
    if ((osl_writeFile(handle, begin, static_cast< sal_uInt32 >(length), &n) !=
         osl_File_E_None) ||
        n != static_cast< sal_uInt32 >(length))
    {
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("write failure")),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void writeData(oslFileHandle handle, rtl::OString const & text) {
    writeData(handle, text.getStr(), text.getLength());
}

void writeAttributeValue(oslFileHandle handle, rtl::OUString const & value) {
    sal_Int32 i = 0;
    sal_Int32 j = i;
    for (; j < value.getLength(); ++j) {
        OSL_ASSERT(
            value[j] == 0x0009 || value[j] == 0x000A || value[j] == 0x000D ||
            (value[j] >= 0x0020 && value[j] != 0xFFFE && value[j] != 0xFFFF));
        switch(value[j]) {
        case '\x09':
            writeData(handle, convertToUtf8(value, i, j - i));
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("&#9;"));
            i = j + 1;
            break;
        case '\x0A':
            writeData(handle, convertToUtf8(value, i, j - i));
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("&#xA;"));
            i = j + 1;
            break;
        case '\x0D':
            writeData(handle, convertToUtf8(value, i, j - i));
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("&#xD;"));
            i = j + 1;
            break;
        case '"':
            writeData(handle, convertToUtf8(value, i, j - i));
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("&quot;"));
            i = j + 1;
            break;
        case '&':
            writeData(handle, convertToUtf8(value, i, j - i));
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("&amp;"));
            i = j + 1;
            break;
        case '<':
            writeData(handle, convertToUtf8(value, i, j - i));
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("&lt;"));
            i = j + 1;
            break;
        default:
            break;
        }
    }
    writeData(handle, convertToUtf8(value, i, j - i));
}

void writeValueContent(oslFileHandle handle, sal_Bool value) {
    if (value) {
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("true"));
    } else {
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("false"));
    }
}

void writeValueContent(oslFileHandle handle, sal_Int16 value) {
    writeData(handle, rtl::OString::valueOf(static_cast< sal_Int32 >(value)));
}

void writeValueContent(oslFileHandle handle, sal_Int32 value) {
    writeData(handle, rtl::OString::valueOf(value));
}

void writeValueContent(oslFileHandle handle, sal_Int64 value) {
    writeData(handle, rtl::OString::valueOf(value));
}

void writeValueContent(oslFileHandle handle, double value) {
    writeData(handle, rtl::OString::valueOf(value));
}

void writeValueContent(oslFileHandle handle, rtl::OUString const & value) {
    sal_Int32 i = 0;
    sal_Int32 j = i;
    for (; j < value.getLength(); ++j) {
        sal_Unicode c = value[j];
        if ((c < 0x0020 && c != 0x0009 && c != 0x000A && c != 0x000D) ||
            c == 0xFFFE || c == 0xFFFF)
        {
            writeData(handle, convertToUtf8(value, i, j - i));
            writeData(
                handle, RTL_CONSTASCII_STRINGPARAM("<unicode oor:scalar=\""));
            writeData(
                handle, rtl::OString::valueOf(static_cast< sal_Int32 >(c)));
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("\"/>"));
            i = j + 1;
        } else if (c == '\x0D') {
            writeData(handle, convertToUtf8(value, i, j - i));
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("&#xD;"));
            i = j + 1;
        } else if (c == '&') {
            writeData(handle, convertToUtf8(value, i, j - i));
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("&amp;"));
            i = j + 1;
        } else if (c == '<') {
            writeData(handle, convertToUtf8(value, i, j - i));
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("&lt;"));
            i = j + 1;
        } else if (c == '>') {
            // "MUST, for compatibility, be escaped [...] when it appears in the
            // string ']]>'":
            writeData(handle, convertToUtf8(value, i, j - i));
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("&gt;"));
            i = j + 1;
        }
    }
    writeData(handle, convertToUtf8(value, i, j - i));
}

void writeValueContent(
    oslFileHandle handle, css::uno::Sequence< sal_Int8 > const & value)
{
    for (sal_Int32 i = 0; i < value.getLength(); ++i) {
        static char const hexDigit[16] = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
            'D', 'E', 'F' };
        writeData(handle, hexDigit + ((value[i] >> 4) & 0xF), 1);
        writeData(handle, hexDigit + (value[i] & 0xF), 1);
    }
}

template< typename T > void writeSingleValue(
    oslFileHandle handle, css::uno::Any const & value)
{
    writeData(handle, RTL_CONSTASCII_STRINGPARAM(">"));
    T val = T();
    value >>= val;
    writeValueContent(handle, val);
    writeData(handle, RTL_CONSTASCII_STRINGPARAM("</value>"));
}

template< typename T > void writeListValue(
    oslFileHandle handle, css::uno::Any const & value)
{
    writeData(handle, RTL_CONSTASCII_STRINGPARAM(">"));
    css::uno::Sequence< T > val;
    value >>= val;
    for (sal_Int32 i = 0; i < val.getLength(); ++i) {
        if (i != 0) {
            writeData(handle, RTL_CONSTASCII_STRINGPARAM(" "));
        }
        writeValueContent(handle, val[i]);
    }
    writeData(handle, RTL_CONSTASCII_STRINGPARAM("</value>"));
}

template< typename T > void writeItemListValue(
    oslFileHandle handle, css::uno::Any const & value)
{
    writeData(handle, RTL_CONSTASCII_STRINGPARAM(">"));
    css::uno::Sequence< T > val;
    value >>= val;
    for (sal_Int32 i = 0; i < val.getLength(); ++i) {
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("<it>"));
        writeValueContent(handle, val[i]);
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("</it>"));
    }
    writeData(handle, RTL_CONSTASCII_STRINGPARAM("</value>"));
}

void writeValue(oslFileHandle handle, Type type, css::uno::Any const & value) {
    switch (type) {
    case TYPE_NIL:
        writeData(handle, RTL_CONSTASCII_STRINGPARAM(" xsi:nil=\"true\"/>"));
        break;
    case TYPE_BOOLEAN:
        writeSingleValue< sal_Bool >(handle, value);
        break;
    case TYPE_SHORT:
        writeSingleValue< sal_Int16 >(handle, value);
        break;
    case TYPE_INT:
        writeSingleValue< sal_Int32 >(handle, value);
        break;
    case TYPE_LONG:
        writeSingleValue< sal_Int64 >(handle, value);
        break;
    case TYPE_DOUBLE:
        writeSingleValue< double >(handle, value);
        break;
    case TYPE_STRING:
        writeSingleValue< rtl::OUString >(handle, value);
        break;
    case TYPE_HEXBINARY:
        writeSingleValue< css::uno::Sequence< sal_Int8 > >(handle, value);
        break;
    case TYPE_BOOLEAN_LIST:
        writeListValue< sal_Bool >(handle, value);
        break;
    case TYPE_SHORT_LIST:
        writeListValue< sal_Int16 >(handle, value);
        break;
    case TYPE_INT_LIST:
        writeListValue< sal_Int32 >(handle, value);
        break;
    case TYPE_LONG_LIST:
        writeListValue< sal_Int64 >(handle, value);
        break;
    case TYPE_DOUBLE_LIST:
        writeListValue< double >(handle, value);
        break;
    case TYPE_STRING_LIST:
        writeItemListValue< rtl::OUString >(handle, value);
        break;
    case TYPE_HEXBINARY_LIST:
        writeItemListValue< css::uno::Sequence< sal_Int8 > >(handle, value);
        break;
    default: // TYPE_ERROR, TYPE_ANY
        OSL_ASSERT(false); // this cannot happen
    }
}

void writeNode(
    oslFileHandle handle, rtl::Reference< Node > const & parent,
    rtl::OUString const & name, rtl::Reference< Node > const & node)
{
    static Span const typeNames[] = {
        Span(), Span(), Span(), // TYPE_ERROR, TYPE_NIL, TYPE_ANY
        Span(RTL_CONSTASCII_STRINGPARAM("xs:boolean")),
        Span(RTL_CONSTASCII_STRINGPARAM("xs:short")),
        Span(RTL_CONSTASCII_STRINGPARAM("xs:int")),
        Span(RTL_CONSTASCII_STRINGPARAM("xs:long")),
        Span(RTL_CONSTASCII_STRINGPARAM("xs:double")),
        Span(RTL_CONSTASCII_STRINGPARAM("xs:string")),
        Span(RTL_CONSTASCII_STRINGPARAM("xs:hexBinary")),
        Span(RTL_CONSTASCII_STRINGPARAM("oor:boolean-list")),
        Span(RTL_CONSTASCII_STRINGPARAM("oor:short-list")),
        Span(RTL_CONSTASCII_STRINGPARAM("oor:int-list")),
        Span(RTL_CONSTASCII_STRINGPARAM("oor:long-list")),
        Span(RTL_CONSTASCII_STRINGPARAM("oor:double-list")),
        Span(RTL_CONSTASCII_STRINGPARAM("oor:string-list")),
        Span(RTL_CONSTASCII_STRINGPARAM("oor:hexBinary-list")) };
    switch (node->kind()) {
    case Node::KIND_PROPERTY:
        {
            PropertyNode * prop = dynamic_cast< PropertyNode * >(node.get());
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("<prop oor:name=\""));
            writeAttributeValue(handle, name);
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("\" oor:op=\"fuse\""));
            Type type = prop->getType();
            if (type == TYPE_ANY) {
                type = mapType(prop->getValue());
                if (type != TYPE_ERROR) { //TODO
                    writeData(
                        handle, RTL_CONSTASCII_STRINGPARAM(" oor:type=\""));
                    writeData(
                        handle, typeNames[type].begin, typeNames[type].length);
                    writeData(handle, RTL_CONSTASCII_STRINGPARAM("\""));
                }
            }
            writeData(handle, "><value");
            writeValue(handle, type, prop->getValue());
            writeData(handle, "</prop>");
        }
        break;
    case Node::KIND_LOCALIZED_PROPERTY:
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("<prop oor:name=\""));
        writeAttributeValue(handle, name);
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("\" oor:op=\"fuse\">"));
        for (NodeMap::iterator i(node->getMembers().begin());
             i != node->getMembers().end(); ++i)
        {
            writeNode(handle, node, i->first, i->second);
        }
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("</prop>"));
        break;
    case Node::KIND_LOCALIZED_VALUE:
        {
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("<value"));
            if (name.getLength() != 0) {
                writeData(handle, RTL_CONSTASCII_STRINGPARAM(" xml:lang=\""));
                writeAttributeValue(handle, name);
                writeData(handle, RTL_CONSTASCII_STRINGPARAM("\""));
            }
            Type type = dynamic_cast< LocalizedPropertyNode * >(parent.get())->
                getType();
            css::uno::Any value(
                dynamic_cast< LocalizedValueNode * >(node.get())->getValue());
            if (type == TYPE_ANY) {
                type = mapType(value);
                if (type != TYPE_ERROR) { // TODO
                    writeData(
                        handle, RTL_CONSTASCII_STRINGPARAM(" oor:type=\""));
                    writeData(
                        handle, typeNames[type].begin, typeNames[type].length);
                    writeData(handle, RTL_CONSTASCII_STRINGPARAM("\""));
                }
            }
            writeValue(handle, type, value);
        }
        break;
    case Node::KIND_GROUP:
    case Node::KIND_SET:
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("<node oor:name=\""));
        writeAttributeValue(handle, name);
        if (node->getTemplateName().getLength() != 0) { // set member
            writeData(
                handle, RTL_CONSTASCII_STRINGPARAM("\" oor:op=\"replace"));
        }
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("\">"));
        for (NodeMap::iterator i(node->getMembers().begin());
             i != node->getMembers().end(); ++i)
        {
            writeNode(handle, node, i->first, i->second);
        }
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("</node>"));
        break;
    }
}

void writeModifications(
    oslFileHandle handle, rtl::OUString const & grandparentPathRepresentation,
    rtl::OUString const & parentName, rtl::Reference< Node > const & parent,
    rtl::OUString const & nodeName, rtl::Reference< Node > const & node,
    Modifications const & modifications)
{
    if (modifications.children.empty()) {
        OSL_ASSERT(parent.is());
            // components themselves have no parent but must have children
        if (node.is()) {
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("<item oor:path=\""));
            writeAttributeValue(
                handle,
                (grandparentPathRepresentation +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) +
                 Data::createSegment(parent->getTemplateName(), parentName)));
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("\">"));
            writeNode(handle, parent, nodeName, node);
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("</item>"));
            // It is never necessary to write the oor:mandatory attribute, as it
            // cannot be set via the UNO API.
        } else {
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("<item oor:path=\""));
            switch (parent->kind()) {
            case Node::KIND_LOCALIZED_PROPERTY:
                writeAttributeValue(handle, grandparentPathRepresentation);
                writeData(
                    handle, RTL_CONSTASCII_STRINGPARAM("\"><prop oor:name=\""));
                writeAttributeValue(handle, parentName);
                writeData(
                    handle,
                    RTL_CONSTASCII_STRINGPARAM("\" oor:op=\"fuse\"><value"));
                if (nodeName.getLength() != 0) {
                    writeData(
                        handle, RTL_CONSTASCII_STRINGPARAM(" xml:lang=\""));
                    writeAttributeValue(handle, nodeName);
                    writeData(handle, RTL_CONSTASCII_STRINGPARAM("\""));
                }
                writeData(
                    handle,
                    RTL_CONSTASCII_STRINGPARAM(
                        " oor:op=\"remove\"/></prop></item>"));
                break;
            case Node::KIND_GROUP:
                OSL_ASSERT(
                    dynamic_cast< GroupNode * >(parent.get())->isExtensible());
                writeAttributeValue(
                    handle,
                    (grandparentPathRepresentation +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) +
                     Data::createSegment(
                         parent->getTemplateName(), parentName)));
                writeData(
                    handle, RTL_CONSTASCII_STRINGPARAM("\"><prop oor:name=\""));
                writeAttributeValue(handle, nodeName);
                writeData(
                    handle,
                    RTL_CONSTASCII_STRINGPARAM(
                        "\" oor:op=\"remove\"/></item>"));
                break;
            case Node::KIND_SET:
                writeAttributeValue(
                    handle,
                    (grandparentPathRepresentation +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) +
                     Data::createSegment(
                         parent->getTemplateName(), parentName)));
                writeData(
                    handle, RTL_CONSTASCII_STRINGPARAM("\"><node oor:name=\""));
                writeAttributeValue(handle, nodeName);
                writeData(
                    handle,
                    RTL_CONSTASCII_STRINGPARAM(
                        "\" oor:op=\"remove\"/></item>"));
                break;
            default:
                OSL_ASSERT(false); // this cannot happen
                break;
            }
        }
    } else {
        rtl::OUString parentPathRep;
        if (parent.is()) { // components themselves have no parent
            parentPathRep = grandparentPathRepresentation +
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) +
                Data::createSegment(parent->getTemplateName(), parentName);
        }
        OSL_ASSERT(node.is());
        for (Modifications::Children::const_iterator i(
                 modifications.children.begin());
             i != modifications.children.end(); ++i)
        {
            writeModifications(
                handle, parentPathRep, nodeName, node, i->first,
                node->getMember(i->first), i->second);
        }
    }
}

}

void writeModFile(rtl::OUString const & url, Data const & data) {
    sal_Int32 i = url.lastIndexOf('/');
    OSL_ASSERT(i != -1);
    rtl::OUString dir(url.copy(0, i));
    switch (osl::Directory::createPath(dir)) {
    case osl::FileBase::E_None:
    case osl::FileBase::E_EXIST:
        break;
    default:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("cannot create directory ")) +
             dir),
            css::uno::Reference< css::uno::XInterface >());
    }
    TempFile tmp;
    if (osl::FileBase::createTempFile(&dir, &tmp.handle, &tmp.url) !=
        osl::FileBase::E_None)
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "cannot create temporary file in ")) +
             dir),
            css::uno::Reference< css::uno::XInterface >());
    }
    writeData(
        tmp.handle,
        RTL_CONSTASCII_STRINGPARAM(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?><oor:items"
            " xmlns:oor=\"http://openoffice.org/2001/registry\""
            " xmlns:xs=\"http://www.w3.org/2001/XMLSchema\""
            " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"));
    //TODO: Do not write back information about those removed items that did not
    // come from the .xcs/.xcu files, anyway (but had been added dynamically
    // instead):
    for (Modifications::Children::const_iterator j(
             data.modifications.children.begin());
         j != data.modifications.children.end(); ++j)
    {
        writeModifications(
            tmp.handle, rtl::OUString(), rtl::OUString(),
            rtl::Reference< Node >(), j->first,
            Data::findNode(Data::NO_LAYER, data.components, j->first),
            j->second);
    }
    writeData(tmp.handle, RTL_CONSTASCII_STRINGPARAM("</oor:items>"));
    oslFileError e = osl_closeFile(tmp.handle);
    tmp.closed = true;
    if (e != osl_File_E_None) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cannot close ")) +
             tmp.url),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (osl::File::move(tmp.url, url) != osl::FileBase::E_None) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cannot move ")) +
             tmp.url),
            css::uno::Reference< css::uno::XInterface >());
    }
    tmp.handle = 0;
}

}
