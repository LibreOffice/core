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
#include <set>

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "comphelper/sequenceasvector.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "xmlreader/span.hxx"
#include "xmlreader/xmlreader.hxx"

#include "localizedvaluenode.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "parsemanager.hxx"
#include "propertynode.hxx"
#include "type.hxx"
#include "valueparser.hxx"
#include "xmldata.hxx"

namespace configmgr {

namespace {

bool parseHexDigit(char c, int * value) {
    assert(value != 0);
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
    assert(text.is() && value != 0);
    if (text.equals("true") || text.equals("1")) {
        *value = true;
        return true;
    }
    if (text.equals("false") || text.equals("0")) {
        *value = false;
        return true;
    }
    return false;
}

bool parseValue(xmlreader::Span const & text, sal_Int16 * value) {
    assert(text.is() && value != 0);
    // For backwards compatibility, support hexadecimal values:
    sal_Int32 n =
        rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
            text.begin, text.length, RTL_CONSTASCII_STRINGPARAM("0X"),
            RTL_CONSTASCII_LENGTH("0X")) == 0 ?
        static_cast< sal_Int32 >(
            OString(
                text.begin + RTL_CONSTASCII_LENGTH("0X"),
                text.length - RTL_CONSTASCII_LENGTH("0X")).toUInt32(16)) :
        OString(text.begin, text.length).toInt32();
        //TODO: check valid lexical representation
    if (n >= SAL_MIN_INT16 && n <= SAL_MAX_INT16) {
        *value = static_cast< sal_Int16 >(n);
        return true;
    }
    return false;
}

bool parseValue(xmlreader::Span const & text, sal_Int32 * value) {
    assert(text.is() && value != 0);
    // For backwards compatibility, support hexadecimal values:
    *value =
        rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
            text.begin, text.length, RTL_CONSTASCII_STRINGPARAM("0X"),
            RTL_CONSTASCII_LENGTH("0X")) == 0 ?
        static_cast< sal_Int32 >(
            OString(
                text.begin + RTL_CONSTASCII_LENGTH("0X"),
                text.length - RTL_CONSTASCII_LENGTH("0X")).toUInt32(16)) :
        OString(text.begin, text.length).toInt32();
        //TODO: check valid lexical representation
    return true;
}

bool parseValue(xmlreader::Span const & text, sal_Int64 * value) {
    assert(text.is() && value != 0);
    // For backwards compatibility, support hexadecimal values:
    *value =
        rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
            text.begin, text.length, RTL_CONSTASCII_STRINGPARAM("0X"),
            RTL_CONSTASCII_LENGTH("0X")) == 0 ?
        static_cast< sal_Int64 >(
            OString(
                text.begin + RTL_CONSTASCII_LENGTH("0X"),
                text.length - RTL_CONSTASCII_LENGTH("0X")).toUInt64(16)) :
        OString(text.begin, text.length).toInt64();
        //TODO: check valid lexical representation
    return true;
}

bool parseValue(xmlreader::Span const & text, double * value) {
    assert(text.is() && value != 0);
    *value = OString(text.begin, text.length).toDouble();
        //TODO: check valid lexical representation
    return true;
}

bool parseValue(xmlreader::Span const & text, OUString * value) {
    assert(text.is() && value != 0);
    *value = text.convertFromUtf8();
    return true;
}

bool parseValue(
    xmlreader::Span const & text, css::uno::Sequence< sal_Int8 > * value)
{
    assert(text.is() && value != 0);
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

template< typename T > css::uno::Any parseSingleValue(
    xmlreader::Span const & text)
{
    T val;
    if (!parseValue(text, &val)) {
        throw css::uno::RuntimeException(
            "invalid value", css::uno::Reference< css::uno::XInterface >());
    }
    return css::uno::makeAny(val);
}

template< typename T > css::uno::Any parseListValue(
    OString const & separator, xmlreader::Span const & text)
{
    comphelper::SequenceAsVector< T > seq;
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
                throw css::uno::RuntimeException(
                    "invalid value",
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

css::uno::Any parseValue(
    OString const & separator, xmlreader::Span const & text, Type type)
{
    switch (type) {
    case TYPE_ANY:
        throw css::uno::RuntimeException(
            "invalid value of type any",
            css::uno::Reference< css::uno::XInterface >());
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
        throw css::uno::RuntimeException(
            "this cannot happen",
            css::uno::Reference< css::uno::XInterface >());
    }
}

}

ValueParser::ValueParser(int layer): layer_(layer), state_() {}

ValueParser::~ValueParser() {}

xmlreader::XmlReader::Text ValueParser::getTextMode() const {
    if (node_.is()) {
        switch (state_) {
        case STATE_TEXT:
            if (!items_.empty()) {
                break;
            }
            // fall through
        case STATE_IT:
            return
                (type_ == TYPE_STRING || type_ == TYPE_STRING_LIST ||
                 !separator_.isEmpty())
                ? xmlreader::XmlReader::TEXT_RAW
                : xmlreader::XmlReader::TEXT_NORMALIZED;
        default:
            break;
        }
    }
    return xmlreader::XmlReader::TEXT_NONE;
}

bool ValueParser::startElement(
    xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name,
    std::set< OUString > const *)
{
    if (!node_.is()) {
        return false;
    }
    switch (state_) {
    case STATE_TEXT:
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE && name.equals("it") &&
            isListType(type_) && separator_.isEmpty())
        {
            pad_.clear();
                // before first <it>, characters are not ignored; assume they
                // are only whitespace
            state_ = STATE_IT;
            return true;
        }
        // fall through
    case STATE_IT:
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
            name.equals("unicode") &&
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
                    attrLn.equals("scalar"))
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
                    "bad unicode scalar attribute in " + reader.getUrl(),
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
        "bad member <" + name.convertFromUtf8() + "> in " + reader.getUrl(),
        css::uno::Reference< css::uno::XInterface >());
}

bool ValueParser::endElement() {
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
                    value = convertItems< OUString >();
                    break;
                case TYPE_HEXBINARY_LIST:
                    value = convertItems< css::uno::Sequence< sal_Int8 > >();
                    break;
                default:
                    assert(false); // this cannot happen
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
                    NodeMap & members = node_->getMembers();
                    NodeMap::iterator i(members.find(localizedName_));
                    if (i == members.end()) {
                        members.insert(
                            NodeMap::value_type(
                                localizedName_,
                                new LocalizedValueNode(layer_, value)));
                    } else {
                        dynamic_cast< LocalizedValueNode * >(i->second.get())->
                            setValue(layer_, value);
                    }
                }
                break;
            default:
                assert(false); // this cannot happen
                break;
            }
            separator_ = OString();
            node_.clear();
        }
        break;
    case STATE_TEXT_UNICODE:
    case STATE_IT_UNICODE:
        state_ = State(state_ - 1);
        break;
    case STATE_IT:
        items_.push_back(
            parseValue(OString(), pad_.get(), elementType(type_)));
        pad_.clear();
        state_ = STATE_TEXT;
        break;
    }
    return true;
}

void ValueParser::characters(xmlreader::Span const & text) {
    if (node_.is()) {
        assert(state_ == STATE_TEXT || state_ == STATE_IT);
        pad_.add(text.begin, text.length);
    }
}

void ValueParser::start(
    rtl::Reference< Node > const & node, OUString const & localizedName)
{
    assert(node.is() && !node_.is());
    node_ = node;
    localizedName_ = localizedName;
    state_ = STATE_TEXT;
}

int ValueParser::getLayer() const {
    return layer_;
}

template< typename T > css::uno::Any ValueParser::convertItems() {
    css::uno::Sequence< T > seq(items_.size());
    for (sal_Int32 i = 0; i < seq.getLength(); ++i) {
        bool ok = (items_[i] >>= seq[i]);
        assert(ok);
        (void) ok; // avoid warnings
    }
    return css::uno::makeAny(seq);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
