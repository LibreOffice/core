/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _XMLOFF_SINGEPROPERTYSETINFOCACHE_HXX
#define _XMLOFF_SINGEPROPERTYSETINFOCACHE_HXX

#include <com/sun/star/beans/XPropertySet.hpp>

#include <boost/unordered_map.hpp>
#include <xmloff/PropertySetInfoHash.hxx>

typedef boost::unordered_map
<
    PropertySetInfoKey,
    sal_Bool,
    PropertySetInfoHash,
    PropertySetInfoHash
>
SinglePropertySetInfoMap_Impl;

class SinglePropertySetInfoCache : private SinglePropertySetInfoMap_Impl
{
    OUString sName;

public:

    inline SinglePropertySetInfoCache( const OUString& rName );
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
        const OUString& rName ) :
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
