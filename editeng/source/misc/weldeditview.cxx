/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <editeng/weldeditview.hxx>
#include <vcl/cursor.hxx>
#include <vcl/event.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

WeldEditView::WeldEditView() {}

void WeldEditView::makeEditEngine()
{
    m_xEditEngine.reset(new EditEngine(EditEngine::CreatePool()));
}

void WeldEditView::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    Size aSize(pDrawingArea->get_size_request());
    if (aSize.Width() == -1)
        aSize.setWidth(500);
    if (aSize.Height() == -1)
        aSize.setHeight(100);
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
    aSize.setHeight(aSize.Height());

    makeEditEngine();
    m_xEditEngine->SetPaperSize(aSize);
    m_xEditEngine->SetRefDevice(&rDevice);

    m_xEditEngine->SetControlWord(m_xEditEngine->GetControlWord() | EEControlBits::MARKFIELDS);

    m_xEditView.reset(new EditView(m_xEditEngine.get(), nullptr));
    m_xEditView->setEditViewCallbacks(this);
    m_xEditView->SetOutputArea(tools::Rectangle(Point(0, 0), aOutputSize));

    m_xEditView->SetBackgroundColor(aBgColor);
    m_xEditEngine->InsertView(m_xEditView.get());

    pDrawingArea->set_cursor(PointerStyle::Text);
}

WeldEditView::~WeldEditView() {}

void WeldEditView::Resize()
{
    OutputDevice& rDevice = GetDrawingArea()->get_ref_device();
    Size aOutputSize(rDevice.PixelToLogic(GetOutputSizePixel()));
    Size aSize(aOutputSize);
    m_xEditEngine->SetPaperSize(aSize);
    m_xEditView->SetOutputArea(tools::Rectangle(Point(0, 0), aOutputSize));
    weld::CustomWidgetController::Resize();
}

void WeldEditView::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    //note: ScEditWindow::Paint is similar

    rRenderContext.Push(PushFlags::ALL);
    rRenderContext.SetClipRegion();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    m_xEditView->SetBackgroundColor(aBgColor);

    rRenderContext.SetBackground(aBgColor);

    tools::Rectangle aLogicRect(rRenderContext.PixelToLogic(rRect));
    m_xEditView->Paint(aLogicRect, &rRenderContext);

    if (HasFocus())
    {
        m_xEditView->ShowCursor();
        vcl::Cursor* pCursor = m_xEditView->GetCursor();
        pCursor->DrawToDevice(rRenderContext);
    }

    std::vector<tools::Rectangle> aLogicRects;

    // get logic selection
    m_xEditView->GetSelectionRectangles(aLogicRects);

    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(COL_BLACK);
    rRenderContext.SetRasterOp(RasterOp::Invert);

    for (const auto& rSelectionRect : aLogicRects)
        rRenderContext.DrawRect(rSelectionRect);

    rRenderContext.Pop();
}

bool WeldEditView::MouseMove(const MouseEvent& rMEvt) { return m_xEditView->MouseMove(rMEvt); }

bool WeldEditView::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (!HasFocus())
        GrabFocus();

    return m_xEditView->MouseButtonDown(rMEvt);
}

bool WeldEditView::MouseButtonUp(const MouseEvent& rMEvt)
{
    return m_xEditView->MouseButtonUp(rMEvt);
}

bool WeldEditView::KeyInput(const KeyEvent& rKEvt)
{
    sal_uInt16 nKey = rKEvt.GetKeyCode().GetCode();

    if (nKey == KEY_TAB)
    {
        return false;
    }
    else if (!m_xEditView->PostKeyEvent(rKEvt))
    {
        return false;
    }

    return true;
}

void WeldEditView::GetFocus()
{
    m_xEditView->ShowCursor();

    weld::CustomWidgetController::GetFocus();
}

void WeldEditView::LoseFocus()
{
    weld::CustomWidgetController::LoseFocus();
    Invalidate(); // redraw without cursor
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
