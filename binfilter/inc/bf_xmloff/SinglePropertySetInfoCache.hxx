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

#ifndef _XMLOFF_SINGEPROPERTYSETINFOCACHE_HXX
#define _XMLOFF_SINGEPROPERTYSETINFOCACHE_HXX

#include <com/sun/star/beans/XPropertySet.hpp>

#include <hash_map>

#include <bf_xmloff/PropertySetInfoHash.hxx>
namespace binfilter {

typedef ::std::hash_map
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

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
