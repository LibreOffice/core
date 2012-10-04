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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <string.h>
//#include <ne_xml.h>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include "UCBDeadPropertyValue.hxx"

using namespace http_dav_ucp;
using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////

struct UCBDeadPropertyValueParseContext
{
    rtl::OUString * pType;
    rtl::OUString * pValue;

    UCBDeadPropertyValueParseContext() : pType( 0 ), pValue( 0 ) {}
    ~UCBDeadPropertyValueParseContext() { delete pType; delete pValue; }
};

// static
const rtl::OUString UCBDeadPropertyValue::aTypeString
    = rtl::OUString::createFromAscii( "string" );
const rtl::OUString UCBDeadPropertyValue::aTypeLong
    = rtl::OUString::createFromAscii( "long" );
const rtl::OUString UCBDeadPropertyValue::aTypeShort
    = rtl::OUString::createFromAscii( "short" );
const rtl::OUString UCBDeadPropertyValue::aTypeBoolean
    = rtl::OUString::createFromAscii( "boolean" );
const rtl::OUString UCBDeadPropertyValue::aTypeChar
    = rtl::OUString::createFromAscii( "char" );
const rtl::OUString UCBDeadPropertyValue::aTypeByte
    = rtl::OUString::createFromAscii( "byte" );
const rtl::OUString UCBDeadPropertyValue::aTypeHyper
    = rtl::OUString::createFromAscii( "hyper" );
const rtl::OUString UCBDeadPropertyValue::aTypeFloat
    = rtl::OUString::createFromAscii( "float" );
const rtl::OUString UCBDeadPropertyValue::aTypeDouble
    = rtl::OUString::createFromAscii( "double" );

// static
const rtl::OUString UCBDeadPropertyValue::aXMLPre
    = rtl::OUString::createFromAscii( "<ucbprop><type>" );
const rtl::OUString UCBDeadPropertyValue::aXMLMid
    = rtl::OUString::createFromAscii( "</type><value>" );
const rtl::OUString UCBDeadPropertyValue::aXMLEnd
    = rtl::OUString::createFromAscii( "</value></ucbprop>" );

#define STATE_TOP (1)

#define STATE_UCBPROP   (STATE_TOP)
#define STATE_TYPE      (STATE_TOP + 1)
#define STATE_VALUE     (STATE_TOP + 2)

/*
//////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////////
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
            OSL_ENSURE( !pCtx->pType,
                        "UCBDeadPropertyValue_endelement_callback - "
                        "Type already set!" );
            pCtx->pType
                = new rtl::OUString( buf, len, RTL_TEXTENCODING_ASCII_US );
            break;

        case STATE_VALUE:
            OSL_ENSURE( !pCtx->pValue,
                        "UCBDeadPropertyValue_endelement_callback - "
                        "Value already set!" );
            pCtx->pValue
                = new rtl::OUString( buf, len, RTL_TEXTENCODING_ASCII_US );
            break;
    }
    return 0; // zero to continue, non-zero to abort parsing
}

//////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////////
static rtl::OUString encodeValue( const rtl::OUString & rValue )
{
    // Note: I do not use the usual &amp; + &lt; + &gt; encoding, because
    //       I want to prevent any XML parser from trying to 'understand'
    //       the value. This caused problems:
    //
    //       Example:
    //       - Unencoded property value: x<z
    //       PROPPATCH:
    //       - Encoded property value: x&lt;z
    //       - UCBDeadPropertyValue::toXML result:
    //              <ucbprop><type>string</type><value>x&lt;z</value></ucbprop>
    //       PROPFIND:
    //       - parser replaces &lt; by > ==> error (not well formed)

    rtl::OUStringBuffer aResult;
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
    return rtl::OUString( aResult );
}

/*
//////////////////////////////////////////////////////////////////////////
static rtl::OUString decodeValue( const rtl::OUString & rValue )
{
    rtl::OUStringBuffer aResult;
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
                OSL_ENSURE( sal_False,
                    "UCBDeadPropertyValue::decodeValue - syntax error!" );
                return rtl::OUString();
            }

            c = pValue[ nPos ];

            if ( 'p' == c )
            {
                // %per;

                if ( nPos > nEnd - 4 )
                {
                    OSL_ENSURE( sal_False,
                        "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return rtl::OUString();
                }

                if ( ( 'e' == pValue[ nPos + 1 ] )
                     &&
                     ( 'r' == pValue[ nPos + 2 ] )
                     &&
                     ( ';' == pValue[ nPos + 3 ] ) )
                {
                    aResult.append( sal_Unicode( '%' ) );
                    nPos += 3;
                }
                else
                {
                    OSL_ENSURE( sal_False,
                        "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return rtl::OUString();
                }
            }
            else if ( 'l' == c )
            {
                // %lt;

                if ( nPos > nEnd - 3 )
                {
                    OSL_ENSURE( sal_False,
                        "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return rtl::OUString();
                }

                if ( ( 't' == pValue[ nPos + 1 ] )
                     &&
                     ( ';' == pValue[ nPos + 2 ] ) )
                {
                    aResult.append( sal_Unicode( '<' ) );
                    nPos += 2;
                }
                else
                {
                    OSL_ENSURE( sal_False,
                        "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return rtl::OUString();
                }
            }
            else if ( 'g' == c )
            {
                // %gt;

                if ( nPos > nEnd - 3 )
                {
                    OSL_ENSURE( sal_False,
                        "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return rtl::OUString();
                }

                if ( ( 't' == pValue[ nPos + 1 ] )
                     &&
                     ( ';' == pValue[ nPos + 2 ] ) )
                {
                    aResult.append( sal_Unicode( '>' ) );
                    nPos += 2;
                }
                else
                {
                    OSL_ENSURE( sal_False,
                        "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return rtl::OUString();
                }
            }
            else
            {
                OSL_ENSURE( sal_False,
                    "UCBDeadPropertyValue::decodeValue - syntax error!" );
                return rtl::OUString();
            }
        }
        else
            aResult.append( c );

        nPos++;
    }

    return rtl::OUString( aResult );
}
*/

//////////////////////////////////////////////////////////////////////////
// static
bool UCBDeadPropertyValue::supportsType( const uno::Type & rType )
{
    if ( ( rType != getCppuType( static_cast< const rtl::OUString * >( 0 ) ) )
         &&
         ( rType != getCppuType( static_cast< const sal_Int32 * >( 0 ) ) )
         &&
         ( rType != getCppuType( static_cast< const sal_Int16 * >( 0 ) ) )
         &&
         ( rType != getCppuBooleanType() )
         &&
         ( rType != getCppuCharType() )
         &&
         ( rType != getCppuType( static_cast< const sal_Int8 * >( 0 ) ) )
         &&
         ( rType != getCppuType( static_cast< const sal_Int64 * >( 0 ) ) )
         &&
         ( rType != getCppuType( static_cast< const float * >( 0 ) ) )
         &&
         ( rType != getCppuType( static_cast< const double * >( 0 ) ) ) )
    {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
// static
bool UCBDeadPropertyValue::createFromXML( const rtl::OString & /*rInData*/,
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
                rtl::OUString aStringValue = decodeValue( *aCtx.pValue );
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
                            rtl::OUString::createFromAscii( "true" ) ) )
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
                    OSL_ENSURE( sal_False,
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

//////////////////////////////////////////////////////////////////////////
// static
bool UCBDeadPropertyValue::toXML( const uno::Any & rInData,
                                  rtl::OUString & rOutData )
{
    // <ucbprop><type>the_type</type><value>the_value</value></ucbprop>

    // Check property type. Extract type and value as string.

    const uno::Type& rType = rInData.getValueType();
    rtl::OUString aStringValue;
    rtl::OUString aStringType;

    if ( rType == getCppuType( static_cast< const rtl::OUString * >( 0 ) ) )
    {
        // string
        rInData >>= aStringValue;
        aStringType = aTypeString;
    }
    else if ( rType == getCppuType( static_cast< const sal_Int32 * >( 0 ) ) )
    {
        // long
        sal_Int32 nValue = 0;
        rInData >>= nValue;
        aStringValue = rtl::OUString::valueOf( nValue );
        aStringType = aTypeLong;
    }
    else if ( rType == getCppuType( static_cast< const sal_Int16 * >( 0 ) ) )
    {
        // short
        sal_Int32 nValue = 0;
        rInData >>= nValue;
        aStringValue = rtl::OUString::valueOf( nValue );
        aStringType = aTypeShort;
    }
    else if ( rType == getCppuBooleanType() )
    {
        // boolean
        sal_Bool bValue = false;
        rInData >>= bValue;
        aStringValue = rtl::OUString::valueOf( bValue );
        aStringType = aTypeBoolean;
    }
    else if ( rType == getCppuCharType() )
    {
        // char
        sal_Unicode cValue = 0;
        rInData >>= cValue;
        aStringValue = rtl::OUString::valueOf( cValue );
        aStringType = aTypeChar;
    }
    else if ( rType == getCppuType( static_cast< const sal_Int8 * >( 0 ) ) )
    {
        // byte
        sal_Int8 nValue = 0;
        rInData >>= nValue;
        aStringValue = rtl::OUString::valueOf( sal_Unicode( nValue ) );
        aStringType = aTypeByte;
    }
    else if ( rType == getCppuType( static_cast< const sal_Int64 * >( 0 ) ) )
    {
        // hyper
        sal_Int64 nValue = 0;
        rInData >>= nValue;
        aStringValue = rtl::OUString::valueOf( nValue );
        aStringType = aTypeHyper;
    }
    else if ( rType == getCppuType( static_cast< const float * >( 0 ) ) )
    {
        // float
        float nValue = 0;
        rInData >>= nValue;
        aStringValue = rtl::OUString::valueOf( nValue );
        aStringType = aTypeFloat;
    }
    else if ( rType == getCppuType( static_cast< const double * >( 0 ) ) )
    {
        // double
        double nValue = 0;
        rInData >>= nValue;
        aStringValue = rtl::OUString::valueOf( nValue );
        aStringType = aTypeDouble;
    }
    else
    {
        OSL_ENSURE( sal_False,
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
