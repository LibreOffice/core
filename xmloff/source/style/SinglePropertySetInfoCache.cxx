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

#include <cppuhelper/weakref.hxx>
#include <xmloff/SinglePropertySetInfoCache.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::com::sun::star::uno;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;

bool SinglePropertySetInfoCache::hasProperty(
        const Reference< XPropertySet >& rPropSet,
        Reference< XPropertySetInfo >& rPropSetInfo )
{
    if( !rPropSetInfo.is() )
        rPropSetInfo = rPropSet->getPropertySetInfo();
    Map::iterator aIter = map_.find( rPropSetInfo );
    if( aIter != map_.end() )
    {
        return (*aIter).second;
    }
    bool bRet = rPropSetInfo->hasPropertyByName( sName );
    // Check whether the property set info is destroyed if it is assigned to a
    // weak reference only; if it is destroyed, then every instance of
    // getPropertySetInfo returns a new object; Such property set infos must not
    // be cached:
    WeakReference < XPropertySetInfo > xWeakInfo( rPropSetInfo );
    rPropSetInfo = nullptr;
    rPropSetInfo = xWeakInfo;
    if( rPropSetInfo.is() )
    {
        map_.emplace(rPropSetInfo, bRet);
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
