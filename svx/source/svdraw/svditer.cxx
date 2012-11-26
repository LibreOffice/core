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

#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdobj.hxx>
#include <svx/scene3d.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjListIter::SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode, bool bReverse)
:   maObjList(),
    mnIndex(0),
    mbReverse(bReverse)
{
    ImpProcessObjectList(rObjList, eMode);
    Reset();
}

SdrObjListIter::SdrObjListIter(const SdrObject& rObj, SdrIterMode eMode, bool bReverse)
:   maObjList(),
    mnIndex(0),
    mbReverse(bReverse)
{
    ImpProcessObj(rObj, eMode); // , true);
    Reset();
}

void SdrObjListIter::ImpProcessObjectList(const SdrObjList& rObjList, SdrIterMode eMode)
{
    for(sal_uInt32 nIdx(0), nCount(rObjList.GetObjCount()); nIdx < nCount; nIdx++)
    {
        SdrObject* pObj = rObjList.GetObj(nIdx);

        if(pObj)
        {
            ImpProcessObj(*pObj, eMode);
        }
        else
        {
            OSL_ENSURE(false, "SdrObjListIter: corrupted SdrObjList (!)");
        }
    }
}

void SdrObjListIter::ImpProcessObj(const SdrObject& rObj, SdrIterMode eMode)
{
    const bool bIsGroup(rObj.getChildrenOfSdrObject());

    if(!bIsGroup || (IM_DEEPNOGROUPS != eMode))
    {
        maObjList.push_back(const_cast< SdrObject* >(&rObj));
    }

    if(bIsGroup && (IM_FLAT != eMode))
    {
        ImpProcessObjectList(*rObj.getChildrenOfSdrObject(), eMode);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
