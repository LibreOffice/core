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

#include <string.h>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include "UCBDeadPropertyValue.hxx"

using namespace http_dav_ucp;
using namespace ::com::sun::star;


// static
const OUString UCBDeadPropertyValue::aTypeString
    = "string";
const OUString UCBDeadPropertyValue::aTypeLong
    = "long";
const OUString UCBDeadPropertyValue::aTypeShort
    = "short";
const OUString UCBDeadPropertyValue::aTypeBoolean
    = "boolean";
const OUString UCBDeadPropertyValue::aTypeChar
    = "char";
const OUString UCBDeadPropertyValue::aTypeByte
    = "byte";
const OUString UCBDeadPropertyValue::aTypeHyper
    = "hyper";
const OUString UCBDeadPropertyValue::aTypeFloat
    = "float";
const OUString UCBDeadPropertyValue::aTypeDouble
    = "double";

// static
const OUString UCBDeadPropertyValue::aXMLPre
    = "<ucbprop><type>";
const OUString UCBDeadPropertyValue::aXMLMid
    = "</type><value>";
const OUString UCBDeadPropertyValue::aXMLEnd
    = "</value></ucbprop>";

/*

#define STATE_TOP (1)

#define STATE_UCBPROP   (STATE_TOP)
#define STATE_TYPE      (STATE_TOP + 1)
#define STATE_VALUE     (STATE_TOP + 2)

extern "C" int UCBDeadPropertyValue_startelement_callback(
    void *,
    int parent,
    const char * nspace,
    const char *name,
    const char ** )
{
    if ( name != 0 )
    {
        switch ( parent )
        {
            case NE_XML_STATEROOT:
                if ( strcmp( name, "ucbprop" ) == 0 )
                    return STATE_UCBPROP;
                break;

            case STATE_UCBPROP:
                if ( strcmp( name, "type" ) == 0 )
                    return STATE_TYPE;
                else if ( strcmp( name, "value" ) == 0 )
                    return STATE_VALUE;
                break;
        }
    }
    return NE_XML_DECLINE;
}


extern "C" int UCBDeadPropertyValue_chardata_callback(
    void *userdata,
    int state,
    const char *buf,
    size_t len )
{
    UCBDeadPropertyValueParseContext * pCtx
            = static_cast< UCBDeadPropertyValueParseContext * >( userdata );

    switch ( state )
    {
        case STATE_TYPE:
            SAL_WARN_IF( pCtx->pType, "ucb.ucp.webdav",
                        "UCBDeadPropertyValue_endelement_callback - "
                        "Type already set!" );
            pCtx->pType
                = new OUString( buf, len, RTL_TEXTENCODING_ASCII_US );
            break;

        case STATE_VALUE:
            SAL_WARN_IF( pCtx->pValue, "ucb.ucp.webdav",
                        "UCBDeadPropertyValue_endelement_callback - "
                        "Value already set!" );
            pCtx->pValue
                = new OUString( buf, len, RTL_TEXTENCODING_ASCII_US );
            break;
    }
    return 0; // zero to continue, non-zero to abort parsing
}


extern "C" int UCBDeadPropertyValue_endelement_callback(
    void *userdata,
    int state,
    const char *,
    const char * )
{
    UCBDeadPropertyValueParseContext * pCtx
            = static_cast< UCBDeadPropertyValueParseContext * >( userdata );

    switch ( state )
    {
        case STATE_TYPE:
            if ( !pCtx->pType )
                return 1; // abort
            break;

        case STATE_VALUE:
            if ( !pCtx->pValue )
                return 1; // abort
            break;

        case STATE_UCBPROP:
            if ( !pCtx->pType || ! pCtx->pValue )
                return 1; // abort
            break;
    }
    return 0; // zero to continue, non-zero to abort parsing
}
*/


static OUString encodeValue( const OUString & rValue )
{
    // Note: I do not use the usual &amp; + &lt; + &gt; encoding, because
    //       I want to prevent any XML parser from trying to 'understand'
    //       the value. This caused problems:

    //       Example:
    //       - Unencoded property value: x<z
    //       PROPPATCH:
    //       - Encoded property value: x&lt;z
    //       - UCBDeadPropertyValue::toXML result:
    //              <ucbprop><type>string</type><value>x&lt;z</value></ucbprop>
    //       PROPFIND:
    //       - parser replaces &lt; by > ==> error (not well formed)

    OUStringBuffer aResult;
    const sal_Unicode * pValue = rValue.getStr();

    sal_Int32 nCount = rValue.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const sal_Unicode c = pValue[ n ];

        if ( '%' == c )
            aResult.appendAscii( "%per;" );
        else if ( '<' == c )
            aResult.appendAscii( "%lt;" );
        else if ( '>' == c )
            aResult.appendAscii( "%gt;" );
        else
            aResult.append( c );
    }
    return aResult.makeStringAndClear();
}

/*

static OUString decodeValue( const OUString & rValue )
{
    OUStringBuffer aResult;
    const sal_Unicode * pValue = rValue.getStr();

    sal_Int32 nPos = 0;
    sal_Int32 nEnd = rValue.getLength();

    while ( nPos < nEnd )
    {
        sal_Unicode c = pValue[ nPos ];

        if ( '%' == c )
        {
            nPos++;

            if ( nPos == nEnd )
            {
                SAL_WARN( "ucb.ucp.webdav",
                    "UCBDeadPropertyValue::decodeValue - syntax error!" );
                return OUString();
            }

            c = pValue[ nPos ];

            if ( 'p' == c )
            {
                // %per;

                if ( nPos > nEnd - 4 )
                {
                    SAL_WARN( "ucb.ucp.webdav",
                        "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return OUString();
                }

                if ( ( 'e' == pValue[ nPos + 1 ] )
                     &&
                     ( 'r' == pValue[ nPos + 2 ] )
                     &&
                     ( ';' == pValue[ nPos + 3 ] ) )
                {
                    aResult.append( '%' );
                    nPos += 3;
                }
                else
                {
                    SAL_WARN( "ucb.ucp.webdav",
                        "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return OUString();
                }
            }
            else if ( 'l' == c )
            {
                // %lt;

                if ( nPos > nEnd - 3 )
                {
                    SAL_WARN( "ucb.ucp.webdav",
                        "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return OUString();
                }

                if ( ( 't' == pValue[ nPos + 1 ] )
                     &&
                     ( ';' == pValue[ nPos + 2 ] ) )
                {
                    aResult.append( '<' );
                    nPos += 2;
                }
                else
                {
                    SAL_WARN( "ucb.ucp.webdav",
                        "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return OUString();
                }
            }
            else if ( 'g' == c )
            {
                // %gt;

                if ( nPos > nEnd - 3 )
                {
                    SAL_WARN( "ucb.ucp.webdav",
                        "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return OUString();
                }

                if ( ( 't' == pValue[ nPos + 1 ] )
                     &&
                     ( ';' == pValue[ nPos + 2 ] ) )
                {
                    aResult.append( '>' );
                    nPos += 2;
                }
                else
                {
                    SAL_WARN( "ucb.ucp.webdav",
                        "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return OUString();
                }
            }
            else
            {
                SAL_WARN( "ucb.ucp.webdav",
                    "UCBDeadPropertyValue::decodeValue - syntax error!" );
                return OUString();
            }
        }
        else
            aResult.append( c );

        nPos++;
    }

    return OUString( aResult );
}
*/


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
bool UCBDeadPropertyValue::createFromXML( const OString & /*rInData*/,
                                          uno::Any & /*rOutData*/ )
{
    bool success = false;

    /*
    ne_xml_parser * parser = ne_xml_create();
    if ( parser )
    {
        UCBDeadPropertyValueParseContext aCtx;
        ne_xml_push_handler( parser,
                             UCBDeadPropertyValue_startelement_callback,
                             UCBDeadPropertyValue_chardata_callback,
                             UCBDeadPropertyValue_endelement_callback,
                             &aCtx );

        ne_xml_parse( parser, rInData.getStr(), rInData.getLength() );

        success = !ne_xml_failed( parser );

        ne_xml_destroy( parser );

        if ( success )
        {
            if ( aCtx.pType && aCtx.pValue )
            {
                // Decode aCtx.pValue! It may contain XML reserved chars.
                OUString aStringValue = decodeValue( *aCtx.pValue );
                if ( aCtx.pType->equalsIgnoreAsciiCase( aTypeString ) )
                {
                    rOutData <<= aStringValue;
                }
                else if ( aCtx.pType->equalsIgnoreAsciiCase( aTypeLong ) )
                {
                    rOutData <<= aStringValue.toInt32();
                }
                else if ( aCtx.pType->equalsIgnoreAsciiCase( aTypeShort ) )
                {
                    rOutData <<= sal_Int16( aStringValue.toInt32() );
                }
                else if ( aCtx.pType->equalsIgnoreAsciiCase( aTypeBoolean ) )
                {
                    if ( aStringValue.equalsIgnoreAsciiCase(
                            OUString( "true" ) ) )
                        rOutData <<= sal_Bool( sal_True );
                    else
                        rOutData <<= sal_Bool( sal_False );
                }
                else if ( aCtx.pType->equalsIgnoreAsciiCase( aTypeChar ) )
                {
                    rOutData <<= aStringValue.toChar();
                }
                else if ( aCtx.pType->equalsIgnoreAsciiCase( aTypeByte ) )
                {
                    rOutData <<= sal_Int8( aStringValue.toChar() );
                }
                else if ( aCtx.pType->equalsIgnoreAsciiCase( aTypeHyper ) )
                {
                    rOutData <<= aStringValue.toInt64();
                }
                else if ( aCtx.pType->equalsIgnoreAsciiCase( aTypeFloat ) )
                {
                    rOutData <<= aStringValue.toFloat();
                }
                else if ( aCtx.pType->equalsIgnoreAsciiCase( aTypeDouble ) )
                {
                    rOutData <<= aStringValue.toDouble();
                }
                else
                {
                    SAL_WARN( "ucb.ucp.webdav",
                                "UCBDeadPropertyValue::createFromXML - "
                                "Unsupported property type!" );
                    success = false;
                }
            }
            else
                success = false;
        }
    }
    */
    return success;
}


// static
bool UCBDeadPropertyValue::toXML( const uno::Any & rInData,
                                  OUString & rOutData )
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
        sal_Bool bValue = false;
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
        return false;
    }

    // Encode value! It must not contain XML reserved chars!
    aStringValue = encodeValue( aStringValue );

    rOutData =  aXMLPre;
    rOutData += aStringType;
    rOutData += aXMLMid;
    rOutData += aStringValue;
    rOutData += aXMLEnd;
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
