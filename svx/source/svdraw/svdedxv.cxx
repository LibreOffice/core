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

#include <com/sun/star/i18n/WordType.hpp>
#include <svtools/accessibilityoptions.hxx>
#include <svx/svdedxv.hxx>
#include <svl/solar.hrc>
#include <svl/itemiter.hxx>
#include <vcl/weld.hxx>
#include <vcl/hatch.hxx>
#include <svl/whiter.hxx>
#include <svl/style.hxx>
#include <editeng/editstat.hxx>
#include <vcl/cursor.hxx>
#include <editeng/unotext.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdoutl.hxx>
#include <svx/sdtfchim.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdundo.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdotable.hxx>
#include <svx/selectioncontroller.hxx>
#ifdef DBG_UTIL
#include <svdibrow.hxx>
#endif
#include <svx/svddrgv.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/globl3d.hxx>
#include <svx/textchain.hxx>
#include <svx/textchaincursor.hxx>
#include <editeng/outliner.hxx>
#include <editeng/adjustitem.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdrundomanager.hxx>
#include <sdr/overlay/overlaytools.hxx>
#include <svx/sdr/table/tablecontroller.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/svdviter.hxx>
#include <svx/sdr/overlay/overlayselection.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpagewindow.hxx>
#include <sal/log.hxx>

#include <memory>

void SdrObjEditView::ImpClearVars()
{
    bQuickTextEditMode=true;
    pTextEditOutliner.reset();
    pTextEditOutlinerView=nullptr;
    pTextEditPV=nullptr;
    pTextEditWin=nullptr;
    pTextEditCursorBuffer=nullptr;
    bTextEditNewObj=false;
    bMacroDown=false;
    pMacroObj=nullptr;
    pMacroPV=nullptr;
    pMacroWin=nullptr;
    nMacroTol=0;
    bTextEditDontDelete=false;
    bTextEditOnlyOneView=false;
}

SdrObjEditView::SdrObjEditView(
    SdrModel& rSdrModel,
    OutputDevice* pOut)
:   SdrGlueEditView(rSdrModel, pOut),
    mpOldTextEditUndoManager(nullptr)
{
    ImpClearVars();
}

SdrObjEditView::~SdrObjEditView()
{
    pTextEditWin = nullptr;            // so there's no ShowCursor in SdrEndTextEdit
    assert(!IsTextEdit());
    if (IsTextEdit())
        SdrEndTextEdit();
    pTextEditOutliner.reset();
    assert(nullptr == mpOldTextEditUndoManager); // should have been reset
}

bool SdrObjEditView::IsAction() const
{
    return IsMacroObj() || SdrGlueEditView::IsAction();
}

void SdrObjEditView::MovAction(const Point& rPnt)
{
    if (IsMacroObj()) MovMacroObj(rPnt);
    SdrGlueEditView::MovAction(rPnt);
}

void SdrObjEditView::EndAction()
{
    if (IsMacroObj()) EndMacroObj();
    SdrGlueEditView::EndAction();
}

void SdrObjEditView::BckAction()
{
    BrkMacroObj();
    SdrGlueEditView::BckAction();
}

void SdrObjEditView::BrkAction()
{
    BrkMacroObj();
    SdrGlueEditView::BrkAction();
}

SdrPageView* SdrObjEditView::ShowSdrPage(SdrPage* pPage)
{
    SdrPageView* pPageView = SdrGlueEditView::ShowSdrPage(pPage);

    if (comphelper::LibreOfficeKit::isActive() && pPageView)
    {
        // Check if other views have an active text edit on the same page as
        // this one.
        SdrViewIter aIter(pPageView->GetPage());
        for (SdrView* pView = aIter.FirstView(); pView; pView = aIter.NextView())
        {
            if (pView == this || !pView->IsTextEdit())
                continue;

            OutputDevice* pOutDev = GetFirstOutputDevice();
            if (!pOutDev || pOutDev->GetOutDevType() != OUTDEV_WINDOW)
                continue;

            // Found one, so create an outliner view, to get invalidations when
            // the text edit changes.
            // Call GetSfxViewShell() to make sure ImpMakeOutlinerView()
            // registers the view shell of this draw view, and not the view
            // shell of pView.
            OutlinerView* pOutlinerView = pView->ImpMakeOutlinerView(static_cast<vcl::Window*>(pOutDev), nullptr, GetSfxViewShell());
            pOutlinerView->HideCursor();
            pView->GetTextEditOutliner()->InsertView(pOutlinerView);
        }
    }

    return pPageView;
}

namespace {

/// Removes outliner views registered in other draw views that use pOutputDevice.
void lcl_RemoveTextEditOutlinerViews(SdrObjEditView const * pThis, SdrPageView const * pPageView, OutputDevice const * pOutputDevice)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    if (!pPageView)
        return;

    if (!pOutputDevice || pOutputDevice->GetOutDevType() != OUTDEV_WINDOW)
        return;

    SdrViewIter aIter(pPageView->GetPage());
    for (SdrView* pView = aIter.FirstView(); pView; pView = aIter.NextView())
    {
        if (pView == pThis || !pView->IsTextEdit())
            continue;

        SdrOutliner* pOutliner = pView->GetTextEditOutliner();
        for (size_t nView = 0; nView < pOutliner->GetViewCount(); ++nView)
        {
            OutlinerView* pOutlinerView = pOutliner->GetView(nView);
            if (pOutlinerView->GetWindow() != pOutputDevice)
                continue;

            pOutliner->RemoveView(pOutlinerView);
            delete pOutlinerView;
        }
    }
}

}

void SdrObjEditView::HideSdrPage()
{
    lcl_RemoveTextEditOutlinerViews(this, GetSdrPageView(), GetFirstOutputDevice());

    SdrGlueEditView::HideSdrPage();
}

void SdrObjEditView::TakeActionRect(tools::Rectangle& rRect) const
{
    if (IsMacroObj()) {
        rRect=pMacroObj->GetCurrentBoundRect();
    } else {
        SdrGlueEditView::TakeActionRect(rRect);
    }
}

void SdrObjEditView::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    SdrGlueEditView::Notify(rBC,rHint);
    if (pTextEditOutliner!=nullptr) {
        // change of printer while editing
        const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint);
        if (pSdrHint!=nullptr) {
            SdrHintKind eKind=pSdrHint->GetKind();
            if (eKind==SdrHintKind::RefDeviceChange) {
                pTextEditOutliner->SetRefDevice(mpModel->GetRefDevice());
            }
            if (eKind==SdrHintKind::DefaultTabChange) {
                pTextEditOutliner->SetDefTab(mpModel->GetDefaultTabulator());
            }
        }
    }
}

void SdrObjEditView::ModelHasChanged()
{
    SdrGlueEditView::ModelHasChanged();
    if (mxTextEditObj.is() && !mxTextEditObj->IsInserted()) SdrEndTextEdit(); // object deleted
    // TextEditObj changed?
    if (IsTextEdit()) {
        SdrTextObj* pTextObj= mxTextEditObj.get();
        if (pTextObj!=nullptr) {
            sal_uIntPtr nOutlViewCnt=pTextEditOutliner->GetViewCount();
            bool bAreaChg=false;
            bool bAnchorChg=false;
            bool bColorChg=false;
            bool bContourFrame=pTextObj->IsContourTextFrame();
            EEAnchorMode eNewAnchor(EEAnchorMode::VCenterHCenter);
            tools::Rectangle aOldArea(aMinTextEditArea);
            aOldArea.Union(aTextEditArea);
            Color aNewColor;
            { // check area
                Size aPaperMin1;
                Size aPaperMax1;
                tools::Rectangle aEditArea1;
                tools::Rectangle aMinArea1;
                pTextObj->TakeTextEditArea(&aPaperMin1,&aPaperMax1,&aEditArea1,&aMinArea1);
                Point aPvOfs(pTextObj->GetTextEditOffset());

                // add possible GridOffset to up-to-now view-independent EditAreas
                basegfx::B2DVector aGridOffset(0.0, 0.0);
                if(getPossibleGridOffsetForSdrObject(aGridOffset, pTextObj, GetSdrPageView()))
                {
                    const Point aOffset(
                        basegfx::fround(aGridOffset.getX()),
                        basegfx::fround(aGridOffset.getY()));

                    aEditArea1 += aOffset;
                    aMinArea1 += aOffset;
                }

                aEditArea1.Move(aPvOfs.X(),aPvOfs.Y());
                aMinArea1.Move(aPvOfs.X(),aPvOfs.Y());
                tools::Rectangle aNewArea(aMinArea1);
                aNewArea.Union(aEditArea1);

                if (aNewArea!=aOldArea || aEditArea1!=aTextEditArea || aMinArea1!=aMinTextEditArea ||
                    pTextEditOutliner->GetMinAutoPaperSize()!=aPaperMin1 || pTextEditOutliner->GetMaxAutoPaperSize()!=aPaperMax1) {
                    aTextEditArea=aEditArea1;
                    aMinTextEditArea=aMinArea1;
                    pTextEditOutliner->SetUpdateMode(false);
                    pTextEditOutliner->SetMinAutoPaperSize(aPaperMin1);
                    pTextEditOutliner->SetMaxAutoPaperSize(aPaperMax1);
                    pTextEditOutliner->SetPaperSize(Size(0,0)); // re-format Outliner
                    if (!bContourFrame) {
                        pTextEditOutliner->ClearPolygon();
                        EEControlBits nStat=pTextEditOutliner->GetControlWord();
                        nStat|=EEControlBits::AUTOPAGESIZE;
                        pTextEditOutliner->SetControlWord(nStat);
                    } else {
                        EEControlBits nStat=pTextEditOutliner->GetControlWord();
                        nStat&=~EEControlBits::AUTOPAGESIZE;
                        pTextEditOutliner->SetControlWord(nStat);
                        tools::Rectangle aAnchorRect;
                        pTextObj->TakeTextAnchorRect(aAnchorRect);
                        pTextObj->ImpSetContourPolygon(*pTextEditOutliner,aAnchorRect, true);
                    }
                    for (sal_uIntPtr nOV=0; nOV<nOutlViewCnt; nOV++) {
                        OutlinerView* pOLV=pTextEditOutliner->GetView(nOV);
                        EVControlBits nStat0=pOLV->GetControlWord();
                        EVControlBits nStat=nStat0;
                        // AutoViewSize only if not ContourFrame.
                        if (!bContourFrame) nStat|=EVControlBits::AUTOSIZE;
                        else nStat&=~EVControlBits::AUTOSIZE;
                        if (nStat!=nStat0) pOLV->SetControlWord(nStat);
                    }
                    pTextEditOutliner->SetUpdateMode(true);
                    bAreaChg=true;
                }
            }
            if (pTextEditOutlinerView!=nullptr) { // check fill and anchor
                EEAnchorMode eOldAnchor=pTextEditOutlinerView->GetAnchorMode();
                eNewAnchor=pTextObj->GetOutlinerViewAnchorMode();
                bAnchorChg=eOldAnchor!=eNewAnchor;
                Color aOldColor(pTextEditOutlinerView->GetBackgroundColor());
                aNewColor = GetTextEditBackgroundColor(*this);
                bColorChg=aOldColor!=aNewColor;
            }
            // refresh always when it's a contour frame. That
            // refresh is necessary since it triggers the repaint
            // which makes the Handles visible. Changes at TakeTextRect()
            // seem to have resulted in a case where no refresh is executed.
            // Before that, a refresh must have been always executed
            // (else this error would have happened earlier), thus I
            // even think here a refresh should be done always.
            // Since follow-up problems cannot even be guessed I only
            // add this one more case to the if below.
            // BTW: It's VERY bad style that here, inside ModelHasChanged()
            // the outliner is again massively changed for the text object
            // in text edit mode. Normally, all necessary data should be
            // set at SdrBeginTextEdit(). Some changes and value assigns in
            // SdrBeginTextEdit() are completely useless since they are set here
            // again on ModelHasChanged().
            if (bContourFrame || bAreaChg || bAnchorChg || bColorChg)
            {
                for (sal_uIntPtr nOV=0; nOV<nOutlViewCnt; nOV++)
                {
                    OutlinerView* pOLV=pTextEditOutliner->GetView(nOV);
                    { // invalidate old OutlinerView area
                        vcl::Window* pWin=pOLV->GetWindow();
                        tools::Rectangle aTmpRect(aOldArea);
                        sal_uInt16 nPixSiz=pOLV->GetInvalidateMore()+1;
                        Size aMore(pWin->PixelToLogic(Size(nPixSiz,nPixSiz)));
                        aTmpRect.AdjustLeft( -(aMore.Width()) );
                        aTmpRect.AdjustRight(aMore.Width() );
                        aTmpRect.AdjustTop( -(aMore.Height()) );
                        aTmpRect.AdjustBottom(aMore.Height() );
                        InvalidateOneWin(*pWin,aTmpRect);
                    }
                    if (bAnchorChg)
                        pOLV->SetAnchorMode(eNewAnchor);
                    if (bColorChg)
                        pOLV->SetBackgroundColor( aNewColor );

                    pOLV->SetOutputArea(aTextEditArea); // because otherwise, we're not re-anchoring correctly
                    ImpInvalidateOutlinerView(*pOLV);
                }
                pTextEditOutlinerView->ShowCursor();
            }
        }
        ImpMakeTextCursorAreaVisible();
    }
}

namespace
{
    /**
        Helper class to visualize the content of an active EditView as an
        OverlayObject. These objects work with Primitives and are handled
        from the OverlayManager(s) in place as needed.

        It allows complete visualization of the content of the active
        EditView without the need of Invalidates triggered by the EditView
        and thus avoiding potentially expensive repaints by using the
        automatically buffered Overlay mechanism.

        It buffers as much as possible locally and *only* triggers a real
        change (see call to objectChange()) when really needed.
     */
    class TextEditOverlayObject : public sdr::overlay::OverlayObject
    {
    protected:
        /// local access to associated sdr::overlay::OverlaySelection
        sdr::overlay::OverlaySelection*     mpOverlaySelection;

        /// local definition depends on active OutlinerView
        OutlinerView&                       mrOutlinerView;

        /// geometry definitions with buffering
        basegfx::B2DRange                   maLastRange;
        basegfx::B2DRange                   maRange;

        /// text content definitions with buffering
        drawinglayer::primitive2d::Primitive2DContainer     maTextPrimitives;
        drawinglayer::primitive2d::Primitive2DContainer     maLastTextPrimitives;

        /// bitfield
        bool const                    mbVisualizeSurroundingFrame : 1;

        // geometry creation for OverlayObject, can use local *Last* values
        virtual drawinglayer::primitive2d::Primitive2DContainer createOverlayObjectPrimitive2DSequence() override;

    public:
        TextEditOverlayObject(
            const Color& rColor,
            OutlinerView& rOutlinerView,
            bool bVisualizeSurroundingFrame);
        virtual ~TextEditOverlayObject() override;

        // data read access
        const sdr::overlay::OverlaySelection* getOverlaySelection() const { return mpOverlaySelection; }
        const OutlinerView& getOutlinerView() const { return mrOutlinerView; }
        bool getVisualizeSurroundingFrame() const { return mbVisualizeSurroundingFrame; }

        /// override to check conditions for last createOverlayObjectPrimitive2DSequence
        virtual drawinglayer::primitive2d::Primitive2DContainer getOverlayObjectPrimitive2DSequence() const override;

        // data write access. In this OverlayObject we only have the
        // callback that triggers detecting if something *has* changed
        void checkDataChange(const basegfx::B2DRange& rMinTextEditArea);
        void checkSelectionChange();
    };

    drawinglayer::primitive2d::Primitive2DContainer TextEditOverlayObject::createOverlayObjectPrimitive2DSequence()
    {
        drawinglayer::primitive2d::Primitive2DContainer aRetval;

        /// outer frame visualization
        if (getVisualizeSurroundingFrame())
        {
            const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
            const double fTransparence(aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01);
            const sal_uInt16 nPixSiz(getOutlinerView().GetInvalidateMore() - 1);

            aRetval.push_back(
                new drawinglayer::primitive2d::OverlayRectanglePrimitive(
                    maRange,
                    getBaseColor().getBColor(),
                    fTransparence,
                    std::max(6, nPixSiz - 2), // grow
                    0.0, // shrink
                    0.0));
        }

        // add buffered TextPrimitives
        aRetval.append(maTextPrimitives);

        return aRetval;
    }

    TextEditOverlayObject::TextEditOverlayObject(
        const Color& rColor,
        OutlinerView& rOutlinerView,
        bool bVisualizeSurroundingFrame)
    :   OverlayObject(rColor),
        mpOverlaySelection(nullptr),
        mrOutlinerView(rOutlinerView),
        maLastRange(),
        maRange(),
        maTextPrimitives(),
        maLastTextPrimitives(),
        mbVisualizeSurroundingFrame(bVisualizeSurroundingFrame)
    {
        // no AA for TextEdit overlay
        allowAntiAliase(false);

        // create local OverlaySelection - this is an integral part of EditText
        // visualization
        const std::vector< basegfx::B2DRange > aEmptySelection{};
        mpOverlaySelection = new sdr::overlay::OverlaySelection(
            sdr::overlay::OverlayType::Transparent,
            rColor,
            aEmptySelection,
            true);
    }

    TextEditOverlayObject::~TextEditOverlayObject()
    {
        if (getOverlaySelection())
        {
            delete mpOverlaySelection;
            mpOverlaySelection = nullptr;
        }

        if (getOverlayManager())
        {
            getOverlayManager()->remove(*this);
        }
    }

    drawinglayer::primitive2d::Primitive2DContainer TextEditOverlayObject::getOverlayObjectPrimitive2DSequence() const
    {
        if (!getPrimitive2DSequence().empty())
        {
            if (!maRange.equal(maLastRange) || maLastTextPrimitives != maTextPrimitives)
            {
                // conditions of last local decomposition have changed, delete to force new evaluation
                const_cast<TextEditOverlayObject*>(this)->resetPrimitive2DSequence();
            }
        }

        if (getPrimitive2DSequence().empty())
        {
            // remember new buffered values
            const_cast<TextEditOverlayObject*>(this)->maLastRange = maRange;
            const_cast<TextEditOverlayObject*>(this)->maLastTextPrimitives = maTextPrimitives;
        }

        // call base implementation
        return OverlayObject::getOverlayObjectPrimitive2DSequence();
    }

    void TextEditOverlayObject::checkDataChange(const basegfx::B2DRange& rMinTextEditArea)
    {
        bool bObjectChange(false);

        // check current range
        const tools::Rectangle aOutArea(mrOutlinerView.GetOutputArea());
        basegfx::B2DRange aNewRange(aOutArea.Left(), aOutArea.Top(), aOutArea.Right(), aOutArea.Bottom());
        aNewRange.expand(rMinTextEditArea);

        if (aNewRange != maRange)
        {
            maRange = aNewRange;
            bObjectChange = true;
        }

        // check if text primitives did change
        SdrOutliner* pSdrOutliner = dynamic_cast<SdrOutliner*>(getOutlinerView().GetOutliner());

        if (pSdrOutliner)
        {
            // get TextPrimitives directly from active Outliner
            basegfx::B2DHomMatrix aNewTransformA;
            basegfx::B2DHomMatrix aNewTransformB;
            basegfx::B2DRange aClipRange;
            drawinglayer::primitive2d::Primitive2DContainer aNewTextPrimitives;

            // active Outliner is always in unified oriented coordinate system (currently)
            // so just translate to TopLeft of visible Range. Keep in mind that top-left
            // depends on vertical text and top-to-bottom text attributes
            const tools::Rectangle aVisArea(mrOutlinerView.GetVisArea());
            const bool bVerticalWriting(pSdrOutliner->IsVertical());
            const bool bTopToBottom(pSdrOutliner->IsTopToBottom());
            const double fStartInX(
                bVerticalWriting && bTopToBottom
                ? aOutArea.Right() - aVisArea.Left()
                : aOutArea.Left() - aVisArea.Left());
            const double fStartInY(
                bVerticalWriting && !bTopToBottom
                ? aOutArea.Bottom() - aVisArea.Top()
                : aOutArea.Top() - aVisArea.Top());

            aNewTransformB.translate(
                fStartInX,
                fStartInY);

            // get the current TextPrimitives. This is the most expensive part
            // of this mechanism, it *may* be possible to buffer layouted
            // primitives per ParaPortion with/in/dependent on the EditEngine
            // content if needed. For now, get and compare
            SdrTextObj::impDecomposeBlockTextPrimitiveDirect(
                aNewTextPrimitives,
                *pSdrOutliner,
                aNewTransformA,
                aNewTransformB,
                aClipRange);

            if (aNewTextPrimitives != maTextPrimitives)
            {
                maTextPrimitives = aNewTextPrimitives;
                bObjectChange = true;
            }
        }

        if (bObjectChange)
        {
            // if there really *was* a change signal the OverlayManager to
            // refresh this object's visualization
            objectChange();

            // on data change, always do a SelectionChange, too
            // since the selection is an integral part of text visualization
            checkSelectionChange();
        }
    }

    void TextEditOverlayObject::checkSelectionChange()
    {
        if (getOverlaySelection() && getOverlayManager())
        {
            std::vector<tools::Rectangle> aLogicRects;
            std::vector<basegfx::B2DRange> aLogicRanges;
            const Size aLogicPixel(getOverlayManager()->getOutputDevice().PixelToLogic(Size(1, 1)));

            // get logic selection
            getOutlinerView().GetSelectionRectangles(aLogicRects);

            aLogicRanges.reserve(aLogicRects.size());
            for (const auto& aRect : aLogicRects)
            {
                // convert from logic Rectangles to logic Ranges, do not forget to add
                // one Unit (in this case logical units for one pixel, pre-calculated)
                aLogicRanges.emplace_back(
                        aRect.Left() - aLogicPixel.Width(), aRect.Top() - aLogicPixel.Height(),
                        aRect.Right() + aLogicPixel.Width(), aRect.Bottom() + aLogicPixel.Height());
            }

            mpOverlaySelection->setRanges(aLogicRanges);
        }
    }
} // end of anonymous namespace

// TextEdit

// callback from the active EditView, forward to evtl. existing instances of the
// TextEditOverlayObject(s). This will additionally update the selection which
// is an integral part of the text visualization
void SdrObjEditView::EditViewInvalidate() const
{
    if (IsTextEdit())
    {
        // MinTextRange may have changed. Forward it, too
        const basegfx::B2DRange aMinTextRange(
            aMinTextEditArea.Left(), aMinTextEditArea.Top(),
            aMinTextEditArea.Right(), aMinTextEditArea.Bottom());

        for (sal_uInt32 a(0); a < maTEOverlayGroup.count(); a++)
        {
            TextEditOverlayObject* pCandidate = dynamic_cast< TextEditOverlayObject* >(&maTEOverlayGroup.getOverlayObject(a));

            if (pCandidate)
            {
                pCandidate->checkDataChange(aMinTextRange);
            }
        }
    }
}

// callback from the active EditView, forward to evtl. existing instances of the
// TextEditOverlayObject(s). This cvall *only* updates the selection visualization
// which is e.g. used when only the selection is changed, but not the text
void SdrObjEditView::EditViewSelectionChange() const
{
    if (IsTextEdit())
    {
        for (sal_uInt32 a(0); a < maTEOverlayGroup.count(); a++)
        {
            TextEditOverlayObject* pCandidate = dynamic_cast< TextEditOverlayObject* >(&maTEOverlayGroup.getOverlayObject(a));

            if (pCandidate)
            {
                pCandidate->checkSelectionChange();
            }
        }
    }
}

void SdrObjEditView::TextEditDrawing(SdrPaintWindow& rPaintWindow) const
{
    if (!comphelper::LibreOfficeKit::isActive())
    {
        // adapt all TextEditOverlayObject(s), so call EditViewInvalidate()
        // to update accordingly (will update selection, too). Suppress new
        // stuff when LibreOfficeKit is active
        EditViewInvalidate();
    }
    else
    {
        // draw old text edit stuff
        if (IsTextEdit())
        {
            const SdrOutliner* pActiveOutliner = GetTextEditOutliner();

            if (pActiveOutliner)
            {
                const sal_uInt32 nViewCount(pActiveOutliner->GetViewCount());

                if (nViewCount)
                {
                    const vcl::Region& rRedrawRegion = rPaintWindow.GetRedrawRegion();
                    const tools::Rectangle aCheckRect(rRedrawRegion.GetBoundRect());

                    for (sal_uInt32 i(0); i < nViewCount; i++)
                    {
                        OutlinerView* pOLV = pActiveOutliner->GetView(i);

                        // If rPaintWindow knows that the output device is a render
                        // context and is aware of the underlying vcl::Window,
                        // compare against that; that's how double-buffering can
                        // still find the matching OutlinerView.
                        OutputDevice* pOutputDevice = rPaintWindow.GetWindow() ? rPaintWindow.GetWindow() : &rPaintWindow.GetOutputDevice();
                        if (pOLV->GetWindow() == pOutputDevice || comphelper::LibreOfficeKit::isActive())
                        {
                            ImpPaintOutlinerView(*pOLV, aCheckRect, rPaintWindow.GetTargetOutputDevice());
                            return;
                        }
                    }
                }
            }
        }
    }
}

void SdrObjEditView::ImpPaintOutlinerView(OutlinerView& rOutlView, const tools::Rectangle& rRect, OutputDevice& rTargetDevice) const
{
    const SdrTextObj* pText = GetTextEditObject();
    bool bTextFrame(pText && pText->IsTextFrame());
    bool bFitToSize(pTextEditOutliner->GetControlWord() & EEControlBits::STRETCHING);
    bool bModified(pTextEditOutliner->IsModified());
    tools::Rectangle aBlankRect(rOutlView.GetOutputArea());
    aBlankRect.Union(aMinTextEditArea);
    tools::Rectangle aPixRect(rTargetDevice.LogicToPixel(aBlankRect));

    // in the tiled rendering case, the setup is incomplete, and we very
    // easily get an empty rRect on input - that will cause that everything is
    // clipped; happens in case of editing text inside a shape in Calc.
    // FIXME would be better to complete the setup so that we don't get an
    // empty rRect here
    if (!comphelper::LibreOfficeKit::isActive() || !rRect.IsEmpty())
        aBlankRect.Intersection(rRect);

    rOutlView.GetOutliner()->SetUpdateMode(true); // Bugfix #22596#
    rOutlView.Paint(aBlankRect, &rTargetDevice);

    if(!bModified)
    {
        pTextEditOutliner->ClearModifyFlag();
    }

    if(bTextFrame && !bFitToSize)
    {
        // completely reworked to use primitives; this ensures same look and functionality
        const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
        std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> xProcessor(drawinglayer::processor2d::createProcessor2DFromOutputDevice(
            rTargetDevice,
            aViewInformation2D));

        if (xProcessor)
        {
            const bool bMapModeEnabled(rTargetDevice.IsMapModeEnabled());
            const basegfx::B2DRange aRange(aPixRect.Left(), aPixRect.Top(), aPixRect.Right(), aPixRect.Bottom());
            const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
            const Color aHilightColor(aSvtOptionsDrawinglayer.getHilightColor());
            const double fTransparence(aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01);
            const sal_uInt16 nPixSiz(rOutlView.GetInvalidateMore() - 1);
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::OverlayRectanglePrimitive(
                    aRange,
                    aHilightColor.getBColor(),
                    fTransparence,
                    std::max(6, nPixSiz - 2), // grow
                    0.0, // shrink
                    0.0));
            const drawinglayer::primitive2d::Primitive2DContainer aSequence { xReference };

            rTargetDevice.EnableMapMode(false);
            xProcessor->process(aSequence);
            rTargetDevice.EnableMapMode(bMapModeEnabled);
        }
    }

    rOutlView.ShowCursor(/*bGotoCursor=*/true, /*bActivate=*/true);
}

void SdrObjEditView::ImpInvalidateOutlinerView(OutlinerView const & rOutlView) const
{
    vcl::Window* pWin = rOutlView.GetWindow();

    if(pWin)
    {
        const SdrTextObj* pText = GetTextEditObject();
        bool bTextFrame(pText && pText->IsTextFrame());
        bool bFitToSize(pText && pText->IsFitToSize());

        if(bTextFrame && !bFitToSize)
        {
            tools::Rectangle aBlankRect(rOutlView.GetOutputArea());
            aBlankRect.Union(aMinTextEditArea);
            tools::Rectangle aPixRect(pWin->LogicToPixel(aBlankRect));
            sal_uInt16 nPixSiz(rOutlView.GetInvalidateMore() - 1);

            aPixRect.AdjustLeft( -1 );
            aPixRect.AdjustTop( -1 );
            aPixRect.AdjustRight( 1 );
            aPixRect.AdjustBottom( 1 );

            {
                // limit xPixRect because of driver problems when pixel coordinates are too far out
                Size aMaxXY(pWin->GetOutputSizePixel());
                long a(2 * nPixSiz);
                long nMaxX(aMaxXY.Width() + a);
                long nMaxY(aMaxXY.Height() + a);

                if (aPixRect.Left  ()<-a) aPixRect.SetLeft(-a );
                if (aPixRect.Top   ()<-a) aPixRect.SetTop(-a );
                if (aPixRect.Right ()>nMaxX) aPixRect.SetRight(nMaxX );
                if (aPixRect.Bottom()>nMaxY) aPixRect.SetBottom(nMaxY );
            }

            tools::Rectangle aOuterPix(aPixRect);
            aOuterPix.AdjustLeft( -nPixSiz );
            aOuterPix.AdjustTop( -nPixSiz );
            aOuterPix.AdjustRight(nPixSiz );
            aOuterPix.AdjustBottom(nPixSiz );

            bool bMapModeEnabled(pWin->IsMapModeEnabled());
            pWin->EnableMapMode(false);
            pWin->Invalidate(aOuterPix);
            pWin->EnableMapMode(bMapModeEnabled);
        }
    }
}

OutlinerView* SdrObjEditView::ImpMakeOutlinerView(vcl::Window* pWin, OutlinerView* pGivenView, SfxViewShell* pViewShell) const
{
    // background
    Color aBackground(GetTextEditBackgroundColor(*this));
    SdrTextObj* pText = mxTextEditObj.get();
    bool bTextFrame=pText!=nullptr && pText->IsTextFrame();
    bool bContourFrame=pText!=nullptr && pText->IsContourTextFrame();
    // create OutlinerView
    OutlinerView* pOutlView=pGivenView;
    pTextEditOutliner->SetUpdateMode(false);

    if (pOutlView == nullptr)
    {
        pOutlView = new OutlinerView(pTextEditOutliner.get(), pWin);
    }
    else
    {
        pOutlView->SetWindow(pWin);
    }

    // disallow scrolling
    EVControlBits nStat=pOutlView->GetControlWord();
    nStat&=~EVControlBits::AUTOSCROLL;
    // AutoViewSize only if not ContourFrame.
    if (!bContourFrame) nStat|=EVControlBits::AUTOSIZE;
    if (bTextFrame) {
        sal_uInt16 nPixSiz=maHdlList.GetHdlSize()*2+1;
        nStat|=EVControlBits::INVONEMORE;
        pOutlView->SetInvalidateMore(nPixSiz);
    }
    pOutlView->SetControlWord(nStat);
    pOutlView->SetBackgroundColor( aBackground );

    // In case we're in the process of constructing a new view shell,
    // SfxViewShell::Current() may still point to the old one. So if possible,
    // depend on the application owning this draw view to provide the view
    // shell.
    SfxViewShell* pSfxViewShell = pViewShell ? pViewShell : GetSfxViewShell();
    pOutlView->RegisterViewShell(pSfxViewShell ? pSfxViewShell : SfxViewShell::Current());

    if (pText!=nullptr)
    {
        pOutlView->SetAnchorMode(pText->GetOutlinerViewAnchorMode());
        pTextEditOutliner->SetFixedCellHeight(pText->GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT).GetValue());
    }
    // do update before setting output area so that aTextEditArea can be recalculated
    pTextEditOutliner->SetUpdateMode(true);
    pOutlView->SetOutputArea(aTextEditArea);
    ImpInvalidateOutlinerView(*pOutlView);
    return pOutlView;
}

IMPL_LINK(SdrObjEditView,ImpOutlinerStatusEventHdl, EditStatus&, rEditStat, void)
{
    if(pTextEditOutliner )
    {
        SdrTextObj* pTextObj = mxTextEditObj.get();
        if( pTextObj )
        {
            pTextObj->onEditOutlinerStatusEvent( &rEditStat );
        }
    }
}

void SdrObjEditView::ImpChainingEventHdl()
{
    if(pTextEditOutliner )
    {
        SdrTextObj* pTextObj = mxTextEditObj.get();
        OutlinerView* pOLV = GetTextEditOutlinerView();
        if( pTextObj && pOLV)
        {
            TextChain *pTextChain = pTextObj->GetTextChain();

             // XXX: IsChainable and GetNilChainingEvent are a bit mixed up atm
            if (!pTextObj->IsChainable()) {
                return;
            }
            // This is true during an underflow-caused overflow (with pEdtOutl->SetText())
            if (pTextChain->GetNilChainingEvent(pTextObj)) {
                return;
            }

            // We prevent to trigger further handling of overflow/underflow for pTextObj
            pTextChain->SetNilChainingEvent(pTextObj, true); // XXX

            // Save previous selection pos // NOTE: It must be done to have the right CursorEvent in KeyInput
            pTextChain->SetPreChainingSel(pTextObj, pOLV->GetSelection());
            //maPreChainingSel = new ESelection(pOLV->GetSelection());

            // Handling Undo
            const int nText = 0; // XXX: hardcoded index (SdrTextObj::getText handles only 0)

            const bool bUndoEnabled = GetModel() && IsUndoEnabled();
            std::unique_ptr<SdrUndoObjSetText> pTxtUndo;
            if (bUndoEnabled)
                pTxtUndo.reset(dynamic_cast< SdrUndoObjSetText* >
                                ( GetModel()->GetSdrUndoFactory().CreateUndoObjectSetText(*pTextObj, nText ).release() ));

            // trigger actual chaining
            pTextObj->onChainingEvent();

            if (pTxtUndo)
            {
                pTxtUndo->AfterSetText();
                if (!pTxtUndo->IsDifferent())
                {
                    pTxtUndo.reset();
                }
            }

            if (pTxtUndo)
                AddUndo(std::move(pTxtUndo));

            //maCursorEvent = new CursorChainingEvent(pTextChain->GetCursorEvent(pTextObj));
            //SdrTextObj *pNextLink = pTextObj->GetNextLinkInChain();

            // NOTE: Must be called. Don't let the function return if you set it to true and not reset it
            pTextChain->SetNilChainingEvent(pTextObj, false);
        } else {
            // XXX
            SAL_INFO("svx.chaining", "[OnChaining] No Edit Outliner View");
        }
    }

}

IMPL_LINK_NOARG(SdrObjEditView,ImpAfterCutOrPasteChainingEventHdl, LinkParamNone*, void)
{
    SdrTextObj* pTextObj = GetTextEditObject();
    if (!pTextObj)
        return;
    ImpChainingEventHdl();
    TextChainCursorManager aCursorManager(this, pTextObj);
    ImpMoveCursorAfterChainingEvent(&aCursorManager);
}

void SdrObjEditView::ImpMoveCursorAfterChainingEvent(TextChainCursorManager *pCursorManager)
{
    if (!mxTextEditObj.is() || !pCursorManager)
        return;

    SdrTextObj* pTextObj = mxTextEditObj.get();

    // Check if it has links to move it to
    if (!pTextObj || !pTextObj->IsChainable())
        return;

    TextChain *pTextChain = pTextObj->GetTextChain();
    ESelection aNewSel = pTextChain->GetPostChainingSel(pTextObj);

    pCursorManager->HandleCursorEventAfterChaining(
        pTextChain->GetCursorEvent(pTextObj),
        aNewSel);

    // Reset event
    pTextChain->SetCursorEvent(pTextObj, CursorChainingEvent::NULL_EVENT);
}


IMPL_LINK(SdrObjEditView,ImpOutlinerCalcFieldValueHdl,EditFieldInfo*,pFI,void)
{
    bool bOk=false;
    OUString& rStr=pFI->GetRepresentation();
    rStr.clear();
    SdrTextObj* pTextObj = mxTextEditObj.get();
    if (pTextObj!=nullptr) {
        boost::optional<Color> pTxtCol;
        boost::optional<Color> pFldCol;
        bOk=pTextObj->CalcFieldValue(pFI->GetField(),pFI->GetPara(),pFI->GetPos(),true,pTxtCol,pFldCol,rStr);
        if (bOk) {
            if (pTxtCol) {
                pFI->SetTextColor(*pTxtCol);
            }
            if (pFldCol) {
                pFI->SetFieldColor(*pFldCol);
            } else {
                pFI->SetFieldColor(COL_LIGHTGRAY); // TODO: remove this later on (357)
            }
        }
    }
    Outliner& rDrawOutl=mpModel->GetDrawOutliner(pTextObj);
    Link<EditFieldInfo*,void> aDrawOutlLink=rDrawOutl.GetCalcFieldValueHdl();
    if (!bOk && aDrawOutlLink.IsSet()) {
        aDrawOutlLink.Call(pFI);
        bOk = !rStr.isEmpty();
    }
    if (!bOk) {
        aOldCalcFieldValueLink.Call(pFI);
    }
}

IMPL_LINK_NOARG(SdrObjEditView, EndTextEditHdl, SdrUndoManager*, void)
{
    SdrEndTextEdit();
}

SdrUndoManager* SdrObjEditView::getSdrUndoManagerForEnhancedTextEdit() const
{
    // default returns registered UndoManager
    return GetModel() ? dynamic_cast< SdrUndoManager* >(GetModel()->GetSdrUndoManager()) : nullptr;
}

bool SdrObjEditView::SdrBeginTextEdit(
    SdrObject* pObj_, SdrPageView* pPV, vcl::Window* pWin,
    bool bIsNewObj, SdrOutliner* pGivenOutliner,
    OutlinerView* pGivenOutlinerView,
    bool bDontDeleteOutliner, bool bOnlyOneView,
    bool bGrabFocus)
{
    assert(!IsTextEdit());
    // FIXME this encourages all sorts of bad habits and should be removed
    SdrEndTextEdit();

    SdrTextObj* pObj = dynamic_cast< SdrTextObj* >( pObj_ );
    if( !pObj )
        return false; // currently only possible with text objects

    if(bGrabFocus && pWin)
    {
        // attention, this call may cause an EndTextEdit() call to this view
        pWin->GrabFocus(); // to force the cursor into the edit view
    }

    bTextEditDontDelete=bDontDeleteOutliner && pGivenOutliner!=nullptr;
    bTextEditOnlyOneView=bOnlyOneView;
    bTextEditNewObj=bIsNewObj;
    const sal_uInt32 nWinCount(PaintWindowCount());
    sal_uInt32 i;
    bool bBrk(false);

    if(!pWin)
    {
        for(i = 0; i < nWinCount && !pWin; i++)
        {
            SdrPaintWindow* pPaintWindow = GetPaintWindow(i);

            if(OUTDEV_WINDOW == pPaintWindow->GetOutputDevice().GetOutDevType())
            {
                pWin = static_cast<vcl::Window*>(&pPaintWindow->GetOutputDevice());
            }
        }

        // break, when no window exists
        if(!pWin)
        {
            bBrk = true;
        }
    }

    if(!bBrk && !pPV)
    {
        pPV = GetSdrPageView();

        // break, when no PageView for the object exists
        if(!pPV)
        {
            bBrk = true;
        }
    }

    // no TextEdit on objects in locked Layer
    if(pPV && pPV->GetLockedLayers().IsSet(pObj->GetLayer()))
    {
        bBrk = true;
    }

    if(pTextEditOutliner)
    {
        OSL_FAIL("SdrObjEditView::SdrBeginTextEdit(): Old Outliner still exists.");
        pTextEditOutliner.reset();
    }

    if(!bBrk)
    {
        pTextEditWin=pWin;
        pTextEditPV=pPV;
        mxTextEditObj.reset( pObj );
        if (pGivenOutliner)
        {
            pTextEditOutliner.reset(pGivenOutliner);
            pGivenOutliner = nullptr; // so we don't delete it on the error path
        }
        else
            pTextEditOutliner = SdrMakeOutliner( OutlinerMode::TextObject, mxTextEditObj->getSdrModelFromSdrObject() );

        {
            SvtAccessibilityOptions aOptions;
            pTextEditOutliner->ForceAutoColor( aOptions.GetIsAutomaticFontColor() );
        }

        aOldCalcFieldValueLink=pTextEditOutliner->GetCalcFieldValueHdl();
        // FieldHdl has to be set by SdrBeginTextEdit, because this call an UpdateFields
        pTextEditOutliner->SetCalcFieldValueHdl(LINK(this,SdrObjEditView,ImpOutlinerCalcFieldValueHdl));
        pTextEditOutliner->SetBeginPasteOrDropHdl(LINK(this,SdrObjEditView,BeginPasteOrDropHdl));
        pTextEditOutliner->SetEndPasteOrDropHdl(LINK(this,SdrObjEditView,EndPasteOrDropHdl));

        // It is just necessary to make the visualized page known. Set it.
        pTextEditOutliner->setVisualizedPage(pPV->GetPage());

        pTextEditOutliner->SetTextObjNoInit( mxTextEditObj.get() );

        if(mxTextEditObj->BegTextEdit(*pTextEditOutliner))
        {
            SdrTextObj* pTextObj = mxTextEditObj.get();
            DBG_ASSERT( pTextObj, "svx::SdrObjEditView::BegTextEdit(), no text object?" );
            if( !pTextObj )
                return false;

            // switch off any running TextAnimations
            pTextObj->SetTextAnimationAllowed(false);

            // remember old cursor
            if (pTextEditOutliner->GetViewCount()!=0)
            {
                pTextEditOutliner->RemoveView(static_cast<size_t>(0));
            }

            // Determine EditArea via TakeTextEditArea.
            // TODO: This could theoretically be left out, because TakeTextRect() calculates the aTextEditArea,
            // but aMinTextEditArea has to happen, too (therefore leaving this in right now)
            pTextObj->TakeTextEditArea(nullptr,nullptr,&aTextEditArea,&aMinTextEditArea);

            tools::Rectangle aTextRect;
            tools::Rectangle aAnchorRect;
            pTextObj->TakeTextRect(*pTextEditOutliner, aTextRect, true,
                &aAnchorRect /* Give true here, not false */);

            if ( !pTextObj->IsContourTextFrame() )
            {
                // FitToSize not together with ContourFrame, for now
                if (pTextObj->IsFitToSize())
                    aTextRect = aAnchorRect;
            }

            aTextEditArea = aTextRect;

            // add possible GridOffset to up-to-now view-independent EditAreas
            basegfx::B2DVector aGridOffset(0.0, 0.0);
            if(getPossibleGridOffsetForSdrObject(aGridOffset, pTextObj, pPV))
            {
                const Point aOffset(
                    basegfx::fround(aGridOffset.getX()),
                    basegfx::fround(aGridOffset.getY()));

                aTextEditArea += aOffset;
                aMinTextEditArea += aOffset;
            }

            Point aPvOfs(pTextObj->GetTextEditOffset());
            aTextEditArea.Move(aPvOfs.X(),aPvOfs.Y());
            aMinTextEditArea.Move(aPvOfs.X(),aPvOfs.Y());
            pTextEditCursorBuffer=pWin->GetCursor();

            maHdlList.SetMoveOutside(true);

            // Since IsMarkHdlWhenTextEdit() is ignored, it is necessary
            // to call AdjustMarkHdl() always.
            AdjustMarkHdl();

            pTextEditOutlinerView=ImpMakeOutlinerView(pWin,pGivenOutlinerView);

            if (!comphelper::LibreOfficeKit::isActive() && pTextEditOutlinerView)
            {
                // activate visualization of EditView on Overlay, suppress when
                // LibreOfficeKit is active
                pTextEditOutlinerView->GetEditView().setEditViewCallbacks(this);

                const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
                const Color aHilightColor(aSvtOptionsDrawinglayer.getHilightColor());
                const SdrTextObj* pText = GetTextEditObject();
                const bool bTextFrame(pText && pText->IsTextFrame());
                const bool bFitToSize(pTextEditOutliner->GetControlWord() & EEControlBits::STRETCHING);
                const bool bVisualizeSurroundingFrame(bTextFrame && !bFitToSize);
                SdrPageView* pPageView = GetSdrPageView();

                if (pPageView)
                {
                    for (sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
                    {
                        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                        if (rPageWindow.GetPaintWindow().OutputToWindow())
                        {
                            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
                            if (xManager.is())
                            {
                                std::unique_ptr<TextEditOverlayObject> pNewTextEditOverlayObject(new TextEditOverlayObject(
                                    aHilightColor,
                                    *pTextEditOutlinerView,
                                    bVisualizeSurroundingFrame));

                                xManager->add(*pNewTextEditOverlayObject);
                                xManager->add(const_cast<sdr::overlay::OverlaySelection&>(*pNewTextEditOverlayObject->getOverlaySelection()));

                                maTEOverlayGroup.append(std::move(pNewTextEditOverlayObject));
                            }
                        }
                    }
                }
            }

            // check if this view is already inserted
            sal_uIntPtr i2,nCount = pTextEditOutliner->GetViewCount();
            for( i2 = 0; i2 < nCount; i2++ )
            {
                if( pTextEditOutliner->GetView(i2) == pTextEditOutlinerView )
                    break;
            }

            if( i2 == nCount )
                pTextEditOutliner->InsertView(pTextEditOutlinerView,0);

            maHdlList.SetMoveOutside(false);
            maHdlList.SetMoveOutside(true);

            // register all windows as OutlinerViews with the Outliner
            if(!bOnlyOneView)
            {
                for(i = 0; i < nWinCount; i++)
                {
                    SdrPaintWindow* pPaintWindow = GetPaintWindow(i);
                    OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();

                    if(&rOutDev != pWin && OUTDEV_WINDOW == rOutDev.GetOutDevType())
                    {
                        OutlinerView* pOutlView = ImpMakeOutlinerView(static_cast<vcl::Window*>(&rOutDev), nullptr);
                        pTextEditOutliner->InsertView(pOutlView, static_cast<sal_uInt16>(i));
                    }
                }

                if (comphelper::LibreOfficeKit::isActive())
                {
                    // Register an outliner view for all other sdr views that
                    // show the same page, so that when the text edit changes,
                    // all interested windows get an invalidation.
                    SdrViewIter aIter(pObj->getSdrPageFromSdrObject());
                    for (SdrView* pView = aIter.FirstView(); pView; pView = aIter.NextView())
                    {
                        if (pView == this)
                            continue;

                        for(sal_uInt32 nViewPaintWindow = 0; nViewPaintWindow < pView->PaintWindowCount(); ++nViewPaintWindow)
                        {
                            SdrPaintWindow* pPaintWindow = pView->GetPaintWindow(nViewPaintWindow);
                            OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();

                            if(&rOutDev != pWin && OUTDEV_WINDOW == rOutDev.GetOutDevType())
                            {
                                OutlinerView* pOutlView = ImpMakeOutlinerView(static_cast<vcl::Window*>(&rOutDev), nullptr);
                                pOutlView->HideCursor();
                                static_cast<vcl::Window*>(&rOutDev)->SetCursor(nullptr);
                                pTextEditOutliner->InsertView(pOutlView);
                            }
                        }
                    }
                }
            }

            pTextEditOutlinerView->ShowCursor();
            pTextEditOutliner->SetStatusEventHdl(LINK(this,SdrObjEditView,ImpOutlinerStatusEventHdl));
            if (pTextObj->IsChainable()) {
                pTextEditOutlinerView->SetEndCutPasteLinkHdl(LINK(this,SdrObjEditView,ImpAfterCutOrPasteChainingEventHdl) );
            }

#ifdef DBG_UTIL
            if (mpItemBrowser!=nullptr) mpItemBrowser->SetDirty();
#endif
            pTextEditOutliner->ClearModifyFlag();

            if (pTextObj->IsFitToSize())
            {
                pWin->Invalidate(aTextEditArea);
            }

            if( GetModel() )
            {
                SdrHint aHint(SdrHintKind::BeginEdit, *pTextObj);
                GetModel()->Broadcast(aHint);
            }

            pTextEditOutliner->setVisualizedPage(nullptr);

            if( mxSelectionController.is() )
                mxSelectionController->onSelectionHasChanged();

            if (GetModel() && IsUndoEnabled() && !GetModel()->GetDisableTextEditUsesCommonUndoManager())
            {
                SdrUndoManager* pSdrUndoManager = getSdrUndoManagerForEnhancedTextEdit();

                if(pSdrUndoManager)
                {
                    // we have an outliner, undo manager and it's an EditUndoManager, exchange
                    // the document undo manager and the default one from the outliner and tell
                    // it that text edit starts by setting a callback if it needs to end text edit mode.
                    assert(nullptr == mpOldTextEditUndoManager);

                    mpOldTextEditUndoManager = pTextEditOutliner->SetUndoManager(pSdrUndoManager);
                    pSdrUndoManager->SetEndTextEditHdl(LINK(this, SdrObjEditView, EndTextEditHdl));
                }
                else
                {
                    OSL_ENSURE(false, "The document undo manager is not derived from SdrUndoManager (!)");
                }
            }

            return true; // ran fine, let TextEdit run now
        }
        else
        {
            pTextEditOutliner->SetCalcFieldValueHdl(aOldCalcFieldValueLink);
            pTextEditOutliner->SetBeginPasteOrDropHdl(Link<PasteOrDropInfos*,void>());
            pTextEditOutliner->SetEndPasteOrDropHdl(Link<PasteOrDropInfos*,void>());
        }
    }
    if (pTextEditOutliner != nullptr)
    {
        pTextEditOutliner->setVisualizedPage(nullptr);
    }

    // something went wrong...
    if(!bDontDeleteOutliner)
    {
        delete pGivenOutliner;
        if(pGivenOutlinerView!=nullptr)
        {
            delete pGivenOutlinerView;
            pGivenOutlinerView = nullptr;
        }
    }
    pTextEditOutliner.reset();

    pTextEditOutlinerView=nullptr;
    mxTextEditObj.reset(nullptr);
    pTextEditPV=nullptr;
    pTextEditWin=nullptr;
    maHdlList.SetMoveOutside(false);

    return false;
}

SdrEndTextEditKind SdrObjEditView::SdrEndTextEdit(bool bDontDeleteReally)
{
    SdrEndTextEditKind eRet=SdrEndTextEditKind::Unchanged;
    SdrTextObj* pTEObj = mxTextEditObj.get();
    vcl::Window*       pTEWin         =pTextEditWin;
    OutlinerView* pTEOutlinerView=pTextEditOutlinerView;
    vcl::Cursor*  pTECursorBuffer=pTextEditCursorBuffer;
    SdrUndoManager* pUndoEditUndoManager = nullptr;
    bool bNeedToUndoSavedRedoTextEdit(false);

    if (GetModel() && IsUndoEnabled() && pTEObj && pTextEditOutliner && !GetModel()->GetDisableTextEditUsesCommonUndoManager())
    {
        // change back the UndoManager to the remembered original one
        SfxUndoManager* pOriginal = pTextEditOutliner->SetUndoManager(mpOldTextEditUndoManager);
        mpOldTextEditUndoManager = nullptr;

        if(pOriginal)
        {
            // check if we got back our document undo manager
            SdrUndoManager* pSdrUndoManager = getSdrUndoManagerForEnhancedTextEdit();

            if(pSdrUndoManager && dynamic_cast< SdrUndoManager* >(pOriginal) == pSdrUndoManager)
            {
                if(pSdrUndoManager->isEndTextEditTriggeredFromUndo())
                {
                    // remember the UndoManager where missing Undos have to be triggered after end
                    // text edit. When the undo had triggered the end text edit, the original action
                    // which had to be undone originally is not yet undone.
                    pUndoEditUndoManager = pSdrUndoManager;

                    // We are ending text edit; if text edit was triggered from undo, execute all redos
                    // to create a complete text change undo action for the redo buffer. Also mark this
                    // state when at least one redo was executed; the created extra TextChange needs to
                    // be undone in addition to the first real undo outside the text edit changes
                    while(pSdrUndoManager->GetRedoActionCount())
                    {
                        bNeedToUndoSavedRedoTextEdit = true;
                        pSdrUndoManager->Redo();
                    }
                }

                // reset the callback link and let the undo manager cleanup all text edit
                // undo actions to get the stack back to the form before the text edit
                pSdrUndoManager->SetEndTextEditHdl(Link<SdrUndoManager*,void>());
            }
            else
            {
                OSL_ENSURE(false, "Got UndoManager back in SdrEndTextEdit which is NOT the expected document UndoManager (!)");
                delete pOriginal;
            }
        }
    }
    else
    {
        assert(nullptr == mpOldTextEditUndoManager); // cannot be restored!
    }

    if( GetModel() && mxTextEditObj.is() )
    {
        SdrHint aHint(SdrHintKind::EndEdit, *mxTextEditObj.get());
        GetModel()->Broadcast(aHint);
    }

    // if new mechanism was used, clean it up. At cleanup no need to check
    // for LibreOfficeKit
    if (pTextEditOutlinerView)
    {
        pTextEditOutlinerView->GetEditView().setEditViewCallbacks(nullptr);
        maTEOverlayGroup.clear();
    }

    mxTextEditObj.reset(nullptr);
    pTextEditPV=nullptr;
    pTextEditWin=nullptr;
    SdrOutliner* pTEOutliner = pTextEditOutliner.release();
    pTextEditOutlinerView=nullptr;
    pTextEditCursorBuffer=nullptr;
    aTextEditArea=tools::Rectangle();

    if (pTEOutliner!=nullptr)
    {
        bool bModified=pTEOutliner->IsModified();
        if (pTEOutlinerView!=nullptr)
        {
            pTEOutlinerView->HideCursor();
        }
        if (pTEObj!=nullptr)
        {
            pTEOutliner->CompleteOnlineSpelling();

            std::unique_ptr<SdrUndoObjSetText> pTxtUndo;

            if( bModified )
            {
                sal_Int32 nText;
                for( nText = 0; nText < pTEObj->getTextCount(); ++nText )
                    if( pTEObj->getText( nText ) == pTEObj->getActiveText() )
                        break;

                pTxtUndo.reset( dynamic_cast< SdrUndoObjSetText* >( GetModel()->GetSdrUndoFactory().CreateUndoObjectSetText(*pTEObj, nText ).release() ) );
            }
            DBG_ASSERT( !bModified || pTxtUndo, "svx::SdrObjEditView::EndTextEdit(), could not create undo action!" );
            // Set old CalcFieldValue-Handler again, this
            // has to happen before Obj::EndTextEdit(), as this does UpdateFields().
            pTEOutliner->SetCalcFieldValueHdl(aOldCalcFieldValueLink);
            pTEOutliner->SetBeginPasteOrDropHdl(Link<PasteOrDropInfos*,void>());
            pTEOutliner->SetEndPasteOrDropHdl(Link<PasteOrDropInfos*,void>());

            const bool bUndo = IsUndoEnabled();
            if( bUndo )
            {
                OUString aObjName(pTEObj->TakeObjNameSingul());
                BegUndo(SvxResId(STR_UndoObjSetText),aObjName);
            }

            pTEObj->EndTextEdit(*pTEOutliner);

            if ((pTEObj->GetRotateAngle() != 0)
                || (dynamic_cast<const SdrTextObj*>(pTEObj) != nullptr && pTEObj->IsFontwork()))
            {
                pTEObj->ActionChanged();
            }

            if (pTxtUndo!=nullptr)
            {
                pTxtUndo->AfterSetText();
                if (!pTxtUndo->IsDifferent())
                {
                    pTxtUndo.reset();
                }
            }
            // check deletion of entire TextObj
            std::unique_ptr<SdrUndoAction> pDelUndo;
            bool bDelObj=false;
            if (bTextEditNewObj)
            {
                bDelObj=pTEObj->IsTextFrame() &&
                        !pTEObj->HasText() &&
                        !pTEObj->IsEmptyPresObj() &&
                        !pTEObj->HasFill() &&
                        !pTEObj->HasLine();

                if(pTEObj->IsInserted() && bDelObj && pTEObj->GetObjInventor()==SdrInventor::Default && !bDontDeleteReally)
                {
                    SdrObjKind eIdent=static_cast<SdrObjKind>(pTEObj->GetObjIdentifier());
                    if(eIdent==OBJ_TEXT)
                    {
                        pDelUndo= GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pTEObj);
                    }
                }
            }
            if (pTxtUndo)
            {
                if( bUndo )
                    AddUndo(std::move(pTxtUndo));
                eRet=SdrEndTextEditKind::Changed;
            }
            if (pDelUndo!=nullptr)
            {
                if( bUndo )
                {
                    AddUndo(std::move(pDelUndo));
                }
                eRet=SdrEndTextEditKind::Deleted;
                DBG_ASSERT(pTEObj->getParentSdrObjListFromSdrObject()!=nullptr,"SdrObjEditView::SdrEndTextEdit(): Fatal: Object edited doesn't have an ObjList!");
                if (pTEObj->getParentSdrObjListFromSdrObject()!=nullptr)
                {
                    pTEObj->getParentSdrObjListFromSdrObject()->RemoveObject(pTEObj->GetOrdNum());
                    CheckMarked(); // remove selection immediately...
                }
            }
            else if (bDelObj)
            { // for Writer: the app has to do the deletion itself.
                eRet=SdrEndTextEditKind::ShouldBeDeleted;
            }

            if( bUndo )
                EndUndo(); // EndUndo after Remove, in case UndoStack is deleted immediately

            // Switch on any TextAnimation again after TextEdit
            if(dynamic_cast<const SdrTextObj*>( pTEObj) !=  nullptr)
            {
                pTEObj->SetTextAnimationAllowed(true);
            }

            // Since IsMarkHdlWhenTextEdit() is ignored, it is necessary
            // to call AdjustMarkHdl() always.
            AdjustMarkHdl();
        }
        // delete all OutlinerViews
        for (sal_uIntPtr i=pTEOutliner->GetViewCount(); i>0;)
        {
            i--;
            OutlinerView* pOLV=pTEOutliner->GetView(i);
            sal_uInt16 nMorePix=pOLV->GetInvalidateMore() + 10;
            vcl::Window* pWin=pOLV->GetWindow();
            tools::Rectangle aRect(pOLV->GetOutputArea());
            pTEOutliner->RemoveView(i);
            if (!bTextEditDontDelete || i!=0)
            {
                // may not own the zeroth one
                delete pOLV;
            }
            aRect.Union(aTextEditArea);
            aRect.Union(aMinTextEditArea);
            aRect=pWin->LogicToPixel(aRect);
            aRect.AdjustLeft( -nMorePix );
            aRect.AdjustTop( -nMorePix );
            aRect.AdjustRight(nMorePix );
            aRect.AdjustBottom(nMorePix );
            aRect=pWin->PixelToLogic(aRect);
            InvalidateOneWin(*pWin,aRect);
            pWin->SetFillColor();
            pWin->SetLineColor(COL_BLACK);
        }
        // and now the Outliner itself
        if (!bTextEditDontDelete) delete pTEOutliner;
        else pTEOutliner->Clear();
        if (pTEWin!=nullptr) {
            pTEWin->SetCursor(pTECursorBuffer);
        }
        maHdlList.SetMoveOutside(false);
        if (eRet!=SdrEndTextEditKind::Unchanged)
        {
            GetMarkedObjectListWriteAccess().SetNameDirty();
        }
#ifdef DBG_UTIL
        if (mpItemBrowser)
        {
            GetMarkedObjectListWriteAccess().SetNameDirty();
            mpItemBrowser->SetDirty();
        }
#endif
    }

    if( pTEObj &&
        !pTEObj->getSdrModelFromSdrObject().isLocked() &&
        pTEObj->GetBroadcaster())
    {
        SdrHint aHint(SdrHintKind::EndEdit, *pTEObj);
        const_cast<SfxBroadcaster*>(pTEObj->GetBroadcaster())->Broadcast(aHint);
    }

    if(pUndoEditUndoManager)
    {
        if(bNeedToUndoSavedRedoTextEdit)
        {
            // undo the text edit action since it was created as part of an EndTextEdit
            // callback from undo itself. This needs to be done after the call to
            // FmFormView::SdrEndTextEdit since it gets created there
            pUndoEditUndoManager->Undo();
        }

        // trigger the Undo which was not executed, but lead to this
        // end text edit
        pUndoEditUndoManager->Undo();
    }

    return eRet;
}


// info about TextEdit. Default is false.
bool SdrObjEditView::IsTextEdit() const
{
    return mxTextEditObj.is();
}

// info about TextEditPageView. Default is 0L.
SdrPageView* SdrObjEditView::GetTextEditPageView() const
{
    return pTextEditPV;
}


OutlinerView* SdrObjEditView::ImpFindOutlinerView(vcl::Window const * pWin) const
{
    if (pWin==nullptr) return nullptr;
    if (pTextEditOutliner==nullptr) return nullptr;
    OutlinerView* pNewView=nullptr;
    sal_uIntPtr nWinCount=pTextEditOutliner->GetViewCount();
    for (sal_uIntPtr i=0; i<nWinCount && pNewView==nullptr; i++) {
        OutlinerView* pView=pTextEditOutliner->GetView(i);
        if (pView->GetWindow()==pWin) pNewView=pView;
    }
    return pNewView;
}

void SdrObjEditView::SetTextEditWin(vcl::Window* pWin)
{
    if(mxTextEditObj.is() && pWin!=nullptr && pWin!=pTextEditWin)
    {
        OutlinerView* pNewView=ImpFindOutlinerView(pWin);
        if (pNewView!=nullptr && pNewView!=pTextEditOutlinerView)
        {
            if (pTextEditOutlinerView!=nullptr)
            {
                pTextEditOutlinerView->HideCursor();
            }
            pTextEditOutlinerView=pNewView;
            pTextEditWin=pWin;
            pWin->GrabFocus(); // Make the cursor blink here as well
            pNewView->ShowCursor();
            ImpMakeTextCursorAreaVisible();
        }
    }
}

bool SdrObjEditView::IsTextEditHit(const Point& rHit) const
{
    bool bOk=false;
    if(mxTextEditObj.is())
    {
        tools::Rectangle aEditArea;
        OutlinerView* pOLV=pTextEditOutliner->GetView(0);
        if (pOLV!=nullptr)
        {
            aEditArea.Union(pOLV->GetOutputArea());
        }
        bOk=aEditArea.IsInside(rHit);
        if (bOk)
        { // check if any characters were actually hit
            Point aPnt(rHit); aPnt-=aEditArea.TopLeft();
            long nHitTol = 2000;
            OutputDevice* pRef = pTextEditOutliner->GetRefDevice();
            if( pRef )
                nHitTol = OutputDevice::LogicToLogic( nHitTol, MapUnit::Map100thMM, pRef->GetMapMode().GetMapUnit() );

            bOk = pTextEditOutliner->IsTextPos( aPnt, static_cast<sal_uInt16>(nHitTol) );
        }
    }
    return bOk;
}

bool SdrObjEditView::IsTextEditFrameHit(const Point& rHit) const
{
    bool bOk=false;
    if(mxTextEditObj.is())
    {
        SdrTextObj* pText = mxTextEditObj.get();
        OutlinerView* pOLV=pTextEditOutliner->GetView(0);
        if( pOLV )
        {
            vcl::Window* pWin=pOLV->GetWindow();
            if (pText != nullptr && pText->IsTextFrame() && pWin != nullptr)
            {
                sal_uInt16 nPixSiz=pOLV->GetInvalidateMore();
                tools::Rectangle aEditArea(aMinTextEditArea);
                aEditArea.Union(pOLV->GetOutputArea());
                if (!aEditArea.IsInside(rHit)) {
                    Size aSiz(pWin->PixelToLogic(Size(nPixSiz,nPixSiz)));
                    aEditArea.AdjustLeft( -(aSiz.Width()) );
                    aEditArea.AdjustTop( -(aSiz.Height()) );
                    aEditArea.AdjustRight(aSiz.Width() );
                    aEditArea.AdjustBottom(aSiz.Height() );
                    bOk=aEditArea.IsInside(rHit);
                }
            }
        }
    }
    return bOk;
}

TextChainCursorManager *SdrObjEditView::ImpHandleMotionThroughBoxesKeyInput(
                                            const KeyEvent& rKEvt,
                                            bool *bOutHandled)
{
    *bOutHandled = false;

    SdrTextObj* pTextObj = mxTextEditObj.get();
    if (!pTextObj)
        return nullptr;

    if (!pTextObj->GetNextLinkInChain() && !pTextObj->GetPrevLinkInChain())
        return nullptr;

    TextChainCursorManager *pCursorManager = new TextChainCursorManager(this, pTextObj);
    if( pCursorManager->HandleKeyEvent(rKEvt) ) {
        // Possibly do other stuff here if necessary...
        // XXX: Careful with the checks below (in KeyInput) for pWin and co. You should do them here I guess.
        *bOutHandled = true;
    }

    return pCursorManager;
}

bool SdrObjEditView::KeyInput(const KeyEvent& rKEvt, vcl::Window* pWin)
{
    if(pTextEditOutlinerView)
    {
        /* Start special handling of keys within a chain */
        // We possibly move to another box before any handling
        bool bHandled = false;
        std::unique_ptr<TextChainCursorManager> xCursorManager(
            ImpHandleMotionThroughBoxesKeyInput(rKEvt, &bHandled));
        if (bHandled)
            return true;
        /* End special handling of keys within a chain */

        if (pTextEditOutlinerView->PostKeyEvent(rKEvt, pWin))
        {
            if( mpModel )
            {
                if( pTextEditOutliner && pTextEditOutliner->IsModified() )
                    mpModel->SetChanged();
            }

            /* Start chaining processing */
            ImpChainingEventHdl();
            ImpMoveCursorAfterChainingEvent(xCursorManager.get());
            /* End chaining processing */

            if (pWin!=nullptr && pWin!=pTextEditWin) SetTextEditWin(pWin);
#ifdef DBG_UTIL
            if (mpItemBrowser!=nullptr) mpItemBrowser->SetDirty();
#endif
            ImpMakeTextCursorAreaVisible();
            return true;
        }
    }
    return SdrGlueEditView::KeyInput(rKEvt,pWin);
}

bool SdrObjEditView::MouseButtonDown(const MouseEvent& rMEvt, vcl::Window* pWin)
{
    if (pTextEditOutlinerView!=nullptr) {
        bool bPostIt=pTextEditOutliner->IsInSelectionMode();
        if (!bPostIt) {
            Point aPt(rMEvt.GetPosPixel());
            if (pWin!=nullptr) aPt=pWin->PixelToLogic(aPt);
            else if (pTextEditWin!=nullptr) aPt=pTextEditWin->PixelToLogic(aPt);
            bPostIt=IsTextEditHit(aPt);
        }
        if (bPostIt) {
            Point aPixPos(rMEvt.GetPosPixel());
            if (pWin)
            {
                tools::Rectangle aR(pWin->LogicToPixel(pTextEditOutlinerView->GetOutputArea()));
                if (aPixPos.X()<aR.Left  ()) aPixPos.setX(aR.Left  () );
                if (aPixPos.X()>aR.Right ()) aPixPos.setX(aR.Right () );
                if (aPixPos.Y()<aR.Top   ()) aPixPos.setY(aR.Top   () );
                if (aPixPos.Y()>aR.Bottom()) aPixPos.setY(aR.Bottom() );
            }
            MouseEvent aMEvt(aPixPos,rMEvt.GetClicks(),rMEvt.GetMode(),
                             rMEvt.GetButtons(),rMEvt.GetModifier());
            if (pTextEditOutlinerView->MouseButtonDown(aMEvt)) {
                if (pWin!=nullptr && pWin!=pTextEditWin) SetTextEditWin(pWin);
#ifdef DBG_UTIL
                if (mpItemBrowser!=nullptr) mpItemBrowser->SetDirty();
#endif
                ImpMakeTextCursorAreaVisible();
                return true;
            }
        }
    }
    return SdrGlueEditView::MouseButtonDown(rMEvt,pWin);
}

bool SdrObjEditView::MouseButtonUp(const MouseEvent& rMEvt, vcl::Window* pWin)
{
    if (pTextEditOutlinerView!=nullptr) {
        bool bPostIt=pTextEditOutliner->IsInSelectionMode();
        if (!bPostIt) {
            Point aPt(rMEvt.GetPosPixel());
            if (pWin!=nullptr) aPt=pWin->PixelToLogic(aPt);
            else if (pTextEditWin!=nullptr) aPt=pTextEditWin->PixelToLogic(aPt);
            bPostIt=IsTextEditHit(aPt);
        }
        if (bPostIt && pWin) {
            Point aPixPos(rMEvt.GetPosPixel());
            tools::Rectangle aR(pWin->LogicToPixel(pTextEditOutlinerView->GetOutputArea()));
            if (aPixPos.X()<aR.Left  ()) aPixPos.setX(aR.Left  () );
            if (aPixPos.X()>aR.Right ()) aPixPos.setX(aR.Right () );
            if (aPixPos.Y()<aR.Top   ()) aPixPos.setY(aR.Top   () );
            if (aPixPos.Y()>aR.Bottom()) aPixPos.setY(aR.Bottom() );
            MouseEvent aMEvt(aPixPos,rMEvt.GetClicks(),rMEvt.GetMode(),
                             rMEvt.GetButtons(),rMEvt.GetModifier());
            if (pTextEditOutlinerView->MouseButtonUp(aMEvt)) {
#ifdef DBG_UTIL
                if (mpItemBrowser!=nullptr) mpItemBrowser->SetDirty();
#endif
                ImpMakeTextCursorAreaVisible();
                return true;
            }
        }
    }
    return SdrGlueEditView::MouseButtonUp(rMEvt,pWin);
}

bool SdrObjEditView::MouseMove(const MouseEvent& rMEvt, vcl::Window* pWin)
{
    if (pTextEditOutlinerView!=nullptr) {
        bool bSelMode=pTextEditOutliner->IsInSelectionMode();
        bool bPostIt=bSelMode;
        if (!bPostIt) {
            Point aPt(rMEvt.GetPosPixel());
            if (pWin)
                aPt=pWin->PixelToLogic(aPt);
            else if (pTextEditWin)
                aPt=pTextEditWin->PixelToLogic(aPt);
            bPostIt=IsTextEditHit(aPt);
        }
        if (bPostIt) {
            Point aPixPos(rMEvt.GetPosPixel());
            tools::Rectangle aR(pTextEditOutlinerView->GetOutputArea());
            if (pWin)
                aR = pWin->LogicToPixel(aR);
            else if (pTextEditWin)
                aR = pTextEditWin->LogicToPixel(aR);
            if (aPixPos.X()<aR.Left  ()) aPixPos.setX(aR.Left  () );
            if (aPixPos.X()>aR.Right ()) aPixPos.setX(aR.Right () );
            if (aPixPos.Y()<aR.Top   ()) aPixPos.setY(aR.Top   () );
            if (aPixPos.Y()>aR.Bottom()) aPixPos.setY(aR.Bottom() );
            MouseEvent aMEvt(aPixPos,rMEvt.GetClicks(),rMEvt.GetMode(),
                             rMEvt.GetButtons(),rMEvt.GetModifier());
            if (pTextEditOutlinerView->MouseMove(aMEvt) && bSelMode) {
#ifdef DBG_UTIL
                if (mpItemBrowser!=nullptr) mpItemBrowser->SetDirty();
#endif
                ImpMakeTextCursorAreaVisible();
                return true;
            }
        }
    }
    return SdrGlueEditView::MouseMove(rMEvt,pWin);
}

bool SdrObjEditView::Command(const CommandEvent& rCEvt, vcl::Window* pWin)
{
    // as long as OutlinerView returns a sal_Bool, it only gets CommandEventId::StartDrag
    if (pTextEditOutlinerView!=nullptr)
    {
        if (rCEvt.GetCommand()==CommandEventId::StartDrag) {
            bool bPostIt=pTextEditOutliner->IsInSelectionMode() || !rCEvt.IsMouseEvent();
            if (!bPostIt && rCEvt.IsMouseEvent()) {
                Point aPt(rCEvt.GetMousePosPixel());
                if (pWin!=nullptr) aPt=pWin->PixelToLogic(aPt);
                else if (pTextEditWin!=nullptr) aPt=pTextEditWin->PixelToLogic(aPt);
                bPostIt=IsTextEditHit(aPt);
            }
            if (bPostIt) {
                Point aPixPos(rCEvt.GetMousePosPixel());
                if (rCEvt.IsMouseEvent() && pWin) {
                    tools::Rectangle aR(pWin->LogicToPixel(pTextEditOutlinerView->GetOutputArea()));
                    if (aPixPos.X()<aR.Left  ()) aPixPos.setX(aR.Left  () );
                    if (aPixPos.X()>aR.Right ()) aPixPos.setX(aR.Right () );
                    if (aPixPos.Y()<aR.Top   ()) aPixPos.setY(aR.Top   () );
                    if (aPixPos.Y()>aR.Bottom()) aPixPos.setY(aR.Bottom() );
                }
                CommandEvent aCEvt(aPixPos,rCEvt.GetCommand(),rCEvt.IsMouseEvent());
                // Command is void at the OutlinerView, sadly
                pTextEditOutlinerView->Command(aCEvt);
                if (pWin!=nullptr && pWin!=pTextEditWin) SetTextEditWin(pWin);
#ifdef DBG_UTIL
                if (mpItemBrowser!=nullptr) mpItemBrowser->SetDirty();
#endif
                ImpMakeTextCursorAreaVisible();
                return true;
            }
        }
        else
        {
            pTextEditOutlinerView->Command(rCEvt);
            return true;
        }
    }
    return SdrGlueEditView::Command(rCEvt,pWin);
}


bool SdrObjEditView::ImpIsTextEditAllSelected() const
{
    bool bRet=false;
    if (pTextEditOutliner!=nullptr && pTextEditOutlinerView!=nullptr)
    {
        if(SdrTextObj::HasTextImpl( pTextEditOutliner.get() ) )
        {
            const sal_Int32 nParaCnt=pTextEditOutliner->GetParagraphCount();
            Paragraph* pLastPara=pTextEditOutliner->GetParagraph( nParaCnt > 1 ? nParaCnt - 1 : 0 );

            ESelection aESel(pTextEditOutlinerView->GetSelection());
            if (aESel.nStartPara==0 && aESel.nStartPos==0 && aESel.nEndPara==(nParaCnt-1))
            {
                if( pTextEditOutliner->GetText(pLastPara).getLength() == aESel.nEndPos )
                    bRet = true;
            }
            // in case the selection was done backwards
            if (!bRet && aESel.nEndPara==0 && aESel.nEndPos==0 && aESel.nStartPara==(nParaCnt-1))
            {
                if(pTextEditOutliner->GetText(pLastPara).getLength() == aESel.nStartPos)
                    bRet = true;
            }
        }
        else
        {
            bRet=true;
        }
    }
    return bRet;
}

void SdrObjEditView::ImpMakeTextCursorAreaVisible()
{
    if (pTextEditOutlinerView!=nullptr && pTextEditWin!=nullptr) {
        vcl::Cursor* pCsr=pTextEditWin->GetCursor();
        if (pCsr!=nullptr) {
            Size aSiz(pCsr->GetSize());
            if (aSiz.Width()!=0 && aSiz.Height()!=0) {
                MakeVisible(tools::Rectangle(pCsr->GetPos(),aSiz),*pTextEditWin);
            }
        }
    }
}

SvtScriptType SdrObjEditView::GetScriptType() const
{
    SvtScriptType nScriptType = SvtScriptType::NONE;

    if( IsTextEdit() )
    {
        if( mxTextEditObj->GetOutlinerParaObject() )
            nScriptType = mxTextEditObj->GetOutlinerParaObject()->GetTextObject().GetScriptType();

        if( pTextEditOutlinerView )
            nScriptType = pTextEditOutlinerView->GetSelectedScriptType();
    }
    else
    {
        const size_t nMarkCount( GetMarkedObjectCount() );

        for( size_t i = 0; i < nMarkCount; ++i )
        {
            OutlinerParaObject* pParaObj = GetMarkedObjectByIndex( i )->GetOutlinerParaObject();

            if( pParaObj )
            {
                nScriptType |= pParaObj->GetTextObject().GetScriptType();
            }
        }
    }

    if( nScriptType == SvtScriptType::NONE )
        nScriptType = SvtScriptType::LATIN;

    return nScriptType;
}

void SdrObjEditView::GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const
{
    if( mxSelectionController.is() )
        if( mxSelectionController->GetAttributes( rTargetSet, bOnlyHardAttr ) )
            return;

    if(IsTextEdit())
    {
        DBG_ASSERT(pTextEditOutlinerView!=nullptr,"SdrObjEditView::GetAttributes(): pTextEditOutlinerView=NULL");
        DBG_ASSERT(pTextEditOutliner!=nullptr,"SdrObjEditView::GetAttributes(): pTextEditOutliner=NULL");

        // take care of bOnlyHardAttr(!)
        if(!bOnlyHardAttr && mxTextEditObj->GetStyleSheet())
            rTargetSet.Put(mxTextEditObj->GetStyleSheet()->GetItemSet());

        // add object attributes
        rTargetSet.Put( mxTextEditObj->GetMergedItemSet() );

        if(pTextEditOutlinerView)
        {
            // FALSE= regard InvalidItems as "holes," not as Default
            rTargetSet.Put(pTextEditOutlinerView->GetAttribs(), false);
        }

        if(GetMarkedObjectCount()==1 && GetMarkedObjectByIndex(0)==mxTextEditObj.get())
        {
            MergeNotPersistAttrFromMarked(rTargetSet);
        }
    }
    else
    {
        SdrGlueEditView::GetAttributes(rTargetSet, bOnlyHardAttr);
    }
}

bool SdrObjEditView::SetAttributes(const SfxItemSet& rSet, bool bReplaceAll)
{
    bool bRet=false;
    bool bTextEdit=pTextEditOutlinerView!=nullptr && mxTextEditObj.is();
    bool bAllTextSelected=ImpIsTextEditAllSelected();
    const SfxItemSet* pSet=&rSet;

    if (!bTextEdit)
    {
        // no TextEdit active -> all Items to drawing object
        if( mxSelectionController.is() )
            bRet=mxSelectionController->SetAttributes(*pSet,bReplaceAll );

        if( !bRet )
        {
            SdrGlueEditView::SetAttributes(*pSet,bReplaceAll);
            bRet=true;
        }
    }
    else
    {
#ifdef DBG_UTIL
        {
            bool bHasEEFeatureItems=false;
            SfxItemIter aIter(rSet);
            const SfxPoolItem* pItem=aIter.FirstItem();
            while (!bHasEEFeatureItems && pItem!=nullptr)
            {
                if (!IsInvalidItem(pItem))
                {
                    sal_uInt16 nW=pItem->Which();
                    if (nW>=EE_FEATURE_START && nW<=EE_FEATURE_END)
                        bHasEEFeatureItems=true;
                }

                pItem=aIter.NextItem();
            }

            if(bHasEEFeatureItems)
            {
                const OUString aMessage("SdrObjEditView::SetAttributes(): Setting EE_FEATURE items at the SdrView does not make sense! It only leads to overhead and unreadable documents.");
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(nullptr,
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              aMessage));
                xInfoBox->run();
            }
        }
#endif

        bool bOnlyEEItems;
        bool bNoEEItems=!SearchOutlinerItems(*pSet,bReplaceAll,&bOnlyEEItems);
        // everything selected? -> attributes to the border, too
        // if no EEItems, attributes to the border only
        if (bAllTextSelected || bNoEEItems)
        {
            if( mxSelectionController.is() )
                bRet=mxSelectionController->SetAttributes(*pSet,bReplaceAll );

            if( !bRet )
            {
                const bool bUndo = IsUndoEnabled();

                if( bUndo )
                {
                    BegUndo(ImpGetDescriptionString(STR_EditSetAttributes));
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*mxTextEditObj.get()));

                    // If this is a text object also rescue the OutlinerParaObject since
                    // applying attributes to the object may change text layout when
                    // multiple portions exist with multiple formats. If a OutlinerParaObject
                    // really exists and needs to be rescued is evaluated in the undo
                    // implementation itself.
                    bool bRescueText = mxTextEditObj.get();

                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*mxTextEditObj.get(),false,!bNoEEItems || bRescueText));
                    EndUndo();
                }

                mxTextEditObj->SetMergedItemSetAndBroadcast(*pSet, bReplaceAll);

                FlushComeBackTimer(); // to set ModeHasChanged immediately
            }
        }
        else if (!bOnlyEEItems)
        {
            // Otherwise split Set, if necessary.
            // Now we build an ItemSet aSet that doesn't contain EE_Items from
            // *pSet (otherwise it would be a copy).
            std::unique_ptr<sal_uInt16[]> pNewWhichTable=RemoveWhichRange(pSet->GetRanges(),EE_ITEMS_START,EE_ITEMS_END);
            SfxItemSet aSet(mpModel->GetItemPool(),pNewWhichTable.get());
            pNewWhichTable.reset();
            SfxWhichIter aIter(aSet);
            sal_uInt16 nWhich=aIter.FirstWhich();
            while (nWhich!=0)
            {
                const SfxPoolItem* pItem;
                SfxItemState eState=pSet->GetItemState(nWhich,false,&pItem);
                if (eState==SfxItemState::SET) aSet.Put(*pItem);
                nWhich=aIter.NextWhich();
            }


            if( mxSelectionController.is() )
                bRet=mxSelectionController->SetAttributes(aSet,bReplaceAll );

            if( !bRet )
            {
                if( IsUndoEnabled() )
                {
                    BegUndo(ImpGetDescriptionString(STR_EditSetAttributes));
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*mxTextEditObj.get()));
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*mxTextEditObj.get()));
                    EndUndo();
                }

                mxTextEditObj->SetMergedItemSetAndBroadcast(aSet, bReplaceAll);

                if (GetMarkedObjectCount()==1 && GetMarkedObjectByIndex(0)==mxTextEditObj.get())
                {
                    SetNotPersistAttrToMarked(aSet);
                }
            }
            FlushComeBackTimer();
        }
        if(!bNoEEItems)
        {
            // and now the attributes to the EditEngine
            if (bReplaceAll) {
                pTextEditOutlinerView->RemoveAttribs( true );
            }
            pTextEditOutlinerView->SetAttribs(rSet);

#ifdef DBG_UTIL
            if (mpItemBrowser!=nullptr)
                mpItemBrowser->SetDirty();
#endif

            ImpMakeTextCursorAreaVisible();
        }
        bRet=true;
    }
    return bRet;
}

SfxStyleSheet* SdrObjEditView::GetStyleSheet() const
{
    SfxStyleSheet* pSheet = nullptr;

    if( mxSelectionController.is() )
    {
        if( mxSelectionController->GetStyleSheet( pSheet ) )
            return pSheet;
    }

    if ( pTextEditOutlinerView )
    {
        pSheet = pTextEditOutlinerView->GetStyleSheet();
    }
    else
    {
        pSheet = SdrGlueEditView::GetStyleSheet();
    }
    return pSheet;
}

void SdrObjEditView::SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    if( mxSelectionController.is() )
    {
        if( mxSelectionController->SetStyleSheet( pStyleSheet, bDontRemoveHardAttr ) )
            return;
    }

    // if we are currently in edit mode we must also set the stylesheet
    // on all paragraphs in the Outliner for the edit view
    if( nullptr != pTextEditOutlinerView )
    {
        Outliner* pOutliner = pTextEditOutlinerView->GetOutliner();

        const sal_Int32 nParaCount = pOutliner->GetParagraphCount();
        for( sal_Int32 nPara = 0; nPara < nParaCount; nPara++ )
        {
            pOutliner->SetStyleSheet( nPara, pStyleSheet );
        }
    }

    SdrGlueEditView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
}


void SdrObjEditView::AddWindowToPaintView(OutputDevice* pNewWin, vcl::Window *pWindow)
{
    SdrGlueEditView::AddWindowToPaintView(pNewWin, pWindow);

    if(mxTextEditObj.is() && !bTextEditOnlyOneView && pNewWin->GetOutDevType()==OUTDEV_WINDOW)
    {
        OutlinerView* pOutlView=ImpMakeOutlinerView(static_cast<vcl::Window*>(pNewWin),nullptr);
        pTextEditOutliner->InsertView(pOutlView);
    }
}

void SdrObjEditView::DeleteWindowFromPaintView(OutputDevice* pOldWin)
{
    SdrGlueEditView::DeleteWindowFromPaintView(pOldWin);

    if(mxTextEditObj.is() && !bTextEditOnlyOneView && pOldWin->GetOutDevType()==OUTDEV_WINDOW)
    {
        for (sal_uIntPtr i=pTextEditOutliner->GetViewCount(); i>0;) {
            i--;
            OutlinerView* pOLV=pTextEditOutliner->GetView(i);
            if (pOLV && pOLV->GetWindow()==static_cast<vcl::Window*>(pOldWin)) {
                pTextEditOutliner->RemoveView(i);
            }
        }
    }

    lcl_RemoveTextEditOutlinerViews(this, GetSdrPageView(), pOldWin);
}

bool SdrObjEditView::IsTextEditInSelectionMode() const
{
    return pTextEditOutliner!=nullptr && pTextEditOutliner->IsInSelectionMode();
}


// MacroMode


void SdrObjEditView::BegMacroObj(const Point& rPnt, short nTol, SdrObject* pObj, SdrPageView* pPV, vcl::Window* pWin)
{
    BrkMacroObj();
    if (pObj!=nullptr && pPV!=nullptr && pWin!=nullptr && pObj->HasMacro()) {
        nTol=ImpGetHitTolLogic(nTol,nullptr);
        pMacroObj=pObj;
        pMacroPV=pPV;
        pMacroWin=pWin;
        bMacroDown=false;
        nMacroTol=sal_uInt16(nTol);
        aMacroDownPos=rPnt;
        MovMacroObj(rPnt);
    }
}

void SdrObjEditView::ImpMacroUp(const Point& rUpPos)
{
    if (pMacroObj!=nullptr && bMacroDown)
    {
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=rUpPos;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        pMacroObj->PaintMacro(*pMacroWin,tools::Rectangle(),aHitRec);
        bMacroDown=false;
    }
}

void SdrObjEditView::ImpMacroDown(const Point& rDownPos)
{
    if (pMacroObj!=nullptr && !bMacroDown)
    {
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=rDownPos;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        pMacroObj->PaintMacro(*pMacroWin,tools::Rectangle(),aHitRec);
        bMacroDown=true;
    }
}

void SdrObjEditView::MovMacroObj(const Point& rPnt)
{
    if (pMacroObj!=nullptr) {
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=rPnt;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        bool bDown=pMacroObj->IsMacroHit(aHitRec);
        if (bDown) ImpMacroDown(rPnt);
        else ImpMacroUp(rPnt);
    }
}

void SdrObjEditView::BrkMacroObj()
{
    if (pMacroObj!=nullptr) {
        ImpMacroUp(aMacroDownPos);
        pMacroObj=nullptr;
        pMacroPV=nullptr;
        pMacroWin=nullptr;
    }
}

bool SdrObjEditView::EndMacroObj()
{
    if (pMacroObj!=nullptr && bMacroDown) {
        ImpMacroUp(aMacroDownPos);
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=aMacroDownPos;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        bool bRet=pMacroObj->DoMacro(aHitRec);
        pMacroObj=nullptr;
        pMacroPV=nullptr;
        pMacroWin=nullptr;
        return bRet;
    } else {
        BrkMacroObj();
        return false;
    }
}

/** fills the given any with a XTextCursor for the current text selection.
    Leaves the any untouched if there currently is no text selected */
void SdrObjEditView::getTextSelection( css::uno::Any& rSelection )
{
    if( IsTextEdit() )
    {
        OutlinerView* pOutlinerView = GetTextEditOutlinerView();
        if( pOutlinerView && pOutlinerView->HasSelection() )
        {
            SdrObject* pObj = GetTextEditObject();

            if( pObj )
            {
                css::uno::Reference< css::text::XText > xText( pObj->getUnoShape(), css::uno::UNO_QUERY );
                if( xText.is() )
                {
                    SvxUnoTextBase* pRange = SvxUnoTextBase::getImplementation( xText );
                    if( pRange )
                    {
                        rSelection <<= pRange->createTextCursorBySelection( pOutlinerView->GetSelection() );
                    }
                }
            }
        }
    }
}

/* check if we have a single selection and that single object likes
    to handle the mouse and keyboard events itself

    TODO: the selection controller should be queried from the
    object specific view contact. Currently this method only
    works for tables.
*/
void SdrObjEditView::MarkListHasChanged()
{
    SdrGlueEditView::MarkListHasChanged();

    if( mxSelectionController.is() )
    {
        mxLastSelectionController = mxSelectionController;
        mxSelectionController->onSelectionHasChanged();
    }

    mxSelectionController.clear();

    const SdrMarkList& rMarkList=GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        const SdrObject* pObj(rMarkList.GetMark(0)->GetMarkedSdrObj());
        SdrView* pView(dynamic_cast< SdrView* >(this));

        // check for table
        if(pObj && pView && (pObj->GetObjInventor() == SdrInventor::Default ) && (pObj->GetObjIdentifier() == OBJ_TABLE))
        {
            mxSelectionController = sdr::table::CreateTableController(
                *pView,
                static_cast<const sdr::table::SdrTableObj&>(*pObj),
                mxLastSelectionController);

            if( mxSelectionController.is() )
            {
                mxLastSelectionController.clear();
                mxSelectionController->onSelectionHasChanged();
            }
        }
    }
}

IMPL_LINK( SdrObjEditView, EndPasteOrDropHdl, PasteOrDropInfos*, pInfo, void )
{
    OnEndPasteOrDrop( pInfo );
}

IMPL_LINK( SdrObjEditView, BeginPasteOrDropHdl, PasteOrDropInfos*, pInfo, void )
{
    OnBeginPasteOrDrop( pInfo );
}

void SdrObjEditView::OnBeginPasteOrDrop( PasteOrDropInfos* )
{
    // applications can derive from these virtual methods to do something before a drop or paste operation
}

void SdrObjEditView::OnEndPasteOrDrop( PasteOrDropInfos* )
{
    // applications can derive from these virtual methods to do something before a drop or paste operation
}

sal_uInt16 SdrObjEditView::GetSelectionLevel() const
{
    sal_uInt16 nLevel = 0xFFFF;
    if( IsTextEdit() )
    {
        DBG_ASSERT(pTextEditOutlinerView!=nullptr,"SdrObjEditView::GetAttributes(): pTextEditOutlinerView=NULL");
        DBG_ASSERT(pTextEditOutliner!=nullptr,"SdrObjEditView::GetAttributes(): pTextEditOutliner=NULL");
        if( pTextEditOutlinerView )
        {
            //start and end position
            ESelection aSelect = pTextEditOutlinerView->GetSelection();
            sal_uInt16 nStartPara = ::std::min( aSelect.nStartPara, aSelect.nEndPara );
            sal_uInt16 nEndPara = ::std::max( aSelect.nStartPara, aSelect.nEndPara );
            //get level from each paragraph
            nLevel = 0;
            for( sal_uInt16 nPara = nStartPara; nPara <= nEndPara; nPara++ )
            {
                sal_uInt16 nParaDepth = 1 << static_cast<sal_uInt16>(pTextEditOutliner->GetDepth(nPara));
                if( !(nLevel & nParaDepth) )
                    nLevel += nParaDepth;
            }
            //reduce one level for Outliner Object
            //if( nLevel > 0 && GetTextEditObject()->GetObjIdentifier() == OBJ_OUTLINETEXT )
            //  nLevel = nLevel >> 1;
            //no bullet paragraph selected
            if( nLevel == 0)
                nLevel = 0xFFFF;
        }
    }
    return nLevel;
}

bool SdrObjEditView::SupportsFormatPaintbrush( SdrInventor nObjectInventor, sal_uInt16 nObjectIdentifier )
{
    if( nObjectInventor != SdrInventor::Default && nObjectInventor != SdrInventor::E3d )
        return false;
    switch(nObjectIdentifier)
    {
        case OBJ_NONE:
        case OBJ_GRUP:
            return false;
        case OBJ_LINE:
        case OBJ_RECT:
        case OBJ_CIRC:
        case OBJ_SECT:
        case OBJ_CARC:
        case OBJ_CCUT:
        case OBJ_POLY:
        case OBJ_PLIN:
        case OBJ_PATHLINE:
        case OBJ_PATHFILL:
        case OBJ_FREELINE:
        case OBJ_FREEFILL:
        case OBJ_SPLNLINE:
        case OBJ_SPLNFILL:
        case OBJ_TEXT:
        case OBJ_TITLETEXT:
        case OBJ_OUTLINETEXT:
        case OBJ_GRAF:
        case OBJ_OLE2:
        case OBJ_TABLE:
            return true;
        case OBJ_EDGE:
        case OBJ_CAPTION:
            return false;
        case OBJ_PATHPOLY:
        case OBJ_PATHPLIN:
            return true;
        case OBJ_PAGE:
        case OBJ_MEASURE:
        case OBJ_FRAME:
        case OBJ_UNO:
            return false;
        case OBJ_CUSTOMSHAPE:
            return true;
        default:
            return false;
    }
}

static const sal_uInt16* GetFormatRangeImpl( bool bTextOnly )
{
    static const sal_uInt16 gRanges[] = {
        SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,
        SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST,
        SDRATTR_TABLE_FIRST, SDRATTR_TABLE_LAST,
        XATTR_LINE_FIRST, XATTR_LINE_LAST,
        XATTR_FILL_FIRST, XATTRSET_FILL,
        EE_PARA_START, EE_PARA_END, // text-only from here on
        EE_CHAR_START, EE_CHAR_END,
        SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST, // table cell formats
        0,0
    };
    return &gRanges[ bTextOnly ? 10 : 0];
}

void SdrObjEditView::TakeFormatPaintBrush( std::shared_ptr< SfxItemSet >& rFormatSet  )
{
    if( mxSelectionController.is() && mxSelectionController->TakeFormatPaintBrush(rFormatSet) )
        return;

    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if( rMarkList.GetMarkCount() > 0 )
    {
        OutlinerView* pOLV = GetTextEditOutlinerView();

        rFormatSet.reset( new SfxItemSet( GetModel()->GetItemPool(), GetFormatRangeImpl( pOLV != nullptr ) ) );
        if( pOLV )
        {
            rFormatSet->Put( pOLV->GetAttribs() );
        }
        else
        {
            const bool bOnlyHardAttr = false;
            rFormatSet->Put( GetAttrFromMarked(bOnlyHardAttr) );
        }

        // check for cloning from table cell, in which case we need to copy cell-specific formatting attributes
        const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if( pObj && (pObj->GetObjInventor() == SdrInventor::Default ) && (pObj->GetObjIdentifier() == OBJ_TABLE) )
        {
            auto pTable = static_cast<const sdr::table::SdrTableObj*>(pObj);
            if (mxSelectionController.is() && pTable->getActiveCell().is()) {
                mxSelectionController->GetAttributes(*rFormatSet, false);
            }
        }
    }
}

static SfxItemSet CreatePaintSet( const sal_uInt16 *pRanges, SfxItemPool& rPool, const SfxItemSet& rSourceSet, const SfxItemSet& rTargetSet, bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    SfxItemSet aPaintSet( rPool, pRanges );

    while( *pRanges )
    {
        sal_uInt16 nWhich = *pRanges++;
        const sal_uInt16 nLastWhich = *pRanges++;

        if( bNoCharacterFormats && (nWhich == EE_CHAR_START) )
            continue;

        if( bNoParagraphFormats && (nWhich == EE_PARA_START ) )
            continue;

        for( ; nWhich < nLastWhich; nWhich++ )
        {
            const SfxPoolItem* pSourceItem = rSourceSet.GetItem( nWhich );
            const SfxPoolItem* pTargetItem = rTargetSet.GetItem( nWhich );

            if( (pSourceItem && !pTargetItem) || (pSourceItem && pTargetItem && *pSourceItem != *pTargetItem ) )
            {
                aPaintSet.Put( *pSourceItem );
            }
        }
    }
    return aPaintSet;
}

void SdrObjEditView::ApplyFormatPaintBrushToText( SfxItemSet const & rFormatSet, SdrTextObj& rTextObj, SdrText* pText, bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    OutlinerParaObject* pParaObj = pText ? pText->GetOutlinerParaObject() : nullptr;
    if(pParaObj)
    {
        SdrOutliner& rOutliner = rTextObj.ImpGetDrawOutliner();
        rOutliner.SetText(*pParaObj);

        sal_Int32 nParaCount(rOutliner.GetParagraphCount());

        if(nParaCount)
        {
            for(sal_Int32 nPara = 0; nPara < nParaCount; nPara++)
            {
                if( !bNoCharacterFormats )
                    rOutliner.RemoveCharAttribs( nPara );

                SfxItemSet aSet(rOutliner.GetParaAttribs(nPara));
                aSet.Put(CreatePaintSet( GetFormatRangeImpl(true), *aSet.GetPool(), rFormatSet, aSet, bNoCharacterFormats, bNoParagraphFormats ) );
                rOutliner.SetParaAttribs(nPara, aSet);
            }

            std::unique_ptr<OutlinerParaObject> pTemp = rOutliner.CreateParaObject(0, nParaCount);
            rOutliner.Clear();

            rTextObj.NbcSetOutlinerParaObjectForText(std::move(pTemp),pText);
        }
    }
}

void SdrObjEditView::ApplyFormatPaintBrush( SfxItemSet& rFormatSet, bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    if( mxSelectionController.is() &&
        mxSelectionController->ApplyFormatPaintBrush( rFormatSet, bNoCharacterFormats, bNoParagraphFormats ) )
    {
        return;
    }

    OutlinerView* pOLV = GetTextEditOutlinerView();
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if( !pOLV )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        const SfxItemSet& rShapeSet = pObj->GetMergedItemSet();

        // if not in text edit mode (aka the user selected text or clicked on a word)
        // apply formatting attributes to selected shape
        // All formatting items (see ranges above) that are unequal in selected shape and
        // the format paintbrush are hard set on the selected shape.

        const sal_uInt16* pRanges = rFormatSet.GetRanges();
        bool bTextOnly = true;

        while( *pRanges )
        {
            if( (*pRanges != EE_PARA_START) && (*pRanges != EE_CHAR_START) )
            {
                bTextOnly = false;
                break;
            }
            pRanges += 2;
        }

        if( !bTextOnly )
        {
            SfxItemSet aPaintSet( CreatePaintSet(
                                    GetFormatRangeImpl(false), *rShapeSet.GetPool(),
                                    rFormatSet, rShapeSet,
                                    bNoCharacterFormats, bNoParagraphFormats ) );
            SetAttrToMarked(aPaintSet, false/*bReplaceAll*/);
        }

        // now apply character and paragraph formatting to text, if the shape has any
        SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>(pObj);
        if( pTextObj )
        {
            sal_Int32 nText = pTextObj->getTextCount();

            while( --nText >= 0 )
            {
                SdrText* pText = pTextObj->getText( nText );
                ApplyFormatPaintBrushToText( rFormatSet, *pTextObj, pText, bNoCharacterFormats, bNoParagraphFormats );
            }
        }
    }
    else
    {
        ::Outliner* pOutliner = pOLV->GetOutliner();
        if( pOutliner )
        {
            const EditEngine& rEditEngine = pOutliner->GetEditEngine();

            ESelection aSel( pOLV->GetSelection() );
            if( !aSel.HasRange() )
                pOLV->SetSelection( rEditEngine.GetWord( aSel, css::i18n::WordType::DICTIONARY_WORD ) );

            const bool bRemoveParaAttribs = !bNoParagraphFormats;
            pOLV->RemoveAttribsKeepLanguages( bRemoveParaAttribs );
            SfxItemSet aSet( pOLV->GetAttribs() );
            SfxItemSet aPaintSet( CreatePaintSet(
                                    GetFormatRangeImpl(true), *aSet.GetPool(),
                                    rFormatSet, aSet,
                                    bNoCharacterFormats, bNoParagraphFormats ) );
            pOLV->SetAttribs( aPaintSet );
        }
    }


    // check for cloning to table cell, in which case we need to copy cell-specific formatting attributes
    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
    if( pObj && (pObj->GetObjInventor() == SdrInventor::Default) && (pObj->GetObjIdentifier() == OBJ_TABLE) )
    {
        auto pTable = static_cast<sdr::table::SdrTableObj*>(pObj);
        if (pTable->getActiveCell().is() && mxSelectionController.is()) {
            mxSelectionController->SetAttributes(rFormatSet, false);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
