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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weakref.hxx>

#ifndef _XMLOFF_SINGLEPROPERTYSETINFOCACHE_HXX
#include <xmloff/SinglePropertySetInfoCache.hxx>
#endif

using namespace ::com::sun::star::uno;
using ::com::sun::star::lang::XTypeProvider;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;

sal_Bool SinglePropertySetInfoCache::hasProperty(
        const Reference< XPropertySet >& rPropSet,
        Reference< XPropertySetInfo >& rPropSetInfo )
{
    if( !rPropSetInfo.is() )
        rPropSetInfo = rPropSet->getPropertySetInfo();
    sal_Bool bRet = sal_False, bValid = sal_False;
    Reference < XTypeProvider > xTypeProv( rPropSet, UNO_QUERY );
    Sequence< sal_Int8 > aImplId;
    if( xTypeProv.is() )
    {
        aImplId = xTypeProv->getImplementationId();
        if( aImplId.getLength() == 16 )
        {
            // The key must not be created outside this block, because it
            // keeps a reference to the property set info.
            PropertySetInfoKey aKey( rPropSetInfo, aImplId );
            iterator aIter = find( aKey );
            if( aIter != end() )
            {
                bRet = (*aIter).second;
                bValid = sal_True;
            }
        }
    }
    if( !bValid )
    {
        bRet = rPropSetInfo->hasPropertyByName( sName );
        if( xTypeProv.is() && aImplId.getLength() == 16 )
        {
            // Check whether the property set info is destroyed if it is
            // assigned to a weak reference only. If it is destroyed, then
            // every instance of getPropertySetInfo returns a new object.
            // Such property set infos must not be cached.
            WeakReference < XPropertySetInfo > xWeakInfo( rPropSetInfo );
            rPropSetInfo = 0;
            rPropSetInfo = xWeakInfo;
            if( rPropSetInfo.is() )
            {
                PropertySetInfoKey aKey( rPropSetInfo, aImplId );
                value_type aValue( aKey, bRet );
                insert( aValue );
            }
        }
    }

    return bRet;
}
