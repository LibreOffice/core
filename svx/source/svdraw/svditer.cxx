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

#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmark.hxx>

SdrObjListIter::SdrObjListIter(const SdrObjList* pObjList, SdrIterMode eMode, bool bReverse)
:   maObjList(),
    mnIndex(0),
    mbReverse(bReverse),
    mbUseZOrder(true)
{
    if(nullptr != pObjList)
    {
        ImpProcessObjectList(*pObjList, eMode);
    }

    Reset();
}

SdrObjListIter::SdrObjListIter(const SdrObjList* pObjList, bool bUseZOrder, SdrIterMode eMode, bool bReverse)
:   maObjList(),
    mnIndex(0),
    mbReverse(bReverse),
    mbUseZOrder(bUseZOrder)
{
    if(nullptr != pObjList)
    {
        // correct when we have no ObjectNavigationOrder
        if(!mbUseZOrder && !pObjList->HasObjectNavigationOrder())
        {
            mbUseZOrder = false;
        }

        ImpProcessObjectList(*pObjList, eMode);
    }

    Reset();
}

SdrObjListIter::SdrObjListIter(const SdrObject& rSdrObject, SdrIterMode eMode, bool bReverse)
:   maObjList(),
    mnIndex(0),
    mbReverse(bReverse),
    mbUseZOrder(true)
{
    ImpProcessObj(rSdrObject, eMode);
    Reset();
}

SdrObjListIter::SdrObjListIter(const SdrPage* pSdrPage, SdrIterMode eMode, bool bReverse)
:   maObjList(),
    mnIndex(0),
    mbReverse(bReverse),
    mbUseZOrder(true)
{
    if (pSdrPage)
        ImpProcessObjectList(*pSdrPage, eMode);
    Reset();
}

SdrObjListIter::SdrObjListIter( const SdrMarkList& rMarkList, SdrIterMode eMode )
:   maObjList(),
    mnIndex(0),
    mbReverse(false),
    mbUseZOrder(true)
{
    ImpProcessMarkList(rMarkList, eMode);
    Reset();
}

void SdrObjListIter::ImpProcessObjectList(const SdrObjList& rObjList, SdrIterMode eMode)
{    for(size_t nIdx(0), nCount(rObjList.GetObjCount()); nIdx < nCount; ++nIdx)
    {
        const SdrObject* pSdrObject(mbUseZOrder
            ? rObjList.GetObj(nIdx)
            : rObjList.GetObjectForNavigationPosition(nIdx));

        if(nullptr == pSdrObject)
        {
            OSL_ENSURE(false, "SdrObjListIter: corrupted SdrObjList (!)");
        }
        else
        {
            ImpProcessObj(*pSdrObject, eMode);
        }
    }
}

void SdrObjListIter::ImpProcessMarkList(const SdrMarkList& rMarkList, SdrIterMode eMode)
{
    for( size_t nIdx = 0, nCount = rMarkList.GetMarkCount(); nIdx < nCount; ++nIdx )
    {
        if( SdrObject* pObj = rMarkList.GetMark( nIdx )->GetMarkedSdrObj() )
        {
            ImpProcessObj(*pObj, eMode);
        }
    }
}

void SdrObjListIter::ImpProcessObj(const SdrObject& rSdrObject, SdrIterMode eMode)
{
    // TTTT: Note: The behaviour has changed here, it will now deep-iterate
    // for SdrObjGroup and E3dScene. Old version only deep-dived for SdrObjGroup,
    // E3dScene was just added flat. This is now more correct, but potentially
    // there will exist code in the 3D area that *self-iterates* with local
    // functions/methods due to this iterator was not doing the expected thing.
    // These will be difficult to find, but in most cases should do no harm,
    // but cost runtime. Will need to have an eye on this aspect on continued
    // changes...
    const SdrObjList* pChildren(rSdrObject.getChildrenOfSdrObject());
    const bool bIsGroup(nullptr != pChildren);

    if(!bIsGroup || (SdrIterMode::DeepNoGroups != eMode))
    {
        maObjList.push_back(&rSdrObject);
    }

    if(bIsGroup && (SdrIterMode::Flat != eMode))
    {
        ImpProcessObjectList(*pChildren, eMode);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
