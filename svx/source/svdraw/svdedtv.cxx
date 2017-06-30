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
#include <svx/svdograf.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpoev.hxx>
#include "svx/svdstr.hrc"
#include "svdglob.hxx"
#include <svx/e3dsceneupdater.hxx>
#include <rtl/strbuf.hxx>
#include <svx/svdview.hxx>
#include <clonelist.hxx>
#include <svx/svdogrp.hxx>
#include <svx/scene3d.hxx>


using namespace com::sun::star;

void SdrEditView::ImpResetPossibilityFlags()
{
    bReadOnly               =false;

    bGroupPossible          =false;
    bUnGroupPossible        =false;
    bGrpEnterPossible       =false;
    bDeletePossible         =false;
    bToTopPossible          =false;
    bToBtmPossible          =false;
    bReverseOrderPossible   =false;

    bImportMtfPossible      =false;
    bCombinePossible        =false;
    bDismantlePossible      =false;
    bCombineNoPolyPolyPossible =false;
    bDismantleMakeLinesPossible=false;
    bOrthoDesiredOnMarked   =false;

    bMoreThanOneNotMovable  =false;
    bOneOrMoreMovable       =false;
    bMoreThanOneNoMovRot    =false;
    bContortionPossible     =false;
    bMoveAllowed            =false;
    bResizeFreeAllowed      =false;
    bResizePropAllowed      =false;
    bRotateFreeAllowed      =false;
    bRotate90Allowed        =false;
    bMirrorFreeAllowed      =false;
    bMirror45Allowed        =false;
    bMirror90Allowed        =false;
    bTransparenceAllowed    =false;
    bCropAllowed            =false;
    bGradientAllowed        =false;
    bShearAllowed           =false;
    bEdgeRadiusAllowed      =false;
    bCanConvToPath          =false;
    bCanConvToPoly          =false;
    bCanConvToContour       =false;
    bCanConvToPathLineToArea=false;
    bCanConvToPolyLineToArea=false;
    bMoveProtect            =false;
    bResizeProtect          =false;
}

void SdrEditView::ImpClearVars()
{
    ImpResetPossibilityFlags();
    bPossibilitiesDirty=true;   // << Purify didn't like this
    bBundleVirtObj=false;
}

SdrEditView::SdrEditView(SdrModel* pModel1, OutputDevice* pOut):
    SdrMarkView(pModel1,pOut)
{
    ImpClearVars();
}

SdrEditView::~SdrEditView()
{
}


void SdrEditView::InsertNewLayer(const OUString& rName, sal_uInt16 nPos)
{
    SdrLayerAdmin& rLA=mpModel->GetLayerAdmin();
    sal_uInt16 nMax=rLA.GetLayerCount();
    if (nPos>nMax) nPos=nMax;
    rLA.NewLayer(rName,nPos);

    if( GetModel()->IsUndoEnabled() )
        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewLayer(nPos,rLA,*mpModel));

    mpModel->SetChanged();
}

bool SdrEditView::ImpDelLayerCheck(SdrObjList* pOL, SdrLayerID nDelID) const
{
    bool bDelAll(true);

    for(size_t nObjNum = pOL->GetObjCount(); nObjNum > 0 && bDelAll;)
    {
        nObjNum--;
        SdrObject* pObj = pOL->GetObj(nObjNum);
        SdrObjList* pSubOL = pObj->GetSubList();

        // explicitly test for group objects and 3d scenes
        if(pSubOL && (dynamic_cast<const SdrObjGroup*>(pObj) != nullptr || dynamic_cast< const E3dScene* >(pObj) !=  nullptr))
        {
            if(!ImpDelLayerCheck(pSubOL, nDelID))
            {
                bDelAll = false;
            }
        }
        else
        {
            if(pObj->GetLayer() != nDelID)
            {
                bDelAll = false;
            }
        }
    }

    return bDelAll;
}

void SdrEditView::ImpDelLayerDelObjs(SdrObjList* pOL, SdrLayerID nDelID)
{
    const size_t nObjCount(pOL->GetObjCount());
    // make sure OrdNums are correct
    pOL->GetObj(0)->GetOrdNum();

    const bool bUndo = GetModel()->IsUndoEnabled();

    for(size_t nObjNum = nObjCount; nObjNum > 0;)
    {
        nObjNum--;
        SdrObject* pObj = pOL->GetObj(nObjNum);
        SdrObjList* pSubOL = pObj->GetSubList();


        // explicitly test for group objects and 3d scenes
        if(pSubOL && (dynamic_cast<const SdrObjGroup*>( pObj) != nullptr || dynamic_cast<const E3dScene* >(pObj) !=  nullptr))
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
    SdrLayerAdmin& rLA = mpModel->GetLayerAdmin();
    SdrLayer* pLayer = rLA.GetLayer(rName);

    if(pLayer)
    {
        sal_uInt16 nLayerNum(rLA.GetLayerPos(pLayer));
        SdrLayerID nDelID = pLayer->GetID();

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo(ImpGetResStr(STR_UndoDelLayer));

        bool bMaPg(true);

        for(sal_uInt16 nPageKind(0); nPageKind < 2; nPageKind++)
        {
            // MasterPages and DrawPages
            sal_uInt16 nPgAnz(bMaPg ? mpModel->GetMasterPageCount() : mpModel->GetPageCount());

            for(sal_uInt16 nPgNum(0); nPgNum < nPgAnz; nPgNum++)
            {
                // over all pages
                SdrPage* pPage = (bMaPg) ? mpModel->GetMasterPage(nPgNum) : mpModel->GetPage(nPgNum);
                const size_t nObjCount(pPage->GetObjCount());

                // make sure OrdNums are correct
                if(nObjCount)
                    pPage->GetObj(0)->GetOrdNum();

                for(size_t nObjNum(nObjCount); nObjNum > 0;)
                {
                    nObjNum--;
                    SdrObject* pObj = pPage->GetObj(nObjNum);
                    SdrObjList* pSubOL = pObj->GetSubList();

                    // explicitly test for group objects and 3d scenes
                    if(pSubOL && (dynamic_cast<const SdrObjGroup*>(pObj) != nullptr || dynamic_cast<const E3dScene* >(pObj) !=  nullptr))
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
            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteLayer(nLayerNum, rLA, *mpModel));
            rLA.RemoveLayer(nLayerNum);
            EndUndo();
        }
        else
        {
            delete rLA.RemoveLayer(nLayerNum);
        }

        mpModel->SetChanged();
    }
}


void SdrEditView::EndUndo()
{
    // #i13033#
    // Comparison changed to 1L since EndUndo() is called later now
    // and EndUndo WILL change count to count-1
    if(1L == mpModel->GetUndoBracketLevel())
    {
        ImpBroadcastEdgesOfMarkedNodes();
    }

    // #i13033#
    // moved to bottom to still have access to UNDOs inside of
    // ImpBroadcastEdgesOfMarkedNodes()
    mpModel->EndUndo();
}

void SdrEditView::ImpBroadcastEdgesOfMarkedNodes()
{
    std::vector<SdrObject*>::const_iterator iterPos;
    const std::vector<SdrObject*>& rAllMarkedObjects = GetTransitiveHullOfMarkedObjects();

    // #i13033#
    // New mechanism to search for necessary disconnections for
    // changed connectors inside the transitive hull of all at
    // the beginning of UNDO selected objects
    for(size_t a(0); a < rAllMarkedObjects.size(); a++)
    {
        SdrEdgeObj* pEdge = dynamic_cast<SdrEdgeObj*>( rAllMarkedObjects[a] );

        if(pEdge)
        {
            SdrObject* pObj1 = pEdge->GetConnectedNode(false);
            SdrObject* pObj2 = pEdge->GetConnectedNode(true);

            if(pObj1 && !pEdge->CheckNodeConnection(false))
            {
                iterPos = std::find(rAllMarkedObjects.begin(),rAllMarkedObjects.end(),pObj1);

                if (iterPos == rAllMarkedObjects.end())
                {
                    if( IsUndoEnabled() )
                        AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pEdge));
                    pEdge->DisconnectFromNode(false);
                }
            }

            if(pObj2 && !pEdge->CheckNodeConnection(true))
            {
                iterPos = std::find(rAllMarkedObjects.begin(),rAllMarkedObjects.end(),pObj2);

                if (iterPos == rAllMarkedObjects.end())
                {
                    if( IsUndoEnabled() )
                        AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pEdge));
                    pEdge->DisconnectFromNode(true);
                }
            }
        }
    }

    const size_t nMarkedEdgeAnz = GetMarkedEdgesOfMarkedNodes().GetMarkCount();

    for (size_t i=0; i<nMarkedEdgeAnz; ++i) {
        SdrMark* pEM = GetMarkedEdgesOfMarkedNodes().GetMark(i);
        SdrObject* pEdgeTmp=pEM->GetMarkedSdrObj();
        SdrEdgeObj* pEdge=dynamic_cast<SdrEdgeObj*>( pEdgeTmp );
        if (pEdge!=nullptr) {
            pEdge->SetEdgeTrackDirty();
        }
    }
}


// Possibilities


void SdrEditView::MarkListHasChanged()
{
    SdrMarkView::MarkListHasChanged();
    bPossibilitiesDirty=true;
}

void SdrEditView::ModelHasChanged()
{
    SdrMarkView::ModelHasChanged();
    bPossibilitiesDirty=true;
}

bool SdrEditView::IsResizeAllowed(bool bProp) const
{
    ForcePossibilities();
    if (bResizeProtect) return false;
    if (bProp) return bResizePropAllowed;
    return bResizeFreeAllowed;
}

bool SdrEditView::IsRotateAllowed(bool b90Deg) const
{
    ForcePossibilities();
    if (bMoveProtect) return false;
    if (b90Deg) return bRotate90Allowed;
    return bRotateFreeAllowed;
}

bool SdrEditView::IsMirrorAllowed(bool b45Deg, bool b90Deg) const
{
    ForcePossibilities();
    if (bMoveProtect) return false;
    if (b90Deg) return bMirror90Allowed;
    if (b45Deg) return bMirror45Allowed;
    return bMirrorFreeAllowed && !bMoveProtect;
}

bool SdrEditView::IsTransparenceAllowed() const
{
    ForcePossibilities();
    return bTransparenceAllowed;
}

bool SdrEditView::IsCropAllowed() const
{
    ForcePossibilities();
    return bCropAllowed;
}

bool SdrEditView::IsGradientAllowed() const
{
    ForcePossibilities();
    return bGradientAllowed;
}

bool SdrEditView::IsShearAllowed() const
{
    ForcePossibilities();
    if (bResizeProtect) return false;
    return bShearAllowed;
}

bool SdrEditView::IsEdgeRadiusAllowed() const
{
    ForcePossibilities();
    return bEdgeRadiusAllowed;
}

bool SdrEditView::IsCrookAllowed(bool bNoContortion) const
{
    // CrookMode missing here (no rotations allowed when shearing ...)
    ForcePossibilities();
    if (bNoContortion) {
        if (!bRotateFreeAllowed) return false;
        return !bMoveProtect && bMoveAllowed;
    } else {
        return !bResizeProtect && bContortionPossible;
    }
}

bool SdrEditView::IsDistortAllowed(bool bNoContortion) const
{
    ForcePossibilities();
    if (bNoContortion) {
        return false;
    } else {
        return !bResizeProtect && bContortionPossible;
    }
}

bool SdrEditView::IsCombinePossible(bool bNoPolyPoly) const
{
    ForcePossibilities();
    if (bNoPolyPoly) return bCombineNoPolyPolyPossible;
    else return bCombinePossible;
}

bool SdrEditView::IsDismantlePossible(bool bMakeLines) const
{
    ForcePossibilities();
    if (bMakeLines) return bDismantleMakeLinesPossible;
    else return bDismantlePossible;
}

void SdrEditView::CheckPossibilities()
{
    if (mbSomeObjChgdFlag) bPossibilitiesDirty=true;

    if(mbSomeObjChgdFlag)
    {
        // This call IS necessary to correct the MarkList, in which
        // no longer to the model belonging objects still can reside.
        // These ones need to be removed.
        CheckMarked();
    }

    if (bPossibilitiesDirty) {
        ImpResetPossibilityFlags();
        SortMarkedObjects();
        const size_t nMarkCount=GetMarkedObjectCount();
        if (nMarkCount!=0) {
            bReverseOrderPossible=nMarkCount>=2;

            size_t nMovableCount=0;
            bGroupPossible=nMarkCount>=2;
            bCombinePossible=nMarkCount>=2;
            if (nMarkCount==1) {
                // check bCombinePossible more thoroughly
                // still missing ...
                const SdrObject* pObj=GetMarkedObjectByIndex(0);
                //const SdrPathObj* pPath=dynamic_cast<SdrPathObj*>( pObj );
                bool bGroup=pObj->GetSubList()!=nullptr;
                bool bHasText=pObj->GetOutlinerParaObject()!=nullptr;
                if (bGroup || bHasText) {
                    bCombinePossible=true;
                }
            }
            bCombineNoPolyPolyPossible=bCombinePossible;
            bDeletePossible=true;
            // accept transformations for now
            bMoveAllowed      =true;
            bResizeFreeAllowed=true;
            bResizePropAllowed=true;
            bRotateFreeAllowed=true;
            bRotate90Allowed  =true;
            bMirrorFreeAllowed=true;
            bMirror45Allowed  =true;
            bMirror90Allowed  =true;
            bShearAllowed     =true;
            bEdgeRadiusAllowed=false;
            bContortionPossible=true;
            bCanConvToContour = true;

            // these ones are only allowed when single object is selected
            bTransparenceAllowed = (nMarkCount == 1);
            bGradientAllowed = (nMarkCount == 1);
            bCropAllowed = (nMarkCount == 1);
            if(bGradientAllowed)
            {
                // gradient depends on fill style
                const SdrMark* pM = GetSdrMarkByIndex(0);
                const SdrObject* pObj = pM->GetMarkedSdrObj();

                // may be group object, so get merged ItemSet
                const SfxItemSet& rSet = pObj->GetMergedItemSet();
                SfxItemState eState = rSet.GetItemState(XATTR_FILLSTYLE, false);

                if(SfxItemState::DONTCARE != eState)
                {
                    // If state is not DONTCARE, test the item
                    drawing::FillStyle eFillStyle = static_cast<const XFillStyleItem&>(rSet.Get(XATTR_FILLSTYLE)).GetValue();

                    if(eFillStyle != drawing::FillStyle_GRADIENT)
                    {
                        bGradientAllowed = false;
                    }
                }
            }

            bool bNoMovRotFound=false;
            const SdrPageView* pPV0=nullptr;

            for (size_t nm=0; nm<nMarkCount; ++nm) {
                const SdrMark* pM=GetSdrMarkByIndex(nm);
                const SdrObject* pObj=pM->GetMarkedSdrObj();
                const SdrPageView* pPV=pM->GetPageView();
                if (pPV!=pPV0) {
                    if (pPV->IsReadOnly()) bReadOnly=true;
                    pPV0=pPV;
                }

                SdrObjTransformInfoRec aInfo;
                pObj->TakeObjInfo(aInfo);
                bool bMovPrt=pObj->IsMoveProtect();
                bool bSizPrt=pObj->IsResizeProtect();
                if (!bMovPrt && aInfo.bMoveAllowed) nMovableCount++; // count MovableObjs
                if (bMovPrt) bMoveProtect=true;
                if (bSizPrt) bResizeProtect=true;

                // not allowed when not allowed at one object
                if(!aInfo.bTransparenceAllowed)
                    bTransparenceAllowed = false;

                // If one of these can't do something, none can
                if (!aInfo.bMoveAllowed      ) bMoveAllowed      =false;
                if (!aInfo.bResizeFreeAllowed) bResizeFreeAllowed=false;
                if (!aInfo.bResizePropAllowed) bResizePropAllowed=false;
                if (!aInfo.bRotateFreeAllowed) bRotateFreeAllowed=false;
                if (!aInfo.bRotate90Allowed  ) bRotate90Allowed  =false;
                if (!aInfo.bMirrorFreeAllowed) bMirrorFreeAllowed=false;
                if (!aInfo.bMirror45Allowed  ) bMirror45Allowed  =false;
                if (!aInfo.bMirror90Allowed  ) bMirror90Allowed  =false;
                if (!aInfo.bShearAllowed     ) bShearAllowed     =false;
                if (aInfo.bEdgeRadiusAllowed) bEdgeRadiusAllowed=true;
                if (aInfo.bNoContortion      ) bContortionPossible=false;
                // For Crook with Contortion: all objects have to be
                // Movable and Rotatable, except for a maximum of 1 of them
                if (!bMoreThanOneNoMovRot) {
                    if (!aInfo.bMoveAllowed || !aInfo.bResizeFreeAllowed) {
                        bMoreThanOneNoMovRot=bNoMovRotFound;
                        bNoMovRotFound=true;
                    }
                }

                // Must be resizeable to allow cropping
                if (!aInfo.bResizeFreeAllowed && !aInfo.bResizePropAllowed)
                    bCropAllowed = false;

                // if one member cannot be converted, no conversion is possible
                if(!aInfo.bCanConvToContour)
                    bCanConvToContour = false;

                // Ungroup
                if (!bUnGroupPossible) bUnGroupPossible=pObj->GetSubList()!=nullptr;
                // ConvertToCurve: If at least one can be converted, that is fine.
                if (aInfo.bCanConvToPath          ) bCanConvToPath          =true;
                if (aInfo.bCanConvToPoly          ) bCanConvToPoly          =true;
                if (aInfo.bCanConvToPathLineToArea) bCanConvToPathLineToArea=true;
                if (aInfo.bCanConvToPolyLineToArea) bCanConvToPolyLineToArea=true;

                // Combine/Dismantle
                if(bCombinePossible)
                {
                    bCombinePossible = ImpCanConvertForCombine(pObj);
                    bCombineNoPolyPolyPossible = bCombinePossible;
                }

                if (!bDismantlePossible) bDismantlePossible = ImpCanDismantle(pObj, false);
                if (!bDismantleMakeLinesPossible) bDismantleMakeLinesPossible = ImpCanDismantle(pObj, true);
                // check OrthoDesiredOnMarked
                if (!bOrthoDesiredOnMarked && !aInfo.bNoOrthoDesired) bOrthoDesiredOnMarked=true;
                // check ImportMtf

                if (!bImportMtfPossible)
                {
                    const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(pObj);
                    const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(pObj);

                    if(pSdrGrafObj && ((pSdrGrafObj->HasGDIMetaFile() && !pSdrGrafObj->IsEPS()) || pSdrGrafObj->isEmbeddedSvg()))
                    {
                        bImportMtfPossible = true;
                    }

                    if(pSdrOle2Obj)
                    {
                        bImportMtfPossible = pSdrOle2Obj->GetObjRef().is();
                    }
                }
            }

            bMoreThanOneNotMovable=nMovableCount<nMarkCount-1;
            bOneOrMoreMovable=nMovableCount!=0;
            bGrpEnterPossible=bUnGroupPossible;
        }
        ImpCheckToTopBtmPossible();
        static_cast<SdrPolyEditView*>(this)->ImpCheckPolyPossibilities();
        bPossibilitiesDirty=false;

        if (bReadOnly) {
            bool bMerker1=bGrpEnterPossible;
            ImpResetPossibilityFlags();
            bReadOnly=true;
            bGrpEnterPossible=bMerker1;
        }
        if (bMoveAllowed) {
            // Don't allow moving glued connectors.
            // Currently only implemented for single selection.
            if (nMarkCount==1) {
                SdrObject* pObj=GetMarkedObjectByIndex(0);
                SdrEdgeObj* pEdge=dynamic_cast<SdrEdgeObj*>( pObj );
                if (pEdge!=nullptr) {
                    SdrObject* pNode1=pEdge->GetConnectedNode(true);
                    SdrObject* pNode2=pEdge->GetConnectedNode(false);
                    if (pNode1!=nullptr || pNode2!=nullptr) bMoveAllowed=false;
                }
            }
        }
    }
}


void SdrEditView::ForceMarkedObjToAnotherPage()
{
    bool bFlg=false;
    for (size_t nm=0; nm<GetMarkedObjectCount(); ++nm) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        tools::Rectangle aObjRect(pObj->GetCurrentBoundRect());
        tools::Rectangle aPgRect(pM->GetPageView()->GetPageRect());
        if (!aObjRect.IsOver(aPgRect)) {
            bool bFnd=false;
            SdrPageView* pPV = GetSdrPageView();

            if(pPV)
            {
                bFnd = aObjRect.IsOver(pPV->GetPageRect());
            }

            if(bFnd)
            {
                pM->GetPageView()->GetObjList()->RemoveObject(pObj->GetOrdNum());
                pPV->GetObjList()->InsertObject(pObj, SAL_MAX_SIZE);
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

std::vector<SdrObject*> SdrEditView::DeleteMarkedList(SdrMarkList const& rMark)
{
    std::vector<SdrObject*> ret;
    if (rMark.GetMarkCount()!=0)
    {
        rMark.ForceSort();

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo();
        const size_t nMarkCount(rMark.GetMarkCount());

        if(nMarkCount)
        {
            std::vector< E3DModifySceneSnapRectUpdater* > aUpdaters;

            if( bUndo )
            {
                for(size_t nm = nMarkCount; nm > 0;)
                {
                    --nm;
                    SdrMark* pM = rMark.GetMark(nm);
                    SdrObject* pObj = pM->GetMarkedSdrObj();

                    // extra undo actions for changed connector which now may hold its laid out path (SJ)
                    std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pObj ) );
                    AddUndoActions( vConnectorUndoActions );

                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));
                }
            }

            // make sure, OrderNums are correct:
            rMark.GetMark(0)->GetMarkedSdrObj()->GetOrdNum();

            for(size_t nm = nMarkCount; nm > 0;)
            {
                --nm;
                SdrMark* pM = rMark.GetMark(nm);
                SdrObject* pObj = pM->GetMarkedSdrObj();
                SdrObjList*  pOL = pObj->GetObjList();
                const size_t nOrdNum(pObj->GetOrdNumDirect());

                bool bIs3D = dynamic_cast< E3dObject* >(pObj);
                // set up a scene updater if object is a 3d object
                if(bIs3D)
                {
                    aUpdaters.push_back(new E3DModifySceneSnapRectUpdater(pObj));
                }

                pOL->RemoveObject(nOrdNum);

                if( !bUndo )
                {
                    // tdf#108863 don't delete objects before EndUndo()
                    ret.push_back(pObj);
                }
            }

            // fire scene updaters
            while(!aUpdaters.empty())
            {
                delete aUpdaters.back();
                aUpdaters.pop_back();
            }
        }

        if( bUndo )
            EndUndo();
    }
    return ret;
}

static void lcl_LazyDelete(std::vector<SdrObject*> & rLazyDelete)
{
    // now delete removed scene objects
    while (!rLazyDelete.empty())
    {
        SdrObject::Free( rLazyDelete.back() );
        rLazyDelete.pop_back();
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
    BegUndo(ImpGetResStr(STR_EditDelete),GetDescriptionOfMarkedObjects(),SdrRepeatFunc::Delete);

    std::vector<SdrObject*> lazyDeleteObjects;
    // remove as long as something is selected. This allows to schedule objects for
    // removal for a next run as needed
    while(GetMarkedObjectCount())
    {
        // vector to remember the parents which may be empty after object removal
        std::vector< SdrObject* > aParents;

        {
            const SdrMarkList& rMarkList = GetMarkedObjectList();
            const size_t nCount(rMarkList.GetMarkCount());

            for(size_t a = 0; a < nCount; ++a)
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
                for(size_t a = 0; a < nCount; ++a)
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
        auto temp(DeleteMarkedList(GetMarkedObjectList()));
        for (auto p : temp)
        {
            lazyDeleteObjects.push_back(p);
        }
        GetMarkedObjectListWriteAccess().Clear();
        maHdlList.Clear();

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

    lcl_LazyDelete(lazyDeleteObjects);
}

void SdrEditView::CopyMarkedObj()
{
    SortMarkedObjects();

    SdrMarkList aSourceObjectsForCopy(GetMarkedObjectList());
    // The following loop is used instead of MarkList::Merge(), to be
    // able to flag the MarkEntries.
    const size_t nEdgeAnz = GetEdgesOfMarkedNodes().GetMarkCount();
    for (size_t nEdgeNum=0; nEdgeNum<nEdgeAnz; ++nEdgeNum) {
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
    size_t nCloneErrCnt=0;
    const size_t nMarkCount=aSourceObjectsForCopy.GetMarkCount();
    for (size_t nm=0; nm<nMarkCount; ++nm) {
        SdrMark* pM=aSourceObjectsForCopy.GetMark(nm);
        SdrObject* pO=pM->GetMarkedSdrObj()->Clone();
        if (pO!=nullptr) {
            pM->GetPageView()->GetObjList()->InsertObject(pO, SAL_MAX_SIZE);

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

    if(nCloneErrCnt)
    {
#ifdef DBG_UTIL
        OStringBuffer aStr("SdrEditView::CopyMarkedObj(): Error when cloning ");

        if(nCloneErrCnt == 1)
        {
            aStr.append("a drawing object.");
        }
        else
        {
            aStr.append(static_cast<sal_Int32>(nCloneErrCnt));
            aStr.append(" drawing objects.");
        }

        aStr.append(" This object's/These objects's connections will not be copied.");
        OSL_FAIL(aStr.getStr());
#endif
    }
    MarkListHasChanged();
}


bool SdrEditView::InsertObjectAtView(SdrObject* pObj, SdrPageView& rPV, SdrInsertFlags nOptions)
{
    if (nOptions & SdrInsertFlags::SETDEFLAYER) {
        SdrLayerID nLayer=rPV.GetPage()->GetLayerAdmin().GetLayerID(maActualLayer);
        if (nLayer==SDRLAYER_NOTFOUND) nLayer=SdrLayerID(0);
        if (rPV.GetLockedLayers().IsSet(nLayer) || !rPV.GetVisibleLayers().IsSet(nLayer)) {
            SdrObject::Free( pObj ); // Layer locked or invisible
            return false;
        }
        pObj->NbcSetLayer(nLayer);
    }
    if (nOptions & SdrInsertFlags::SETDEFATTR) {
        if (mpDefaultStyleSheet!=nullptr) pObj->NbcSetStyleSheet(mpDefaultStyleSheet, false);
        pObj->SetMergedItemSet(maDefaultAttr);
    }
    if (!pObj->IsInserted()) {
        if (nOptions & SdrInsertFlags::NOBROADCAST) {
            rPV.GetObjList()->NbcInsertObject(pObj, SAL_MAX_SIZE);
        } else {
            rPV.GetObjList()->InsertObject(pObj, SAL_MAX_SIZE);
        }
    }
    if( IsUndoEnabled() )
        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pObj));

    if (!(nOptions & SdrInsertFlags::DONTMARK)) {
        if (!(nOptions & SdrInsertFlags::ADDMARK)) UnmarkAllObj();
        MarkObj(pObj,&rPV);
    }
    return true;
}

void SdrEditView::ReplaceObjectAtView(SdrObject* pOldObj, SdrPageView& rPV, SdrObject* pNewObj, bool bMark)
{
    if(IsTextEdit())
    {
#ifdef DBG_UTIL
        if(dynamic_cast< SdrTextObj* >(pOldObj) && static_cast< SdrTextObj* >(pOldObj)->IsTextEditActive())
        {
            OSL_ENSURE(false, "OldObject is in TextEdit mode, this has to be ended before replacing it usnig SdrEndTextEdit (!)");
        }

        if(dynamic_cast< SdrTextObj* >(pNewObj) && static_cast< SdrTextObj* >(pNewObj)->IsTextEditActive())
        {
            OSL_ENSURE(false, "NewObject is in TextEdit mode, this has to be ended before replacing it usnig SdrEndTextEdit (!)");
        }
#endif

        // #i123468# emergency repair situation, needs to cast up to a class derived from
        // this one; (aw080 has a mechanism for that and the view hierarchy is secured to
        // always be a SdrView)
        SdrView *pSdrView = dynamic_cast<SdrView*>(this);
        if (pSdrView)
            pSdrView->SdrEndTextEdit();
    }

    SdrObjList* pOL=pOldObj->GetObjList();
    const bool bUndo = IsUndoEnabled();
    if( bUndo  )
        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoReplaceObject(*pOldObj,*pNewObj));

    if( IsObjMarked( pOldObj ) )
        MarkObj( pOldObj, &rPV, true /*unmark!*/ );

    pOL->ReplaceObject(pNewObj,pOldObj->GetOrdNum());

    if( !bUndo )
        SdrObject::Free( pOldObj );

    if (bMark) MarkObj(pNewObj,&rPV);
}


bool SdrEditView::IsUndoEnabled() const
{
    return mpModel->IsUndoEnabled();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
