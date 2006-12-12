/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: STLPropertySet.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:52:31 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SD_STLPROPERTYSET_HXX
#include "STLPropertySet.hxx"
#endif

using namespace com::sun::star::beans;

using rtl::OUString;
using com::sun::star::uno::Any;

namespace sd
{

STLPropertySet::STLPropertySet()
{
}

STLPropertySet::~STLPropertySet()
{
}

void STLPropertySet::setPropertyDefaultValue( sal_Int32 nHandle, const Any& rValue )
{
    STLPropertyMapEntry aEntry( rValue, STLPropertyState_DEFAULT );
    maPropertyMap[ nHandle ] = aEntry;
}

void STLPropertySet::setPropertyValue( sal_Int32 nHandle, const Any& rValue, sal_Int32 /* nState = STLPropertyState_DIRECT */ )
{
    PropertyMapIter aIter;
    if( findProperty( nHandle, aIter ) )
    {
        (*aIter).second.mnState = STLPropertyState_DIRECT;
        (*aIter).second.maValue = rValue;
    }
    else
    {
        DBG_ERROR( "sd::STLPropertySet::setPropertyValue(), unknown property!" );
    }
}

Any STLPropertySet::getPropertyValue( sal_Int32 nHandle ) const
{
    PropertyMapConstIter aIter;
    if( findProperty( nHandle, aIter ) )
    {
        return (*aIter).second.maValue;
    }
    else
    {
        DBG_ERROR( "sd::STLPropertySet::setPropertyValue(), unknown property!" );

        Any aAny;
        return aAny;
    }
}

sal_Int32 STLPropertySet::getPropertyState( sal_Int32 nHandle ) const
{
    PropertyMapConstIter aIter;
    if( findProperty( nHandle, aIter ) )
    {
        return (*aIter).second.mnState;
    }
    else
    {
        DBG_ERROR( "sd::STLPropertySet::setPropertyState(), unknown property!" );
        return STLPropertyState_AMBIGUOUS;
    }
}

void STLPropertySet::setPropertyState( sal_Int32 nHandle, sal_Int32 nState )
{
    PropertyMapIter aIter;
    if( findProperty( nHandle, aIter ) )
    {
        (*aIter).second.mnState = nState;
    }
    else
    {
        DBG_ERROR( "sd::STLPropertySet::setPropertyState(), unknown property!" );
    }
}

bool STLPropertySet::findProperty( sal_Int32 nHandle, PropertyMapIter& rIter )
{
    rIter = maPropertyMap.find(nHandle);
    return( rIter != maPropertyMap.end() );
}

bool STLPropertySet::findProperty( sal_Int32 nHandle, PropertyMapConstIter& rIter ) const
{
    rIter = maPropertyMap.find(nHandle);
    return( rIter != maPropertyMap.end() );
}

}
