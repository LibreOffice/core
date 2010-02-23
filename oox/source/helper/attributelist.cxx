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

// ============================================================================

AttributeList::AttributeList( const Reference< XFastAttributeList >& rxAttribs ) :
    mxAttribs( rxAttribs )
{
    OSL_ENSURE( mxAttribs.is(), "AttributeList::AttributeList - missing attribute list interface" );
}

bool AttributeList::hasAttribute( sal_Int32 nElement ) const
{
    return mxAttribs->hasAttribute( nElement );
}

// static string conversion -----------------------------------------------

OUString AttributeList::decodeXString( const OUString& rValue )
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

double AttributeList::decodeDouble( const OUString& rValue )
{
    return rValue.toDouble();
}

sal_Int32 AttributeList::decodeInteger( const OUString& rValue )
{
    return rValue.toInt32();
}

sal_uInt32 AttributeList::decodeUnsigned( const OUString& rValue )
{
    return getLimitedValue< sal_uInt32, sal_Int64 >( rValue.toInt64(), 0, SAL_MAX_UINT32 );
}

sal_Int64 AttributeList::decodeHyper( const OUString& rValue )
{
    return rValue.toInt64();
}

sal_Int32 AttributeList::decodeIntegerHex( const OUString& rValue )
{
    return rValue.toInt32( 16 );
}

sal_uInt32 AttributeList::decodeUnsignedHex( const OUString& rValue )
{
    return getLimitedValue< sal_uInt32, sal_Int64 >( rValue.toInt64( 16 ), 0, SAL_MAX_UINT32 );
}

sal_Int64 AttributeList::decodeHyperHex( const OUString& rValue )
{
    return rValue.toInt64( 16 );
}

// optional return values -----------------------------------------------------

OptValue< sal_Int32 > AttributeList::getToken( sal_Int32 nElement ) const
{
    sal_Int32 nToken = mxAttribs->getOptionalValueToken( nElement, XML_TOKEN_INVALID );
    return OptValue< sal_Int32 >( nToken != XML_TOKEN_INVALID, nToken );
}

OptValue< OUString > AttributeList::getString( sal_Int32 nElement ) const
{
    return OptValue< OUString >( mxAttribs->hasAttribute( nElement ), mxAttribs->getOptionalValue( nElement ) );
}

OptValue< OUString > AttributeList::getXString( sal_Int32 nElement ) const
{
    return OptValue< OUString >( mxAttribs->hasAttribute( nElement ), decodeXString( mxAttribs->getOptionalValue( nElement ) ) );
}

OptValue< double > AttributeList::getDouble( sal_Int32 nElement ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nElement );
    bool bValid = aValue.getLength() > 0;
    return OptValue< double >( bValid, bValid ? decodeDouble( aValue ) : 0.0 );
}

OptValue< sal_Int32 > AttributeList::getInteger( sal_Int32 nElement ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nElement );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_Int32 >( bValid, bValid ? decodeInteger( aValue ) : 0 );
}

OptValue< sal_uInt32 > AttributeList::getUnsigned( sal_Int32 nElement ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nElement );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_uInt32 >( bValid, decodeUnsigned( aValue ) );
}

OptValue< sal_Int64 > AttributeList::getHyper( sal_Int32 nElement ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nElement );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_Int64 >( bValid, bValid ? decodeHyper( aValue ) : 0 );
}

OptValue< sal_Int32 > AttributeList::getIntegerHex( sal_Int32 nElement ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nElement );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_Int32 >( bValid, bValid ? decodeIntegerHex( aValue ) : 0 );
}

OptValue< sal_uInt32 > AttributeList::getUnsignedHex( sal_Int32 nElement ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nElement );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_uInt32 >( bValid, bValid ? decodeUnsignedHex( aValue ) : 0 );
}

OptValue< sal_Int64 > AttributeList::getHyperHex( sal_Int32 nElement ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nElement );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_Int64 >( bValid, bValid ? decodeHyperHex( aValue ) : 0 );
}

OptValue< bool > AttributeList::getBool( sal_Int32 nElement ) const
{
    // boolean attributes may be "t", "f", "true", "false", "on", "off", "1", or "0"
    switch( getToken( nElement, -1 ) )
    {
        case XML_t:     return OptValue< bool >( true, true );  // used in VML
        case XML_true:  return OptValue< bool >( true, true );
        case XML_on:    return OptValue< bool >( true, true );
        case XML_f:     return OptValue< bool >( true, false ); // used in VML
        case XML_false: return OptValue< bool >( true, false );
        case XML_off:   return OptValue< bool >( true, false );
    }
    OptValue< sal_Int32 > onValue = getInteger( nElement );
    return OptValue< bool >( onValue.has(), onValue.get() != 0 );
}

OptValue< DateTime > AttributeList::getDateTime( sal_Int32 nElement ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nElement );
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

sal_Int32 AttributeList::getToken( sal_Int32 nElement, sal_Int32 nDefault ) const
{
    return mxAttribs->getOptionalValueToken( nElement, nDefault );
}

OUString AttributeList::getString( sal_Int32 nElement, const OUString& rDefault ) const
{
    try
    {
        return mxAttribs->getValue( nElement );
    }
    catch( Exception& )
    {
    }
    return rDefault;
}

OUString AttributeList::getXString( sal_Int32 nElement, const OUString& rDefault ) const
{
    return getXString( nElement ).get( rDefault );
}

double AttributeList::getDouble( sal_Int32 nElement, double fDefault ) const
{
    return getDouble( nElement ).get( fDefault );
}

sal_Int32 AttributeList::getInteger( sal_Int32 nElement, sal_Int32 nDefault ) const
{
    return getInteger( nElement ).get( nDefault );
}

sal_uInt32 AttributeList::getUnsigned( sal_Int32 nElement, sal_uInt32 nDefault ) const
{
    return getUnsigned( nElement ).get( nDefault );
}

sal_Int64 AttributeList::getHyper( sal_Int32 nElement, sal_Int64 nDefault ) const
{
    return getHyper( nElement ).get( nDefault );
}

sal_Int32 AttributeList::getIntegerHex( sal_Int32 nElement, sal_Int32 nDefault ) const
{
    return getIntegerHex( nElement ).get( nDefault );
}

sal_uInt32 AttributeList::getUnsignedHex( sal_Int32 nElement, sal_uInt32 nDefault ) const
{
    return getUnsignedHex( nElement ).get( nDefault );
}

sal_Int64 AttributeList::getHyperHex( sal_Int32 nElement, sal_Int64 nDefault ) const
{
    return getHyperHex( nElement ).get( nDefault );
}

bool AttributeList::getBool( sal_Int32 nElement, bool bDefault ) const
{
    return getBool( nElement ).get( bDefault );
}

DateTime AttributeList::getDateTime( sal_Int32 nElement, const DateTime& rDefault ) const
{
    return getDateTime( nElement ).get( rDefault );
}

// ============================================================================

} // namespace oox

