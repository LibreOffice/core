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
#include "precompiled_svx.hxx"
#include "svx/svditer.hxx"
#include <svx/svdpage.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmark.hxx>

// #99190#
#include <svx/scene3d.hxx>

SdrObjListIter::SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode, sal_Bool bReverse)
:   maObjList(1024, 64, 64),
    mnIndex(0L),
    mbReverse(bReverse)
{
    ImpProcessObjectList(rObjList, eMode, sal_True);
    Reset();
}

SdrObjListIter::SdrObjListIter(const SdrObjList& rObjList, sal_Bool bUseZOrder, SdrIterMode eMode, sal_Bool bReverse)
:   maObjList(1024, 64, 64),
    mnIndex(0L),
    mbReverse(bReverse)
{
    ImpProcessObjectList(rObjList, eMode, bUseZOrder);
    Reset();
}

SdrObjListIter::SdrObjListIter( const SdrObject& rObj, SdrIterMode eMode, sal_Bool bReverse )
:   maObjList(1024, 64, 64),
    mnIndex(0L),
    mbReverse(bReverse)
{
    if ( rObj.ISA( SdrObjGroup ) )
        ImpProcessObjectList(*rObj.GetSubList(), eMode, sal_True);
    else
        maObjList.Insert( (void*)&rObj, LIST_APPEND );
    Reset();
}

SdrObjListIter::SdrObjListIter( const SdrMarkList& rMarkList, SdrIterMode eMode, sal_Bool bReverse )
:   maObjList(1024, 64, 64),
    mnIndex(0L),
    mbReverse(bReverse)
{
    ImpProcessMarkList(rMarkList, eMode);
    Reset();
}

void SdrObjListIter::ImpProcessObjectList(const SdrObjList& rObjList, SdrIterMode eMode, sal_Bool bUseZOrder)
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
            ImpProcessObj( pObj, eMode, sal_False );
}

void SdrObjListIter::ImpProcessObj(SdrObject* pObj, SdrIterMode eMode, sal_Bool bUseZOrder)
{
    bool bIsGroup = pObj->IsGroupObject();
    // #99190# 3D objects are no group objects, IsGroupObject()
    // only tests if pSub is not null ptr :-(
    if( bIsGroup && pObj->ISA( E3dObject ) && !pObj->ISA( E3dScene ) )
        bIsGroup = false;

    if( !bIsGroup || (eMode != IM_DEEPNOGROUPS) )
        maObjList.Insert( pObj, LIST_APPEND );

    if( bIsGroup && (eMode != IM_FLAT) )
        ImpProcessObjectList( *pObj->GetSubList(), eMode, bUseZOrder );
}
