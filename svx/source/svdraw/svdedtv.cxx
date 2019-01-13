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
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/e3dsceneupdater.hxx>
#include <rtl/strbuf.hxx>
#include <svx/svdview.hxx>
#include <clonelist.hxx>
#include <svx/svdogrp.hxx>
#include <svx/scene3d.hxx>


using namespace com::sun::star;

void SdrEditView::ImpResetPossibilityFlags()
{
    m_bReadOnly               =false;

    m_bGroupPossible          =false;
    m_bUnGroupPossible        =false;
    m_bGrpEnterPossible       =false;
    m_bToTopPossible          =false;
    m_bToBtmPossible          =false;
    m_bReverseOrderPossible   =false;

    m_bImportMtfPossible      =false;
    m_bCombinePossible        =false;
    m_bDismantlePossible      =false;
    m_bCombineNoPolyPolyPossible =false;
    m_bDismantleMakeLinesPossible=false;
    m_bOrthoDesiredOnMarked   =false;

    m_bOneOrMoreMovable       =false;
    m_bMoreThanOneNoMovRot    =false;
    m_bContortionPossible     =false;
    m_bMoveAllowed            =false;
    m_bResizeFreeAllowed      =false;
    m_bResizePropAllowed      =false;
    m_bRotateFreeAllowed      =false;
    m_bRotate90Allowed        =false;
    m_bMirrorFreeAllowed      =false;
    m_bMirror45Allowed        =false;
    m_bMirror90Allowed        =false;
    m_bTransparenceAllowed    =false;
    m_bCropAllowed            =false;
    m_bGradientAllowed        =false;
    m_bShearAllowed           =false;
    m_bEdgeRadiusAllowed      =false;
    m_bCanConvToPath          =false;
    m_bCanConvToPoly          =false;
    m_bCanConvToContour       =false;
    m_bMoveProtect            =false;
    m_bResizeProtect          =false;
}

void SdrEditView::ImpClearVars()
{
    ImpResetPossibilityFlags();
    m_bPossibilitiesDirty=true;   // << Purify didn't like this
}

SdrEditView::SdrEditView(
    SdrModel& rSdrModel,
    OutputDevice* pOut)
:   SdrMarkView(rSdrModel, pOut)
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

bool SdrEditView::ImpDelLayerCheck(SdrObjList const * pOL, SdrLayerID nDelID) const
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
            BegUndo(SvxResId(STR_UndoDelLayer));

        bool bMaPg(true);

        for(sal_uInt16 nPageKind(0); nPageKind < 2; nPageKind++)
        {
            // MasterPages and DrawPages
            sal_uInt16 nPgCount(bMaPg ? mpModel->GetMasterPageCount() : mpModel->GetPageCount());

            for(sal_uInt16 nPgNum(0); nPgNum < nPgCount; nPgNum++)
            {
                // over all pages
                SdrPage* pPage = bMaPg ? mpModel->GetMasterPage(nPgNum) : mpModel->GetPage(nPgNum);
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
            rLA.RemoveLayer(nLayerNum).release();
            EndUndo();
        }
        else
        {
            rLA.RemoveLayer(nLayerNum);
        }

        mpModel->SetChanged();
    }
}


void SdrEditView::EndUndo()
{
    // #i13033#
    // Comparison changed to 1L since EndUndo() is called later now
    // and EndUndo WILL change count to count-1
    if(1 == mpModel->GetUndoBracketLevel())
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

    const size_t nMarkedEdgeCnt = GetMarkedEdgesOfMarkedNodes().GetMarkCount();

    for (size_t i=0; i<nMarkedEdgeCnt; ++i) {
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
    m_bPossibilitiesDirty=true;
}

void SdrEditView::ModelHasChanged()
{
    SdrMarkView::ModelHasChanged();
    m_bPossibilitiesDirty=true;
}

bool SdrEditView::IsResizeAllowed(bool bProp) const
{
    ForcePossibilities();
    if (m_bResizeProtect) return false;
    if (bProp) return m_bResizePropAllowed;
    return m_bResizeFreeAllowed;
}

bool SdrEditView::IsRotateAllowed(bool b90Deg) const
{
    ForcePossibilities();
    if (m_bMoveProtect) return false;
    if (b90Deg) return m_bRotate90Allowed;
    return m_bRotateFreeAllowed;
}

bool SdrEditView::IsMirrorAllowed(bool b45Deg, bool b90Deg) const
{
    ForcePossibilities();
    if (m_bMoveProtect) return false;
    if (b90Deg) return m_bMirror90Allowed;
    if (b45Deg) return m_bMirror45Allowed;
    return m_bMirrorFreeAllowed;
}

bool SdrEditView::IsTransparenceAllowed() const
{
    ForcePossibilities();
    return m_bTransparenceAllowed;
}

bool SdrEditView::IsCropAllowed() const
{
    ForcePossibilities();
    return m_bCropAllowed;
}

bool SdrEditView::IsGradientAllowed() const
{
    ForcePossibilities();
    return m_bGradientAllowed;
}

bool SdrEditView::IsShearAllowed() const
{
    ForcePossibilities();
    if (m_bResizeProtect) return false;
    return m_bShearAllowed;
}

bool SdrEditView::IsEdgeRadiusAllowed() const
{
    ForcePossibilities();
    return m_bEdgeRadiusAllowed;
}

bool SdrEditView::IsCrookAllowed(bool bNoContortion) const
{
    // CrookMode missing here (no rotations allowed when shearing ...)
    ForcePossibilities();
    if (bNoContortion) {
        if (!m_bRotateFreeAllowed) return false;
        return !m_bMoveProtect && m_bMoveAllowed;
    } else {
        return !m_bResizeProtect && m_bContortionPossible;
    }
}

bool SdrEditView::IsDistortAllowed(bool bNoContortion) const
{
    ForcePossibilities();
    if (bNoContortion) {
        return false;
    } else {
        return !m_bResizeProtect && m_bContortionPossible;
    }
}

bool SdrEditView::IsCombinePossible(bool bNoPolyPoly) const
{
    ForcePossibilities();
    if (bNoPolyPoly) return m_bCombineNoPolyPolyPossible;
    else return m_bCombinePossible;
}

bool SdrEditView::IsDismantlePossible(bool bMakeLines) const
{
    ForcePossibilities();
    if (bMakeLines) return m_bDismantleMakeLinesPossible;
    else return m_bDismantlePossible;
}

void SdrEditView::CheckPossibilities()
{
    if (mbSomeObjChgdFlag)
        m_bPossibilitiesDirty=true;

    if (mbSomeObjChgdFlag)
    {
        // This call IS necessary to correct the MarkList, in which
        // no longer to the model belonging objects still can reside.
        // These ones need to be removed.
        CheckMarked();
    }

    if (m_bPossibilitiesDirty)
    {
        ImpResetPossibilityFlags();
        SortMarkedObjects();
        const size_t nMarkCount = GetMarkedObjectCount();
        if (nMarkCount != 0)
        {
            m_bReverseOrderPossible = (nMarkCount >= 2);

            size_t nMovableCount=0;
            m_bGroupPossible=nMarkCount>=2;
            m_bCombinePossible=nMarkCount>=2;
            if (nMarkCount==1)
            {
                // check bCombinePossible more thoroughly
                // still missing ...
                const SdrObject* pObj=GetMarkedObjectByIndex(0);
                //const SdrPathObj* pPath=dynamic_cast<SdrPathObj*>( pObj );
                bool bGroup=pObj->GetSubList()!=nullptr;
                bool bHasText=pObj->GetOutlinerParaObject()!=nullptr;
                if (bGroup || bHasText) {
                    m_bCombinePossible=true;
                }
            }
            m_bCombineNoPolyPolyPossible=m_bCombinePossible;
            // accept transformations for now
            m_bMoveAllowed      =true;
            m_bResizeFreeAllowed=true;
            m_bResizePropAllowed=true;
            m_bRotateFreeAllowed=true;
            m_bRotate90Allowed  =true;
            m_bMirrorFreeAllowed=true;
            m_bMirror45Allowed  =true;
            m_bMirror90Allowed  =true;
            m_bShearAllowed     =true;
            m_bEdgeRadiusAllowed=false;
            m_bContortionPossible=true;
            m_bCanConvToContour = true;

            // these ones are only allowed when single object is selected
            m_bTransparenceAllowed = (nMarkCount == 1);
            m_bGradientAllowed = (nMarkCount == 1);
            m_bCropAllowed = (nMarkCount == 1);
            if(m_bGradientAllowed)
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
                    drawing::FillStyle eFillStyle = rSet.Get(XATTR_FILLSTYLE).GetValue();

                    if(eFillStyle != drawing::FillStyle_GRADIENT)
                    {
                        m_bGradientAllowed = false;
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
                    if (pPV->IsReadOnly()) m_bReadOnly=true;
                    pPV0=pPV;
                }

                SdrObjTransformInfoRec aInfo;
                pObj->TakeObjInfo(aInfo);
                bool bMovPrt=pObj->IsMoveProtect();
                bool bSizPrt=pObj->IsResizeProtect();
                if (!bMovPrt && aInfo.bMoveAllowed) nMovableCount++; // count MovableObjs
                if (bMovPrt) m_bMoveProtect=true;
                if (bSizPrt) m_bResizeProtect=true;

                // not allowed when not allowed at one object
                if(!aInfo.bTransparenceAllowed)
                    m_bTransparenceAllowed = false;

                // If one of these can't do something, none can
                if (!aInfo.bMoveAllowed      ) m_bMoveAllowed      =false;
                if (!aInfo.bResizeFreeAllowed) m_bResizeFreeAllowed=false;
                if (!aInfo.bResizePropAllowed) m_bResizePropAllowed=false;
                if (!aInfo.bRotateFreeAllowed) m_bRotateFreeAllowed=false;
                if (!aInfo.bRotate90Allowed  ) m_bRotate90Allowed  =false;
                if (!aInfo.bMirrorFreeAllowed) m_bMirrorFreeAllowed=false;
                if (!aInfo.bMirror45Allowed  ) m_bMirror45Allowed  =false;
                if (!aInfo.bMirror90Allowed  ) m_bMirror90Allowed  =false;
                if (!aInfo.bShearAllowed     ) m_bShearAllowed     =false;
                if (aInfo.bEdgeRadiusAllowed) m_bEdgeRadiusAllowed=true;
                if (aInfo.bNoContortion      ) m_bContortionPossible=false;
                // For Crook with Contortion: all objects have to be
                // Movable and Rotatable, except for a maximum of 1 of them
                if (!m_bMoreThanOneNoMovRot) {
                    if (!aInfo.bMoveAllowed || !aInfo.bResizeFreeAllowed) {
                        m_bMoreThanOneNoMovRot=bNoMovRotFound;
                        bNoMovRotFound=true;
                    }
                }

                // Must be resizable to allow cropping
                if (!aInfo.bResizeFreeAllowed && !aInfo.bResizePropAllowed)
                    m_bCropAllowed = false;

                // if one member cannot be converted, no conversion is possible
                if(!aInfo.bCanConvToContour)
                    m_bCanConvToContour = false;

                // Ungroup
                if (!m_bUnGroupPossible) m_bUnGroupPossible=pObj->GetSubList()!=nullptr;
                // ConvertToCurve: If at least one can be converted, that is fine.
                if (aInfo.bCanConvToPath          ) m_bCanConvToPath          =true;
                if (aInfo.bCanConvToPoly          ) m_bCanConvToPoly          =true;

                // Combine/Dismantle
                if(m_bCombinePossible)
                {
                    m_bCombinePossible = ImpCanConvertForCombine(pObj);
                    m_bCombineNoPolyPolyPossible = m_bCombinePossible;
                }

                if (!m_bDismantlePossible) m_bDismantlePossible = ImpCanDismantle(pObj, false);
                if (!m_bDismantleMakeLinesPossible) m_bDismantleMakeLinesPossible = ImpCanDismantle(pObj, true);
                // check OrthoDesiredOnMarked
                if (!m_bOrthoDesiredOnMarked && !aInfo.bNoOrthoDesired) m_bOrthoDesiredOnMarked=true;
                // check ImportMtf

                if (!m_bImportMtfPossible)
                {
                    const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(pObj);
                    if (pSdrGrafObj != nullptr)
                    {
                        if ((pSdrGrafObj->HasGDIMetaFile() && !pSdrGrafObj->IsEPS()) ||
                            pSdrGrafObj->isEmbeddedVectorGraphicData() ||
                            pSdrGrafObj->isEmbeddedPdfData())
                        {
                            m_bImportMtfPossible = true;
                        }
                    }

                    const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(pObj);
                    if (pSdrOle2Obj)
                    {
                        m_bImportMtfPossible = pSdrOle2Obj->GetObjRef().is();
                    }
                }
            }

            m_bOneOrMoreMovable=nMovableCount!=0;
            m_bGrpEnterPossible=m_bUnGroupPossible;
        }
        ImpCheckToTopBtmPossible();
        static_cast<SdrPolyEditView*>(this)->ImpCheckPolyPossibilities();
        m_bPossibilitiesDirty=false;

        if (m_bReadOnly) {
            bool bTemp=m_bGrpEnterPossible;
            ImpResetPossibilityFlags();
            m_bReadOnly=true;
            m_bGrpEnterPossible=bTemp;
        }
        if (m_bMoveAllowed) {
            // Don't allow moving glued connectors.
            // Currently only implemented for single selection.
            if (nMarkCount==1) {
                SdrObject* pObj=GetMarkedObjectByIndex(0);
                SdrEdgeObj* pEdge=dynamic_cast<SdrEdgeObj*>( pObj );
                if (pEdge!=nullptr) {
                    SdrObject* pNode1=pEdge->GetConnectedNode(true);
                    SdrObject* pNode2=pEdge->GetConnectedNode(false);
                    if (pNode1!=nullptr || pNode2!=nullptr) m_bMoveAllowed=false;
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
                    AddUndoActions(CreateConnectorUndo( *pObj ));

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
                SdrObjList*  pOL = pObj->getParentSdrObjListFromSdrObject();
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
    BegUndo(SvxResId(STR_EditDelete),GetDescriptionOfMarkedObjects(),SdrRepeatFunc::Delete);

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
                SdrMark* pMark(rMarkList.GetMark(a));
                SdrObject* pObject(pMark->GetMarkedSdrObj());
                SdrObject* pParent(pObject->getParentSdrObjectFromSdrObject());

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

        while(!aParents.empty() && !GetMarkedObjectCount())
        {
            // iterate over remembered parents
            SdrObject* pParent = aParents.back();
            aParents.pop_back();

            if(pParent->GetSubList() && 0 == pParent->GetSubList()->GetObjCount())
            {
                // we detected an empty parent, a candidate to leave group/3DScene
                // if entered
                if(GetSdrPageView()->GetCurrentGroup()
                    && GetSdrPageView()->GetCurrentGroup() == pParent)
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
    const size_t nEdgeCnt = GetEdgesOfMarkedNodes().GetMarkCount();
    for (size_t nEdgeNum=0; nEdgeNum<nEdgeCnt; ++nEdgeNum) {
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
        SdrObject* pSource(pM->GetMarkedSdrObj());
        SdrObject* pO(pSource->CloneSdrObject(pSource->getSdrModelFromSdrObject()));
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
        rPV.GetObjList()->InsertObject(pObj, SAL_MAX_SIZE);
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
            OSL_ENSURE(false, "OldObject is in TextEdit mode, this has to be ended before replacing it using SdrEndTextEdit (!)");
        }

        if(dynamic_cast< SdrTextObj* >(pNewObj) && static_cast< SdrTextObj* >(pNewObj)->IsTextEditActive())
        {
            OSL_ENSURE(false, "NewObject is in TextEdit mode, this has to be ended before replacing it using SdrEndTextEdit (!)");
        }
#endif

        // #i123468# emergency repair situation, needs to cast up to a class derived from
        // this one; (aw080 has a mechanism for that and the view hierarchy is secured to
        // always be a SdrView)
        SdrView *pSdrView = dynamic_cast<SdrView*>(this);
        if (pSdrView)
            pSdrView->SdrEndTextEdit();
    }

    SdrObjList* pOL=pOldObj->getParentSdrObjListFromSdrObject();
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
