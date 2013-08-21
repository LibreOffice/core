/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sal/config.h"

#include <cassert>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/file.h"
#include "osl/file.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textcvt.h"
#include "rtl/textenc.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "sal/types.h"
#include "xmlreader/span.hxx"

#include "data.hxx"
#include "groupnode.hxx"
#include "localizedpropertynode.hxx"
#include "localizedvaluenode.hxx"
#include "modifications.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "propertynode.hxx"
#include "type.hxx"
#include "writemodfile.hxx"

namespace configmgr {

class Components;

namespace {

OString convertToUtf8(
    OUString const & text, sal_Int32 offset, sal_Int32 length)
{
    assert(offset <= text.getLength() && text.getLength() - offset >= length);
    OString s;
    if (!rtl_convertUStringToString(
            &s.pData, text.pData->buffer + offset, length,
            RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        throw css::uno::RuntimeException(
            "cannot convert to UTF-8",
            css::uno::Reference< css::uno::XInterface >());
    }
    return s;
}

struct TempFile: public boost::noncopyable {
    OUString url;
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
                SAL_WARN("configmgr", "osl_closeFile failed with " << +e);
            }
        }
        osl::FileBase::RC e = osl::File::remove(url);
        if (e != osl::FileBase::E_None) {
            SAL_WARN(
                "configmgr",
                "osl::File::remove(" << url << ") failed with " << +e);
        }
    }
}

void writeData(oslFileHandle handle, char const * begin, sal_Int32 length) {
    assert(length >= 0);
    sal_uInt64 n;
    if ((osl_writeFile(handle, begin, static_cast< sal_uInt32 >(length), &n) !=
         osl_File_E_None) ||
        n != static_cast< sal_uInt32 >(length))
    {
        throw css::uno::RuntimeException(
            "write failure", css::uno::Reference< css::uno::XInterface >());
    }
}

void writeData(oslFileHandle handle, OString const & text) {
    writeData(handle, text.getStr(), text.getLength());
}

void writeAttributeValue(oslFileHandle handle, OUString const & value) {
    sal_Int32 i = 0;
    sal_Int32 j = i;
    for (; j < value.getLength(); ++j) {
        assert(
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
    writeData(handle, OString::number(value));
}

void writeValueContent(oslFileHandle handle, sal_Int32 value) {
    writeData(handle, OString::number(value));
}

void writeValueContent(oslFileHandle handle, sal_Int64 value) {
    writeData(handle, OString::number(value));
}

void writeValueContent(oslFileHandle handle, double value) {
    writeData(handle, OString::number(value));
}

void writeValueContent(oslFileHandle handle, OUString const & value) {
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
                handle, OString::number(c));
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
        writeSingleValue< OUString >(handle, value);
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
        writeItemListValue< OUString >(handle, value);
        break;
    case TYPE_HEXBINARY_LIST:
        writeItemListValue< css::uno::Sequence< sal_Int8 > >(handle, value);
        break;
    default: // TYPE_ERROR, TYPE_NIL, TYPE_ANY
        assert(false); // this cannot happen
    }
}

void writeNode(
    Components & components, oslFileHandle handle,
    rtl::Reference< Node > const & parent, OUString const & name,
    rtl::Reference< Node > const & node)
{
    static xmlreader::Span const typeNames[] = {
        xmlreader::Span(), xmlreader::Span(), xmlreader::Span(),
            // TYPE_ERROR, TYPE_NIL, TYPE_ANY
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("xs:boolean")),
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("xs:short")),
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("xs:int")),
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("xs:long")),
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("xs:double")),
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("xs:string")),
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("xs:hexBinary")),
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("oor:boolean-list")),
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("oor:short-list")),
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("oor:int-list")),
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("oor:long-list")),
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("oor:double-list")),
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("oor:string-list")),
        xmlreader::Span(RTL_CONSTASCII_STRINGPARAM("oor:hexBinary-list")) };
    switch (node->kind()) {
    case Node::KIND_PROPERTY:
        {
            PropertyNode * prop = dynamic_cast< PropertyNode * >(node.get());
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("<prop oor:name=\""));
            writeAttributeValue(handle, name);
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("\" oor:op=\"fuse\""));
            Type type = prop->getStaticType();
            Type dynType = getDynamicType(prop->getValue(components));
            assert(dynType != TYPE_ERROR);
            if (type == TYPE_ANY) {
                type = dynType;
                if (type != TYPE_NIL) {
                    writeData(
                        handle, RTL_CONSTASCII_STRINGPARAM(" oor:type=\""));
                    writeData(
                        handle, typeNames[type].begin, typeNames[type].length);
                    writeData(handle, RTL_CONSTASCII_STRINGPARAM("\""));
                }
            }
            writeData(handle, "><value");
            if (dynType == TYPE_NIL) {
                writeData(
                    handle, RTL_CONSTASCII_STRINGPARAM(" xsi:nil=\"true\"/>"));
            } else {
                writeValue(handle, type, prop->getValue(components));
            }
            writeData(handle, "</prop>");
        }
        break;
    case Node::KIND_LOCALIZED_PROPERTY:
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("<prop oor:name=\""));
        writeAttributeValue(handle, name);
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("\" oor:op=\"fuse\">"));
        for (NodeMap::const_iterator i(node->getMembers().begin());
             i != node->getMembers().end(); ++i)
        {
            writeNode(components, handle, node, i->first, i->second);
        }
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("</prop>"));
        break;
    case Node::KIND_LOCALIZED_VALUE:
        {
            writeData(handle, RTL_CONSTASCII_STRINGPARAM("<value"));
            if (!name.isEmpty()) {
                writeData(handle, RTL_CONSTASCII_STRINGPARAM(" xml:lang=\""));
                writeAttributeValue(handle, name);
                writeData(handle, RTL_CONSTASCII_STRINGPARAM("\""));
            }
            Type type = dynamic_cast< LocalizedPropertyNode * >(parent.get())->
                getStaticType();
            css::uno::Any value(
                dynamic_cast< LocalizedValueNode * >(node.get())->getValue());
            Type dynType = getDynamicType(value);
            assert(dynType != TYPE_ERROR);
            if (type == TYPE_ANY) {
                type = dynType;
                if (type != TYPE_NIL) {
                    writeData(
                        handle, RTL_CONSTASCII_STRINGPARAM(" oor:type=\""));
                    writeData(
                        handle, typeNames[type].begin, typeNames[type].length);
                    writeData(handle, RTL_CONSTASCII_STRINGPARAM("\""));
                }
            }
            if (dynType == TYPE_NIL) {
                writeData(
                    handle, RTL_CONSTASCII_STRINGPARAM(" xsi:nil=\"true\"/>"));
            } else {
                writeValue(handle, type, value);
            }
        }
        break;
    case Node::KIND_GROUP:
    case Node::KIND_SET:
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("<node oor:name=\""));
        writeAttributeValue(handle, name);
        if (!node->getTemplateName().isEmpty()) { // set member
            writeData(
                handle, RTL_CONSTASCII_STRINGPARAM("\" oor:op=\"replace"));
        }
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("\">"));
        for (NodeMap::const_iterator i(node->getMembers().begin());
             i != node->getMembers().end(); ++i)
        {
            writeNode(components, handle, node, i->first, i->second);
        }
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("</node>"));
        break;
    case Node::KIND_ROOT:
        assert(false); // this cannot happen
        break;
    }
}

void writeModifications(
    Components & components, oslFileHandle handle,
    OUString const & parentPathRepresentation,
    rtl::Reference< Node > const & parent, OUString const & nodeName,
    rtl::Reference< Node > const & node,
    Modifications::Node const & modifications)
{
    // It is never necessary to write oor:finalized or oor:mandatory attributes,
    // as they cannot be set via the UNO API.
    if (modifications.children.empty()) {
        assert(parent.is());
            // components themselves have no parent but must have children
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("<item oor:path=\""));
        writeAttributeValue(handle, parentPathRepresentation);
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("\">"));
        if (node.is()) {
            writeNode(components, handle, parent, nodeName, node);
        } else {
            switch (parent->kind()) {
            case Node::KIND_LOCALIZED_PROPERTY:
                writeData(handle, RTL_CONSTASCII_STRINGPARAM("<value"));
                if (!nodeName.isEmpty()) {
                    writeData(
                        handle, RTL_CONSTASCII_STRINGPARAM(" xml:lang=\""));
                    writeAttributeValue(handle, nodeName);
                    writeData(handle, RTL_CONSTASCII_STRINGPARAM("\""));
                }
                writeData(
                    handle, RTL_CONSTASCII_STRINGPARAM(" oor:op=\"remove\"/>"));
                break;
            case Node::KIND_GROUP:
                assert(
                    dynamic_cast< GroupNode * >(parent.get())->isExtensible());
                writeData(
                    handle, RTL_CONSTASCII_STRINGPARAM("<prop oor:name=\""));
                writeAttributeValue(handle, nodeName);
                writeData(
                    handle,
                    RTL_CONSTASCII_STRINGPARAM("\" oor:op=\"remove\"/>"));
                break;
            case Node::KIND_SET:
                writeData(
                    handle, RTL_CONSTASCII_STRINGPARAM("<node oor:name=\""));
                writeAttributeValue(handle, nodeName);
                writeData(
                    handle,
                    RTL_CONSTASCII_STRINGPARAM("\" oor:op=\"remove\"/>"));
                break;
            default:
                assert(false); // this cannot happen
                break;
            }
        }
        writeData(handle, RTL_CONSTASCII_STRINGPARAM("</item>\n"));
    } else {
        assert(node.is());
        OUString pathRep(
            parentPathRepresentation + "/" +
            Data::createSegment(node->getTemplateName(), nodeName));
        for (Modifications::Node::Children::const_iterator i(
                 modifications.children.begin());
             i != modifications.children.end(); ++i)
        {
            writeModifications(
                components, handle, pathRep, node, i->first,
                node->getMember(i->first), i->second);
        }
    }
}

}

void writeModFile(
    Components & components, OUString const & url, Data const & data)
{
    sal_Int32 i = url.lastIndexOf('/');
    assert(i != -1);
    OUString dir(url.copy(0, i));
    switch (osl::Directory::createPath(dir)) {
    case osl::FileBase::E_None:
    case osl::FileBase::E_EXIST:
        break;
    case osl::FileBase::E_ACCES:
        SAL_INFO(
            "configmgr",
            ("cannot create registrymodifications.xcu path (E_ACCES); changes"
             " will be lost"));
        return;
    default:
        throw css::uno::RuntimeException(
            "cannot create directory " + dir,
            css::uno::Reference< css::uno::XInterface >());
    }
    TempFile tmp;
    switch (osl::FileBase::createTempFile(&dir, &tmp.handle, &tmp.url)) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_ACCES:
        SAL_INFO(
            "configmgr",
            ("cannot create temp registrymodifications.xcu (E_ACCES); changes"
             " will be lost"));
        return;
    default:
        throw css::uno::RuntimeException(
            "cannot create temporary file in " + dir,
            css::uno::Reference< css::uno::XInterface >());
    }
    writeData(
        tmp.handle,
        RTL_CONSTASCII_STRINGPARAM(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<oor:items"
            " xmlns:oor=\"http://openoffice.org/2001/registry\""
            " xmlns:xs=\"http://www.w3.org/2001/XMLSchema\""
            " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"));
    //TODO: Do not write back information about those removed items that did not
    // come from the .xcs/.xcu files, anyway (but had been added dynamically
    // instead):
    for (Modifications::Node::Children::const_iterator j(
             data.modifications.getRoot().children.begin());
         j != data.modifications.getRoot().children.end(); ++j)
    {
        writeModifications(
            components, tmp.handle, "", rtl::Reference< Node >(), j->first,
            Data::findNode(Data::NO_LAYER, data.getComponents(), j->first),
            j->second);
    }
    writeData(tmp.handle, RTL_CONSTASCII_STRINGPARAM("</oor:items>\n"));
    oslFileError e = osl_closeFile(tmp.handle);
    tmp.closed = true;
    if (e != osl_File_E_None) {
        throw css::uno::RuntimeException(
            "cannot close " + tmp.url,
            css::uno::Reference< css::uno::XInterface >());
    }
    if (osl::File::move(tmp.url, url) != osl::FileBase::E_None) {
        throw css::uno::RuntimeException(
            "cannot move " + tmp.url,
            css::uno::Reference< css::uno::XInterface >());
    }
    tmp.handle = 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
