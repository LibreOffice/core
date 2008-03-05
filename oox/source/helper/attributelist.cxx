/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attributelist.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:42:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

