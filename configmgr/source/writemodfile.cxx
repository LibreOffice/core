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

#include <sal/config.h>

#include <cassert>
#include <cstddef>
#include <limits>
#include <string_view>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <o3tl/safeint.hxx>
#include <osl/file.h>
#include <osl/file.hxx>
#include <rtl/string.h>
#include <rtl/string.hxx>
#include <rtl/textcvt.h>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <xmlreader/span.hxx>

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

OString convertToUtf8(std::u16string_view text) {
    OString s;
    assert(text.size() <= o3tl::make_unsigned(std::numeric_limits<sal_Int32>::max()));
    if (!rtl_convertUStringToString(
            &s.pData, text.data(), text.size(),
            RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        throw css::uno::RuntimeException(
            "cannot convert to UTF-8");
    }
    return s;
}

} // anonymous namespace

TempFile::~TempFile() {
    if (handle == nullptr)
        return;

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

#ifdef _WIN32
oslFileError TempFile::closeWithoutUnlink() {
    flush();
    oslFileError e = osl_closeFile(handle);
    handle = nullptr;
    closed = true;
    return e;
}
#endif

void TempFile::closeAndRename(const OUString &_url) {
    oslFileError e = flush();
    if (e != osl_File_E_None) {
        throw css::uno::RuntimeException(
            "cannot write to " + url);
    }
    e = osl_closeFile(handle);
    closed = true;
    if (e != osl_File_E_None) {
        throw css::uno::RuntimeException(
            "cannot close " + url);
    }
    if (osl::File::replace(url, _url) != osl::FileBase::E_None) {
        throw css::uno::RuntimeException(
            "cannot move " + url);
    }
    handle = nullptr;
}

oslFileError TempFile::flush() {
    oslFileError e = osl_File_E_None;
    if (!buffer.isEmpty()) {
        sal_uInt64 nBytesWritten = 0;
        e = osl_writeFile(handle, buffer.getStr(),
                          static_cast< sal_uInt32 >(buffer.getLength()),
                          &nBytesWritten);
        if (nBytesWritten != static_cast< sal_uInt32 >(buffer.getLength())) {
            // queue up any error / exception until close.
            buffer.remove(0, static_cast< sal_Int32 >( nBytesWritten ) );
        } else {
            buffer.setLength(0);
        }
    }
    return e;
}

void TempFile::writeString(std::string_view text) {
    buffer.append(text.data(), text.size());
    if (buffer.getLength() > 0x10000)
        flush();
}

namespace {

void writeValueContent_(TempFile &, bool) = delete;
    // silence loplugin:salbool
void writeValueContent_(TempFile &handle, sal_Bool value) {
    if (value) {
        handle.writeString("true");
    } else {
        handle.writeString("false");
    }
}

void writeValueContent_(TempFile &handle, sal_Int16 value) {
    handle.writeString(OString::number(value));
}

void writeValueContent_(TempFile &handle, sal_Int32 value) {
    handle.writeString(OString::number(value));
}

void writeValueContent_(TempFile &handle, sal_Int64 value) {
    handle.writeString(OString::number(value));
}

void writeValueContent_(TempFile &handle, double value) {
    handle.writeString(OString::number(value));
}

void writeValueContent_(TempFile &handle, std::u16string_view value) {
    writeValueContent(handle, value);
}

void writeValueContent_(
    TempFile &handle, css::uno::Sequence< sal_Int8 > const & value)
{
    for (const auto & v : value) {
        static char const hexDigit[16] = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
            'D', 'E', 'F' };
        handle.writeString(
            std::string_view(hexDigit + ((v >> 4) & 0xF), 1));
        handle.writeString(std::string_view(hexDigit + (v & 0xF), 1));
    }
}

template< typename T > void writeSingleValue(
    TempFile &handle, css::uno::Any const & value)
{
    handle.writeString(">");
    T val = T();
    value >>= val;
    writeValueContent_(handle, val);
    handle.writeString("</value>");
}

template< typename T > void writeListValue(
    TempFile &handle, css::uno::Any const & value)
{
    handle.writeString(">");
    css::uno::Sequence< T > val;
    value >>= val;
    for (sal_Int32 i = 0; i < val.getLength(); ++i) {
        if (i != 0) {
            handle.writeString(" ");
        }
        writeValueContent_(handle, std::as_const(val)[i]);
    }
    handle.writeString("</value>");
}

template< typename T > void writeItemListValue(
    TempFile &handle, css::uno::Any const & value)
{
    handle.writeString(">");
    css::uno::Sequence< T > val;
    value >>= val;
    for (const auto & i : std::as_const(val)) {
        handle.writeString("<it>");
        writeValueContent_(handle, i);
        handle.writeString("</it>");
    }
    handle.writeString("</value>");
}

void writeValue(TempFile &handle, Type type, css::uno::Any const & value) {
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
    Components & components, TempFile &handle,
    rtl::Reference< Node > const & parent, std::u16string_view name,
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
            PropertyNode * prop = static_cast< PropertyNode * >(node.get());
            handle.writeString("<prop oor:name=\"");
            writeAttributeValue(handle, name);
            handle.writeString("\" oor:op=\"fuse\"");
            Type type = prop->getStaticType();
            Type dynType = getDynamicType(prop->getValue(components));
            assert(dynType != TYPE_ERROR);
            if (type == TYPE_ANY) {
                type = dynType;
                if (type != TYPE_NIL) {
                    handle.writeString(" oor:type=\"");
                    handle.writeString(
                        std::string_view(
                            typeNames[type].begin, typeNames[type].length));
                    handle.writeString("\"");
                }
            }
            handle.writeString("><value");
            if (dynType == TYPE_NIL) {
                handle.writeString(" xsi:nil=\"true\"/>");
            } else {
                writeValue(handle, type, prop->getValue(components));
            }
            handle.writeString("</prop>");
        }
        break;
    case Node::KIND_LOCALIZED_PROPERTY:
        handle.writeString("<prop oor:name=\"");
        writeAttributeValue(handle, name);
        handle.writeString("\" oor:op=\"fuse\">");
        for (auto const& member : node->getMembers())
        {
            writeNode(components, handle, node, member.first, member.second);
        }
        handle.writeString("</prop>");
        break;
    case Node::KIND_LOCALIZED_VALUE:
        {
            handle.writeString("<value");
            if (!name.empty()) {
                handle.writeString(" xml:lang=\"");
                writeAttributeValue(handle, name);
                handle.writeString("\"");
            }
            Type type = static_cast< LocalizedPropertyNode * >(parent.get())->
                getStaticType();
            css::uno::Any value(
                static_cast< LocalizedValueNode * >(node.get())->getValue());
            Type dynType = getDynamicType(value);
            assert(dynType != TYPE_ERROR);
            if (type == TYPE_ANY) {
                type = dynType;
                if (type != TYPE_NIL) {
                    handle.writeString(" oor:type=\"");
                    handle.writeString(
                        std::string_view(
                            typeNames[type].begin, typeNames[type].length));
                    handle.writeString("\"");
                }
            }
            if (dynType == TYPE_NIL) {
                handle.writeString(" xsi:nil=\"true\"/>");
            } else {
                writeValue(handle, type, value);
            }
        }
        break;
    case Node::KIND_GROUP:
    case Node::KIND_SET:
        handle.writeString("<node oor:name=\"");
        writeAttributeValue(handle, name);
        if (!node->getTemplateName().isEmpty()) { // set member
            handle.writeString("\" oor:op=\"replace");
        }
        handle.writeString("\">");
        for (auto const& member : node->getMembers())
        {
            writeNode(components, handle, node, member.first, member.second);
        }
        handle.writeString("</node>");
        break;
    case Node::KIND_ROOT:
        assert(false); // this cannot happen
        break;
    }
}

// helpers to allow sorting of configmgr::Modifications::Node
typedef std::pair< const OUString, configmgr::Modifications::Node > ModNodePairEntry;
struct PairEntrySorter
{
    bool operator() (const ModNodePairEntry* pValue1, const ModNodePairEntry* pValue2) const
    {
        return pValue1->first.compareTo(pValue2->first) < 0;
    }
};

void writeModifications(
    Components & components, TempFile &handle,
    std::u16string_view parentPathRepresentation,
    rtl::Reference< Node > const & parent, OUString const & nodeName,
    rtl::Reference< Node > const & node,
    Modifications::Node const & modifications)
{
    // It is never necessary to write oor:finalized or oor:mandatory attributes,
    // as they cannot be set via the UNO API.
    if (modifications.children.empty()) {
        assert(parent.is());
            // components themselves have no parent but must have children
        handle.writeString("<item oor:path=\"");
        writeAttributeValue(handle, parentPathRepresentation);
        handle.writeString("\">");
        if (node.is()) {
            writeNode(components, handle, parent, nodeName, node);
        } else {
            switch (parent->kind()) {
            case Node::KIND_LOCALIZED_PROPERTY:
                handle.writeString("<value");
                if (!nodeName.isEmpty()) {
                    handle.writeString(" xml:lang=\"");
                    writeAttributeValue(handle, nodeName);
                    handle.writeString("\"");
                }
                handle.writeString(" oor:op=\"remove\"/>");
                break;
            case Node::KIND_GROUP:
                assert(
                    static_cast< GroupNode * >(parent.get())->isExtensible());
                handle.writeString("<prop oor:name=\"");
                writeAttributeValue(handle, nodeName);
                handle.writeString("\" oor:op=\"remove\"/>");
                break;
            case Node::KIND_SET:
                handle.writeString("<node oor:name=\"");
                writeAttributeValue(handle, nodeName);
                handle.writeString("\" oor:op=\"remove\"/>");
                break;
            default:
                assert(false); // this cannot happen
                break;
            }
        }
        handle.writeString("</item>\n");
    } else {
        assert(node.is());
        OUString pathRep(
            OUString::Concat(parentPathRepresentation) + "/" +
            Data::createSegment(node->getTemplateName(), nodeName));

        // copy configmgr::Modifications::Node's to a sortable list. Use pointers
        // to just reference the data instead of copying it
        std::vector< const ModNodePairEntry* > ModNodePairEntryVector;
        ModNodePairEntryVector.reserve(modifications.children.size());

        for (const auto& rCand : modifications.children)
        {
            ModNodePairEntryVector.push_back(&rCand);
        }

        // sort the list
        std::sort(ModNodePairEntryVector.begin(), ModNodePairEntryVector.end(), PairEntrySorter());

        // now use the list to write entries in sorted order
        // instead of random as from the unordered map
        for (const auto & i : ModNodePairEntryVector)
        {
            writeModifications(
                components, handle, pathRep, node, i->first,
                node->getMember(i->first), i->second);
        }
    }
}

}

void writeAttributeValue(TempFile &handle, std::u16string_view value) {
    std::size_t i = 0;
    std::size_t j = i;
    for (; j != value.size(); ++j) {
        assert(
            value[j] == 0x0009 || value[j] == 0x000A || value[j] == 0x000D ||
            (value[j] >= 0x0020 && value[j] != 0xFFFE && value[j] != 0xFFFF));
        switch(value[j]) {
        case '\x09':
            handle.writeString(convertToUtf8(value.substr(i, j - i)));
            handle.writeString("&#9;");
            i = j + 1;
            break;
        case '\x0A':
            handle.writeString(convertToUtf8(value.substr(i, j - i)));
            handle.writeString("&#xA;");
            i = j + 1;
            break;
        case '\x0D':
            handle.writeString(convertToUtf8(value.substr(i, j - i)));
            handle.writeString("&#xD;");
            i = j + 1;
            break;
        case '"':
            handle.writeString(convertToUtf8(value.substr(i, j - i)));
            handle.writeString("&quot;");
            i = j + 1;
            break;
        case '&':
            handle.writeString(convertToUtf8(value.substr(i, j - i)));
            handle.writeString("&amp;");
            i = j + 1;
            break;
        case '<':
            handle.writeString(convertToUtf8(value.substr(i, j - i)));
            handle.writeString("&lt;");
            i = j + 1;
            break;
        default:
            break;
        }
    }
    handle.writeString(convertToUtf8(value.substr(i, j - i)));
}

void writeValueContent(TempFile &handle, std::u16string_view value) {
    std::size_t i = 0;
    std::size_t j = i;
    for (; j != value.size(); ++j) {
        char16_t c = value[j];
        if ((c < 0x0020 && c != 0x0009 && c != 0x000A && c != 0x000D) ||
            c == 0xFFFE || c == 0xFFFF)
        {
            handle.writeString(convertToUtf8(value.substr(i, j - i)));
            handle.writeString("<unicode oor:scalar=\"");
            handle.writeString(OString::number(c));
            handle.writeString("\"/>");
            i = j + 1;
        } else if (c == '\x0D') {
            handle.writeString(convertToUtf8(value.substr(i, j - i)));
            handle.writeString("&#xD;");
            i = j + 1;
        } else if (c == '&') {
            handle.writeString(convertToUtf8(value.substr(i, j - i)));
            handle.writeString("&amp;");
            i = j + 1;
        } else if (c == '<') {
            handle.writeString(convertToUtf8(value.substr(i, j - i)));
            handle.writeString("&lt;");
            i = j + 1;
        } else if (c == '>') {
            // "MUST, for compatibility, be escaped [...] when it appears in the
            // string ']]>'":
            handle.writeString(convertToUtf8(value.substr(i, j - i)));
            handle.writeString("&gt;");
            i = j + 1;
        }
    }
    handle.writeString(convertToUtf8(value.substr(i, j - i)));
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
            "cannot create directory " + dir);
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
            "cannot create temporary file in " + dir);
    }
    tmp.writeString(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<oor:items"
        " xmlns:oor=\"http://openoffice.org/2001/registry\""
        " xmlns:xs=\"http://www.w3.org/2001/XMLSchema\""
        " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
    //TODO: Do not write back information about those removed items that did not
    // come from the .xcs/.xcu files, anyway (but had been added dynamically
    // instead):

    // For profilesafemode it is necessary to detect changes in the
    // registrymodifications file, this is done based on file size in bytes and crc32.
    // Unfortunately this write is based on writing unordered map entries, which creates
    // valid and semantically equal XML-Files, bubt with different crc32 checksums. For
    // the future usage it will be preferable to have easily comparable config files
    // which is guaranteed by writing the entries in sorted order. Indeed with this change
    // (and in the recursive writeModifications call) the same config files get written

    // copy configmgr::Modifications::Node's to a sortable list. Use pointers
    // to just reference the data instead of copying it
    std::vector< const ModNodePairEntry* > ModNodePairEntryVector;
    ModNodePairEntryVector.reserve(data.modifications.getRoot().children.size());

    for (const auto& rCand : data.modifications.getRoot().children)
    {
        ModNodePairEntryVector.push_back(&rCand);
    }

    // sort the list
    std::sort(ModNodePairEntryVector.begin(), ModNodePairEntryVector.end(), PairEntrySorter());

    // now use the list to write entries in sorted order
    // instead of random as from the unordered map
    for (const auto& j : ModNodePairEntryVector)
    {
        writeModifications(
            components, tmp, u"", rtl::Reference< Node >(), j->first,
            data.getComponents().findNode(Data::NO_LAYER, j->first),
            j->second);
    }
    tmp.writeString("</oor:items>\n");
    tmp.closeAndRename(url);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
