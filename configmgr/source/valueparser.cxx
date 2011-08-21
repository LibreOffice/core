/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
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
*
************************************************************************/

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "comphelper/sequenceasvector.hxx"
#include "osl/diagnose.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/ustring.h"
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

namespace css = com::sun::star;

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

bool parseValue(xmlreader::Span const & text, sal_Bool * value) {
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

bool parseValue(xmlreader::Span const & text, sal_Int16 * value) {
    OSL_ASSERT(text.is() && value != 0);
    // For backwards compatibility, support hexadecimal values:
    sal_Int32 n =
        rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
            text.begin, text.length, RTL_CONSTASCII_STRINGPARAM("0X"),
            RTL_CONSTASCII_LENGTH("0X")) == 0 ?
        rtl::OString(
            text.begin + RTL_CONSTASCII_LENGTH("0X"),
            text.length - RTL_CONSTASCII_LENGTH("0X")).toInt32(16) :
        rtl::OString(text.begin, text.length).toInt32();
        //TODO: check valid lexical representation
    if (n >= SAL_MIN_INT16 && n <= SAL_MAX_INT16) {
        *value = static_cast< sal_Int16 >(n);
        return true;
    }
    return false;
}

bool parseValue(xmlreader::Span const & text, sal_Int32 * value) {
    OSL_ASSERT(text.is() && value != 0);
    // For backwards compatibility, support hexadecimal values:
    *value =
        rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
            text.begin, text.length, RTL_CONSTASCII_STRINGPARAM("0X"),
            RTL_CONSTASCII_LENGTH("0X")) == 0 ?
        rtl::OString(
            text.begin + RTL_CONSTASCII_LENGTH("0X"),
            text.length - RTL_CONSTASCII_LENGTH("0X")).toInt32(16) :
        rtl::OString(text.begin, text.length).toInt32();
        //TODO: check valid lexical representation
    return true;
}

bool parseValue(xmlreader::Span const & text, sal_Int64 * value) {
    OSL_ASSERT(text.is() && value != 0);
    // For backwards compatibility, support hexadecimal values:
    *value =
        rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
            text.begin, text.length, RTL_CONSTASCII_STRINGPARAM("0X"),
            RTL_CONSTASCII_LENGTH("0X")) == 0 ?
        rtl::OString(
            text.begin + RTL_CONSTASCII_LENGTH("0X"),
            text.length - RTL_CONSTASCII_LENGTH("0X")).toInt64(16) :
        rtl::OString(text.begin, text.length).toInt64();
        //TODO: check valid lexical representation
    return true;
}

bool parseValue(xmlreader::Span const & text, double * value) {
    OSL_ASSERT(text.is() && value != 0);
    *value = rtl::OString(text.begin, text.length).toDouble();
        //TODO: check valid lexical representation
    return true;
}

bool parseValue(xmlreader::Span const & text, rtl::OUString * value) {
    OSL_ASSERT(text.is() && value != 0);
    *value = text.convertFromUtf8();
    return true;
}

bool parseValue(
    xmlreader::Span const & text, css::uno::Sequence< sal_Int8 > * value)
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

template< typename T > css::uno::Any parseSingleValue(
    xmlreader::Span const & text)
{
    T val;
    if (!parseValue(text, &val)) {
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid value")),
            css::uno::Reference< css::uno::XInterface >());
    }
    return css::uno::makeAny(val);
}

template< typename T > css::uno::Any parseListValue(
    rtl::OString const & separator, xmlreader::Span const & text)
{
    comphelper::SequenceAsVector< T > seq;
    xmlreader::Span sep;
    if (separator.getLength() == 0) {
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

css::uno::Any parseValue(
    rtl::OString const & separator, xmlreader::Span const & text, Type type)
{
    switch (type) {
    case TYPE_ANY:
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid value of type any")),
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
        return parseSingleValue< rtl::OUString >(text);
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
        return parseListValue< rtl::OUString >(separator, text);
    case TYPE_HEXBINARY_LIST:
        return parseListValue< css::uno::Sequence< sal_Int8 > >(
            separator, text);
    default:
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            css::uno::Reference< css::uno::XInterface >());
    }
}

}

ValueParser::ValueParser(int layer): layer_(layer) {}

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
                 separator_.getLength() != 0)
                ? xmlreader::XmlReader::TEXT_RAW
                : xmlreader::XmlReader::TEXT_NORMALIZED;
        default:
            break;
        }
    }
    return xmlreader::XmlReader::TEXT_NONE;
}

bool ValueParser::startElement(
    xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name)
{
    if (!node_.is()) {
        return false;
    }
    switch (state_) {
    case STATE_TEXT:
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("it")) &&
            isListType(type_) && separator_.getLength() == 0)
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
            name.equals(RTL_CONSTASCII_STRINGPARAM("unicode")) &&
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
                    attrLn.equals(RTL_CONSTASCII_STRINGPARAM("scalar")))
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
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad unicode scalar attribute in ")) +
                     reader.getUrl()),
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
         name.convertFromUtf8() +
         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) + reader.getUrl()),
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
                                new LocalizedValueNode(layer_, value)));
                    } else {
                        dynamic_cast< LocalizedValueNode * >(i->second.get())->
                            setValue(layer_, value);
                    }
                }
                break;
            default:
                OSL_ASSERT(false); // this cannot happen
                break;
            }
            separator_ = rtl::OString();
            node_.clear();
        }
        break;
    case STATE_TEXT_UNICODE:
    case STATE_IT_UNICODE:
        state_ = State(state_ - 1);
        break;
    case STATE_IT:
        items_.push_back(
            parseValue(rtl::OString(), pad_.get(), elementType(type_)));
        pad_.clear();
        state_ = STATE_TEXT;
        break;
    }
    return true;
}

void ValueParser::characters(xmlreader::Span const & text) {
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

int ValueParser::getLayer() const {
    return layer_;
}

template< typename T > css::uno::Any ValueParser::convertItems() {
    css::uno::Sequence< T > seq(items_.size());
    for (sal_Int32 i = 0; i < seq.getLength(); ++i) {
        OSL_VERIFY(items_[i] >>= seq[i]);
    }
    return css::uno::makeAny(seq);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
