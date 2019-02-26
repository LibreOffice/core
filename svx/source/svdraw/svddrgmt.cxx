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

#include "svddrgm1.hxx"
#include <math.h>

#include <tools/bigint.hxx>
#include <o3tl/numeric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>
#include <svx/xattr.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svddrgv.hxx>
#include <svx/svdograf.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/sdgcpitm.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <sdr/overlay/overlayrollingrectangle.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/overlay/overlayprimitive2dsequenceobject.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/svditer.hxx>
#include <svx/svdopath.hxx>
#include <svx/polypolygoneditor.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdovirt.hxx>
#include <svx/svdouno.hxx>
#include <sdr/primitive2d/sdrprimitivetools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>
#include <svl/itempool.hxx>
#include <map>
#include <vector>


SdrDragEntry::SdrDragEntry()
:   mbAddToTransparent(false)
{
}

SdrDragEntry::~SdrDragEntry()
{
}


SdrDragEntryPolyPolygon::SdrDragEntryPolyPolygon(const basegfx::B2DPolyPolygon& rOriginalPolyPolygon)
:   SdrDragEntry(),
    maOriginalPolyPolygon(rOriginalPolyPolygon)
{
}

SdrDragEntryPolyPolygon::~SdrDragEntryPolyPolygon()
{
}

drawinglayer::primitive2d::Primitive2DContainer SdrDragEntryPolyPolygon::createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod)
{
    drawinglayer::primitive2d::Primitive2DContainer aRetval;

    if(maOriginalPolyPolygon.count())
    {
        basegfx::B2DPolyPolygon aCopy(maOriginalPolyPolygon);
        const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;

        rDragMethod.applyCurrentTransformationToPolyPolygon(aCopy);
        basegfx::BColor aColA(aSvtOptionsDrawinglayer.GetStripeColorA().getBColor());
        basegfx::BColor aColB(aSvtOptionsDrawinglayer.GetStripeColorB().getBColor());
        const double fStripeLength(aSvtOptionsDrawinglayer.GetStripeLength());

        if(Application::GetSettings().GetStyleSettings().GetHighContrastMode())
        {
            aColA = aColB = Application::GetSettings().GetStyleSettings().GetHighlightColor().getBColor();
            aColB.invert();
        }

        aRetval.resize(2);
        aRetval[0] = new drawinglayer::primitive2d::PolyPolygonMarkerPrimitive2D(
            aCopy,
            aColA,
            aColB,
            fStripeLength);

        const basegfx::BColor aHilightColor(aSvtOptionsDrawinglayer.getHilightColor().getBColor());
        const double fTransparence(aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01);

        aRetval[1] = new drawinglayer::primitive2d::PolyPolygonSelectionPrimitive2D(
            aCopy,
            aHilightColor,
            fTransparence,
            3.0,
            false);
    }

    return aRetval;
}


SdrDragEntrySdrObject::SdrDragEntrySdrObject(
    const SdrObject& rOriginal,
    bool bModify)
:   SdrDragEntry(),
    maOriginal(rOriginal),
    mpClone(nullptr),
    mbModify(bModify)
{
    // add SdrObject parts to transparent overlay stuff
    setAddToTransparent(true);
}

SdrDragEntrySdrObject::~SdrDragEntrySdrObject()
{
    if(mpClone)
    {
        SdrObject::Free(mpClone);
    }
}

void SdrDragEntrySdrObject::prepareCurrentState(SdrDragMethod& rDragMethod)
{
    // for the moment, i need to re-create the clone in all cases. I need to figure
    // out when clone and original have the same class, so that i can use operator=
    // in those cases

    if(mpClone)
    {
        SdrObject::Free(mpClone);
        mpClone = nullptr;
    }

    if(mbModify)
    {
        mpClone = maOriginal.getFullDragClone();

        // apply original transformation, implemented at the DragMethods
        rDragMethod.applyCurrentTransformationToSdrObject(*mpClone);
    }
}

drawinglayer::primitive2d::Primitive2DContainer SdrDragEntrySdrObject::createPrimitive2DSequenceInCurrentState(SdrDragMethod&)
{
    const SdrObject* pSource = &maOriginal;

    if(mbModify && mpClone)
    {
        // choose source for geometry data
        pSource = mpClone;
    }

    // use the view-independent primitive representation (without
    // evtl. GridOffset, that may be applied to the DragEntry individually)
    return pSource->GetViewContact().getViewIndependentPrimitive2DContainer();
}


SdrDragEntryPrimitive2DSequence::SdrDragEntryPrimitive2DSequence(
    const drawinglayer::primitive2d::Primitive2DContainer& rSequence)
:   SdrDragEntry(),
    maPrimitive2DSequence(rSequence)
{
    // add parts to transparent overlay stuff if necessary
    setAddToTransparent(true);
}

SdrDragEntryPrimitive2DSequence::~SdrDragEntryPrimitive2DSequence()
{
}

drawinglayer::primitive2d::Primitive2DContainer SdrDragEntryPrimitive2DSequence::createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod)
{
    drawinglayer::primitive2d::Primitive2DReference aTransformPrimitive2D(
        new drawinglayer::primitive2d::TransformPrimitive2D(
            rDragMethod.getCurrentTransformation(),
            maPrimitive2DSequence));

    return drawinglayer::primitive2d::Primitive2DContainer { aTransformPrimitive2D };
}


SdrDragEntryPointGlueDrag::SdrDragEntryPointGlueDrag(const std::vector< basegfx::B2DPoint >& rPositions, bool bIsPointDrag)
:   maPositions(rPositions),
    mbIsPointDrag(bIsPointDrag)
{
    // add SdrObject parts to transparent overlay stuff
    setAddToTransparent(true);
}

SdrDragEntryPointGlueDrag::~SdrDragEntryPointGlueDrag()
{
}

drawinglayer::primitive2d::Primitive2DContainer SdrDragEntryPointGlueDrag::createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod)
{
    drawinglayer::primitive2d::Primitive2DContainer aRetval;

    if(!maPositions.empty())
    {
        basegfx::B2DPolygon aPolygon;

        for(auto const & a: maPositions)
        {
            aPolygon.append(a);
        }

        basegfx::B2DPolyPolygon aPolyPolygon(aPolygon);

        rDragMethod.applyCurrentTransformationToPolyPolygon(aPolyPolygon);

        const basegfx::B2DPolygon aTransformed(aPolyPolygon.getB2DPolygon(0));
        std::vector< basegfx::B2DPoint > aTransformedPositions;

        aTransformedPositions.reserve(aTransformed.count());

        for(sal_uInt32 a = 0; a < aTransformed.count(); a++)
        {
            aTransformedPositions.push_back(aTransformed.getB2DPoint(a));
        }

        if(mbIsPointDrag)
        {
            const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
            basegfx::BColor aColor(aSvtOptionsDrawinglayer.GetStripeColorA().getBColor());

            if(Application::GetSettings().GetStyleSettings().GetHighContrastMode())
            {
                aColor = Application::GetSettings().GetStyleSettings().GetHighlightColor().getBColor();
            }

            drawinglayer::primitive2d::Primitive2DReference aMarkerArrayPrimitive2D(
                new drawinglayer::primitive2d::MarkerArrayPrimitive2D(aTransformedPositions,
                    drawinglayer::primitive2d::createDefaultCross_3x3(aColor)));

            aRetval = drawinglayer::primitive2d::Primitive2DContainer { aMarkerArrayPrimitive2D };
        }
        else
        {
            drawinglayer::primitive2d::Primitive2DReference aMarkerArrayPrimitive2D(
                new drawinglayer::primitive2d::MarkerArrayPrimitive2D(aTransformedPositions,
                                                                      SdrHdl::createGluePointBitmap()));
            aRetval = drawinglayer::primitive2d::Primitive2DContainer { aMarkerArrayPrimitive2D };
        }
    }

    return aRetval;
}


void SdrDragMethod::resetSdrDragEntries()
{
    // clear entries; creation is on demand
    clearSdrDragEntries();
}

basegfx::B2DRange SdrDragMethod::getCurrentRange() const
{
    return maOverlayObjectList.getBaseRange();
}

void SdrDragMethod::clearSdrDragEntries()
{
    maSdrDragEntries.clear();
}

void SdrDragMethod::addSdrDragEntry(std::unique_ptr<SdrDragEntry> pNew)
{
    assert(pNew);
    maSdrDragEntries.push_back(std::move(pNew));
}

void SdrDragMethod::createSdrDragEntries()
{
    if(getSdrDragView().GetSdrPageView() && getSdrDragView().GetSdrPageView()->HasMarkedObjPageView())
    {
        if(getSdrDragView().IsDraggingPoints())
        {
            createSdrDragEntries_PointDrag();
        }
        else if(getSdrDragView().IsDraggingGluePoints())
        {
            createSdrDragEntries_GlueDrag();
        }
        else
        {
            if(getSolidDraggingActive())
            {
                createSdrDragEntries_SolidDrag();
            }
            else
            {
                createSdrDragEntries_PolygonDrag();
            }
        }
    }
}

void SdrDragMethod::createSdrDragEntryForSdrObject(const SdrObject& rOriginal)
{
    // add full object drag; Clone() at the object has to work
    // for this
    addSdrDragEntry(std::unique_ptr<SdrDragEntry>(new SdrDragEntrySdrObject(rOriginal, true/*bModify*/)));
}

void SdrDragMethod::insertNewlyCreatedOverlayObjectForSdrDragMethod(
    std::unique_ptr<sdr::overlay::OverlayObject> pOverlayObject,
    const sdr::contact::ObjectContact& rObjectContact,
    sdr::overlay::OverlayManager& rOverlayManager)
{
    // check if we have an OverlayObject
    if(!pOverlayObject)
    {
        return;
    }

    // add to OverlayManager
    rOverlayManager.add(*pOverlayObject);

    // Add GridOffset for non-linear ViewToDevice transformation (calc)
    if(rObjectContact.supportsGridOffsets())
    {
        const basegfx::B2DRange& rNewRange(pOverlayObject->getBaseRange());

        if(!rNewRange.isEmpty())
        {
            basegfx::B2DVector aOffset(0.0, 0.0);
            rObjectContact.calculateGridOffsetForB2DRange(aOffset, rNewRange);

            if(!aOffset.equalZero())
            {
                pOverlayObject->setOffset(aOffset);
            }
        }
    }

    // add to local OverlayObjectList - ownership change (!)
    maOverlayObjectList.append(std::move(pOverlayObject));
}

void SdrDragMethod::createSdrDragEntries_SolidDrag()
{
    const size_t nMarkCount(getSdrDragView().GetMarkedObjectCount());
    SdrPageView* pPV = getSdrDragView().GetSdrPageView();

    if(pPV)
    {
        for(size_t a = 0; a < nMarkCount; ++a)
        {
            SdrMark* pM = getSdrDragView().GetSdrMarkByIndex(a);

            if(pM->GetPageView() == pPV)
            {
                const SdrObject* pObject = pM->GetMarkedSdrObj();

                if(pObject)
                {
                    if(pPV->PageWindowCount())
                    {
                        SdrObjListIter aIter(*pObject);

                        while(aIter.IsMore())
                        {
                            SdrObject* pCandidate = aIter.Next();

                            if(pCandidate)
                            {
                                const bool bSuppressFullDrag(!pCandidate->supportsFullDrag());
                                bool bAddWireframe(bSuppressFullDrag);

                                if(!bAddWireframe && !pCandidate->HasLineStyle())
                                {
                                    // add wireframe for objects without outline
                                    bAddWireframe = true;
                                }

                                if(!bSuppressFullDrag)
                                {
                                    // add full object drag; Clone() at the object has to work
                                    // for this
                                    createSdrDragEntryForSdrObject(*pCandidate);
                                }

                                if(bAddWireframe)
                                {
                                    // when dragging a 50% transparent copy of a filled or not filled object without
                                    // outline, this is normally hard to see. Add extra wireframe in that case. This
                                    // works nice e.g. with text frames etc.
                                    addSdrDragEntry(std::unique_ptr<SdrDragEntry>(new SdrDragEntryPolyPolygon(pCandidate->TakeXorPoly())));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void SdrDragMethod::createSdrDragEntries_PolygonDrag()
{
    const size_t nMarkCount(getSdrDragView().GetMarkedObjectCount());
    bool bNoPolygons(getSdrDragView().IsNoDragXorPolys() || nMarkCount > SdrDragView::GetDragXorPolyLimit());
    basegfx::B2DPolyPolygon aResult;
    sal_uInt32 nPointCount(0);

    for(size_t a = 0; !bNoPolygons && a < nMarkCount; ++a)
    {
        SdrMark* pM = getSdrDragView().GetSdrMarkByIndex(a);

        if(pM->GetPageView() == getSdrDragView().GetSdrPageView())
        {
            const basegfx::B2DPolyPolygon aNewPolyPolygon(pM->GetMarkedSdrObj()->TakeXorPoly());

            for(auto const& rPolygon : aNewPolyPolygon)
            {
                nPointCount += rPolygon.count();
            }

            if(nPointCount > SdrDragView::GetDragXorPointLimit())
            {
                bNoPolygons = true;
            }

            if(!bNoPolygons)
            {
                aResult.append(aNewPolyPolygon);
            }
        }
    }

    if(bNoPolygons)
    {
        const tools::Rectangle aR(getSdrDragView().GetSdrPageView()->MarkSnap());
        const basegfx::B2DRange aNewRectangle(aR.Left(), aR.Top(), aR.Right(), aR.Bottom());
        basegfx::B2DPolygon aNewPolygon(basegfx::utils::createPolygonFromRect(aNewRectangle));

        aResult = basegfx::B2DPolyPolygon(basegfx::utils::expandToCurve(aNewPolygon));
    }

    if(aResult.count())
    {
        addSdrDragEntry(std::unique_ptr<SdrDragEntry>(new SdrDragEntryPolyPolygon(aResult)));
    }
}

void SdrDragMethod::createSdrDragEntries_PointDrag()
{
    const size_t nMarkCount(getSdrDragView().GetMarkedObjectCount());
    std::vector< basegfx::B2DPoint > aPositions;

    for(size_t nm = 0; nm < nMarkCount; ++nm)
    {
        SdrMark* pM = getSdrDragView().GetSdrMarkByIndex(nm);

        if(pM->GetPageView() == getSdrDragView().GetSdrPageView())
        {
            const SdrUShortCont& rPts = pM->GetMarkedPoints();

            if (!rPts.empty())
            {
                const SdrObject* pObj = pM->GetMarkedSdrObj();
                const SdrPathObj* pPath = dynamic_cast< const SdrPathObj* >(pObj);

                if(pPath)
                {
                    const basegfx::B2DPolyPolygon& aPathXPP = pPath->GetPathPoly();

                    if(aPathXPP.count())
                    {
                        for(const sal_uInt16 nObjPt : rPts)
                        {
                            sal_uInt32 nPolyNum, nPointNum;

                            if(sdr::PolyPolygonEditor::GetRelativePolyPoint(aPathXPP, nObjPt, nPolyNum, nPointNum))
                            {
                                aPositions.push_back(aPathXPP.getB2DPolygon(nPolyNum).getB2DPoint(nPointNum));
                            }
                        }
                    }
                }
            }
        }
    }

    if(!aPositions.empty())
    {
        addSdrDragEntry(std::unique_ptr<SdrDragEntry>(new SdrDragEntryPointGlueDrag(aPositions, true)));
    }
}

void SdrDragMethod::createSdrDragEntries_GlueDrag()
{
    const size_t nMarkCount(getSdrDragView().GetMarkedObjectCount());
    std::vector< basegfx::B2DPoint > aPositions;

    for(size_t nm = 0; nm < nMarkCount; ++nm)
    {
        SdrMark* pM = getSdrDragView().GetSdrMarkByIndex(nm);

        if(pM->GetPageView() == getSdrDragView().GetSdrPageView())
        {
            const SdrUShortCont& rPts = pM->GetMarkedGluePoints();

            if (!rPts.empty())
            {
                const SdrObject* pObj = pM->GetMarkedSdrObj();
                const SdrGluePointList* pGPL = pObj->GetGluePointList();

                if (pGPL)
                {
                    for(const sal_uInt16 nObjPt : rPts)
                    {
                        const sal_uInt16 nGlueNum(pGPL->FindGluePoint(nObjPt));

                        if(SDRGLUEPOINT_NOTFOUND != nGlueNum)
                        {
                            const Point aPoint((*pGPL)[nGlueNum].GetAbsolutePos(*pObj));
                            aPositions.emplace_back(aPoint.X(), aPoint.Y());
                        }
                    }
                }
            }
        }
    }

    if(!aPositions.empty())
    {
        addSdrDragEntry(std::unique_ptr<SdrDragEntry>(new SdrDragEntryPointGlueDrag(aPositions, false)));
    }
}

void SdrDragMethod::ImpTakeDescriptionStr(const char* pStrCacheID, OUString& rStr) const
{
    ImpGetDescriptionOptions nOpt=ImpGetDescriptionOptions::NONE;
    if (IsDraggingPoints()) {
        nOpt=ImpGetDescriptionOptions::POINTS;
    } else if (IsDraggingGluePoints()) {
        nOpt=ImpGetDescriptionOptions::GLUEPOINTS;
    }
    rStr = getSdrDragView().ImpGetDescriptionString(pStrCacheID, nOpt);
}

SdrObject* SdrDragMethod::GetDragObj() const
{
    SdrObject* pObj=nullptr;
    if (getSdrDragView().mpDragHdl!=nullptr) pObj=getSdrDragView().mpDragHdl->GetObj();
    if (pObj==nullptr) pObj=getSdrDragView().mpMarkedObj;
    return pObj;
}

SdrPageView* SdrDragMethod::GetDragPV() const
{
    SdrPageView* pPV=nullptr;
    if (getSdrDragView().mpDragHdl!=nullptr) pPV=getSdrDragView().mpDragHdl->GetPageView();
    if (pPV==nullptr) pPV=getSdrDragView().mpMarkedPV;
    return pPV;
}

void SdrDragMethod::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    // the original applies the transformation using TRGetBaseGeometry/TRSetBaseGeometry.
    // Later this should be the only needed one for linear transforms (not for SdrDragCrook and
    // SdrDragDistort, those are NOT linear). Currently, this can not yet be used since the
    // special handling of rotate/mirror due to the not-being-able to handle it in the old
    // drawinglayer stuff. Text would currently not correctly be mirrored in the preview.
    basegfx::B2DHomMatrix aObjectTransform;
    basegfx::B2DPolyPolygon aObjectPolyPolygon;
    bool bPolyUsed(rTarget.TRGetBaseGeometry(aObjectTransform, aObjectPolyPolygon));

    // apply transform to object transform
    aObjectTransform *= getCurrentTransformation();

    if(bPolyUsed)
    {
        // do something special since the object size is in the polygon
        // break up matrix to get the scale
        const basegfx::utils::B2DHomMatrixBufferedDecompose aTmpDecomp(aObjectTransform);

        // get polygon's position and size
        const basegfx::B2DRange aPolyRange(aObjectPolyPolygon.getB2DRange());

        // get the scaling factors (do not mirror, this is in the object transformation)
        const double fScaleX(fabs(aTmpDecomp.getScale().getX()) / (basegfx::fTools::equalZero(aPolyRange.getWidth()) ? 1.0 : aPolyRange.getWidth()));
        const double fScaleY(fabs(aTmpDecomp.getScale().getY()) / (basegfx::fTools::equalZero(aPolyRange.getHeight()) ? 1.0 : aPolyRange.getHeight()));

        // prepare transform matrix for polygon
        basegfx::B2DHomMatrix aPolyTransform(
            basegfx::utils::createTranslateB2DHomMatrix(
                -aPolyRange.getMinX(),
                -aPolyRange.getMinY()));
        aPolyTransform.scale(fScaleX, fScaleY);

        // transform the polygon
        aObjectPolyPolygon.transform(aPolyTransform);
    }

    rTarget.TRSetBaseGeometry(getCurrentTransformation() * aObjectTransform, aObjectPolyPolygon);
}

void SdrDragMethod::applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget)
{
    // original uses CurrentTransformation
    rTarget.transform(getCurrentTransformation());
}

SdrDragMethod::SdrDragMethod(SdrDragView& rNewView)
:   maSdrDragEntries(),
    maOverlayObjectList(),
    mrSdrDragView(rNewView),
    mbMoveOnly(false),
    mbSolidDraggingActive(getSdrDragView().IsSolidDragging()),
    mbShiftPressed(false)
{
    if(mbSolidDraggingActive && Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        // fallback to wireframe when high contrast is used
        mbSolidDraggingActive = false;
    }
}

SdrDragMethod::~SdrDragMethod()
{
    clearSdrDragEntries();
}

void SdrDragMethod::Show()
{
    getSdrDragView().ShowDragObj();
}

void SdrDragMethod::Hide()
{
    getSdrDragView().HideDragObj();
}

basegfx::B2DHomMatrix SdrDragMethod::getCurrentTransformation()
{
    return basegfx::B2DHomMatrix();
}

void SdrDragMethod::CancelSdrDrag()
{
    Hide();
}

typedef std::map< const SdrObject*, SdrObject* > SdrObjectAndCloneMap;

void SdrDragMethod::CreateOverlayGeometry(
    sdr::overlay::OverlayManager& rOverlayManager,
    const sdr::contact::ObjectContact& rObjectContact)
{
    // create SdrDragEntries on demand
    if(maSdrDragEntries.empty())
    {
        createSdrDragEntries();
    }

    // if there are entries, derive OverlayObjects from the entries, including
    // modification from current interactive state
    if(!maSdrDragEntries.empty())
    {
        // #i54102# SdrDragEntrySdrObject creates clones of SdrObjects as base for creating the needed
        // primitives, holding the original and the clone. If connectors (Edges) are involved,
        // the cloned connectors need to be connected to the cloned SdrObjects (after cloning
        // they are connected to the original SdrObjects). To do so, trigger the preparation
        // steps for SdrDragEntrySdrObject, save an association of (orig, clone) in a helper
        // and evtl. remember if it was an edge
        SdrObjectAndCloneMap aOriginalAndClones;
        std::vector< SdrEdgeObj* > aEdges;

        // #i54102# execute prepareCurrentState for all SdrDragEntrySdrObject, register pair of original and
        // clone, remember edges
        for(auto const & a: maSdrDragEntries)
        {
            SdrDragEntrySdrObject* pSdrDragEntrySdrObject = dynamic_cast< SdrDragEntrySdrObject*>(a.get());

            if(pSdrDragEntrySdrObject)
            {
                pSdrDragEntrySdrObject->prepareCurrentState(*this);

                SdrEdgeObj* pSdrEdgeObj = dynamic_cast< SdrEdgeObj* >(pSdrDragEntrySdrObject->getClone());

                if(pSdrEdgeObj)
                {
                    aEdges.push_back(pSdrEdgeObj);
                }

                if(pSdrDragEntrySdrObject->getClone())
                {
                    aOriginalAndClones[&pSdrDragEntrySdrObject->getOriginal()] = pSdrDragEntrySdrObject->getClone();
                }
            }
        }

        // #i54102# if there are edges, reconnect their ends to the corresponding clones (if found)
        for(SdrEdgeObj* pSdrEdgeObj: aEdges)
        {
            SdrObject* pConnectedTo = pSdrEdgeObj->GetConnectedNode(true);

            if(pConnectedTo)
            {
                SdrObjectAndCloneMap::iterator aEntry = aOriginalAndClones.find(pConnectedTo);

                if(aEntry != aOriginalAndClones.end())
                {
                    pSdrEdgeObj->ConnectToNode(true, aEntry->second);
                }
            }

            pConnectedTo = pSdrEdgeObj->GetConnectedNode(false);

            if(pConnectedTo)
            {
                SdrObjectAndCloneMap::iterator aEntry = aOriginalAndClones.find(pConnectedTo);

                if(aEntry != aOriginalAndClones.end())
                {
                    pSdrEdgeObj->ConnectToNode(false, aEntry->second);
                }
            }
        }

        // collect primitives for visualisation
        drawinglayer::primitive2d::Primitive2DContainer aResult;
        drawinglayer::primitive2d::Primitive2DContainer aResultTransparent;

        for(auto & pCandidate: maSdrDragEntries)
        {
            const drawinglayer::primitive2d::Primitive2DContainer aCandidateResult(pCandidate->createPrimitive2DSequenceInCurrentState(*this));

            if(!aCandidateResult.empty())
            {
                if(pCandidate->getAddToTransparent())
                {
                    aResultTransparent.append(aCandidateResult);
                }
                else
                {
                    aResult.append(aCandidateResult);
                }
            }
        }

        if(DoAddConnectorOverlays())
        {
            const drawinglayer::primitive2d::Primitive2DContainer aConnectorOverlays(AddConnectorOverlays());

            if(!aConnectorOverlays.empty())
            {
                // add connector overlays to transparent part
                aResultTransparent.append(aConnectorOverlays);
            }
        }

        if(!aResult.empty())
        {
            std::unique_ptr<sdr::overlay::OverlayObject> pNewOverlayObject(
                new sdr::overlay::OverlayPrimitive2DSequenceObject(
                    aResult));

            insertNewlyCreatedOverlayObjectForSdrDragMethod(
                std::move(pNewOverlayObject),
                rObjectContact,
                rOverlayManager);
        }

        if(!aResultTransparent.empty())
        {
            drawinglayer::primitive2d::Primitive2DReference aUnifiedTransparencePrimitive2D(new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(aResultTransparent, 0.5));
            aResultTransparent = drawinglayer::primitive2d::Primitive2DContainer { aUnifiedTransparencePrimitive2D };

            std::unique_ptr<sdr::overlay::OverlayObject> pNewOverlayObject(
                new sdr::overlay::OverlayPrimitive2DSequenceObject(
                    aResultTransparent));

            insertNewlyCreatedOverlayObjectForSdrDragMethod(
                std::move(pNewOverlayObject),
                rObjectContact,
                rOverlayManager);
        }
    }

    // add DragStripes if necessary (help lines cross the page when dragging)
    if(getSdrDragView().IsDragStripes())
    {
        tools::Rectangle aActionRectangle;
        getSdrDragView().TakeActionRect(aActionRectangle);

        const basegfx::B2DPoint aTopLeft(aActionRectangle.Left(), aActionRectangle.Top());
        const basegfx::B2DPoint aBottomRight(aActionRectangle.Right(), aActionRectangle.Bottom());
        std::unique_ptr<sdr::overlay::OverlayRollingRectangleStriped> pNew(
            new sdr::overlay::OverlayRollingRectangleStriped(
                aTopLeft,
                aBottomRight,
                true,
                false));

        insertNewlyCreatedOverlayObjectForSdrDragMethod(
            std::move(pNew),
            rObjectContact,
            rOverlayManager);
    }
}

void SdrDragMethod::destroyOverlayGeometry()
{
    maOverlayObjectList.clear();
}

bool SdrDragMethod::DoAddConnectorOverlays()
{
    // these conditions are translated from SdrDragView::ImpDrawEdgeXor
    const SdrMarkList& rMarkedNodes = getSdrDragView().GetEdgesOfMarkedNodes();

    if(!rMarkedNodes.GetMarkCount())
    {
        return false;
    }

    if(getSdrDragView().IsDraggingPoints() || getSdrDragView().IsDraggingGluePoints())
    {
        return false;
    }

    if(!getMoveOnly() && !(
        dynamic_cast<const SdrDragMove*>(this) != nullptr || dynamic_cast<const SdrDragResize*>(this) != nullptr ||
        dynamic_cast<const SdrDragRotate*>(this) != nullptr || dynamic_cast<const SdrDragMirror*>(this) != nullptr ))
    {
        return false;
    }

    // one more migrated from SdrEdgeObj::NspToggleEdgeXor
    if( dynamic_cast< const SdrDragObjOwn* >(this) != nullptr || dynamic_cast< const SdrDragMovHdl* >(this) != nullptr )
    {
        return false;
    }

    return true;
}

drawinglayer::primitive2d::Primitive2DContainer SdrDragMethod::AddConnectorOverlays()
{
    drawinglayer::primitive2d::Primitive2DContainer aRetval;
    const bool bDetail(getMoveOnly());
    const SdrMarkList& rMarkedNodes = getSdrDragView().GetEdgesOfMarkedNodes();

    for(size_t a = 0; a < rMarkedNodes.GetMarkCount(); ++a)
    {
        SdrMark* pEM = rMarkedNodes.GetMark(a);

        if(pEM && pEM->GetMarkedSdrObj())
        {
            SdrEdgeObj* pEdge = dynamic_cast< SdrEdgeObj* >(pEM->GetMarkedSdrObj());

            if(pEdge)
            {
                const basegfx::B2DPolygon aEdgePolygon(pEdge->ImplAddConnectorOverlay(*this, pEM->IsCon1(), pEM->IsCon2(), bDetail));

                if(aEdgePolygon.count())
                {
                    // this polygon is a temporary calculated connector path, so it is not possible to fetch
                    // the needed primitives directly from the pEdge object which does not get changed. If full
                    // drag is on, use the SdrObjects ItemSet to create a adequate representation
                    bool bUseSolidDragging(getSolidDraggingActive());

                    if(bUseSolidDragging)
                    {
                        // switch off solid dragging if connector is not visible
                        if(!pEdge->HasLineStyle())
                        {
                            bUseSolidDragging = false;
                        }
                    }

                    if(bUseSolidDragging)
                    {
                        const SfxItemSet& rItemSet = pEdge->GetMergedItemSet();
                        const drawinglayer::attribute::SdrLineAttribute aLine(
                            drawinglayer::primitive2d::createNewSdrLineAttribute(rItemSet));

                        if(!aLine.isDefault())
                        {
                            const drawinglayer::attribute::SdrLineStartEndAttribute aLineStartEnd(
                                drawinglayer::primitive2d::createNewSdrLineStartEndAttribute(
                                    rItemSet,
                                    aLine.getWidth()));

                            aRetval.push_back(drawinglayer::primitive2d::createPolygonLinePrimitive(
                                    aEdgePolygon,
                                    aLine,
                                    aLineStartEnd));
                        }
                    }
                    else
                    {
                        const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
                        basegfx::BColor aColA(aSvtOptionsDrawinglayer.GetStripeColorA().getBColor());
                        basegfx::BColor aColB(aSvtOptionsDrawinglayer.GetStripeColorB().getBColor());
                        const double fStripeLength(aSvtOptionsDrawinglayer.GetStripeLength());

                        if(Application::GetSettings().GetStyleSettings().GetHighContrastMode())
                        {
                            aColA = aColB = Application::GetSettings().GetStyleSettings().GetHighlightColor().getBColor();
                            aColB.invert();
                        }

                        drawinglayer::primitive2d::Primitive2DReference aPolyPolygonMarkerPrimitive2D(
                            new drawinglayer::primitive2d::PolygonMarkerPrimitive2D(
                                aEdgePolygon, aColA, aColB, fStripeLength));
                        aRetval.push_back(aPolyPolygonMarkerPrimitive2D);
                    }
                }
            }
        }
    }

    return aRetval;
}


SdrDragMovHdl::SdrDragMovHdl(SdrDragView& rNewView)
:   SdrDragMethod(rNewView)
{
}

void SdrDragMovHdl::createSdrDragEntries()
{
    // SdrDragMovHdl does not use the default drags,
    // but creates nothing
}

void SdrDragMovHdl::TakeSdrDragComment(OUString& rStr) const
{
    rStr=SvxResId(STR_DragMethMovHdl);
    if (getSdrDragView().IsDragWithCopy()) rStr+=SvxResId(STR_EditWithCopy);
}

bool SdrDragMovHdl::BeginSdrDrag()
{
    if( !GetDragHdl() )
        return false;

    DragStat().SetRef1(GetDragHdl()->GetPos());
    DragStat().SetShown(!DragStat().IsShown());
    SdrHdlKind eKind=GetDragHdl()->GetKind();
    SdrHdl* pH1=GetHdlList().GetHdl(SdrHdlKind::Ref1);
    SdrHdl* pH2=GetHdlList().GetHdl(SdrHdlKind::Ref2);

    if (eKind==SdrHdlKind::MirrorAxis)
    {
        if (pH1==nullptr || pH2==nullptr)
        {
            OSL_FAIL("SdrDragMovHdl::BeginSdrDrag(): Moving the axis of reflection: reference handles not found.");
            return false;
        }

        DragStat().SetActionRect(tools::Rectangle(pH1->GetPos(),pH2->GetPos()));
    }
    else
    {
        Point aPt(GetDragHdl()->GetPos());
        DragStat().SetActionRect(tools::Rectangle(aPt,aPt));
    }

    return true;
}

void SdrDragMovHdl::MoveSdrDrag(const Point& rNoSnapPnt)
{
    Point aPnt(rNoSnapPnt);

    if ( GetDragHdl() && DragStat().CheckMinMoved(rNoSnapPnt))
    {
        if (GetDragHdl()->GetKind()==SdrHdlKind::MirrorAxis)
        {
            SdrHdl* pH1=GetHdlList().GetHdl(SdrHdlKind::Ref1);
            SdrHdl* pH2=GetHdlList().GetHdl(SdrHdlKind::Ref2);

            if (pH1==nullptr || pH2==nullptr)
                return;

            if (!DragStat().IsNoSnap())
            {
                long nBestXSnap=0;
                long nBestYSnap=0;
                bool bXSnapped=false;
                bool bYSnapped=false;
                Point aDif(aPnt-DragStat().GetStart());
                getSdrDragView().CheckSnap(Ref1()+aDif,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
                getSdrDragView().CheckSnap(Ref2()+aDif,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
                aPnt.AdjustX(nBestXSnap );
                aPnt.AdjustY(nBestYSnap );
            }

            if (aPnt!=DragStat().GetNow())
            {
                Hide();
                DragStat().NextMove(aPnt);
                Point aDif(DragStat().GetNow()-DragStat().GetStart());
                pH1->SetPos(Ref1()+aDif);
                pH2->SetPos(Ref2()+aDif);

                SdrHdl* pHM = GetHdlList().GetHdl(SdrHdlKind::MirrorAxis);

                if(pHM)
                    pHM->Touch();

                Show();
                DragStat().SetActionRect(tools::Rectangle(pH1->GetPos(),pH2->GetPos()));
            }
        }
        else
        {
            if (!DragStat().IsNoSnap()) SnapPos(aPnt);
            long nSA=0;

            if (getSdrDragView().IsAngleSnapEnabled())
                nSA=getSdrDragView().GetSnapAngle();

            if (getSdrDragView().IsMirrorAllowed(true,true))
            { // limited
                if (!getSdrDragView().IsMirrorAllowed()) nSA=4500;
                if (!getSdrDragView().IsMirrorAllowed(true)) nSA=9000;
            }

            if (getSdrDragView().IsOrtho() && nSA!=9000)
                nSA=4500;

            if (nSA!=0)
            { // angle snapping
                SdrHdlKind eRef=SdrHdlKind::Ref1;

                if (GetDragHdl()->GetKind()==SdrHdlKind::Ref1)
                    eRef=SdrHdlKind::Ref2;

                SdrHdl* pH=GetHdlList().GetHdl(eRef);

                if (pH!=nullptr)
                {
                    Point aRef(pH->GetPos());
                    long nAngle=NormAngle36000(GetAngle(aPnt-aRef));
                    long nNewAngle=nAngle;
                    nNewAngle+=nSA/2;
                    nNewAngle/=nSA;
                    nNewAngle*=nSA;
                    nNewAngle=NormAngle36000(nNewAngle);
                    double a=(nNewAngle-nAngle)*F_PI18000;
                    double nSin=sin(a);
                    double nCos=cos(a);
                    RotatePoint(aPnt,aRef,nSin,nCos);

                    // eliminate rounding errors for certain values
                    if (nSA==9000)
                    {
                        if (nNewAngle==0    || nNewAngle==18000) aPnt.setY(aRef.Y() );
                        if (nNewAngle==9000 || nNewAngle==27000) aPnt.setX(aRef.X() );
                    }

                    if (nSA==4500)
                        OrthoDistance8(aRef,aPnt,true);
                }
            }

            if (aPnt!=DragStat().GetNow())
            {
                Hide();
                DragStat().NextMove(aPnt);
                GetDragHdl()->SetPos(DragStat().GetNow());
                SdrHdl* pHM = GetHdlList().GetHdl(SdrHdlKind::MirrorAxis);

                if(pHM)
                    pHM->Touch();

                Show();
                DragStat().SetActionRect(tools::Rectangle(aPnt,aPnt));
            }
        }
    }
}

bool SdrDragMovHdl::EndSdrDrag(bool /*bCopy*/)
{
    if( GetDragHdl() )
    {
        switch (GetDragHdl()->GetKind())
        {
            case SdrHdlKind::Ref1:
                Ref1()=DragStat().GetNow();
                break;

            case SdrHdlKind::Ref2:
                Ref2()=DragStat().GetNow();
                break;

            case SdrHdlKind::MirrorAxis:
                Ref1()+=DragStat().GetNow()-DragStat().GetStart();
                Ref2()+=DragStat().GetNow()-DragStat().GetStart();
                break;

            default: break;
        }
    }

    return true;
}

void SdrDragMovHdl::CancelSdrDrag()
{
    Hide();

    SdrHdl* pHdl = GetDragHdl();
    if( pHdl )
        pHdl->SetPos(DragStat().GetRef1());

    SdrHdl* pHM = GetHdlList().GetHdl(SdrHdlKind::MirrorAxis);

    if(pHM)
        pHM->Touch();
}

PointerStyle SdrDragMovHdl::GetSdrDragPointer() const
{
    const SdrHdl* pHdl = GetDragHdl();

    if (pHdl!=nullptr)
    {
        return pHdl->GetPointer();
    }

    return PointerStyle::RefHand;
}


SdrDragObjOwn::SdrDragObjOwn(SdrDragView& rNewView)
:   SdrDragMethod(rNewView),
    mpClone(nullptr)
{
    const SdrObject* pObj = GetDragObj();

    if(pObj)
    {
        // suppress full drag for some object types
        setSolidDraggingActive(pObj->supportsFullDrag());
    }
}

SdrDragObjOwn::~SdrDragObjOwn()
{
    if(mpClone)
    {
        SdrObject::Free(mpClone);
    }
}

void SdrDragObjOwn::createSdrDragEntries()
{
    if(mpClone)
    {
        basegfx::B2DPolyPolygon aDragPolyPolygon;
        bool bAddWireframe(true);

        if(getSolidDraggingActive())
        {
            SdrPageView* pPV = getSdrDragView().GetSdrPageView();

            if(pPV && pPV->PageWindowCount())
            {
                addSdrDragEntry(std::unique_ptr<SdrDragEntry>(new SdrDragEntrySdrObject(*mpClone, false)));

                // potentially no wireframe needed, full drag works
                bAddWireframe = false;
            }
        }

        if(!bAddWireframe)
        {
            // check for extra conditions for wireframe, e.g. no border at
            // objects
            if(!mpClone->HasLineStyle())
            {
                bAddWireframe = true;
            }
        }

        if(bAddWireframe)
        {
            // use wireframe poly when full drag is off or did not work
            aDragPolyPolygon = mpClone->TakeXorPoly();
        }

        // add evtl. extra DragPolyPolygon
        const basegfx::B2DPolyPolygon aSpecialDragPolyPolygon(mpClone->getSpecialDragPoly(DragStat()));

        if(aSpecialDragPolyPolygon.count())
        {
            aDragPolyPolygon.append(aSpecialDragPolyPolygon);
        }

        if(aDragPolyPolygon.count())
        {
            addSdrDragEntry(std::unique_ptr<SdrDragEntry>(new SdrDragEntryPolyPolygon(aDragPolyPolygon)));
        }
    }
}

void SdrDragObjOwn::TakeSdrDragComment(OUString& rStr) const
{
    // #i103058# get info string from the clone preferred, the original will
    // not be changed. For security, use original as fallback
    if(mpClone)
    {
        rStr = mpClone->getSpecialDragComment(DragStat());
    }
    else
    {
        const SdrObject* pObj = GetDragObj();

        if(pObj)
        {
            rStr = pObj->getSpecialDragComment(DragStat());
        }
    }
}

bool SdrDragObjOwn::BeginSdrDrag()
{
    if(!mpClone)
    {
        const SdrObject* pObj = GetDragObj();

        if(pObj && !pObj->IsResizeProtect())
        {
            if(pObj->beginSpecialDrag(DragStat()))
            {
                // create initial clone to have a start visualization
                mpClone = pObj->getFullDragClone();
                mpClone->applySpecialDrag(DragStat());

                return true;
            }
        }
    }

    return false;
}

void SdrDragObjOwn::MoveSdrDrag(const Point& rNoSnapPnt)
{
    const SdrObject* pObj = GetDragObj();

    if (!pObj)
        // No object to drag.  Bail out.
        return;

    Point aPnt(rNoSnapPnt);
    SdrPageView* pPV = GetDragPV();

    if (!pPV)
        // No page view available.  Bail out.
        return;

    if(!DragStat().IsNoSnap())
    {
        SnapPos(aPnt);
    }
    if(getSdrDragView().IsOrtho())
    {
        if (DragStat().IsOrtho8Possible())
        {
            OrthoDistance8(DragStat().GetStart(),aPnt,getSdrDragView().IsBigOrtho());
        }
        else if (DragStat().IsOrtho4Possible())
        {
            OrthoDistance4(DragStat().GetStart(),aPnt,getSdrDragView().IsBigOrtho());
        }
    }

    if (!DragStat().CheckMinMoved(rNoSnapPnt))
        // Not moved by the minimum threshold. Nothing to do.
        return;

    Hide();
    DragStat().NextMove(aPnt);

    // since SdrDragObjOwn currently supports no transformation of
    // existing SdrDragEntries but only their recreation, a recreation
    // after every move is needed in this mode. Delete existing
    // SdrDragEntries here  to force their recreation in the following Show().
    clearSdrDragEntries();

    // delete current clone (after the last reference to it is deleted above)
    if(mpClone)
    {
        SdrObject::Free(mpClone);
        mpClone = nullptr;
    }

    // create a new clone and modify to current drag state
    mpClone = pObj->getFullDragClone();
    mpClone->applySpecialDrag(DragStat());

    // AutoGrowWidth may change for SdrTextObj due to the automatism used
    // with bDisableAutoWidthOnDragging, so not only geometry changes but
    // also this (pretty indirect) property change is possible. If it gets
    // changed, it needs to be copied to the original since nothing will
    // happen when it only changes in the drag clone
    const bool bOldAutoGrowWidth(pObj->GetMergedItem(SDRATTR_TEXT_AUTOGROWWIDTH).GetValue());
    const bool bNewAutoGrowWidth(mpClone->GetMergedItem(SDRATTR_TEXT_AUTOGROWWIDTH).GetValue());

    if (bOldAutoGrowWidth != bNewAutoGrowWidth)
    {
        GetDragObj()->SetMergedItem(makeSdrTextAutoGrowWidthItem(bNewAutoGrowWidth));
    }

    Show();
}

bool SdrDragObjOwn::EndSdrDrag(bool /*bCopy*/)
{
    Hide();
    std::vector< std::unique_ptr<SdrUndoAction> > vConnectorUndoActions;
    bool bRet = false;
    SdrObject* pObj = GetDragObj();

    if(pObj)
    {
        std::unique_ptr<SdrUndoAction> pUndo;
        std::unique_ptr<SdrUndoAction> pUndo2;
        const bool bUndo = getSdrDragView().IsUndoEnabled();

        if( bUndo )
        {
            if(!getSdrDragView().IsInsObjPoint() && pObj->IsInserted() )
            {
                if (DragStat().IsEndDragChangesAttributes())
                {
                    pUndo=getSdrDragView().GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj);

                    if (DragStat().IsEndDragChangesGeoAndAttributes())
                    {
                        vConnectorUndoActions = getSdrDragView().CreateConnectorUndo( *pObj );
                        pUndo2 = getSdrDragView().GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj);
                    }
                }
                else
                {
                    vConnectorUndoActions = getSdrDragView().CreateConnectorUndo( *pObj );
                    pUndo= getSdrDragView().GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj);
                }
            }

            if( pUndo )
            {
                getSdrDragView().BegUndo( pUndo->GetComment() );
            }
            else
            {
                getSdrDragView().BegUndo();
            }
        }

        // Maybe use operator = for setting changed object data (do not change selection in
        // view, this will destroy the interactor). This is possible since a clone is now
        // directly modified by the modifiers. Only SdrTableObj is adding own UNDOs
        // in its SdrTableObj::endSpecialDrag, so currently not possible. OTOH it uses
        // a CreateUndoGeoObject(), so maybe setting SetEndDragChangesAttributes is okay. I
        // will test this now
        tools::Rectangle aBoundRect0;

        if(pObj->GetUserCall())
        {
            aBoundRect0 = pObj->GetLastBoundRect();
        }

        bRet = pObj->applySpecialDrag(DragStat());
        if (DragStat().IsEndDragChangesLayout())
        {
            auto pGeoUndo = dynamic_cast<SdrUndoGeoObj*>(pUndo.get());
            if (pGeoUndo)
                pGeoUndo->SetSkipChangeLayout(true);
        }

        if(bRet)
        {
            pObj->SetChanged();
            pObj->BroadcastObjectChange();
            pObj->SendUserCall( SdrUserCallType::Resize, aBoundRect0 );
        }

        if(bRet && bUndo )
        {
            getSdrDragView().AddUndoActions( std::move(vConnectorUndoActions) );

            if ( pUndo )
            {
                getSdrDragView().AddUndo(std::move(pUndo));
            }

            if ( pUndo2 )
            {
                getSdrDragView().AddUndo(std::move(pUndo2));
            }
        }

        if( bUndo )
            getSdrDragView().EndUndo();
    }

    return bRet;
}

PointerStyle SdrDragObjOwn::GetSdrDragPointer() const
{
    const SdrHdl* pHdl=GetDragHdl();

    if (pHdl)
    {
        return pHdl->GetPointer();
    }

    return PointerStyle::Move;
}


void SdrDragMove::createSdrDragEntryForSdrObject(const SdrObject& rOriginal)
{
    // use the view-independent primitive representation (without
    // evtl. GridOffset, that may be applied to the DragEntry individually)
    addSdrDragEntry(
        std::unique_ptr<SdrDragEntry>(
            new SdrDragEntryPrimitive2DSequence(
                rOriginal.GetViewContact().getViewIndependentPrimitive2DContainer())));

}

void SdrDragMove::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    rTarget.Move(Size(DragStat().GetDX(), DragStat().GetDY()));
}

SdrDragMove::SdrDragMove(SdrDragView& rNewView)
    : SdrDragMethod(rNewView)
    , nBestXSnap(0)
    , nBestYSnap(0)
    , bXSnapped(false)
    , bYSnapped(false)
{
    setMoveOnly(true);
}

void SdrDragMove::TakeSdrDragComment(OUString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethMove, rStr);
    rStr += " (x="
            + getSdrDragView().GetModel()->GetMetricString(DragStat().GetDX())
            + " y="
            + getSdrDragView().GetModel()->GetMetricString(DragStat().GetDY())
            + ")";

    if(getSdrDragView().IsDragWithCopy())
    {
        if(!getSdrDragView().IsInsObjPoint() && !getSdrDragView().IsInsGluePoint())
        {
            rStr += SvxResId(STR_EditWithCopy);
        }
    }
}

bool SdrDragMove::BeginSdrDrag()
{
    DragStat().SetActionRect(GetMarkedRect());
    Show();

    return true;
}

basegfx::B2DHomMatrix SdrDragMove::getCurrentTransformation()
{
    return basegfx::utils::createTranslateB2DHomMatrix(DragStat().GetDX(), DragStat().GetDY());
}

void SdrDragMove::ImpCheckSnap(const Point& rPt)
{
    Point aPt(rPt);
    SdrSnap nRet=SnapPos(aPt);
    aPt-=rPt;

    if (nRet & SdrSnap::XSNAPPED)
    {
        if (bXSnapped)
        {
            if (std::abs(aPt.X())<std::abs(nBestXSnap))
            {
                nBestXSnap=aPt.X();
            }
        }
        else
        {
            nBestXSnap=aPt.X();
            bXSnapped=true;
        }
    }

    if (nRet & SdrSnap::YSNAPPED)
    {
        if (bYSnapped)
        {
            if (std::abs(aPt.Y())<std::abs(nBestYSnap))
            {
                nBestYSnap=aPt.Y();
            }
        }
        else
        {
            nBestYSnap=aPt.Y();
            bYSnapped=true;
        }
    }
}

void SdrDragMove::MoveSdrDrag(const Point& rNoSnapPnt_)
{
    nBestXSnap=0;
    nBestYSnap=0;
    bXSnapped=false;
    bYSnapped=false;
    Point aNoSnapPnt(rNoSnapPnt_);
    const tools::Rectangle& aSR=GetMarkedRect();
    long nMovedx=aNoSnapPnt.X()-DragStat().GetStart().X();
    long nMovedy=aNoSnapPnt.Y()-DragStat().GetStart().Y();
    Point aLO(aSR.TopLeft());      aLO.AdjustX(nMovedx ); aLO.AdjustY(nMovedy );
    Point aRU(aSR.BottomRight());  aRU.AdjustX(nMovedx ); aRU.AdjustY(nMovedy );
    Point aLU(aLO.X(),aRU.Y());
    Point aRO(aRU.X(),aLO.Y());
    ImpCheckSnap(aLO);

    if (!getSdrDragView().IsMoveSnapOnlyTopLeft())
    {
        ImpCheckSnap(aRO);
        ImpCheckSnap(aLU);
        ImpCheckSnap(aRU);
    }

    Point aPnt(aNoSnapPnt.X()+nBestXSnap,aNoSnapPnt.Y()+nBestYSnap);
    bool bOrtho=getSdrDragView().IsOrtho();

    if (bOrtho)
        OrthoDistance8(DragStat().GetStart(),aPnt,getSdrDragView().IsBigOrtho());

    if (DragStat().CheckMinMoved(aNoSnapPnt))
    {
        Point aPt1(aPnt);
        tools::Rectangle aLR(getSdrDragView().GetWorkArea());
        bool bWorkArea=!aLR.IsEmpty();
        bool bDragLimit=IsDragLimit();

        if (bDragLimit || bWorkArea)
        {
            tools::Rectangle aSR2(GetMarkedRect());
            Point aD(aPt1-DragStat().GetStart());

            if (bDragLimit)
            {
                tools::Rectangle aR2(GetDragLimitRect());

                if (bWorkArea)
                    aLR.Intersection(aR2);
                else
                    aLR=aR2;
            }

            if (aSR2.Left()>aLR.Left() || aSR2.Right()<aLR.Right())
            { // any space to move to?
                aSR2.Move(aD.X(),0);

                if (aSR2.Left()<aLR.Left())
                {
                    aPt1.AdjustX( -(aSR2.Left()-aLR.Left()) );
                }
                else if (aSR2.Right()>aLR.Right())
                {
                    aPt1.AdjustX( -(aSR2.Right()-aLR.Right()) );
                }
            }
            else
                aPt1.setX(DragStat().GetStart().X() ); // no space to move to

            if (aSR2.Top()>aLR.Top() || aSR2.Bottom()<aLR.Bottom())
            { // any space to move to?
                aSR2.Move(0,aD.Y());

                if (aSR2.Top()<aLR.Top())
                {
                    aPt1.AdjustY( -(aSR2.Top()-aLR.Top()) );
                }
                else if (aSR2.Bottom()>aLR.Bottom())
                {
                    aPt1.AdjustY( -(aSR2.Bottom()-aLR.Bottom()) );
                }
            }
            else
                aPt1.setY(DragStat().GetStart().Y() ); // no space to move to
        }

        if (getSdrDragView().IsDraggingGluePoints())
        { // restrict glue points to the BoundRect of the Obj
            aPt1-=DragStat().GetStart();
            const SdrMarkList& rML=GetMarkedObjectList();
            const size_t nMarkCount=rML.GetMarkCount();

            for (size_t nMarkNum=0; nMarkNum<nMarkCount; ++nMarkNum)
            {
                const SdrMark* pM=rML.GetMark(nMarkNum);
                const SdrUShortCont& rPts = pM->GetMarkedGluePoints();

                if (!rPts.empty())
                {
                    const SdrObject* pObj=pM->GetMarkedSdrObj();
                    const SdrGluePointList* pGPL=pObj->GetGluePointList();
                    tools::Rectangle aBound(pObj->GetCurrentBoundRect());

                    for (sal_uInt16 nId : rPts)
                    {
                        sal_uInt16 nGlueNum=pGPL->FindGluePoint(nId);

                        if (nGlueNum!=SDRGLUEPOINT_NOTFOUND)
                        {
                            Point aPt((*pGPL)[nGlueNum].GetAbsolutePos(*pObj));
                            aPt+=aPt1; // move by this much
                            if (aPt.X()<aBound.Left()  ) aPt1.AdjustX( -(aPt.X()-aBound.Left()) )  ;
                            if (aPt.X()>aBound.Right() ) aPt1.AdjustX( -(aPt.X()-aBound.Right()) ) ;
                            if (aPt.Y()<aBound.Top()   ) aPt1.AdjustY( -(aPt.Y()-aBound.Top()) )   ;
                            if (aPt.Y()>aBound.Bottom()) aPt1.AdjustY( -(aPt.Y()-aBound.Bottom()) );
                        }
                    }
                }
            }

            aPt1+=DragStat().GetStart();
        }

        if (bOrtho)
            OrthoDistance8(DragStat().GetStart(),aPt1,false);

        if (aPt1!=DragStat().GetNow())
        {
            Hide();
            DragStat().NextMove(aPt1);
            tools::Rectangle aAction(GetMarkedRect());
            aAction.Move(DragStat().GetDX(),DragStat().GetDY());
            DragStat().SetActionRect(aAction);
            Show();
        }
    }
}

bool SdrDragMove::EndSdrDrag(bool bCopy)
{
    Hide();

    if (getSdrDragView().IsInsObjPoint() || getSdrDragView().IsInsGluePoint())
        bCopy=false;

    if (IsDraggingPoints())
    {
        getSdrDragView().MoveMarkedPoints(Size(DragStat().GetDX(),DragStat().GetDY()));
    }
    else if (IsDraggingGluePoints())
    {
        getSdrDragView().MoveMarkedGluePoints(Size(DragStat().GetDX(),DragStat().GetDY()),bCopy);
    }
    else
    {
        getSdrDragView().MoveMarkedObj(Size(DragStat().GetDX(),DragStat().GetDY()),bCopy);
    }

    return true;
}

PointerStyle SdrDragMove::GetSdrDragPointer() const
{
    if (IsDraggingPoints() || IsDraggingGluePoints())
    {
        return PointerStyle::MovePoint;
    }
    else
    {
        return PointerStyle::Move;
    }
}


SdrDragResize::SdrDragResize(SdrDragView& rNewView)
:   SdrDragMethod(rNewView),
    aXFact(1,1),
    aYFact(1,1)
{
}

void SdrDragResize::TakeSdrDragComment(OUString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethResize, rStr);
    Fraction aFact1(1,1);
    Point aStart(DragStat().GetStart());
    Point aRef(DragStat().GetRef1());
    sal_Int32 nXDiv(aStart.X() - aRef.X());

    if(!nXDiv)
        nXDiv = 1;

    sal_Int32 nYDiv(aStart.Y() - aRef.Y());

    if(!nYDiv)
        nYDiv = 1;

    bool bX(aXFact != aFact1 && std::abs(nXDiv) > 1);
    bool bY(aYFact != aFact1 && std::abs(nYDiv) > 1);

    if(bX || bY)
    {
        rStr += " (";

        bool bEqual(aXFact == aYFact);
        if(bX)
        {
            if(!bEqual)
                rStr += "x=";

            rStr += SdrModel::GetPercentString(aXFact);
        }

        if(bY && !bEqual)
        {
            if(bX)
                rStr += " ";

            rStr += "y=" + SdrModel::GetPercentString(aYFact);
        }

        rStr += ")";
    }

    if(getSdrDragView().IsDragWithCopy())
        rStr += SvxResId(STR_EditWithCopy);
}

bool SdrDragResize::BeginSdrDrag()
{
    SdrHdlKind eRefHdl=SdrHdlKind::Move;
    SdrHdl* pRefHdl=nullptr;

    switch (GetDragHdlKind())
    {
        case SdrHdlKind::UpperLeft: eRefHdl=SdrHdlKind::LowerRight; break;
        case SdrHdlKind::Upper: eRefHdl=SdrHdlKind::Lower; DragStat().SetHorFixed(true); break;
        case SdrHdlKind::UpperRight: eRefHdl=SdrHdlKind::LowerLeft; break;
        case SdrHdlKind::Left : eRefHdl=SdrHdlKind::Right; DragStat().SetVerFixed(true); break;
        case SdrHdlKind::Right: eRefHdl=SdrHdlKind::Left ; DragStat().SetVerFixed(true); break;
        case SdrHdlKind::LowerLeft: eRefHdl=SdrHdlKind::UpperRight; break;
        case SdrHdlKind::Lower: eRefHdl=SdrHdlKind::Upper; DragStat().SetHorFixed(true); break;
        case SdrHdlKind::LowerRight: eRefHdl=SdrHdlKind::UpperLeft; break;
        default: break;
    }

    if (eRefHdl!=SdrHdlKind::Move)
        pRefHdl=GetHdlList().GetHdl(eRefHdl);

    if (pRefHdl!=nullptr && !getSdrDragView().IsResizeAtCenter())
    {
        DragStat().SetRef1(pRefHdl->GetPos());
    }
    else
    {
        SdrHdl* pRef1=GetHdlList().GetHdl(SdrHdlKind::UpperLeft);
        SdrHdl* pRef2=GetHdlList().GetHdl(SdrHdlKind::LowerRight);

        if (pRef1!=nullptr && pRef2!=nullptr)
        {
            DragStat().SetRef1(tools::Rectangle(pRef1->GetPos(),pRef2->GetPos()).Center());
        }
        else
        {
            DragStat().SetRef1(GetMarkedRect().Center());
        }
    }

    Show();

    return true;
}

basegfx::B2DHomMatrix SdrDragResize::getCurrentTransformation()
{
    basegfx::B2DHomMatrix aRetval(basegfx::utils::createTranslateB2DHomMatrix(
        -DragStat().GetRef1().X(), -DragStat().GetRef1().Y()));
    aRetval.scale(double(aXFact), double(aYFact));
    aRetval.translate(DragStat().GetRef1().X(), DragStat().GetRef1().Y());

    return aRetval;
}

void SdrDragResize::MoveSdrDrag(const Point& rNoSnapPnt)
{
    Point aPnt(GetSnapPos(rNoSnapPnt));
    Point aStart(DragStat().GetStart());
    Point aRef(DragStat().GetRef1());
    Fraction aMaxFact(0x7FFFFFFF,1);
    tools::Rectangle aLR(getSdrDragView().GetWorkArea());
    bool bWorkArea=!aLR.IsEmpty();
    bool bDragLimit=IsDragLimit();

    if (bDragLimit || bWorkArea)
    {
        tools::Rectangle aSR(GetMarkedRect());

        if (bDragLimit)
        {
            tools::Rectangle aR2(GetDragLimitRect());

            if (bWorkArea)
                aLR.Intersection(aR2);
            else
                aLR=aR2;
        }

        if (aPnt.X()<aLR.Left())
            aPnt.setX(aLR.Left() );
        else if (aPnt.X()>aLR.Right())
            aPnt.setX(aLR.Right() );

        if (aPnt.Y()<aLR.Top())
            aPnt.setY(aLR.Top() );
        else if (aPnt.Y()>aLR.Bottom())
            aPnt.setY(aLR.Bottom() );

        if (aRef.X()>aSR.Left())
        {
            Fraction aMax(aRef.X()-aLR.Left(),aRef.X()-aSR.Left());

            if (aMax<aMaxFact)
                aMaxFact=aMax;
        }

        if (aRef.X()<aSR.Right())
        {
            Fraction aMax(aLR.Right()-aRef.X(),aSR.Right()-aRef.X());

            if (aMax<aMaxFact)
                aMaxFact=aMax;
        }

        if (aRef.Y()>aSR.Top())
        {
            Fraction aMax(aRef.Y()-aLR.Top(),aRef.Y()-aSR.Top());

            if (aMax<aMaxFact)
                aMaxFact=aMax;
        }

        if (aRef.Y()<aSR.Bottom())
        {
            Fraction aMax(aLR.Bottom()-aRef.Y(),aSR.Bottom()-aRef.Y());

            if (aMax<aMaxFact)
                aMaxFact=aMax;
        }
    }

    long nXDiv=aStart.X()-aRef.X(); if (nXDiv==0) nXDiv=1;
    long nYDiv=aStart.Y()-aRef.Y(); if (nYDiv==0) nYDiv=1;
    long nXMul=aPnt.X()-aRef.X();
    long nYMul=aPnt.Y()-aRef.Y();

    if (nXDiv<0)
    {
        nXDiv=-nXDiv;
        nXMul=-nXMul;
    }

    if (nYDiv<0)
    {
        nYDiv=-nYDiv;
        nYMul=-nYMul;
    }

    bool bXNeg=nXMul<0; if (bXNeg) nXMul=-nXMul;
    bool bYNeg=nYMul<0; if (bYNeg) nYMul=-nYMul;
    bool bOrtho=getSdrDragView().IsOrtho() || !getSdrDragView().IsResizeAllowed();

    if (!DragStat().IsHorFixed() && !DragStat().IsVerFixed())
    {
        if (std::abs(nXDiv)<=1 || std::abs(nYDiv)<=1)
            bOrtho=false;

        if (bOrtho)
        {
            if ((Fraction(nXMul,nXDiv)>Fraction(nYMul,nYDiv)) !=getSdrDragView().IsBigOrtho())
            {
                nXMul=nYMul;
                nXDiv=nYDiv;
            }
            else
            {
                nYMul=nXMul;
                nYDiv=nXDiv;
            }
        }
    }
    else
    {
        if (bOrtho)
        {
            if (DragStat().IsHorFixed())
            {
                bXNeg=false;
                nXMul=nYMul;
                nXDiv=nYDiv;
            }

            if (DragStat().IsVerFixed())
            {
                bYNeg=false;
                nYMul=nXMul;
                nYDiv=nXDiv;
            }
        }
        else
        {
            if (DragStat().IsHorFixed())
            {
                bXNeg=false;
                nXMul=1;
                nXDiv=1;
            }

            if (DragStat().IsVerFixed())
            {
                bYNeg=false;
                nYMul=1;
                nYDiv=1;
            }
        }
    }

    Fraction aNewXFact(nXMul,nXDiv);
    Fraction aNewYFact(nYMul,nYDiv);

    if (bOrtho)
    {
        if (aNewXFact>aMaxFact)
        {
            aNewXFact=aMaxFact;
            aNewYFact=aMaxFact;
        }

        if (aNewYFact>aMaxFact)
        {
            aNewXFact=aMaxFact;
            aNewYFact=aMaxFact;
        }
    }

    if (bXNeg)
        aNewXFact=Fraction(-aNewXFact.GetNumerator(),aNewXFact.GetDenominator());

    if (bYNeg)
        aNewYFact=Fraction(-aNewYFact.GetNumerator(),aNewYFact.GetDenominator());

    if (DragStat().CheckMinMoved(aPnt))
    {
        if ((!DragStat().IsHorFixed() && aPnt.X()!=DragStat().GetNow().X()) ||
            (!DragStat().IsVerFixed() && aPnt.Y()!=DragStat().GetNow().Y()))
        {
            Hide();
            DragStat().NextMove(aPnt);
            aXFact=aNewXFact;
            aYFact=aNewYFact;
            Show();
        }
    }
}

void SdrDragResize::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    rTarget.Resize(DragStat().GetRef1(),aXFact,aYFact);
}

bool SdrDragResize::EndSdrDrag(bool bCopy)
{
    Hide();

    if (IsDraggingPoints())
    {
        getSdrDragView().ResizeMarkedPoints(DragStat().GetRef1(),aXFact,aYFact);
    }
    else if (IsDraggingGluePoints())
    {
        getSdrDragView().ResizeMarkedGluePoints(DragStat().GetRef1(),aXFact,aYFact,bCopy);
    }
    else
    {
        getSdrDragView().ResizeMarkedObj(DragStat().GetRef1(),aXFact,aYFact,bCopy);
    }

    return true;
}

PointerStyle SdrDragResize::GetSdrDragPointer() const
{
    const SdrHdl* pHdl=GetDragHdl();

    if (pHdl!=nullptr)
    {
        return pHdl->GetPointer();
    }

    return PointerStyle::Move;
}


void SdrDragRotate::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    rTarget.Rotate(DragStat().GetRef1(), nAngle, sin(nAngle * F_PI18000), cos(nAngle * F_PI18000));
}

SdrDragRotate::SdrDragRotate(SdrDragView& rNewView)
:   SdrDragMethod(rNewView),
    nSin(0.0),
    nCos(1.0),
    nAngle0(0),
    nAngle(0),
    bRight(false)
{
}

void SdrDragRotate::TakeSdrDragComment(OUString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethRotate, rStr);
    rStr += " (";
    sal_Int32 nTmpAngle(NormAngle36000(nAngle));

    if(bRight && nAngle)
    {
        nTmpAngle -= 36000;
    }

    rStr += SdrModel::GetAngleString(nTmpAngle) + ")";

    if(getSdrDragView().IsDragWithCopy())
        rStr += SvxResId(STR_EditWithCopy);
}

bool SdrDragRotate::BeginSdrDrag()
{
    SdrHdl* pH=GetHdlList().GetHdl(SdrHdlKind::Ref1);

    if (nullptr != pH)
    {
        Show();
        DragStat().SetRef1(pH->GetPos());
        nAngle0=GetAngle(DragStat().GetStart()-DragStat().GetRef1());
        return true;
    }

    // RotGrfFlyFrame: Support rotation around center *without* Ref1 (normally
    // the rotation point)
    const tools::Rectangle aLocalMarkRect(getSdrDragView().GetMarkedObjRect());

    if(!aLocalMarkRect.IsEmpty())
    {
        Show();
        DragStat().SetRef1(aLocalMarkRect.Center());
        nAngle0=GetAngle(DragStat().GetStart()-DragStat().GetRef1());
        return true;
    }

    OSL_FAIL("SdrDragRotate::BeginSdrDrag(): No reference point handle found.");
    return false;
}

basegfx::B2DHomMatrix SdrDragRotate::getCurrentTransformation()
{
    return basegfx::utils::createRotateAroundPoint(
        DragStat().GetRef1().X(), DragStat().GetRef1().Y(),
        -atan2(nSin, nCos));
}

void SdrDragRotate::MoveSdrDrag(const Point& rPnt_)
{
    Point aPnt(rPnt_);
    if (DragStat().CheckMinMoved(aPnt))
    {
        long nNewAngle=NormAngle36000(GetAngle(aPnt-DragStat().GetRef1())-nAngle0);
        long nSA=0;

        if (getSdrDragView().IsAngleSnapEnabled())
            nSA=getSdrDragView().GetSnapAngle();

        if (!getSdrDragView().IsRotateAllowed())
            nSA=9000;

        if (nSA!=0)
        { // angle snapping
            nNewAngle+=nSA/2;
            nNewAngle/=nSA;
            nNewAngle*=nSA;
        }

        nNewAngle=NormAngle18000(nNewAngle);

        if (nAngle!=nNewAngle)
        {
            sal_uInt16 nSekt0=GetAngleSector(nAngle);
            sal_uInt16 nSekt1=GetAngleSector(nNewAngle);

            if (nSekt0==0 && nSekt1==3)
                bRight=true;

            if (nSekt0==3 && nSekt1==0)
                bRight=false;

            nAngle=nNewAngle;
            double a = nAngle * F_PI18000;
            double nSin1=sin(a); // calculate now, so as little time as possible
            double nCos1=cos(a); // passes between Hide() and Show()
            Hide();
            nSin=nSin1;
            nCos=nCos1;
            DragStat().NextMove(aPnt);
            Show();
        }
    }
}

bool SdrDragRotate::EndSdrDrag(bool bCopy)
{
    Hide();

    if (nAngle!=0)
    {
        if (IsDraggingPoints())
        {
            getSdrDragView().RotateMarkedPoints(DragStat().GetRef1(),nAngle);
        }
        else if (IsDraggingGluePoints())
        {
            getSdrDragView().RotateMarkedGluePoints(DragStat().GetRef1(),nAngle,bCopy);
        }
        else
        {
            getSdrDragView().RotateMarkedObj(DragStat().GetRef1(),nAngle,bCopy);
        }
    }
    return true;
}

PointerStyle SdrDragRotate::GetSdrDragPointer() const
{
    return PointerStyle::Rotate;
}


SdrDragShear::SdrDragShear(SdrDragView& rNewView, bool bSlant1)
:   SdrDragMethod(rNewView),
    aFact(1,1),
    nAngle0(0),
    nAngle(0),
    nTan(0.0),
    bVertical(false),
    bResize(false),
    bUpSideDown(false),
    bSlant(bSlant1)
{
}

void SdrDragShear::TakeSdrDragComment(OUString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethShear, rStr);
    rStr += " (";

    sal_Int32 nTmpAngle(nAngle);

    if(bUpSideDown)
        nTmpAngle += 18000;

    nTmpAngle = NormAngle18000(nTmpAngle);

    rStr += SdrModel::GetAngleString(nTmpAngle) + ")";

    if(getSdrDragView().IsDragWithCopy())
        rStr += SvxResId(STR_EditWithCopy);
}

bool SdrDragShear::BeginSdrDrag()
{
    SdrHdlKind eRefHdl=SdrHdlKind::Move;
    SdrHdl* pRefHdl=nullptr;

    switch (GetDragHdlKind())
    {
        case SdrHdlKind::Upper: eRefHdl=SdrHdlKind::Lower; break;
        case SdrHdlKind::Lower: eRefHdl=SdrHdlKind::Upper; break;
        case SdrHdlKind::Left : eRefHdl=SdrHdlKind::Right; bVertical=true; break;
        case SdrHdlKind::Right: eRefHdl=SdrHdlKind::Left ; bVertical=true; break;
        default: break;
    }

    if (eRefHdl!=SdrHdlKind::Move)
        pRefHdl=GetHdlList().GetHdl(eRefHdl);

    if (pRefHdl!=nullptr)
    {
        DragStat().SetRef1(pRefHdl->GetPos());
        nAngle0=GetAngle(DragStat().GetStart()-DragStat().GetRef1());
    }
    else
    {
        OSL_FAIL("SdrDragShear::BeginSdrDrag(): No reference point handle for shearing found.");
        return false;
    }

    Show();
    return true;
}

basegfx::B2DHomMatrix SdrDragShear::getCurrentTransformation()
{
    basegfx::B2DHomMatrix aRetval(basegfx::utils::createTranslateB2DHomMatrix(
        -DragStat().GetRef1().X(), -DragStat().GetRef1().Y()));

    if (bResize)
    {
        if (bVertical)
        {
            aRetval.scale(double(aFact), 1.0);
            aRetval.shearY(-nTan);
        }
        else
        {
            aRetval.scale(1.0, double(aFact));
            aRetval.shearX(-nTan);
        }
    }

    aRetval.translate(DragStat().GetRef1().X(), DragStat().GetRef1().Y());

    return aRetval;
}

void SdrDragShear::MoveSdrDrag(const Point& rPnt)
{
    if (!DragStat().CheckMinMoved(rPnt))
        return;

    bResize=!getSdrDragView().IsOrtho();
    long nSA=0;

    if (getSdrDragView().IsAngleSnapEnabled())
        nSA=getSdrDragView().GetSnapAngle();

    Point aP0(DragStat().GetStart());
    Point aPnt(rPnt);
    Fraction aNewFract(1,1);

    // if angle snapping not activated, snap to raster (except when using slant)
    if (nSA==0 && !bSlant)
        aPnt=GetSnapPos(aPnt);

    if (!bSlant && !bResize)
    { // shear, but no resize
        if (bVertical)
            aPnt.setX(aP0.X() );
        else
            aPnt.setY(aP0.Y() );
    }

    Point aRef(DragStat().GetRef1());
    Point aDif(aPnt-aRef);

    long nNewAngle=0;

    if (bSlant)
    {
        nNewAngle=NormAngle18000(-(GetAngle(aDif)-nAngle0));

        if (bVertical)
            nNewAngle=NormAngle18000(-nNewAngle);
    }
    else
    {
        if (bVertical)
            nNewAngle=NormAngle18000(GetAngle(aDif));
        else
            nNewAngle=NormAngle18000(-(GetAngle(aDif)-9000));

        if (nNewAngle<-9000 || nNewAngle>9000)
            nNewAngle=NormAngle18000(nNewAngle+18000);

        if (bResize)
        {
            Point aPt2(aPnt);

            if (nSA!=0)
                aPt2=GetSnapPos(aPnt); // snap this one in any case

            if (bVertical)
            {
                aNewFract=Fraction(aPt2.X()-aRef.X(),aP0.X()-aRef.X());
            }
            else
            {
                aNewFract=Fraction(aPt2.Y()-aRef.Y(),aP0.Y()-aRef.Y());
            }
        }
    }

    bool bNeg=nNewAngle<0;

    if (bNeg)
        nNewAngle=-nNewAngle;

    if (nSA!=0)
    { // angle snapping
        nNewAngle+=nSA/2;
        nNewAngle/=nSA;
        nNewAngle*=nSA;
    }

    nNewAngle=NormAngle36000(nNewAngle);
    bUpSideDown=nNewAngle>9000 && nNewAngle<27000;

    if (bSlant)
    { // calculate resize for slant
        // when angle snapping is activated, disable 89 degree limit
        long nTmpAngle=nNewAngle;
        if (bUpSideDown) nNewAngle-=18000;
        if (bNeg) nTmpAngle=-nTmpAngle;
        bResize=true;
        aNewFract = cos(nTmpAngle * F_PI18000);
        aFact.ReduceInaccurate(10); // three decimals should be enough
    }

    if (nNewAngle>8900)
        nNewAngle=8900;

    if (bNeg)
        nNewAngle=-nNewAngle;

    if (nAngle!=nNewAngle || aFact!=aNewFract)
    {
        nAngle=nNewAngle;
        aFact=aNewFract;
        double a = nAngle * F_PI18000;
        double nTan1=tan(a); // calculate now, so as little time as possible passes between Hide() and Show()
        Hide();
        nTan=nTan1;
        DragStat().NextMove(rPnt);
        Show();
    }
}

void SdrDragShear::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    if (bResize)
    {
        if (bVertical)
        {
            rTarget.Resize(DragStat().GetRef1(),aFact,Fraction(1,1));
        }
        else
        {
            rTarget.Resize(DragStat().GetRef1(),Fraction(1,1),aFact);
        }
    }

    if (nAngle!=0)
    {
        rTarget.Shear(DragStat().GetRef1(), nAngle, tan(nAngle * F_PI18000), bVertical);
    }
}

bool SdrDragShear::EndSdrDrag(bool bCopy)
{
    Hide();

    if (bResize && aFact==Fraction(1,1))
        bResize=false;

    if (nAngle!=0 || bResize)
    {
        if (nAngle!=0 && bResize)
        {
            OUString aStr;
            ImpTakeDescriptionStr(STR_EditShear,aStr);

            if (bCopy)
                aStr += SvxResId(STR_EditWithCopy);

            getSdrDragView().BegUndo(aStr);
        }

        if (bResize)
        {
            if (bVertical)
            {
                getSdrDragView().ResizeMarkedObj(DragStat().GetRef1(),aFact,Fraction(1,1),bCopy);
            }
            else
            {
                getSdrDragView().ResizeMarkedObj(DragStat().GetRef1(),Fraction(1,1),aFact,bCopy);
            }

            bCopy=false;
        }

        if (nAngle!=0)
        {
            getSdrDragView().ShearMarkedObj(DragStat().GetRef1(),nAngle,bVertical,bCopy);
        }

        if (nAngle!=0 && bResize)
            getSdrDragView().EndUndo();

        return true;
    }

    return false;
}

PointerStyle SdrDragShear::GetSdrDragPointer() const
{
    if (bVertical)
        return PointerStyle::VShear;
    else
        return PointerStyle::HShear;
}


void SdrDragMirror::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    if(bMirrored)
    {
        rTarget.Mirror(DragStat().GetRef1(), DragStat().GetRef2());
    }
}

SdrDragMirror::SdrDragMirror(SdrDragView& rNewView)
:   SdrDragMethod(rNewView),
    nAngle(0),
    bMirrored(false),
    bSide0(false)
{
}

bool SdrDragMirror::ImpCheckSide(const Point& rPnt) const
{
    long nAngle1=GetAngle(rPnt-DragStat().GetRef1());
    nAngle1-=nAngle;
    nAngle1=NormAngle36000(nAngle1);

    return nAngle1<18000;
}

void SdrDragMirror::TakeSdrDragComment(OUString& rStr) const
{
    if (aDif.X()==0)
        ImpTakeDescriptionStr(STR_DragMethMirrorHori,rStr);
    else if (aDif.Y()==0)
        ImpTakeDescriptionStr(STR_DragMethMirrorVert,rStr);
    else if (std::abs(aDif.X()) == std::abs(aDif.Y()))
        ImpTakeDescriptionStr(STR_DragMethMirrorDiag,rStr);
    else
        ImpTakeDescriptionStr(STR_DragMethMirrorFree,rStr);

    if (getSdrDragView().IsDragWithCopy())
        rStr+=SvxResId(STR_EditWithCopy);
}

bool SdrDragMirror::BeginSdrDrag()
{
    SdrHdl* pH1=GetHdlList().GetHdl(SdrHdlKind::Ref1);
    SdrHdl* pH2=GetHdlList().GetHdl(SdrHdlKind::Ref2);

    if (pH1!=nullptr && pH2!=nullptr)
    {
        DragStat().SetRef1(pH1->GetPos());
        DragStat().SetRef2(pH2->GetPos());
        Ref1()=pH1->GetPos();
        Ref2()=pH2->GetPos();
        aDif=pH2->GetPos()-pH1->GetPos();
        bool b90=(aDif.X()==0) || aDif.Y()==0;
        bool b45=b90 || (std::abs(aDif.X()) == std::abs(aDif.Y()));
        nAngle=NormAngle36000(GetAngle(aDif));

        if (!getSdrDragView().IsMirrorAllowed() && !b45)
            return false; // free choice of axis angle not allowed

        if (!getSdrDragView().IsMirrorAllowed() && !b90)
            return false;  // 45 degrees not allowed either

        bSide0=ImpCheckSide(DragStat().GetStart());
        Show();
        return true;
    }
    else
    {
        OSL_FAIL("SdrDragMirror::BeginSdrDrag(): Axis of reflection not found.");
        return false;
    }
}

basegfx::B2DHomMatrix SdrDragMirror::getCurrentTransformation()
{
    basegfx::B2DHomMatrix aRetval;

    if (bMirrored)
    {
        const double fDeltaX(DragStat().GetRef2().X() - DragStat().GetRef1().X());
        const double fDeltaY(DragStat().GetRef2().Y() - DragStat().GetRef1().Y());
        const double fRotation(atan2(fDeltaY, fDeltaX));

        aRetval = basegfx::utils::createTranslateB2DHomMatrix(-DragStat().GetRef1().X(), -DragStat().GetRef1().Y());
        aRetval.rotate(-fRotation);
        aRetval.scale(1.0, -1.0);
        aRetval.rotate(fRotation);
        aRetval.translate(DragStat().GetRef1().X(), DragStat().GetRef1().Y());
    }

    return aRetval;
}

void SdrDragMirror::MoveSdrDrag(const Point& rPnt)
{
    if (DragStat().CheckMinMoved(rPnt))
    {
        bool bNewSide=ImpCheckSide(rPnt);
        bool bNewMirrored=bSide0!=bNewSide;

        if (bMirrored!=bNewMirrored)
        {
            Hide();
            bMirrored=bNewMirrored;
            DragStat().NextMove(rPnt);
            Show();
        }
    }
}

bool SdrDragMirror::EndSdrDrag(bool bCopy)
{
    Hide();

    if (bMirrored)
    {
        getSdrDragView().MirrorMarkedObj(DragStat().GetRef1(),DragStat().GetRef2(),bCopy);
    }

    return true;
}

PointerStyle SdrDragMirror::GetSdrDragPointer() const
{
    return PointerStyle::Mirror;
}


SdrDragGradient::SdrDragGradient(SdrDragView& rNewView, bool bGrad)
:   SdrDragMethod(rNewView),
    pIAOHandle(nullptr),
    bIsGradient(bGrad)
{
}

void SdrDragGradient::TakeSdrDragComment(OUString& rStr) const
{
    if(IsGradient())
        ImpTakeDescriptionStr(STR_DragMethGradient, rStr);
    else
        ImpTakeDescriptionStr(STR_DragMethTransparence, rStr);
}

bool SdrDragGradient::BeginSdrDrag()
{
    bool bRetval(false);

    pIAOHandle = static_cast<SdrHdlGradient*>(GetHdlList().GetHdl(IsGradient() ? SdrHdlKind::Gradient : SdrHdlKind::Transparence));

    if(pIAOHandle)
    {
        // save old values
        DragStat().SetRef1( pIAOHandle->GetPos() );
        DragStat().SetRef2( pIAOHandle->Get2ndPos() );

        // what was hit?
        bool bHit(false);
        SdrHdlColor* pColHdl = pIAOHandle->GetColorHdl1();

        // init handling flags
        pIAOHandle->SetMoveSingleHandle(false);
        pIAOHandle->SetMoveFirstHandle(false);

        // test first color handle
        if(pColHdl)
        {
            basegfx::B2DPoint aPosition(DragStat().GetStart().X(), DragStat().GetStart().Y());

            if(pColHdl->getOverlayObjectList().isHitLogic(aPosition))
            {
                bHit = true;
                pIAOHandle->SetMoveSingleHandle(true);
                pIAOHandle->SetMoveFirstHandle(true);
            }
        }

        // test second color handle
        pColHdl = pIAOHandle->GetColorHdl2();

        if(!bHit && pColHdl)
        {
            basegfx::B2DPoint aPosition(DragStat().GetStart().X(), DragStat().GetStart().Y());

            if(pColHdl->getOverlayObjectList().isHitLogic(aPosition))
            {
                bHit = true;
                pIAOHandle->SetMoveSingleHandle(true);
            }
        }

        // test gradient handle itself
        if(!bHit)
        {
            basegfx::B2DPoint aPosition(DragStat().GetStart().X(), DragStat().GetStart().Y());

            if(pIAOHandle->getOverlayObjectList().isHitLogic(aPosition))
            {
                bHit = true;
            }
        }

        // everything up and running :o}
        bRetval = bHit;
    }
    else
    {
        OSL_FAIL("SdrDragGradient::BeginSdrDrag(): IAOGradient not found.");
    }

    return bRetval;
}

void SdrDragGradient::MoveSdrDrag(const Point& rPnt)
{
    if(pIAOHandle && DragStat().CheckMinMoved(rPnt))
    {
        DragStat().NextMove(rPnt);

        // Do the Move here!!! DragStat().GetStart()
        Point aMoveDiff = rPnt - DragStat().GetStart();

        if(pIAOHandle->IsMoveSingleHandle())
        {
            if(pIAOHandle->IsMoveFirstHandle())
            {
                pIAOHandle->SetPos(DragStat().GetRef1() + aMoveDiff);
                if(pIAOHandle->GetColorHdl1())
                    pIAOHandle->GetColorHdl1()->SetPos(DragStat().GetRef1() + aMoveDiff);
            }
            else
            {
                pIAOHandle->Set2ndPos(DragStat().GetRef2() + aMoveDiff);
                if(pIAOHandle->GetColorHdl2())
                    pIAOHandle->GetColorHdl2()->SetPos(DragStat().GetRef2() + aMoveDiff);
            }
        }
        else
        {
            pIAOHandle->SetPos(DragStat().GetRef1() + aMoveDiff);
            pIAOHandle->Set2ndPos(DragStat().GetRef2() + aMoveDiff);

            if(pIAOHandle->GetColorHdl1())
                pIAOHandle->GetColorHdl1()->SetPos(DragStat().GetRef1() + aMoveDiff);

            if(pIAOHandle->GetColorHdl2())
                pIAOHandle->GetColorHdl2()->SetPos(DragStat().GetRef2() + aMoveDiff);
        }

        // new state
        pIAOHandle->FromIAOToItem(getSdrDragView().GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj(), false, false);
    }
}

bool SdrDragGradient::EndSdrDrag(bool /*bCopy*/)
{
    Ref1() = pIAOHandle->GetPos();
    Ref2() = pIAOHandle->Get2ndPos();

    // new state
    pIAOHandle->FromIAOToItem(getSdrDragView().GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj(), true, true);

    return true;
}

void SdrDragGradient::CancelSdrDrag()
{
    // restore old values
    pIAOHandle->SetPos(DragStat().GetRef1());
    pIAOHandle->Set2ndPos(DragStat().GetRef2());

    if(pIAOHandle->GetColorHdl1())
        pIAOHandle->GetColorHdl1()->SetPos(DragStat().GetRef1());

    if(pIAOHandle->GetColorHdl2())
        pIAOHandle->GetColorHdl2()->SetPos(DragStat().GetRef2());

    // new state
    pIAOHandle->FromIAOToItem(getSdrDragView().GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj(), true, false);
}

PointerStyle SdrDragGradient::GetSdrDragPointer() const
{
    return PointerStyle::RefHand;
}


SdrDragCrook::SdrDragCrook(SdrDragView& rNewView)
:   SdrDragMethod(rNewView),
    aFact(1,1),
    bContortionAllowed(false),
    bNoContortionAllowed(false),
    bContortion(false),
    bResizeAllowed(false),
    bResize(false),
    bRotateAllowed(false),
    bRotate(false),
    bVertical(false),
    bValid(false),
    bLft(false),
    bRgt(false),
    bUpr(false),
    bLwr(false),
    bAtCenter(false),
    nAngle(0),
    nMarkSize(0),
    eMode(SdrCrookMode::Rotate)
{
}

void SdrDragCrook::TakeSdrDragComment(OUString& rStr) const
{
    ImpTakeDescriptionStr(!bContortion ? STR_DragMethCrook : STR_DragMethCrookContortion, rStr);

    if(bValid)
    {
        rStr += " (";

        sal_Int32 nVal(nAngle);

        if(bAtCenter)
            nVal *= 2;

        nVal = std::abs(nVal);
        rStr += SdrModel::GetAngleString(nVal) + ")";
    }

    if(getSdrDragView().IsDragWithCopy())
        rStr += SvxResId(STR_EditWithCopy);
}

// These defines parametrize the created raster
// for interactions
#define DRAG_CROOK_RASTER_MINIMUM   (4)
#define DRAG_CROOK_RASTER_MAXIMUM   (15)
#define DRAG_CROOK_RASTER_DISTANCE  (30)

static basegfx::B2DPolyPolygon impCreateDragRaster(SdrPageView const & rPageView, const tools::Rectangle& rMarkRect)
{
    basegfx::B2DPolyPolygon aRetval;

    if(rPageView.PageWindowCount())
    {
        OutputDevice& rOut = rPageView.GetPageWindow(0)->GetPaintWindow().GetOutputDevice();
        tools::Rectangle aPixelSize = rOut.LogicToPixel(rMarkRect);
        sal_uInt32 nHorDiv(aPixelSize.GetWidth() / DRAG_CROOK_RASTER_DISTANCE);
        sal_uInt32 nVerDiv(aPixelSize.GetHeight() / DRAG_CROOK_RASTER_DISTANCE);

        if(nHorDiv > DRAG_CROOK_RASTER_MAXIMUM)
            nHorDiv = DRAG_CROOK_RASTER_MAXIMUM;
        if(nHorDiv < DRAG_CROOK_RASTER_MINIMUM)
            nHorDiv = DRAG_CROOK_RASTER_MINIMUM;

        if(nVerDiv > DRAG_CROOK_RASTER_MAXIMUM)
            nVerDiv = DRAG_CROOK_RASTER_MAXIMUM;
        if(nVerDiv < DRAG_CROOK_RASTER_MINIMUM)
            nVerDiv = DRAG_CROOK_RASTER_MINIMUM;

        const double fXLen(rMarkRect.GetWidth() / static_cast<double>(nHorDiv));
        const double fYLen(rMarkRect.GetHeight() / static_cast<double>(nVerDiv));
        double fYPos(rMarkRect.Top());
        sal_uInt32 a, b;

        for(a = 0; a <= nVerDiv; a++)
        {
            // horizontal lines
            for(b = 0; b < nHorDiv; b++)
            {
                basegfx::B2DPolygon aHorLineSegment;

                const double fNewX(rMarkRect.Left() + (b * fXLen));
                aHorLineSegment.append(basegfx::B2DPoint(fNewX, fYPos));
                aHorLineSegment.appendBezierSegment(
                    basegfx::B2DPoint(fNewX + (fXLen * (1.0 / 3.0)), fYPos),
                    basegfx::B2DPoint(fNewX + (fXLen * (2.0 / 3.0)), fYPos),
                    basegfx::B2DPoint(fNewX + fXLen, fYPos));
                aRetval.append(aHorLineSegment);
            }

            // increments
            fYPos += fYLen;
        }

        double fXPos(rMarkRect.Left());

        for(a = 0; a <= nHorDiv; a++)
        {
            // vertical lines
            for(b = 0; b < nVerDiv; b++)
            {
                basegfx::B2DPolygon aVerLineSegment;

                const double fNewY(rMarkRect.Top() + (b * fYLen));
                aVerLineSegment.append(basegfx::B2DPoint(fXPos, fNewY));
                aVerLineSegment.appendBezierSegment(
                    basegfx::B2DPoint(fXPos, fNewY + (fYLen * (1.0 / 3.0))),
                    basegfx::B2DPoint(fXPos, fNewY + (fYLen * (2.0 / 3.0))),
                    basegfx::B2DPoint(fXPos, fNewY + fYLen));
                aRetval.append(aVerLineSegment);
            }

            // increments
            fXPos += fXLen;
        }
    }

    return aRetval;
}

void SdrDragCrook::createSdrDragEntries()
{
    // Add extended frame raster first, so it will be behind objects
    if(getSdrDragView().GetSdrPageView())
    {
        const basegfx::B2DPolyPolygon aDragRaster(impCreateDragRaster(*getSdrDragView().GetSdrPageView(), GetMarkedRect()));

        if(aDragRaster.count())
        {
            addSdrDragEntry(std::unique_ptr<SdrDragEntry>(new SdrDragEntryPolyPolygon(aDragRaster)));
        }
    }

    // call parent
    SdrDragMethod::createSdrDragEntries();
}

bool SdrDragCrook::BeginSdrDrag()
{
    bContortionAllowed=getSdrDragView().IsCrookAllowed();
    bNoContortionAllowed=getSdrDragView().IsCrookAllowed(true);
    bResizeAllowed=getSdrDragView().IsResizeAllowed();
    bRotateAllowed=getSdrDragView().IsRotateAllowed();

    if (bContortionAllowed || bNoContortionAllowed)
    {
        bVertical=(GetDragHdlKind()==SdrHdlKind::Lower || GetDragHdlKind()==SdrHdlKind::Upper);
        aMarkRect=GetMarkedRect();
        aMarkCenter=aMarkRect.Center();
        nMarkSize=bVertical ? (aMarkRect.GetHeight()-1) : (aMarkRect.GetWidth()-1);
        aCenter=aMarkCenter;
        aStart=DragStat().GetStart();
        Show();
        return true;
    }
    else
    {
        return false;
    }
}

void SdrDragCrook::MovAllPoints(basegfx::B2DPolyPolygon& rTarget)
{
    SdrPageView* pPV = getSdrDragView().GetSdrPageView();

    if(pPV)
    {
        XPolyPolygon aTempPolyPoly(rTarget);

        if (pPV->HasMarkedObjPageView())
        {
            sal_uInt16 nPolyCount=aTempPolyPoly.Count();

            if (!bContortion && !getSdrDragView().IsNoDragXorPolys())
            {
                sal_uInt16 n1st=0,nLast=0;
                Point aC(aCenter);

                while (n1st<nPolyCount)
                {
                    nLast=n1st;
                    while (nLast<nPolyCount && aTempPolyPoly[nLast].GetPointCount()!=0) nLast++;
                    tools::Rectangle aBound(aTempPolyPoly[n1st].GetBoundRect());
                    sal_uInt16 i;

                    for (i=n1st+1; i<nLast; i++)
                    {
                        aBound.Union(aTempPolyPoly[n1st].GetBoundRect());
                    }

                    Point aCtr0(aBound.Center());
                    Point aCtr1(aCtr0);

                    if (bResize)
                    {
                        Fraction aFact1(1,1);

                        if (bVertical)
                        {
                            ResizePoint(aCtr1,aC,aFact1,aFact);
                        }
                        else
                        {
                            ResizePoint(aCtr1,aC,aFact,aFact1);
                        }
                    }

                    bool bRotOk=false;
                    double nSin=0,nCos=0;

                    if (aRad.X()!=0 && aRad.Y()!=0)
                    {
                        bRotOk=bRotate;

                        switch (eMode)
                        {
                            case SdrCrookMode::Rotate : CrookRotateXPoint (aCtr1,nullptr,nullptr,aC,aRad,nSin,nCos,bVertical);           break;
                            case SdrCrookMode::Slant  : CrookSlantXPoint  (aCtr1,nullptr,nullptr,aC,aRad,nSin,nCos,bVertical);           break;
                            case SdrCrookMode::Stretch: CrookStretchXPoint(aCtr1,nullptr,nullptr,aC,aRad,nSin,nCos,bVertical,aMarkRect); break;
                        } // switch
                    }

                    aCtr1-=aCtr0;

                    for (i=n1st; i<nLast; i++)
                    {
                        if (bRotOk)
                        {
                            RotateXPoly(aTempPolyPoly[i],aCtr0,nSin,nCos);
                        }

                        aTempPolyPoly[i].Move(aCtr1.X(),aCtr1.Y());
                    }

                    n1st=nLast+1;
                }
            }
            else
            {
                sal_uInt16 i,j;

                for (j=0; j<nPolyCount; j++)
                {
                    XPolygon& aPol=aTempPolyPoly[j];
                    sal_uInt16 nPointCount=aPol.GetPointCount();
                    i=0;

                    while (i<nPointCount)
                    {
                        Point* pPnt=&aPol[i];
                        Point* pC1=nullptr;
                        Point* pC2=nullptr;

                        if (i+1<nPointCount && aPol.IsControl(i))
                        { // control point on the left
                            pC1=pPnt;
                            i++;
                            pPnt=&aPol[i];
                        }

                        i++;

                        if (i<nPointCount && aPol.IsControl(i))
                        { // control point on the right
                            pC2=&aPol[i];
                            i++;
                        }

                        MovCrookPoint(*pPnt,pC1,pC2);
                    }
                }
            }
        }

        rTarget = aTempPolyPoly.getB2DPolyPolygon();
    }
}

void SdrDragCrook::MovCrookPoint(Point& rPnt, Point* pC1, Point* pC2)
{
    bool bVert=bVertical;
    bool bC1=pC1!=nullptr;
    bool bC2=pC2!=nullptr;
    Point aC(aCenter);

    if (bResize)
    {
        Fraction aFact1(1,1);

        if (bVert)
        {
            ResizePoint(rPnt,aC,aFact1,aFact);

            if (bC1)
                ResizePoint(*pC1,aC,aFact1,aFact);

            if (bC2)
                ResizePoint(*pC2,aC,aFact1,aFact);
        }
        else
        {
            ResizePoint(rPnt,aC,aFact,aFact1);

            if (bC1)
                ResizePoint(*pC1,aC,aFact,aFact1);

            if (bC2)
                ResizePoint(*pC2,aC,aFact,aFact1);
        }
    }

    if (aRad.X()!=0 && aRad.Y()!=0)
    {
        double nSin,nCos;

        switch (eMode)
        {
            case SdrCrookMode::Rotate : CrookRotateXPoint (rPnt,pC1,pC2,aC,aRad,nSin,nCos,bVert);           break;
            case SdrCrookMode::Slant  : CrookSlantXPoint  (rPnt,pC1,pC2,aC,aRad,nSin,nCos,bVert);           break;
            case SdrCrookMode::Stretch: CrookStretchXPoint(rPnt,pC1,pC2,aC,aRad,nSin,nCos,bVert,aMarkRect); break;
        } // switch
    }
}

void SdrDragCrook::MoveSdrDrag(const Point& rPnt)
{
    if (!DragStat().CheckMinMoved(rPnt))
        return;

    bool bNewMoveOnly=getSdrDragView().IsMoveOnlyDragging();
    bAtCenter=false;
    SdrCrookMode eNewMode=getSdrDragView().GetCrookMode();
    bool bNewContortion=!bNewMoveOnly && ((bContortionAllowed && !getSdrDragView().IsCrookNoContortion()) || !bNoContortionAllowed);
    bResize=!getSdrDragView().IsOrtho() && bResizeAllowed && !bNewMoveOnly;
    bool bNewRotate=bRotateAllowed && !bNewContortion && !bNewMoveOnly && eNewMode==SdrCrookMode::Rotate;

    Point aPnt(GetSnapPos(rPnt));

    Point aNewCenter(aMarkCenter.X(),aStart.Y());

    if (bVertical)
    {
        aNewCenter.setX(aStart.X() );
        aNewCenter.setY(aMarkCenter.Y() );
    }

    if (!getSdrDragView().IsCrookAtCenter())
    {
        switch (GetDragHdlKind())
        {
            case SdrHdlKind::UpperLeft: aNewCenter.setX(aMarkRect.Right() );  bLft=true; break;
            case SdrHdlKind::Upper: aNewCenter.setY(aMarkRect.Bottom() ); bUpr=true; break;
            case SdrHdlKind::UpperRight: aNewCenter.setX(aMarkRect.Left() );   bRgt=true; break;
            case SdrHdlKind::Left : aNewCenter.setX(aMarkRect.Right() );  bLft=true; break;
            case SdrHdlKind::Right: aNewCenter.setX(aMarkRect.Left() );   bRgt=true; break;
            case SdrHdlKind::LowerLeft: aNewCenter.setX(aMarkRect.Right() );  bLft=true; break;
            case SdrHdlKind::Lower: aNewCenter.setY(aMarkRect.Top() );    bLwr=true; break;
            case SdrHdlKind::LowerRight: aNewCenter.setX(aMarkRect.Left() );   bRgt=true; break;
            default: bAtCenter=true;
        }
    }
    else
        bAtCenter=true;

    Fraction aNewFract(1,1);
    long dx1=aPnt.X()-aNewCenter.X();
    long dy1=aPnt.Y()-aNewCenter.Y();
    bValid=bVertical ? dx1!=0 : dy1!=0;

    if (bValid)
    {
        if (bVertical)
            bValid = std::abs(dx1)*100>std::abs(dy1);
        else
            bValid = std::abs(dy1)*100>std::abs(dx1);
    }

    long nNewRad=0;
    nAngle=0;

    if (bValid)
    {
        double a=0; // slope of the radius
        long nPntWink=0;

        if (bVertical)
        {
            a=static_cast<double>(dy1)/static_cast<double>(dx1); // slope of the radius
            nNewRad=(static_cast<long>(dy1*a)+dx1) /2;
            aNewCenter.AdjustX(nNewRad );
            nPntWink=GetAngle(aPnt-aNewCenter);
        }
        else
        {
            a=static_cast<double>(dx1)/static_cast<double>(dy1); // slope of the radius
            nNewRad=(static_cast<long>(dx1*a)+dy1) /2;
            aNewCenter.AdjustY(nNewRad );
            nPntWink=GetAngle(aPnt-aNewCenter)-9000;
        }

        if (!bAtCenter)
        {
            if (nNewRad<0)
            {
                if (bRgt) nPntWink+=18000;
                if (bLft) nPntWink=18000-nPntWink;
                if (bLwr) nPntWink=-nPntWink;
            }
            else
            {
                if (bRgt) nPntWink=-nPntWink;
                if (bUpr) nPntWink=18000-nPntWink;
                if (bLwr) nPntWink+=18000;
            }

            nPntWink=NormAngle36000(nPntWink);
        }
        else
        {
            if (nNewRad<0) nPntWink+=18000;
            if (bVertical) nPntWink=18000-nPntWink;
            nPntWink=NormAngle18000(nPntWink);
            nPntWink = std::abs(nPntWink);
        }

        double nUmfang = 2 * std::abs(nNewRad) * M_PI;

        if (bResize)
        {
            long nMul=static_cast<long>(nUmfang*NormAngle36000(nPntWink)/36000);

            if (bAtCenter)
                nMul*=2;

            aNewFract=Fraction(nMul,nMarkSize);
            nAngle=nPntWink;
        }
        else
        {
            nAngle=static_cast<long>((nMarkSize*360/nUmfang)*100)/2;

            if (nAngle==0)
                bValid=false;
        }
    }

    if (nAngle==0 || nNewRad==0)
        bValid=false;

    if (!bValid)
        nNewRad=0;

    if (!bValid && bResize)
    {
        long nMul=bVertical ? dy1 : dx1;

        if (bLft || bUpr)
            nMul=-nMul;

        long nDiv=nMarkSize;

        if (bAtCenter)
        {
            nMul*=2;
            nMul = std::abs(nMul);
        }

        aNewFract=Fraction(nMul,nDiv);
    }

    if (aNewCenter==aCenter && bNewContortion==bContortion && aNewFract==aFact &&
        bNewMoveOnly == getMoveOnly() && bNewRotate==bRotate && eNewMode==eMode)
        return;

    Hide();
    setMoveOnly(bNewMoveOnly);
    bRotate=bNewRotate;
    eMode=eNewMode;
    bContortion=bNewContortion;
    aCenter=aNewCenter;
    aFact=aNewFract;
    aRad=Point(nNewRad,nNewRad);
    bResize=aFact!=Fraction(1,1) && aFact.GetDenominator()!=0 && aFact.IsValid();
    DragStat().NextMove(aPnt);
    Show();
}

void SdrDragCrook::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    const bool bDoResize(aFact!=Fraction(1,1));
    const bool bDoCrook(aCenter!=aMarkCenter && aRad.X()!=0 && aRad.Y()!=0);

    if (bDoCrook || bDoResize)
    {
        if (bDoResize)
        {
            Fraction aFact1(1,1);

            if (bContortion)
            {
                if (bVertical)
                {
                    rTarget.Resize(aCenter,aFact1,aFact);
                }
                else
                {
                    rTarget.Resize(aCenter,aFact,aFact1);
                }
            }
            else
            {
                Point aCtr0(rTarget.GetSnapRect().Center());
                Point aCtr1(aCtr0);

                if (bVertical)
                {
                    ResizePoint(aCtr1,aCenter,aFact1,aFact);
                }
                else
                {
                    ResizePoint(aCtr1,aCenter,aFact,aFact1);
                }

                Size aSiz(aCtr1.X()-aCtr0.X(),aCtr1.Y()-aCtr0.Y());

                rTarget.Move(aSiz);
            }
        }

        if (bDoCrook)
        {
            const tools::Rectangle aLocalMarkRect(getSdrDragView().GetMarkedObjRect());
            const bool bLocalRotate(!bContortion && eMode == SdrCrookMode::Rotate && getSdrDragView().IsRotateAllowed());

            SdrEditView::ImpCrookObj(&rTarget,aCenter,aRad,eMode,bVertical,!bContortion,bLocalRotate,aLocalMarkRect);
        }
    }
}

void SdrDragCrook::applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget)
{
    // use helper derived from old stuff
    MovAllPoints(rTarget);
}

bool SdrDragCrook::EndSdrDrag(bool bCopy)
{
    Hide();

    if (bResize && aFact==Fraction(1,1))
        bResize=false;

    const bool bUndo = getSdrDragView().IsUndoEnabled();

    bool bDoCrook=aCenter!=aMarkCenter && aRad.X()!=0 && aRad.Y()!=0;

    if (bDoCrook || bResize)
    {
        if (bResize && bUndo)
        {
            OUString aStr;
            ImpTakeDescriptionStr(!bContortion?STR_EditCrook:STR_EditCrookContortion,aStr);

            if (bCopy)
                aStr += SvxResId(STR_EditWithCopy);

            getSdrDragView().BegUndo(aStr);
        }

        if (bResize)
        {
            Fraction aFact1(1,1);

            if (bContortion)
            {
                if (bVertical)
                    getSdrDragView().ResizeMarkedObj(aCenter,aFact1,aFact,bCopy);
                else
                    getSdrDragView().ResizeMarkedObj(aCenter,aFact,aFact1,bCopy);
            }
            else
            {
                if (bCopy)
                    getSdrDragView().CopyMarkedObj();

                const size_t nMarkCount=getSdrDragView().GetMarkedObjectList().GetMarkCount();

                for (size_t nm=0; nm<nMarkCount; ++nm)
                {
                    SdrMark* pM=getSdrDragView().GetMarkedObjectList().GetMark(nm);
                    SdrObject* pO=pM->GetMarkedSdrObj();
                    Point aCtr0(pO->GetSnapRect().Center());
                    Point aCtr1(aCtr0);

                    if (bVertical)
                        ResizePoint(aCtr1,aCenter,aFact1,aFact);
                    else
                        ResizePoint(aCtr1,aCenter,aFact,aFact1);

                    Size aSiz(aCtr1.X()-aCtr0.X(),aCtr1.Y()-aCtr0.Y());
                    if( bUndo )
                        AddUndo(getSdrDragView().GetModel()->GetSdrUndoFactory().CreateUndoMoveObject(*pO,aSiz));
                    pO->Move(aSiz);
                }
            }

            bCopy=false;
        }

        if (bDoCrook)
        {
            getSdrDragView().CrookMarkedObj(aCenter,aRad,eMode,bVertical,!bContortion,bCopy);
        }

        if (bResize && bUndo)
            getSdrDragView().EndUndo();

        return true;
    }

    return false;
}

PointerStyle SdrDragCrook::GetSdrDragPointer() const
{
    return PointerStyle::Crook;
}


SdrDragDistort::SdrDragDistort(SdrDragView& rNewView)
:   SdrDragMethod(rNewView),
    nPolyPt(0),
    bContortionAllowed(false),
    bNoContortionAllowed(false),
    bContortion(false)
{
}

void SdrDragDistort::TakeSdrDragComment(OUString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethDistort, rStr);

    rStr += " (x="
            + getSdrDragView().GetModel()->GetMetricString(DragStat().GetDX())
            + " y="
            + getSdrDragView().GetModel()->GetMetricString(DragStat().GetDY())
            + ")";

    if(getSdrDragView().IsDragWithCopy())
        rStr += SvxResId(STR_EditWithCopy);
}

void SdrDragDistort::createSdrDragEntries()
{
    // Add extended frame raster first, so it will be behind objects
    if(getSdrDragView().GetSdrPageView())
    {
        const basegfx::B2DPolyPolygon aDragRaster(impCreateDragRaster(*getSdrDragView().GetSdrPageView(), GetMarkedRect()));

        if(aDragRaster.count())
        {
            addSdrDragEntry(std::unique_ptr<SdrDragEntry>(new SdrDragEntryPolyPolygon(aDragRaster)));
        }
    }

    // call parent
    SdrDragMethod::createSdrDragEntries();
}

bool SdrDragDistort::BeginSdrDrag()
{
    bContortionAllowed=getSdrDragView().IsDistortAllowed();
    bNoContortionAllowed=getSdrDragView().IsDistortAllowed(true);

    if (bContortionAllowed || bNoContortionAllowed)
    {
        SdrHdlKind eKind=GetDragHdlKind();
        nPolyPt=0xFFFF;

        if (eKind==SdrHdlKind::UpperLeft) nPolyPt=0;
        if (eKind==SdrHdlKind::UpperRight) nPolyPt=1;
        if (eKind==SdrHdlKind::LowerRight) nPolyPt=2;
        if (eKind==SdrHdlKind::LowerLeft) nPolyPt=3;
        if (nPolyPt>3) return false;

        aMarkRect=GetMarkedRect();
        aDistortedRect=XPolygon(aMarkRect);
        Show();
        return true;
    }
    else
    {
        return false;
    }
}

void SdrDragDistort::MovAllPoints(basegfx::B2DPolyPolygon& rTarget)
{
    if (bContortion)
    {
        SdrPageView* pPV = getSdrDragView().GetSdrPageView();

        if(pPV && pPV->HasMarkedObjPageView())
        {
            basegfx::B2DPolyPolygon aDragPolygon(rTarget);
            const basegfx::B2DRange aOriginalRange(aMarkRect.Left(), aMarkRect.Top(), aMarkRect.Right(), aMarkRect.Bottom());
            const basegfx::B2DPoint aTopLeft(aDistortedRect[0].X(), aDistortedRect[0].Y());
            const basegfx::B2DPoint aTopRight(aDistortedRect[1].X(), aDistortedRect[1].Y());
            const basegfx::B2DPoint aBottomLeft(aDistortedRect[3].X(), aDistortedRect[3].Y());
            const basegfx::B2DPoint aBottomRight(aDistortedRect[2].X(), aDistortedRect[2].Y());

            aDragPolygon = basegfx::utils::distort(aDragPolygon, aOriginalRange, aTopLeft, aTopRight, aBottomLeft, aBottomRight);
            rTarget = aDragPolygon;
        }
    }
}

void SdrDragDistort::MoveSdrDrag(const Point& rPnt)
{
    if (DragStat().CheckMinMoved(rPnt))
    {
        Point aPnt(GetSnapPos(rPnt));

        if (getSdrDragView().IsOrtho())
            OrthoDistance8(DragStat().GetStart(),aPnt,getSdrDragView().IsBigOrtho());

        bool bNewContortion=(bContortionAllowed && !getSdrDragView().IsCrookNoContortion()) || !bNoContortionAllowed;

        if (bNewContortion!=bContortion || aDistortedRect[nPolyPt]!=aPnt)
        {
            Hide();
            aDistortedRect[nPolyPt]=aPnt;
            bContortion=bNewContortion;
            DragStat().NextMove(aPnt);
            Show();
        }
    }
}

bool SdrDragDistort::EndSdrDrag(bool bCopy)
{
    Hide();
    bool bDoDistort=DragStat().GetDX()!=0 || DragStat().GetDY()!=0;

    if (bDoDistort)
    {
        getSdrDragView().DistortMarkedObj(aMarkRect,aDistortedRect,!bContortion,bCopy);
        return true;
    }

    return false;
}

PointerStyle SdrDragDistort::GetSdrDragPointer() const
{
    return PointerStyle::RefHand;
}

void SdrDragDistort::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    const bool bDoDistort(DragStat().GetDX()!=0 || DragStat().GetDY()!=0);

    if (bDoDistort)
    {
        SdrEditView::ImpDistortObj(&rTarget, aMarkRect, aDistortedRect, !bContortion);
    }
}

void SdrDragDistort::applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget)
{
    // use helper derived from old stuff
    MovAllPoints(rTarget);
}


SdrDragCrop::SdrDragCrop(SdrDragView& rNewView)
:   SdrDragObjOwn(rNewView)
{
    // switch off solid dragging for crop; it just makes no sense since showing
    // a 50% transparent object above the original will not be visible
    setSolidDraggingActive(false);
}

void SdrDragCrop::TakeSdrDragComment(OUString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethCrop, rStr);

    rStr += " (x="
            + getSdrDragView().GetModel()->GetMetricString(DragStat().GetDX())
            + " y="
            + getSdrDragView().GetModel()->GetMetricString(DragStat().GetDY())
            + ")";

    if(getSdrDragView().IsDragWithCopy())
        rStr += SvxResId(STR_EditWithCopy);
}

bool SdrDragCrop::BeginSdrDrag()
{
    // call parent
    bool bRetval(SdrDragObjOwn::BeginSdrDrag());

    if(!GetDragHdl())
    {
        // we need the DragHdl, break if not there
        bRetval = false;
    }

    return bRetval;
}

bool SdrDragCrop::EndSdrDrag(bool /*bCopy*/)
{
    Hide();

    if(0 == DragStat().GetDX() && 0 == DragStat().GetDY())
    {
        // no change, done
        return false;
    }

    const SdrMarkList& rMarkList = getSdrDragView().GetMarkedObjectList();

    if(1 != rMarkList.GetMarkCount())
    {
        // Crop only with single Object selected
        return false;
    }

    // prepare for SdrGrafObj or others. This code has to work with usual
    // SdrGrafObj's from Draw/Impress/Calc, but also with SdrObjects from
    // Writer. It would be better to handle this in Writer directly, but
    // there are currently no easy mechanisms to plug an alternative interaction
    // from there
    SdrObject* pSdrObject = rMarkList.GetMark(0)->GetMarkedSdrObj();
    std::unique_ptr< SdrObject, SdrObjectFreeOp > pFullDragClone;
    bool bExternal(false);
    SdrObject* pExternalSdrObject(nullptr);

    // RotGrfFlyFrame: Crop decision for DrawingLayer/Writer now
    // locally, no two-in-one methods any more
    if (nullptr != pSdrObject && dynamic_cast< const SdrGrafObj* >(pSdrObject) ==  nullptr)
    {
        // If Writer, get the already offered for interaction SdrGrafObj
        // and set up for using that replacement object that contains the
        // real transformation. That SdrObject is owned and has to be deleted,
        // so use a std::unique_ptr with special handling for the protected
        // SDrObject destructor
        pFullDragClone.reset(pSdrObject->getFullDragClone());

        if(dynamic_cast< SdrGrafObj* >(pFullDragClone.get()))
        {
            bExternal = true;
            pExternalSdrObject = pSdrObject;
            pSdrObject = pFullDragClone.get();
        }
    }

    // get and check for SdrGrafObj now
    SdrGrafObj* pObj = dynamic_cast<SdrGrafObj*>( pSdrObject );

    if(!pObj)
    {
        return false;
    }

    // no undo for external needed, done there
    const bool bUndo(!bExternal && getSdrDragView().IsUndoEnabled());

    if(bUndo)
    {
        OUString aUndoStr;
        ImpTakeDescriptionStr(STR_DragMethCrop, aUndoStr);

        getSdrDragView().BegUndo( aUndoStr );
        getSdrDragView().AddUndo( getSdrDragView().GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
        // also need attr undo, the SdrGrafCropItem will be changed
        getSdrDragView().AddUndo( getSdrDragView().GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj));
    }

    // get the original objects transformation
    basegfx::B2DHomMatrix aOriginalMatrix;
    basegfx::B2DPolyPolygon aPolyPolygon;
    bool bShearCorrected(false);
    pObj->TRGetBaseGeometry(aOriginalMatrix, aPolyPolygon);

    {   // correct shear, it comes currently mirrored from TRGetBaseGeometry, can be removed with aw080
        const basegfx::utils::B2DHomMatrixBufferedDecompose aTmpDecomp(aOriginalMatrix);

        if(!basegfx::fTools::equalZero(aTmpDecomp.getShearX()))
        {
            bShearCorrected = true;
            aOriginalMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
                aTmpDecomp.getScale(),
                -aTmpDecomp.getShearX(),
                aTmpDecomp.getRotate(),
                aTmpDecomp.getTranslate());
        }
    }

    // generate start point of original drag vector in unit coordinates (the
    // vis-a-vis of the drag point)
    basegfx::B2DPoint aLocalStart(0.0, 0.0);
    bool bOnAxis(false);

    switch(GetDragHdlKind())
    {
        case SdrHdlKind::UpperLeft: aLocalStart.setX(1.0); aLocalStart.setY(1.0); break;
        case SdrHdlKind::Upper: aLocalStart.setX(0.5); aLocalStart.setY(1.0); bOnAxis = true; break;
        case SdrHdlKind::UpperRight: aLocalStart.setX(0.0); aLocalStart.setY(1.0); break;
        case SdrHdlKind::Left : aLocalStart.setX(1.0); aLocalStart.setY(0.5); bOnAxis = true; break;
        case SdrHdlKind::Right: aLocalStart.setX(0.0); aLocalStart.setY(0.5); bOnAxis = true; break;
        case SdrHdlKind::LowerLeft: aLocalStart.setX(1.0); aLocalStart.setY(0.0); break;
        case SdrHdlKind::Lower: aLocalStart.setX(0.5); aLocalStart.setY(0.0); bOnAxis = true; break;
        case SdrHdlKind::LowerRight: aLocalStart.setX(0.0); aLocalStart.setY(0.0); break;
        default: break;
    }

    // create the current drag position in unit coordinates. To get there,
    // transform back the DragPoint to UnitCoordinates
    basegfx::B2DHomMatrix aInverse(aOriginalMatrix);
    aInverse.invert();
    basegfx::B2DPoint aLocalCurrent(aInverse * basegfx::B2DPoint(DragStat().GetNow().X(), DragStat().GetNow().Y()));

    // if one of the edge handles is used, limit to X or Y drag only
    if(bOnAxis)
    {
        if(basegfx::fTools::equal(aLocalStart.getX(), 0.5))
        {
            aLocalCurrent.setX(aLocalStart.getX());
        }
        else
        {
            aLocalCurrent.setY(aLocalStart.getY());
        }
    }

    // create internal change in unit coordinates
    basegfx::B2DHomMatrix aDiscreteChangeMatrix;

    if(!basegfx::fTools::equal(aLocalCurrent.getX(), aLocalStart.getX()))
    {
        if(aLocalStart.getX() < 0.5)
        {
            aDiscreteChangeMatrix.scale(aLocalCurrent.getX(), 1.0);
        }
        else
        {
            aDiscreteChangeMatrix.scale(1.0 - aLocalCurrent.getX(), 1.0);
            aDiscreteChangeMatrix.translate(aLocalCurrent.getX(), 0.0);
        }
    }

    if(!basegfx::fTools::equal(aLocalCurrent.getY(), aLocalStart.getY()))
    {
        if(aLocalStart.getY() < 0.5)
        {
            aDiscreteChangeMatrix.scale(1.0, aLocalCurrent.getY());
        }
        else
        {
            aDiscreteChangeMatrix.scale(1.0, 1.0 - aLocalCurrent.getY());
            aDiscreteChangeMatrix.translate(0.0, aLocalCurrent.getY());
        }
    }

    // We now have the whole executed Crop in UnitCoordinates in
    // aDiscreteChangeMatrix, go to concrete sizes now.
    // Create the unrotated original rectangle and the unrotated modified
    // rectangle as Ranges
    const basegfx::utils::B2DHomMatrixBufferedDecompose aOriginalMatrixDecomp(aOriginalMatrix);

    // prepare unsheared/unrotated versions of the old and new transformation
    const basegfx::B2DHomMatrix aOriginalMatrixNoShearNoRotate(
        basegfx::utils::createScaleTranslateB2DHomMatrix(
            basegfx::absolute(aOriginalMatrixDecomp.getScale()),
            aOriginalMatrixDecomp.getTranslate()));

    // create the ranges for these
    basegfx::B2DRange aRangeOriginalNoShearNoRotate(0.0, 0.0, 1.0, 1.0);
    basegfx::B2DRange aRangeNewNoShearNoRotate(0.0, 0.0, 1.0, 1.0);
    aRangeOriginalNoShearNoRotate.transform(aOriginalMatrixNoShearNoRotate);
    aRangeNewNoShearNoRotate.transform(aOriginalMatrixNoShearNoRotate * aDiscreteChangeMatrix);

    if(bExternal)
    {
        // With aLocalStart point (opposed to dragged point), X scale and Y scale,
        // we call crop (virtual method) on pSdrObject which calls VirtFlyDrawObj
        // crop. Use aLocalStart unchanged, so being relative to the Crop-Action,
        // the called instance knows best how to use it
        const double fScaleX(aRangeNewNoShearNoRotate.getWidth() / aRangeOriginalNoShearNoRotate.getWidth());
        const double fScaleY(aRangeNewNoShearNoRotate.getHeight() / aRangeOriginalNoShearNoRotate.getHeight());

        pExternalSdrObject->Crop(
            aLocalStart,
            fScaleX,
            fScaleY);
    }
    else
    {
        // prepare matrix to apply to object; evtl. back-correct shear
        basegfx::B2DHomMatrix aNewObjectMatrix(aOriginalMatrix * aDiscreteChangeMatrix);

        if(bShearCorrected)
        {
            // back-correct shear
            const basegfx::utils::B2DHomMatrixBufferedDecompose aTmpDecomp(aNewObjectMatrix);

            aNewObjectMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
                aTmpDecomp.getScale(),
                -aTmpDecomp.getShearX(),
                aTmpDecomp.getRotate(),
                aTmpDecomp.getTranslate());
        }

        // apply change to object by applying the unit coordinate change followed
        // by the original change
        pObj->TRSetBaseGeometry(aNewObjectMatrix, aPolyPolygon);

        // extract the old Rectangle structures
        tools::Rectangle aOldRect(
            basegfx::fround(aRangeOriginalNoShearNoRotate.getMinX()),
            basegfx::fround(aRangeOriginalNoShearNoRotate.getMinY()),
            basegfx::fround(aRangeOriginalNoShearNoRotate.getMaxX()),
            basegfx::fround(aRangeOriginalNoShearNoRotate.getMaxY()));
        tools::Rectangle aNewRect(
            basegfx::fround(aRangeNewNoShearNoRotate.getMinX()),
            basegfx::fround(aRangeNewNoShearNoRotate.getMinY()),
            basegfx::fround(aRangeNewNoShearNoRotate.getMaxX()),
            basegfx::fround(aRangeNewNoShearNoRotate.getMaxY()));

        // continue with the old original stuff
        if (!aOldRect.GetWidth() || !aOldRect.GetHeight())
        {
            throw o3tl::divide_by_zero();
        }

        if((pObj->GetGraphicType() == GraphicType::NONE) || (pObj->GetGraphicType() == GraphicType::Default))
        {
            return false;
        }

        const GraphicObject& rGraphicObject(pObj->GetGraphicObject());
        // tdf#117145 Usually Writer will go the bExternal path (see above), but more correct for
        // the future is to use the MapMode from the SdrModel/SfxItemPool if the Writer's current
        // special handling should be unified to this path in the future. Usually it *should* be
        // MapUnit::Map100thMM, but better do not mix up Units.
        // Checked now what SwVirtFlyDrawObj::NbcCrop is doing - it calculates everything forced
        // to MapUnit::Map100thMM, but extracts/packs Twips to the used SdrGrafCropItem in Writer.
        const MapMode aMapModePool(pObj->getSdrModelFromSdrObject().GetItemPool().GetMetric(0));
        Size aGraphicSize(rGraphicObject.GetPrefSize());

        if(MapUnit::MapPixel == rGraphicObject.GetPrefMapMode().GetMapUnit())
        {
            aGraphicSize = Application::GetDefaultDevice()->PixelToLogic(aGraphicSize, aMapModePool);
        }
        else
        {
            aGraphicSize = OutputDevice::LogicToLogic(aGraphicSize, rGraphicObject.GetPrefMapMode(), aMapModePool);
        }

        if(0 == aGraphicSize.Width() || 0 == aGraphicSize.Height())
        {
            return false;
        }

        const SdrGrafCropItem& rOldCrop = pObj->GetMergedItem(SDRATTR_GRAFCROP);
        double fScaleX = ( aGraphicSize.Width() - rOldCrop.GetLeft() - rOldCrop.GetRight() ) / static_cast<double>(aOldRect.GetWidth());
        double fScaleY = ( aGraphicSize.Height() - rOldCrop.GetTop() - rOldCrop.GetBottom() ) / static_cast<double>(aOldRect.GetHeight());

        sal_Int32 nDiffLeft = aNewRect.Left() - aOldRect.Left();
        sal_Int32 nDiffTop = aNewRect.Top() - aOldRect.Top();
        sal_Int32 nDiffRight = aNewRect.Right() - aOldRect.Right();
        sal_Int32 nDiffBottom = aNewRect.Bottom() - aOldRect.Bottom();

        if(pObj->IsMirrored())
        {
            // mirrored X or Y, for old stuff, exchange X
            // check for aw080
            sal_Int32 nTmp(nDiffLeft);
            nDiffLeft = -nDiffRight;
            nDiffRight = -nTmp;
        }

        sal_Int32 nLeftCrop = static_cast<sal_Int32>( rOldCrop.GetLeft() + nDiffLeft * fScaleX );
        sal_Int32 nTopCrop = static_cast<sal_Int32>( rOldCrop.GetTop() + nDiffTop * fScaleY );
        sal_Int32 nRightCrop = static_cast<sal_Int32>( rOldCrop.GetRight() - nDiffRight * fScaleX );
        sal_Int32 nBottomCrop = static_cast<sal_Int32>( rOldCrop.GetBottom() - nDiffBottom * fScaleY );

        SfxItemPool& rPool = getSdrDragView().GetModel()->GetItemPool();
        SfxItemSet aSet( rPool, svl::Items<SDRATTR_GRAFCROP, SDRATTR_GRAFCROP>{} );
        aSet.Put( SdrGrafCropItem( nLeftCrop, nTopCrop, nRightCrop, nBottomCrop ) );
        getSdrDragView().SetAttributes( aSet, false );
    }

    if(bUndo)
    {
        getSdrDragView().EndUndo();
    }

    return true;
}

PointerStyle SdrDragCrop::GetSdrDragPointer() const
{
    return PointerStyle::Crop;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
