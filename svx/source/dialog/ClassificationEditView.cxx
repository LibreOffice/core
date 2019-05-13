/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svx/ClassificationEditView.hxx>
#include <svx/ClassificationField.hxx>

#include <vcl/builder.hxx>
#include <vcl/cursor.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>
#include <svl/itemset.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdr/overlay/overlayselection.hxx>

namespace svx {

ClassificationEditEngine::ClassificationEditEngine(SfxItemPool* pItemPool)
    : EditEngine(pItemPool)
{}

OUString ClassificationEditEngine::CalcFieldValue(const SvxFieldItem& rField, sal_Int32 /*nPara*/,
                                                  sal_Int32 /*nPos*/, boost::optional<Color>& /*rTxtColor*/, boost::optional<Color>& /*rFldColor*/)
{
    OUString aString;
    const ClassificationField* pClassificationField = dynamic_cast<const ClassificationField*>(rField.GetField());
    if (pClassificationField)
        aString = pClassificationField->msDescription;
    else
        aString = "Unknown";
    return aString;
}

ClassificationEditView::ClassificationEditView()
{
}

void ClassificationEditView::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    Size aSize(500, 100);
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    SetOutputSizePixel(aSize);

    weld::CustomWidgetController::SetDrawingArea(pDrawingArea);

    EnableRTL(false);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    OutputDevice& rDevice = pDrawingArea->get_ref_device();

    rDevice.SetMapMode(MapMode(MapUnit::MapTwip));
    rDevice.SetBackground(aBgColor);

    Size aOutputSize(rDevice.PixelToLogic(aSize));
    aSize = aOutputSize;
    aSize.setHeight( aSize.Height() * 4 );

    pEdEngine.reset(new ClassificationEditEngine(EditEngine::CreatePool()));
    pEdEngine->SetPaperSize( aSize );
    pEdEngine->SetRefDevice( &rDevice );

    pEdEngine->SetControlWord(pEdEngine->GetControlWord() | EEControlBits::MARKFIELDS);

    pEdView.reset(new EditView(pEdEngine.get(), nullptr));
    pEdView->setEditViewCallbacks(this);
    pEdView->SetOutputArea(tools::Rectangle(Point(0,0), aOutputSize));

    pEdView->SetBackgroundColor(aBgColor);
    pEdEngine->InsertView(pEdView.get());

    pDrawingArea->set_cursor(PointerStyle::Text);
}

ClassificationEditView::~ClassificationEditView()
{
}

void ClassificationEditView::Resize()
{
    OutputDevice& rDevice = GetDrawingArea()->get_ref_device();
    Size aOutputSize(rDevice.PixelToLogic(GetOutputSizePixel()));
    Size aSize(aOutputSize);
    aSize.setHeight( aSize.Height() * 4 );
    pEdEngine->SetPaperSize(aSize);
    pEdView->SetOutputArea(tools::Rectangle(Point(0,0), aOutputSize));
    weld::CustomWidgetController::Resize();
}

void ClassificationEditView::InsertField(const SvxFieldItem& rFieldItem)
{
    pEdView->InsertField(rFieldItem);
    pEdView->Invalidate();
}

void ClassificationEditView::InvertSelectionWeight()
{
    ESelection aSelection = pEdView->GetSelection();

    for (sal_Int32 nParagraph = aSelection.nStartPara; nParagraph <= aSelection.nEndPara; ++nParagraph)
    {
        FontWeight eFontWeight = WEIGHT_BOLD;

        std::unique_ptr<SfxItemSet> pSet(new SfxItemSet(pEdEngine->GetParaAttribs(nParagraph)));
        if (const SfxPoolItem* pItem = pSet->GetItem(EE_CHAR_WEIGHT, false))
        {
            const SvxWeightItem* pWeightItem = dynamic_cast<const SvxWeightItem*>(pItem);
            if (pWeightItem && pWeightItem->GetWeight() == WEIGHT_BOLD)
                eFontWeight = WEIGHT_NORMAL;
        }
        SvxWeightItem aWeight(eFontWeight, EE_CHAR_WEIGHT);
        pSet->Put(aWeight);
        pEdEngine->SetParaAttribs(nParagraph, *pSet);
    }

    pEdView->Invalidate();
}

void ClassificationEditView::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    //note: ScEditWindow::Paint is similar

    rRenderContext.Push(PushFlags::ALL);
    rRenderContext.SetClipRegion();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    pEdView->SetBackgroundColor(aBgColor);

    rRenderContext.SetBackground(aBgColor);

    tools::Rectangle aLogicRect(rRenderContext.PixelToLogic(rRect));
    pEdView->Paint(aLogicRect, &rRenderContext);

    if (HasFocus())
    {
        pEdView->ShowCursor();
        vcl::Cursor* pCursor = pEdView->GetCursor();
        pCursor->DrawToDevice(rRenderContext);
    }

    std::vector<tools::Rectangle> aLogicRects;

    // get logic selection
    pEdView->GetSelectionRectangles(aLogicRects);

    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(COL_BLACK);
    rRenderContext.SetRasterOp(RasterOp::Invert);

    for (const auto &rSelectionRect : aLogicRects)
        rRenderContext.DrawRect(rSelectionRect);

    rRenderContext.Pop();
}

bool ClassificationEditView::MouseMove(const MouseEvent& rMEvt)
{
    return pEdView->MouseMove(rMEvt);
}

bool ClassificationEditView::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (!HasFocus())
        GrabFocus();

    return pEdView->MouseButtonDown(rMEvt);
}

bool ClassificationEditView::MouseButtonUp(const MouseEvent& rMEvt)
{
    return pEdView->MouseButtonUp(rMEvt);
}

bool ClassificationEditView::KeyInput(const KeyEvent& rKEvt)
{
    sal_uInt16 nKey =  rKEvt.GetKeyCode().GetModifier() + rKEvt.GetKeyCode().GetCode();

    if (nKey == KEY_TAB || nKey == KEY_TAB + KEY_SHIFT)
    {
        return false;
    }
    else if (!pEdView->PostKeyEvent(rKEvt))
    {
        return false;
    }

    return true;
}

void ClassificationEditView::GetFocus()
{
    pEdView->ShowCursor();

    weld::CustomWidgetController::GetFocus();
}

} // end sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
