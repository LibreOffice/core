/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: attributelist.cxx,v $
 * $Revision: 1.4 $
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

sal_Int32 AttributeList::getToken( sal_Int32 nElement, sal_Int32 nDefault ) const
{
    return mxAttribs->getOptionalValueToken( nElement, nDefault );
}

OUString AttributeList::getString( sal_Int32 nElement ) const
{
    return mxAttribs->getOptionalValue( nElement );
}

double AttributeList::getDouble( sal_Int32 nElement, double fDefault ) const
{
    OUString aValue = getString( nElement );
    return (aValue.getLength() == 0) ? fDefault : aValue.toDouble();
}

sal_Int32 AttributeList::getInteger( sal_Int32 nElement, sal_Int32 nDefault ) const
{
    OUString aValue = getString( nElement );
    return (aValue.getLength() == 0) ? nDefault : aValue.toInt32();
}

sal_uInt32 AttributeList::getUnsignedInteger( sal_Int32 nElement, sal_uInt32 nDefault ) const
{
    OUString aValue = getString( nElement );
    if( aValue.getLength() == 0 )
        return nDefault;
    sal_Int64 nValue = aValue.toInt64();
    return static_cast< sal_uInt32 >( ((nValue < 0) || (nValue > SAL_MAX_UINT32)) ? 0 : nValue );
}

sal_Int64 AttributeList::getInteger64( sal_Int32 nElement, sal_Int64 nDefault ) const
{
    OUString aValue = getString( nElement );
    return (aValue.getLength() == 0) ? nDefault : aValue.toInt64();
}

sal_Int32 AttributeList::getHex( sal_Int32 nElement, sal_Int32 nDefault ) const
{
    OUString aValue = getString( nElement );
    return (aValue.getLength() == 0) ? nDefault : aValue.toInt32( 16 );
}

bool AttributeList::getBool( sal_Int32 nElement, bool bDefault ) const
{
    // boolean attributes may be "true", "false", "on", "off", "1", or "0"
    switch( getToken( nElement ) )
    {
        case XML_true:  return true;
        case XML_on:    return true;
        case XML_false: return false;
        case XML_off:   return false;
    }
    return getInteger( nElement, bDefault ? 1 : 0 ) != 0;
}

// ============================================================================

} // namespace oox

