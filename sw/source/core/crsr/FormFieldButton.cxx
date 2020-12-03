/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DropDownFormFieldButton.hxx>
#include <edtwin.hxx>
#include <basegfx/color/bcolortools.hxx>
#include <bookmrk.hxx>
#include <vcl/weldutils.hxx>
#include <vcl/event.hxx>

FormFieldButton::FormFieldButton(SwEditWin* pEditWin, sw::mark::Fieldmark& rFieldmark)
    : Control(pEditWin, WB_DIALOGCONTROL)
    , m_rFieldmark(rFieldmark)
{
    assert(GetParent());
    assert(dynamic_cast<SwEditWin*>(GetParent()));

    SetBackground();
    EnableChildTransparentMode();
    SetParentClipMode(ParentClipMode::NoClip);
    SetPaintTransparent(true);
}

FormFieldButton::~FormFieldButton() { disposeOnce(); }

void FormFieldButton::LaunchPopup()
{
    m_xFieldPopup->connect_closed(LINK(this, DropDownFormFieldButton, FieldPopupModeEndHdl));

    tools::Rectangle aRect(Point(0, 0), GetSizePixel());
    weld::Window* pParent = weld::GetPopupParent(*this, aRect);
    m_xFieldPopup->popup_at_rect(pParent, aRect);
}

void FormFieldButton::DestroyPopup()
{
    m_xFieldPopup.reset();
    m_xFieldPopupBuilder.reset();
}

void FormFieldButton::dispose()
{
    DestroyPopup();
    Control::dispose();
}

void FormFieldButton::CalcPosAndSize(const SwRect& rPortionPaintArea)
{
    assert(GetParent());

    Point aBoxPos = GetParent()->LogicToPixel(rPortionPaintArea.Pos());
    Size aBoxSize = GetParent()->LogicToPixel(rPortionPaintArea.SSize());

    // First calculate the size of the frame around the field
    int nPadding = aBoxSize.Height() / 4;
    aBoxPos.AdjustX(-nPadding);
    aBoxPos.AdjustY(-nPadding);
    aBoxSize.AdjustWidth(2 * nPadding);
    aBoxSize.AdjustHeight(2 * nPadding);

    m_aFieldFramePixel = tools::Rectangle(aBoxPos, aBoxSize);

    // Then extend the size with the button area
    aBoxSize.AdjustWidth(GetParent()->LogicToPixel(rPortionPaintArea.SSize()).Height());

    if (aBoxPos != GetPosPixel() || aBoxSize != GetSizePixel())
    {
        SetPosSizePixel(aBoxPos, aBoxSize);
        Invalidate();
    }
}

void FormFieldButton::MouseButtonDown(const MouseEvent&)
{
    LaunchPopup();
    Invalidate();
}

IMPL_LINK_NOARG(FormFieldButton, FieldPopupModeEndHdl, weld::Popover&, void)
{
    DestroyPopup();
    m_rFieldmark.Invalidate();
    // Hide the button here and make it visible later, to make transparent background work with SAL_USE_VCLPLUGIN=gen
    Show(false);
    Invalidate();
}

static basegfx::BColor lcl_GetFillColor(const basegfx::BColor& rLineColor, double aLuminance)
{
    basegfx::BColor aHslLine = basegfx::utils::rgb2hsl(rLineColor);
    aHslLine.setZ(aLuminance);
    return basegfx::utils::hsl2rgb(aHslLine);
}

void FormFieldButton::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    SetMapMode(MapMode(MapUnit::MapPixel));

    //const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    Color aLineColor = COL_BLACK;
    Color aFillColor(lcl_GetFillColor(aLineColor.getBColor(), (m_xFieldPopup ? 0.5 : 0.75)));

    // Draw the frame around the field
    // GTK3 backend cuts down the frame's top and left border, to avoid that add a padding around the frame
    int nPadding = 1;
    Point aPos(nPadding, nPadding);
    Size aSize(m_aFieldFramePixel.GetSize().Width() - nPadding,
               m_aFieldFramePixel.GetSize().Height() - nPadding);
    const tools::Rectangle aFrameRect(tools::Rectangle(aPos, aSize));
    rRenderContext.SetLineColor(aLineColor);
    rRenderContext.SetFillColor(COL_TRANSPARENT);
    rRenderContext.DrawRect(aFrameRect);

    // Draw the button next to the frame
    Point aButtonPos(aFrameRect.TopLeft());
    aButtonPos.AdjustX(aFrameRect.GetSize().getWidth() - 1);
    Size aButtonSize(aFrameRect.GetSize());
    aButtonSize.setWidth(GetSizePixel().getWidth() - aFrameRect.getWidth() - nPadding);
    const tools::Rectangle aButtonRect(tools::Rectangle(aButtonPos, aButtonSize));

    // Background & border
    rRenderContext.SetLineColor(aLineColor);
    rRenderContext.SetFillColor(aFillColor);
    rRenderContext.DrawRect(aButtonRect);

    // the arrowhead
    rRenderContext.SetLineColor(aLineColor);
    rRenderContext.SetFillColor(aLineColor);

    Point aCenter(aButtonPos.X() + (aButtonSize.Width() / 2),
                  aButtonPos.Y() + (aButtonSize.Height() / 2));
    Size aArrowSize(aButtonSize.Width() / 4, aButtonSize.Height() / 10);

    tools::Polygon aPoly(3);
    aPoly.SetPoint(Point(aCenter.X() - aArrowSize.Width(), aCenter.Y() - aArrowSize.Height()), 0);
    aPoly.SetPoint(Point(aCenter.X() + aArrowSize.Width(), aCenter.Y() - aArrowSize.Height()), 1);
    aPoly.SetPoint(Point(aCenter.X(), aCenter.Y() + aArrowSize.Height()), 2);
    rRenderContext.DrawPolygon(aPoly);
}

WindowHitTest FormFieldButton::ImplHitTest(const Point& rFramePos)
{
    // We need to check whether the position hits the button (the frame should be mouse transparent)
    WindowHitTest aResult = Control::ImplHitTest(rFramePos);
    if (aResult != WindowHitTest::Inside)
        return aResult;
    else
    {
        return rFramePos.X() >= m_aFieldFramePixel.Right() ? WindowHitTest::Inside
                                                           : WindowHitTest::Transparent;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
