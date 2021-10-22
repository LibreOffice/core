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

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include "UCBDeadPropertyValue.hxx"

using namespace http_dav_ucp;
using namespace ::com::sun::star;


// static
constexpr OUStringLiteral aTypeString = u"string";
constexpr OUStringLiteral aTypeLong = u"long";
constexpr OUStringLiteral aTypeShort = u"short";
constexpr OUStringLiteral aTypeBoolean = u"boolean";
constexpr OUStringLiteral aTypeChar = u"char";
constexpr OUStringLiteral aTypeByte = u"byte";
constexpr OUStringLiteral aTypeHyper = u"hyper";
constexpr OUStringLiteral aTypeFloat = u"float";
constexpr OUStringLiteral aTypeDouble = u"double";

// static
bool UCBDeadPropertyValue::supportsType( const uno::Type & rType )
{
    if ( ( rType != cppu::UnoType<OUString>::get() )
         &&
         ( rType != cppu::UnoType<sal_Int32>::get() )
         &&
         ( rType != cppu::UnoType<sal_Int16>::get() )
         &&
         ( rType != cppu::UnoType<bool>::get() )
         &&
         ( rType != cppu::UnoType<cppu::UnoCharType>::get() )
         &&
         ( rType != cppu::UnoType<sal_Int8>::get() )
         &&
         ( rType != cppu::UnoType<sal_Int64>::get() )
         &&
         ( rType != cppu::UnoType<float>::get() )
         &&
         ( rType != cppu::UnoType<double>::get() ) )
    {
        return false;
    }

    return true;
}


// static
bool UCBDeadPropertyValue::createFromXML(OUString const& rType,
                                         OUString const& rValue,
                                         uno::Any & rOutData)
{
    bool success = false;

    if (rType.equalsIgnoreAsciiCase(aTypeString))
    {
        rOutData <<= rValue;
    }
    else if (rType.equalsIgnoreAsciiCase(aTypeLong))
    {
        rOutData <<= rValue.toInt32();
    }
    else if (rType.equalsIgnoreAsciiCase(aTypeShort))
    {
        rOutData <<= sal_Int16( rValue.toInt32() );
    }
    else if (rType.equalsIgnoreAsciiCase(aTypeBoolean))
    {
        if (rValue.equalsIgnoreAsciiCase(u"true"))
        {
            rOutData <<= true;
        }
        else
        {
            rOutData <<= false;
        }
    }
    else if (rType.equalsIgnoreAsciiCase(aTypeChar))
    {
        rOutData <<= rValue.toChar();
    }
    else if (rType.equalsIgnoreAsciiCase(aTypeByte))
    {
        rOutData <<= sal_Int8( rValue.toChar() );
    }
    else if (rType.equalsIgnoreAsciiCase(aTypeHyper))
    {
        rOutData <<= rValue.toInt64();
    }
    else if (rType.equalsIgnoreAsciiCase(aTypeFloat))
    {
        rOutData <<= rValue.toFloat();
    }
    else if (rType.equalsIgnoreAsciiCase(aTypeDouble))
    {
        rOutData <<= rValue.toDouble();
    }
    else
    {
        SAL_WARN( "ucb.ucp.webdav",
                    "UCBDeadPropertyValue::createFromXML - "
                    "Unsupported property type!" );
        success = false;
    }
    return success;
}

// static
::std::optional<::std::pair<OUString, OUString>>
UCBDeadPropertyValue::toXML(const uno::Any & rInData)
{
    // <ucbprop><type>the_type</type><value>the_value</value></ucbprop>

    // Check property type. Extract type and value as string.

    const uno::Type& rType = rInData.getValueType();
    OUString aStringValue;
    OUString aStringType;

    if ( rType == cppu::UnoType<OUString>::get() )
    {
        // string
        rInData >>= aStringValue;
        aStringType = aTypeString;
    }
    else if ( rType == cppu::UnoType<sal_Int32>::get() )
    {
        // long
        sal_Int32 nValue = 0;
        rInData >>= nValue;
        aStringValue = OUString::number( nValue );
        aStringType = aTypeLong;
    }
    else if ( rType == cppu::UnoType<sal_Int16>::get() )
    {
        // short
        sal_Int32 nValue = 0;
        rInData >>= nValue;
        aStringValue = OUString::number( nValue );
        aStringType = aTypeShort;
    }
    else if ( rType == cppu::UnoType<bool>::get() )
    {
        // boolean
        bool bValue = false;
        rInData >>= bValue;
        aStringValue = OUString::boolean( bValue );
        aStringType = aTypeBoolean;
    }
    else if ( rType == cppu::UnoType<cppu::UnoCharType>::get() )
    {
        // char
        sal_Unicode cValue = 0;
        rInData >>= cValue;
        aStringValue = OUString( cValue );
        aStringType = aTypeChar;
    }
    else if ( rType == cppu::UnoType<sal_Int8>::get() )
    {
        // byte
        sal_Int8 nValue = 0;
        rInData >>= nValue;
        aStringValue = OUString( sal_Unicode( nValue ) );
        aStringType = aTypeByte;
    }
    else if ( rType == cppu::UnoType<sal_Int64>::get() )
    {
        // hyper
        sal_Int64 nValue = 0;
        rInData >>= nValue;
        aStringValue = OUString::number( nValue );
        aStringType = aTypeHyper;
    }
    else if ( rType == cppu::UnoType<float>::get() )
    {
        // float
        float nValue = 0;
        rInData >>= nValue;
        aStringValue = OUString::number( nValue );
        aStringType = aTypeFloat;
    }
    else if ( rType == cppu::UnoType<double>::get() )
    {
        // double
        double nValue = 0;
        rInData >>= nValue;
        aStringValue = OUString::number( nValue );
        aStringType = aTypeDouble;
    }
    else
    {
        SAL_WARN( "ucb.ucp.webdav",
                    "UCBDeadPropertyValue::toXML - "
                    "Unsupported property type!" );
        return {};
    }

    return { { aStringType, aStringValue } };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
