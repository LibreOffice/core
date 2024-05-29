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


#include <svl/itempool.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svxids.hrc>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/lathe3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/sphere3d.hxx>
#include <extrud3d.hxx>
#include <svx/view3d.hxx>
#include <svx/cube3d.hxx>
#include <svx/xlineit0.hxx>
#include <com/sun/star/drawing/LineStyle.hpp>

void E3dView::ConvertMarkedToPolyObj()
{
    rtl::Reference<SdrObject> pNewObj;

    if (GetMarkedObjectList().GetMarkCount() == 1)
    {
        SdrObject* pObj = GetMarkedObjectByIndex(0);

        if (pObj)
        {
            const E3dScene* pScene = DynCastE3dScene(pObj);
            if (pScene)
            {
                pNewObj = pScene->ConvertToPolyObj(false/*bBezier*/, false/*bLineToArea*/);
                if (pNewObj)
                {
                    BegUndo(SvxResId(RID_SVX_3D_UNDO_EXTRUDE));
                    ReplaceObjectAtView(pObj, *GetSdrPageView(), pNewObj.get());
                    EndUndo();
                }
            }
        }
    }

    if (!pNewObj)
    {
        SdrView::ConvertMarkedToPolyObj();
    }
}

static void Imp_E3dView_InorderRun3DObjects(const SdrObject& rObj, sal_uInt32& rMask)
{
    if(dynamic_cast< const E3dLatheObj* >(&rObj) !=  nullptr)
    {
        rMask |= 0x0001;
    }
    else if(dynamic_cast< const E3dExtrudeObj* >(&rObj) !=  nullptr)
    {
        rMask |= 0x0002;
    }
    else if(dynamic_cast< const E3dSphereObj* >(&rObj) !=  nullptr)
    {
        rMask |= 0x0004;
    }
    else if(dynamic_cast< const E3dCubeObj* >(&rObj) !=  nullptr)
    {
        rMask |= 0x0008;
    }
    else if (rObj.IsGroupObject())
    {
        SdrObjList* pList = rObj.GetSubList();
        for (const rtl::Reference<SdrObject>& pChildObj : *pList)
            Imp_E3dView_InorderRun3DObjects(*pChildObj, rMask);
    }
}

SfxItemSet E3dView::Get3DAttributes() const
{
    // Creating itemset with corresponding field
    SfxItemPool& rPool = GetModel().GetItemPool();
    SfxItemSet aSet(
        rPool,
        svl::Items<SDRATTR_START, SDRATTR_END, SID_ATTR_3D_INTERN, SID_ATTR_3D_INTERN>);

    sal_uInt32 nSelectedItems(0);

    // get attributes from all selected objects
    MergeAttrFromMarked(aSet, false);

    // calc flags for SID_ATTR_3D_INTERN
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    const size_t nMarkCnt(rMarkList.GetMarkCount());

    for(size_t a = 0; a < nMarkCnt; ++a)
    {
        SdrObject* pObj = GetMarkedObjectByIndex(a);
        Imp_E3dView_InorderRun3DObjects(*pObj, nSelectedItems);
    }

    // Set SID_ATTR_3D_INTERN on the status of the selected objects
    aSet.Put(SfxUInt32Item(SID_ATTR_3D_INTERN, nSelectedItems));

    // maintain default values
    if(!nSelectedItems)
    {
        // Get defaults and apply
        SfxItemSetFixed<SDRATTR_3D_FIRST, SDRATTR_3D_LAST> aDefaultSet(GetModel().GetItemPool());
        GetAttributes(aDefaultSet);
        aSet.Put(aDefaultSet);

        // ... but no lines for 3D
        aSet.Put(XLineStyleItem (css::drawing::LineStyle_NONE));

        // new defaults for distance and focal length
        aSet.Put(makeSvx3DDistanceItem(100));
        aSet.Put(makeSvx3DFocalLengthItem(10000));
    }

    // return ItemSet
    return aSet;
}

void E3dView::Set3DAttributes( const SfxItemSet& rAttr)
{
    sal_uInt32 nSelectedItems(0);

    // #i94832# removed usage of E3DModifySceneSnapRectUpdater here.
    // They are not needed here, they are already handled in SetAttrToMarked

    // set at selected objects
    SetAttrToMarked(rAttr, false/*bReplaceAll*/);

    // old run
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    const size_t nMarkCnt(rMarkList.GetMarkCount());

    for(size_t a = 0; a < nMarkCnt; ++a)
    {
        SdrObject* pObj = GetMarkedObjectByIndex(a);
        Imp_E3dView_InorderRun3DObjects(*pObj, nSelectedItems);
    }

    // Maintain default values
    if(!nSelectedItems)
    {
        // Set defaults
        SfxItemSetFixed<SDRATTR_3D_FIRST, SDRATTR_3D_LAST> aDefaultSet(GetModel().GetItemPool());
        aDefaultSet.Put(rAttr);
        SetAttributes(aDefaultSet);
    }
}

double E3dView::GetDefaultCamPosZ()
{
    return static_cast<double>(GetModel().GetItemPool().GetUserOrPoolDefaultItem(SDRATTR_3DSCENE_DISTANCE).GetValue());
}

double E3dView::GetDefaultCamFocal()
{
    return static_cast<double>(GetModel().GetItemPool().GetUserOrPoolDefaultItem(SDRATTR_3DSCENE_FOCAL_LENGTH).GetValue());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
