/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: STLPropertySet.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:52:45 $
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

#ifndef _SD_STLPROPERTYSET_HXX
#define _SD_STLPROPERTYSET_HXX

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _UTL_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#include <list>
#include <map>

namespace sd
{

const sal_Int32 STLPropertyState_DEFAULT = 0;
const sal_Int32 STLPropertyState_DIRECT = 1;
const sal_Int32 STLPropertyState_AMBIGUOUS = 3;

struct STLPropertyMapEntry
{
    ::com::sun::star::uno::Any maValue;
    sal_Int32 mnState;

    STLPropertyMapEntry()
        : mnState( STLPropertyState_AMBIGUOUS ) {}
    STLPropertyMapEntry( ::com::sun::star::uno::Any aValue, sal_Int32 nState = STLPropertyState_DEFAULT )
        : maValue( aValue ), mnState( nState ) {}

};

typedef std::map<sal_Int32, STLPropertyMapEntry > PropertyMap;
typedef PropertyMap::iterator PropertyMapIter;
typedef PropertyMap::const_iterator PropertyMapConstIter;

class STLPropertySet
{
public:
    STLPropertySet();
    ~STLPropertySet();

    void setPropertyDefaultValue( sal_Int32 nHandle, const com::sun::star::uno::Any& rValue );
    void setPropertyValue( sal_Int32 nHandle, const com::sun::star::uno::Any& rValue, sal_Int32 nState = STLPropertyState_DIRECT );
    ::com::sun::star::uno::Any getPropertyValue( sal_Int32 nHandle ) const;

    sal_Int32 getPropertyState( sal_Int32 nHandle ) const;
    void setPropertyState( sal_Int32 nHandle, sal_Int32 nState );

private:
    bool findProperty( sal_Int32 nHandle, PropertyMapIter& rIter );
    bool findProperty( sal_Int32 nHandle, PropertyMapConstIter& rIter ) const;

private:
    PropertyMap maPropertyMap;
};

}

#endif // _SD_STLPROPERTYSET_HXX
