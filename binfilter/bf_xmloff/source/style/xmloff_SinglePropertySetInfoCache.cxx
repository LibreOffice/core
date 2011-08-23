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

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weakref.hxx>

#include "SinglePropertySetInfoCache.hxx"
namespace binfilter {

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
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
