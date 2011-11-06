/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _XMLOFF_SINGEPROPERTYSETINFOCACHE_HXX
#define _XMLOFF_SINGEPROPERTYSETINFOCACHE_HXX

#include <com/sun/star/beans/XPropertySet.hpp>

#include <hash_map>
#include <xmloff/PropertySetInfoHash.hxx>

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
