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

#include <vcl/metaact.hxx>
#include <svx/svdedtv.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdograf.hxx>  // fuer Possibilities
#include <svx/svdopath.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpoev.hxx>  // fuer die PolyPossiblities
#include <svx/svdstr.hrc>   // Namen aus der Resource
#include <svx/svdglob.hxx>  // StringCache
#include <svx/e3dsceneupdater.hxx>
#include <svx/svdview.hxx>

// #i13033#
#include <clonelist.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/svdogrp.hxx>
#include <svx/scene3d.hxx>
#include <svx/svditer.hxx>
#include <svx/svdview.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::ImpResetPossibilityFlags()
{
    mbSelectionIsReadOnly = false;
    mbGroupPossible = false;
    mbUnGroupPossible = false;
    mbGrpEnterPossible = false;
    mbDeletePossible = false;
    mbToTopPossible = false;
    mbToBtmPossible = false;
    mbReverseOrderPossible = false;
    mbImportMtfPossible = false;
    mbCombinePossible = false;
    mbDismantlePossible = false;
    mbCombineNoPolyPolyPossible = false;
    mbDismantleMakeLinesPossible = false;
    mbOrthoDesiredOnMarked = false;
    mbMoreThanOneNotMovable = false;
    mbOneOrMoreMovable = false;
    mbMoreThanOneNoMovRot = false;
    mbContortionPossible = false;
    mbMoveAllowedOnSelection = false;
    mbResizeFreeAllowed = false;
    mbResizePropAllowed = false;
    mbRotateFreeAllowed = false;
    mbRotate90Allowed = false;
    mbMirrorFreeAllowed = false;
    mbMirror45Allowed = false;
    mbMirror90Allowed = false;
    mbShearAllowed = false;
    mbEdgeRadiusAllowed = false;
    mbTransparenceAllowed = false;
    mbGradientAllowed = false;
    mbCanConvToPath = false;
    mbCanConvToPoly = false;
    mbCanConvToContour = false;
    mbCanConvToPathLineToArea = false;
    mbCanConvToPolyLineToArea = false;
    mbMoveProtect = false;
    mbResizeProtect = false;
}

SdrEditView::SdrEditView(SdrModel& rModel1, OutputDevice* pOut)
:   SdrMarkView(rModel1, pOut),
    mbPossibilitiesDirty(true),
    mbSelectionIsReadOnly(false),
    mbGroupPossible(false),
    mbUnGroupPossible(false),
    mbGrpEnterPossible(false),
    mbDeletePossible(false),
    mbToTopPossible(false),
    mbToBtmPossible(false),
    mbReverseOrderPossible(false),
    mbImportMtfPossible(false),
    mbCombinePossible(false),
    mbDismantlePossible(false),
    mbCombineNoPolyPolyPossible(false),
    mbDismantleMakeLinesPossible(false),
    mbOrthoDesiredOnMarked(false),
    mbMoreThanOneNotMovable(false),
    mbOneOrMoreMovable(false),
    mbMoreThanOneNoMovRot(false),
    mbContortionPossible(false),
    mbMoveAllowedOnSelection(false),
    mbResizeFreeAllowed(false),
    mbResizePropAllowed(false),
    mbRotateFreeAllowed(false),
    mbRotate90Allowed(false),
    mbMirrorFreeAllowed(false),
    mbMirror45Allowed(false),
    mbMirror90Allowed(false),
    mbShearAllowed(false),
    mbEdgeRadiusAllowed(false),
    mbTransparenceAllowed(false),
    mbGradientAllowed(false),
    mbCanConvToPath(false),
    mbCanConvToPoly(false),
    mbCanConvToContour(false),
    mbCanConvToPathLineToArea(false),
    mbCanConvToPolyLineToArea(false),
    mbMoveProtect(false),
    mbResizeProtect(false),
    mbBundleVirtObj(false)
{
}

SdrEditView::~SdrEditView()
{
}

SdrLayer* SdrEditView::InsertNewLayer(const XubString& rName, sal_uInt32 nPos)
{
    SdrLayerAdmin& rLA = getSdrModelFromSdrView().GetModelLayerAdmin();
    const sal_uInt32 nMax(rLA.GetLayerCount());

    if(nPos > nMax)
    {
        nPos = nMax;
    }

    SdrLayer* pNewLayer = rLA.NewLayer(rName, nPos);

    if(getSdrModelFromSdrView().IsUndoEnabled())
    {
        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewLayer(nPos, rLA, getSdrModelFromSdrView()));
    }

    getSdrModelFromSdrView().SetChanged();

    return pNewLayer;
}


bool SdrEditView::ImpDelLayerCheck(SdrObjList* pOL, SdrLayerID nDelID) const
{
    bool bDelAll(true);
    const sal_uInt32 nObjAnz(pOL->GetObjCount());

    for(sal_uInt32 nObjNum(nObjAnz); nObjNum > 0 && bDelAll;)
    {
        nObjNum--;
        SdrObject* pObj = pOL->GetObj(nObjNum);
        SdrObjList* pSubOL = pObj->getChildrenOfSdrObject();

        if(pSubOL)
        {
            if(!ImpDelLayerCheck(pSubOL, nDelID))
            {
                // Rekursion
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
    const sal_uInt32 nObjAnz(pOL->GetObjCount());
    const bool bUndo(getSdrModelFromSdrView().IsUndoEnabled());

    for(sal_uInt32 nObjNum(nObjAnz); nObjNum > 0;)
    {
        nObjNum--;
        SdrObject* pObj = pOL->GetObj(nObjNum);
        SdrObjList* pSubOL = pObj->getChildrenOfSdrObject();

        if(pSubOL)
        {
            if(ImpDelLayerCheck(pSubOL, nDelID))
            {
                if(bUndo)
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));
                }

                pOL->RemoveObjectFromSdrObjList(nObjNum);

                if(!bUndo)
                {
                    deleteSdrObjectSafeAndClearPointer(pObj);
                }
            }
            else
            {
                ImpDelLayerDelObjs(pSubOL, nDelID);
            }
        }
        else
        {
            if(nDelID == pObj->GetLayer())
            {
                if(bUndo)
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));
                }

                pOL->RemoveObjectFromSdrObjList(nObjNum);

                if(!bUndo)
                {
                    deleteSdrObjectSafeAndClearPointer(pObj);
                }
            }
        }
    }
}

void SdrEditView::DeleteLayer(const XubString& rName)
{
    SdrLayerAdmin& rLA = getSdrModelFromSdrView().GetModelLayerAdmin();
    SdrLayer* pLayer = rLA.GetLayer(rName, true);
    const sal_uInt32 nLayerNum(rLA.GetLayerPos(pLayer));

    if(SDRLAYER_NOTFOUND != nLayerNum)
    {
        const SdrLayerID nDelID(pLayer->GetID());
        const bool bUndo(IsUndoEnabled());
        bool bMaPg(true);

        if(bUndo)
        {
            BegUndo(ImpGetResStr(STR_UndoDelLayer));
        }

        for(sal_uInt16 nPageKind(0); nPageKind < 2; nPageKind++)
        {
            // MasterPages and DrawPages
            const sal_uInt32 nPgAnz(bMaPg ? getSdrModelFromSdrView().GetMasterPageCount() : getSdrModelFromSdrView().GetPageCount());

            for(sal_uInt32 nPgNum(0); nPgNum < nPgAnz; nPgNum++)
            {
                // over all pages
                SdrPage* pPage = (bMaPg) ? getSdrModelFromSdrView().GetMasterPage(nPgNum) : getSdrModelFromSdrView().GetPage(nPgNum);
                const sal_uInt32 nObjAnz(pPage->GetObjCount());

                for(sal_uInt32 nObjNum(nObjAnz); nObjNum > 0;)
                {
                    nObjNum--;
                    SdrObject* pObj = pPage->GetObj(nObjNum);
                    SdrObjList* pSubOL = pObj->getChildrenOfSdrObject();

                    if(pSubOL)
                    {
                        if(ImpDelLayerCheck(pSubOL, nDelID))
                        {
                            if(bUndo)
                            {
                                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));
                            }

                            pPage->RemoveObjectFromSdrObjList(nObjNum);

                            if(!bUndo)
                            {
                                deleteSdrObjectSafeAndClearPointer(pObj);
                            }
                        }
                        else
                        {
                            ImpDelLayerDelObjs(pSubOL, nDelID);
                        }
                    }
                    else
                    {
                        if(nDelID == pObj->GetLayer())
                        {
                            if(bUndo)
                            {
                                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));
                            }

                            pPage->RemoveObjectFromSdrObjList(nObjNum);

                            if(!bUndo)
                            {
                                deleteSdrObjectSafeAndClearPointer(pObj);
                            }
                        }
                    }
                }
            }

            bMaPg = false;
        }

        if(bUndo)
        {
            AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoDeleteLayer(nLayerNum, rLA, getSdrModelFromSdrView()));
            rLA.RemoveLayer(nLayerNum);
            EndUndo();
        }
        else
        {
            delete rLA.RemoveLayer(nLayerNum);
        }

        getSdrModelFromSdrView().SetChanged();
    }
}

void SdrEditView::EndUndo()
{
    // #i13033#
    // Comparison changed to 1L since EndUndo() is called later now
    // and EndUndo WILL change count to count-1
    if(getSdrModelFromSdrView().IsLastEndUndo())
    {
        ImpBroadcastEdgesOfMarkedNodes();
    }

    // #i13033#
    // moved to bottom to still have access to UNDOs inside of
    // ImpBroadcastEdgesOfMarkedNodes()
    getSdrModelFromSdrView().EndUndo();
}

void SdrEditView::ImpBroadcastEdgesOfMarkedNodes()
{
    if(areSdrObjectsSelected())
    {
        // New mechanism to search for necessary disconnections for
        // changed connectors inside the transitive hull of all at
        // the beginning of UNDO selected objects
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 a(0); a < aSelection.size(); a++)
        {
            SdrEdgeObj* pEdge = dynamic_cast< SdrEdgeObj* >(aSelection[a]);

            if(pEdge)
            {
                SdrObject* pObj1 = pEdge->GetSdrObjectConnection(false);
                SdrObject* pObj2 = pEdge->GetSdrObjectConnection(true);

                if(pObj1 && !pEdge->CheckSdrObjectConnection(false))
                {
                    bool bContains(false);

                    for(sal_uInt32 b(0); b < aSelection.size(); b++)
                    {
                        if(aSelection[b] == pObj1)
                        {
                            bContains = true;
                            break;
                        }
                    }

                    if(!bContains)
                    {
                        if(IsUndoEnabled())
                        {
                            AddUndo( getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pEdge));
                        }

                        pEdge->ConnectToSdrObject(false);
                    }
                }

                if(pObj2 && !pEdge->CheckSdrObjectConnection(true))
                {
                    bool bContains(false);

                    for(sal_uInt32 b(0); b < aSelection.size(); b++)
                    {
                        if(aSelection[b] == pObj2)
                        {
                            bContains = true;
                            break;
                        }
                    }

                    if(!bContains)
                    {
                        if(IsUndoEnabled())
                        {
                            AddUndo( getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pEdge));
                        }

                        pEdge->ConnectToSdrObject(true);
                    }
                }
            }
        }

        ///// get all SdrEdgeObj which are connected to a selected SdrObject and are selected themselves
        //const ::std::vector< SdrEdgeObj* > aConnectedSdrEdgeObjs(getAllSdrEdgeObjConnectedToSdrObjectVector(aSelection, true));
        //
        //for(sal_uInt32 i(0); i < aConnectedSdrEdgeObjs.size(); i++)
        //{
        //  SdrEdgeObj* pEdge = aConnectedSdrEdgeObjs[i];
        //
        //  /// TTTT: is this needed or will this happen automatically...?
        //  pEdge->SetEdgeTrackDirty();
        //}
    }   //
}

void SdrEditView::handleSelectionChange()
{
    // call parent
    SdrMarkView::handleSelectionChange();

    // invalidate possibilities which are based on selection
    mbPossibilitiesDirty = true;
}

bool SdrEditView::IsResizeAllowed(bool bProp) const
{
    ForcePossibilities();

    if(mbResizeProtect)
    {
        return false;
    }

    if(bProp)
    {
        return mbResizePropAllowed;
    }

    return mbResizeFreeAllowed;
}

bool SdrEditView::IsRotateAllowed(bool b90Deg) const
{
    ForcePossibilities();

    if(mbMoveProtect)
    {
        return false;
    }

    if(b90Deg)
    {
        return mbRotate90Allowed;
    }

    return mbRotateFreeAllowed;
}

bool SdrEditView::IsMirrorAllowed(bool b45Deg, bool b90Deg) const
{
    ForcePossibilities();

    if(mbMoveProtect)
    {
        return false;
    }

    if(b90Deg)
    {
        return mbMirror90Allowed;
    }

    if(b45Deg)
    {
        return mbMirror45Allowed;
    }

    return mbMirrorFreeAllowed && !mbMoveProtect;
}

bool SdrEditView::IsTransparenceAllowed() const
{
    ForcePossibilities();

    return mbTransparenceAllowed;
}

bool SdrEditView::IsGradientAllowed() const
{
    ForcePossibilities();

    return mbGradientAllowed;
}

bool SdrEditView::IsShearAllowed() const
{
    ForcePossibilities();

    if(mbResizeProtect)
    {
        return false;
    }

    return mbShearAllowed;
}

bool SdrEditView::IsEdgeRadiusAllowed() const
{
    ForcePossibilities();

    return mbEdgeRadiusAllowed;
}

bool SdrEditView::IsCrookAllowed(bool bNoContortion) const
{
    // CrookMode fehlt hier (weil kein Rotate bei Shear ...)
    ForcePossibilities();

    if(bNoContortion)
    {
        if(!mbRotateFreeAllowed)
        {
            return false; // Crook is nich
        }

        return !mbMoveProtect && mbMoveAllowedOnSelection;
    }
    else
    {
        return !mbResizeProtect && mbContortionPossible;
    }
}

bool SdrEditView::IsDistortAllowed(bool bNoContortion) const
{
    ForcePossibilities();

    if(bNoContortion)
    {
        return false;
    }
    else
    {
        return !mbResizeProtect && mbContortionPossible;
    }
}

bool SdrEditView::IsCombinePossible(bool bNoPolyPoly) const
{
    ForcePossibilities();

    if(bNoPolyPoly)
    {
        return mbCombineNoPolyPolyPossible;
    }
    else
    {
        return mbCombinePossible;
    }
}

bool SdrEditView::IsDismantlePossible(bool bMakeLines) const
{
    ForcePossibilities();

    if(bMakeLines)
    {
        return mbDismantleMakeLinesPossible;
    }
    else
    {
        return mbDismantlePossible;
    }
}

void SdrEditView::CheckPossibilities()
{
    if(mbPossibilitiesDirty)
    {
        ImpResetPossibilityFlags();
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        const bool bExactlyOne(1 == aSelection.size());

        if(aSelection.size())
        {
            sal_uInt32 nMovableCount(0);
            const bool bMoreOrEqualTwo(aSelection.size() >= 2);
            mbReverseOrderPossible = bMoreOrEqualTwo;
            mbGroupPossible = bMoreOrEqualTwo;
            mbCombinePossible = bMoreOrEqualTwo;

            if(bExactlyOne)
            {
                const SdrObject* pObj = aSelection[0];
                const bool bGroup(pObj->getChildrenOfSdrObject());
                const bool bHasText(0 != pObj->GetOutlinerParaObject());

                if(bGroup || bHasText)
                {
                    mbCombinePossible = true;
                }
            }

            mbCombineNoPolyPolyPossible = mbCombinePossible;
            mbDeletePossible = true;

            // Zu den Transformationen erstmal ja sagen
            mbMoveAllowedOnSelection = true;
            mbResizeFreeAllowed = true;
            mbResizePropAllowed = true;
            mbRotateFreeAllowed = true;
            mbRotate90Allowed = true;
            mbMirrorFreeAllowed = true;
            mbMirror45Allowed = true;
            mbMirror90Allowed = true;
            mbShearAllowed = true;
            mbEdgeRadiusAllowed = false;
            mbContortionPossible = true;
            mbCanConvToContour = true;

            // these ones are only allowed when single object is selected
            mbTransparenceAllowed = bExactlyOne;
            mbGradientAllowed = bExactlyOne;

            if(mbGradientAllowed)
            {
                // gradient depends on fillstyle
                const SdrObject* pObj = aSelection[0];

                // maybe group object, so get merged ItemSet
                const SfxItemSet& rSet = pObj->GetMergedItemSet();
                const SfxItemState eState = rSet.GetItemState(XATTR_FILLSTYLE, false);

                if(SFX_ITEM_DONTCARE != eState)
                {
                    // If state is not DONTCARE, test the item
                    const XFillStyle eFillStyle(((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue());

                    if(XFILL_GRADIENT != eFillStyle)
                    {
                        mbGradientAllowed = false;
                    }
                }
            }

            bool bNoMovRotFound(false);
            mbSelectionIsReadOnly = GetSdrPageView() ? GetSdrPageView()->IsReadOnly() : false;

            for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
            {
                const SdrObject* pObj = aSelection[nm];
                SdrObjTransformInfoRec aInfo;
                pObj->TakeObjInfo(aInfo);
                const bool bMovPrt(pObj->IsMoveProtect());
                const bool bSizPrt(pObj->IsResizeProtect());

                if(!bMovPrt && aInfo.mbMoveAllowed)
                {
                    nMovableCount++; // Menge der MovableObjs zaehlen
                }

                if(bMovPrt)
                {
                    mbMoveProtect = true;
                }

                if(bSizPrt)
                {
                    mbResizeProtect = true;
                }

                // not allowed when not allowed at one object
                if(!aInfo.mbTransparenceAllowed)
                {
                    mbTransparenceAllowed = false;
                }

                // Wenn einer was nicht kann, duerfen's alle nicht
                if(!aInfo.mbMoveAllowed)
                {
                    mbMoveAllowedOnSelection = false;
                }

                if(!aInfo.mbResizeFreeAllowed)
                {
                    mbResizeFreeAllowed = false;
                }

                if(!aInfo.mbResizePropAllowed)
                {
                    mbResizePropAllowed = false;
                }

                if(!aInfo.mbRotateFreeAllowed)
                {
                    mbRotateFreeAllowed = false;
                }

                if(!aInfo.mbRotate90Allowed)
                {
                    mbRotate90Allowed = false;
                }

                if(!aInfo.mbMirrorFreeAllowed)
                {
                    mbMirrorFreeAllowed = false;
                }

                if(!aInfo.mbMirror45Allowed)
                {
                    mbMirror45Allowed = false;
                }

                if(!aInfo.mbMirror90Allowed)
                {
                    mbMirror90Allowed = false;
                }

                if(!aInfo.mbShearAllowed)
                {
                    mbShearAllowed = false;
                }

                if(aInfo.mbEdgeRadiusAllowed)
                {
                    mbEdgeRadiusAllowed = true;
                }

                if(aInfo.mbNoContortion)
                {
                    mbContortionPossible = false;
                }

                // Fuer Crook mit Contortion: Alle Objekte muessen
                // Movable und Rotatable sein, ausser maximal 1
                if(!mbMoreThanOneNoMovRot)
                {
                    if(!aInfo.mbMoveAllowed || !aInfo.mbResizeFreeAllowed)
                    {
                        mbMoreThanOneNoMovRot = bNoMovRotFound;
                        bNoMovRotFound = true;
                    }
                }

                // when one member cannot be converted, no conversion is possible
                if(!aInfo.mbCanConvToContour)
                {
                    mbCanConvToContour = false;
                }

                // Ungroup
                if(!mbUnGroupPossible)
                {
                    mbUnGroupPossible = (0 != pObj->getChildrenOfSdrObject());
                }

                // ConvertToCurve: Wenn mind. einer konvertiert werden kann ist das ok.
                if(aInfo.mbCanConvToPath)
                {
                    mbCanConvToPath = true;
                }

                if(aInfo.mbCanConvToPoly)
                {
                    mbCanConvToPoly = true;
                }

                if(aInfo.mbCanConvToPathLineToArea)
                {
                    mbCanConvToPathLineToArea = true;
                }

                if(aInfo.mbCanConvToPolyLineToArea)
                {
                    mbCanConvToPolyLineToArea = true;
                }

                // Combine/Dismantle
                if(mbCombinePossible)
                {
                    mbCombinePossible = ImpCanConvertForCombine(pObj);
                    mbCombineNoPolyPolyPossible = mbCombinePossible;
                }

                if(!mbDismantlePossible)
                {
                    mbDismantlePossible = ImpCanDismantle(pObj, false);
                }

                if(!mbDismantleMakeLinesPossible)
                {
                    mbDismantleMakeLinesPossible = ImpCanDismantle(pObj, true);
                }

                // OrthoDesiredOnMarked checken
                if(!mbOrthoDesiredOnMarked && !aInfo.mbNoOrthoDesired)
                {
                    mbOrthoDesiredOnMarked = true;
                }

                // ImportMtf checken
                if(!mbImportMtfPossible)
                {
                    const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(pObj);
                    const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(pObj);

                    if(pSdrGrafObj && ((pSdrGrafObj->HasGDIMetaFile() && !pSdrGrafObj->IsEPS()) || pSdrGrafObj->isEmbeddedSvg()))
                    {
                        mbImportMtfPossible = true;
                    }

                    if(pSdrOle2Obj)
                    {
                        mbImportMtfPossible = pSdrOle2Obj->GetObjRef().is();
                    }
                }
            }

            mbMoreThanOneNotMovable = (nMovableCount < aSelection.size() - 1);
            mbOneOrMoreMovable = (0 != nMovableCount);
            mbGrpEnterPossible = mbUnGroupPossible;
        }

        ImpCheckToTopBtmPossible();
        ImpCheckPolyPossibilities();
        mbPossibilitiesDirty = false;

        if(mbSelectionIsReadOnly)
        {
            const bool bMerker1(mbGrpEnterPossible);

            ImpResetPossibilityFlags();
            mbSelectionIsReadOnly = true;
            mbGrpEnterPossible = bMerker1;
        }

        if(mbMoveAllowedOnSelection)
        {
            // Verschieben von angeklebten Verbindern unterbinden
            // Derzeit nur fuer Einfachselektion implementiert.
            if(bExactlyOne)
            {
                SdrEdgeObj* pEdge = dynamic_cast< SdrEdgeObj* >(aSelection[0]);

                if(pEdge)
                {
                    SdrObject* pNode1 = pEdge->GetSdrObjectConnection(true);
                    SdrObject* pNode2 = pEdge->GetSdrObjectConnection(false);

                    if(pNode1 && pNode2)
                    {
                        mbMoveAllowedOnSelection = false;
                    }
                }
            }
        }
    }
}

void SdrEditView::ForcePossibilities() const
{
    if(mbPossibilitiesDirty)
    {
        const_cast< SdrEditView* >(this)->CheckPossibilities();
    }
}

void SdrEditView::ImpCheckPolyPossibilities()
{
}

void SdrEditView::deleteSdrObjectsWithUndo(const SdrObjectVector& rSdrObjectVector)
{
    if(rSdrObjectVector.size())
    {
        const bool bUndo(IsUndoEnabled());

        if(bUndo)
        {
            BegUndo();
        }

        sal_uInt32 nm(0);
        std::vector< E3DModifySceneSnapRectUpdater* > aUpdaters;

        if(bUndo)
        {
            for(nm = rSdrObjectVector.size(); nm > 0;)
            {
                nm--;
                SdrObject* pObj = rSdrObjectVector[nm];

                // extra undo actions for changed connector which now may hold it's layouted path (SJ)
                std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pObj ) );
                AddUndoActions( vConnectorUndoActions );
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));
            }
        }

        SdrObjectVector aRemoved3DObjects;

        for(nm = rSdrObjectVector.size(); nm > 0;)
        {
            nm--;
            SdrObject* pObj = rSdrObjectVector[nm];
            const bool bIs3D = dynamic_cast< E3dObject* >(pObj);

            // set up a scene updater if object is a 3d object
            if(bIs3D)
            {
                aUpdaters.push_back(new E3DModifySceneSnapRectUpdater(pObj));
            }

            SdrObjList* pParentList = pObj->getParentOfSdrObject();

            if(pParentList)
            {
                pParentList->RemoveObjectFromSdrObjList(pObj->GetNavigationPosition());
            }

            if(!bUndo)
            {
                if(bIs3D)
                {
                    aRemoved3DObjects.push_back( pObj ); // may be needed later
                }
                else
                {
                    deleteSdrObjectSafeAndClearPointer(pObj);
                }
            }
        }

        // fire scene updaters
        while(aUpdaters.size())
        {
            delete aUpdaters.back();
            aUpdaters.pop_back();
        }

        if(!bUndo)
        {
            // now delete removed scene objects
            while(aRemoved3DObjects.size())
            {
                deleteSdrObjectSafeAndClearPointer(aRemoved3DObjects.back());
                aRemoved3DObjects.pop_back();
            }
        }

        if(bUndo)
        {
            EndUndo();
        }
    }
}

void SdrEditView::DeleteMarkedObj()
{
    // #i110981# return when nothing is to be done at all
    if(!areSdrObjectsSelected())
    {
        return;
    }

    // moved breaking action and undo start outside loop
    BrkAction();
    SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    BegUndo(ImpGetResStr(STR_EditDelete), getSelectionDescription(aSelection), SDRREPFUNC_OBJ_DELETE);
    clearSdrObjectSelection();

    // remove as long as something is selected. This allows to schedule objects for
    // removal for a next run as needed
    while(aSelection.size())
    {
        // vector to remember the parents which may be empty after object removal
        ::std::set< SdrObject*, sdr::selection::IndicesComparator > aParents;

        for(sal_uInt32 a(0); a < aSelection.size(); a++)
        {
            // in the first run, add all found parents, but only once
            SdrObject* pObject = aSelection[a];
            SdrObjList* pObjectsParentList = pObject->getParentOfSdrObject();

            if(pObjectsParentList)
            {
                SdrObject* pParent = pObjectsParentList->getSdrObjectFromSdrObjList();

                if(pParent)
                {
                    aParents.insert(pParent);
                }
            }
        }

        if(!aParents.empty())
        {
            // in a 2nd run, remove all objects which may already be scheduled for
            // removal. I am not sure if this can happen, but theoretically
            // a to-be-removed object may already be the group/3DScene itself
            for(sal_uInt32 b(0); !aParents.empty() && b < aSelection.size(); b++)
            {
                aParents.erase(aSelection[b]);
            }
        }

        // original stuff: remove selected objects. Handle clear will
        // do something only once
        deleteSdrObjectsWithUndo(aSelection);
        aSelection.clear();

        while(!aParents.empty() && aSelection.empty())
        {
            // iterate over remembered parents
            SdrObject* pParent = *aParents.begin();
            aParents.erase(aParents.begin());

            if(pParent->getChildrenOfSdrObject() && 0 == pParent->getChildrenOfSdrObject()->GetObjCount())
            {
                // we detected an empty parent, a candidate to leave group/3DScene
                // if entered
                if(GetSdrPageView()
                    && GetSdrPageView()->GetCurrentGroup()
                    && GetSdrPageView()->GetCurrentGroup() == pParent)
                {
                    GetSdrPageView()->LeaveOneGroup();
                }

                // schedule empty parent for removal
                aSelection.push_back(pParent);
            }
        }
    }

    // end undo and change messaging moved at the end
    EndUndo();
}

void SdrEditView::CopyMarkedObj()
{
    if(areSdrObjectsSelected() && GetSdrPageView() && GetSdrPageView()->GetCurrentObjectList())
    {
        // not only the selected SdrObjects are copied, but also SdrEdgeObjs which are
        // connected to a selected SdrObject, thus add them to the selection. To be able to
        // identify those later, keep them in a local set
        SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        const ::std::vector< SdrEdgeObj* > aConnectedSdrEdgeObjs(getAllSdrEdgeObjConnectedToSdrObjectVector(aSelection, false));

        // use a simple, just by address of SdrObject sorted set of extzra-selected SdrEdgeObjs
        ::std::set< SdrEdgeObj*, sdr::selection::IndicesComparator > aSdrEdgeObjSet;

        SdrObjectVector aNewSelection;
        CloneList aCloneList;
        sal_uInt32 nCloneErrCnt(0);
        const bool bUndo(IsUndoEnabled());

        if(aConnectedSdrEdgeObjs.size())
        {
            // add extra-selected SdrEdgeObjs to remember them
            aSdrEdgeObjSet.insert(aConnectedSdrEdgeObjs.begin(), aConnectedSdrEdgeObjs.end());

            // also add them to the selection. Set the selection once ang re-get it to have the
            // correctly sorted order which the view wants to have
            aSelection.insert(aSelection.begin(), aConnectedSdrEdgeObjs.begin(), aConnectedSdrEdgeObjs.end());
            setSdrObjectSelection(aSelection);
            aSelection = getSelectedSdrObjectVectorFromSdrMarkView();
        }

        for(sal_uInt32 a(0); a < aSelection.size(); a++)
        {
            SdrObject* pOriginal = aSelection[a];
            SdrObject* pClone = pOriginal->CloneSdrObject();

            if(pClone)
            {
                GetSdrPageView()->GetCurrentObjectList()->InsertObjectToSdrObjList(*pClone);

                if(bUndo)
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoCopyObject(*pClone));
                }

                aCloneList.AddPair(pOriginal, pClone);
                bool bNewSelection(aSdrEdgeObjSet.empty());

                if(!bNewSelection)
                {
                    SdrEdgeObj* pOriginalEdgeObj = dynamic_cast< SdrEdgeObj* >(pOriginal);

                    if(pOriginalEdgeObj)
                    {
                        if(0 == aSdrEdgeObjSet.count(pOriginalEdgeObj))
                        {
                            bNewSelection = true;
                        }
                    }
                    else
                    {
                        bNewSelection = true;
                    }
                }

                if(bNewSelection)
                {
                    // add o new selection, else it was an extra-selected SdrEdgeObj which
                    // we do not want to have in the new selection
                    aNewSelection.push_back(pClone);
                }
            }
            else
            {
                nCloneErrCnt++;
            }
        }

        // New mechanism to re-create the connections of cloned connectors
        aCloneList.CopyConnections();
        setSdrObjectSelection(aNewSelection);

        if(nCloneErrCnt)
        {
#ifdef DBG_UTIL
            ByteString aStr("SdrEditView::CopyMarkedObj(): Error while cloning ");

            if(nCloneErrCnt == 1)
            {
                aStr += "a SdrObject.";
            }
            else
            {
                aStr += ByteString::CreateFromInt32( nCloneErrCnt );
                aStr += " SdrObjects.";
            }

            aStr += " Connectors are not cloned.";
            OSL_ENSURE(false, aStr.GetBuffer());
#endif
        }

    }
}

bool SdrEditView::InsertObjectAtView(SdrObject& rObj, sal_uInt32 nOptions)
{
    if(!GetSdrPageView() || !GetSdrPageView()->GetCurrentObjectList())
    {
        OSL_ENSURE(false, "InsertObjectAtView without target (!)");
        deleteSdrObjectSafe( &rObj );
        return false;
    }

    if(!dynamic_cast< E3dObject* >(&rObj))
    {
        SdrObject* pParent = GetSdrPageView()->GetCurrentObjectList()->getSdrObjectFromSdrObjList();

        if(pParent && dynamic_cast< E3dObject* >(pParent))
        {
            OSL_ENSURE(false, "InsertObjectAtView non-3D to 3D parent (!)");
            deleteSdrObjectSafe( &rObj );
            return false;
        }
    }

    if(nOptions & SDRINSERT_SETDEFLAYER)
    {
        SdrLayerID nLayer(GetSdrPageView()->getSdrPageFromSdrPageView().GetPageLayerAdmin().GetLayerID(GetActiveLayer(), true));

        if(SDRLAYER_NOTFOUND == nLayer)
        {
            nLayer = 0;
        }

        if(GetSdrPageView()->GetLockedLayers().IsSet(nLayer) || !GetSdrPageView()->GetVisibleLayers().IsSet(nLayer))
        {
            deleteSdrObjectSafe( &rObj ); // Layer gesperrt oder nicht sichtbar
            return false;
        }

        rObj.SetLayer(nLayer);
    }

    if(nOptions & SDRINSERT_SETDEFATTR)
    {
        if(GetDefaultStyleSheet())
        {
            rObj.SetStyleSheet(GetDefaultStyleSheet(), false);
        }

        rObj.SetMergedItemSet(GetDefaultAttr());
    }

    if(!rObj.IsObjectInserted())
    {
        GetSdrPageView()->GetCurrentObjectList()->InsertObjectToSdrObjList(rObj);
    }

    if(IsUndoEnabled())
    {
        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(rObj));
    }

    if(0 == (nOptions & SDRINSERT_DONTMARK))
    {
        if(0 == (nOptions & SDRINSERT_ADDMARK))
        {
            UnmarkAllObj();
        }

        MarkObj(rObj);
    }

    return true;
}

void SdrEditView::ReplaceObjectAtView(SdrObject& rOldObj, SdrObject& rNewObj, bool bMark)
{
    if(IsTextEdit())
    {
#ifdef DBG_UTIL
        if(dynamic_cast< SdrTextObj* >(&rOldObj) && static_cast< SdrTextObj& >(rOldObj).IsTextEditActive())
        {
            OSL_ENSURE(false, "OldObject is in TextEdit mode, this has to be ended before replacing it usnig SdrEndTextEdit (!)");
        }

        if(dynamic_cast< SdrTextObj* >(&rNewObj) && static_cast< SdrTextObj& >(rNewObj).IsTextEditActive())
        {
            OSL_ENSURE(false, "NewObject is in TextEdit mode, this has to be ended before replacing it usnig SdrEndTextEdit (!)");
        }
#endif

        // #123468# emergency repair situation, needs to cast up to a class derived from
        // this one; (aw080 has a mechanism for that and the view hierarchy is secured to
        // always be a SdrView)
        if(dynamic_cast< SdrView* >(this)) static_cast< SdrView* >(this)->SdrEndTextEdit();
    }

    SdrObjList* pOL = rOldObj.getParentOfSdrObject();
    const bool bUndo = IsUndoEnabled();

    if(bUndo)
    {
        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoReplaceObject(rOldObj, rNewObj));
    }

    if(IsObjMarked(rOldObj))
    {
        MarkObj(rOldObj, true /*unmark!*/ );
    }

    pOL->ReplaceObjectInSdrObjList(rNewObj, rOldObj.GetNavigationPosition());

    if(!bUndo )
    {
        deleteSdrObjectSafe( &rOldObj );
    }

    if(bMark)
    {
        MarkObj(rNewObj);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
