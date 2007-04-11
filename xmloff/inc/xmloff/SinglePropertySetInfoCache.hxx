/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SinglePropertySetInfoCache.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:22:14 $
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

#ifndef _XMLOFF_SINGEPROPERTYSETINFOCACHE_HXX
#define _XMLOFF_SINGEPROPERTYSETINFOCACHE_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#include <hash_map>

#ifndef _XMLOFF_PROPERTYSETINFOHASH_HXX
#include <xmloff/PropertySetInfoHash.hxx>
#endif

typedef std::hash_map
<
    PropertySetInfoKey,
    sal_Bool,
    PropertySetInfoHash,
    PropertySetInfoHash
>
SinglePropertySetInfoMap_Impl;

class SinglePropertySetInfoCache : private SinglePropertySetInfoMap_Impl
{
    ::rtl::OUString sName;

public:

    inline SinglePropertySetInfoCache( const ::rtl::OUString& rName );
    ~SinglePropertySetInfoCache() {};

    sal_Bool hasProperty(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >& rPropSet,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySetInfo >& rPropSetInfo );
    inline sal_Bool hasProperty(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet>& rPropSet );
};

inline SinglePropertySetInfoCache::SinglePropertySetInfoCache(
        const ::rtl::OUString& rName ) :
    sName( rName )
{
}

inline sal_Bool SinglePropertySetInfoCache::hasProperty(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet>& rPropSet )
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySetInfo > xInfo;
    return hasProperty( rPropSet, xInfo );
}

#endif
