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


#include <string.h>
#include <ne_xml.h>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include "UCBDeadPropertyValue.hxx"

using namespace webdav_ucp;
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
const rtl::OUString UCBDeadPropertyValue::aTypeString("string");
const rtl::OUString UCBDeadPropertyValue::aTypeLong("long");
const rtl::OUString UCBDeadPropertyValue::aTypeShort("short");
const rtl::OUString UCBDeadPropertyValue::aTypeBoolean("boolean");
const rtl::OUString UCBDeadPropertyValue::aTypeChar("char");
const rtl::OUString UCBDeadPropertyValue::aTypeByte("byte");
const rtl::OUString UCBDeadPropertyValue::aTypeHyper("hyper");
const rtl::OUString UCBDeadPropertyValue::aTypeFloat("float");
const rtl::OUString UCBDeadPropertyValue::aTypeDouble("double");

// static
const rtl::OUString UCBDeadPropertyValue::aXMLPre("<ucbprop><type>");
const rtl::OUString UCBDeadPropertyValue::aXMLMid("</type><value>");
const rtl::OUString UCBDeadPropertyValue::aXMLEnd("</value></ucbprop>");

#define STATE_TOP (1)

#define STATE_UCBPROP   (STATE_TOP)
#define STATE_TYPE      (STATE_TOP + 1)
#define STATE_VALUE     (STATE_TOP + 2)

//////////////////////////////////////////////////////////////////////////
extern "C" int UCBDeadPropertyValue_startelement_callback(
    void *,
    int parent,
    const char * /*nspace*/,
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
    return aResult.makeStringAndClear();
}

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
                OSL_FAIL( "UCBDeadPropertyValue::decodeValue - syntax error!" );
                return rtl::OUString();
            }

            c = pValue[ nPos ];

            if ( 'p' == c )
            {
                // %per;

                if ( nPos > nEnd - 4 )
                {
                    OSL_FAIL( "UCBDeadPropertyValue::decodeValue - syntax error!" );
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
                    OSL_FAIL( "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return rtl::OUString();
                }
            }
            else if ( 'l' == c )
            {
                // %lt;

                if ( nPos > nEnd - 3 )
                {
                    OSL_FAIL( "UCBDeadPropertyValue::decodeValue - syntax error!" );
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
                    OSL_FAIL( "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return rtl::OUString();
                }
            }
            else if ( 'g' == c )
            {
                // %gt;

                if ( nPos > nEnd - 3 )
                {
                    OSL_FAIL( "UCBDeadPropertyValue::decodeValue - syntax error!" );
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
                    OSL_FAIL( "UCBDeadPropertyValue::decodeValue - syntax error!" );
                    return rtl::OUString();
                }
            }
            else
            {
                OSL_FAIL( "UCBDeadPropertyValue::decodeValue - syntax error!" );
                return rtl::OUString();
            }
        }
        else
            aResult.append( c );

        nPos++;
    }

    return aResult.makeStringAndClear();
}

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
bool UCBDeadPropertyValue::createFromXML( const rtl::OString & rInData,
                                          uno::Any & rOutData )
{
    bool success = false;

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
                            rtl::OUString("true") ) )
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
                    OSL_FAIL( "UCBDeadPropertyValue::createFromXML - "
                                "Unsupported property type!" );
                    success = false;
                }
            }
            else
                success = false;
        }
    }

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
        OSL_FAIL( "UCBDeadPropertyValue::toXML - "
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
