/*************************************************************************
 *
 *  $RCSfile: UCBDeadPropertyValue.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kso $ $Date: 2002-08-22 11:37:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _UCBDEADPROPERTYVALUE_HXX_
#include "UCBDeadPropertyValue.hxx"
#endif

using namespace webdav_ucp;
using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////

#define DAV_ELM_LOCK_FIRST (NE_ELM_UNUSED)

#define DAV_ELM_ucbprop (DAV_ELM_LOCK_FIRST +  1)
#define DAV_ELM_type    (DAV_ELM_LOCK_FIRST +  2)
#define DAV_ELM_value   (DAV_ELM_LOCK_FIRST +  3)

// static
const struct ne_xml_elm UCBDeadPropertyValue::elements[] =
{
    { "", "ucbprop", DAV_ELM_ucbprop, 0 },
    { "", "type",    DAV_ELM_type,    NE_XML_CDATA },
    { "", "value",   DAV_ELM_value,   NE_XML_CDATA },
    { 0 }
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

struct UCBDeadPropertyValueParseContext
{
    rtl::OUString * pType;
    rtl::OUString * pValue;

    UCBDeadPropertyValueParseContext() : pType( 0 ), pValue( 0 ) {}
    ~UCBDeadPropertyValueParseContext() { delete pType; delete pValue; }
};

//////////////////////////////////////////////////////////////////////////
extern "C" static int validate_callback( void * userdata,
                                         ne_xml_elmid parent,
                                         ne_xml_elmid child )
{
    switch ( parent )
    {
        case 0:
            if ( child == DAV_ELM_ucbprop )
                return NE_XML_VALID;

            break;

        case DAV_ELM_ucbprop:
            return NE_XML_VALID;

        default:
            break;
    }

    return NE_XML_DECLINE;
}

//////////////////////////////////////////////////////////////////////////
// static
extern "C" static int endelement_callback( void * userdata,
                                           const struct ne_xml_elm * s,
                                           const char * cdata )
{
    UCBDeadPropertyValueParseContext * pCtx
            = static_cast< UCBDeadPropertyValueParseContext * >( userdata );

    switch ( s->id )
    {
        case DAV_ELM_type:
            OSL_ENSURE( !pCtx->pType,
                        "UCBDeadPropertyValue::endelement_callback - "
                        "Type already set!" );
            pCtx->pType = new rtl::OUString(
                                rtl::OUString::createFromAscii( cdata ) );
            break;

        case DAV_ELM_value:
            OSL_ENSURE( !pCtx->pValue,
                        "UCBDeadPropertyValue::endelement_callback - "
                        "Value already set!" );
            pCtx->pValue = new rtl::OUString(
                                rtl::OUString::createFromAscii( cdata ) );
            break;

        default:
            break;
    }
    return 0;
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
    //          <ucbprop><type>string</type><value>x&lt;z</value></ucbprop>
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
                                  elements,
                                  validate_callback,
                                  0, // startelement_callback
                                  endelement_callback,
                                  &aCtx );

        ne_xml_parse( parser, rInData.getStr(), rInData.getLength() );

        success = !!ne_xml_valid( parser );

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
        sal_Int32 nValue;
        rInData >>= nValue;
        aStringValue = rtl::OUString::valueOf( nValue );
        aStringType = aTypeLong;
    }
    else if ( rType == getCppuType( static_cast< const sal_Int16 * >( 0 ) ) )
    {
        // short
        sal_Int32 nValue;
        rInData >>= nValue;
        aStringValue = rtl::OUString::valueOf( nValue );
        aStringType = aTypeShort;
    }
    else if ( rType == getCppuBooleanType() )
    {
        // boolean
        sal_Bool bValue;
        rInData >>= bValue;
        aStringValue = rtl::OUString::valueOf( bValue );
        aStringType = aTypeBoolean;
    }
    else if ( rType == getCppuCharType() )
    {
        // char
        sal_Unicode cValue;
        rInData >>= cValue;
        aStringValue = rtl::OUString::valueOf( cValue );
        aStringType = aTypeChar;
    }
    else if ( rType == getCppuType( static_cast< const sal_Int8 * >( 0 ) ) )
    {
        // byte
        sal_Int8 nValue;
        rInData >>= nValue;
        aStringValue = rtl::OUString::valueOf( sal_Unicode( nValue ) );
        aStringType = aTypeByte;
    }
    else if ( rType == getCppuType( static_cast< const sal_Int64 * >( 0 ) ) )
    {
        // hyper
        sal_Int64 nValue;
        rInData >>= nValue;
        aStringValue = rtl::OUString::valueOf( nValue );
        aStringType = aTypeHyper;
    }
    else if ( rType == getCppuType( static_cast< const float * >( 0 ) ) )
    {
        // float
        float nValue;
        rInData >>= nValue;
        aStringValue = rtl::OUString::valueOf( nValue );
        aStringType = aTypeFloat;
    }
    else if ( rType == getCppuType( static_cast< const double * >( 0 ) ) )
    {
        // double
        double nValue;
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
