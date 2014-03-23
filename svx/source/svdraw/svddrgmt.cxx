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
#include <svx/svdocapt.hxx>
#include <svx/svdpagv.hxx>
#include "svx/svdstr.hrc"   // Namen aus der Resource
#include "svx/svdglob.hxx"  // StringCache
#include <svx/svddrgv.hxx>
#include <svx/svdundo.hxx>
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
#include <svx/svdouno.hxx>
#include <svx/sdr/primitive2d/sdrprimitivetools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>
#include <svx/svdlegacy.hxx>
#include <map>
#include <vector>

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

        aRetval.realloc(2);
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
        deleteSdrObjectSafeAndClearPointer(mpClone);
    }
}

void SdrDragEntrySdrObject::prepareCurrentState(SdrDragMethod& rDragMethod)
{
    // for the moment, i need to re-create the clone in all cases. I need to figure
    // out when clone and original have the same class, so that i can use operator=
    // in those cases
    if(mpClone)
    {
        deleteSdrObjectSafeAndClearPointer(mpClone);
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
    }
}

drawinglayer::primitive2d::Primitive2DSequence SdrDragEntrySdrObject::createPrimitive2DSequenceInCurrentState(SdrDragMethod& /* rDragMethod */)
{
    const SdrObject* pSource = &maOriginal;

    if(mbModify && mpClone)
    {
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
    // add parts to transparent overlay stuff eventually
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

    if(maPositions.size())
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
            const basegfx::BColor aBackPen(1.0, 1.0, 1.0);
            const basegfx::BColor aRGBFrontColor(0.0, 0.0, 1.0); // COL_LIGHTBLUE
            drawinglayer::primitive2d::Primitive2DReference aMarkerArrayPrimitive2D(
                new drawinglayer::primitive2d::MarkerArrayPrimitive2D(aTransformedPositions,
                    drawinglayer::primitive2d::createDefaultGluepoint_7x7(aBackPen, aRGBFrontColor)));

            aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aMarkerArrayPrimitive2D, 1);
        }
    }

    return aRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const basegfx::B2DRange& SdrDragMethod::GetMarkedRange() const
{
    if(HDL_POLY == getSdrView().GetDragHdlKind())
    {
        return getSdrView().getMarkedPointRange();
    }

    if(HDL_GLUE == getSdrView().GetDragHdlKind())
    {
        return getSdrView().getMarkedGluePointRange();
    }

    return getSdrView().getMarkedObjectSnapRange();
}

void SdrDragMethod::resetSdrDragEntries()
{
    // clear entries; creation is on demand
    clearSdrDragEntries();
}

basegfx::B2DRange SdrDragMethod::getCurrentRange() const
{
    return getB2DRangeFromOverlayObjectList();
}

void SdrDragMethod::clearSdrDragEntries()
{
    for(sal_uInt32 a(0); a < maSdrDragEntries.size(); a++)
    {
        delete maSdrDragEntries[a];
    }

    maSdrDragEntries.clear();
}

void SdrDragMethod::addSdrDragEntry(SdrDragEntry* pNew)
{
    if(pNew)
    {
        maSdrDragEntries.push_back(pNew);
    }
}

void SdrDragMethod::createSdrDragEntries()
{
    if(getSdrView().GetSdrPageView())
    {
        if(getSdrView().IsDraggingPoints())
        {
            createSdrDragEntries_PointDrag();
        }
        else if(getSdrView().IsDraggingGluePoints())
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
    // add full obejct drag; Clone() at the object has to work
    // for this
    addSdrDragEntry(new SdrDragEntrySdrObject(rOriginal, rObjectContact, bModify));
}

void SdrDragMethod::createSdrDragEntries_SolidDrag()
{
    SdrPageView* pPV = getSdrView().GetSdrPageView();

    if(pPV && pPV->PageWindowCount())
    {
        sdr::contact::ObjectContact& rOC = pPV->GetPageWindow(0)->GetObjectContact();
        const SdrObjectVector aSelection(getSdrView().getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 a(0); a < aSelection.size(); a++)
        {
            const SdrObject* pObject = aSelection[a];
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
                        // add full obejct drag; Clone() at the object has to work
                        // for this
                        createSdrDragEntryForSdrObject(*pCandidate, rOC, true);
                    }

                    if(bAddWireframe)
                    {
                        // when dragging a 50% transparent copy of a filled or not filled object without
                        // outline, this is normally hard to see. Add extra wireframe in that case. This
                        // works nice e.g. with thext frames etc.
                        addSdrDragEntry(new SdrDragEntryPolyPolygon(pCandidate->TakeXorPoly()));
                    }
                }
            }
        }
    }
}

void SdrDragMethod::createSdrDragEntries_PolygonDrag()
{
    const SdrObjectVector aSelection(getSdrView().getSelectedSdrObjectVectorFromSdrMarkView());
    bool bNoPolygons(getSdrView().IsNoDragXorPolys() || aSelection.size() > 1200); // DragXorPolyLimit, was at 100
    basegfx::B2DPolyPolygon aResult;
    sal_uInt32 nPointCount(0);

    for(sal_uInt32 a(0); !bNoPolygons && a < aSelection.size(); a++)
    {
        const basegfx::B2DPolyPolygon aNewPolyPolygon(aSelection[a]->TakeXorPoly());

        for(sal_uInt32 b(0); b < aNewPolyPolygon.count(); b++)
        {
            nPointCount += aNewPolyPolygon.getB2DPolygon(b).count();
        }

        if(nPointCount > 50000) // old DragXorPointLimit, was at 500
        {
            bNoPolygons = true;
        }

        if(!bNoPolygons)
        {
            aResult.append(aNewPolyPolygon);
        }
    }

    if(bNoPolygons)
    {
        const basegfx::B2DRange aNewRange(sdr::legacy::GetAllObjBoundRange(aSelection));
        basegfx::B2DPolygon aNewPolygon(basegfx::tools::createPolygonFromRect(aNewRange));

        aResult = basegfx::B2DPolyPolygon(basegfx::tools::expandToCurve(aNewPolygon));
    }

    if(aResult.count())
    {
        addSdrDragEntry(new SdrDragEntryPolyPolygon(aResult));
    }
}

void SdrDragMethod::createSdrDragEntries_PointDrag()
{
    if(getSdrView().areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSdrView().getSelectedSdrObjectVectorFromSdrMarkView());
        std::vector< basegfx::B2DPoint > aPositions;

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            const SdrPathObj* pPath = dynamic_cast< const SdrPathObj* >(aSelection[nm]);

            if(pPath)
            {
                const sdr::selection::Indices aMarkedPoints(getSdrView().getSelectedPointsForSelectedSdrObject(*pPath));

                if(aMarkedPoints.size())
                {
                    const basegfx::B2DPolyPolygon aPathXPP(pPath->getB2DPolyPolygonInObjectCoordinates());

                    if(aPathXPP.count())
                    {
                        for(sdr::selection::Indices::const_iterator aMarkedPoint(aMarkedPoints.begin());
                            aMarkedPoint != aMarkedPoints.end(); aMarkedPoint++)
                        {
                            sal_uInt32 nPolyNum, nPointNum;

                            if(sdr::PolyPolygonEditor::GetRelativePolyPoint(aPathXPP, *aMarkedPoint, nPolyNum, nPointNum))
                            {
                                aPositions.push_back(aPathXPP.getB2DPolygon(nPolyNum).getB2DPoint(nPointNum));
                            }
                        }
                    }
                }
            }
        }

        if(aPositions.size())
        {
            addSdrDragEntry(new SdrDragEntryPointGlueDrag(aPositions, true));
        }
    }
}

void SdrDragMethod::createSdrDragEntries_GlueDrag()
{
    if(getSdrView().areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSdrView().getSelectedSdrObjectVectorFromSdrMarkView());
        std::vector< basegfx::B2DPoint > aPositions;

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            const SdrObject* pSdrObjCandidate = aSelection[nm];

            if(pSdrObjCandidate)
            {
                const sdr::selection::Indices aMarkedGluePoints(getSdrView().getSelectedGluesForSelectedSdrObject(*pSdrObjCandidate));

                if(aMarkedGluePoints.size())
                {
                    const sdr::glue::GluePointProvider& rProvider = pSdrObjCandidate->GetGluePointProvider();

                    if(rProvider.hasUserGluePoints())
                    {
                        for(sdr::selection::Indices::const_iterator aCurrent(aMarkedGluePoints.begin());
                            aCurrent != aMarkedGluePoints.end(); aCurrent++)
                        {
                            const sal_uInt32 nObjPt(*aCurrent);
                            const sdr::glue::GluePoint* pCandidate = rProvider.findUserGluePointByID(nObjPt);

                            if(pCandidate)
                            {
                                aPositions.push_back(pSdrObjCandidate->getSdrObjectTransformation() * pCandidate->getUnitPosition());
                            }
                        }
                    }
                }
            }
            else
            {
                OSL_ENSURE(false, "ObjectSelection vector with unallowed gaps (!)");
            }
        }

        if(aPositions.size())
        {
            addSdrDragEntry(new SdrDragEntryPointGlueDrag(aPositions, false));
        }
    }
}

void SdrDragMethod::TakeMarkedDescriptionString(sal_uInt16 nStrCacheID, XubString& rStr, sal_uInt16 nVal) const
{
    sal_uInt16 nOpt(0);

    if(IsDraggingPoints())
    {
        nOpt = IMPSDR_POINTSDESCRIPTION;
    }
    else if(IsDraggingGluePoints())
    {
        nOpt = IMPSDR_GLUEPOINTSDESCRIPTION;
    }

    getSdrView().TakeMarkedDescriptionString(nStrCacheID, rStr, nVal, nOpt);
}

SdrObject* SdrDragMethod::GetDragObj() const
{
    SdrObject* pObj = 0;

    if(getSdrView().GetDragHdl())
    {
        pObj = const_cast< SdrObject*>(getSdrView().GetDragHdl()->GetObj());
    }

    if(!pObj)
    {
        pObj = getSdrView().getSelectedIfSingle();
    }

    return pObj;
}

void SdrDragMethod::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    // use get/setSdrObjectTransformation now. This will also work when object has a path
    // correct to minimal scaling for zero-width/height objects
    basegfx::B2DHomMatrix aObjectMatrix(basegfx::tools::guaranteeMinimalScaling(rTarget.getSdrObjectTransformation()));

    // apply current transformation and set
    aObjectMatrix = getCurrentTransformation() * aObjectMatrix;
    rTarget.setSdrObjectTransformation(aObjectMatrix);
}

void SdrDragMethod::applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget)
{
    // original uses CurrentTransformation
    rTarget.transform(getCurrentTransformation());
}

SdrDragMethod::SdrDragMethod(SdrView& rNewView)
:   maSdrDragEntries(),
    maOverlayObjectList(),
    mrSdrView(rNewView),
    mbMoveOnly(false),
    mbSolidDraggingActive(getSdrView().IsSolidDragging())
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
    getSdrView().ShowDragObj();
}

void SdrDragMethod::Hide()
{
    getSdrView().HideDragObj();
}

basegfx::B2DHomMatrix SdrDragMethod::getCurrentTransformation()
{
    return basegfx::B2DHomMatrix();
}

void SdrDragMethod::CancelSdrDrag()
{
    Hide();
}

struct compareConstSdrObjectRefs
{
    bool operator()(const SdrObject* p1, const SdrObject* p2) const
    {
        return (p1 < p2);
    }
};

typedef std::map< const SdrObject*, SdrObject*, compareConstSdrObjectRefs> SdrObjectAndCloneMap;

void SdrDragMethod::CreateOverlayGeometry(sdr::overlay::OverlayManager& rOverlayManager)
{
    // create SdrDragEntries on demand
    if(!maSdrDragEntries.size())
    {
        createSdrDragEntries();
    }

    // if there are entries, derive OverlayObjects from the entries, including
    // modification from current interactive state
    if(maSdrDragEntries.size())
    {
        // #54102# SdrDragEntrySdrObject creates clones of SdrObjects as base for creating the needed
        // primitives, holding the original and the clone. If connectors (Edges) are involved,
        // the cloned connectors need to be connected to the cloned SdrObjects (after cloning
        // they are connected to the original SdrObjects). To do so, trigger the preparation
        // steps for SdrDragEntrySdrObject, save an association of (orig, clone) in a helper
        // and evtl. remember if it was an edge
        SdrObjectAndCloneMap aOriginalAndClones;
        std::vector< SdrEdgeObj* > aEdges;
        sal_uInt32 a;

        // #54102# execute prepareCurrentState for all SdrDragEntrySdrObject, register pair of original and
        // clone, remember edges
        for(a = 0; a < maSdrDragEntries.size(); a++)
        {
            SdrDragEntrySdrObject* pSdrDragEntrySdrObject = dynamic_cast< SdrDragEntrySdrObject*>(maSdrDragEntries[a]);

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

        // #54102# if there are edges, reconnect their ends to the corresponding clones (if found)
        if(aEdges.size())
        {
            for(a = 0; a < aEdges.size(); a++)
            {
                SdrEdgeObj* pSdrEdgeObj = aEdges[a];
                SdrObject* pConnectedTo = pSdrEdgeObj->GetSdrObjectConnection(true);

                if(pConnectedTo)
                {
                    SdrObjectAndCloneMap::iterator aEntry = aOriginalAndClones.find(pConnectedTo);

                    if(aEntry != aOriginalAndClones.end())
                    {
                        pSdrEdgeObj->ConnectToSdrObject(true, aEntry->second);
                    }
                }

                pConnectedTo = pSdrEdgeObj->GetSdrObjectConnection(false);

                if(pConnectedTo)
                {
                    SdrObjectAndCloneMap::iterator aEntry = aOriginalAndClones.find(pConnectedTo);

                    if(aEntry != aOriginalAndClones.end())
                    {
                        pSdrEdgeObj->ConnectToSdrObject(false, aEntry->second);
                    }
                }
            }
        }

        // collect primitives for visualisation
        drawinglayer::primitive2d::Primitive2DSequence aResult;
        drawinglayer::primitive2d::Primitive2DSequence aResultTransparent;

        for(a = 0; a < maSdrDragEntries.size(); a++)
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

    // evtl add DragStripes (help lines cross the page when dragging)
    if(getSdrView().IsDragStripes())
    {
        const basegfx::B2DRange aActionRange(getSdrView().TakeActionRange());
        sdr::overlay::OverlayRollingRectangleStriped* pNew =
            new sdr::overlay::OverlayRollingRectangleStriped(
                aActionRange.getMinimum(),
                aActionRange.getMaximum(),
                true,
                false);

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
    if(getSdrView().areSdrObjectsSelected())
    {
        if(!getMoveOnly() && !(
            dynamic_cast< SdrDragMove* >(this) ||
            dynamic_cast< SdrDragResize* >(this) ||
            dynamic_cast< SdrDragRotate* >(this) ||
            dynamic_cast< SdrDragMirror* >(this)))
        {
            return false;
        }

        if(getSdrView().IsDraggingPoints() || getSdrView().IsDraggingGluePoints())
        {
            return false;
        }

        if(dynamic_cast< SdrDragObjOwn* >(this) || dynamic_cast< SdrDragMovHdl* >(this))
        {
            return false;
        }

        /// get all SdrEdgeObj which are connected to selected SdrObjects, but not selected themselves
        const SdrObjectVector aSelection(getSdrView().getSelectedSdrObjectVectorFromSdrMarkView());
        const ::std::vector< SdrEdgeObj* > aConnectedSdrEdgeObjs(getAllSdrEdgeObjConnectedToSdrObjectVector(aSelection, false));

        if(!aConnectedSdrEdgeObjs.size())
        {
            return false;
        }
    }

    return true;
}

drawinglayer::primitive2d::Primitive2DSequence SdrDragMethod::AddConnectorOverlays()
{
    drawinglayer::primitive2d::Primitive2DSequence aRetval;
    const bool bDetail(getMoveOnly());

    if(getSdrView().areSdrObjectsSelected())
    {
        /// get all SdrEdgeObj which are connected to selected SdrObjects, but not selected themselves
        const SdrObjectVector aSelection(getSdrView().getSelectedSdrObjectVectorFromSdrMarkView());
        const ::std::vector< SdrEdgeObj* > aConnectedSdrEdgeObjs(getAllSdrEdgeObjConnectedToSdrObjectVector(aSelection, false));

        for(sal_uInt32 a(0); a < aConnectedSdrEdgeObjs.size(); a++)
        {
            const SdrEdgeObj* pEdge = aConnectedSdrEdgeObjs[a];
            const SdrObject* pCon1(pEdge->GetSdrObjectConnection(true));
            const SdrObject* pCon2(pEdge->GetSdrObjectConnection(false));
            const bool bCon1(pCon1 && getSdrView().isSdrObjectSelected(*pCon1));
            const bool bCon2(pCon2 && getSdrView().isSdrObjectSelected(*pCon2));
            const basegfx::B2DPolygon aEdgePolygon(
                pEdge->CreateConnectorOverlay(
                    getCurrentTransformation(),
                    bCon1,
                    bCon2,
                    bDetail));

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

    return aRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragMovHdl::SdrDragMovHdl(SdrView& rNewView)
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
    if (getSdrView().IsDragWithCopy()) rStr+=ImpGetResStr(STR_EditWithCopy);
}

bool SdrDragMovHdl::BeginSdrDrag()
{
    if( !GetDragHdl() )
        return false;

    DragStat().SetRef1(GetDragHdl()->getPosition());
    DragStat().SetShown(!DragStat().IsShown());

    const SdrHdlKind eKind(GetDragHdl()->GetKind());
    SdrHdl* pH1 = GetHdlList().GetHdlByKind(HDL_REF1);
    SdrHdl* pH2 = GetHdlList().GetHdlByKind(HDL_REF2);

    if(HDL_MIRX == eKind)
    {
        if(!pH1 || !pH2)
        {
            DBG_ERROR("SdrDragMovHdl::BeginSdrDrag(): Verschieben der Spiegelachse: Referenzhandles nicht gefunden");
            return false;
        }

        DragStat().SetActionRange(basegfx::B2DRange(pH1->getPosition(), pH2->getPosition()));
    }
    else
    {
        const basegfx::B2DPoint aPt(GetDragHdl()->getPosition());

        DragStat().SetActionRange(basegfx::B2DRange(aPt,aPt));
    }

    return true;
}

void SdrDragMovHdl::MoveSdrDrag(const basegfx::B2DPoint& rNoSnapPnt)
{
    if(GetDragHdl() && DragStat().CheckMinMoved(rNoSnapPnt))
    {
        basegfx::B2DPoint aPnt(rNoSnapPnt);

        if(HDL_MIRX == GetDragHdl()->GetKind())
        {
            SdrHdl* pH1 = GetHdlList().GetHdlByKind(HDL_REF1);
            SdrHdl* pH2 = GetHdlList().GetHdlByKind(HDL_REF2);

            if(!pH1 || !pH2)
            {
                return;
            }

            if(!DragStat().IsNoSnap())
            {
                const basegfx::B2DVector aOffset(aPnt - DragStat().GetStart());
                const basegfx::B2DPoint aPossibleRef1(GetRef1() + aOffset);
                const basegfx::B2DPoint aPossibleRef2(GetRef2() + aOffset);
                const basegfx::B2DVector aSnap1(aPossibleRef1 - getSdrView().GetSnapPos(aPossibleRef1));
                const basegfx::B2DVector aSnap2(aPossibleRef2 - getSdrView().GetSnapPos(aPossibleRef2));

                if(basegfx::fTools::less(aSnap1.getLength(), aSnap2.getLength()))
                {
                    aPnt += aSnap1;
                }
                else
                {
                    aPnt += aSnap2;
                }
            }

            if(!aPnt.equal(DragStat().GetNow()))
            {
                Hide();
                DragStat().NextMove(aPnt);

                const basegfx::B2DVector aDif(DragStat().GetNow() - DragStat().GetStart());

                pH1->setPosition(GetRef1() + aDif);
                pH2->setPosition(GetRef2() + aDif);

                SdrHdl* pHM = GetHdlList().GetHdlByKind(HDL_MIRX);

                if(pHM)
                {
                    pHM->Touch();
                }

                Show();
                DragStat().SetActionRange(basegfx::B2DRange(pH1->getPosition(), pH2->getPosition()));
            }
        }
        else
        {
            if(!DragStat().IsNoSnap())
            {
                aPnt = getSdrView().GetSnapPos(aPnt);
            }

            double fSnapAngle(0.0);

            if(getSdrView().IsAngleSnapEnabled())
            {
                fSnapAngle = ((getSdrView().GetSnapAngle() % 36000) * F_PI) / 18000.0;
            }

            if(getSdrView().IsMirrorAllowed(true, true))
            {
                // eingeschraenkt
                if(!getSdrView().IsMirrorAllowed(false, false))
                {
                    fSnapAngle = F_PI4; // 4500;
                }

                if(!getSdrView().IsMirrorAllowed(true, false))
                {
                    fSnapAngle = F_PI2; // 9000;
                }
            }

            if(getSdrView().IsOrthogonal() && !basegfx::fTools::equal(fSnapAngle, F_PI2))
            {
                fSnapAngle = F_PI4; // 4500;
            }

            if(!basegfx::fTools::equalZero(fSnapAngle))
            {
                // Winkelfang
                SdrHdlKind eRef = HDL_REF1;

                if(HDL_REF1 == GetDragHdl()->GetKind())
                {
                    eRef = HDL_REF2;
                }

                SdrHdl* pH = GetHdlList().GetHdlByKind(eRef);

                if(pH)
                {
                    const basegfx::B2DPoint aFixPoint(pH->getPosition());

                    if(!aFixPoint.equal(aPnt))
                    {
                        const basegfx::B2DVector aDelta(aPnt - aFixPoint);
                        const double fCurrentAngle(atan2(aDelta.getY(), aDelta.getX()));
                        double fNewAngle(basegfx::snapToNearestMultiple(fCurrentAngle, fSnapAngle));

                        if(!basegfx::fTools::equal(fCurrentAngle, fNewAngle))
                        {
                            aPnt += basegfx::B2DPoint(cos(fNewAngle), sin(fNewAngle));

                            if(basegfx::fTools::equal(fSnapAngle, F_PI4)) // 4500 == fSnapAngle
                            {
                                aPnt = OrthoDistance8(aFixPoint, aPnt, true);
                            }
                        }
                    }
                }
            }

            if(!aPnt.equal(DragStat().GetNow()))
            {
                Hide();
                DragStat().NextMove(aPnt);
                GetDragHdl()->setPosition(DragStat().GetNow());
                SdrHdl* pHM = GetHdlList().GetHdlByKind(HDL_MIRX);

                if(pHM)
                {
                    pHM->Touch();
                }

                Show();
                DragStat().SetActionRange(basegfx::B2DRange(aPnt));
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
                SetRef1(DragStat().GetNow());
                break;

            case HDL_REF2:
                SetRef2(DragStat().GetNow());
                break;

            case HDL_MIRX:
                SetRef1(GetRef1()+DragStat().GetNow()-DragStat().GetStart());
                SetRef2(GetRef2()+DragStat().GetNow()-DragStat().GetStart());
                break;

            default:
                break;
        }
    }

    return true;
}

void SdrDragMovHdl::CancelSdrDrag()
{
    Hide();

    if(GetDragHdl())
    {
        GetDragHdl()->setPosition(DragStat().GetRef1());

        SdrHdl* pHM = GetHdlList().GetHdlByKind(HDL_MIRX);

        if(pHM)
        {
            pHM->Touch();
        }
    }
}

Pointer SdrDragMovHdl::GetSdrDragPointer() const
{
    const SdrHdl* pHdl = GetDragHdl();

    if (pHdl)
    {
        return pHdl->GetPointer();
    }

    return Pointer(POINTER_REFHAND);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragObjOwn::SdrDragObjOwn(SdrView& rNewView)
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
        deleteSdrObjectSafeAndClearPointer(mpClone);
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
            SdrPageView* pPV = getSdrView().GetSdrPageView();

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
                // create nitial clone to have a start visualisation
                mpClone = pObj->getFullDragClone();
                mpClone->applySpecialDrag(DragStat());

                return true;
            }
        }
    }

    return false;
}

void SdrDragObjOwn::MoveSdrDrag(const basegfx::B2DPoint& rNoSnapPnt)
{
    const SdrObject* pObj = GetDragObj();

    if(pObj)
    {
        SdrPageView* pPV = getSdrView().GetSdrPageView();

        if(pPV)
        {
            basegfx::B2DPoint aPnt(rNoSnapPnt);

            if(!DragStat().IsNoSnap())
            {
                aPnt = getSdrView().GetSnapPos(aPnt);
            }

            if(getSdrView().IsOrthogonal())
            {
                if (DragStat().IsOrtho8Possible())
                {
                    aPnt = OrthoDistance8(DragStat().GetStart(), aPnt, getSdrView().IsBigOrthogonal());
                }
                else if (DragStat().IsOrtho4Possible())
                {
                    aPnt = OrthoDistance4(DragStat().GetStart(), aPnt, getSdrView().IsBigOrthogonal());
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
                        deleteSdrObjectSafeAndClearPointer(mpClone);
                        mpClone = 0;
                    }

                    // create a new clone and modify to current drag state
                    if(!mpClone)
                    {
                        mpClone = pObj->getFullDragClone();
                        mpClone->applySpecialDrag(DragStat());

                        // #120999# AutoGrowWidth may change for SdrTextObj due to the automatism used
                        // with bDisableAutoWidthOnDragging, so not only geometry changes but
                        // also this (pretty indirect) property change is possible. If it gets
                        // changed, it needs to be copied to the original since nothing will
                        // happen when it only changes in the drag clone
                        const bool bOldAutoGrowWidth(((SdrOnOffItem&)pObj->GetMergedItem(SDRATTR_TEXT_AUTOGROWWIDTH)).GetValue());
                        const bool bNewAutoGrowWidth(((SdrOnOffItem&)mpClone->GetMergedItem(SDRATTR_TEXT_AUTOGROWWIDTH)).GetValue());

                        if(bOldAutoGrowWidth != bNewAutoGrowWidth)
                        {
                            GetDragObj()->SetMergedItem(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, bNewAutoGrowWidth));
                        }
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
    SdrUndoAction* pUndo = 0;
    SdrUndoAction* pUndo2 = 0;
    std::vector< SdrUndoAction* > vConnectorUndoActions;
    bool bRet = false;
    SdrObject* pObj = GetDragObj();

    if(pObj)
    {
        const bool bUndo = getSdrView().IsUndoEnabled();

        if( bUndo )
        {
            if(!getSdrView().IsInsObjPoint() && pObj->IsObjectInserted() )
            {
                if (DragStat().IsEndDragChangesAttributes())
                {
                    pUndo = getSdrView().getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoAttrObject(*pObj);

                    if (DragStat().IsEndDragChangesGeoAndAttributes())
                    {
                        vConnectorUndoActions = getSdrView().CreateConnectorUndo( *pObj );
                        pUndo2 = getSdrView().getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pObj);
                    }
                }
                else
                {
                    vConnectorUndoActions = getSdrView().CreateConnectorUndo( *pObj );
                    pUndo = getSdrView().getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pObj);
                }
            }

            if( pUndo )
            {
                getSdrView().BegUndo( pUndo->GetComment() );
            }
            else
            {
                getSdrView().BegUndo();
            }
        }

        // evtl. use opertator= for setting changed object data (do not change selection in
        // view, this will destroy the interactor). This is possible since a clone is now
        // directly modified by the modifiers. Only SdrTableObj is adding own UNDOs
        // in it's SdrTableObj::endSpecialDrag, so currently not possible. OTOH it uses
        // a CreateUndoGeoObject() so maybe setting SetEndDragChangesAttributes is okay. I
        // will test this now
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pObj, HINT_OBJCHG_RESIZE);
        bRet = pObj->applySpecialDrag(DragStat());

        if(bRet)
        {
            pObj->SetChanged();
        }

        if(bRet)
        {
            if( bUndo )
            {
                getSdrView().AddUndoActions( vConnectorUndoActions );

                if ( pUndo )
                {
                    getSdrView().AddUndo(pUndo);
                }

                if ( pUndo2 )
                {
                    getSdrView().AddUndo(pUndo2);
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
            getSdrView().EndUndo();
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

void SdrDragMove::createSdrDragEntryForSdrObject(const SdrObject& rOriginal, sdr::contact::ObjectContact& rObjectContact, bool /*bModify*/)
{
    // for SdrDragMove, use current Primitive2DSequence of SdrObject visualisation
    // in given ObjectContact directly
    sdr::contact::ViewContact& rVC = rOriginal.GetViewContact();
    sdr::contact::ViewObjectContact& rVOC = rVC.GetViewObjectContact(rObjectContact);
    sdr::contact::DisplayInfo aDisplayInfo;

    // Do not use the last ViewPort set at the OC from the last ProcessDisplay(),
    // here we want the complete primitive sequence without visibility clippings
    rObjectContact.resetViewPort();

    addSdrDragEntry(new SdrDragEntryPrimitive2DSequence(rVOC.getPrimitive2DSequenceHierarchy(aDisplayInfo), true));
}

SdrDragMove::SdrDragMove(SdrView& rNewView)
:   SdrDragMethod(rNewView),
    maBestSnap(0.0, 0.0),
    mbXSnapped(false),
    mbYSnapped(false)
{
    setMoveOnly(true);
}

void SdrDragMove::TakeSdrDragComment(XubString& rStr) const
{
    XubString aStr;

    TakeMarkedDescriptionString(STR_DragMethMove, rStr);
    rStr.AppendAscii(" (x=");
    getSdrView().getSdrModelFromSdrView().TakeMetricStr(DragStat().GetDX(), aStr);
    rStr += aStr;
    rStr.AppendAscii(" y=");
    getSdrView().getSdrModelFromSdrView().TakeMetricStr(DragStat().GetDY(), aStr);
    rStr += aStr;
    rStr += sal_Unicode(')');

    if(getSdrView().IsDragWithCopy())
    {
        if(!getSdrView().IsInsObjPoint() && !(getSdrView().GetDragMethod() && getSdrView().IsInsertGluePoint()))
        {
            rStr += ImpGetResStr(STR_EditWithCopy);
        }
    }
}

bool SdrDragMove::BeginSdrDrag()
{
    DragStat().SetActionRange(GetMarkedRange());
    Show();

    return true;
}

basegfx::B2DHomMatrix SdrDragMove::getCurrentTransformation()
{
    return basegfx::tools::createTranslateB2DHomMatrix(DragStat().GetNow() - DragStat().GetPrev());
}

void SdrDragMove::ImpCheckSnap(const basegfx::B2DPoint& rPt)
{
    sal_uInt16 nWhichOrientationSnapped(0);
    const basegfx::B2DPoint aSnappedPoint(getSdrView().GetSnapPos(rPt, &nWhichOrientationSnapped));
    const basegfx::B2DVector aSnapVector(aSnappedPoint - rPt);

    if(nWhichOrientationSnapped & SDRSNAP_XSNAPPED)
    {
        if(mbXSnapped)
        {
            maBestSnap.setX(fabs(aSnapVector.getX()) < fabs(maBestSnap.getX())
                ? aSnapVector.getX()
                : maBestSnap.getX());
        }
        else
        {
            maBestSnap.setX(aSnapVector.getX());
            mbXSnapped = true;
        }
    }

    if(nWhichOrientationSnapped & SDRSNAP_YSNAPPED)
    {
        if(mbYSnapped)
        {
            maBestSnap.setY(fabs(aSnapVector.getY()) < fabs(maBestSnap.getY())
                ? aSnapVector.getY()
                : maBestSnap.getY());
        }
        else
        {
            maBestSnap.setY(aSnapVector.getY());
            mbYSnapped = true;
        }
    }
}

void SdrDragMove::MoveSdrDrag(const basegfx::B2DPoint& rPoint)
{
    if(DragStat().CheckMinMoved(rPoint))
    {
        maBestSnap.setX(0.0);
        maBestSnap.setY(0.0);
        mbXSnapped = mbYSnapped = false;

        const basegfx::B2DRange& rFixSnapRange(GetMarkedRange());
        basegfx::B2DRange aSnapRange(rFixSnapRange);
        aSnapRange.transform(basegfx::tools::createTranslateB2DHomMatrix(rPoint - DragStat().GetStart()));

        ImpCheckSnap(aSnapRange.getMinimum());

        if(!getSdrView().IsMoveSnapOnlyTopLeft())
        {
            ImpCheckSnap(basegfx::B2DPoint(aSnapRange.getMaxX(), aSnapRange.getMinY()));
            ImpCheckSnap(basegfx::B2DPoint(aSnapRange.getMinX(), aSnapRange.getMaxY()));
            ImpCheckSnap(aSnapRange.getMaximum());
        }

        basegfx::B2DPoint aNewPos(rPoint + maBestSnap);

        if(getSdrView().IsOrthogonal())
        {
            aNewPos = OrthoDistance8(DragStat().GetStart(), aNewPos, getSdrView().IsBigOrthogonal());
        }

        basegfx::B2DRange aWorkArea(getSdrView().GetWorkArea());
        const bool bWorkArea(!aWorkArea.isEmpty());
        const bool bDragLimit(IsDragLimit());

        if(bDragLimit || bWorkArea)
        {
            basegfx::B2DRange aSnapRange2(rFixSnapRange);
            const basegfx::B2DVector aDelta(aNewPos - DragStat().GetStart());

            if(bDragLimit)
            {
                const basegfx::B2DRange aDragLimit(getSdrView().GetDragLimit());

                if(bWorkArea)
                {
                    aWorkArea.intersect(aDragLimit);
                }
                else
                {
                    aWorkArea = aDragLimit;
                }
            }

            if(aSnapRange2.getMinX() > aWorkArea.getMinX() || aSnapRange2.getMaxX() < aWorkArea.getMaxX())
            {
                // ist ueberhaupt Platz zum verschieben?
                aSnapRange2.transform(basegfx::tools::createTranslateB2DHomMatrix(aDelta.getX(), 0.0));

                if(aSnapRange2.getMinX() < aWorkArea.getMinX())
                {
                    aNewPos.setX(aNewPos.getX() - (aSnapRange2.getMinX() - aWorkArea.getMinX()));
                }
                else if(aSnapRange2.getMaxX() > aWorkArea.getMaxX())
                {
                    aNewPos.setX(aNewPos.getX() - (aSnapRange2.getMaxX() - aWorkArea.getMaxX()));
                }
            }
            else
            {
                // kein Platz zum verschieben
                aNewPos.setX(DragStat().GetStart().getX());
            }

            if(aSnapRange2.getMinY() > aWorkArea.getMinY() || aSnapRange2.getMaxY() < aWorkArea.getMaxY())
            {
                // ist ueberhaupt Platz zum verschieben?
                aSnapRange2.transform(basegfx::tools::createTranslateB2DHomMatrix(0.0, aDelta.getY()));

                if(aSnapRange2.getMinY() < aWorkArea.getMinY())
                {
                    aNewPos.setY(aNewPos.getY() - (aSnapRange2.getMinY() - aWorkArea.getMinY()));
                }
                else if(aSnapRange2.getMaxY() > aWorkArea.getMaxY())
                {
                    aNewPos.setY(aNewPos.getY() - (aSnapRange2.getMaxY() - aWorkArea.getMaxY()));
                }
            }
            else
            {
                // kein Platz zum verschieben
                aNewPos.setY(DragStat().GetStart().getY());
            }
        }

        if(getSdrView().IsDraggingGluePoints() && getSdrView().areSdrObjectsSelected())
        {
            // limit delta move to possible GluePoint move possibilities
            const basegfx::B2DVector aOriginalAbsoluteDelta(aNewPos - DragStat().GetStart());
            basegfx::B2DVector aNewAbsoluteDelta(aOriginalAbsoluteDelta);
            const SdrObjectVector aSelection(getSdrView().getSelectedSdrObjectVectorFromSdrMarkView());

            for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size(); nMarkNum++)
            {
                const SdrObject* pObj = aSelection[nMarkNum];

                if(pObj)
                {
                    const sdr::selection::Indices rMarkedGluePoints = getSdrView().getSelectedGluesForSelectedSdrObject(*pObj);

                    if(rMarkedGluePoints.size())
                    {
                        const sdr::glue::GluePointProvider& rProvider = pObj->GetGluePointProvider();

                        if(rProvider.hasUserGluePoints())
                        {
                            // the SdrObject candidate with potentially moved GluePoints is identified. GetObjectMatrix,
                            // but take care for objects with zero width/height. Also prepare inverse transformation
                            const basegfx::B2DHomMatrix aCorrectedObjectTransformation(basegfx::tools::guaranteeMinimalScaling(pObj->getSdrObjectTransformation()));
                            basegfx::B2DHomMatrix aInverseCorrectedObjectTransformation(aCorrectedObjectTransformation);

                            aInverseCorrectedObjectTransformation.invert();

                            for(sdr::selection::Indices::const_iterator aCurrent(rMarkedGluePoints.begin()); aCurrent != rMarkedGluePoints.end(); aCurrent++)
                            {
                                const sal_uInt32 nId(*aCurrent);
                                const sdr::glue::GluePoint* pGlueCandidate = rProvider.findUserGluePointByID(nId);

                                if(pGlueCandidate)
                                {
                                    // get potentially moved position as absolute position and add absolute delta
                                    const basegfx::B2DPoint aAbsolutePosition(aCorrectedObjectTransformation * pGlueCandidate->getUnitPosition());
                                    basegfx::B2DPoint aClampedAbsolutePosition(aAbsolutePosition + aNewAbsoluteDelta);

                                    // calculate back to unit position of moved point, clamp that position in unit coordinates
                                    // to unit coordinates and convert back to absolute coordinates; this gives the clamped potentially
                                    // moved position
                                    aClampedAbsolutePosition = aInverseCorrectedObjectTransformation * aClampedAbsolutePosition;
                                    aClampedAbsolutePosition = basegfx::B2DRange::getUnitB2DRange().clamp(aClampedAbsolutePosition);
                                    aClampedAbsolutePosition = aCorrectedObjectTransformation * aClampedAbsolutePosition;

                                    // calculate the potentially changed delta move
                                    const basegfx::B2DVector aClampedDelta(aClampedAbsolutePosition - aAbsolutePosition);

                                    // prefer the new delta move vector when it's shorter than the original
                                    if(aClampedDelta.getLength() < aNewAbsoluteDelta.getLength())
                                    {
                                        aNewAbsoluteDelta = aClampedDelta;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    OSL_ENSURE(false, "ObjectSelection vector with illegal empty slots (!)");
                }
            }

            if(!aOriginalAbsoluteDelta.equal(aNewAbsoluteDelta))
            {
                aNewPos = aNewPos - aOriginalAbsoluteDelta + aNewAbsoluteDelta;
            }
        }

        if(getSdrView().IsOrthogonal())
        {
            aNewPos = OrthoDistance8(DragStat().GetStart(), aNewPos, false);
        }

        if(!aNewPos.equal(DragStat().GetNow()))
        {
            Hide();
            DragStat().NextMove(aNewPos);
            basegfx::B2DRange aActionRange(GetMarkedRange());
            aActionRange.transform(basegfx::tools::createTranslateB2DHomMatrix(DragStat().GetNow() - DragStat().GetPrev()));
            DragStat().SetActionRange(aActionRange);
            Show();
        }
    }
}

bool SdrDragMove::EndSdrDrag(bool bCopy)
{
    Hide();

    if(getSdrView().IsInsObjPoint() || (getSdrView().GetDragMethod() && getSdrView().IsInsertGluePoint()))
    {
        bCopy = false;
    }

    if(IsDraggingPoints())
    {
        getSdrView().TransformMarkedPoints(
            basegfx::tools::createTranslateB2DHomMatrix(DragStat().GetNow() - DragStat().GetPrev()),
            SDRREPFUNC_OBJ_MOVE,
            bCopy);
    }
    else if(IsDraggingGluePoints())
    {
        getSdrView().TransformMarkedGluePoints(
            basegfx::tools::createTranslateB2DHomMatrix(DragStat().GetNow() - DragStat().GetPrev()),
            SDRREPFUNC_OBJ_MOVE,
            bCopy);
    }
    else
    {
        getSdrView().MoveMarkedObj(DragStat().GetNow() - DragStat().GetPrev(), bCopy);
    }

    return true;
}

Pointer SdrDragMove::GetSdrDragPointer() const
{
    if(IsDraggingPoints() || IsDraggingGluePoints())
    {
        return Pointer(POINTER_MOVEPOINT);
    }
    else
    {
        return Pointer(POINTER_MOVE);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragResize::SdrDragResize(SdrView& rNewView)
:   SdrDragMethod(rNewView),
    maScale(1.0, 1.0)
{
}

void SdrDragResize::TakeSdrDragComment(XubString& rStr) const
{
    TakeMarkedDescriptionString(STR_DragMethResize, rStr);
    const basegfx::B2DVector aDelta(absolute(DragStat().GetStart() - DragStat().GetRef1()));
    bool bX(!basegfx::fTools::equal(maScale.getX(), 1.0) && aDelta.getX() > 1.0);
    bool bY(!basegfx::fTools::equal(maScale.getY(), 1.0) && aDelta.getY() > 1.0);

    if(bX || bY)
    {
        XubString aStr;
        bool bEqual(basegfx::fTools::equal(maScale.getX(), maScale.getY()));

        rStr.AppendAscii(" (");

        if(bX)
        {
            if(!bEqual)
            {
                rStr.AppendAscii("x=");
            }

            getSdrView().getSdrModelFromSdrView().TakePercentStr(maScale.getX(), aStr);
            rStr += aStr;
        }

        if(bY && !bEqual)
        {
            if(bX)
            {
                rStr += sal_Unicode(' ');
            }

            rStr.AppendAscii("y=");
            getSdrView().getSdrModelFromSdrView().TakePercentStr(maScale.getY(), aStr);
            rStr += aStr;
        }

        rStr += sal_Unicode(')');
    }

    if(getSdrView().IsDragWithCopy())
    {
        rStr += ImpGetResStr(STR_EditWithCopy);
    }
}

bool SdrDragResize::BeginSdrDrag()
{
    SdrHdlKind eRefHdl(HDL_MOVE);
    SdrHdl* pRefHdl = 0;

    switch(GetDragHdlKind())
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

    if(HDL_MOVE != eRefHdl)
    {
        pRefHdl = GetHdlList().GetHdlByKind(eRefHdl);
    }

    if(pRefHdl && !getSdrView().IsResizeAtCenter())
    {
        DragStat().SetRef1(pRefHdl->getPosition());
    }
    else
    {
        SdrHdl* pRef1 = GetHdlList().GetHdlByKind(HDL_UPLFT);
        SdrHdl* pRef2 = GetHdlList().GetHdlByKind(HDL_LWRGT);

        if(pRef1 && pRef2)
        {
            const basegfx::B2DRange aRefRange(pRef1->getPosition(), pRef2->getPosition());

            DragStat().SetRef1(aRefRange.getCenter());
        }
        else
        {
            DragStat().SetRef1(GetMarkedRange().getCenter());
        }
    }

    Show();

    return true;
}

basegfx::B2DHomMatrix SdrDragResize::getCurrentTransformation()
{
    basegfx::B2DHomMatrix aRetval;

    aRetval.translate(-DragStat().GetRef1());
    aRetval.scale(maScale);
    aRetval.translate(DragStat().GetRef1());

    return aRetval;
}

void SdrDragResize::MoveSdrDrag(const basegfx::B2DPoint& rNoSnapPnt)
{
    basegfx::B2DPoint aNewPos(getSdrView().GetSnapPos(rNoSnapPnt));
    double fMaximumLimit(DBL_MAX);
    const basegfx::B2DPoint& rRef1(DragStat().GetRef1());
    basegfx::B2DRange aWorkArea(getSdrView().GetWorkArea());
    const bool bWorkArea(!aWorkArea.isEmpty());
    const bool bDragLimit(IsDragLimit());
    const bool bOrtho(getSdrView().IsOrthogonal() || !getSdrView().IsResizeAllowed(false));

    if(bDragLimit || bWorkArea)
    {
        const basegfx::B2DRange aSnapRange(GetMarkedRange());

        if(bDragLimit)
        {
            const basegfx::B2DRange aDragLimit(getSdrView().GetDragLimit());

            if(bWorkArea)
            {
                aWorkArea.intersect(aDragLimit);
            }
            else
            {
                aWorkArea = aDragLimit;
            }
        }

        aNewPos = aWorkArea.clamp(aNewPos);

        if(bOrtho)
        {
            if(rRef1.getX() > aSnapRange.getMinX())
            {
                const double fDividend(rRef1.getX() - aWorkArea.getMinX());
                const double fDivisor(rRef1.getX() - aSnapRange.getMinX());
                const double fMax(fDividend / (basegfx::fTools::equalZero(fDivisor) ? 1.0 : fDivisor));

                fMaximumLimit = std::min(fMax, fabs(fMaximumLimit));
            }

            if (rRef1.getX()<aSnapRange.getMaxX())
            {
                const double fDividend(aWorkArea.getMaxX() - rRef1.getX());
                const double fDivisor(aSnapRange.getMaxX() - rRef1.getX());
                const double fMax(fDividend / (basegfx::fTools::equalZero(fDivisor) ? 1.0 : fDivisor));

                fMaximumLimit = std::min(fMax, fabs(fMaximumLimit));
            }

            if (rRef1.getY()>aSnapRange.getMinY())
            {
                const double fDividend(rRef1.getY() - aWorkArea.getMinY());
                const double fDivisor(rRef1.getY() - aSnapRange.getMinY());
                const double fMax(fDividend / (basegfx::fTools::equalZero(fDivisor) ? 1.0 : fDivisor));

                fMaximumLimit = std::min(fMax, fabs(fMaximumLimit));
            }

            if (rRef1.getY()<aSnapRange.getMaxY())
            {
                const double fDividend(aWorkArea.getMaxY() - rRef1.getY());
                const double fDivisor(aSnapRange.getMaxY() - rRef1.getY());
                const double fMax(fDividend / (basegfx::fTools::equalZero(fDivisor) ? 1.0 : fDivisor));

                fMaximumLimit = std::min(fMax, fabs(fMaximumLimit));
            }
        }
    }

    const basegfx::B2DPoint aOldDelta(DragStat().GetStart() - rRef1);
    const basegfx::B2DPoint aNewDelta(aNewPos - rRef1);
    double fXFactor(aNewDelta.getX() / (basegfx::fTools::equalZero(aOldDelta.getX()) ? 1.0 : aOldDelta.getX()));
    double fYFactor(aNewDelta.getY() / (basegfx::fTools::equalZero(aOldDelta.getY()) ? 1.0 : aOldDelta.getY()));

    if(!DragStat().IsHorFixed() && !DragStat().IsVerFixed())
    {
        if(bOrtho)
        {
            if(basegfx::fTools::more(fabs(fXFactor), fabs(fYFactor)) != getSdrView().IsBigOrthogonal())
            {
                fXFactor = basegfx::copySign(fYFactor, fXFactor);
            }
            else
            {
                fYFactor = basegfx::copySign(fXFactor, fYFactor);
            }
        }
    }
    else
    {
        if(bOrtho)
        {
            if(DragStat().IsHorFixed())
            {
                fXFactor = fabs(fYFactor);
            }

            if(DragStat().IsVerFixed())
            {
                fYFactor = fabs(fXFactor);
            }
        }
        else
        {
            if(DragStat().IsHorFixed())
            {
                fXFactor = 1.0;
            }

            if (DragStat().IsVerFixed())
            {
                fYFactor = 1.0;
            }
        }
    }

    if(bOrtho)
    {
        if(fabs(fXFactor) > fMaximumLimit || fabs(fYFactor) > fMaximumLimit)
        {
            fXFactor = (fXFactor > 0.0) ? fMaximumLimit : -fMaximumLimit;
            fYFactor = (fYFactor > 0.0) ? fMaximumLimit : -fMaximumLimit;
        }
    }

    if(DragStat().CheckMinMoved(aNewPos))
    {
        if((!DragStat().IsHorFixed() && !basegfx::fTools::equal(aNewPos.getX(), DragStat().GetNow().getX())) ||
            (!DragStat().IsVerFixed() && !basegfx::fTools::equal(aNewPos.getY(), DragStat().GetNow().getY())))
        {
            Hide();
            DragStat().NextMove(aNewPos);
            maScale.setX(fXFactor);
            maScale.setY(fYFactor);
            Show();
        }
    }
}

bool SdrDragResize::EndSdrDrag(bool bCopy)
{
    Hide();

    if(IsDraggingPoints())
    {
        basegfx::B2DHomMatrix aTransform;

        aTransform.translate(-DragStat().GetRef1());
        aTransform.scale(maScale);
        aTransform.translate(DragStat().GetRef1());

        getSdrView().TransformMarkedPoints(
            aTransform,
            SDRREPFUNC_OBJ_RESIZE,
            bCopy);
    }
    else if(IsDraggingGluePoints())
    {
        basegfx::B2DHomMatrix aTransform;

        aTransform.translate(-DragStat().GetRef1());
        aTransform.scale(maScale);
        aTransform.translate(DragStat().GetRef1());

        getSdrView().TransformMarkedGluePoints(
            aTransform,
            SDRREPFUNC_OBJ_MOVE,
            bCopy);
    }
    else
    {
        getSdrView().ResizeMarkedObj(DragStat().GetRef1(), maScale, bCopy);
    }

    return true;
}

Pointer SdrDragResize::GetSdrDragPointer() const
{
    const SdrHdl* pHdl = GetDragHdl();

    if(pHdl)
    {
        return pHdl->GetPointer();
    }

    return Pointer(POINTER_MOVE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragRotate::SdrDragRotate(SdrView& rNewView)
:   SdrDragMethod(rNewView),
    mfStartRotation(0.0),
    mfDeltaRotation(0.0)
{
}

void SdrDragRotate::TakeSdrDragComment(XubString& rStr) const
{
    TakeMarkedDescriptionString(STR_DragMethRotate, rStr);
    rStr.AppendAscii(" (");
    XubString aStr;
    const double fSnappedDelta(basegfx::snapToRange(mfDeltaRotation, -F_2PI, F_2PI));
    sal_Int32 nOldAngle(basegfx::fround((fSnappedDelta * -18000.0) / F_PI) % 36000);

    getSdrView().getSdrModelFromSdrView().TakeWinkStr(nOldAngle, aStr);
    rStr += aStr;
    rStr += sal_Unicode(')');

    if(getSdrView().IsDragWithCopy())
    {
        rStr += ImpGetResStr(STR_EditWithCopy);
    }
}

bool SdrDragRotate::BeginSdrDrag()
{
    SdrHdl* pH = GetHdlList().GetHdlByKind(HDL_REF1);

    if(pH)
    {
        Show();
        DragStat().SetRef1(pH->getPosition());
        const basegfx::B2DVector aDirection(DragStat().GetStart() - DragStat().GetRef1());
        mfStartRotation = atan2(aDirection.getY(), aDirection.getX());
        return true;
    }
    else
    {
        DBG_ERROR("SdrDragRotate::BeginSdrDrag(): Kein Referenzpunkt-Handle gefunden");
        return false;
    }
}

basegfx::B2DHomMatrix SdrDragRotate::getCurrentTransformation()
{
    return basegfx::tools::createRotateAroundPoint(DragStat().GetRef1(), mfDeltaRotation);
}

void SdrDragRotate::MoveSdrDrag(const basegfx::B2DPoint& rPoint)
{
    if(DragStat().CheckMinMoved(rPoint))
    {
        const basegfx::B2DVector aDirection(rPoint - DragStat().GetRef1());
        double fNewDeltaRotation(atan2(aDirection.getY(), aDirection.getX()) - mfStartRotation);

        if(!basegfx::fTools::equalZero(fNewDeltaRotation))
        {
            double fSnapAngle(0.0);

            if(getSdrView().IsAngleSnapEnabled())
            {
                fSnapAngle = ((getSdrView().GetSnapAngle() % 36000) * F_PI) / 18000.0;
            }

            if(!getSdrView().IsRotateAllowed(false))
            {
                fSnapAngle = F_PI2;
            }

            if(!basegfx::fTools::equalZero(fSnapAngle))
            {
                fNewDeltaRotation = basegfx::snapToNearestMultiple(fNewDeltaRotation, fSnapAngle);
            }

            if(!basegfx::fTools::equal(fNewDeltaRotation, mfDeltaRotation))
            {
                mfDeltaRotation = fNewDeltaRotation;

                Hide();
                DragStat().NextMove(rPoint);
                Show();
            }
        }
    }
}

bool SdrDragRotate::EndSdrDrag(bool bCopy)
{
    Hide();

    if(!basegfx::fTools::equalZero(mfDeltaRotation))
    {
        if(IsDraggingPoints())
        {
            getSdrView().TransformMarkedPoints(
                basegfx::tools::createRotateAroundPoint(DragStat().GetRef1(), mfDeltaRotation),
                SDRREPFUNC_OBJ_ROTATE,
                bCopy);
        }
        else if(IsDraggingGluePoints())
        {
            getSdrView().TransformMarkedGluePoints(
                basegfx::tools::createRotateAroundPoint(DragStat().GetRef1(), mfDeltaRotation),
                SDRREPFUNC_OBJ_ROTATE,
                bCopy);
        }
        else
        {
            getSdrView().RotateMarkedObj(DragStat().GetRef1(), mfDeltaRotation, bCopy);
        }
    }
    return true;
}

Pointer SdrDragRotate::GetSdrDragPointer() const
{
    return Pointer(POINTER_ROTATE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragShear::SdrDragShear(SdrView& rNewView)
:   SdrDragMethod(rNewView),
    mfStartAngle(0.0),
    mfDeltaAngle(0.0),
    mfFactor(1.0),
    mbVertical(false)
{
}

void SdrDragShear::TakeSdrDragComment(XubString& rStr) const
{
    TakeMarkedDescriptionString(STR_DragMethShear, rStr);
    sal_Int32 nOldAngle(basegfx::fround((mfDeltaAngle * -18000.0) / F_PI) % 36000);
    XubString aStr;

    if(nOldAngle > 18000)
    {
        nOldAngle = 36000 - nOldAngle;
    }

    rStr.AppendAscii(" (");
    getSdrView().getSdrModelFromSdrView().TakeWinkStr(nOldAngle, aStr);
    rStr += aStr;
    rStr += sal_Unicode(')');

    if(getSdrView().IsDragWithCopy())
    {
        rStr += ImpGetResStr(STR_EditWithCopy);
    }
}

bool SdrDragShear::BeginSdrDrag()
{
    SdrHdlKind eRefHdl(HDL_MOVE);
    SdrHdl* pRefHdl = 0;

    switch(GetDragHdlKind())
    {
        case HDL_UPPER: eRefHdl = HDL_LOWER; break;
        case HDL_LOWER: eRefHdl = HDL_UPPER; break;
        case HDL_LEFT : eRefHdl = HDL_RIGHT; mbVertical = true; break;
        case HDL_RIGHT: eRefHdl = HDL_LEFT; mbVertical = true; break;
        default: break;
    }

    if(HDL_MOVE != eRefHdl)
    {
        pRefHdl = GetHdlList().GetHdlByKind(eRefHdl);
    }

    if(pRefHdl)
    {
        DragStat().SetRef1(pRefHdl->getPosition());

        const basegfx::B2DVector aDirection(DragStat().GetStart() - DragStat().GetRef1());
        mfStartAngle = atan2(aDirection.getY(), aDirection.getX());
    }
    else
    {
        DBG_ERROR("SdrDragShear::BeginSdrDrag(): Kein Referenzpunkt-Handle fuer Shear gefunden");

        return false;
    }

    Show();

    return true;
}

basegfx::B2DHomMatrix SdrDragShear::getCurrentTransformation()
{
    basegfx::B2DHomMatrix aRetval;
    const bool bResizeUsed(!basegfx::fTools::equal(mfFactor, 1.0));
    const bool bShearUsed(!basegfx::fTools::equalZero(mfDeltaAngle));

    if(bResizeUsed || bShearUsed)
    {
        aRetval.translate(-DragStat().GetRef1());

        if(bResizeUsed)
        {
            if(mbVertical)
            {
                aRetval.scale(mfFactor, 1.0);
            }
            else
            {
                aRetval.scale(1.0, mfFactor);
            }
        }

        if(bShearUsed)
        {
            if(mbVertical)
            {
                aRetval.shearY(tan(mfDeltaAngle));
            }
            else
            {
                aRetval.shearX(tan(mfDeltaAngle));
            }
        }

        aRetval.translate(DragStat().GetRef1());
    }

    return aRetval;
}

void SdrDragShear::MoveSdrDrag(const basegfx::B2DPoint& rPnt)
{
    if(DragStat().CheckMinMoved(rPnt))
    {
        const basegfx::B2DVector aDirection(rPnt - DragStat().GetRef1());
        double fNewDeltaAngle(0.0);

        if(mbVertical)
        {
            fNewDeltaAngle = atan2(aDirection.getY(), aDirection.getX()) - mfStartAngle;
        }
        else
        {
            fNewDeltaAngle = mfStartAngle - atan2(aDirection.getY(), aDirection.getX());
        }

        fNewDeltaAngle = basegfx::snapToRange(fNewDeltaAngle, -F_PI, F_PI);

        if(getSdrView().IsAngleSnapEnabled())
        {
            const double fSnapAngle(((getSdrView().GetSnapAngle() % 36000) * F_PI) / 18000.0);

            fNewDeltaAngle = basegfx::snapToNearestMultiple(fNewDeltaAngle, fSnapAngle);
        }

        // calc resize for slant
        double fNewFactor(cos(fNewDeltaAngle));

        if(!basegfx::fTools::equal(mfDeltaAngle, fNewDeltaAngle) || !basegfx::fTools::equal(mfFactor, fNewFactor))
        {
            mfDeltaAngle = fNewDeltaAngle;
            mfFactor = fNewFactor;

            Hide();
            DragStat().NextMove(rPnt);
            Show();
        }
    }
}

bool SdrDragShear::EndSdrDrag(bool bCopy)
{
    Hide();

    const bool bResizeUsed(!basegfx::fTools::equal(mfFactor, 1.0));
    const bool bShearUsed(!basegfx::fTools::equalZero(mfDeltaAngle));

    if(bResizeUsed || bShearUsed)
    {
        XubString aStr;
        TakeMarkedDescriptionString(STR_EditShear, aStr);

        if(bCopy)
        {
            aStr += ImpGetResStr(STR_EditWithCopy);
        }

        getSdrView().BegUndo(aStr);

        if(bResizeUsed)
        {
            if(mbVertical)
            {
                getSdrView().ResizeMarkedObj(DragStat().GetRef1(), basegfx::B2DVector(mfFactor, 1.0), bCopy);
            }
            else
            {
                getSdrView().ResizeMarkedObj(DragStat().GetRef1(), basegfx::B2DVector(1.0, mfFactor), bCopy);
            }

            bCopy = false;
        }

        if(bShearUsed)
        {
            getSdrView().ShearMarkedObj(DragStat().GetRef1(), mfDeltaAngle, mbVertical, bCopy);
        }

        getSdrView().EndUndo();

        return true;
    }

    return false;
}

Pointer SdrDragShear::GetSdrDragPointer() const
{
    if(mbVertical)
    {
        return Pointer(POINTER_VSHEAR);
    }
    else
    {
        return Pointer(POINTER_HSHEAR);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragMirror::SdrDragMirror(SdrView& rNewView)
:   SdrDragMethod(rNewView)
{
}

void SdrDragMirror::TakeSdrDragComment(XubString& rStr) const
{
    SdrHdl* pH1 = GetHdlList().GetHdlByKind(HDL_REF1);
    SdrHdl* pH2 = GetHdlList().GetHdlByKind(HDL_REF2);

    if(pH1 && pH2)
    {
        const basegfx::B2DVector aDelta(pH2->getPosition() - pH1->getPosition());
        const bool bHorizontal(basegfx::fTools::equalZero(aDelta.getX()));
        const bool bVertical(basegfx::fTools::equalZero(aDelta.getY()));
        const bool b90(bHorizontal || bVertical);
        const bool b45(b90 || basegfx::fTools::equal(fabs(aDelta.getX()), fabs(aDelta.getY())));

        if(bHorizontal)
        {
            TakeMarkedDescriptionString(STR_DragMethMirrorHori, rStr);
        }
        else if(bVertical)
        {
            TakeMarkedDescriptionString(STR_DragMethMirrorVert, rStr);
        }
        else if(b45)
        {
            TakeMarkedDescriptionString(STR_DragMethMirrorDiag, rStr);
        }
        else
        {
            TakeMarkedDescriptionString(STR_DragMethMirrorFree, rStr);
        }

        if(getSdrView().IsDragWithCopy())
        {
            rStr += ImpGetResStr(STR_EditWithCopy);
        }
    }
}

bool SdrDragMirror::BeginSdrDrag()
{
    SdrHdl* pH1 = GetHdlList().GetHdlByKind(HDL_REF1);
    SdrHdl* pH2 = GetHdlList().GetHdlByKind(HDL_REF2);

    if(pH1 && pH2)
    {
        DragStat().SetRef1(pH1->getPosition());
        DragStat().SetRef2(pH2->getPosition());
        SetRef1(pH1->getPosition());
        SetRef2(pH2->getPosition());

        const basegfx::B2DVector aDelta(pH2->getPosition() - pH1->getPosition());
        const bool bHorizontal(basegfx::fTools::equalZero(aDelta.getX()));
        const bool bVertical(basegfx::fTools::equalZero(aDelta.getY()));
        const bool b90(bHorizontal || bVertical);
        const bool b45(b90 || basegfx::fTools::equal(fabs(aDelta.getX()), fabs(aDelta.getY())));

        if(!getSdrView().IsMirrorAllowed(false, false) && !b45)
        {
            // freier Achsenwinkel nicht erlaubt
            return false;
        }

        if(!getSdrView().IsMirrorAllowed(true, false) && !b90)
        {
            // 45deg auch nicht erlaubt
            return false;
        }

        Show();

        return true;
    }
    else
    {
        DBG_ERROR("SdrDragMirror::BeginSdrDrag(): Spiegelachse nicht gefunden");

        return false;
    }
}

basegfx::B2DHomMatrix SdrDragMirror::getCurrentTransformation()
{
    basegfx::B2DHomMatrix aRetval;
    const bool bMirrored(!basegfx::tools::arePointsOnSameSideOfLine(GetRef1(), GetRef2(), DragStat().GetStart(), DragStat().GetNow()));

    if(bMirrored)
    {
        const basegfx::B2DVector aDelta(DragStat().GetRef2() - DragStat().GetRef1());
        const double fRotation(atan2(aDelta.getY(), aDelta.getX()));

        aRetval.translate(-DragStat().GetRef1());
        aRetval.rotate(-fRotation);
        aRetval.scale(1.0, -1.0);
        aRetval.rotate(fRotation);
        aRetval.translate(DragStat().GetRef1());
    }

    return aRetval;
}

void SdrDragMirror::MoveSdrDrag(const basegfx::B2DPoint& rPnt)
{
    if(DragStat().CheckMinMoved(rPnt))
    {
        const bool bMirroredOld(!basegfx::tools::arePointsOnSameSideOfLine(GetRef1(), GetRef2(), DragStat().GetStart(), DragStat().GetNow()));
        const bool bMirroredNew(!basegfx::tools::arePointsOnSameSideOfLine(GetRef1(), GetRef2(), DragStat().GetStart(), rPnt));

        if(bMirroredOld != bMirroredNew)
        {
            Hide();
            DragStat().NextMove(rPnt);
            Show();
        }
    }
}

bool SdrDragMirror::EndSdrDrag(bool bCopy)
{
    Hide();

    const bool bMirrored(!basegfx::tools::arePointsOnSameSideOfLine(GetRef1(), GetRef2(), DragStat().GetStart(), DragStat().GetNow()));

    if(bMirrored)
    {
        getSdrView().MirrorMarkedObj(DragStat().GetRef1(), DragStat().GetRef2(), bCopy);
    }

    return true;
}

Pointer SdrDragMirror::GetSdrDragPointer() const
{
    return Pointer(POINTER_MIRROR);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragGradient::SdrDragGradient(SdrView& rNewView, bool bGrad)
:   SdrDragMethod(rNewView),
    pIAOHandle(0),
    bIsGradient(bGrad)
{
}

void SdrDragGradient::TakeSdrDragComment(XubString& rStr) const
{
    if(IsGradient())
    {
        TakeMarkedDescriptionString(STR_DragMethGradient, rStr);
    }
    else
    {
        TakeMarkedDescriptionString(STR_DragMethTransparence, rStr);
    }
}

bool SdrDragGradient::BeginSdrDrag()
{
    bool bRetval(false);
    pIAOHandle = dynamic_cast< SdrHdlGradient* >(GetHdlList().GetHdlByKind(IsGradient() ? HDL_GRAD : HDL_TRNS));

    if(pIAOHandle)
    {
        // save old values
        DragStat().SetRef1(pIAOHandle->getPosition());
        DragStat().SetRef2(pIAOHandle->get2ndPosition());

        // what was hit?
        bool bHit(false);

        // init handling flags
        pIAOHandle->SetMoveSingleHandle(false);
        pIAOHandle->SetMoveFirstHandle(false);

        // test first color handle
        if(pIAOHandle->getColHdl1().getOverlayObjectList().isHitLogic(DragStat().GetStart()))
        {
            bHit = true;
            pIAOHandle->SetMoveSingleHandle(true);
            pIAOHandle->SetMoveFirstHandle(true);
        }

        // test second color handle
        if(!bHit && pIAOHandle->getColHdl2().getOverlayObjectList().isHitLogic(DragStat().GetStart()))
        {
            bHit = true;
            pIAOHandle->SetMoveSingleHandle(true);
        }

        // test gradient handle itself
        if(!bHit && pIAOHandle->getOverlayObjectList().isHitLogic(DragStat().GetStart()))
        {
            bHit = true;
        }

        // everything up and running :o}
        bRetval = bHit;
    }
    else
    {
        DBG_ERROR("SdrDragGradient::BeginSdrDrag(): IAOGradient nicht gefunden");
    }

    return bRetval;
}

void SdrDragGradient::MoveSdrDrag(const basegfx::B2DPoint& rPnt)
{
    if(pIAOHandle && DragStat().CheckMinMoved(rPnt))
    {
        DragStat().NextMove(rPnt);
        const basegfx::B2DVector aMoveDiff(rPnt - DragStat().GetStart());

        if(pIAOHandle->IsMoveSingleHandle())
        {
            if(pIAOHandle->IsMoveFirstHandle())
            {
                pIAOHandle->setPosition(DragStat().GetRef1() + aMoveDiff);
            }
            else
            {
                pIAOHandle->set2ndPosition(DragStat().GetRef2() + aMoveDiff);
            }
        }
        else
        {
            pIAOHandle->setPosition(DragStat().GetRef1() + aMoveDiff);
            pIAOHandle->set2ndPosition(DragStat().GetRef2() + aMoveDiff);
        }

        // new state
        pIAOHandle->FromIAOToItem(false, false);
    }
}

bool SdrDragGradient::EndSdrDrag(bool /*bCopy*/)
{
    // here the result is clear, do something with the values
    SetRef1(pIAOHandle->getColHdl1().getPosition());
    SetRef2(pIAOHandle->getColHdl2().getPosition());

    // new state
    pIAOHandle->FromIAOToItem(true, true);

    return true;
}

void SdrDragGradient::CancelSdrDrag()
{
    // restore old values
    pIAOHandle->setPosition(DragStat().GetRef1());
    pIAOHandle->set2ndPosition(DragStat().GetRef2());

    // new state
    pIAOHandle->FromIAOToItem(true, false);
}

Pointer SdrDragGradient::GetSdrDragPointer() const
{
    return Pointer(POINTER_REFHAND);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragCrook::SdrDragCrook(SdrView& rNewView)
:   SdrDragMethod(rNewView),
    maMarkedRange(),
    maMarkedCenter(0.0, 0.0),
    maCenter(0.0, 0.0),
    maStart(0.0, 0.0),
    maRadius(0.0, 0.0),
    mfFactor(1.0),
    mfAngle(0.0),
    mfMarkedSize(0.0),
    meMode(SDRCROOK_ROTATE),
    mbContortionAllowed(false),
    mbNoContortionAllowed(false),
    mbContortion(false),
    mbResizeAllowed(false),
    mbResize(false),
    mbRotateAllowed(false),
    mbRotate(false),
    mbVertical(false),
    mbValid(false),
    mbLeft(false),
    mbRight(false),
    mbTop(false),
    mbBottom(false),
    mbAtCenter(false)
{
}

void SdrDragCrook::TakeSdrDragComment(XubString& rStr) const
{
    TakeMarkedDescriptionString(!mbContortion ? STR_DragMethCrook : STR_DragMethCrookContortion, rStr);

    if(mbValid)
    {
        rStr.AppendAscii(" (");

        XubString aStr;
        sal_Int32 nVal(basegfx::fround(mfAngle * (18000.0 / F_PI)) % 36000);

        if(nVal < 0)
        {
            nVal += 36000;
        }

        if(mbAtCenter)
        {
            nVal *= 2;
        }

        nVal = Abs(nVal);
        getSdrView().getSdrModelFromSdrView().TakeWinkStr(nVal, aStr);
        rStr += aStr;
        rStr += sal_Unicode(')');
    }

    if(getSdrView().IsDragWithCopy())
        rStr += ImpGetResStr(STR_EditWithCopy);
}

// #96920# These defines parametrise the created raster
// for interactions
#define DRAG_CROOK_RASTER_MINIMUM   (4)
#define DRAG_CROOK_RASTER_MAXIMUM   (15)
#define DRAG_CROOK_RASTER_DISTANCE  (30)

basegfx::B2DPolyPolygon impCreateDragRaster(SdrPageView& rPageView, const basegfx::B2DRange& rMarkRange)
{
    basegfx::B2DPolyPolygon aRetval;

    if(rPageView.PageWindowCount())
    {
        OutputDevice& rOut = (rPageView.GetPageWindow(0)->GetPaintWindow().GetOutputDevice());
        const basegfx::B2DRange aDiscreteRange(rOut.GetViewTransformation() * rMarkRange);
        sal_uInt32 nHorDiv(basegfx::fround(aDiscreteRange.getWidth() / DRAG_CROOK_RASTER_DISTANCE));
        sal_uInt32 nVerDiv(basegfx::fround(aDiscreteRange.getHeight() / DRAG_CROOK_RASTER_DISTANCE));

        if(nHorDiv > DRAG_CROOK_RASTER_MAXIMUM)
        {
            nHorDiv = DRAG_CROOK_RASTER_MAXIMUM;
        }

        if(nHorDiv < DRAG_CROOK_RASTER_MINIMUM)
        {
            nHorDiv = DRAG_CROOK_RASTER_MINIMUM;
        }

        if(nVerDiv > DRAG_CROOK_RASTER_MAXIMUM)
        {
            nVerDiv = DRAG_CROOK_RASTER_MAXIMUM;
        }

        if(nVerDiv < DRAG_CROOK_RASTER_MINIMUM)
        {
            nVerDiv = DRAG_CROOK_RASTER_MINIMUM;
        }

        const double fXLen(rMarkRange.getWidth() / (double)nHorDiv);
        const double fYLen(rMarkRange.getHeight() / (double)nVerDiv);
        double fYPos(rMarkRange.getMinY());
        sal_uInt32 a, b;

        for(a = 0; a <= nVerDiv; a++)
        {
            // hor lines
            for(b = 0; b < nHorDiv; b++)
            {
                basegfx::B2DPolygon aHorLineSegment;

                const double fNewX(rMarkRange.getMinX() + (b * fXLen));
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

        double fXPos(rMarkRange.getMinX());

        for(a = 0; a <= nHorDiv; a++)
        {
            // ver lines
            for(b = 0; b < nVerDiv; b++)
            {
                basegfx::B2DPolygon aVerLineSegment;

                const double fNewY(rMarkRange.getMinY() + (b * fYLen));
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
    if(getSdrView().GetSdrPageView())
    {
        const basegfx::B2DPolyPolygon aDragRaster(impCreateDragRaster(*getSdrView().GetSdrPageView(), GetMarkedRange()));

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
    mbContortionAllowed = getSdrView().IsCrookAllowed(false);
    mbNoContortionAllowed = getSdrView().IsCrookAllowed(true);
    mbResizeAllowed = getSdrView().IsResizeAllowed(false);
    mbRotateAllowed = getSdrView().IsRotateAllowed(false);

    if(mbContortionAllowed || mbNoContortionAllowed)
    {
        mbVertical = (HDL_LOWER == GetDragHdlKind() || HDL_UPPER == GetDragHdlKind());
        maMarkedRange = GetMarkedRange();
        maMarkedCenter = maMarkedRange.getCenter();
        mfMarkedSize = mbVertical ? maMarkedRange.getHeight() : maMarkedRange.getWidth();
        maCenter = maMarkedCenter;
        maStart = DragStat().GetStart();
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
    const sal_uInt32 nPolyAnz(rTarget.count());

    if(!mbContortion && !getSdrView().IsNoDragXorPolys())
    {
        sal_uInt32 n1st(0);
        sal_uInt32 nLast(0);
        basegfx::B2DPoint aCenter(maCenter);

        while(n1st < nPolyAnz)
        {
            nLast = n1st;

            while(nLast < nPolyAnz && rTarget.getB2DPolygon(nLast).count())
            {
                nLast++;
            }

            basegfx::B2DRange aBound;
            sal_uInt32 i(0);

            for(i = n1st; i < nLast; i++)
            {
                aBound.expand(rTarget.getB2DPolygon(i).getB2DRange());
            }

            basegfx::B2DPoint aCtr0(aBound.getCenter());
            basegfx::B2DPoint aCtr1(aCtr0);

            if(mbResize)
            {
                Fraction aFact1(1,1);

                if (mbVertical)
                {
                    aCtr1.setY(aCenter.getY() + ((aCtr1.getY() - aCenter.getY()) * mfFactor));
                }
                else
                {
                    aCtr1.setX(aCenter.getX() + ((aCtr1.getX() - aCenter.getX()) * mfFactor));
                }
            }

            bool bRotOk(false);
            double fSinus(0.0), fCosinus(0.0);

            if(!maRadius.equalZero())
            {
                bRotOk = mbRotate;

                switch(meMode)
                {
                    case SDRCROOK_ROTATE : CrookRotateXPoint(aCtr1, 0, 0, aCenter, maRadius, fSinus, fCosinus, mbVertical); break;
                    case SDRCROOK_SLANT : CrookSlantXPoint(aCtr1, 0, 0, aCenter, maRadius, fSinus, fCosinus, mbVertical); break;
                    case SDRCROOK_STRETCH: CrookStretchXPoint(aCtr1, 0, 0, aCenter, maRadius, fSinus, fCosinus, mbVertical, maMarkedRange); break;
                }
            }

            aCtr1 -= aCtr0;

            for(i = n1st; i < nLast; i++)
            {
                basegfx::B2DHomMatrix aTransform;

                if(bRotOk)
                {
                    aTransform.translate(-aCtr0);
                    aTransform.rotate(atan2(fSinus, fCosinus));
                    aTransform.translate(aCtr0);
                }

                aTransform.translate(aCtr1);

                basegfx::B2DPolygon aPartialTarget(rTarget.getB2DPolygon(i));

                aPartialTarget.transform(aTransform);
                rTarget.setB2DPolygon(i, aPartialTarget);
            }

            n1st = nLast + 1;
        }
    }
    else
    {
        for(sal_uInt32 j(0); j < nPolyAnz; j++)
        {
            basegfx::B2DPolygon aPol(rTarget.getB2DPolygon(j));
            const sal_uInt32 nPtAnz(aPol.count());
            const bool bIsCurve(aPol.areControlPointsUsed());

            for(sal_uInt32 i(0); i < nPtAnz; i++)
            {
                basegfx::B2DPoint aPnt(aPol.getB2DPoint(i));

                if(bIsCurve && (aPol.isNextControlPointUsed(i) || aPol.isPrevControlPointUsed(i)))
                {
                    basegfx::B2DPoint aPrev(aPol.getPrevControlPoint(i));
                    basegfx::B2DPoint aNext(aPol.getNextControlPoint(i));

                    _MovCrookPoint(aPnt, &aPrev, &aNext);

                    aPol.setB2DPoint(i, aPnt);
                    aPol.setControlPoints(i, aPrev, aNext);
                }
                else
                {
                    _MovCrookPoint(aPnt, 0, 0);
                    aPol.setB2DPoint(i, aPnt);
                }
            }

            rTarget.setB2DPolygon(j, aPol);
        }
    }
}

void SdrDragCrook::_MovCrookPoint(basegfx::B2DPoint& rPnt, basegfx::B2DPoint* pC1, basegfx::B2DPoint* pC2)
{
    bool bVert(mbVertical);
    basegfx::B2DPoint aCenter(maCenter);

    if(mbResize)
    {
        if(bVert)
        {
            rPnt.setY(aCenter.getY() + (rPnt.getY() - aCenter.getY()) * mfFactor);

            if(pC1)
            {
                pC1->setY(aCenter.getY() + (pC1->getY() - aCenter.getY()) * mfFactor);
            }

            if(pC2)
            {
                pC2->setY(aCenter.getY() + (pC2->getY() - aCenter.getY()) * mfFactor);
            }
        }
        else
        {
            rPnt.setX(aCenter.getX() + (rPnt.getX() - aCenter.getX()) * mfFactor);

            if(pC1)
            {
                pC1->setX(aCenter.getX() + (pC1->getX() - aCenter.getX()) * mfFactor);
            }

            if(pC2)
            {
                pC2->setX(aCenter.getX() + (pC2->getX() - aCenter.getX()) * mfFactor);
            }
        }
    }

    if(!maRadius.equalZero())
    {
        double nSin, nCos;

        switch(meMode)
        {
            case SDRCROOK_ROTATE : CrookRotateXPoint(rPnt, pC1, pC2, aCenter, maRadius, nSin, nCos, bVert); break;
            case SDRCROOK_SLANT : CrookSlantXPoint(rPnt, pC1, pC2, aCenter, maRadius, nSin, nCos, bVert); break;
            case SDRCROOK_STRETCH : CrookStretchXPoint(rPnt, pC1, pC2, aCenter, maRadius, nSin, nCos, bVert, maMarkedRange); break;
        }
    }
}

void SdrDragCrook::MoveSdrDrag(const basegfx::B2DPoint& rPnt)
{
    if(DragStat().CheckMinMoved(rPnt))
    {
        basegfx::B2DPoint aPnt(rPnt);
        const bool bNeuMoveOnly(getSdrView().IsMoveOnlyDragging());
        mbAtCenter = false;
        SdrCrookMode eNeuMode(getSdrView().GetCrookMode());
        const bool bNeuContortion(!bNeuMoveOnly && ((mbContortionAllowed && !getSdrView().IsCrookNoContortion()) || !mbNoContortionAllowed));
        mbResize = !getSdrView().IsOrthogonal() && mbResizeAllowed && !bNeuMoveOnly;
        const bool bNeuRotate(mbRotateAllowed && !bNeuContortion && !bNeuMoveOnly && SDRCROOK_ROTATE == eNeuMode);

        aPnt = getSdrView().GetSnapPos(aPnt);
        basegfx::B2DPoint aNeuCenter(maMarkedCenter.getX(), maStart.getY());

        if(mbVertical)
        {
            aNeuCenter.setX(maStart.getX());
            aNeuCenter.setY(maMarkedCenter.getY());
        }

        if(!getSdrView().IsCrookAtCenter())
        {
            switch(GetDragHdlKind())
            {
                case HDL_UPLFT: aNeuCenter.setX(maMarkedRange.getMaxX()); mbLeft = true; break;
                case HDL_UPPER: aNeuCenter.setY(maMarkedRange.getMaxY()); mbTop = true; break;
                case HDL_UPRGT: aNeuCenter.setX(maMarkedRange.getMinX()); mbRight = true; break;
                case HDL_LEFT:  aNeuCenter.setX(maMarkedRange.getMaxX()); mbLeft = true; break;
                case HDL_RIGHT: aNeuCenter.setX(maMarkedRange.getMinX()); mbRight = true; break;
                case HDL_LWLFT: aNeuCenter.setX(maMarkedRange.getMaxX()); mbLeft = true; break;
                case HDL_LOWER: aNeuCenter.setY(maMarkedRange.getMinY()); mbBottom = true; break;
                case HDL_LWRGT: aNeuCenter.setX(maMarkedRange.getMinX()); mbRight = true; break;
                default: mbAtCenter = true;
            }
        }
        else
        {
            mbAtCenter = true;
        }

        double fNewFactor(1.0);
        const basegfx::B2DVector aDelta(aPnt - aNeuCenter);
        mbValid = !basegfx::fTools::equalZero(mbVertical ? aDelta.getX() : aDelta.getY());

        if(mbValid)
        {
            if(mbVertical)
            {
                mbValid = fabs(aDelta.getX()) * 100.0 > fabs(aDelta.getY());
            }
            else
            {
                mbValid = fabs(aDelta.getY()) * 100.0 > fabs(aDelta.getX());
            }
        }

        double fNewRadius(0.0);
        mfAngle = 0.0;

        if(mbValid)
        {
            double a(0.0); // Steigung des Radius
            double fAngleWithPnt(0.0);

            if(mbVertical)
            {
                a = aDelta.getY() / aDelta.getX(); // Steigung des Radius
                fNewRadius = ((aDelta.getY() * a) + aDelta.getX()) * 0.5;
                aNeuCenter.setX(aNeuCenter.getX() + fNewRadius);

                const basegfx::B2DVector aVector(aPnt - aNeuCenter);
                // keep fAngleWithPnt in old angle to not change below
                // calculations using it in a hardly comprehensible way;
                // at least double precision will be used now.
                fAngleWithPnt = atan2(-aVector.getY(), aVector.getX())/ (F_PI / 18000.0);
            }
            else
            {
                a = aDelta.getX() / aDelta.getY(); // Steigung des Radius
                fNewRadius = ((aDelta.getX() * a) + aDelta.getY()) * 0.5;
                aNeuCenter.setY(aNeuCenter.getY() + fNewRadius);

                const basegfx::B2DVector aVector(aPnt - aNeuCenter);
                // keep fAngleWithPnt in old angle to not change below
                // calculations using it in a hardly comprehensible way;
                // at least double precision will be used now.
                fAngleWithPnt = atan2(-aVector.getY(), aVector.getX()) / (F_PI / 18000.0);
                fAngleWithPnt = fAngleWithPnt - 9000.0;
            }

            if(!mbAtCenter)
            {
                if(fNewRadius < 0.0)
                {
                    if(mbRight)
                    {
                        fAngleWithPnt += 18000.0;
                    }

                    if(mbLeft)
                    {
                        fAngleWithPnt = 18000.0 - fAngleWithPnt;
                    }

                    if(mbBottom)
                    {
                        fAngleWithPnt = -fAngleWithPnt;
                    }
                }
                else
                {
                    if(mbRight)
                    {
                        fAngleWithPnt = -fAngleWithPnt;
                    }

                    if(mbTop)
                    {
                        fAngleWithPnt = 18000.0 - fAngleWithPnt;
                    }

                    if(mbBottom)
                    {
                        fAngleWithPnt += 18000;
                    }
                }

                fAngleWithPnt = basegfx::snapToZeroRange(fAngleWithPnt, 36000.0);
            }
            else
            {
                if(fNewRadius < 0.0)
                {
                    fAngleWithPnt += 18000.0;
                }

                if(mbVertical)
                {
                    fAngleWithPnt = 18000.0 - fAngleWithPnt;
                }

                fAngleWithPnt = fabs(basegfx::snapToRange(fAngleWithPnt, -18000.0, 18000.0));
            }

            double fPerimeter(2.0 * fabs(fNewRadius) * F_PI);

            if(mbResize)
            {
                double fMultiplicator(fPerimeter * basegfx::snapToZeroRange(fAngleWithPnt, 36000.0) / 36000.0);

                if(mbAtCenter)
                {
                    fMultiplicator *= 2.0;
                }

                fNewFactor = fMultiplicator / mfMarkedSize;
                mfAngle = fAngleWithPnt * (F_PI / 18000.0);
            }
            else
            {
                mfAngle = (mfMarkedSize * 360.0 / fPerimeter) * 50.0;
                mfAngle = mfAngle * (F_PI / 18000.0);

                if(basegfx::fTools::equalZero(mfAngle))
                {
                    mbValid = false;
                }
            }
        }

        if(basegfx::fTools::equalZero(mfAngle) || basegfx::fTools::equalZero(fNewRadius))
        {
            mbValid = false;
        }

        if(!mbValid)
        {
            fNewRadius = 0.0;
        }

        if(!mbValid && mbResize)
        {
            double fMultiplicator(mbVertical ? aDelta.getY() : aDelta.getX());

            if(mbLeft || mbTop)
            {
                fMultiplicator = -fMultiplicator;
            }

            if(mbAtCenter)
            {
                fMultiplicator = fabs(fMultiplicator * 2.0);
            }

            fNewFactor = fMultiplicator / mfMarkedSize;
        }

        if(!aNeuCenter.equal(maCenter)
            || bNeuContortion != mbContortion
            || !basegfx::fTools::equal(fNewFactor, mfFactor)
            || bNeuMoveOnly != getMoveOnly()
            || bNeuRotate != mbRotate
            || eNeuMode != meMode)
        {
            Hide();
            setMoveOnly(bNeuMoveOnly);
            mbRotate = bNeuRotate;
            meMode = eNeuMode;
            mbContortion = bNeuContortion;
            maCenter = aNeuCenter;
            mfFactor = fNewFactor;
            maRadius = basegfx::B2DPoint(fNewRadius, fNewRadius);
            mbResize = !basegfx::fTools::equal(mfFactor, 1.0);
            DragStat().NextMove(aPnt);
            Show();
        }
    }
}

void SdrDragCrook::applyCurrentTransformationToSdrObject(SdrObject& rTarget)
{
    const bool bDoResize(!basegfx::fTools::equal(mfFactor, 1.0));
    const bool bDoCrook(!maCenter.equal(maMarkedCenter) && !maRadius.equalZero());

    if(bDoCrook || bDoResize)
    {
        if(bDoResize)
        {
            if(mbContortion)
            {
                basegfx::B2DHomMatrix aTransform;

                aTransform.translate(-maCenter);
                aTransform.scale(mbVertical ? 1.0 : mfFactor, mbVertical ? mfFactor : 1.0);
                aTransform.translate(maCenter);

                sdr::legacy::transformSdrObject(rTarget, aTransform);
            }
            else
            {
                const basegfx::B2DPoint aCtr0(sdr::legacy::GetSnapRange(rTarget).getCenter());
                basegfx::B2DPoint aCtr1(aCtr0);

                if(mbVertical)
                {
                    aCtr1.setY(maCenter.getY() + ((aCtr1.getY() - maCenter.getY()) * mfFactor));
                }
                else
                {
                    aCtr1.setX(maCenter.getX() + ((aCtr1.getX() - maCenter.getX()) * mfFactor));
                }

                sdr::legacy::transformSdrObject(rTarget, basegfx::tools::createTranslateB2DHomMatrix(aCtr1 - aCtr0));
            }
        }

        if(bDoCrook)
        {
            const basegfx::B2DRange aLocalMarkRange(getSdrView().getMarkedObjectSnapRange());
            const bool bLocalRotate(!mbContortion && SDRCROOK_ROTATE == meMode && getSdrView().IsRotateAllowed(false));

            getSdrView().ImpCrookObj(rTarget, maCenter, maRadius, meMode, mbVertical, !mbContortion, bLocalRotate, aLocalMarkRange);
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

    if(mbResize && basegfx::fTools::equal(mfFactor, 1.0))
    {
        mbResize = false;
    }

    const bool bUndo(getSdrView().IsUndoEnabled());
    const bool bDoCrook(!maCenter.equal(maMarkedCenter) && !maRadius.equalZero());

    if(bDoCrook || mbResize)
    {
        if(mbResize && bUndo)
        {
            XubString aStr;
            TakeMarkedDescriptionString(!mbContortion ? STR_EditCrook : STR_EditCrookContortion, aStr);

            if(bCopy)
            {
                aStr += ImpGetResStr(STR_EditWithCopy);
            }

            getSdrView().BegUndo(aStr);
        }

        if(mbResize)
        {
            if(mbContortion)
            {
                if(mbVertical)
                {
                    getSdrView().ResizeMarkedObj(maCenter, basegfx::B2DVector(1.0, mfFactor), bCopy);
                }
                else
                {
                    getSdrView().ResizeMarkedObj(maCenter, basegfx::B2DVector(mfFactor, 1.0), bCopy);
                }
            }
            else
            {
                if(bCopy)
                {
                    getSdrView().CopyMarkedObj();
                }

                if(getSdrView().areSdrObjectsSelected())
                {
                    const SdrObjectVector aSelection(getSdrView().getSelectedSdrObjectVectorFromSdrMarkView());

                    for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
                    {
                        SdrObject* pO = aSelection[nm];
                        const basegfx::B2DPoint aCtr0(sdr::legacy::GetSnapRange(*pO).getCenter());
                        basegfx::B2DPoint aCtr1(aCtr0);

                        if(mbVertical)
                        {
                            aCtr1.setY(maCenter.getY() + ((aCtr1.getY() - maCenter.getY()) * mfFactor));
                        }
                        else
                        {
                            aCtr1.setX(maCenter.getX() + ((aCtr1.getX() - maCenter.getX()) * mfFactor));
                        }

                        if(bUndo)
                        {
                            AddUndo(getSdrView().getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pO));
                        }

                        sdr::legacy::transformSdrObject(*pO, basegfx::tools::createTranslateB2DHomMatrix(aCtr1 - aCtr0));
                    }
                }
            }

            bCopy = false;
        }

        if(bDoCrook)
        {
            getSdrView().CrookMarkedObj(maCenter, maRadius, meMode, mbVertical, !mbContortion, bCopy);
            getSdrView().SetLastCrookCenter(maCenter);
        }

        if(mbResize && bUndo)
        {
            getSdrView().EndUndo();
        }

        return true;
    }

    return false;
}

Pointer SdrDragCrook::GetSdrDragPointer() const
{
    return Pointer(POINTER_CROOK);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragDistort::SdrDragDistort(SdrView& rNewView)
:   SdrDragMethod(rNewView),
    maMarkedRange(),
    maDistortedRangePolygon(),
    mnPointIndex(0),
    mbContortionAllowed(false),
    mbNoContortionAllowed(false),
    mbContortion(false)
{
}

void SdrDragDistort::TakeSdrDragComment(XubString& rStr) const
{
    TakeMarkedDescriptionString(STR_DragMethDistort, rStr);
    XubString aStr;

    rStr.AppendAscii(" (x=");
    getSdrView().getSdrModelFromSdrView().TakeMetricStr(DragStat().GetDX(), aStr);
    rStr += aStr;
    rStr.AppendAscii(" y=");
    getSdrView().getSdrModelFromSdrView().TakeMetricStr(DragStat().GetDY(), aStr);
    rStr += aStr;
    rStr += sal_Unicode(')');

    if(getSdrView().IsDragWithCopy())
    {
        rStr += ImpGetResStr(STR_EditWithCopy);
    }
}

void SdrDragDistort::createSdrDragEntries()
{
    // Add extended frame raster first, so it will be behind objects
    if(getSdrView().GetSdrPageView())
    {
        const basegfx::B2DPolyPolygon aDragRaster(impCreateDragRaster(*getSdrView().GetSdrPageView(), GetMarkedRange()));

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
    mbContortionAllowed = getSdrView().IsDistortAllowed(false);
    mbNoContortionAllowed = getSdrView().IsDistortAllowed(true);

    if(mbContortionAllowed || mbNoContortionAllowed)
    {
        SdrHdlKind eKind(GetDragHdlKind());
        mnPointIndex = 4;

        if(HDL_UPLFT == eKind) mnPointIndex = 0;
        if(HDL_UPRGT == eKind) mnPointIndex = 1;
        if(HDL_LWRGT == eKind) mnPointIndex = 2;
        if(HDL_LWLFT == eKind) mnPointIndex = 3;

        if(mnPointIndex > 3)
        {
            return false;
        }

        maMarkedRange = GetMarkedRange();
        maDistortedRangePolygon = basegfx::tools::createPolygonFromRect(maMarkedRange);
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
    if(mbContortion)
    {
        if(maDistortedRangePolygon.count() > 3)
        {
            rTarget = basegfx::tools::distort(
                rTarget,
                maMarkedRange,
                maDistortedRangePolygon.getB2DPoint(0),  // TopLeft
                maDistortedRangePolygon.getB2DPoint(1),  // rTopRight
                maDistortedRangePolygon.getB2DPoint(3),  // rBottomLeft
                maDistortedRangePolygon.getB2DPoint(2)); // rBottomRight
        }
    }
}

void SdrDragDistort::MoveSdrDrag(const basegfx::B2DPoint& rPnt)
{
    if(DragStat().CheckMinMoved(rPnt))
    {
        basegfx::B2DPoint aPnt(getSdrView().GetSnapPos(rPnt));

        if(getSdrView().IsOrthogonal())
        {
            aPnt = OrthoDistance8(DragStat().GetStart(), aPnt, getSdrView().IsBigOrthogonal());
        }

        bool bNeuContortion((mbContortionAllowed && !getSdrView().IsCrookNoContortion()) || !mbNoContortionAllowed);

        if(bNeuContortion != mbContortion || !aPnt.equal(maDistortedRangePolygon.getB2DPoint(mnPointIndex)))
        {
            Hide();
            maDistortedRangePolygon.setB2DPoint(mnPointIndex, aPnt);
            mbContortion = bNeuContortion;
            DragStat().NextMove(aPnt);
            Show();
        }
    }
}

bool SdrDragDistort::EndSdrDrag(bool bCopy)
{
    Hide();
    const basegfx::B2DVector aDelta(DragStat().GetNow() - DragStat().GetPrev());
    bool bDoDistort(!aDelta.equalZero());

    if(bDoDistort)
    {
        getSdrView().DistortMarkedObj(maMarkedRange, maDistortedRangePolygon, !mbContortion, bCopy);

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
    const basegfx::B2DVector aDelta(DragStat().GetNow() - DragStat().GetPrev());
    bool bDoDistort(!aDelta.equalZero());

    if(bDoDistort)
    {
        getSdrView().ImpDistortObj(rTarget, maMarkedRange, maDistortedRangePolygon, !mbContortion);
    }
}

void SdrDragDistort::applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget)
{
    // use helper derived from old stuff
    _MovAllPoints(rTarget);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragCrop::SdrDragCrop(SdrView& rNewView)
:   SdrDragObjOwn(rNewView)
{
    // switch off solid dragging for crop; it just makes no sense since showing
    // a 50% transparent object above the original will not be visible
    setSolidDraggingActive(false);
}

void SdrDragCrop::TakeSdrDragComment(XubString& rStr) const
{
    TakeMarkedDescriptionString(STR_DragMethCrop, rStr);

    XubString aStr;

    rStr.AppendAscii(" (x=");
    getSdrView().getSdrModelFromSdrView().TakeMetricStr(DragStat().GetDX(), aStr);
    rStr += aStr;
    rStr.AppendAscii(" y=");
    getSdrView().getSdrModelFromSdrView().TakeMetricStr(DragStat().GetDY(), aStr);
    rStr += aStr;
    rStr += sal_Unicode(')');

    if(getSdrView().IsDragWithCopy())
        rStr += ImpGetResStr(STR_EditWithCopy);
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

    if( DragStat().GetDX()==0 && DragStat().GetDY()==0 )
        return false;

    SdrGrafObj* pObj = dynamic_cast< SdrGrafObj* >(getSdrView().getSelectedIfSingle());

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

    const bool bUndo = getSdrView().IsUndoEnabled();

    if( bUndo )
    {
        String aUndoStr;
        TakeMarkedDescriptionString(STR_DragMethCrop, aUndoStr);

        getSdrView().BegUndo( aUndoStr );
        getSdrView().AddUndo( getSdrView().getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
        // also need attr undo, the SdrGrafCropItem will be changed
        getSdrView().AddUndo( getSdrView().getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoAttrObject(*pObj));
    }

    // new part to comute the user's drag activities
    // get the original objects transformation
    basegfx::B2DHomMatrix aOriginalMatrix(pObj->getSdrObjectTransformation());

    // invert it to be able to work on unit coordinates
    basegfx::B2DHomMatrix aInverse(aOriginalMatrix);

    aInverse.invert();

    // gererate start point of original drag vector in unit coordinates (the
    // vis-a-vis of the drag point)
    basegfx::B2DPoint aLocalStart(0.0, 0.0);
    bool bOnAxis(false);

    switch(GetDragHdlKind())
    {
        case HDL_UPLFT: aLocalStart.setX(1.0); aLocalStart.setY(1.0); break;
        case HDL_UPPER: aLocalStart.setX(0.5); aLocalStart.setY(1.0); bOnAxis = true; break;
        case HDL_UPRGT: aLocalStart.setX(0.0); aLocalStart.setY(1.0); break;
        case HDL_LEFT : aLocalStart.setX(1.0); aLocalStart.setY(0.5); bOnAxis = true; break;
        case HDL_RIGHT: aLocalStart.setX(0.0); aLocalStart.setY(0.5); bOnAxis = true; break;
        case HDL_LWLFT: aLocalStart.setX(1.0); aLocalStart.setY(0.0); break;
        case HDL_LOWER: aLocalStart.setX(0.5); aLocalStart.setY(0.0); bOnAxis = true; break;
        case HDL_LWRGT: aLocalStart.setX(0.0); aLocalStart.setY(0.0); break;
        default: break;
    }

    // create the current drag position in unit coordinates
    basegfx::B2DPoint aLocalCurrent(aInverse * DragStat().GetNow());

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

    // preparematrix to apply to object; evtl. back-correct shear
    basegfx::B2DHomMatrix aNewObjectMatrix(aOriginalMatrix * aDiscreteChangeMatrix);

    // apply change to object by applying the unit coordinate change followed
    // by the original change
    pObj->setSdrObjectTransformation(aNewObjectMatrix);

    // the following old code uses aOldRect/aNewRect to calculate the crop change for
    // the crop item. It implies unrotated objects, so create the unrotated original
    // erctangle and the unrotated modified rectangle. Latter can in case of shear and/or
    // rotation not be fetched by using
    //
    //Rectangle aNewRect( pObj->GetLogicRect() );
    //
    // as it was done before because the top-left of that new rect *will* have an offset
    // caused by the evtl. existing shear and/or rotation, so calculate a unrotated
    // rectangle how it would be as a result when appling the unit coordinate change
    // to the unrotated original transformation.
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate, fShearX;

    // get access to scale and translate
    aOriginalMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

    // prepare unsheared/unrotated versions of the old and new transformation
    const basegfx::B2DHomMatrix aMatrixOriginalNoShearNoRotate(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            basegfx::absolute(aScale),
            aTranslate));

    // create the ranges for these
    basegfx::B2DRange aRangeOriginalNoShearNoRotate(0.0, 0.0, 1.0, 1.0);
    basegfx::B2DRange aRangeNewNoShearNoRotate(0.0, 0.0, 1.0, 1.0);

    aRangeOriginalNoShearNoRotate.transform(aMatrixOriginalNoShearNoRotate);
    aRangeNewNoShearNoRotate.transform(aMatrixOriginalNoShearNoRotate * aDiscreteChangeMatrix);

    // extract the old Rectangle structures
    Rectangle aOldRect(
        basegfx::fround(aRangeOriginalNoShearNoRotate.getMinX()),
        basegfx::fround(aRangeOriginalNoShearNoRotate.getMinY()),
        basegfx::fround(aRangeOriginalNoShearNoRotate.getMaxX()),
        basegfx::fround(aRangeOriginalNoShearNoRotate.getMaxY()));
    Rectangle aNewRect(
        basegfx::fround(aRangeNewNoShearNoRotate.getMinX()),
        basegfx::fround(aRangeNewNoShearNoRotate.getMinY()),
        basegfx::fround(aRangeNewNoShearNoRotate.getMaxX()),
        basegfx::fround(aRangeNewNoShearNoRotate.getMaxY()));

    // continue with the old original stuff
    double fScaleX = ( aGraphicSize.Width() - rOldCrop.GetLeft() - rOldCrop.GetRight() ) / (double)aOldRect.GetWidth();
    double fScaleY = ( aGraphicSize.Height() - rOldCrop.GetTop() - rOldCrop.GetBottom() ) / (double)aOldRect.GetHeight();

    // not needed since the modification is done in unit coordinates, free from shear/rotate and mirror
    // // TTTT may be removed or exhanged by other stuff in aw080
    // // to correct the never working combination of cropped images and mirroring
    // // I have to correct the rectangles the calculation is based on here. In the current
    // // core geometry stuff a vertical mirror is expressed as 180 degree rotation. All
    // // this can be removed again when aw080 will have cleaned up the old
    // // (non-)transformation mess in the core.
    // if(18000 == pObj->GetGeoStat().nDrehWink)
    // {
    //     // old notation of vertical mirror, need to correct diffs since both rects
    //     // are rotated by 180 degrees
    //     aOldRect = Rectangle(aOldRect.TopLeft() - (aOldRect.BottomRight() - aOldRect.TopLeft()), aOldRect.TopLeft());
    //     aNewRect = Rectangle(aNewRect.TopLeft() - (aNewRect.BottomRight() - aNewRect.TopLeft()), aNewRect.TopLeft());
    // }

    const sal_Int32 nDiffLeft(aNewRect.nLeft - aOldRect.nLeft);
    const sal_Int32 nDiffTop(aNewRect.nTop - aOldRect.nTop);
    const sal_Int32 nDiffRight(aNewRect.nRight - aOldRect.nRight);
    const sal_Int32 nDiffBottom(aNewRect.nBottom - aOldRect.nBottom);

    // TTTT: Need to check if the whole crop preview stuff still works
    // due to the different mirror and object transformation handling (!)

    sal_Int32 nLeftCrop = static_cast<sal_Int32>( rOldCrop.GetLeft() + nDiffLeft * fScaleX );
    sal_Int32 nTopCrop = static_cast<sal_Int32>( rOldCrop.GetTop() + nDiffTop * fScaleY );
    sal_Int32 nRightCrop = static_cast<sal_Int32>( rOldCrop.GetRight() - nDiffRight * fScaleX );
    sal_Int32 nBottomCrop = static_cast<sal_Int32>( rOldCrop.GetBottom() - nDiffBottom * fScaleY );

    SfxItemPool& rPool = getSdrView().getSdrModelFromSdrView().GetItemPool();
    SfxItemSet aSet( rPool, SDRATTR_GRAFCROP, SDRATTR_GRAFCROP );
    aSet.Put( SdrGrafCropItem( nLeftCrop, nTopCrop, nRightCrop, nBottomCrop ) );
    getSdrView().SetAttributes( aSet, false );

    if( bUndo )
        getSdrView().EndUndo();

    return true;
}

Pointer SdrDragCrop::GetSdrDragPointer() const
{
    return Pointer(POINTER_CROP);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
