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
#include "precompiled_svx.hxx"

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

/*************************************************************************
|*
|* Konvertierung in Polygone
|*
\************************************************************************/

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

/*************************************************************************
|*
|* Get3DAttributes
|*
\************************************************************************/

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
    // ItemSet mit entspr. Bereich anlegen
    SfxItemSet aSet(
        pMod->GetItemPool(),
        SDRATTR_START,      SDRATTR_END,
        SID_ATTR_3D_INTERN, SID_ATTR_3D_INTERN,
        0, 0);

    sal_uInt32 nSelectedItems(0L);

    if(pInScene)
    {
        // special scene
        aSet.Put(pInScene->GetMergedItemSet());
    }
    else
    {
        // get attributes from all selected objects
        MergeAttrFromMarked(aSet, sal_False);

        // calc flags for SID_ATTR_3D_INTERN
        const SdrMarkList& rMarkList = GetMarkedObjectList();
        sal_uInt32 nMarkCnt(rMarkList.GetMarkCount());

        for(sal_uInt32 a(0); a < nMarkCnt; a++)
        {
            SdrObject* pObj = GetMarkedObjectByIndex(a);
            Imp_E3dView_InorderRun3DObjects(pObj, nSelectedItems);
        }
    }

    // setze SID_ATTR_3D_INTERN auf den Status der selektierten Objekte
    aSet.Put(SfxUInt32Item(SID_ATTR_3D_INTERN, nSelectedItems));

    // DefaultValues pflegen
    if(!nSelectedItems  && !pInScene)
    {
        // Defaults holen und hinzufuegen
        SfxItemSet aDefaultSet(pMod->GetItemPool(), SDRATTR_3D_FIRST, SDRATTR_3D_LAST);
        GetAttributes(aDefaultSet);
        aSet.Put(aDefaultSet);

        // ... aber keine Linien fuer 3D
        aSet.Put(XLineStyleItem (XLINE_NONE));

        // #84061# new defaults for distance and focal length
        aSet.Put(Svx3DDistanceItem(100));
        aSet.Put(Svx3DFocalLengthItem(10000));
    }

    // ItemSet zurueckgeben
    return(aSet);
}

/*************************************************************************
|*
|* Set3DAttributes:
|*
\************************************************************************/

void E3dView::Set3DAttributes( const SfxItemSet& rAttr, E3dScene* pInScene, sal_Bool bReplaceAll)
{
    sal_uInt32 nSelectedItems(0L);

    if(pInScene)
    {
        //pInScene->SetItemSetAndBroadcast(rAttr, bReplaceAll);
        pInScene->SetMergedItemSetAndBroadcast(rAttr, bReplaceAll);
    }
    else
    {
        // #i94832# removed usage of E3DModifySceneSnapRectUpdater here.
        // They are not needed here, they are already handled in SetAttrToMarked

        // set at selected objects
        SetAttrToMarked(rAttr, bReplaceAll);

        // old run
        const SdrMarkList& rMarkList = GetMarkedObjectList();
        const sal_uInt32 nMarkCnt(rMarkList.GetMarkCount());

        for(sal_uInt32 a(0); a < nMarkCnt; a++)
        {
            SdrObject* pObj = GetMarkedObjectByIndex(a);
            Imp_E3dView_InorderRun3DObjects(pObj, nSelectedItems);
        }
    }

    // DefaultValues pflegen
    if(!nSelectedItems && !pInScene)
    {
        // Defaults setzen
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
