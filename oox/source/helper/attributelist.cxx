/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: attributelist.cxx,v $
 * $Revision: 1.6 $
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

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::util::DateTime;
using ::com::sun::star::xml::sax::XFastAttributeList;

namespace oox {

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

OptValue< double > AttributeList::getDouble( sal_Int32 nElement ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nElement );
    bool bValid = aValue.getLength() > 0;
    return OptValue< double >( bValid, bValid ? aValue.toDouble() : 0.0 );
}

OptValue< sal_Int32 > AttributeList::getInteger( sal_Int32 nElement ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nElement );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_Int32 >( bValid, bValid ? aValue.toInt32() : 0 );
}

OptValue< sal_uInt32 > AttributeList::getUnsignedInteger( sal_Int32 nElement ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nElement );
    bool bValid = aValue.getLength() > 0;
    sal_Int64 nValue = bValid ? aValue.toInt64() : 0;
    return OptValue< sal_uInt32 >( bValid, static_cast< sal_uInt32 >( ((nValue < 0) || (nValue > SAL_MAX_UINT32)) ? 0 : nValue ) );
}

OptValue< sal_Int64 > AttributeList::getInteger64( sal_Int32 nElement ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nElement );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_Int64 >( bValid, bValid ? aValue.toInt64() : 0 );
}

OptValue< sal_Int32 > AttributeList::getHex( sal_Int32 nElement ) const
{
    OUString aValue = mxAttribs->getOptionalValue( nElement );
    bool bValid = aValue.getLength() > 0;
    return OptValue< sal_Int32 >( bValid, bValid ? aValue.toInt32( 16 ) : 0 );
}

OptValue< bool > AttributeList::getBool( sal_Int32 nElement ) const
{
    // boolean attributes may be "true", "false", "on", "off", "1", or "0"
    switch( getToken( nElement, -1 ) )
    {
        case XML_true:  return OptValue< bool >( true, true );
        case XML_on:    return OptValue< bool >( true, true );
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

double AttributeList::getDouble( sal_Int32 nElement, double fDefault ) const
{
    return getDouble( nElement ).get( fDefault );
}

sal_Int32 AttributeList::getInteger( sal_Int32 nElement, sal_Int32 nDefault ) const
{
    return getInteger( nElement ).get( nDefault );
}

sal_uInt32 AttributeList::getUnsignedInteger( sal_Int32 nElement, sal_uInt32 nDefault ) const
{
    return getUnsignedInteger( nElement ).get( nDefault );
}

sal_Int64 AttributeList::getInteger64( sal_Int32 nElement, sal_Int64 nDefault ) const
{
    return getInteger64( nElement ).get( nDefault );
}

sal_Int32 AttributeList::getHex( sal_Int32 nElement, sal_Int32 nDefault ) const
{
    return getHex( nElement ).get( nDefault );
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

