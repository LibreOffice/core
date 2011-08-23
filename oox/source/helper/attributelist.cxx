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

#include "oox/helper/attributelist.hxx"
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::util::DateTime;
using ::com::sun::star::xml::sax::XFastAttributeList;

namespace oox {

// ============================================================================

namespace {

const sal_Int32 XSTRING_ENCCHAR_LEN     = 7;

bool lclAddHexDigit( sal_Unicode& orcChar, sal_Unicode cDigit, int nBitShift )
{
    if( ('0' <= cDigit) && (cDigit <= '9') ) { orcChar |= ((cDigit - '0') << nBitShift); return true; }
    if( ('a' <= cDigit) && (cDigit <= 'f') ) { orcChar |= ((cDigit - 'a' + 10) << nBitShift); return true; }
    if( ('A' <= cDigit) && (cDigit <= 'F') ) { orcChar |= ((cDigit - 'A' + 10) << nBitShift); return true; }
    return false;
}

sal_Unicode lclGetXChar( const sal_Unicode*& rpcStr, const sal_Unicode* pcEnd )
{
    sal_Unicode cChar = 0;
    if( (pcEnd - rpcStr >= XSTRING_ENCCHAR_LEN) &&
        (rpcStr[ 0 ] == '_') &&
        (rpcStr[ 1 ] == 'x') &&
        (rpcStr[ 6 ] == '_') &&
        lclAddHexDigit( cChar, rpcStr[ 2 ], 12 ) &&
        lclAddHexDigit( cChar, rpcStr[ 3 ], 8 ) &&
        lclAddHexDigit( cChar, rpcStr[ 4 ], 4 ) &&
        lclAddHexDigit( cChar, rpcStr[ 5 ], 0 ) )
    {
        rpcStr += XSTRING_ENCCHAR_LEN;
        return cChar;
    }
    return *rpcStr++;
}

} // namespace

// ----------------------------------------------------------------------------

OUString AttributeConversion::decodeXString( const OUString& rValue )
{
    // string shorter than one encoded character - no need to decode
    if( rValue.getLength() < XSTRING_ENCCHAR_LEN )
        return rValue;
    OUStringBuffer aBuffer;
    const sal_Unicode* pcStr = rValue.getStr();
    const sal_Unicode* pcEnd = pcStr + rValue.getLength();
    while( pcStr < pcEnd )
        aBuffer.append( lclGetXChar( pcStr, pcEnd ) );
    return aBuffer.makeStringAndClear();
}

double AttributeConversion::decodeDouble( const OUString& rValue )
{
    return rValue.toDouble();
}

sal_Int32 AttributeConversion::decodeInteger( const OUString& rValue )
{
    return rValue.toInt32();
}

sal_uInt32 AttributeConversion::decodeUnsigned( const OUString& rValue )
{
    return getLimitedValue< sal_uInt32, sal_Int64 >( rValue.toInt64(), 0, SAL_MAX_UINT32 );
}

sal_Int64 AttributeConversion::decodeHyper( const OUString& rValue )
{
    return rValue.toInt64();
}

sal_Int32 AttributeConversion::decodeIntegerHex( const OUString& rValue )
{
    return rValue.toInt32( 16 );
}

sal_uInt32 AttributeConversion::decodeUnsignedHex( const OUString& rValue )
{
    return getLimitedValue< sal_uInt32, sal_Int64 >( rValue.toInt64( 16 ), 0, SAL_MAX_UINT32 );
}

sal_Int64 AttributeConversion::decodeHyperHex( const OUString& rValue )
{
    return rValue.toInt64( 16 );
}

// ============================================================================

AttributeList::AttributeList( const Reference< XFastAttributeList >& rxAttribs ) :
    mxAttribs( rxAttribs )
{
    OSL_ENSURE( mxAttribs.is(), "AttributeList::AttributeList - missing attribute list interface" );
}

bool AttributeList::hasAttribute( sal_Int32 nAttrToken ) const
{
    return mxAttribs->hasAttribute( nAttrToken );
}

// optional return values -----------------------------------------------------

OptValue< sal_Int32 > AttributeList::getToken( sal_Int32 nAttrToken ) const
{
    sal_Int32 nToken = mxAttribs->getOptionalValueToken( nAttrToken, XML_TOKEN_INVALID );
    return OptValue< sal_Int32 >( nToken != XML_TOKEN_INVALID, nToken );
}

OptValue< OUString > AttributeList::getString( sal_Int32 nAttrToken ) const
{
    // check if the attribute exists (empty string may be different to missing attribute)
    if( mxAttribs->hasAttribute( nAttrToken ) )
        return OptValue< OUString >( mxAttribs->getOptionalValue( nAttrToken ) );
    return OptValue< OUString >();
}

OptValue< OUString > AttributeList::getXString( sal_Int32 nAttrToken ) const
{
    // check if the attribute exists (empty string may be different to missing attribute)
    if( mxAttribs->hasAttribute( nAttrToken ) )
        return OptValue< OUString >( AttributeConversion::decodeXString( mxAttribs->getOptionalValue( nAttrToken ) ) );
    return OptValue< OUString >();
}

OptValue< double > AttributeList::getDouble( sal_Int32 nAttrToken ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nAttrToken );
    bool bValid = aValue.getLength() > 0;
    return OptValue< double >( bValid, bValid ? AttributeConversion::decodeDouble( aValue ) : 0.0 );
}

OptValue< sal_Int32 > AttributeList::getInteger( sal_Int32 nAttrToken ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nAttrToken );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_Int32 >( bValid, bValid ? AttributeConversion::decodeInteger( aValue ) : 0 );
}

OptValue< sal_uInt32 > AttributeList::getUnsigned( sal_Int32 nAttrToken ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nAttrToken );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_uInt32 >( bValid, AttributeConversion::decodeUnsigned( aValue ) );
}

OptValue< sal_Int64 > AttributeList::getHyper( sal_Int32 nAttrToken ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nAttrToken );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_Int64 >( bValid, bValid ? AttributeConversion::decodeHyper( aValue ) : 0 );
}

OptValue< sal_Int32 > AttributeList::getIntegerHex( sal_Int32 nAttrToken ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nAttrToken );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_Int32 >( bValid, bValid ? AttributeConversion::decodeIntegerHex( aValue ) : 0 );
}

OptValue< sal_uInt32 > AttributeList::getUnsignedHex( sal_Int32 nAttrToken ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nAttrToken );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_uInt32 >( bValid, bValid ? AttributeConversion::decodeUnsignedHex( aValue ) : 0 );
}

OptValue< sal_Int64 > AttributeList::getHyperHex( sal_Int32 nAttrToken ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nAttrToken );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_Int64 >( bValid, bValid ? AttributeConversion::decodeHyperHex( aValue ) : 0 );
}

OptValue< bool > AttributeList::getBool( sal_Int32 nAttrToken ) const
{
    // boolean attributes may be "t", "f", "true", "false", "on", "off", "1", or "0"
    switch( getToken( nAttrToken, XML_TOKEN_INVALID ) )
    {
        case XML_t:     return OptValue< bool >( true );  // used in VML
        case XML_true:  return OptValue< bool >( true );
        case XML_on:    return OptValue< bool >( true );
        case XML_f:     return OptValue< bool >( false ); // used in VML
        case XML_false: return OptValue< bool >( false );
        case XML_off:   return OptValue< bool >( false );
    }
    OptValue< sal_Int32 > onValue = getInteger( nAttrToken );
    return OptValue< bool >( onValue.has(), onValue.get() != 0 );
}

OptValue< DateTime > AttributeList::getDateTime( sal_Int32 nAttrToken ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nAttrToken );
    DateTime aDateTime;
    bool bValid = (aValue.getLength() == 19) && (aValue[ 4 ] == '-') && (aValue[ 7 ] == '-') &&
        (aValue[ 10 ] == 'T') && (aValue[ 13 ] == ':') && (aValue[ 16 ] == ':');
    if( bValid )
    {
        aDateTime.Year    = static_cast< sal_uInt16 >( aValue.copy( 0, 4 ).toInt32() );
        aDateTime.Month   = static_cast< sal_uInt16 >( aValue.copy( 5, 2 ).toInt32() );
        aDateTime.Day     = static_cast< sal_uInt16 >( aValue.copy( 8, 2 ).toInt32() );
        aDateTime.Hours   = static_cast< sal_uInt16 >( aValue.copy( 11, 2 ).toInt32() );
        aDateTime.Minutes = static_cast< sal_uInt16 >( aValue.copy( 14, 2 ).toInt32() );
        aDateTime.Seconds = static_cast< sal_uInt16 >( aValue.copy( 17, 2 ).toInt32() );
    }
    return OptValue< DateTime >( bValid, aDateTime );
}

// defaulted return values ----------------------------------------------------

sal_Int32 AttributeList::getToken( sal_Int32 nAttrToken, sal_Int32 nDefault ) const
{
    return mxAttribs->getOptionalValueToken( nAttrToken, nDefault );
}

OUString AttributeList::getString( sal_Int32 nAttrToken, const OUString& rDefault ) const
{
    try
    {
        return mxAttribs->getValue( nAttrToken );
    }
    catch( Exception& )
    {
    }
    return rDefault;
}

OUString AttributeList::getXString( sal_Int32 nAttrToken, const OUString& rDefault ) const
{
    return getXString( nAttrToken ).get( rDefault );
}

double AttributeList::getDouble( sal_Int32 nAttrToken, double fDefault ) const
{
    return getDouble( nAttrToken ).get( fDefault );
}

sal_Int32 AttributeList::getInteger( sal_Int32 nAttrToken, sal_Int32 nDefault ) const
{
    return getInteger( nAttrToken ).get( nDefault );
}

sal_uInt32 AttributeList::getUnsigned( sal_Int32 nAttrToken, sal_uInt32 nDefault ) const
{
    return getUnsigned( nAttrToken ).get( nDefault );
}

sal_Int64 AttributeList::getHyper( sal_Int32 nAttrToken, sal_Int64 nDefault ) const
{
    return getHyper( nAttrToken ).get( nDefault );
}

sal_Int32 AttributeList::getIntegerHex( sal_Int32 nAttrToken, sal_Int32 nDefault ) const
{
    return getIntegerHex( nAttrToken ).get( nDefault );
}

sal_uInt32 AttributeList::getUnsignedHex( sal_Int32 nAttrToken, sal_uInt32 nDefault ) const
{
    return getUnsignedHex( nAttrToken ).get( nDefault );
}

sal_Int64 AttributeList::getHyperHex( sal_Int32 nAttrToken, sal_Int64 nDefault ) const
{
    return getHyperHex( nAttrToken ).get( nDefault );
}

bool AttributeList::getBool( sal_Int32 nAttrToken, bool bDefault ) const
{
    return getBool( nAttrToken ).get( bDefault );
}

DateTime AttributeList::getDateTime( sal_Int32 nAttrToken, const DateTime& rDefault ) const
{
    return getDateTime( nAttrToken ).get( rDefault );
}

// ============================================================================

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
