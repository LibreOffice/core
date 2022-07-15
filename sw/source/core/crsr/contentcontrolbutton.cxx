/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <contentcontrolbutton.hxx>

#include <vcl/weldutils.hxx>
#include <vcl/event.hxx>
#include <vcl/decoview.hxx>

#include <edtwin.hxx>
#include <dview.hxx>

SwContentControlButton::SwContentControlButton(
    SwEditWin* pEditWin, const std::shared_ptr<SwContentControl>& pContentControl)
    : Control(pEditWin, WB_DIALOGCONTROL)
    , m_pContentControl(pContentControl)
{
    assert(GetParent());
    assert(dynamic_cast<SwEditWin*>(GetParent()));

    SetBackground();
    EnableChildTransparentMode();
    SetParentClipMode(ParentClipMode::NoClip);
    SetPaintTransparent(true);
}

SwContentControlButton::~SwContentControlButton() { disposeOnce(); }

void SwContentControlButton::LaunchPopup()
{
    m_xPopup->connect_closed(LINK(this, SwContentControlButton, PopupModeEndHdl));

    tools::Rectangle aRect(Point(0, 0), GetSizePixel());
    weld::Window* pParent = weld::GetPopupParent(*this, aRect);
    m_xPopup->popup_at_rect(pParent, aRect);
}

void SwContentControlButton::DestroyPopup()
{
    m_xPopup.reset();
    m_xPopupBuilder.reset();
}

void SwContentControlButton::dispose()
{
    DestroyPopup();
    Control::dispose();
}

void SwContentControlButton::CalcPosAndSize(const SwRect& rPortionPaintArea)
{
    assert(GetParent());

    Point aBoxPos = GetParent()->LogicToPixel(rPortionPaintArea.Pos());
    Size aBoxSize = GetParent()->LogicToPixel(rPortionPaintArea.SSize());

    // First calculate the size of the frame around the content control's last portion
    int nPadding = aBoxSize.Height() / 4;
    aBoxPos.AdjustX(-nPadding / 2);
    aBoxPos.AdjustY(-1);
    aBoxSize.AdjustWidth(nPadding);
    aBoxSize.AdjustHeight(2);

    m_aFramePixel = tools::Rectangle(aBoxPos, aBoxSize);

    // Then extend the size with the button area
    aBoxSize.AdjustWidth(GetParent()->LogicToPixel(rPortionPaintArea.SSize()).Height());

    if (aBoxPos != GetPosPixel() || aBoxSize != GetSizePixel())
    {
        SetPosSizePixel(aBoxPos, aBoxSize);
        Invalidate();
    }
}

void SwContentControlButton::MouseButtonDown(const MouseEvent&) { StartPopup(); }

void SwContentControlButton::StartPopup()
{
    LaunchPopup();
    Invalidate();
}

IMPL_LINK_NOARG(SwContentControlButton, PopupModeEndHdl, weld::Popover&, void)
{
    DestroyPopup();
    Show(false);
    Invalidate();
}

void SwContentControlButton::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    SetMapMode(MapMode(MapUnit::MapPixel));

    Color aLineColor = COL_BLACK;
    Color aFillColor = aLineColor;
    aFillColor.IncreaseLuminance(255 * (m_xPopup ? 0.5 : 0.75));

    // Calc the frame around the content control's last portion
    int nPadding = 1;
    Point aPos(nPadding, nPadding);
    Size aSize(m_aFramePixel.GetSize().Width() - nPadding,
               m_aFramePixel.GetSize().Height() - nPadding);
    const tools::Rectangle aFrameRect(tools::Rectangle(aPos, aSize));

    // Draw the button next to the frame
    Point aButtonPos(aFrameRect.TopLeft());
    aButtonPos.AdjustX(aFrameRect.GetSize().getWidth() - nPadding * 2);
    Size aButtonSize(aFrameRect.GetSize());
    aButtonSize.setWidth(GetSizePixel().getWidth() - aFrameRect.getWidth() - nPadding);
    const tools::Rectangle aButtonRect(tools::Rectangle(aButtonPos, aButtonSize));

    // Background & border
    rRenderContext.SetLineColor(aLineColor);
    rRenderContext.SetFillColor(aFillColor);
    rRenderContext.DrawRect(aButtonRect);

    // the arrowhead
    DecorationView aDecoView(&rRenderContext);
    tools::Rectangle aSymbolRect(aButtonRect);
    // 20% distance to the left and right button border
    const tools::Long nBorderDistanceLeftAndRight = aSymbolRect.GetWidth() / 4;
    aSymbolRect.AdjustLeft(nBorderDistanceLeftAndRight);
    aSymbolRect.AdjustRight(-nBorderDistanceLeftAndRight);
    // 20% distance to the top and bottom button border
    const tools::Long nBorderDistanceTopAndBottom = aSymbolRect.GetHeight() / 4;
    aSymbolRect.AdjustTop(nBorderDistanceTopAndBottom);
    aSymbolRect.AdjustBottom(-nBorderDistanceTopAndBottom);
    AntialiasingFlags eAntialiasing = rRenderContext.GetAntialiasing();
    if (SwDrawView::IsAntiAliasing())
    {
        rRenderContext.SetAntialiasing(eAntialiasing | AntialiasingFlags::Enable);
    }
    aDecoView.DrawSymbol(aSymbolRect, SymbolType::SPIN_DOWN, GetTextColor(), DrawSymbolFlags::NONE);
    if (SwDrawView::IsAntiAliasing())
    {
        rRenderContext.SetAntialiasing(eAntialiasing);
    }
}

WindowHitTest SwContentControlButton::ImplHitTest(const Point& rFramePos)
{
    // We need to check whether the position hits the button (the frame should be mouse transparent)
    WindowHitTest aResult = Control::ImplHitTest(rFramePos);
    if (aResult != WindowHitTest::Inside)
        return aResult;
    else
    {
        return rFramePos.X() >= m_aFramePixel.Right() ? WindowHitTest::Inside
                                                      : WindowHitTest::Transparent;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
