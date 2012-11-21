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

#include "svx/svditer.hxx"
#include <svx/svdpage.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmark.hxx>
#include <svx/scene3d.hxx>

SdrObjListIter::SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode, bool bReverse)
:   mnIndex(0L),
    mbReverse(bReverse)
{
    ImpProcessObjectList(rObjList, eMode, true);
    Reset();
}

SdrObjListIter::SdrObjListIter(const SdrObjList& rObjList, bool bUseZOrder, SdrIterMode eMode, bool bReverse)
:   mnIndex(0L),
    mbReverse(bReverse)
{
    ImpProcessObjectList(rObjList, eMode, bUseZOrder);
    Reset();
}

SdrObjListIter::SdrObjListIter( const SdrObject& rObj, SdrIterMode eMode, bool bReverse )
:   mnIndex(0L),
    mbReverse(bReverse)
{
    if ( rObj.ISA( SdrObjGroup ) )
        ImpProcessObjectList(*rObj.GetSubList(), eMode, true);
    else
        maObjList.push_back(const_cast<SdrObject*>(&rObj));
    Reset();
}

SdrObjListIter::SdrObjListIter( const SdrMarkList& rMarkList, SdrIterMode eMode, bool bReverse )
:   mnIndex(0L),
    mbReverse(bReverse)
{
    ImpProcessMarkList(rMarkList, eMode);
    Reset();
}

void SdrObjListIter::ImpProcessObjectList(const SdrObjList& rObjList, SdrIterMode eMode, bool bUseZOrder)
{
    for( sal_uIntPtr nIdx = 0, nCount = rObjList.GetObjCount(); nIdx < nCount; ++nIdx )
    {
        SdrObject* pObj = bUseZOrder ?
            rObjList.GetObj( nIdx ) : rObjList.GetObjectForNavigationPosition( nIdx );
        OSL_ASSERT( pObj != 0 );
        if( pObj )
            ImpProcessObj( pObj, eMode, bUseZOrder );
    }
}

void SdrObjListIter::ImpProcessMarkList( const SdrMarkList& rMarkList, SdrIterMode eMode )
{
    for( sal_uIntPtr nIdx = 0, nCount = rMarkList.GetMarkCount(); nIdx < nCount; ++nIdx )
        if( SdrObject* pObj = rMarkList.GetMark( nIdx )->GetMarkedSdrObj() )
            ImpProcessObj( pObj, eMode, false );
}

void SdrObjListIter::ImpProcessObj(SdrObject* pObj, SdrIterMode eMode, bool bUseZOrder)
{
    bool bIsGroup = pObj->IsGroupObject();
    // 3D objects are not group objects, IsGroupObject()
    // only tests if pSub is not null ptr :-(
    if( bIsGroup && pObj->ISA( E3dObject ) && !pObj->ISA( E3dScene ) )
        bIsGroup = false;

    if( !bIsGroup || (eMode != IM_DEEPNOGROUPS) )
        maObjList.push_back(pObj);

    if( bIsGroup && (eMode != IM_FLAT) )
        ImpProcessObjectList( *pObj->GetSubList(), eMode, bUseZOrder );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
