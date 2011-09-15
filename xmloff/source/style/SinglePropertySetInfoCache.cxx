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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <cppuhelper/weakref.hxx>

#include <xmloff/SinglePropertySetInfoCache.hxx>

using namespace ::com::sun::star::uno;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;

sal_Bool SinglePropertySetInfoCache::hasProperty(
        const Reference< XPropertySet >& rPropSet,
        Reference< XPropertySetInfo >& rPropSetInfo )
{
    if( !rPropSetInfo.is() )
        rPropSetInfo = rPropSet->getPropertySetInfo();
    iterator aIter = find( rPropSetInfo );
    if( aIter != end() )
    {
        return aIter->second;
    }
    bool bRet = rPropSetInfo->hasPropertyByName( sName );
    // Check whether the property set info is destroyed if it is
    // assigned to a weak reference only. If it is destroyed, then
    // every instance of getPropertySetInfo returns a new object.
    // Such property set infos must not be cached.
    WeakReference < XPropertySetInfo > xWeakInfo( rPropSetInfo );
    rPropSetInfo = 0;
    rPropSetInfo = xWeakInfo;
    if( rPropSetInfo.is() )
    {
        value_type aValue( rPropSetInfo, bRet );
        insert( aValue );
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
