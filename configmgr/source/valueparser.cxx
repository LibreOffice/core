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

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/sequence.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/math.h>
#include <rtl/string.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <xmlreader/span.hxx>
#include <xmlreader/xmlreader.hxx>

#include "data.hxx"
#include "localizedvaluenode.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "parsemanager.hxx"
#include "propertynode.hxx"
#include "type.hxx"
#include "valueparser.hxx"

namespace configmgr {

namespace {

bool parseHexDigit(char c, int * value) {
    assert(value != nullptr);
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

bool parseValue(xmlreader::Span const & text, sal_Bool * value) {
    assert(text.is() && value != nullptr);
    if (text == "true" || text == "1") {
        *value = true;
        return true;
    }
    if (text == "false" || text == "0") {
        *value = false;
        return true;
    }
    return false;
}

bool parseValue(xmlreader::Span const & text, sal_Int16 * value) {
    assert(text.is() && value != nullptr);
    // For backwards compatibility, support hexadecimal values:
    bool bStartWithHexPrefix =
        rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
            text.begin, text.length, RTL_CONSTASCII_STRINGPARAM("0X"),
            RTL_CONSTASCII_LENGTH("0X")) == 0;
    sal_Int32 n;
    if (bStartWithHexPrefix)
    {
        std::string_view sView(
                text.begin + RTL_CONSTASCII_LENGTH("0X"),
                text.length - RTL_CONSTASCII_LENGTH("0X"));
        n = o3tl::toUInt32(sView, 16);
    }
    else
        n = o3tl::toInt32(std::string_view(text.begin, text.length));
    //TODO: check valid lexical representation
    if (n >= SAL_MIN_INT16 && n <= SAL_MAX_INT16) {
        *value = static_cast< sal_Int16 >(n);
        return true;
    }
    return false;
}

bool parseValue(xmlreader::Span const & text, sal_Int32 * value) {
    assert(text.is() && value != nullptr);
    // For backwards compatibility, support hexadecimal values:
    bool bStartWithHexPrefix = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
            text.begin, text.length, RTL_CONSTASCII_STRINGPARAM("0X"),
            RTL_CONSTASCII_LENGTH("0X")) == 0;

    if (bStartWithHexPrefix)
    {
        std::string_view sView(text.begin + RTL_CONSTASCII_LENGTH("0X"),
                text.length - RTL_CONSTASCII_LENGTH("0X"));
        *value = static_cast< sal_Int32 >(o3tl::toUInt32(sView, 16));
    }
    else
    {
        std::string_view sView(text.begin, text.length);
        *value = o3tl::toInt32(sView);
    }
    //TODO: check valid lexical representation
    return true;
}

bool parseValue(xmlreader::Span const & text, sal_Int64 * value) {
    assert(text.is() && value != nullptr);
    // For backwards compatibility, support hexadecimal values:
    bool bStartWithHexPrefix =
        rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
            text.begin, text.length, RTL_CONSTASCII_STRINGPARAM("0X"),
            RTL_CONSTASCII_LENGTH("0X")) == 0;
    if (bStartWithHexPrefix)
    {
        OString sSuffix(
                text.begin + RTL_CONSTASCII_LENGTH("0X"),
                text.length - RTL_CONSTASCII_LENGTH("0X"));
        *value = static_cast< sal_Int64 >(sSuffix.toUInt64(16));
    }
    else *value = o3tl::toInt64(std::string_view(text.begin, text.length));
    //TODO: check valid lexical representation
    return true;
}

bool parseValue(xmlreader::Span const & text, double * value) {
    assert(text.is() && value != nullptr);
    *value = rtl_math_stringToDouble(
        text.begin, text.begin + text.length, '.', 0, nullptr, nullptr);
        //TODO: check valid lexical representation
    return true;
}

bool parseValue(xmlreader::Span const & text, OUString * value) {
    assert(text.is() && value != nullptr);
    *value = text.convertFromUtf8();
    return true;
}

bool parseValue(
    xmlreader::Span const & text, css::uno::Sequence< sal_Int8 > * value)
{
    assert(text.is() && value != nullptr);
    if ((text.length & 1) != 0) {
        return false;
    }
    std::vector< sal_Int8 > seq;
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
    *value = comphelper::containerToSequence(seq);
    return true;
}

template< typename T > css::uno::Any parseSingleValue(
    xmlreader::Span const & text)
{
    T val;
    if (!parseValue(text, &val)) {
        throw css::uno::RuntimeException(u"invalid value"_ustr);
    }
    return css::uno::Any(val);
}

template< typename T > css::uno::Any parseListValue(
    OString const & separator, xmlreader::Span const & text)
{
    std::vector< T > seq;
    xmlreader::Span sep;
    if (separator.isEmpty()) {
        sep = xmlreader::Span(RTL_CONSTASCII_STRINGPARAM(" "));
    } else {
        sep = xmlreader::Span(separator.getStr(), separator.getLength());
    }
    if (text.length != 0) {
        for (xmlreader::Span t(text);;) {
            sal_Int32 i = rtl_str_indexOfStr_WithLength(
                t.begin, t.length, sep.begin, sep.length);
            T val;
            if (!parseValue(
                    xmlreader::Span(t.begin, i == -1 ? t.length : i), &val))
            {
                throw css::uno::RuntimeException(u"invalid value"_ustr);
            }
            seq.push_back(val);
            if (i < 0) {
                break;
            }
            t.begin += i + sep.length;
            t.length -= i + sep.length;
        }
    }
    return css::uno::Any(comphelper::containerToSequence(seq));
}

css::uno::Any parseValue(
    OString const & separator, xmlreader::Span const & text, Type type)
{
    switch (type) {
    case TYPE_ANY:
        throw css::uno::RuntimeException(u"invalid value of type any"_ustr);
    case TYPE_BOOLEAN:
        return parseSingleValue< sal_Bool >(text);
    case TYPE_SHORT:
        return parseSingleValue< sal_Int16 >(text);
    case TYPE_INT:
        return parseSingleValue< sal_Int32 >(text);
    case TYPE_LONG:
        return parseSingleValue< sal_Int64 >(text);
    case TYPE_DOUBLE:
        return parseSingleValue< double >(text);
    case TYPE_STRING:
        return parseSingleValue< OUString >(text);
    case TYPE_HEXBINARY:
        return parseSingleValue< css::uno::Sequence< sal_Int8 > >(text);
    case TYPE_BOOLEAN_LIST:
        return parseListValue< sal_Bool >(separator, text);
    case TYPE_SHORT_LIST:
        return parseListValue< sal_Int16 >(separator, text);
    case TYPE_INT_LIST:
        return parseListValue< sal_Int32 >(separator, text);
    case TYPE_LONG_LIST:
        return parseListValue< sal_Int64 >(separator, text);
    case TYPE_DOUBLE_LIST:
        return parseListValue< double >(separator, text);
    case TYPE_STRING_LIST:
        return parseListValue< OUString >(separator, text);
    case TYPE_HEXBINARY_LIST:
        return parseListValue< css::uno::Sequence< sal_Int8 > >(
            separator, text);
    default:
        assert(false);
        throw css::uno::RuntimeException(u"this cannot happen"_ustr);
    }
}

}

ValueParser::ValueParser(int layer): type_(TYPE_ERROR), layer_(layer), state_() {}

ValueParser::~ValueParser() {}

xmlreader::XmlReader::Text ValueParser::getTextMode() const {
    if (!node_)
        return xmlreader::XmlReader::Text::NONE;

    switch (state_) {
    case State::Text:
        if (!items_.empty()) {
            break;
        }
        [[fallthrough]];
    case State::IT:
        return
            (type_ == TYPE_STRING || type_ == TYPE_STRING_LIST ||
             !separator_.isEmpty())
            ? xmlreader::XmlReader::Text::Raw
            : xmlreader::XmlReader::Text::Normalized;
    default:
        break;
    }
    return xmlreader::XmlReader::Text::NONE;
}

bool ValueParser::startElement(
    xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name)
{
    if (!node_.is()) {
        return false;
    }
    switch (state_) {
    case State::Text:
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE && name == "it" &&
            isListType(type_) && separator_.isEmpty())
        {
            pad_.clear();
                // before first <it>, characters are not ignored; assume they
                // are only whitespace
            state_ = State::IT;
            return true;
        }
        [[fallthrough]];
    case State::IT:
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
            name == "unicode" &&
            (type_ == TYPE_STRING || type_ == TYPE_STRING_LIST))
        {
            sal_Int32 scalar = -1;
            for (;;) {
                int attrNsId;
                xmlreader::Span attrLn;
                if (!reader.nextAttribute(&attrNsId, &attrLn)) {
                    break;
                }
                if (attrNsId == ParseManager::NAMESPACE_OOR &&
                    attrLn == "scalar")
                {
                    if (!parseValue(reader.getAttributeValue(true), &scalar)) {
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
                    "bad unicode scalar attribute in " + reader.getUrl());
            }
            state_ = state_ == State::Text ? State::TextUnicode : State::ITUnicode;
            return true;
        }
        break;
    default:
        break;
    }
    throw css::uno::RuntimeException(
        "bad member <" + name.convertFromUtf8() + "> in " + reader.getUrl());
}

bool ValueParser::endElement() {
    if (!node_.is()) {
        return false;
    }
    switch (state_) {
    case State::Text:
        {
            css::uno::Any *pValue = nullptr;

            switch (node_->kind()) {
            case Node::KIND_PROPERTY:
                pValue = static_cast< PropertyNode * >(node_.get())->getValuePtr(layer_, layer_ == Data::NO_LAYER);
                break;
            case Node::KIND_LOCALIZED_PROPERTY:
                {
                    NodeMap & members = node_->getMembers();
                    auto [i, bInserted] = members.insert(NodeMap::value_type(localizedName_, nullptr));
                    LocalizedValueNode *pLVNode;
                    if (bInserted) {
                        pLVNode = new LocalizedValueNode(layer_);
                        i->second = pLVNode;
                    } else {
                        pLVNode = static_cast< LocalizedValueNode * >(i->second.get());
                    }
                    pValue = pLVNode->getValuePtr(layer_, layer_ == Data::NO_LAYER);
                }
                break;
            default:
                assert(false); // this cannot happen
                return false;
            }

            if (items_.empty()) {
                *pValue = parseValue(separator_, pad_.get(), type_);
                pad_.clear();
            } else {
                switch (type_) {
                case TYPE_BOOLEAN_LIST:
                    *pValue = convertItems< sal_Bool >();
                    break;
                case TYPE_SHORT_LIST:
                    *pValue = convertItems< sal_Int16 >();
                    break;
                case TYPE_INT_LIST:
                    *pValue = convertItems< sal_Int32 >();
                    break;
                case TYPE_LONG_LIST:
                    *pValue = convertItems< sal_Int64 >();
                    break;
                case TYPE_DOUBLE_LIST:
                    *pValue = convertItems< double >();
                    break;
                case TYPE_STRING_LIST:
                    *pValue = convertItems< OUString >();
                    break;
                case TYPE_HEXBINARY_LIST:
                    *pValue = convertItems< css::uno::Sequence< sal_Int8 > >();
                    break;
                default:
                    assert(false); // this cannot happen
                    break;
                }
                items_.clear();
            }
            separator_.clear();
            node_.clear();
        }
        break;
    case State::TextUnicode:
        state_ = State::Text;
        break;
    case State::ITUnicode:
        state_ = State::IT;
        break;
    case State::IT:
        items_.push_back(
            parseValue(OString(), pad_.get(), elementType(type_)));
        pad_.clear();
        state_ = State::Text;
        break;
    }
    return true;
}

void ValueParser::characters(xmlreader::Span const & text) {
    if (node_.is()) {
        assert(state_ == State::Text || state_ == State::IT);
        pad_.add(text.begin, text.length);
    }
}

void ValueParser::start(
    rtl::Reference< Node > const & node, OUString const & localizedName)
{
    assert(node.is() && !node_.is());
    node_ = node;
    localizedName_ = localizedName;
    state_ = State::Text;
}


template< typename T > css::uno::Any ValueParser::convertItems() {
    css::uno::Sequence< T > seq(items_.size());
    auto seqRange = asNonConstRange(seq);
    for (sal_Int32 i = 0; i < seq.getLength(); ++i) {
        bool ok = (items_[i] >>= seqRange[i]);
        assert(ok);
        (void) ok; // avoid warnings
    }
    return css::uno::Any(seq);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
