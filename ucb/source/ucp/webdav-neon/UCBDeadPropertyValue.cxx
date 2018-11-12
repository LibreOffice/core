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

#include <config_lgpl.h>
#include <string.h>
#include <ne_xml.h>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include "UCBDeadPropertyValue.hxx"
#include <memory>

using namespace webdav_ucp;
using namespace com::sun::star;


struct UCBDeadPropertyValueParseContext
{
    std::unique_ptr<OUString> pType;
    std::unique_ptr<OUString> pValue;

    UCBDeadPropertyValueParseContext() {}
};

static const char aTypeString[] = "string";
static const char aTypeLong[] = "long";
static const char aTypeShort[] = "short";
static const char aTypeBoolean[] = "boolean";
static const char aTypeChar[] = "char";
static const char aTypeByte[] = "byte";
static const char aTypeHyper[] = "hyper";
static const char aTypeFloat[] = "float";
static const char aTypeDouble[] = "double";

static const char aXMLPre[] = "<ucbprop><type>";
static const char aXMLMid[] = "</type><value>";
static const char aXMLEnd[] = "</value></ucbprop>";


#define STATE_TOP (1)

#define STATE_UCBPROP   (STATE_TOP)
#define STATE_TYPE      (STATE_TOP + 1)
#define STATE_VALUE     (STATE_TOP + 2)


extern "C" {

static int UCBDeadPropertyValue_startelement_callback(
    void *,
    int parent,
    const char * /*nspace*/,
    const char *name,
    const char ** )
{
    if ( name != nullptr )
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


static int UCBDeadPropertyValue_chardata_callback(
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
            assert( !pCtx->pType &&
                        "UCBDeadPropertyValue_endelement_callback - "
                        "Type already set!" );
            pCtx->pType.reset( new OUString( buf, len, RTL_TEXTENCODING_ASCII_US ) );
            break;

        case STATE_VALUE:
            assert( !pCtx->pValue &&
                        "UCBDeadPropertyValue_endelement_callback - "
                        "Value already set!" );
            pCtx->pValue.reset( new OUString( buf, len, RTL_TEXTENCODING_ASCII_US ) );
            break;
    }
    return 0; // zero to continue, non-zero to abort parsing
}


static int UCBDeadPropertyValue_endelement_callback(
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

}

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
            aResult.append( "%per;" );
        else if ( '<' == c )
            aResult.append( "%lt;" );
        else if ( '>' == c )
            aResult.append( "%gt;" );
        else
            aResult.append( c );
    }
    return aResult.makeStringAndClear();
}


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
                SAL_WARN( "ucb.ucp.webdav", "decodeValue() - syntax error!" );
                return OUString();
            }

            c = pValue[ nPos ];

            if ( 'p' == c )
            {
                // %per;

                if ( nPos > nEnd - 4 )
                {
                    SAL_WARN( "ucb.ucp.webdav", "decodeValue() - syntax error!" );
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
                    SAL_WARN( "ucb.ucp.webdav", "decodeValue() - syntax error!" );
                    return OUString();
                }
            }
            else if ( 'l' == c )
            {
                // %lt;

                if ( nPos > nEnd - 3 )
                {
                    SAL_WARN( "ucb.ucp.webdav", "decodeValue() - syntax error!" );
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
                    SAL_WARN( "ucb.ucp.webdav", "decodeValue() - syntax error!" );
                    return OUString();
                }
            }
            else if ( 'g' == c )
            {
                // %gt;

                if ( nPos > nEnd - 3 )
                {
                    SAL_WARN( "ucb.ucp.webdav", "decodeValue() - syntax error!" );
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
                    SAL_WARN( "ucb.ucp.webdav", "decodeValue() - syntax error!" );
                    return OUString();
                }
            }
            else
            {
                SAL_WARN( "ucb.ucp.webdav", "decodeValue() - syntax error!" );
                return OUString();
            }
        }
        else
            aResult.append( c );

        nPos++;
    }

    return aResult.makeStringAndClear();
}


// static
bool UCBDeadPropertyValue::supportsType( const uno::Type & rType )
{
    return rType == cppu::UnoType<OUString>::get()
         || rType == cppu::UnoType<sal_Int32>::get()
         || rType == cppu::UnoType<sal_Int16>::get()
         || rType == cppu::UnoType<bool>::get()
         || rType == cppu::UnoType<cppu::UnoCharType>::get()
         || rType == cppu::UnoType<sal_Int8>::get()
         || rType == cppu::UnoType<sal_Int64>::get()
         || rType == cppu::UnoType<float>::get()
         || rType == cppu::UnoType<double>::get();
}


// static
bool UCBDeadPropertyValue::createFromXML( const OString & rInData,
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
                    if ( aStringValue.equalsIgnoreAsciiCase("true") )
                        rOutData <<= true;
                    else
                        rOutData <<= false;
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
                    SAL_WARN( "ucb.ucp.webdav", "createFromXML() - "
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
        SAL_WARN( "ucb.ucp.webdav", "toXML() - unsupported property type!" );
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
