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
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <unoprnms.hxx>
#include <unotbl.hxx>
#include <IDocumentState.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <svl/grabbagitem.hxx>
#include <doc.hxx>

#define TEXT_PADDING 3
#define BOX_DISTANCE 3
#define BUTTON_WIDTH 12

UnfloatTableButton::UnfloatTableButton(SwEditWin* pEditWin, const SwFrame* pFrame)
    : SwFrameMenuButtonBase(pEditWin, pFrame, u"modules/swriter/ui/unfloatbutton.ui"_ustr,
                            u"UnfloatButton"_ustr)
    , m_xPushButton(m_xBuilder->weld_button(u"button"_ustr))
    , m_sLabel(SwResId(STR_UNFLOAT_TABLE))
{
    m_xPushButton->set_accessible_name(m_sLabel);
    m_xVirDev = m_xPushButton->create_virtual_device();
    m_xPushButton->connect_clicked(LINK(this, UnfloatTableButton, ClickHdl));
    SetVirDevFont();
}

weld::Button* UnfloatTableButton::GetButton() { return m_xPushButton.get(); }

UnfloatTableButton::~UnfloatTableButton() { disposeOnce(); }

void UnfloatTableButton::dispose()
{
    m_xPushButton.reset();
    m_xVirDev.disposeAndClear();
    SwFrameMenuButtonBase::dispose();
}

void UnfloatTableButton::SetOffset(Point aTopRightPixel)
{
    // Compute the text size and get the box position & size from it
    tools::Rectangle aTextRect;
    m_xVirDev->GetTextBoundRect(aTextRect, m_sLabel);
    tools::Rectangle aTextPxRect = m_xVirDev->LogicToPixel(aTextRect);
    FontMetric aFontMetric = m_xVirDev->GetFontMetric(m_xVirDev->GetFont());
    Size aBoxSize(aTextPxRect.GetWidth() + BUTTON_WIDTH + TEXT_PADDING * 2,
                  aFontMetric.GetLineHeight() + TEXT_PADDING * 2);

    Point aBoxPos(aTopRightPixel.X() - aBoxSize.Width() - BOX_DISTANCE, aTopRightPixel.Y());

    if (AllSettings::GetLayoutRTL())
    {
        aBoxPos.setX(aTopRightPixel.X() + BOX_DISTANCE);
    }

    // Set the position & Size of the window
    SetPosSizePixel(aBoxPos, aBoxSize);
    m_xVirDev->SetOutputSizePixel(aBoxSize);

    PaintButton();
}

IMPL_LINK_NOARG(UnfloatTableButton, ClickHdl, weld::Button&, void)
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

    SwTableNode* pTableNode = pTableFrame->GetTable()->GetTableNode();
    if (pTableNode == nullptr)
        return;

    SwDoc& rDoc = pTextFrame->GetDoc();

    // tdf#129176: clear "TablePosition" grab bag, since we explicitly change the position here
    // See DomainMapperTableHandler::endTableGetTableStyle, where the grab bag is filled, and
    // DocxAttributeOutput::TableDefinition that uses it on export
    SwFrameFormat* pTableFormat = pTableFrame->GetTable()->GetFrameFormat();
    assert(pTableFormat);
    if (const SfxGrabBagItem* pGrabBagItem = pTableFormat->GetAttrSet().GetItem(RES_FRMATR_GRABBAG))
    {
        SfxGrabBagItem aGrabBagItem(*pGrabBagItem); // Editable copy
        if (aGrabBagItem.GetGrabBag().erase(u"TablePosition"_ustr))
        {
            css::uno::Any aVal;
            aGrabBagItem.QueryValue(aVal);
            const rtl::Reference<SwXTextTable> xTable
                = SwXTextTable::CreateXTextTable(pTableFormat);
            xTable->setPropertyValue(UNO_NAME_TABLE_INTEROP_GRAB_BAG, aVal);
        }
    }

    // When we move the table before the first text node, we need to clear RES_PAGEDESC attribute
    // of the text node otherwise LO will create a page break after the table
    if (pTextFrame->GetTextNodeFirst())
    {
        const SwPageDesc* pPageDesc
            = pTextFrame->GetPageDescItem().GetPageDesc(); // First text node of the page has this
        if (pPageDesc)
        {
            // First set the existing page desc for the table node
            SfxItemSetFixed<RES_PAGEDESC, RES_PAGEDESC> aSet(
                GetEditWin()->GetView().GetWrtShell().GetAttrPool());
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

    SwWrtShell& rWrtShell = GetEditWin()->GetView().GetWrtShell();
    rWrtShell.UnfloatFlyFrame();

    rDoc.getIDocumentState().SetModified();

    // Undoing MoveNodeRange() is not working correctly in case of tables, it crashes sometimes
    // So don't allow to undo after unfloating (similar to MakeFlyAndMove() method)
    if (rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        rDoc.GetIDocumentUndoRedo().DelAllUndoObj();
    }
}

void UnfloatTableButton::PaintButton()
{
    if (!m_xVirDev)
        return;

    m_xVirDev->SetMapMode(MapMode(MapUnit::MapPixel));
    drawinglayer::primitive2d::Primitive2DContainer aSeq;
    const ::tools::Rectangle aRect(
        ::tools::Rectangle(Point(0, 0), m_xVirDev->PixelToLogic(GetSizePixel())));

    // Create button
    SwFrameButtonPainter::PaintButton(aSeq, aRect, true);

    // Create the text primitive
    basegfx::BColor aLineColor
        = SwViewOption::GetCurrentViewOptions().GetHeaderFooterMarkColor().getBColor();
    basegfx::B2DVector aFontSize;
    drawinglayer::attribute::FontAttribute aFontAttr
        = drawinglayer::primitive2d::getFontAttributeFromVclFont(aFontSize, m_xVirDev->GetFont(),
                                                                 false, false);

    FontMetric aFontMetric = m_xVirDev->GetFontMetric(m_xVirDev->GetFont());
    double nTextOffsetY = aFontMetric.GetAscent() + TEXT_PADDING;
    double nTextOffsetX = std::abs(aRect.GetWidth() - m_xVirDev->GetTextWidth(m_sLabel)) / 2.0;
    Point aTextPos(nTextOffsetX, nTextOffsetY);

    basegfx::B2DHomMatrix aTextMatrix(basegfx::utils::createScaleTranslateB2DHomMatrix(
        aFontSize.getX(), aFontSize.getY(), static_cast<double>(aTextPos.X()),
        static_cast<double>(aTextPos.Y())));

    aSeq.push_back(new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
        aTextMatrix, m_sLabel, 0, m_sLabel.getLength(), std::vector<double>(), {},
        std::move(aFontAttr), css::lang::Locale(), aLineColor));

    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createProcessor2DFromOutputDevice(*m_xVirDev, aNewViewInfos));

    pProcessor->process(aSeq);

    m_xPushButton->set_custom_button(m_xVirDev.get());
}

void UnfloatTableButton::ShowAll(bool bShow) { Show(bShow); }

bool UnfloatTableButton::Contains(const Point& rDocPt) const
{
    ::tools::Rectangle aRect(GetPosPixel(), GetSizePixel());
    if (aRect.Contains(rDocPt))
        return true;

    return false;
}

void UnfloatTableButton::SetReadonly(bool bReadonly) { ShowAll(!bReadonly); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
