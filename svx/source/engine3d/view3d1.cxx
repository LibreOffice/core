/*************************************************************************
 *
 *  $RCSfile: view3d1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define ITEMID_COLOR            SID_ATTR_3D_LIGHTCOLOR

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif

#ifndef _SVDPOOL_HXX
#include "svdpool.hxx"
#endif

#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif

#ifndef _XOUTX_HXX
#include "xoutx.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _XTABLE_HXX
#include "xtable.hxx"
#endif

#ifndef _FM_FMVIEW_HXX
#include "fmview.hxx"
#endif

#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _E3D_OBJ3D_HXX
#include "obj3d.hxx"
#endif

#ifndef _E3D_POLYSC3D_HXX
#include "polysc3d.hxx"
#endif

#ifndef _SVXE3DITEM_HXX
#include "e3ditem.hxx"
#endif

#ifndef _SVX_COLRITEM_HXX
#include "colritem.hxx"
#endif

#ifndef _E3D_DLIGHT3D_HXX
#include "dlight3d.hxx"
#endif

#ifndef _E3D_LATHE3D_HXX
#include "lathe3d.hxx"
#endif

#ifndef _E3D_POLYOB3D_HXX
#include "polyob3d.hxx"
#endif

#ifndef _E3D_SPHERE3D_HXX
#include "sphere3d.hxx"
#endif

#ifndef _E3D_EXTRUD3D_HXX
#include "extrud3d.hxx"
#endif

#ifndef _E3D_UNDO_HXX
#include "e3dundo.hxx"
#endif

#ifndef _E3D_VIEW3D_HXX
#include "view3d.hxx"
#endif

#ifndef _E3D_CUBE3D_HXX
#include "cube3d.hxx"
#endif

#ifndef _SVX_XFLCLIT_HXX
#include "xflclit.hxx"
#endif

#ifndef _SVDOGRP_HXX
#include "svdogrp.hxx"
#endif

/*************************************************************************
|*
|* Konvertierung in Polygone
|*
\************************************************************************/

void E3dView::ConvertMarkedToPolyObj(BOOL bLineToArea)
{
    SdrObject* pNewObj = NULL;

    if (aMark.GetMarkCount() == 1)
    {
        SdrObject* pObj = aMark.GetMark(0)->GetObj();

        if (pObj && pObj->ISA(E3dPolyScene))
        {
            BOOL bBezier = FALSE;
            pNewObj = ((E3dPolyScene*) pObj)->ConvertToPolyObj(bBezier, bLineToArea);

            if (pNewObj)
            {
                BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_EXTRUDE));
                ReplaceObject(pObj, *GetPageViewPvNum(0), pNewObj);
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

SfxItemSet E3dView::Get3DAttributes(E3dScene* pInScene,
    BOOL bOnly3DAttr) const
{
    // ItemSet mit entspr. Bereich anlegen
    SfxItemSet aSet(pMod->GetItemPool(),
        SDRATTR_START,  SDRATTR_END,
        SID_ATTR_3D_START, SID_ATTR_3D_END,
        0, 0);

    UINT32 nSelectedItems = 0L;
    if(pInScene)
    {
        // Spezielle Szene
        pInScene->TakeAttributes(aSet, TRUE, TRUE);
        nSelectedItems = 0x0010;
    }
    else
    {
        const SdrMarkList& rMarkList = GetMarkList();
        UINT32 nMarkCnt = rMarkList.GetMarkCount();
        if(nMarkCnt)
        {
            BOOL bSceneSelected = FALSE;
            BOOL b3DSelected = FALSE;

            UINT32 a;
            for(a=0;a<nMarkCnt;a++)
            {
                SdrObject* pObj = aMark.GetMark(a)->GetObj();
                if(pObj)
                {
                    if(pObj->ISA(E3dScene))
                    {
                        bSceneSelected = TRUE;
                        b3DSelected = TRUE;
                    }
                    else if(pObj->ISA(E3dObject))
                    {
                        b3DSelected = TRUE;
                    }
                    else if(pObj->ISA(SdrObjGroup) && !b3DSelected && !bSceneSelected)
                    {
                        // Befinden sich 3D-Objekte in dieser Gruppe?
                        // Mache einen TIEFEN Durchlauf, solange
                        // b3DSelected == FALSE
                        SdrObjListIter aIter(*(pObj->GetSubList()), IM_DEEPWITHGROUPS);
                        while(aIter.IsMore() && !b3DSelected)
                        {
                            SdrObject* pIterObj = aIter.Next();
                            if(pIterObj->ISA(E3dObject))
                                b3DSelected = TRUE;
                        }
                    }
                }
            }

            if(b3DSelected)
            {
                for(a=0;a<nMarkCnt;a++)
                {
                    SdrObject* pObj = aMark.GetMark(a)->GetObj();

                    // Nimm die Szene des ersten selektierten CompoundObjects,
                    // falls keine Szene selektiert war
                    if(!bSceneSelected && pObj && pObj->ISA(E3dObject))
                    {
                        E3dScene* pScene = ((E3dObject*)pObj)->GetScene();
                        if(pScene)
                        {
                            pScene->SetAttrUseSubObjects(FALSE);
                            pScene->TakeAttributes(aSet, TRUE, TRUE);
                            bSceneSelected = TRUE;
                        }
                    }

                    // Das Objekt selbst aufnehmen
                    pObj->TakeAttributes(aSet, TRUE, TRUE);

                    // Selektion Pflegen
                    if(pObj->ISA(E3dLatheObj))
                        nSelectedItems |= 0x0001;
                    else if(pObj->ISA(E3dExtrudeObj))
                        nSelectedItems |= 0x0002;
                    else if(pObj->ISA(E3dSphereObj))
                        nSelectedItems |= 0x0004;
                    else if(pObj->ISA(E3dCubeObj))
                        nSelectedItems |= 0x0008;
                }
            }
            else
            {
                nSelectedItems = 0x0010;
            }
        }
        else
        {
            nSelectedItems = 0x0010;
        }
    }

    // setze SID_ATTR_3D_INTERN auf den Status der selektierten Objekte
    aSet.Put(SfxUInt32Item(SID_ATTR_3D_INTERN, nSelectedItems));

    // DefaultValues pflegen
    if(nSelectedItems == 0x0010 && !pInScene)
    {
        // aus den 3D-Objekten holen
        ((E3dView*)this)->Get3DDefaultAttributes().TakeDefaultValues(aSet);

        // Weitere, alte Attribute
        // Camera Distance (SID_ATTR_3D_DISTANCE)
        double fCamPosZ = DefaultCamPos().Z();
        aSet.Put(SfxUInt32Item(SID_ATTR_3D_DISTANCE, (UINT32)(fCamPosZ + 0.5)));

        // Camera FocalLength (SID_ATTR_3D_FOCAL_LENGTH)
        double fFocalLength = DefaultCamFocal();
        aSet.Put(SfxUInt32Item(SID_ATTR_3D_FOCAL_LENGTH, (INT32)(fFocalLength*100.0)));

        // 2D Defaults holen und hinzufuegen
        SfxItemSet aSet2D(GetModel()->GetItemPool());
        GetAttributes(aSet2D);
        aSet.Put(aSet2D);

        // ... aber keine Linien fuer 3D
        aSet.Put(XLineStyleItem (XLINE_NONE));
    }

    // ItemSet zurueckgeben
    return( aSet );
}

/*************************************************************************
|*
|* Set3DAttributes:
|*
\************************************************************************/

void E3dView::Set3DAttributes( const SfxItemSet& rAttr,
    E3dScene* pInScene, BOOL bReplaceAll)
{
    UINT32 nSelectedItems = 0L;
    if(pInScene)
    {
        // Spezielle Szene
        pInScene->SetAttributes(rAttr, bReplaceAll);
        nSelectedItems = 0x0010;
    }
    else
    {
        const SdrMarkList& rMarkList = GetMarkList();
        UINT32 nMarkCnt = rMarkList.GetMarkCount();
        if(nMarkCnt)
        {
            BOOL bIsSceneSelected = FALSE;
            UINT32 a;
            for(a=0;a<nMarkCnt;a++)
            {
                SdrObject* pObj = aMark.GetMark(a)->GetObj();
                if(pObj && pObj->ISA(E3dScene))
                    bIsSceneSelected = TRUE;
            }

            BOOL bSceneSelected = bIsSceneSelected;
            BegUndo( String( SVX_RES( RID_SVX_3D_UNDO_ATTRIBUTES ) ) );
            for(a=0;a<nMarkCnt;a++)
            {
                SdrObject* pObj = aMark.GetMark(a)->GetObj();
                if(pObj->ISA(E3dObject))
                {
                    // Nimm die Szene des ersten selektierten CompoundObjects,
                    // falls keine Szene selektiert war
                    if(pObj->ISA(E3dObject) && !bSceneSelected)
                    {
                        E3dScene* pScene = ((E3dObject*)pObj)->GetScene();
                        if(pScene)
                        {
                            SfxItemSet aOldSet(pMod->GetItemPool(),
                                SDRATTR_START,  SDRATTR_END,
                                SID_ATTR_3D_START, SID_ATTR_3D_END,
                                0, 0);
                            pScene->SetAttrUseSubObjects(FALSE);
                            pScene->TakeAttributes(aOldSet, TRUE, FALSE);
                            pScene->SetAttrUseSubObjects(FALSE);
                            pScene->SetAttributes(rAttr, bReplaceAll);
                            AddUndo(new E3dAttributesUndoAction(*pMod, this, pScene, rAttr, aOldSet, FALSE));
                            bSceneSelected = TRUE;
                        }
                    }

                    // Das Objekt selbst in UNDO aufnehmen
                    if(!pObj->ISA(SdrObjGroup))
                    {
                        SfxItemSet aOldSet(pMod->GetItemPool(),
                            SDRATTR_START,  SDRATTR_END,
                            SID_ATTR_3D_START, SID_ATTR_3D_END,
                            0, 0);
                        if(pObj->ISA(E3dObject))
                            ((E3dObject*)pObj)->SetAttrUseSubObjects(FALSE);
                        pObj->TakeAttributes(aOldSet, TRUE, FALSE);
                        AddUndo(new E3dAttributesUndoAction(*pMod, this, (E3dObject*)pObj, rAttr, aOldSet, FALSE));
                    }

                    // SubObjekte in UNDO aufnehmen
                    if(pObj->ISA(SdrObjGroup) || pObj->ISA(E3dObject))
                    {
                        SdrObjListIter aIter(*(pObj->GetSubList()), IM_DEEPWITHGROUPS);
                        while(aIter.IsMore())
                        {
                            SdrObject* pIterObj = aIter.Next();
                            SfxItemSet aOldSet(pMod->GetItemPool(),
                                SDRATTR_START,  SDRATTR_END,
                                SID_ATTR_3D_START, SID_ATTR_3D_END,
                                0, 0);
                            if(pIterObj->ISA(E3dObject))
                                ((E3dObject*)pIterObj)->SetAttrUseSubObjects(FALSE);
                            pIterObj->TakeAttributes(aOldSet, TRUE, FALSE);
                            AddUndo(new E3dAttributesUndoAction(*pMod, this, (E3dObject*)pIterObj, rAttr, aOldSet, FALSE));
                        }
                    }

                    // Am Objekt selbst Attribute setzen
                    pObj->SetAttributes(rAttr, bReplaceAll);

                    // Selektion Pflegen
                    if(pObj->ISA(E3dLatheObj))
                        nSelectedItems |= 0x0001;
                    else if(pObj->ISA(E3dExtrudeObj))
                        nSelectedItems |= 0x0002;
                    else if(pObj->ISA(E3dSphereObj))
                        nSelectedItems |= 0x0004;
                    else if(pObj->ISA(E3dCubeObj))
                        nSelectedItems |= 0x0008;
                }
            }
            EndUndo();

            // Nochmaliger Durchlauf mit Korrektur der veraenderten Szenen,
            // da die enthaltenen Objekte geometrisch veraendert sein koennen
            E3dScene* pToBeRefreshedScene = NULL;
            bSceneSelected = bIsSceneSelected;
            for(a=0;a<nMarkCnt;a++)
            {
                SdrObject* pObj = aMark.GetMark(a)->GetObj();
                if(!bSceneSelected && pObj && pObj->ISA(E3dObject))
                {
                    E3dScene* pScene = ((E3dObject*)pObj)->GetScene();
                    if(pScene)
                    {
                        if(pScene != pToBeRefreshedScene)
                        {
                            pScene->CorrectSceneDimensions();
                            pToBeRefreshedScene = pScene;
                        }
                    }
                }
                if(pObj->ISA(E3dScene))
                {
                    E3dScene* pTheScene = (E3dScene*)pObj;
                    if(pTheScene != pToBeRefreshedScene)
                    {
                        pTheScene->CorrectSceneDimensions();
                        pToBeRefreshedScene = pTheScene;
                    }
                }
            }
        }
        else
        {
            nSelectedItems = 0x0010;
        }
    }

    // DefaultValues pflegen
    if(nSelectedItems == 0x0010 && !pInScene)
    {
        // in den 3D-Objekten setzen
        Get3DDefaultAttributes().SetDefaultValues(rAttr);

        // Weitere, alte Attribute
        const SfxPoolItem* pPoolItem = NULL;

        // Camera Distance (SID_ATTR_3D_DISTANCE)
        if(SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_DISTANCE, TRUE, &pPoolItem))
        {
            double fCamPosZ = ((const SfxUInt32Item*)pPoolItem)->GetValue();
            DefaultCamPos() = Vector3D(DefaultCamPos().X(), DefaultCamPos().Y(), fCamPosZ);
        }

        // Camera FocalLength (SID_ATTR_3D_FOCAL_LENGTH)
        if(SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_3D_FOCAL_LENGTH, TRUE, &pPoolItem))
        {
            double fFocalLength = ((double)((const SfxUInt32Item*)pPoolItem)->GetValue()) / 100.0;
            DefaultCamFocal() = fFocalLength;
        }
    }
}


