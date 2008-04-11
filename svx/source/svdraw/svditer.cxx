/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svditer.cxx,v $
 * $Revision: 1.9 $
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
#include "precompiled_svx.hxx"
#include "svditer.hxx"
#include <svx/svdpage.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdobj.hxx>

// #99190#
#include <svx/scene3d.hxx>

SdrObjListIter::SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode, BOOL bReverse)
:   maObjList(1024, 64, 64),
    mnIndex(0L),
    mbReverse(bReverse)
{
    ImpProcessObjectList(rObjList, eMode, TRUE);
    Reset();
}

SdrObjListIter::SdrObjListIter(const SdrObjList& rObjList, BOOL bUseZOrder, SdrIterMode eMode, BOOL bReverse)
:   maObjList(1024, 64, 64),
    mnIndex(0L),
    mbReverse(bReverse)
{
    ImpProcessObjectList(rObjList, eMode, bUseZOrder);
    Reset();
}

SdrObjListIter::SdrObjListIter( const SdrObject& rObj, SdrIterMode eMode, BOOL bReverse )
:   maObjList(1024, 64, 64),
    mnIndex(0L),
    mbReverse(bReverse)
{
    if ( rObj.ISA( SdrObjGroup ) )
        ImpProcessObjectList(*rObj.GetSubList(), eMode, TRUE);
    else
        maObjList.Insert( (void*)&rObj, LIST_APPEND );
    Reset();
}

void SdrObjListIter::ImpProcessObjectList(const SdrObjList& rObjList, SdrIterMode eMode, BOOL bUseZOrder)
{
    for(sal_uInt32 a(0L); a < rObjList.GetObjCount(); a++)
    {
        SdrObject* pObj = NULL;
        if (bUseZOrder)
            pObj = rObjList.GetObj(a);
        else
            pObj = rObjList.GetObjectForNavigationPosition(a);
        if (pObj == NULL)
        {
            OSL_ASSERT(pObj!=NULL);
            continue;
        }

        sal_Bool bIsGroup(pObj->IsGroupObject());

        // #99190# 3D objects are no group objects, IsGroupObject()
        // only tests if pSub is not null ptr :-(
        if(bIsGroup && pObj->ISA(E3dObject) && !pObj->ISA(E3dScene))
            bIsGroup = sal_False;

        if(eMode != IM_DEEPNOGROUPS || !bIsGroup)
            maObjList.Insert(pObj, LIST_APPEND);

        if(bIsGroup && eMode != IM_FLAT)
            ImpProcessObjectList(*pObj->GetSubList(), eMode, bUseZOrder);
    }
}

