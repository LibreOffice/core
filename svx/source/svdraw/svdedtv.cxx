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

#include <vcl/metaact.hxx>
#include <svx/svdedtv.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdograf.hxx>  // for Possibilities
#include <svx/svdopath.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpoev.hxx>  // for the PolyPossiblities
#include "svx/svdstr.hrc"   // names taken from the resource
#include "svx/svdglob.hxx"  // StringCache
#include <svx/e3dsceneupdater.hxx>
#include <rtl/strbuf.hxx>

// #i13033#
#include <clonelist.hxx>

void SdrEditView::ImpResetPossibilityFlags()
{
    bReadOnly               =sal_False;

    bGroupPossible          =sal_False;
    bUnGroupPossible        =sal_False;
    bGrpEnterPossible       =sal_False;
    bDeletePossible         =sal_False;
    bToTopPossible          =sal_False;
    bToBtmPossible          =sal_False;
    bReverseOrderPossible   =sal_False;

    bImportMtfPossible      =sal_False;
    bCombinePossible        =sal_False;
    bDismantlePossible      =sal_False;
    bCombineNoPolyPolyPossible =sal_False;
    bDismantleMakeLinesPossible=sal_False;
    bOrthoDesiredOnMarked   =sal_False;

    bMoreThanOneNotMovable  =sal_False;
    bOneOrMoreMovable       =sal_False;
    bMoreThanOneNoMovRot    =sal_False;
    bContortionPossible     =sal_False;
    bAllPolys               =sal_False;
    bOneOrMorePolys         =sal_False;
    bMoveAllowed            =sal_False;
    bResizeFreeAllowed      =sal_False;
    bResizePropAllowed      =sal_False;
    bRotateFreeAllowed      =sal_False;
    bRotate90Allowed        =sal_False;
    bMirrorFreeAllowed      =sal_False;
    bMirror45Allowed        =sal_False;
    bMirror90Allowed        =sal_False;
    bTransparenceAllowed    =sal_False;
    bGradientAllowed        =sal_False;
    bShearAllowed           =sal_False;
    bEdgeRadiusAllowed      =sal_False;
    bCanConvToPath          =sal_False;
    bCanConvToPoly          =sal_False;
    bCanConvToContour       =sal_False;
    bCanConvToPathLineToArea=sal_False;
    bCanConvToPolyLineToArea=sal_False;
    bMoveProtect            =sal_False;
    bResizeProtect          =sal_False;
}

void SdrEditView::ImpClearVars()
{
    ImpResetPossibilityFlags();
    bPossibilitiesDirty=sal_True;   // << Purify didn't like this
    bBundleVirtObj=sal_False;
}

SdrEditView::SdrEditView(SdrModel* pModel1, OutputDevice* pOut):
    SdrMarkView(pModel1,pOut)
{
    ImpClearVars();
}

SdrEditView::~SdrEditView()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrLayer* SdrEditView::InsertNewLayer(const OUString& rName, sal_uInt16 nPos)
{
    SdrLayerAdmin& rLA=pMod->GetLayerAdmin();
    sal_uInt16 nMax=rLA.GetLayerCount();
    if (nPos>nMax) nPos=nMax;
    SdrLayer* pNewLayer=rLA.NewLayer(rName,nPos);

    if( GetModel()->IsUndoEnabled() )
        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewLayer(nPos,rLA,*pMod));

    pMod->SetChanged();
    return pNewLayer;
}

#include <svx/svdogrp.hxx>
#include <svx/scene3d.hxx>

sal_Bool SdrEditView::ImpDelLayerCheck(SdrObjList* pOL, SdrLayerID nDelID) const
{
    sal_Bool bDelAll(sal_True);
    sal_uInt32 nObjAnz(pOL->GetObjCount());

    for(sal_uInt32 nObjNum(nObjAnz); nObjNum > 0 && bDelAll;)
    {
        nObjNum--;
        SdrObject* pObj = pOL->GetObj(nObjNum);
        SdrObjList* pSubOL = pObj->GetSubList();

        // explicitly test for group objects and 3d scenes
        if(pSubOL && (pObj->ISA(SdrObjGroup) || pObj->ISA(E3dScene)))
        {
            if(!ImpDelLayerCheck(pSubOL, nDelID))
            {
                bDelAll = sal_False;
            }
        }
        else
        {
            if(pObj->GetLayer() != nDelID)
            {
                bDelAll = sal_False;
            }
        }
    }

    return bDelAll;
}

void SdrEditView::ImpDelLayerDelObjs(SdrObjList* pOL, SdrLayerID nDelID)
{
    sal_uInt32 nObjAnz(pOL->GetObjCount());
    // make sure OrdNums are correct
    pOL->GetObj(0)->GetOrdNum();

    const bool bUndo = GetModel()->IsUndoEnabled();

    for(sal_uInt32 nObjNum(nObjAnz); nObjNum > 0;)
    {
        nObjNum--;
        SdrObject* pObj = pOL->GetObj(nObjNum);
        SdrObjList* pSubOL = pObj->GetSubList();


        // explicitly test for group objects and 3d scenes
        if(pSubOL && (pObj->ISA(SdrObjGroup) || pObj->ISA(E3dScene)))
        {
            if(ImpDelLayerCheck(pSubOL, nDelID))
            {
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj, true));
                pOL->RemoveObject(nObjNum);

                if( !bUndo )
                    SdrObject::Free( pObj );
            }
            else
            {
                ImpDelLayerDelObjs(pSubOL, nDelID);
            }
        }
        else
        {
            if(pObj->GetLayer() == nDelID)
            {
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj, true));
                pOL->RemoveObject(nObjNum);
                if( !bUndo )
                    SdrObject::Free( pObj );
            }
        }
    }
}

void SdrEditView::DeleteLayer(const OUString& rName)
{
    SdrLayerAdmin& rLA = pMod->GetLayerAdmin();
    SdrLayer* pLayer = rLA.GetLayer(rName, sal_True);
    sal_uInt16 nLayerNum(rLA.GetLayerPos(pLayer));

    if(SDRLAYER_NOTFOUND != nLayerNum)
    {

        SdrLayerID nDelID = pLayer->GetID();

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo(ImpGetResStr(STR_UndoDelLayer));

        bool bMaPg(true);

        for(sal_uInt16 nPageKind(0); nPageKind < 2; nPageKind++)
        {
            // MasterPages and DrawPages
            sal_uInt16 nPgAnz(bMaPg ? pMod->GetMasterPageCount() : pMod->GetPageCount());

            for(sal_uInt16 nPgNum(0); nPgNum < nPgAnz; nPgNum++)
            {
                // over all pages
                SdrPage* pPage = (bMaPg) ? pMod->GetMasterPage(nPgNum) : pMod->GetPage(nPgNum);
                sal_uInt32 nObjAnz(pPage->GetObjCount());

                // make sure OrdNums are correct
                if(nObjAnz)
                    pPage->GetObj(0)->GetOrdNum();

                for(sal_uInt32 nObjNum(nObjAnz); nObjNum > 0;)
                {
                    nObjNum--;
                    SdrObject* pObj = pPage->GetObj(nObjNum);
                    SdrObjList* pSubOL = pObj->GetSubList();

                    // explicitly test for group objects and 3d scenes
                    if(pSubOL && (pObj->ISA(SdrObjGroup) || pObj->ISA(E3dScene)))
                    {
                        if(ImpDelLayerCheck(pSubOL, nDelID))
                        {
                            if( bUndo )
                                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj, true));
                            pPage->RemoveObject(nObjNum);
                            if( !bUndo )
                                SdrObject::Free(pObj);
                        }
                        else
                        {
                            ImpDelLayerDelObjs(pSubOL, nDelID);
                        }
                    }
                    else
                    {
                        if(pObj->GetLayer() == nDelID)
                        {
                            if( bUndo )
                                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj, true));
                            pPage->RemoveObject(nObjNum);
                            if( !bUndo )
                                SdrObject::Free(pObj);
                        }
                    }
                }
            }
            bMaPg = false;
        }

        if( bUndo )
        {
            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteLayer(nLayerNum, rLA, *pMod));
            rLA.RemoveLayer(nLayerNum);
            EndUndo();
        }
        else
        {
            delete rLA.RemoveLayer(nLayerNum);
        }

        pMod->SetChanged();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::EndUndo()
{
    // #i13033#
    // Comparison changed to 1L since EndUndo() is called later now
    // and EndUndo WILL change count to count-1
    if(1L == pMod->GetUndoBracketLevel())
    {
        ImpBroadcastEdgesOfMarkedNodes();
    }

    // #i13033#
    // moved to bottom to still have access to UNDOs inside of
    // ImpBroadcastEdgesOfMarkedNodes()
    pMod->EndUndo();
}

void SdrEditView::ImpBroadcastEdgesOfMarkedNodes()
{
    std::vector<SdrObject*>::const_iterator iterPos;
    const std::vector<SdrObject*>& rAllMarkedObjects = GetTransitiveHullOfMarkedObjects();

    // #i13033#
    // New mechanism to search for necessary disconnections for
    // changed connectors inside the transitive hull of all at
    // the beginning of UNDO selected objects
    for(sal_uInt32 a(0L); a < rAllMarkedObjects.size(); a++)
    {
        SdrEdgeObj* pEdge = PTR_CAST(SdrEdgeObj, rAllMarkedObjects[a]);

        if(pEdge)
        {
            SdrObject* pObj1 = pEdge->GetConnectedNode(sal_False);
            SdrObject* pObj2 = pEdge->GetConnectedNode(sal_True);

            if(pObj1 && !pEdge->CheckNodeConnection(sal_False))
            {
                iterPos = std::find(rAllMarkedObjects.begin(),rAllMarkedObjects.end(),pObj1);

                if (iterPos == rAllMarkedObjects.end())
                {
                    if( IsUndoEnabled() )
                        AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pEdge));
                    pEdge->DisconnectFromNode(sal_False);
                }
            }

            if(pObj2 && !pEdge->CheckNodeConnection(sal_True))
            {
                iterPos = std::find(rAllMarkedObjects.begin(),rAllMarkedObjects.end(),pObj2);

                if (iterPos == rAllMarkedObjects.end())
                {
                    if( IsUndoEnabled() )
                        AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pEdge));
                    pEdge->DisconnectFromNode(sal_True);
                }
            }
        }
    }

    sal_uIntPtr nMarkedEdgeAnz = GetMarkedEdgesOfMarkedNodes().GetMarkCount();
    sal_uInt16 i;

    for (i=0; i<nMarkedEdgeAnz; i++) {
        SdrMark* pEM = GetMarkedEdgesOfMarkedNodes().GetMark(i);
        SdrObject* pEdgeTmp=pEM->GetMarkedSdrObj();
        SdrEdgeObj* pEdge=PTR_CAST(SdrEdgeObj,pEdgeTmp);
        if (pEdge!=NULL) {
            pEdge->SetEdgeTrackDirty();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Possibilities
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::MarkListHasChanged()
{
    SdrMarkView::MarkListHasChanged();
    bPossibilitiesDirty=sal_True;
}

void SdrEditView::ModelHasChanged()
{
    SdrMarkView::ModelHasChanged();
    bPossibilitiesDirty=sal_True;
}

sal_Bool SdrEditView::IsResizeAllowed(sal_Bool bProp) const
{
    ForcePossibilities();
    if (bResizeProtect) return sal_False;
    if (bProp) return bResizePropAllowed;
    return bResizeFreeAllowed;
}

sal_Bool SdrEditView::IsRotateAllowed(sal_Bool b90Deg) const
{
    ForcePossibilities();
    if (bMoveProtect) return sal_False;
    if (b90Deg) return bRotate90Allowed;
    return bRotateFreeAllowed;
}

sal_Bool SdrEditView::IsMirrorAllowed(sal_Bool b45Deg, sal_Bool b90Deg) const
{
    ForcePossibilities();
    if (bMoveProtect) return sal_False;
    if (b90Deg) return bMirror90Allowed;
    if (b45Deg) return bMirror45Allowed;
    return bMirrorFreeAllowed && !bMoveProtect;
}

sal_Bool SdrEditView::IsTransparenceAllowed() const
{
    ForcePossibilities();
    return bTransparenceAllowed;
}

sal_Bool SdrEditView::IsGradientAllowed() const
{
    ForcePossibilities();
    return bGradientAllowed;
}

sal_Bool SdrEditView::IsShearAllowed() const
{
    ForcePossibilities();
    if (bResizeProtect) return sal_False;
    return bShearAllowed;
}

sal_Bool SdrEditView::IsEdgeRadiusAllowed() const
{
    ForcePossibilities();
    return bEdgeRadiusAllowed;
}

sal_Bool SdrEditView::IsCrookAllowed(sal_Bool bNoContortion) const
{
    // CrookMode missing here (no rotations allowed when shearing ...)
    ForcePossibilities();
    if (bNoContortion) {
        if (!bRotateFreeAllowed) return sal_False;
        return !bMoveProtect && bMoveAllowed;
    } else {
        return !bResizeProtect && bContortionPossible;
    }
}

sal_Bool SdrEditView::IsDistortAllowed(sal_Bool bNoContortion) const
{
    ForcePossibilities();
    if (bNoContortion) {
        return sal_False;
    } else {
        return !bResizeProtect && bContortionPossible;
    }
}

sal_Bool SdrEditView::IsCombinePossible(sal_Bool bNoPolyPoly) const
{
    ForcePossibilities();
    if (bNoPolyPoly) return bCombineNoPolyPolyPossible;
    else return bCombinePossible;
}

sal_Bool SdrEditView::IsDismantlePossible(sal_Bool bMakeLines) const
{
    ForcePossibilities();
    if (bMakeLines) return bDismantleMakeLinesPossible;
    else return bDismantlePossible;
}

void SdrEditView::CheckPossibilities()
{
    if (bSomeObjChgdFlag) bPossibilitiesDirty=sal_True;

    if(bSomeObjChgdFlag)
    {
        // This call IS necessary to correct the MarkList, in which
        // no longer to the model belonging objects still can reside.
        // These ones need to be removed.
        CheckMarked();
    }

    if (bPossibilitiesDirty) {
        ImpResetPossibilityFlags();
        SortMarkedObjects();
        sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
        if (nMarkAnz!=0) {
            bReverseOrderPossible=nMarkAnz>=2;

            sal_uIntPtr nMovableCount=0;
            bGroupPossible=nMarkAnz>=2;
            bCombinePossible=nMarkAnz>=2;
            if (nMarkAnz==1) {
                // check bCombinePossible more thoroughly
                // still missing ...
                const SdrObject* pObj=GetMarkedObjectByIndex(0);
                //const SdrPathObj* pPath=PTR_CAST(SdrPathObj,pObj);
                bool bGroup=pObj->GetSubList()!=NULL;
                bool bHasText=pObj->GetOutlinerParaObject()!=NULL;
                if (bGroup || bHasText) {
                    bCombinePossible=sal_True;
                }
            }
            bCombineNoPolyPolyPossible=bCombinePossible;
            bDeletePossible=sal_True;
            // accept transformations for now
            bMoveAllowed      =sal_True;
            bResizeFreeAllowed=sal_True;
            bResizePropAllowed=sal_True;
            bRotateFreeAllowed=sal_True;
            bRotate90Allowed  =sal_True;
            bMirrorFreeAllowed=sal_True;
            bMirror45Allowed  =sal_True;
            bMirror90Allowed  =sal_True;
            bShearAllowed     =sal_True;
            bEdgeRadiusAllowed=sal_False;
            bContortionPossible=sal_True;
            bCanConvToContour = sal_True;

            // these ones are only allowed when single object is selected
            bTransparenceAllowed = (nMarkAnz == 1);
            bGradientAllowed = (nMarkAnz == 1);
            if(bGradientAllowed)
            {
                // gradient depends on fill style
                const SdrMark* pM = GetSdrMarkByIndex(0);
                const SdrObject* pObj = pM->GetMarkedSdrObj();

                // may be group object, so get merged ItemSet
                const SfxItemSet& rSet = pObj->GetMergedItemSet();
                SfxItemState eState = rSet.GetItemState(XATTR_FILLSTYLE, sal_False);

                if(SFX_ITEM_DONTCARE != eState)
                {
                    // If state is not DONTCARE, test the item
                    XFillStyle eFillStyle = ((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();

                    if(eFillStyle != XFILL_GRADIENT)
                    {
                        bGradientAllowed = sal_False;
                    }
                }
            }

            sal_Bool bNoMovRotFound=sal_False;
            const SdrPageView* pPV0=NULL;

            for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++) {
                const SdrMark* pM=GetSdrMarkByIndex(nm);
                const SdrObject* pObj=pM->GetMarkedSdrObj();
                const SdrPageView* pPV=pM->GetPageView();
                if (pPV!=pPV0) {
                    if (pPV->IsReadOnly()) bReadOnly=sal_True;
                    pPV0=pPV;
                }

                SdrObjTransformInfoRec aInfo;
                pObj->TakeObjInfo(aInfo);
                sal_Bool bMovPrt=pObj->IsMoveProtect();
                sal_Bool bSizPrt=pObj->IsResizeProtect();
                if (!bMovPrt && aInfo.bMoveAllowed) nMovableCount++; // count MovableObjs
                if (bMovPrt) bMoveProtect=sal_True;
                if (bSizPrt) bResizeProtect=sal_True;

                // not allowed when not allowed at one object
                if(!aInfo.bTransparenceAllowed)
                    bTransparenceAllowed = sal_False;

                // If one of these can't do something, none can
                if (!aInfo.bMoveAllowed      ) bMoveAllowed      =sal_False;
                if (!aInfo.bResizeFreeAllowed) bResizeFreeAllowed=sal_False;
                if (!aInfo.bResizePropAllowed) bResizePropAllowed=sal_False;
                if (!aInfo.bRotateFreeAllowed) bRotateFreeAllowed=sal_False;
                if (!aInfo.bRotate90Allowed  ) bRotate90Allowed  =sal_False;
                if (!aInfo.bMirrorFreeAllowed) bMirrorFreeAllowed=sal_False;
                if (!aInfo.bMirror45Allowed  ) bMirror45Allowed  =sal_False;
                if (!aInfo.bMirror90Allowed  ) bMirror90Allowed  =sal_False;
                if (!aInfo.bShearAllowed     ) bShearAllowed     =sal_False;
                if (aInfo.bEdgeRadiusAllowed) bEdgeRadiusAllowed=sal_True;
                if (aInfo.bNoContortion      ) bContortionPossible=sal_False;
                // For Crook with Contortion: all objects have to be
                // Movable and Rotatable, except for a maximum of 1 of them
                if (!bMoreThanOneNoMovRot) {
                    if (!aInfo.bMoveAllowed || !aInfo.bResizeFreeAllowed) {
                        bMoreThanOneNoMovRot=bNoMovRotFound;
                        bNoMovRotFound=sal_True;
                    }
                }

                // if one member cannot be converted, no conversion is possible
                if(!aInfo.bCanConvToContour)
                    bCanConvToContour = sal_False;

                // Ungroup
                if (!bUnGroupPossible) bUnGroupPossible=pObj->GetSubList()!=NULL;
                // ConvertToCurve: If at least one can be converted, that is fine.
                if (aInfo.bCanConvToPath          ) bCanConvToPath          =sal_True;
                if (aInfo.bCanConvToPoly          ) bCanConvToPoly          =sal_True;
                if (aInfo.bCanConvToPathLineToArea) bCanConvToPathLineToArea=sal_True;
                if (aInfo.bCanConvToPolyLineToArea) bCanConvToPolyLineToArea=sal_True;

                // Combine/Dismantle
                if(bCombinePossible)
                {
                    bCombinePossible = ImpCanConvertForCombine(pObj);
                    bCombineNoPolyPolyPossible = bCombinePossible;
                }

                if (!bDismantlePossible) bDismantlePossible = ImpCanDismantle(pObj, sal_False);
                if (!bDismantleMakeLinesPossible) bDismantleMakeLinesPossible = ImpCanDismantle(pObj, sal_True);
                // check OrthoDesiredOnMarked
                if (!bOrthoDesiredOnMarked && !aInfo.bNoOrthoDesired) bOrthoDesiredOnMarked=sal_True;
                // check ImportMtf

                if (!bImportMtfPossible)
                {
                    const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(pObj);
                    const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(pObj);

                    if(pSdrGrafObj && ((pSdrGrafObj->HasGDIMetaFile() && !pSdrGrafObj->IsEPS()) || pSdrGrafObj->isEmbeddedSvg()))
                    {
                        bImportMtfPossible = sal_True;
                    }

                    if(pSdrOle2Obj)
                    {
                        bImportMtfPossible = pSdrOle2Obj->GetObjRef().is();
                    }
                }
            }

            bMoreThanOneNotMovable=nMovableCount<nMarkAnz-1;
            bOneOrMoreMovable=nMovableCount!=0;
            bGrpEnterPossible=bUnGroupPossible;
        }
        ImpCheckToTopBtmPossible();
        ((SdrPolyEditView*)this)->ImpCheckPolyPossibilities();
        bPossibilitiesDirty=sal_False;

        if (bReadOnly) {
            sal_Bool bMerker1=bGrpEnterPossible;
            ImpResetPossibilityFlags();
            bReadOnly=sal_True;
            bGrpEnterPossible=bMerker1;
        }
        if (bMoveAllowed) {
            // Don't allow moving glued connectors.
            // Currently only implemented for single selection.
            if (nMarkAnz==1) {
                SdrObject* pObj=GetMarkedObjectByIndex(0);
                SdrEdgeObj* pEdge=PTR_CAST(SdrEdgeObj,pObj);
                if (pEdge!=NULL) {
                    SdrObject* pNode1=pEdge->GetConnectedNode(sal_True);
                    SdrObject* pNode2=pEdge->GetConnectedNode(sal_False);
                    if (pNode1!=NULL || pNode2!=NULL) bMoveAllowed=sal_False;
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::ForceMarkedObjToAnotherPage()
{
    bool bFlg=false;
    for (sal_uIntPtr nm=0; nm<GetMarkedObjectCount(); nm++) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        Rectangle aObjRect(pObj->GetCurrentBoundRect());
        Rectangle aPgRect(pM->GetPageView()->GetPageRect());
        if (!aObjRect.IsOver(aPgRect)) {
            sal_Bool bFnd=sal_False;
            SdrPageView* pPV = GetSdrPageView();

            if(pPV)
            {
                bFnd = aObjRect.IsOver(pPV->GetPageRect());
            }

            if(bFnd)
            {
                pM->GetPageView()->GetObjList()->RemoveObject(pObj->GetOrdNum());
                SdrInsertReason aReason(SDRREASON_VIEWCALL);
                pPV->GetObjList()->InsertObject(pObj,CONTAINER_APPEND,&aReason);
                pM->SetPageView(pPV);
                InvalidateAllWin(aObjRect);
                bFlg=true;
            }
        }
    }
    if (bFlg) {
        MarkListHasChanged();
    }
}

void SdrEditView::DeleteMarkedList(const SdrMarkList& rMark)
{
    if (rMark.GetMarkCount()!=0)
    {
        rMark.ForceSort();

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo();
        const sal_uInt32 nMarkAnz(rMark.GetMarkCount());

        if(nMarkAnz)
        {
            sal_uInt32 nm(0);
            std::vector< E3DModifySceneSnapRectUpdater* > aUpdaters;

            if( bUndo )
            {
                for(nm = nMarkAnz; nm > 0;)
                {
                    nm--;
                    SdrMark* pM = rMark.GetMark(nm);
                    SdrObject* pObj = pM->GetMarkedSdrObj();

                    // extra undo actions for changed connector which now may hold its layed out path (SJ)
                    std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pObj ) );
                    AddUndoActions( vConnectorUndoActions );

                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));
                }
            }

            // make sure, OrderNums are correct:
            rMark.GetMark(0)->GetMarkedSdrObj()->GetOrdNum();

            std::vector< SdrObject* > aRemoved3DObjects;

            for(nm = nMarkAnz; nm > 0;)
            {
                nm--;
                SdrMark* pM = rMark.GetMark(nm);
                SdrObject* pObj = pM->GetMarkedSdrObj();
                SdrObjList*  pOL = pObj->GetObjList();
                const sal_uInt32 nOrdNum(pObj->GetOrdNumDirect());

                bool bIs3D = dynamic_cast< E3dObject* >(pObj);
                // set up a scene updater if object is a 3d object
                if(bIs3D)
                {
                    aUpdaters.push_back(new E3DModifySceneSnapRectUpdater(pObj));
                }

                pOL->RemoveObject(nOrdNum);

                if( !bUndo )
                {
                    if( bIs3D )
                        aRemoved3DObjects.push_back( pObj ); // may be needed later
                    else
                        SdrObject::Free(pObj);
                }
            }

            // fire scene updaters
            while(!aUpdaters.empty())
            {
                delete aUpdaters.back();
                aUpdaters.pop_back();
            }

            if( !bUndo )
            {
                // now delete removed scene objects
                while(!aRemoved3DObjects.empty())
                {
                    SdrObject::Free( aRemoved3DObjects.back() );
                    aRemoved3DObjects.pop_back();
                }
            }
        }

        if( bUndo )
            EndUndo();
    }
}

void SdrEditView::DeleteMarkedObj()
{
    // #i110981# return when nothing is to be done at all
    if(!GetMarkedObjectCount())
    {
        return;
    }

    // moved breaking action and undo start outside loop
    BrkAction();
    BegUndo(ImpGetResStr(STR_EditDelete),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_DELETE);

    // remove as long as something is selected. This allows to schedule objects for
    // removal for a next run as needed
    while(GetMarkedObjectCount())
    {
        // vector to remember the parents which may be empty after object removal
        std::vector< SdrObject* > aParents;

        {
            const SdrMarkList& rMarkList = GetMarkedObjectList();
            const sal_uInt32 nCount(rMarkList.GetMarkCount());
            sal_uInt32 a(0);

            for(a = 0; a < nCount; a++)
            {
                // in the first run, add all found parents, but only once
                SdrMark* pMark = rMarkList.GetMark(a);
                SdrObject* pObject = pMark->GetMarkedSdrObj();
                SdrObject* pParent = pObject->GetObjList()->GetOwnerObj();

                if(pParent)
                {
                    if(!aParents.empty())
                    {
                        std::vector< SdrObject* >::iterator aFindResult =
                            std::find(aParents.begin(), aParents.end(), pParent);

                        if(aFindResult == aParents.end())
                        {
                            aParents.push_back(pParent);
                        }
                    }
                    else
                    {
                        aParents.push_back(pParent);
                    }
                }
            }

            if(!aParents.empty())
            {
                // in a 2nd run, remove all objects which may already be scheduled for
                // removal. I am not sure if this can happen, but theoretically
                // a to-be-removed object may already be the group/3DScene itself
                for(a = 0; a < nCount; a++)
                {
                    SdrMark* pMark = rMarkList.GetMark(a);
                    SdrObject* pObject = pMark->GetMarkedSdrObj();

                    std::vector< SdrObject* >::iterator aFindResult =
                        std::find(aParents.begin(), aParents.end(), pObject);

                    if(aFindResult != aParents.end())
                    {
                        aParents.erase(aFindResult);
                    }
                }
            }
        }

        // original stuff: remove selected objects. Handle clear will
        // do something only once
        DeleteMarkedList(GetMarkedObjectList());
        GetMarkedObjectListWriteAccess().Clear();
        aHdl.Clear();

        while(aParents.size() && !GetMarkedObjectCount())
        {
            // iterate over remembered parents
            SdrObject* pParent = aParents.back();
            aParents.pop_back();

            if(pParent->GetSubList() && 0 == pParent->GetSubList()->GetObjCount())
            {
                // we detected an empty parent, a candidate to leave group/3DScene
                // if entered
                if(GetSdrPageView()->GetAktGroup()
                    && GetSdrPageView()->GetAktGroup() == pParent)
                {
                    GetSdrPageView()->LeaveOneGroup();
                }

                // schedule empty parent for removal
                GetMarkedObjectListWriteAccess().InsertEntry(
                    SdrMark(pParent, GetSdrPageView()));
            }
        }
    }

    // end undo and change messaging moved at the end
    EndUndo();
    MarkListHasChanged();
}

void SdrEditView::CopyMarkedObj()
{
    SortMarkedObjects();

    SdrMarkList aSourceObjectsForCopy(GetMarkedObjectList());
    // The following loop is used instead of MarkList::Merge(), to be
    // able to flag the MarkEntries.
    sal_uIntPtr nEdgeAnz = GetEdgesOfMarkedNodes().GetMarkCount();
    for (sal_uIntPtr nEdgeNum=0; nEdgeNum<nEdgeAnz; nEdgeNum++) {
        SdrMark aM(*GetEdgesOfMarkedNodes().GetMark(nEdgeNum));
        aM.SetUser(1);
        aSourceObjectsForCopy.InsertEntry(aM);
    }
    aSourceObjectsForCopy.ForceSort();

    // #i13033#
    // New mechanism to re-create the connections of cloned connectors
    CloneList aCloneList;

    const bool bUndo = IsUndoEnabled();

    GetMarkedObjectListWriteAccess().Clear();
    sal_uIntPtr nCloneErrCnt=0;
    sal_uIntPtr nMarkAnz=aSourceObjectsForCopy.GetMarkCount();
    sal_uIntPtr nm;
    for (nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=aSourceObjectsForCopy.GetMark(nm);
        SdrObject* pO=pM->GetMarkedSdrObj()->Clone();
        if (pO!=NULL) {
            SdrInsertReason aReason(SDRREASON_VIEWCALL);
            pM->GetPageView()->GetObjList()->InsertObject(pO,CONTAINER_APPEND,&aReason);

            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoCopyObject(*pO));

            SdrMark aME(*pM);
            aME.SetMarkedSdrObj(pO);
            aCloneList.AddPair(pM->GetMarkedSdrObj(), pO);

            if (pM->GetUser()==0)
            {
                // otherwise it is only an Edge we have to copy as well
                GetMarkedObjectListWriteAccess().InsertEntry(aME);
            }
        } else {
            nCloneErrCnt++;
        }
    }

    // #i13033#
    // New mechanism to re-create the connections of cloned connectors
    aCloneList.CopyConnections();

    if(0L != nCloneErrCnt)
    {
#ifdef DBG_UTIL
        OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
            "SdrEditView::CopyMarkedObj(): Error when cloning "));

        if(nCloneErrCnt == 1)
        {
            aStr.append(RTL_CONSTASCII_STRINGPARAM("a drawing object."));
        }
        else
        {
            aStr.append(static_cast<sal_Int32>(nCloneErrCnt));
            aStr.append(RTL_CONSTASCII_STRINGPARAM(" drawing objects."));
        }

        aStr.append(RTL_CONSTASCII_STRINGPARAM(
            " This object's/These objects's connections will not be copied."));
        OSL_FAIL(aStr.getStr());
#endif
    }
    MarkListHasChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrEditView::InsertObjectAtView(SdrObject* pObj, SdrPageView& rPV, sal_uIntPtr nOptions)
{
    if ((nOptions & SDRINSERT_SETDEFLAYER)!=0) {
        SdrLayerID nLayer=rPV.GetPage()->GetLayerAdmin().GetLayerID(aAktLayer,sal_True);
        if (nLayer==SDRLAYER_NOTFOUND) nLayer=0;
        if (rPV.GetLockedLayers().IsSet(nLayer) || !rPV.GetVisibleLayers().IsSet(nLayer)) {
            SdrObject::Free( pObj ); // Layer locked or invisible
            return sal_False;
        }
        pObj->NbcSetLayer(nLayer);
    }
    if ((nOptions & SDRINSERT_SETDEFATTR)!=0) {
        if (pDefaultStyleSheet!=NULL) pObj->NbcSetStyleSheet(pDefaultStyleSheet, sal_False);
        pObj->SetMergedItemSet(aDefaultAttr);
    }
    if (!pObj->IsInserted()) {
        SdrInsertReason aReason(SDRREASON_VIEWCALL);
        if ((nOptions & SDRINSERT_NOBROADCAST)!=0) {
            rPV.GetObjList()->NbcInsertObject(pObj,CONTAINER_APPEND,&aReason);
        } else {
            rPV.GetObjList()->InsertObject(pObj,CONTAINER_APPEND,&aReason);
        }
    }
    if( IsUndoEnabled() )
        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pObj));

    if ((nOptions & SDRINSERT_DONTMARK)==0) {
        if ((nOptions & SDRINSERT_ADDMARK)==0) UnmarkAllObj();
        MarkObj(pObj,&rPV);
    }
    return sal_True;
}

void SdrEditView::ReplaceObjectAtView(SdrObject* pOldObj, SdrPageView& rPV, SdrObject* pNewObj, sal_Bool bMark)
{
    SdrObjList* pOL=pOldObj->GetObjList();
    const bool bUndo = IsUndoEnabled();
    if( bUndo  )
        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoReplaceObject(*pOldObj,*pNewObj));

    if( IsObjMarked( pOldObj ) )
        MarkObj( pOldObj, &rPV, sal_True /*unmark!*/ );

    pOL->ReplaceObject(pNewObj,pOldObj->GetOrdNum());

    if( !bUndo )
        SdrObject::Free( pOldObj );

    if (bMark) MarkObj(pNewObj,&rPV);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrEditView::IsUndoEnabled() const
{
    return pMod->IsUndoEnabled();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
