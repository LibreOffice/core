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

#include <notemark.hxx>
#include <postit.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/overlay/overlayprimitive2dsequenceobject.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <dbfunc.hxx>

#define SC_NOTEOVERLAY_TIME    800
#define SC_NOTEOVERLAY_SHORT   70

ScNoteOverlay::ScNoteOverlay(
    ScGridWindow& rScGridWindow,
    ScAddress& aPos,
    OUString aUser,
    bool bLeftEdge,
    bool bForce,
    bool bKeyboard)
: Timer("ScNoteOverlay Timer")
, mrScGridWindow(rScGridWindow)
, maDocPos(aPos)
, maUserText(std::move(aUser))
, maNoteOverlayGroup()
, mxObject()
, maSequence()
, mbLeft(bLeftEdge)
, mbKeyboard(bKeyboard)
{
    SetTimeout(bForce ? SC_NOTEOVERLAY_SHORT : SC_NOTEOVERLAY_TIME);
    Start();
}

const drawinglayer::primitive2d::Primitive2DContainer& ScNoteOverlay::getOrCreatePrimitive2DSequence()
{
    if (!maSequence.empty())
        // sequence already created, return it
        return maSequence;

    // get some local data ptrs
    ScViewData& rViewData(mrScGridWindow.getViewData());
    ScDocument& rDoc(rViewData.GetDocument());
    ScDrawLayer* pScDrawLayer(rDoc.GetDrawLayer());

    // use existing SdrPage - old version did allocate a SdrModel/SdrPage
    // for every visualization
    SdrPage* pSdrPage(pScDrawLayer->GetPage(0));
    if (nullptr == pSdrPage)
        return maSequence;

    // when using existing SdrPage do not forget to save change state of
    // the DrawingLayer. Unfortunately CreateTempCaption below *does* add
    // the SdrObject to the SdrPage - not necessary for the model stuff,
    // but a comment there claims that else the text cannot be set (?)
    const bool bModelChanged(pScDrawLayer->IsChanged());
    const tools::Rectangle aVisibleRectangle(calculateVisibleRectangle());

    // create the temporary SdrObject
    mxObject = ScNoteUtil::CreateTempCaption(
        rDoc,
        maDocPos,
        *pSdrPage,
        maUserText,
        aVisibleRectangle,
        mbLeft);

    if (mxObject.is())
    {
        // get the primitives from it
        mxObject->GetViewContact().getViewIndependentPrimitive2DContainer(maSequence);

        // cleanup: remove again immediately
        pSdrPage->NbcRemoveObject(mxObject->GetOrdNum());

        // show the visualization with slight transparency
        static bool bUseTransparency(true);
        if (bUseTransparency && !maSequence.empty())
        {
            maSequence = drawinglayer::primitive2d::Primitive2DContainer{
                rtl::Reference<drawinglayer::primitive2d::UnifiedTransparencePrimitive2D>(
                    new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(std::move(maSequence), 0.1))};
        }
    }

    // restore changed state of DrawingLayer
    pScDrawLayer->SetChanged(bModelChanged);
    return maSequence;
}

tools::Rectangle ScNoteOverlay::calculateVisibleRectangle()
{
    // to not change anything for now in positioning/object
    // creation sticked together from previous versions. This
    // can (should) be converted to transformation stuff
    // later. In principle it calculates the size of the
    // merged WindowSpace (all SplitWindows) and transforms
    // that range to logical coordinates in one of the
    // ScGridWindow's
    const ScViewData& rViewData(mrScGridWindow.getViewData());
    const bool bHSplit(SC_SPLIT_NONE != rViewData.GetHSplitMode());
    const bool bVSplit(SC_SPLIT_NONE != rViewData.GetVSplitMode());
    const ScTabView* pScTabView(rViewData.GetView());
    const vcl::Window* pLeft(pScTabView->GetWindowByPos(bVSplit ? SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT));
    const vcl::Window* pRight(bHSplit ? pScTabView->GetWindowByPos( bVSplit ? SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT ) : nullptr);
    const vcl::Window* pBottom(bVSplit ? pScTabView->GetWindowByPos( SC_SPLIT_BOTTOMLEFT ) : nullptr);
    const vcl::Window* pDiagonal((bHSplit && bVSplit) ? pScTabView->GetWindowByPos(SC_SPLIT_BOTTOMRIGHT) : nullptr);
    assert(pLeft && "ScNoteOverlay - missing top-left grid window");

    /*  If caption is shown from right or bottom windows, adjust
        mapmode to include size of top-left window. */
    Size aSizePixel(pLeft->GetOutputSizePixel());
    MapMode aMapMode(mrScGridWindow.GetDrawMapMode(true));
    const Size aLeftSize(pLeft->PixelToLogic(aSizePixel, aMapMode));
    Point aOrigin(aMapMode.GetOrigin());

    if ((&mrScGridWindow == pRight) || (&mrScGridWindow == pDiagonal))
        aOrigin.AdjustX(aLeftSize.Width());

    if ((&mrScGridWindow == pBottom) || (&mrScGridWindow == pDiagonal))
        aOrigin.AdjustY(aLeftSize.Height());

    aMapMode.SetOrigin(aOrigin);

    if (nullptr != pRight)
        aSizePixel.AdjustWidth(pRight->GetOutputSizePixel().Width());

    if (nullptr != pBottom)
        aSizePixel.AdjustHeight(pBottom->GetOutputSizePixel().Height());

    return mrScGridWindow.PixelToLogic(tools::Rectangle(Point(0, 0), aSizePixel), aMapMode);
}

void ScNoteOverlay::createOverlaySubContent(
    ScGridWindow* pTarget,
    const basegfx::B2DHomMatrix& rTransformToPixels,
    const basegfx::B2DPoint& rTopLeft)
{
    // create additional visualization in given ScGridWindow
    // with given partial transform and discrete offset
    rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager(pTarget->getOverlayManager());
    if (!xOverlayManager.is())
        // no OverlayManager, no visualization
        return;

    // create a transformation from initial ScGridWindow for which the
    // visualization was created and in who's DrawMapMode it is to the
    // target ScGridWindow. That transformation spans over the merged
    // SplitWindow display. To do so, transform:
    // 1 from initial ScGridWindow logic DrawMapMode to discrete (pixels)
    //   in the displaying window
    // 2 to merged SplitWindow display (may have different top-left)
    // 3 to window displaying target ScGridWindow
    // 4 to logic coordinates in it's DrawMapMode
    // NOTE: 1+2 are already in rTransformToPixels
    basegfx::B2DHomMatrix aTransformToTarget(rTransformToPixels);
    aTransformToTarget.translate(-rTopLeft);
    const MapMode aOrig(pTarget->GetMapMode());
    pTarget->SetMapMode(pTarget->GetDrawMapMode(true));
    aTransformToTarget = pTarget->GetOutDev()->GetInverseViewTransformation() * aTransformToTarget;
    pTarget->SetMapMode(aOrig);

    // embed to TransformPrimitive
    drawinglayer::primitive2d::Primitive2DContainer aSequence(getOrCreatePrimitive2DSequence());
    aSequence = drawinglayer::primitive2d::Primitive2DContainer{
        rtl::Reference<drawinglayer::primitive2d::TransformPrimitive2D>(
            new drawinglayer::primitive2d::TransformPrimitive2D(
                aTransformToTarget,
                std::move(aSequence)))};

    // create OverlayObject
    std::unique_ptr<sdr::overlay::OverlayObject> pOverlayObject(
        new sdr::overlay::OverlayPrimitive2DSequenceObject(
            std::move(aSequence)));

    // add to OverlayManager and local data holder (for
    // destruction)
    xOverlayManager->add(*pOverlayObject);
    maNoteOverlayGroup.append(std::move(pOverlayObject));
}

void ScNoteOverlay::createAdditionalRepresentations()
{
    // check if we have a split at all
    const ScViewData& rViewData(mrScGridWindow.getViewData());
    const ScTabView* pScTabView(rViewData.GetView());
    const bool bHSplit(SC_SPLIT_NONE != rViewData.GetHSplitMode());
    const bool bVSplit(SC_SPLIT_NONE != rViewData.GetVSplitMode());

    if (!bHSplit && !bVSplit)
        // no split screen, no additional visualizations needed
        return;

    if (getOrCreatePrimitive2DSequence().empty())
        // no visualization, done
        return;

    // if we have a split screen with multiple ScGridWindow's
    // the visualization might overlap with other ones than the
    // one this gets initially constructed. get the logic range
    // of the original visualization in the original ScGridWindow
    const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
    const basegfx::B2DRange aContentRange(getOrCreatePrimitive2DSequence().getB2DRange(aViewInformation2D));

    // prep values to be filled in own scope to not hold the vars
    basegfx::B2DRange aGlobalPixel(aContentRange);
    basegfx::B2DVector aTopLeftOffset;
    basegfx::B2DHomMatrix aTransformToPixels;

    {
        // calculate general TopLeft offsets for potential other windows
        // from top-left window
        const ScSplitPos myScSplitPos(mrScGridWindow.getScSplitPos());
        const ScGridWindow* pLeft(static_cast<ScGridWindow*>(
            pScTabView->GetWindowByPos(bVSplit ? SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT)));
        aTopLeftOffset.setX(pLeft->GetOutputSizePixel().Width());
        aTopLeftOffset.setY(pLeft->GetOutputSizePixel().Height());

        // create transformation from logic coordinates in original
        // ScGridWindow to discrete coordinates (pixels), then to
        // merged SplitWindow display
        const MapMode aOrig(mrScGridWindow.GetMapMode());
        mrScGridWindow.SetMapMode(mrScGridWindow.GetDrawMapMode(true));
        aTransformToPixels = mrScGridWindow.GetOutDev()->GetViewTransformation();
        if (SC_SPLIT_TOPRIGHT == myScSplitPos || SC_SPLIT_BOTTOMRIGHT == myScSplitPos)
            aTransformToPixels.translate(aTopLeftOffset.getX(), 0);
        if (SC_SPLIT_BOTTOMLEFT == myScSplitPos || SC_SPLIT_BOTTOMRIGHT == myScSplitPos)
            aTransformToPixels.translate(0, aTopLeftOffset.getY());
        aGlobalPixel.transform(aTransformToPixels);
        mrScGridWindow.SetMapMode(aOrig);
    }

    // try all four possible ScGridWindows
    ScGridWindow* pBottomLeft(static_cast<ScGridWindow*>(pScTabView->GetWindowByPos(SC_SPLIT_BOTTOMLEFT)));

    // nothing to do when ScGridWindow does not exists or is same as
    // original ScGridWindow for which visualization is already done
    if (nullptr != pBottomLeft && pBottomLeft != &mrScGridWindow)
    {
        // calculate pixel range relative to merged SplitWindow display
        const basegfx::B2DPoint aTopLeft(0, bVSplit ? aTopLeftOffset.getY() : 0);
        const basegfx::B2DVector aSize(pBottomLeft->GetOutputSizePixel().Width(), pBottomLeft->GetOutputSizePixel().Height());
        const basegfx::B2DRange aLocalPixel(aTopLeft, aTopLeft + aSize);

        if(aLocalPixel.overlaps(aGlobalPixel))
        {
            // if needed visualization is visible there, create an OverlayObject there
            // with the same content but at corrected position
            createOverlaySubContent(pBottomLeft, aTransformToPixels, aTopLeft);
        }
    }

    ScGridWindow* pBottomRight(static_cast<ScGridWindow*>(pScTabView->GetWindowByPos(SC_SPLIT_BOTTOMRIGHT)));
    if (nullptr != pBottomRight && pBottomRight != &mrScGridWindow)
    {
        const basegfx::B2DPoint aTopLeft(bHSplit ? aTopLeftOffset.getX() : 0, bVSplit ? aTopLeftOffset.getY() : 0);
        const basegfx::B2DVector aSize(pBottomRight->GetOutputSizePixel().Width(), pBottomRight->GetOutputSizePixel().Height());
        const basegfx::B2DRange aLocalPixel(aTopLeft, aTopLeft + aSize);

        if(aLocalPixel.overlaps(aGlobalPixel))
        {
            createOverlaySubContent(pBottomRight, aTransformToPixels, aTopLeft);
        }
    }

    ScGridWindow* pTopLeft(static_cast<ScGridWindow*>(pScTabView->GetWindowByPos(SC_SPLIT_TOPLEFT)));
    if (nullptr != pTopLeft && pTopLeft != &mrScGridWindow)
    {
        const basegfx::B2DPoint aTopLeft(0, 0);
        const basegfx::B2DVector aSize(pTopLeft->GetOutputSizePixel().Width(), pTopLeft->GetOutputSizePixel().Height());
        const basegfx::B2DRange aLocalPixel(aTopLeft, aTopLeft + aSize);

        if(aLocalPixel.overlaps(aGlobalPixel))
        {
            createOverlaySubContent(pTopLeft, aTransformToPixels, aTopLeft);
        }
    }

    ScGridWindow* pTopRight(static_cast<ScGridWindow*>(pScTabView->GetWindowByPos(SC_SPLIT_TOPRIGHT)));
    if (nullptr != pTopRight && pTopRight != &mrScGridWindow)
    {
        const basegfx::B2DPoint aTopLeft(bHSplit ? aTopLeftOffset.getX() : 0, 0);
        const basegfx::B2DVector aSize(pTopRight->GetOutputSizePixel().Width(), pTopRight->GetOutputSizePixel().Height());
        const basegfx::B2DRange aLocalPixel(aTopLeft, aTopLeft + aSize);

        if(aLocalPixel.overlaps(aGlobalPixel))
        {
            createOverlaySubContent(pTopRight, aTransformToPixels, aTopLeft);
        }
    }
}

void ScNoteOverlay::Invoke()
{
    if (0 != maNoteOverlayGroup.count())
        // already visualized, done
        return;

    rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager(
        mrScGridWindow.getOverlayManager());
    if (!xOverlayManager.is())
        // no OverlayManager, no display
        return;

    if (getOrCreatePrimitive2DSequence().empty())
        // no visualization data, no display
        return;

    // create OverlayObject for initial ScGridWindow
    drawinglayer::primitive2d::Primitive2DContainer aSequence(
        getOrCreatePrimitive2DSequence());
    std::unique_ptr<sdr::overlay::OverlayObject> pNewOverlayObject(
        new sdr::overlay::OverlayPrimitive2DSequenceObject(
            std::move(aSequence)));

    // add to OverlayManager and local data holder (for
    // destruction)
    xOverlayManager->add(*pNewOverlayObject);
    maNoteOverlayGroup.append(std::move(pNewOverlayObject));

    // check and evtl. create visualizations for other ScGridWindows
    // in SplitScreen mode
    createAdditionalRepresentations();
}

ScNoteOverlay::~ScNoteOverlay()
{
    // cleanup OverlayObjects - would also be done by destructor
    maNoteOverlayGroup.clear();

    // destruct temporary SdrObject. It *needs* to be kept alive
    // during visualization due to it being used for decmpose
    // of the TextPrimitive. That is an old compromize in the
    // primitives: the text primitive is not self-contained in
    // the sense that it needs the SdrTextObj for decompose.
    // Would be hard to correct, but would be good for the future
    mxObject.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
