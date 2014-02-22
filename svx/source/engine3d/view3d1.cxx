/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <tools/shl.hxx>
#include "svx/svditer.hxx"
#include <svx/svdpool.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svxids.hrc>
#include <svx/xtable.hxx>
#include <svx/fmview.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include "svx/globl3d.hxx"
#include <svx/obj3d.hxx>
#include <svx/polysc3d.hxx>
#include <svx/e3ditem.hxx>
#include <editeng/colritem.hxx>
#include <svx/lathe3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/extrud3d.hxx>
#include <svx/e3dundo.hxx>
#include <svx/view3d.hxx>
#include <svx/cube3d.hxx>
#include <svx/xflclit.hxx>
#include <svx/svdogrp.hxx>
#include <svx/e3dsceneupdater.hxx>

void E3dView::ConvertMarkedToPolyObj(sal_Bool bLineToArea)
{
    SdrObject* pNewObj = NULL;

    if (GetMarkedObjectCount() == 1)
    {
        SdrObject* pObj = GetMarkedObjectByIndex(0);

        if (pObj && pObj->ISA(E3dPolyScene))
        {
            sal_Bool bBezier = sal_False;
            pNewObj = ((E3dPolyScene*) pObj)->ConvertToPolyObj(bBezier, bLineToArea);

            if (pNewObj)
            {
                BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_EXTRUDE));
                ReplaceObjectAtView(pObj, *GetSdrPageView(), pNewObj);
                EndUndo();
            }
        }
    }

    if (!pNewObj)
    {
        SdrView::ConvertMarkedToPolyObj(bLineToArea);
    }
}

void Imp_E3dView_InorderRun3DObjects(const SdrObject* pObj, sal_uInt32& rMask)
{
    if(pObj->ISA(E3dLatheObj))
    {
        rMask |= 0x0001;
    }
    else if(pObj->ISA(E3dExtrudeObj))
    {
        rMask |= 0x0002;
    }
    else if(pObj->ISA(E3dSphereObj))
    {
        rMask |= 0x0004;
    }
    else if(pObj->ISA(E3dCubeObj))
    {
        rMask |= 0x0008;
    }
    else if(pObj->IsGroupObject())
    {
        SdrObjList* pList = pObj->GetSubList();
        for(sal_uInt32 a(0); a < pList->GetObjCount(); a++)
            Imp_E3dView_InorderRun3DObjects(pList->GetObj(a), rMask);
    }
}

SfxItemSet E3dView::Get3DAttributes(E3dScene* pInScene, sal_Bool /*bOnly3DAttr*/) const
{
    
    SfxItemSet aSet(
        pMod->GetItemPool(),
        SDRATTR_START,      SDRATTR_END,
        SID_ATTR_3D_INTERN, SID_ATTR_3D_INTERN,
        0, 0);

    sal_uInt32 nSelectedItems(0L);

    if(pInScene)
    {
        
        aSet.Put(pInScene->GetMergedItemSet());
    }
    else
    {
        
        MergeAttrFromMarked(aSet, sal_False);

        
        const SdrMarkList& rMarkList = GetMarkedObjectList();
        sal_uInt32 nMarkCnt(rMarkList.GetMarkCount());

        for(sal_uInt32 a(0); a < nMarkCnt; a++)
        {
            SdrObject* pObj = GetMarkedObjectByIndex(a);
            Imp_E3dView_InorderRun3DObjects(pObj, nSelectedItems);
        }
    }

    
    aSet.Put(SfxUInt32Item(SID_ATTR_3D_INTERN, nSelectedItems));

    
    if(!nSelectedItems  && !pInScene)
    {
        
        SfxItemSet aDefaultSet(pMod->GetItemPool(), SDRATTR_3D_FIRST, SDRATTR_3D_LAST);
        GetAttributes(aDefaultSet);
        aSet.Put(aDefaultSet);

        
        aSet.Put(XLineStyleItem (XLINE_NONE));

        
        aSet.Put(Svx3DDistanceItem(100));
        aSet.Put(Svx3DFocalLengthItem(10000));
    }

    
    return(aSet);
}

void E3dView::Set3DAttributes( const SfxItemSet& rAttr, E3dScene* pInScene, sal_Bool bReplaceAll)
{
    sal_uInt32 nSelectedItems(0L);

    if(pInScene)
    {
        pInScene->SetMergedItemSetAndBroadcast(rAttr, bReplaceAll);
    }
    else
    {
        
        

        
        SetAttrToMarked(rAttr, bReplaceAll);

        
        const SdrMarkList& rMarkList = GetMarkedObjectList();
        const sal_uInt32 nMarkCnt(rMarkList.GetMarkCount());

        for(sal_uInt32 a(0); a < nMarkCnt; a++)
        {
            SdrObject* pObj = GetMarkedObjectByIndex(a);
            Imp_E3dView_InorderRun3DObjects(pObj, nSelectedItems);
        }
    }

    
    if(!nSelectedItems && !pInScene)
    {
        
        SfxItemSet aDefaultSet(pMod->GetItemPool(), SDRATTR_3D_FIRST, SDRATTR_3D_LAST);
        aDefaultSet.Put(rAttr);
        SetAttributes(aDefaultSet);

    }
}

double E3dView::GetDefaultCamPosZ()
{
    return (double)((const SfxUInt32Item&)pMod->GetItemPool().GetDefaultItem(SDRATTR_3DSCENE_DISTANCE)).GetValue();
}

double E3dView::GetDefaultCamFocal()
{
    return (double)((const SfxUInt32Item&)pMod->GetItemPool().GetDefaultItem(SDRATTR_3DSCENE_FOCAL_LENGTH)).GetValue();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
