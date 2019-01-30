/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <UnfloatTableButton.hxx>
#include <HeaderFooterWin.hxx>

#include <edtwin.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <strings.hrc>
#include <fmtpdsc.hxx>
#include <vcl/metric.hxx>
#include <vcl/settings.hxx>
#include <viewopt.hxx>
#include <frame.hxx>
#include <flyfrm.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <ndindex.hxx>
#include <swtable.hxx>
#include <IDocumentState.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <DocumentContentOperationsManager.hxx>

#define TEXT_PADDING 3
#define BOX_DISTANCE 3
#define BUTTON_WIDTH 12

UnfloatTableButton::UnfloatTableButton(SwEditWin* pEditWin, const SwFrame* pFrame)
    : SwFrameMenuButtonBase(pEditWin, pFrame)
    , m_sLabel(SwResId(STR_UNFLOAT_TABLE))
{
}

UnfloatTableButton::~UnfloatTableButton() { disposeOnce(); }

void UnfloatTableButton::SetOffset(Point aBottomRightPixel)
{
    // Compute the text size and get the box position & size from it
    tools::Rectangle aTextRect;
    GetTextBoundRect(aTextRect, m_sLabel);
    tools::Rectangle aTextPxRect = LogicToPixel(aTextRect);
    FontMetric aFontMetric = GetFontMetric(GetFont());
    Size aBoxSize(aTextPxRect.GetWidth() + BUTTON_WIDTH + TEXT_PADDING * 2,
                  aFontMetric.GetLineHeight() + TEXT_PADDING * 2);

    Point aBoxPos(aBottomRightPixel.X() - aBoxSize.Width() - BOX_DISTANCE,
                  aBottomRightPixel.Y() - aBoxSize.Height());

    if (AllSettings::GetLayoutRTL())
    {
        aBoxPos.setX(aBottomRightPixel.X() + BOX_DISTANCE);
    }

    // Set the position & Size of the window
    SetPosSizePixel(aBoxPos, aBoxSize);
}

void UnfloatTableButton::MouseButtonDown(const MouseEvent& /*rMEvt*/)
{
    assert(GetFrame()->IsFlyFrame());
    // const_cast is needed because of bad design of ISwFrameControl and derived classes
    SwFlyFrame* pFlyFrame = const_cast<SwFlyFrame*>(static_cast<const SwFlyFrame*>(GetFrame()));

    // Find the table inside the text frame
    SwTabFrame* pTableFrame = nullptr;
    SwFrame* pLower = pFlyFrame->GetLower();
    while (pLower)
    {
        if (pLower->IsTabFrame())
        {
            pTableFrame = static_cast<SwTabFrame*>(pLower);
            break;
        }
        pLower = pLower->GetNext();
    }

    if (pTableFrame == nullptr)
        return;

    // Insert the table at the position of the text node which has the frame anchored to
    SwFrame* pAnchoreFrame = pFlyFrame->AnchorFrame();
    if (pAnchoreFrame == nullptr || !pAnchoreFrame->IsTextFrame())
        return;

    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pAnchoreFrame);
    if (pTextFrame->GetTextNodeFirst() == nullptr)
        return;

    SwNodeIndex aInsertPos((*pTextFrame->GetTextNodeFirst()));

    SwTableNode* pTableNode = pTableFrame->GetTable()->GetTableNode();
    if (pTableNode == nullptr)
        return;

    SwDoc& rDoc = pTextFrame->GetDoc();

    // When we move the table before the first text node, we need to clear RES_PAGEDESC attribute
    // of the text node otherwise LO will create a page break after the table
    if (pTextFrame->GetTextNodeFirst())
    {
        const SwPageDesc* pPageDesc
            = pTextFrame->GetPageDescItem().GetPageDesc(); // First text node of the page has this
        if (pPageDesc)
        {
            // First set the existing page desc for the table node
            SfxItemSet aSet(GetEditWin()->GetView().GetWrtShell().GetAttrPool(),
                            svl::Items<RES_PAGEDESC, RES_PAGEDESC>{});
            aSet.Put(SwFormatPageDesc(pPageDesc));
            SwPaM aPaMTable(*pTableNode);
            rDoc.getIDocumentContentOperations().InsertItemSet(
                aPaMTable, aSet, SetAttrMode::DEFAULT, GetPageFrame()->getRootFrame());

            // Then remove pagedesc from the attributes of the text node
            aSet.Put(SwFormatPageDesc(nullptr));
            SwPaM aPaMTextNode(*pTextFrame->GetTextNodeFirst());
            rDoc.getIDocumentContentOperations().InsertItemSet(
                aPaMTextNode, aSet, SetAttrMode::DEFAULT, GetPageFrame()->getRootFrame());
        }
    }

    // Move the table outside of the text frame
    SwNodeRange aRange(*pTableNode, 0, *pTableNode->EndOfSectionNode(), 1);
    rDoc.getIDocumentContentOperations().MoveNodeRange(aRange, aInsertPos, SwMoveFlags::DEFAULT);

    // Remove the floating table's frame
    SwFlyFrameFormat* pFrameFormat = pFlyFrame->GetFormat();
    if (pFrameFormat)
    {
        rDoc.getIDocumentLayoutAccess().DelLayoutFormat(pFrameFormat);
    }

    rDoc.getIDocumentState().SetModified();

    // Undoing MoveNodeRange() is not working correctly in case of tables, it crashes sometimes
    // So don't allow to undo after unfloating (similar to MakeFlyAndMove() method)
    if (rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        rDoc.GetIDocumentUndoRedo().DelAllUndoObj();
    }
}

void UnfloatTableButton::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    SetMapMode(MapMode(MapUnit::MapPixel));
    drawinglayer::primitive2d::Primitive2DContainer aSeq;
    const ::tools::Rectangle aRect(
        ::tools::Rectangle(Point(0, 0), rRenderContext.PixelToLogic(GetSizePixel())));

    // Create button
    SwFrameButtonPainter::PaintButton(aSeq, aRect, false);

    // Create the text primitive
    basegfx::BColor aLineColor = SwViewOption::GetHeaderFooterMarkColor().getBColor();
    basegfx::B2DVector aFontSize;
    drawinglayer::attribute::FontAttribute aFontAttr
        = drawinglayer::primitive2d::getFontAttributeFromVclFont(
            aFontSize, rRenderContext.GetFont(), false, false);

    FontMetric aFontMetric = rRenderContext.GetFontMetric(rRenderContext.GetFont());
    double nTextOffsetY = aFontMetric.GetAscent() + TEXT_PADDING;
    double nTextOffsetX = std::abs(aRect.GetWidth() - rRenderContext.GetTextWidth(m_sLabel)) / 2.0;
    Point aTextPos(nTextOffsetX, nTextOffsetY);

    basegfx::B2DHomMatrix aTextMatrix(basegfx::utils::createScaleTranslateB2DHomMatrix(
        aFontSize.getX(), aFontSize.getY(), static_cast<double>(aTextPos.X()),
        static_cast<double>(aTextPos.Y())));

    aSeq.push_back(drawinglayer::primitive2d::Primitive2DReference(
        new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
            aTextMatrix, m_sLabel, 0, m_sLabel.getLength(), std::vector<double>(), aFontAttr,
            css::lang::Locale(), aLineColor)));

    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(rRenderContext,
                                                                         aNewViewInfos));

    pProcessor->process(aSeq);
}

void UnfloatTableButton::ShowAll(bool bShow) { Show(bShow); }

bool UnfloatTableButton::Contains(const Point& rDocPt) const
{
    ::tools::Rectangle aRect(GetPosPixel(), GetSizePixel());
    if (aRect.IsInside(rDocPt))
        return true;

    return false;
}

void UnfloatTableButton::SetReadonly(bool bReadonly) { ShowAll(!bReadonly); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
