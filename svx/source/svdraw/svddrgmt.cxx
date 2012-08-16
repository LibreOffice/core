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


#include "svddrgm1.hxx"
#include <math.h>

#ifndef _MATH_H
#define _MATH_H
#endif
#include <tools/bigint.hxx>
#include <vcl/svapp.hxx>

#include "svx/xattr.hxx"
#include <svx/xpoly.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpagv.hxx>
#include "svx/svdstr.hrc"   // names taken from the resource
#include "svx/svdglob.hxx"  // StringCache
#include <svx/svddrgv.hxx>
#include <svx/svdograf.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/sdgcpitm.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdr/overlay/overlayrollingrectangle.hxx>
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
#include "svx/svditer.hxx"
#include <svx/svdopath.hxx>
#include <svx/polypolygoneditor.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdovirt.hxx>
#include <svx/svdouno.hxx>
#include <svx/sdr/primitive2d/sdrprimitivetools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragEntry::SdrDragEntry()
:   mbAddToTransparent(false)
{
}

SdrDragEntry::~SdrDragEntry()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragEntryPolyPolygon::SdrDragEntryPolyPolygon(const basegfx::B2DPolyPolygon& rOriginalPolyPolygon)
:   SdrDragEntry(),
    maOriginalPolyPolygon(rOriginalPolyPolygon)
{
}

SdrDragEntryPolyPolygon::~SdrDragEntryPolyPolygon()
{
}

drawinglayer::primitive2d::Primitive2DSequence SdrDragEntryPolyPolygon::createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod)
{
    drawinglayer::primitive2d::Primitive2DSequence aRetval;

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

        drawinglayer::primitive2d::Primitive2DReference aPolyPolygonMarkerPrimitive2D(
            new drawinglayer::primitive2d::PolyPolygonMarkerPrimitive2D(aCopy, aColA, aColB, fStripeLength));

        aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aPolyPolygonMarkerPrimitive2D, 1);
    }

    return aRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragEntrySdrObject::SdrDragEntrySdrObject(const SdrObject& rOriginal, sdr::contact::ObjectContact& rObjectContact, bool bModify)
:   SdrDragEntry(),
    maOriginal(rOriginal),
    mpClone(0),
    mrObjectContact(rObjectContact),
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

drawinglayer::primitive2d::Primitive2DSequence SdrDragEntrySdrObject::createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod)
{
    // for the moment, i need to re-create the clone in all cases. I need to figure
    // out when clone and original have the same class, so that i can use operator=
    // in those cases

    const SdrObject* pSource = &maOriginal;

    if(mpClone)
    {
        SdrObject::Free(mpClone);
        mpClone = 0;
    }

    if(mbModify)
    {
        if(!mpClone)
        {
            mpClone = maOriginal.getFullDragClone();
        }

        // apply original transformation, implemented at the DragMethods
        rDragMethod.applyCurrentTransformationToSdrObject(*mpClone);

        // choose source for geometry data
        pSource = mpClone;
    }

    // get VOC and Primitive2DSequence
    sdr::contact::ViewContact& rVC = pSource->GetViewContact();
    sdr::contact::ViewObjectContact& rVOC = rVC.GetViewObjectContact(mrObjectContact);
    sdr::contact::DisplayInfo aDisplayInfo;

    // Do not use the last ViewPort set at the OC from the last ProcessDisplay(),
    // here we want the complete primitive sequence without visibility clippings
    mrObjectContact.resetViewPort();

    return rVOC.getPrimitive2DSequenceHierarchy(aDisplayInfo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragEntryPrimitive2DSequence::SdrDragEntryPrimitive2DSequence(
    const drawinglayer::primitive2d::Primitive2DSequence& rSequence,
    bool bAddToTransparent)
:   SdrDragEntry(),
    maPrimitive2DSequence(rSequence)
{
    // add parts to transparent overlay stuff if necessary
    setAddToTransparent(bAddToTransparent);
}

SdrDragEntryPrimitive2DSequence::~SdrDragEntryPrimitive2DSequence()
{
}

drawinglayer::primitive2d::Primitive2DSequence SdrDragEntryPrimitive2DSequence::createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod)
{
    drawinglayer::primitive2d::Primitive2DReference aTransformPrimitive2D(
        new drawinglayer::primitive2d::TransformPrimitive2D(
            rDragMethod.getCurrentTransformation(),
            maPrimitive2DSequence));

    return drawinglayer::primitive2d::Primitive2DSequence(&aTransformPrimitive2D, 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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

drawinglayer::primitive2d::Primitive2DSequence SdrDragEntryPointGlueDrag::createPrimitive2DSequenceInCurrentState(SdrDragMethod& rDragMethod)
{
    drawinglayer::primitive2d::Primitive2DSequence aRetval;

    if(!maPositions.empty())
    {
        basegfx::B2DPolygon aPolygon;
        sal_uInt32 a(0);

        for(a = 0; a < maPositions.size(); a++)
        {
            aPolygon.append(maPositions[a]);
        }

        basegfx::B2DPolyPolygon aPolyPolygon(aPolygon);

        rDragMethod.applyCurrentTransformationToPolyPolygon(aPolyPolygon);

        const basegfx::B2DPolygon aTransformed(aPolyPolygon.getB2DPolygon(0));
        std::vector< basegfx::B2DPoint > aTransformedPositions;

        aTransformedPositions.reserve(aTransformed.count());

        for(a = 0; a < aTransformed.count(); a++)
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

            aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aMarkerArrayPrimitive2D, 1);
        }
        else
        {
            drawinglayer::primitive2d::Primitive2DReference aMarkerArrayPrimitive2D(
                new drawinglayer::primitive2d::MarkerArrayPrimitive2D(aTransformedPositions,
                                                                      SdrHdl::createGluePointBitmap()));
            aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aMarkerArrayPrimitive2D, 1);
        }
    }

    return aRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT0(SdrDragMethod);

void SdrDragMethod::resetSdrDragEntries()
{
    // clear entries; creation is on demand
    clearSdrDragEntries();
}

basegfx::B2DRange SdrDragMethod::getCurrentRange() const
{
    return getB2DRangeFromOverlayObjectList();
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

void SdrDragMethod::createSdrDragEntryForSdrObject(const SdrObject& rOriginal, sdr::contact::ObjectContact& rObjectContact, bool bModify)
{
    // add full object drag; Clone() at the object has to work
    // for this
    addSdrDragEntry(new SdrDragEntrySdrObject(rOriginal, rObjectContact, bModify));
}

void SdrDragMethod::createSdrDragEntries_SolidDrag()
{
    const sal_uInt32 nMarkAnz(getSdrDragView().GetMarkedObjectCount());
    SdrPageView* pPV = getSdrDragView().GetSdrPageView();

    if(pPV)
    {
        for(sal_uInt32 a(0); a < nMarkAnz; a++)
        {
            SdrMark* pM = getSdrDragView().GetSdrMarkByIndex(a);

            if(pM->GetPageView() == pPV)
            {
                const SdrObject* pObject = pM->GetMarkedSdrObj();

                if(pObject)
                {
                    if(pPV->PageWindowCount())
                    {
                        sdr::contact::ObjectContact& rOC = pPV->GetPageWindow(0)->GetObjectContact();
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
                                    createSdrDragEntryForSdrObject(*pCandidate, rOC, true);
                                }

                                if(bAddWireframe)
                                {
                                    // when dragging a 50% transparent copy of a filled or not filled object without
                                    // outline, this is normally hard to see. Add extra wireframe in that case. This
                                    // works nice e.g. with text frames etc.
                                    addSdrDragEntry(new SdrDragEntryPolyPolygon(pCandidate->TakeXorPoly()));
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
    const sal_uInt32 nMarkAnz(getSdrDragView().GetMarkedObjectCount());
    bool bNoPolygons(getSdrDragView().IsNoDragXorPolys() || nMarkAnz > getSdrDragView().GetDragXorPolyLimit());
    basegfx::B2DPolyPolygon aResult;
    sal_uInt32 nPointCount(0);

    for(sal_uInt32 a(0); !bNoPolygons && a < nMarkAnz; a++)
    {
        SdrMark* pM = getSdrDragView().GetSdrMarkByIndex(a);

        if(pM->GetPageView() == getSdrDragView().GetSdrPageView())
        {
            const basegfx::B2DPolyPolygon aNewPolyPolygon(pM->GetMarkedSdrObj()->TakeXorPoly());

            for(sal_uInt32 b(0); b < aNewPolyPolygon.count(); b++)
            {
                nPointCount += aNewPolyPolygon.getB2DPolygon(b).count();
            }

            if(nPointCount > getSdrDragView().GetDragXorPointLimit())
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
        const Rectangle aR(getSdrDragView().GetSdrPageView()->MarkSnap());
        const basegfx::B2DRange aNewRectangle(aR.Left(), aR.Top(), aR.Right(), aR.Bottom());
        basegfx::B2DPolygon aNewPolygon(basegfx::tools::createPolygonFromRect(aNewRectangle));

        aResult = basegfx::B2DPolyPolygon(basegfx::tools::expandToCurve(aNewPolygon));
    }

    if(aResult.count())
    {
        addSdrDragEntry(new SdrDragEntryPolyPolygon(aResult));
    }
}

void SdrDragMethod::createSdrDragEntries_PointDrag()
{
    const sal_uInt32 nMarkAnz(getSdrDragView().GetMarkedObjectCount());
    std::vector< basegfx::B2DPoint > aPositions;

    for(sal_uInt32 nm(0); nm < nMarkAnz; nm++)
    {
        SdrMark* pM = getSdrDragView().GetSdrMarkByIndex(nm);

        if(pM->GetPageView() == getSdrDragView().GetSdrPageView())
        {
            const SdrUShortCont* pPts = pM->GetMarkedPoints();

            if(pPts && pPts->GetCount())
            {
                const SdrObject* pObj = pM->GetMarkedSdrObj();
                const SdrPathObj* pPath = dynamic_cast< const SdrPathObj* >(pObj);

                if(pPath)
                {
                    const basegfx::B2DPolyPolygon aPathXPP = pPath->GetPathPoly();

                    if(aPathXPP.count())
                    {
                        const sal_uInt32 nPtAnz(pPts->GetCount());

                        for(sal_uInt32 nPtNum(0); nPtNum < nPtAnz; nPtNum++)
                        {
                            sal_uInt32 nPolyNum, nPointNum;
                            const sal_uInt16 nObjPt(pPts->GetObject(nPtNum));

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
        addSdrDragEntry(new SdrDragEntryPointGlueDrag(aPositions, true));
    }
}

void SdrDragMethod::createSdrDragEntries_GlueDrag()
{
    const sal_uInt32 nMarkAnz(getSdrDragView().GetMarkedObjectCount());
    std::vector< basegfx::B2DPoint > aPositions;

    for(sal_uInt32 nm(0); nm < nMarkAnz; nm++)
    {
        SdrMark* pM = getSdrDragView().GetSdrMarkByIndex(nm);

        if(pM->GetPageView() == getSdrDragView().GetSdrPageView())
        {
            const SdrUShortCont* pPts = pM->GetMarkedGluePoints();

            if(pPts && pPts->GetCount())
            {
                const SdrObject* pObj = pM->GetMarkedSdrObj();
                const SdrGluePointList* pGPL = pObj->GetGluePointList();

                if(pGPL)
                {
                    const sal_uInt32 nPtAnz(pPts->GetCount());

                    for(sal_uInt32 nPtNum(0); nPtNum < nPtAnz; nPtNum++)
                    {
                        const sal_uInt16 nObjPt(pPts->GetObject(nPtNum));
                        const sal_uInt16 nGlueNum(pGPL->FindGluePoint(nObjPt));

                        if(SDRGLUEPOINT_NOTFOUND != nGlueNum)
                        {
                            const Point aPoint((*pGPL)[nGlueNum].GetAbsolutePos(*pObj));
                            aPositions.push_back(basegfx::B2DPoint(aPoint.X(), aPoint.Y()));
                        }
                    }
                }
            }
        }
    }

    if(!aPositions.empty())
    {
        addSdrDragEntry(new SdrDragEntryPointGlueDrag(aPositions, false));
    }
}

void SdrDragMethod::ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, XubString& rStr, sal_uInt16 nVal) const
{
    sal_uInt16 nOpt=0;
    if (IsDraggingPoints()) {
        nOpt=IMPSDR_POINTSDESCRIPTION;
    } else if (IsDraggingGluePoints()) {
        nOpt=IMPSDR_GLUEPOINTSDESCRIPTION;
    }
    getSdrDragView().ImpTakeDescriptionStr(nStrCacheID,rStr,nVal,nOpt);
}

SdrObject* SdrDragMethod::GetDragObj() const
{
    SdrObject* pObj=NULL;
    if (getSdrDragView().pDragHdl!=NULL) pObj=getSdrDragView().pDragHdl->GetObj();
    if (pObj==NULL) pObj=getSdrDragView().pMarkedObj;
    return pObj;
}

SdrPageView* SdrDragMethod::GetDragPV() const
{
    SdrPageView* pPV=NULL;
    if (getSdrDragView().pDragHdl!=NULL) pPV=getSdrDragView().pDragHdl->GetPageView();
    if (pPV==NULL) pPV=getSdrDragView().pMarkedPV;
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
        basegfx::B2DTuple aScale;
        basegfx::B2DTuple aTranslate;
        double fRotate, fShearX;
        aObjectTransform.decompose(aScale, aTranslate, fRotate, fShearX);

        // get polygon's position and size
        const basegfx::B2DRange aPolyRange(aObjectPolyPolygon.getB2DRange());

        // get the scaling factors (do not mirror, this is in the object transformation)
        const double fScaleX(fabs(aScale.getX()) / (basegfx::fTools::equalZero(aPolyRange.getWidth()) ? 1.0 : aPolyRange.getWidth()));
        const double fScaleY(fabs(aScale.getY()) / (basegfx::fTools::equalZero(aPolyRange.getHeight()) ? 1.0 : aPolyRange.getHeight()));

        // prepare transform matrix for polygon
        basegfx::B2DHomMatrix aPolyTransform(basegfx::tools::createTranslateB2DHomMatrix(
            -aPolyRange.getMinX(), -aPolyRange.getMinY()));
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
    mbSolidDraggingActive(getSdrDragView().IsSolidDragging())
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

void SdrDragMethod::CreateOverlayGeometry(sdr::overlay::OverlayManager& rOverlayManager)
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
        drawinglayer::primitive2d::Primitive2DSequence aResult;
        drawinglayer::primitive2d::Primitive2DSequence aResultTransparent;

        for(sal_uInt32 a(0); a < maSdrDragEntries.size(); a++)
        {
            SdrDragEntry* pCandidate = maSdrDragEntries[a];

            if(pCandidate)
            {
                const drawinglayer::primitive2d::Primitive2DSequence aCandidateResult(pCandidate->createPrimitive2DSequenceInCurrentState(*this));

                if(aCandidateResult.hasElements())
                {
                    if(pCandidate->getAddToTransparent())
                    {
                        drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(aResultTransparent, aCandidateResult);
                    }
                    else
                    {
                        drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(aResult, aCandidateResult);
                    }
                }
            }
        }

        if(DoAddConnectorOverlays())
        {
            const drawinglayer::primitive2d::Primitive2DSequence aConnectorOverlays(AddConnectorOverlays());

            if(aConnectorOverlays.hasElements())
            {
                // add connector overlays to transparent part
                drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(aResultTransparent, aConnectorOverlays);
            }
        }

        if(aResult.hasElements())
        {
            sdr::overlay::OverlayObject* pNewOverlayObject = new sdr::overlay::OverlayPrimitive2DSequenceObject(aResult);
            rOverlayManager.add(*pNewOverlayObject);
            addToOverlayObjectList(*pNewOverlayObject);
        }

        if(aResultTransparent.hasElements())
        {
            drawinglayer::primitive2d::Primitive2DReference aUnifiedTransparencePrimitive2D(new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(aResultTransparent, 0.5));
            aResultTransparent = drawinglayer::primitive2d::Primitive2DSequence(&aUnifiedTransparencePrimitive2D, 1);

            sdr::overlay::OverlayObject* pNewOverlayObject = new sdr::overlay::OverlayPrimitive2DSequenceObject(aResultTransparent);
            rOverlayManager.add(*pNewOverlayObject);
            addToOverlayObjectList(*pNewOverlayObject);
        }
    }

    // add DragStripes if necessary (help lines cross the page when dragging)
    if(getSdrDragView().IsDragStripes())
    {
        Rectangle aActionRectangle;
        getSdrDragView().TakeActionRect(aActionRectangle);

        const basegfx::B2DPoint aTopLeft(aActionRectangle.Left(), aActionRectangle.Top());
        const basegfx::B2DPoint aBottomRight(aActionRectangle.Right(), aActionRectangle.Bottom());
        sdr::overlay::OverlayRollingRectangleStriped* pNew = new sdr::overlay::OverlayRollingRectangleStriped(
            aTopLeft, aBottomRight, true, false);

        rOverlayManager.add(*pNew);
        addToOverlayObjectList(*pNew);
    }
}

void SdrDragMethod::destroyOverlayGeometry()
{
    clearOverlayObjectList();
}

bool SdrDragMethod::DoAddConnectorOverlays()
{
    // these conditions are translated from SdrDragView::ImpDrawEdgeXor
    const SdrMarkList& rMarkedNodes = getSdrDragView().GetEdgesOfMarkedNodes();

    if(!rMarkedNodes.GetMarkCount())
    {
        return false;
    }

    if(!getSdrDragView().IsRubberEdgeDragging() && !getSdrDragView().IsDetailedEdgeDragging())
    {
        return false;
    }

    if(getSdrDragView().IsDraggingPoints() || getSdrDragView().IsDraggingGluePoints())
    {
        return false;
    }

    if(!getMoveOnly() && !(
        IS_TYPE(SdrDragMove, this) || IS_TYPE(SdrDragResize, this) ||
        IS_TYPE(SdrDragRotate,this) || IS_TYPE(SdrDragMirror,this)))
    {
        return false;
    }

    const bool bDetail(getSdrDragView().IsDetailedEdgeDragging() && getMoveOnly());

    if(!bDetail && !getSdrDragView().IsRubberEdgeDragging())
    {
        return false;
    }

    // one more migrated from SdrEdgeObj::NspToggleEdgeXor
    if(IS_TYPE(SdrDragObjOwn, this) || IS_TYPE(SdrDragMovHdl, this))
    {
        return false;
    }

    return true;
}

drawinglayer::primitive2d::Primitive2DSequence SdrDragMethod::AddConnectorOverlays()
{
    drawinglayer::primitive2d::Primitive2DSequence aRetval;
    const bool bDetail(getSdrDragView().IsDetailedEdgeDragging() && getMoveOnly());
    const SdrMarkList& rMarkedNodes = getSdrDragView().GetEdgesOfMarkedNodes();

    for(sal_uInt16 a(0); a < rMarkedNodes.GetMarkCount(); a++)
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

                            drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(
                                aRetval, drawinglayer::primitive2d::createPolygonLinePrimitive(
                                    aEdgePolygon,
                                    basegfx::B2DHomMatrix(),
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
                            new drawinglayer::primitive2d::PolyPolygonMarkerPrimitive2D(
                                basegfx::B2DPolyPolygon(aEdgePolygon), aColA, aColB, fStripeLength));
                        drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aPolyPolygonMarkerPrimitive2D);
                    }
                }
            }
        }
    }

    return aRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragMovHdl,SdrDragMethod);

SdrDragMovHdl::SdrDragMovHdl(SdrDragView& rNewView)
:   SdrDragMethod(rNewView)
{
}

void SdrDragMovHdl::createSdrDragEntries()
{
    // SdrDragMovHdl does not use the default drags,
    // but creates nothing
}

void SdrDragMovHdl::TakeSdrDragComment(XubString& rStr) const
{
    rStr=ImpGetResStr(STR_DragMethMovHdl);
    if (getSdrDragView().IsDragWithCopy()) rStr+=ImpGetResStr(STR_EditWithCopy);
}

bool SdrDragMovHdl::BeginSdrDrag()
{
    if( !GetDragHdl() )
        return false;

    DragStat().Ref1()=GetDragHdl()->GetPos();
    DragStat().SetShown(!DragStat().IsShown());
    SdrHdlKind eKind=GetDragHdl()->GetKind();
    SdrHdl* pH1=GetHdlList().GetHdl(HDL_REF1);
    SdrHdl* pH2=GetHdlList().GetHdl(HDL_REF2);

    if (eKind==HDL_MIRX)
    {
        if (pH1==NULL || pH2==NULL)
        {
            OSL_FAIL("SdrDragMovHdl::BeginSdrDrag(): Moving the axis of reflection: reference handles not found.");
            return false;
        }

        DragStat().SetActionRect(Rectangle(pH1->GetPos(),pH2->GetPos()));
    }
    else
    {
        Point aPt(GetDragHdl()->GetPos());
        DragStat().SetActionRect(Rectangle(aPt,aPt));
    }

    return true;
}

void SdrDragMovHdl::MoveSdrDrag(const Point& rNoSnapPnt)
{
    Point aPnt(rNoSnapPnt);

    if ( GetDragHdl() && DragStat().CheckMinMoved(rNoSnapPnt))
    {
        if (GetDragHdl()->GetKind()==HDL_MIRX)
        {
            SdrHdl* pH1=GetHdlList().GetHdl(HDL_REF1);
            SdrHdl* pH2=GetHdlList().GetHdl(HDL_REF2);

            if (pH1==NULL || pH2==NULL)
                return;

            if (!DragStat().IsNoSnap())
            {
                long nBestXSnap=0;
                long nBestYSnap=0;
                bool bXSnapped=false;
                bool bYSnapped=false;
                Point aDif(aPnt-DragStat().GetStart());
                getSdrDragView().CheckSnap(Ref1()+aDif,NULL,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
                getSdrDragView().CheckSnap(Ref2()+aDif,NULL,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
                aPnt.X()+=nBestXSnap;
                aPnt.Y()+=nBestYSnap;
            }

            if (aPnt!=DragStat().GetNow())
            {
                Hide();
                DragStat().NextMove(aPnt);
                Point aDif(DragStat().GetNow()-DragStat().GetStart());
                pH1->SetPos(Ref1()+aDif);
                pH2->SetPos(Ref2()+aDif);

                SdrHdl* pHM = GetHdlList().GetHdl(HDL_MIRX);

                if(pHM)
                    pHM->Touch();

                Show();
                DragStat().SetActionRect(Rectangle(pH1->GetPos(),pH2->GetPos()));
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
                if (!getSdrDragView().IsMirrorAllowed(false,false)) nSA=4500;
                if (!getSdrDragView().IsMirrorAllowed(true,false)) nSA=9000;
            }

            if (getSdrDragView().IsOrtho() && nSA!=9000)
                nSA=4500;

            if (nSA!=0)
            { // angle snapping
                SdrHdlKind eRef=HDL_REF1;

                if (GetDragHdl()->GetKind()==HDL_REF1)
                    eRef=HDL_REF2;

                SdrHdl* pH=GetHdlList().GetHdl(eRef);

                if (pH!=NULL)
                {
                    Point aRef(pH->GetPos());
                    long nWink=NormAngle360(GetAngle(aPnt-aRef));
                    long nNeuWink=nWink;
                    nNeuWink+=nSA/2;
                    nNeuWink/=nSA;
                    nNeuWink*=nSA;
                    nNeuWink=NormAngle360(nNeuWink);
                    double a=(nNeuWink-nWink)*nPi180;
                    double nSin=sin(a);
                    double nCos=cos(a);
                    RotatePoint(aPnt,aRef,nSin,nCos);

                    // eliminate rounding errors for certain values
                    if (nSA==9000)
                    {
                        if (nNeuWink==0    || nNeuWink==18000) aPnt.Y()=aRef.Y();
                        if (nNeuWink==9000 || nNeuWink==27000) aPnt.X()=aRef.X();
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
                SdrHdl* pHM = GetHdlList().GetHdl(HDL_MIRX);

                if(pHM)
                    pHM->Touch();

                Show();
                DragStat().SetActionRect(Rectangle(aPnt,aPnt));
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
            case HDL_REF1:
                Ref1()=DragStat().GetNow();
                break;

            case HDL_REF2:
                Ref2()=DragStat().GetNow();
                break;

            case HDL_MIRX:
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

    SdrHdl* pHM = GetHdlList().GetHdl(HDL_MIRX);

    if(pHM)
        pHM->Touch();
}

Pointer SdrDragMovHdl::GetSdrDragPointer() const
{
    const SdrHdl* pHdl = GetDragHdl();

    if (pHdl!=NULL)
    {
        return pHdl->GetPointer();
    }

    return Pointer(POINTER_REFHAND);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragObjOwn,SdrDragMethod);

SdrDragObjOwn::SdrDragObjOwn(SdrDragView& rNewView)
:   SdrDragMethod(rNewView),
    mpClone(0)
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
                sdr::contact::ObjectContact& rOC = pPV->GetPageWindow(0)->GetObjectContact();
                addSdrDragEntry(new SdrDragEntrySdrObject(*mpClone, rOC, false));

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
            addSdrDragEntry(new SdrDragEntryPolyPolygon(aDragPolyPolygon));
        }
    }
}

void SdrDragObjOwn::TakeSdrDragComment(XubString& rStr) const
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

    if(pObj)
    {
        Point aPnt(rNoSnapPnt);
        SdrPageView* pPV = GetDragPV();

        if(pPV)
        {
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

            if(DragStat().CheckMinMoved(rNoSnapPnt))
            {
                if(aPnt != DragStat().GetNow())
                {
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
                        mpClone = 0;
                    }

                    // create a new clone and modify to current drag state
                    if(!mpClone)
                    {
                        mpClone = pObj->getFullDragClone();
                        mpClone->applySpecialDrag(DragStat());
                    }

                    Show();
                }
            }
        }
    }
}

bool SdrDragObjOwn::EndSdrDrag(bool /*bCopy*/)
{
    Hide();
    SdrUndoAction* pUndo = NULL;
    SdrUndoAction* pUndo2 = NULL;
    std::vector< SdrUndoAction* > vConnectorUndoActions;
    bool bRet = false;
    SdrObject* pObj = GetDragObj();

    if(pObj)
    {
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
        Rectangle aBoundRect0;

        if(pObj->GetUserCall())
        {
            aBoundRect0 = pObj->GetLastBoundRect();
        }

        bRet = pObj->applySpecialDrag(DragStat());

        if(bRet)
        {
            pObj->SetChanged();
            pObj->BroadcastObjectChange();
            pObj->SendUserCall( SDRUSERCALL_RESIZE, aBoundRect0 );
        }

        if(bRet)
        {
            if( bUndo )
            {
                getSdrDragView().AddUndoActions( vConnectorUndoActions );

                if ( pUndo )
                {
                    getSdrDragView().AddUndo(pUndo);
                }

                if ( pUndo2 )
                {
                    getSdrDragView().AddUndo(pUndo2);
                }
            }
        }
        else
        {
            if( bUndo )
            {
                std::vector< SdrUndoAction* >::iterator vConnectorUndoIter( vConnectorUndoActions.begin() );

                while( vConnectorUndoIter != vConnectorUndoActions.end() )
                {
                    delete *vConnectorUndoIter++;
                }

                delete pUndo;
                delete pUndo2;
            }
        }

        if( bUndo )
            getSdrDragView().EndUndo();
    }

    return bRet;
}

Pointer SdrDragObjOwn::GetSdrDragPointer() const
{
    const SdrHdl* pHdl=GetDragHdl();

    if (pHdl)
    {
        return pHdl->GetPointer();
    }

    return Pointer(POINTER_MOVE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragMove,SdrDragMethod);

void SdrDragMove::createSdrDragEntryForSdrObject(const SdrObject& rOriginal, sdr::contact::ObjectContact& rObjectContact, bool /*bModify*/)
{
    // for SdrDragMove, use current Primitive2DSequence of SdrObject visualization
    // in given ObjectContact directly
    sdr::contact::ViewContact& rVC = rOriginal.GetViewContact();
    sdr::contact::ViewObjectContact& rVOC = rVC.GetViewObjectContact(rObjectContact);
    sdr::contact::DisplayInfo aDisplayInfo;

    // Do not use the last ViewPort set at the OC from the last ProcessDisplay(),
    // here we want the complete primitive sequence without visible clippings
    rObjectContact.resetViewPort();

    addSdrDragEntry(new SdrDragEntryPrimitive2DSequence(rVOC.getPrimitive2DSequenceHierarchy(aDisplayInfo), true));
}

void SdrDragMove::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    rTarget.Move(Size(DragStat().GetDX(), DragStat().GetDY()));
}

SdrDragMove::SdrDragMove(SdrDragView& rNewView)
:   SdrDragMethod(rNewView)
{
    setMoveOnly(true);
}

void SdrDragMove::TakeSdrDragComment(XubString& rStr) const
{
    rtl::OUString aStr;

    ImpTakeDescriptionStr(STR_DragMethMove, rStr);
    rStr.AppendAscii(" (x=");
    getSdrDragView().GetModel()->TakeMetricStr(DragStat().GetDX(), aStr);
    rStr.Append(aStr);
    rStr.AppendAscii(" y=");
    getSdrDragView().GetModel()->TakeMetricStr(DragStat().GetDY(), aStr);
    rStr.Append(aStr);
    rStr += sal_Unicode(')');

    if(getSdrDragView().IsDragWithCopy())
    {
        if(!getSdrDragView().IsInsObjPoint() && !getSdrDragView().IsInsGluePoint())
        {
            rStr += ImpGetResStr(STR_EditWithCopy);
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
    return basegfx::tools::createTranslateB2DHomMatrix(DragStat().GetDX(), DragStat().GetDY());
}

void SdrDragMove::ImpCheckSnap(const Point& rPt)
{
    Point aPt(rPt);
    sal_uInt16 nRet=SnapPos(aPt);
    aPt-=rPt;

    if ((nRet & SDRSNAP_XSNAPPED) !=0)
    {
        if (bXSnapped)
        {
            if (Abs(aPt.X())<Abs(nBestXSnap))
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

    if ((nRet & SDRSNAP_YSNAPPED) !=0)
    {
        if (bYSnapped)
        {
            if (Abs(aPt.Y())<Abs(nBestYSnap))
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
    const Rectangle& aSR=GetMarkedRect();
    long nMovedx=aNoSnapPnt.X()-DragStat().GetStart().X();
    long nMovedy=aNoSnapPnt.Y()-DragStat().GetStart().Y();
    Point aLO(aSR.TopLeft());      aLO.X()+=nMovedx; aLO.Y()+=nMovedy;
    Point aRU(aSR.BottomRight());  aRU.X()+=nMovedx; aRU.Y()+=nMovedy;
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
        Rectangle aLR(getSdrDragView().GetWorkArea());
        bool bWorkArea=!aLR.IsEmpty();
        bool bDragLimit=IsDragLimit();

        if (bDragLimit || bWorkArea)
        {
            Rectangle aSR2(GetMarkedRect());
            Point aD(aPt1-DragStat().GetStart());

            if (bDragLimit)
            {
                Rectangle aR2(GetDragLimitRect());

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
                    aPt1.X()-=aSR2.Left()-aLR.Left();
                }
                else if (aSR2.Right()>aLR.Right())
                {
                    aPt1.X()-=aSR2.Right()-aLR.Right();
                }
            }
            else
                aPt1.X()=DragStat().GetStart().X(); // no space to move to

            if (aSR2.Top()>aLR.Top() || aSR2.Bottom()<aLR.Bottom())
            { // any space to move to?
                aSR2.Move(0,aD.Y());

                if (aSR2.Top()<aLR.Top())
                {
                    aPt1.Y()-=aSR2.Top()-aLR.Top();
                }
                else if (aSR2.Bottom()>aLR.Bottom())
                {
                    aPt1.Y()-=aSR2.Bottom()-aLR.Bottom();
                }
            }
            else
                aPt1.Y()=DragStat().GetStart().Y(); // no space to move to
        }

        if (getSdrDragView().IsDraggingGluePoints())
        { // restrict glue points to the BoundRect of the Obj
            aPt1-=DragStat().GetStart();
            const SdrMarkList& rML=GetMarkedObjectList();
            sal_uLong nMarkAnz=rML.GetMarkCount();

            for (sal_uLong nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++)
            {
                const SdrMark* pM=rML.GetMark(nMarkNum);
                const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
                sal_uLong nPtAnz=pPts==NULL ? 0 : pPts->GetCount();

                if (nPtAnz!=0)
                {
                    const SdrObject* pObj=pM->GetMarkedSdrObj();
                    const SdrGluePointList* pGPL=pObj->GetGluePointList();
                    Rectangle aBound(pObj->GetCurrentBoundRect());

                    for (sal_uLong nPtNum=0; nPtNum<nPtAnz; nPtNum++)
                    {
                        sal_uInt16 nId=pPts->GetObject(nPtNum);
                        sal_uInt16 nGlueNum=pGPL->FindGluePoint(nId);

                        if (nGlueNum!=SDRGLUEPOINT_NOTFOUND)
                        {
                            Point aPt((*pGPL)[nGlueNum].GetAbsolutePos(*pObj));
                            aPt+=aPt1; // move by this much
                            if (aPt.X()<aBound.Left()  ) aPt1.X()-=aPt.X()-aBound.Left()  ;
                            if (aPt.X()>aBound.Right() ) aPt1.X()-=aPt.X()-aBound.Right() ;
                            if (aPt.Y()<aBound.Top()   ) aPt1.Y()-=aPt.Y()-aBound.Top()   ;
                            if (aPt.Y()>aBound.Bottom()) aPt1.Y()-=aPt.Y()-aBound.Bottom();
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
            Rectangle aAction(GetMarkedRect());
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
        getSdrDragView().MoveMarkedPoints(Size(DragStat().GetDX(),DragStat().GetDY()),bCopy);
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

Pointer SdrDragMove::GetSdrDragPointer() const
{
    if (IsDraggingPoints() || IsDraggingGluePoints())
    {
        return Pointer(POINTER_MOVEPOINT);
    }
    else
    {
        return Pointer(POINTER_MOVE);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragResize,SdrDragMethod);

SdrDragResize::SdrDragResize(SdrDragView& rNewView)
:   SdrDragMethod(rNewView),
    aXFact(1,1),
    aYFact(1,1)
{
}

void SdrDragResize::TakeSdrDragComment(XubString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethResize, rStr);
    bool bEqual(aXFact == aYFact);
    Fraction aFact1(1,1);
    Point aStart(DragStat().GetStart());
    Point aRef(DragStat().GetRef1());
    sal_Int32 nXDiv(aStart.X() - aRef.X());

    if(!nXDiv)
        nXDiv = 1;

    sal_Int32 nYDiv(aStart.Y() - aRef.Y());

    if(!nYDiv)
        nYDiv = 1;

    bool bX(aXFact != aFact1 && Abs(nXDiv) > 1);
    bool bY(aYFact != aFact1 && Abs(nYDiv) > 1);

    if(bX || bY)
    {
        XubString aStr;

        rStr.AppendAscii(" (");

        if(bX)
        {
            if(!bEqual)
                rStr.AppendAscii("x=");

            getSdrDragView().GetModel()->TakePercentStr(aXFact, aStr);
            rStr += aStr;
        }

        if(bY && !bEqual)
        {
            if(bX)
                rStr += sal_Unicode(' ');

            rStr.AppendAscii("y=");
            getSdrDragView().GetModel()->TakePercentStr(aYFact, aStr);
            rStr += aStr;
        }

        rStr += sal_Unicode(')');
    }

    if(getSdrDragView().IsDragWithCopy())
        rStr += ImpGetResStr(STR_EditWithCopy);
}

bool SdrDragResize::BeginSdrDrag()
{
    SdrHdlKind eRefHdl=HDL_MOVE;
    SdrHdl* pRefHdl=NULL;

    switch (GetDragHdlKind())
    {
        case HDL_UPLFT: eRefHdl=HDL_LWRGT; break;
        case HDL_UPPER: eRefHdl=HDL_LOWER; DragStat().SetHorFixed(true); break;
        case HDL_UPRGT: eRefHdl=HDL_LWLFT; break;
        case HDL_LEFT : eRefHdl=HDL_RIGHT; DragStat().SetVerFixed(true); break;
        case HDL_RIGHT: eRefHdl=HDL_LEFT ; DragStat().SetVerFixed(true); break;
        case HDL_LWLFT: eRefHdl=HDL_UPRGT; break;
        case HDL_LOWER: eRefHdl=HDL_UPPER; DragStat().SetHorFixed(true); break;
        case HDL_LWRGT: eRefHdl=HDL_UPLFT; break;
        default: break;
    }

    if (eRefHdl!=HDL_MOVE)
        pRefHdl=GetHdlList().GetHdl(eRefHdl);

    if (pRefHdl!=NULL && !getSdrDragView().IsResizeAtCenter())
    {
        DragStat().Ref1()=pRefHdl->GetPos();
    }
    else
    {
        SdrHdl* pRef1=GetHdlList().GetHdl(HDL_UPLFT);
        SdrHdl* pRef2=GetHdlList().GetHdl(HDL_LWRGT);

        if (pRef1!=NULL && pRef2!=NULL)
        {
            DragStat().Ref1()=Rectangle(pRef1->GetPos(),pRef2->GetPos()).Center();
        }
        else
        {
            DragStat().Ref1()=GetMarkedRect().Center();
        }
    }

    Show();

    return true;
}

basegfx::B2DHomMatrix SdrDragResize::getCurrentTransformation()
{
    basegfx::B2DHomMatrix aRetval(basegfx::tools::createTranslateB2DHomMatrix(
        -DragStat().Ref1().X(), -DragStat().Ref1().Y()));
    aRetval.scale(aXFact, aYFact);
    aRetval.translate(DragStat().Ref1().X(), DragStat().Ref1().Y());

    return aRetval;
}

void SdrDragResize::MoveSdrDrag(const Point& rNoSnapPnt)
{
    Point aPnt(GetSnapPos(rNoSnapPnt));
    Point aStart(DragStat().GetStart());
    Point aRef(DragStat().GetRef1());
    Fraction aMaxFact(0x7FFFFFFF,1);
    Rectangle aLR(getSdrDragView().GetWorkArea());
    bool bWorkArea=!aLR.IsEmpty();
    bool bDragLimit=IsDragLimit();

    if (bDragLimit || bWorkArea)
    {
        Rectangle aSR(GetMarkedRect());

        if (bDragLimit)
        {
            Rectangle aR2(GetDragLimitRect());

            if (bWorkArea)
                aLR.Intersection(aR2);
            else
                aLR=aR2;
        }

        if (aPnt.X()<aLR.Left())
            aPnt.X()=aLR.Left();
        else if (aPnt.X()>aLR.Right())
            aPnt.X()=aLR.Right();

        if (aPnt.Y()<aLR.Top())
            aPnt.Y()=aLR.Top();
        else if (aPnt.Y()>aLR.Bottom())
            aPnt.Y()=aLR.Bottom();

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
    bool bOrtho=getSdrDragView().IsOrtho() || !getSdrDragView().IsResizeAllowed(false);

    if (!DragStat().IsHorFixed() && !DragStat().IsVerFixed())
    {
        if (Abs(nXDiv)<=1 || Abs(nYDiv)<=1)
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

    Fraction aNeuXFact(nXMul,nXDiv);
    Fraction aNeuYFact(nYMul,nYDiv);

    if (bOrtho)
    {
        if (aNeuXFact>aMaxFact)
        {
            aNeuXFact=aMaxFact;
            aNeuYFact=aMaxFact;
        }

        if (aNeuYFact>aMaxFact)
        {
            aNeuXFact=aMaxFact;
            aNeuYFact=aMaxFact;
        }
    }

    if (bXNeg)
        aNeuXFact=Fraction(-aNeuXFact.GetNumerator(),aNeuXFact.GetDenominator());

    if (bYNeg)
        aNeuYFact=Fraction(-aNeuYFact.GetNumerator(),aNeuYFact.GetDenominator());

    if (DragStat().CheckMinMoved(aPnt))
    {
        if ((!DragStat().IsHorFixed() && aPnt.X()!=DragStat().GetNow().X()) ||
            (!DragStat().IsVerFixed() && aPnt.Y()!=DragStat().GetNow().Y()))
        {
            Hide();
            DragStat().NextMove(aPnt);
            aXFact=aNeuXFact;
            aYFact=aNeuYFact;
            Show();
        }
    }
}

void SdrDragResize::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    rTarget.Resize(DragStat().Ref1(),aXFact,aYFact);
}

bool SdrDragResize::EndSdrDrag(bool bCopy)
{
    Hide();

    if (IsDraggingPoints())
    {
        getSdrDragView().ResizeMarkedPoints(DragStat().Ref1(),aXFact,aYFact,bCopy);
    }
    else if (IsDraggingGluePoints())
    {
        getSdrDragView().ResizeMarkedGluePoints(DragStat().Ref1(),aXFact,aYFact,bCopy);
    }
    else
    {
        getSdrDragView().ResizeMarkedObj(DragStat().Ref1(),aXFact,aYFact,bCopy);
    }

    return true;
}

Pointer SdrDragResize::GetSdrDragPointer() const
{
    const SdrHdl* pHdl=GetDragHdl();

    if (pHdl!=NULL)
    {
        return pHdl->GetPointer();
    }

    return Pointer(POINTER_MOVE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragRotate,SdrDragMethod);

void SdrDragRotate::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    rTarget.Rotate(DragStat().GetRef1(), nWink, sin(nWink*nPi180), cos(nWink*nPi180));
}

SdrDragRotate::SdrDragRotate(SdrDragView& rNewView)
:   SdrDragMethod(rNewView),
    nSin(0.0),
    nCos(1.0),
    nWink0(0),
    nWink(0),
    bRight(false)
{
}

void SdrDragRotate::TakeSdrDragComment(XubString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethRotate, rStr);
    rStr.AppendAscii(" (");
    sal_Int32 nTmpWink(NormAngle360(nWink));

    if(bRight && nWink)
    {
        nTmpWink -= 36000;
    }

    rtl::OUString aStr;
    getSdrDragView().GetModel()->TakeWinkStr(nTmpWink, aStr);
    rStr.Append(aStr);
    rStr += sal_Unicode(')');

    if(getSdrDragView().IsDragWithCopy())
        rStr += ImpGetResStr(STR_EditWithCopy);
}

bool SdrDragRotate::BeginSdrDrag()
{
    SdrHdl* pH=GetHdlList().GetHdl(HDL_REF1);

    if (pH!=NULL)
    {
        Show();
        DragStat().Ref1()=pH->GetPos();
        nWink0=GetAngle(DragStat().GetStart()-DragStat().GetRef1());
        return true;
    }
    else
    {
        OSL_FAIL("SdrDragRotate::BeginSdrDrag(): No reference point handle found.");
        return false;
    }
}

basegfx::B2DHomMatrix SdrDragRotate::getCurrentTransformation()
{
    return basegfx::tools::createRotateAroundPoint(
        DragStat().GetRef1().X(), DragStat().GetRef1().Y(),
        -atan2(nSin, nCos));
}

void SdrDragRotate::MoveSdrDrag(const Point& rPnt_)
{
    Point aPnt(rPnt_);
    if (DragStat().CheckMinMoved(aPnt))
    {
        long nNeuWink=NormAngle360(GetAngle(aPnt-DragStat().GetRef1())-nWink0);
        long nSA=0;

        if (getSdrDragView().IsAngleSnapEnabled())
            nSA=getSdrDragView().GetSnapAngle();

        if (!getSdrDragView().IsRotateAllowed(false))
            nSA=9000;

        if (nSA!=0)
        { // angle snapping
            nNeuWink+=nSA/2;
            nNeuWink/=nSA;
            nNeuWink*=nSA;
        }

        nNeuWink=NormAngle180(nNeuWink);

        if (nWink!=nNeuWink)
        {
            sal_uInt16 nSekt0=GetAngleSector(nWink);
            sal_uInt16 nSekt1=GetAngleSector(nNeuWink);

            if (nSekt0==0 && nSekt1==3)
                bRight=true;

            if (nSekt0==3 && nSekt1==0)
                bRight=false;

            nWink=nNeuWink;
            double a=nWink*nPi180;
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

    if (nWink!=0)
    {
        if (IsDraggingPoints())
        {
            getSdrDragView().RotateMarkedPoints(DragStat().GetRef1(),nWink,bCopy);
        }
        else if (IsDraggingGluePoints())
        {
            getSdrDragView().RotateMarkedGluePoints(DragStat().GetRef1(),nWink,bCopy);
        }
        else
        {
            getSdrDragView().RotateMarkedObj(DragStat().GetRef1(),nWink,bCopy);
        }
    }
    return true;
}

Pointer SdrDragRotate::GetSdrDragPointer() const
{
    return Pointer(POINTER_ROTATE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragShear,SdrDragMethod);

SdrDragShear::SdrDragShear(SdrDragView& rNewView, bool bSlant1)
:   SdrDragMethod(rNewView),
    aFact(1,1),
    nWink0(0),
    nWink(0),
    nTan(0.0),
    bVertical(false),
    bResize(false),
    bUpSideDown(false),
    bSlant(bSlant1)
{
}

void SdrDragShear::TakeSdrDragComment(XubString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethShear, rStr);
    rStr.AppendAscii(" (");

    sal_Int32 nTmpWink(nWink);

    if(bUpSideDown)
        nTmpWink += 18000;

    nTmpWink = NormAngle180(nTmpWink);

    rtl::OUString aStr;
    getSdrDragView().GetModel()->TakeWinkStr(nTmpWink, aStr);
    rStr.Append(aStr);
    rStr += sal_Unicode(')');

    if(getSdrDragView().IsDragWithCopy())
        rStr += ImpGetResStr(STR_EditWithCopy);
}

bool SdrDragShear::BeginSdrDrag()
{
    SdrHdlKind eRefHdl=HDL_MOVE;
    SdrHdl* pRefHdl=NULL;

    switch (GetDragHdlKind())
    {
        case HDL_UPPER: eRefHdl=HDL_LOWER; break;
        case HDL_LOWER: eRefHdl=HDL_UPPER; break;
        case HDL_LEFT : eRefHdl=HDL_RIGHT; bVertical=true; break;
        case HDL_RIGHT: eRefHdl=HDL_LEFT ; bVertical=true; break;
        default: break;
    }

    if (eRefHdl!=HDL_MOVE)
        pRefHdl=GetHdlList().GetHdl(eRefHdl);

    if (pRefHdl!=NULL)
    {
        DragStat().Ref1()=pRefHdl->GetPos();
        nWink0=GetAngle(DragStat().GetStart()-DragStat().GetRef1());
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
    basegfx::B2DHomMatrix aRetval(basegfx::tools::createTranslateB2DHomMatrix(
        -DragStat().GetRef1().X(), -DragStat().GetRef1().Y()));

    if (bResize)
    {
        if (bVertical)
        {
            aRetval.scale(aFact, 1.0);
            aRetval.shearY(-nTan);
        }
        else
        {
            aRetval.scale(1.0, aFact);
            aRetval.shearX(-nTan);
        }
    }

    aRetval.translate(DragStat().GetRef1().X(), DragStat().GetRef1().Y());

    return aRetval;
}

void SdrDragShear::MoveSdrDrag(const Point& rPnt)
{
    if (DragStat().CheckMinMoved(rPnt))
    {
        bResize=!getSdrDragView().IsOrtho();
        long nSA=0;

        if (getSdrDragView().IsAngleSnapEnabled())
            nSA=getSdrDragView().GetSnapAngle();

        Point aP0(DragStat().GetStart());
        Point aPnt(rPnt);
        Fraction aNeuFact(1,1);

        // if angle snapping not activated, snap to raster (except when using slant)
        if (nSA==0 && !bSlant)
            aPnt=GetSnapPos(aPnt);

        if (!bSlant && !bResize)
        { // shear, but no resize
            if (bVertical)
                aPnt.X()=aP0.X();
            else
                aPnt.Y()=aP0.Y();
        }

        Point aRef(DragStat().GetRef1());
        Point aDif(aPnt-aRef);

        long nNeuWink=0;

        if (bSlant)
        {
            nNeuWink=NormAngle180(-(GetAngle(aDif)-nWink0));

            if (bVertical)
                nNeuWink=NormAngle180(-nNeuWink);
        }
        else
        {
            if (bVertical)
                nNeuWink=NormAngle180(GetAngle(aDif));
            else
                nNeuWink=NormAngle180(-(GetAngle(aDif)-9000));

            if (nNeuWink<-9000 || nNeuWink>9000)
                nNeuWink=NormAngle180(nNeuWink+18000);

            if (bResize)
            {
                Point aPt2(aPnt);

                if (nSA!=0)
                    aPt2=GetSnapPos(aPnt); // snap this one in any case

                if (bVertical)
                {
                    aNeuFact=Fraction(aPt2.X()-aRef.X(),aP0.X()-aRef.X());
                }
                else
                {
                    aNeuFact=Fraction(aPt2.Y()-aRef.Y(),aP0.Y()-aRef.Y());
                }
            }
        }

        bool bNeg=nNeuWink<0;

        if (bNeg)
            nNeuWink=-nNeuWink;

        if (nSA!=0)
        { // angle snapping
            nNeuWink+=nSA/2;
            nNeuWink/=nSA;
            nNeuWink*=nSA;
        }

        nNeuWink=NormAngle360(nNeuWink);
        bUpSideDown=nNeuWink>9000 && nNeuWink<27000;

        if (bSlant)
        { // calculate resize for slant
            // when angle snapping is activated, disable 89 degree limit
            long nTmpWink=nNeuWink;
            if (bUpSideDown) nNeuWink-=18000;
            if (bNeg) nTmpWink=-nTmpWink;
            bResize=true;
            double nCos=cos(nTmpWink*nPi180);
            aNeuFact=nCos;
            Kuerzen(aFact,10); // three decimals should be enough
        }

        if (nNeuWink>8900)
            nNeuWink=8900;

        if (bNeg)
            nNeuWink=-nNeuWink;

        if (nWink!=nNeuWink || aFact!=aNeuFact)
        {
            nWink=nNeuWink;
            aFact=aNeuFact;
            double a=nWink*nPi180;
            double nTan1=0.0;
            nTan1=tan(a); // calculate now, so as little time as possible passes between Hide() and Show()
            Hide();
            nTan=nTan1;
            DragStat().NextMove(rPnt);
            Show();
        }
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

    if (nWink!=0)
    {
        rTarget.Shear(DragStat().GetRef1(),nWink,tan(nWink*nPi180),bVertical);
    }
}

bool SdrDragShear::EndSdrDrag(bool bCopy)
{
    Hide();

    if (bResize && aFact==Fraction(1,1))
        bResize=false;

    if (nWink!=0 || bResize)
    {
        if (nWink!=0 && bResize)
        {
            XubString aStr;
            ImpTakeDescriptionStr(STR_EditShear,aStr);

            if (bCopy)
                aStr+=ImpGetResStr(STR_EditWithCopy);

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

        if (nWink!=0)
        {
            getSdrDragView().ShearMarkedObj(DragStat().GetRef1(),nWink,bVertical,bCopy);
        }

        if (nWink!=0 && bResize)
            getSdrDragView().EndUndo();

        return true;
    }

    return false;
}

Pointer SdrDragShear::GetSdrDragPointer() const
{
    if (bVertical)
        return Pointer(POINTER_VSHEAR);
    else
        return Pointer(POINTER_HSHEAR);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragMirror,SdrDragMethod);

void SdrDragMirror::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    if(bMirrored)
    {
        rTarget.Mirror(DragStat().GetRef1(), DragStat().GetRef2());
    }
}

SdrDragMirror::SdrDragMirror(SdrDragView& rNewView)
:   SdrDragMethod(rNewView),
    nWink(0),
    bMirrored(false),
    bSide0(false)
{
}

bool SdrDragMirror::ImpCheckSide(const Point& rPnt) const
{
    long nWink1=GetAngle(rPnt-DragStat().GetRef1());
    nWink1-=nWink;
    nWink1=NormAngle360(nWink1);

    return nWink1<18000;
}

void SdrDragMirror::TakeSdrDragComment(XubString& rStr) const
{
    if (aDif.X()==0)
        ImpTakeDescriptionStr(STR_DragMethMirrorHori,rStr);
    else if (aDif.Y()==0)
        ImpTakeDescriptionStr(STR_DragMethMirrorVert,rStr);
    else if (Abs(aDif.X())==Abs(aDif.Y()))
        ImpTakeDescriptionStr(STR_DragMethMirrorDiag,rStr);
    else
        ImpTakeDescriptionStr(STR_DragMethMirrorFree,rStr);

    if (getSdrDragView().IsDragWithCopy())
        rStr+=ImpGetResStr(STR_EditWithCopy);
}

bool SdrDragMirror::BeginSdrDrag()
{
    SdrHdl* pH1=GetHdlList().GetHdl(HDL_REF1);
    SdrHdl* pH2=GetHdlList().GetHdl(HDL_REF2);

    if (pH1!=NULL && pH2!=NULL)
    {
        DragStat().Ref1()=pH1->GetPos();
        DragStat().Ref2()=pH2->GetPos();
        Ref1()=pH1->GetPos();
        Ref2()=pH2->GetPos();
        aDif=pH2->GetPos()-pH1->GetPos();
        bool b90=(aDif.X()==0) || aDif.Y()==0;
        bool b45=b90 || (Abs(aDif.X())==Abs(aDif.Y()));
        nWink=NormAngle360(GetAngle(aDif));

        if (!getSdrDragView().IsMirrorAllowed(false,false) && !b45)
            return false; // free choice of axis angle not allowed

        if (!getSdrDragView().IsMirrorAllowed(true,false) && !b90)
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

        aRetval = basegfx::tools::createTranslateB2DHomMatrix(-DragStat().GetRef1().X(), -DragStat().GetRef1().Y());
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
        bool bNeuSide=ImpCheckSide(rPnt);
        bool bNeuMirr=bSide0!=bNeuSide;

        if (bMirrored!=bNeuMirr)
        {
            Hide();
            bMirrored=bNeuMirr;
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

Pointer SdrDragMirror::GetSdrDragPointer() const
{
    return Pointer(POINTER_MIRROR);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragGradient, SdrDragMethod);

SdrDragGradient::SdrDragGradient(SdrDragView& rNewView, bool bGrad)
:   SdrDragMethod(rNewView),
    pIAOHandle(NULL),
    bIsGradient(bGrad)
{
}

void SdrDragGradient::TakeSdrDragComment(XubString& rStr) const
{
    if(IsGradient())
        ImpTakeDescriptionStr(STR_DragMethGradient, rStr);
    else
        ImpTakeDescriptionStr(STR_DragMethTransparence, rStr);
}

bool SdrDragGradient::BeginSdrDrag()
{
    bool bRetval(false);

    pIAOHandle = (SdrHdlGradient*)GetHdlList().GetHdl(IsGradient() ? HDL_GRAD : HDL_TRNS);

    if(pIAOHandle)
    {
        // save old values
        DragStat().Ref1() = pIAOHandle->GetPos();
        DragStat().Ref2() = pIAOHandle->Get2ndPos();

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
                pIAOHandle->SetPos(DragStat().Ref1() + aMoveDiff);
                if(pIAOHandle->GetColorHdl1())
                    pIAOHandle->GetColorHdl1()->SetPos(DragStat().Ref1() + aMoveDiff);
            }
            else
            {
                pIAOHandle->Set2ndPos(DragStat().Ref2() + aMoveDiff);
                if(pIAOHandle->GetColorHdl2())
                    pIAOHandle->GetColorHdl2()->SetPos(DragStat().Ref2() + aMoveDiff);
            }
        }
        else
        {
            pIAOHandle->SetPos(DragStat().Ref1() + aMoveDiff);
            pIAOHandle->Set2ndPos(DragStat().Ref2() + aMoveDiff);

            if(pIAOHandle->GetColorHdl1())
                pIAOHandle->GetColorHdl1()->SetPos(DragStat().Ref1() + aMoveDiff);

            if(pIAOHandle->GetColorHdl2())
                pIAOHandle->GetColorHdl2()->SetPos(DragStat().Ref2() + aMoveDiff);
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
    pIAOHandle->SetPos(DragStat().Ref1());
    pIAOHandle->Set2ndPos(DragStat().Ref2());

    if(pIAOHandle->GetColorHdl1())
        pIAOHandle->GetColorHdl1()->SetPos(DragStat().Ref1());

    if(pIAOHandle->GetColorHdl2())
        pIAOHandle->GetColorHdl2()->SetPos(DragStat().Ref2());

    // new state
    pIAOHandle->FromIAOToItem(getSdrDragView().GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj(), true, false);
}

Pointer SdrDragGradient::GetSdrDragPointer() const
{
    return Pointer(POINTER_REFHAND);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragCrook,SdrDragMethod);

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
    nWink(0),
    nMarkSize(0),
    eMode(SDRCROOK_ROTATE)
{
}

void SdrDragCrook::TakeSdrDragComment(XubString& rStr) const
{
    ImpTakeDescriptionStr(!bContortion ? STR_DragMethCrook : STR_DragMethCrookContortion, rStr);

    if(bValid)
    {
        rStr.AppendAscii(" (");

        sal_Int32 nVal(nWink);

        if(bAtCenter)
            nVal *= 2;

        nVal = Abs(nVal);
        rtl::OUString aStr;
        getSdrDragView().GetModel()->TakeWinkStr(nVal, aStr);
        rStr.Append(aStr);
        rStr += sal_Unicode(')');
    }

    if(getSdrDragView().IsDragWithCopy())
        rStr += ImpGetResStr(STR_EditWithCopy);
}

// These defines parameterize the created raster
// for interactions
#define DRAG_CROOK_RASTER_MINIMUM   (4)
#define DRAG_CROOK_RASTER_MAXIMUM   (15)
#define DRAG_CROOK_RASTER_DISTANCE  (30)

basegfx::B2DPolyPolygon impCreateDragRaster(SdrPageView& rPageView, const Rectangle& rMarkRect)
{
    basegfx::B2DPolyPolygon aRetval;

    if(rPageView.PageWindowCount())
    {
        OutputDevice& rOut = (rPageView.GetPageWindow(0)->GetPaintWindow().GetOutputDevice());
        Rectangle aPixelSize = rOut.LogicToPixel(rMarkRect);
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

        const double fXLen(rMarkRect.GetWidth() / (double)nHorDiv);
        const double fYLen(rMarkRect.GetHeight() / (double)nVerDiv);
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
            addSdrDragEntry(new SdrDragEntryPolyPolygon(aDragRaster));
        }
    }

    // call parent
    SdrDragMethod::createSdrDragEntries();
}

bool SdrDragCrook::BeginSdrDrag()
{
    bContortionAllowed=getSdrDragView().IsCrookAllowed(false);
    bNoContortionAllowed=getSdrDragView().IsCrookAllowed(true);
    bResizeAllowed=getSdrDragView().IsResizeAllowed(false);
    bRotateAllowed=getSdrDragView().IsRotateAllowed(false);

    if (bContortionAllowed || bNoContortionAllowed)
    {
        bVertical=(GetDragHdlKind()==HDL_LOWER || GetDragHdlKind()==HDL_UPPER);
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

void SdrDragCrook::_MovAllPoints(basegfx::B2DPolyPolygon& rTarget)
{
    SdrPageView* pPV = getSdrDragView().GetSdrPageView();

    if(pPV)
    {
        XPolyPolygon aTempPolyPoly(rTarget);

        if (pPV->HasMarkedObjPageView())
        {
            sal_uInt16 nPolyAnz=aTempPolyPoly.Count();

            if (!bContortion && !getSdrDragView().IsNoDragXorPolys())
            {
                sal_uInt16 n1st=0,nLast=0;
                Point aC(aCenter);

                while (n1st<nPolyAnz)
                {
                    nLast=n1st;
                    while (nLast<nPolyAnz && aTempPolyPoly[nLast].GetPointCount()!=0) nLast++;
                    Rectangle aBound(aTempPolyPoly[n1st].GetBoundRect());
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
                            case SDRCROOK_ROTATE : CrookRotateXPoint (aCtr1,NULL,NULL,aC,aRad,nSin,nCos,bVertical);           break;
                            case SDRCROOK_SLANT  : CrookSlantXPoint  (aCtr1,NULL,NULL,aC,aRad,nSin,nCos,bVertical);           break;
                            case SDRCROOK_STRETCH: CrookStretchXPoint(aCtr1,NULL,NULL,aC,aRad,nSin,nCos,bVertical,aMarkRect); break;
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

                for (j=0; j<nPolyAnz; j++)
                {
                    XPolygon& aPol=aTempPolyPoly[j];
                    sal_uInt16 nPtAnz=aPol.GetPointCount();
                    i=0;

                    while (i<nPtAnz)
                    {
                        Point* pPnt=&aPol[i];
                        Point* pC1=NULL;
                        Point* pC2=NULL;

                        if (i+1<nPtAnz && aPol.IsControl(i))
                        { // control point on the left
                            pC1=pPnt;
                            i++;
                            pPnt=&aPol[i];
                        }

                        i++;

                        if (i<nPtAnz && aPol.IsControl(i))
                        { // control point on the right
                            pC2=&aPol[i];
                            i++;
                        }

                        _MovCrookPoint(*pPnt,pC1,pC2);
                    }
                }
            }
        }

        rTarget = aTempPolyPoly.getB2DPolyPolygon();
    }
}

void SdrDragCrook::_MovCrookPoint(Point& rPnt, Point* pC1, Point* pC2)
{
    bool bVert=bVertical;
    bool bC1=pC1!=NULL;
    bool bC2=pC2!=NULL;
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
            case SDRCROOK_ROTATE : CrookRotateXPoint (rPnt,pC1,pC2,aC,aRad,nSin,nCos,bVert);           break;
            case SDRCROOK_SLANT  : CrookSlantXPoint  (rPnt,pC1,pC2,aC,aRad,nSin,nCos,bVert);           break;
            case SDRCROOK_STRETCH: CrookStretchXPoint(rPnt,pC1,pC2,aC,aRad,nSin,nCos,bVert,aMarkRect); break;
        } // switch
    }
}

void SdrDragCrook::MoveSdrDrag(const Point& rPnt)
{
    if (DragStat().CheckMinMoved(rPnt))
    {
        Point aPnt(rPnt);
        bool bNeuMoveOnly=getSdrDragView().IsMoveOnlyDragging();
        bAtCenter=false;
        SdrCrookMode eNeuMode=getSdrDragView().GetCrookMode();
        bool bNeuContortion=!bNeuMoveOnly && ((bContortionAllowed && !getSdrDragView().IsCrookNoContortion()) || !bNoContortionAllowed);
        bResize=!getSdrDragView().IsOrtho() && bResizeAllowed && !bNeuMoveOnly;
        bool bNeuRotate=bRotateAllowed && !bNeuContortion && !bNeuMoveOnly && eNeuMode==SDRCROOK_ROTATE;
        long nSA=0;

        if (nSA==0)
            aPnt=GetSnapPos(aPnt);

        Point aNeuCenter(aMarkCenter.X(),aStart.Y());

        if (bVertical)
        {
            aNeuCenter.X()=aStart.X();
            aNeuCenter.Y()=aMarkCenter.Y();
        }

        if (!getSdrDragView().IsCrookAtCenter())
        {
            switch (GetDragHdlKind())
            {
                case HDL_UPLFT: aNeuCenter.X()=aMarkRect.Right();  bLft=true; break;
                case HDL_UPPER: aNeuCenter.Y()=aMarkRect.Bottom(); bUpr=true; break;
                case HDL_UPRGT: aNeuCenter.X()=aMarkRect.Left();   bRgt=true; break;
                case HDL_LEFT : aNeuCenter.X()=aMarkRect.Right();  bLft=true; break;
                case HDL_RIGHT: aNeuCenter.X()=aMarkRect.Left();   bRgt=true; break;
                case HDL_LWLFT: aNeuCenter.X()=aMarkRect.Right();  bLft=true; break;
                case HDL_LOWER: aNeuCenter.Y()=aMarkRect.Top();    bLwr=true; break;
                case HDL_LWRGT: aNeuCenter.X()=aMarkRect.Left();   bRgt=true; break;
                default: bAtCenter=true;
            }
        }
        else
            bAtCenter=true;

        Fraction aNeuFact(1,1);
        long dx1=aPnt.X()-aNeuCenter.X();
        long dy1=aPnt.Y()-aNeuCenter.Y();
        bValid=bVertical ? dx1!=0 : dy1!=0;

        if (bValid)
        {
            if (bVertical)
                bValid=Abs(dx1)*100>Abs(dy1);
            else
                bValid=Abs(dy1)*100>Abs(dx1);
        }

        long nNeuRad=0;
        nWink=0;

        if (bValid)
        {
            double a=0; // slope of the radius
            long nPntWink=0;

            if (bVertical)
            {
                a=((double)dy1)/((double)dx1); // slope of the radius
                nNeuRad=((long)(dy1*a)+dx1) /2;
                aNeuCenter.X()+=nNeuRad;
                nPntWink=GetAngle(aPnt-aNeuCenter);
            }
            else
            {
                a=((double)dx1)/((double)dy1); // slope of the radius
                nNeuRad=((long)(dx1*a)+dy1) /2;
                aNeuCenter.Y()+=nNeuRad;
                nPntWink=GetAngle(aPnt-aNeuCenter)-9000;
            }

            if (!bAtCenter)
            {
                if (nNeuRad<0)
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

                nPntWink=NormAngle360(nPntWink);
            }
            else
            {
                if (nNeuRad<0) nPntWink+=18000;
                if (bVertical) nPntWink=18000-nPntWink;
                nPntWink=NormAngle180(nPntWink);
                nPntWink=Abs(nPntWink);
            }

            double nUmfang=2*Abs(nNeuRad)*nPi;

            if (bResize)
            {
                if (nSA!=0)
                { // angle snapping
                    long nWink0=nPntWink;
                    nPntWink+=nSA/2;
                    nPntWink/=nSA;
                    nPntWink*=nSA;
                    BigInt a2(nNeuRad);
                    a2*=BigInt(nWink);
                    a2/=BigInt(nWink0);
                    nNeuRad=long(a2);

                    if (bVertical)
                        aNeuCenter.X()=aStart.X()+nNeuRad;
                    else
                        aNeuCenter.Y()=aStart.Y()+nNeuRad;
                }

                long nMul=(long)(nUmfang*NormAngle360(nPntWink)/36000);

                if (bAtCenter)
                    nMul*=2;

                aNeuFact=Fraction(nMul,nMarkSize);
                nWink=nPntWink;
            }
            else
            {
                nWink=(long)((nMarkSize*360/nUmfang)*100)/2;

                if (nWink==0)
                    bValid=false;

                if (bValid && nSA!=0)
                { // angle snapping
                    long nWink0=nWink;
                    nWink+=nSA/2;
                    nWink/=nSA;
                    nWink*=nSA;
                    BigInt a2(nNeuRad);
                    a2*=BigInt(nWink);
                    a2/=BigInt(nWink0);
                    nNeuRad=long(a2);

                    if (bVertical)
                        aNeuCenter.X()=aStart.X()+nNeuRad;
                    else
                        aNeuCenter.Y()=aStart.Y()+nNeuRad;
                }
            }
        }

        if (nWink==0 || nNeuRad==0)
            bValid=false;

        if (!bValid)
            nNeuRad=0;

        if (!bValid && bResize)
        {
            long nMul=bVertical ? dy1 : dx1;

            if (bLft || bUpr)
                nMul=-nMul;

            long nDiv=nMarkSize;

            if (bAtCenter)
            {
                nMul*=2;
                nMul=Abs(nMul);
            }

            aNeuFact=Fraction(nMul,nDiv);
        }

        if (aNeuCenter!=aCenter || bNeuContortion!=bContortion || aNeuFact!=aFact ||
            bNeuMoveOnly != getMoveOnly() || bNeuRotate!=bRotate || eNeuMode!=eMode)
        {
            Hide();
            setMoveOnly(bNeuMoveOnly);
            bRotate=bNeuRotate;
            eMode=eNeuMode;
            bContortion=bNeuContortion;
            aCenter=aNeuCenter;
            aFact=aNeuFact;
            aRad=Point(nNeuRad,nNeuRad);
            bResize=aFact!=Fraction(1,1) && aFact.GetDenominator()!=0 && aFact.IsValid();
            DragStat().NextMove(aPnt);
            Show();
        }
    }
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
            const Rectangle aLocalMarkRect(getSdrDragView().GetMarkedObjRect());
            const bool bLocalRotate(!bContortion && eMode == SDRCROOK_ROTATE && getSdrDragView().IsRotateAllowed(false));

            getSdrDragView().ImpCrookObj(&rTarget,aCenter,aRad,eMode,bVertical,!bContortion,bLocalRotate,aLocalMarkRect);
        }
    }
}

void SdrDragCrook::applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget)
{
    // use helper derived from old stuff
    _MovAllPoints(rTarget);
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
            XubString aStr;
            ImpTakeDescriptionStr(!bContortion?STR_EditCrook:STR_EditCrookContortion,aStr);

            if (bCopy)
                aStr+=ImpGetResStr(STR_EditWithCopy);

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

                sal_uLong nMarkAnz=getSdrDragView().GetMarkedObjectList().GetMarkCount();

                for (sal_uLong nm=0; nm<nMarkAnz; nm++)
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
            getSdrDragView().SetLastCrookCenter(aCenter);
        }

        if (bResize && bUndo)
            getSdrDragView().EndUndo();

        return true;
    }

    return false;
}

Pointer SdrDragCrook::GetSdrDragPointer() const
{
    return Pointer(POINTER_CROOK);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragDistort,SdrDragMethod);

SdrDragDistort::SdrDragDistort(SdrDragView& rNewView)
:   SdrDragMethod(rNewView),
    nPolyPt(0),
    bContortionAllowed(false),
    bNoContortionAllowed(false),
    bContortion(false)
{
}

void SdrDragDistort::TakeSdrDragComment(XubString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethDistort, rStr);

    rtl::OUString aStr;

    rStr.AppendAscii(" (x=");
    getSdrDragView().GetModel()->TakeMetricStr(DragStat().GetDX(), aStr);
    rStr.Append(aStr);
    rStr.AppendAscii(" y=");
    getSdrDragView().GetModel()->TakeMetricStr(DragStat().GetDY(), aStr);
    rStr.Append(aStr);
    rStr += sal_Unicode(')');

    if(getSdrDragView().IsDragWithCopy())
        rStr += ImpGetResStr(STR_EditWithCopy);
}

void SdrDragDistort::createSdrDragEntries()
{
    // Add extended frame raster first, so it will be behind objects
    if(getSdrDragView().GetSdrPageView())
    {
        const basegfx::B2DPolyPolygon aDragRaster(impCreateDragRaster(*getSdrDragView().GetSdrPageView(), GetMarkedRect()));

        if(aDragRaster.count())
        {
            addSdrDragEntry(new SdrDragEntryPolyPolygon(aDragRaster));
        }
    }

    // call parent
    SdrDragMethod::createSdrDragEntries();
}

bool SdrDragDistort::BeginSdrDrag()
{
    bContortionAllowed=getSdrDragView().IsDistortAllowed(false);
    bNoContortionAllowed=getSdrDragView().IsDistortAllowed(true);

    if (bContortionAllowed || bNoContortionAllowed)
    {
        SdrHdlKind eKind=GetDragHdlKind();
        nPolyPt=0xFFFF;

        if (eKind==HDL_UPLFT) nPolyPt=0;
        if (eKind==HDL_UPRGT) nPolyPt=1;
        if (eKind==HDL_LWRGT) nPolyPt=2;
        if (eKind==HDL_LWLFT) nPolyPt=3;
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

void SdrDragDistort::_MovAllPoints(basegfx::B2DPolyPolygon& rTarget)
{
    if (bContortion)
    {
        SdrPageView* pPV = getSdrDragView().GetSdrPageView();

        if(pPV)
        {
            if (pPV->HasMarkedObjPageView())
            {
                basegfx::B2DPolyPolygon aDragPolygon(rTarget);
                const basegfx::B2DRange aOriginalRange(aMarkRect.Left(), aMarkRect.Top(), aMarkRect.Right(), aMarkRect.Bottom());
                const basegfx::B2DPoint aTopLeft(aDistortedRect[0].X(), aDistortedRect[0].Y());
                const basegfx::B2DPoint aTopRight(aDistortedRect[1].X(), aDistortedRect[1].Y());
                const basegfx::B2DPoint aBottomLeft(aDistortedRect[3].X(), aDistortedRect[3].Y());
                const basegfx::B2DPoint aBottomRight(aDistortedRect[2].X(), aDistortedRect[2].Y());

                aDragPolygon = basegfx::tools::distort(aDragPolygon, aOriginalRange, aTopLeft, aTopRight, aBottomLeft, aBottomRight);
                rTarget = aDragPolygon;
            }
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

        bool bNeuContortion=(bContortionAllowed && !getSdrDragView().IsCrookNoContortion()) || !bNoContortionAllowed;

        if (bNeuContortion!=bContortion || aDistortedRect[nPolyPt]!=aPnt)
        {
            Hide();
            aDistortedRect[nPolyPt]=aPnt;
            bContortion=bNeuContortion;
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

Pointer SdrDragDistort::GetSdrDragPointer() const
{
    return Pointer(POINTER_REFHAND);
}

void SdrDragDistort::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    const bool bDoDistort(DragStat().GetDX()!=0 || DragStat().GetDY()!=0);

    if (bDoDistort)
    {
        getSdrDragView().ImpDistortObj(&rTarget, aMarkRect, aDistortedRect, !bContortion);
    }
}

void SdrDragDistort::applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget)
{
    // use helper derived from old stuff
    _MovAllPoints(rTarget);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrDragCrop,SdrDragResize);

SdrDragCrop::SdrDragCrop(SdrDragView& rNewView)
:   SdrDragResize(rNewView)
{
    // switch off solid dragging for crop; it just makes no sense since showing
    // a 50% transparent object above the original will not be visible
    setSolidDraggingActive(false);
}

void SdrDragCrop::TakeSdrDragComment(XubString& rStr) const
{
    ImpTakeDescriptionStr(STR_DragMethCrop, rStr);

    rtl::OUString aStr;

    rStr.AppendAscii(" (x=");
    getSdrDragView().GetModel()->TakeMetricStr(DragStat().GetDX(), aStr);
    rStr.Append(aStr);
    rStr.AppendAscii(" y=");
    getSdrDragView().GetModel()->TakeMetricStr(DragStat().GetDY(), aStr);
    rStr.Append(aStr);
    rStr += sal_Unicode(')');

    if(getSdrDragView().IsDragWithCopy())
        rStr += ImpGetResStr(STR_EditWithCopy);
}

bool SdrDragCrop::EndSdrDrag(bool bCopy)
{
    Hide();

    if( DragStat().GetDX()==0 && DragStat().GetDY()==0 )
        return false;

    const SdrMarkList& rMarkList = getSdrDragView().GetMarkedObjectList();

    if( rMarkList.GetMarkCount() != 1 )
        return false;

    SdrGrafObj* pObj = dynamic_cast<SdrGrafObj*>( rMarkList.GetMark( 0 )->GetMarkedSdrObj() );

    if( !pObj || (pObj->GetGraphicType() == GRAPHIC_NONE) || (pObj->GetGraphicType() == GRAPHIC_DEFAULT) )
        return false;

    const GraphicObject& rGraphicObject = pObj->GetGraphicObject();
    const MapMode aMapMode100thmm(MAP_100TH_MM);
    Size aGraphicSize(rGraphicObject.GetPrefSize());

    if( MAP_PIXEL == rGraphicObject.GetPrefMapMode().GetMapUnit() )
        aGraphicSize = Application::GetDefaultDevice()->PixelToLogic( aGraphicSize, aMapMode100thmm );
    else
        aGraphicSize = Application::GetDefaultDevice()->LogicToLogic( aGraphicSize, rGraphicObject.GetPrefMapMode(), aMapMode100thmm);

    if( aGraphicSize.nA == 0 || aGraphicSize.nB == 0 )
        return false;

    const SdrGrafCropItem& rOldCrop = (const SdrGrafCropItem&)pObj->GetMergedItem(SDRATTR_GRAFCROP);

    const bool bUndo = getSdrDragView().IsUndoEnabled();

    if( bUndo )
    {
        String aUndoStr;
        ImpTakeDescriptionStr(STR_DragMethCrop, aUndoStr);

        getSdrDragView().BegUndo( aUndoStr );
        getSdrDragView().AddUndo( getSdrDragView().GetModel()->GetSdrUndoFactory().CreateUndoGeoObject( *pObj ) );
    }

    Rectangle aOldRect( pObj->GetLogicRect() );
    getSdrDragView().ResizeMarkedObj(DragStat().Ref1(),aXFact,aYFact,bCopy);
    Rectangle aNewRect( pObj->GetLogicRect() );

    double fScaleX = ( aGraphicSize.Width() - rOldCrop.GetLeft() - rOldCrop.GetRight() ) / (double)aOldRect.GetWidth();
    double fScaleY = ( aGraphicSize.Height() - rOldCrop.GetTop() - rOldCrop.GetBottom() ) / (double)aOldRect.GetHeight();

    sal_Int32 nDiffLeft = aNewRect.nLeft - aOldRect.nLeft;
    sal_Int32 nDiffTop = aNewRect.nTop - aOldRect.nTop;
    sal_Int32 nDiffRight = aNewRect.nRight - aOldRect.nRight;
    sal_Int32 nDiffBottom = aNewRect.nBottom - aOldRect.nBottom;

    sal_Int32 nLeftCrop = static_cast<sal_Int32>( rOldCrop.GetLeft() + nDiffLeft * fScaleX );
    sal_Int32 nTopCrop = static_cast<sal_Int32>( rOldCrop.GetTop() + nDiffTop * fScaleY );
    sal_Int32 nRightCrop = static_cast<sal_Int32>( rOldCrop.GetRight() - nDiffRight * fScaleX );
    sal_Int32 nBottomCrop = static_cast<sal_Int32>( rOldCrop.GetBottom() - nDiffBottom * fScaleY );

    SfxItemPool& rPool = getSdrDragView().GetModel()->GetItemPool();
    SfxItemSet aSet( rPool, SDRATTR_GRAFCROP, SDRATTR_GRAFCROP );
    aSet.Put( SdrGrafCropItem( nLeftCrop, nTopCrop, nRightCrop, nBottomCrop ) );
    getSdrDragView().SetAttributes( aSet, false );

    if( bUndo )
        getSdrDragView().EndUndo();

    return true;
}

Pointer SdrDragCrop::GetSdrDragPointer() const
{
    return Pointer(POINTER_CROP);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
