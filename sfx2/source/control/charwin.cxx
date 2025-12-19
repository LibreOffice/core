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

#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld/weldutils.hxx>
#include <sfx2/charwin.hxx>
#include <sfx2/charmapcontainer.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

using namespace com::sun::star;

SvxCharView::SvxCharView(const VclPtr<VirtualDevice>& rVirDev)
    : mxVirDev(rVirDev)
    , maHasInsert(true)
{
}

SvxCharView::~SvxCharView() {}

void SvxCharView::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    vcl::Font aFont = rStyleSettings.GetLabelFont();
    const Size aFontSize = aFont.GetFontSize();
    aFont.SetFontSize(Size(aFontSize.Width() * 2, aFontSize.Height() * 2));
    auto popIt = mxVirDev->ScopedPush(PUSH_ALLFONT);
    weld::SetPointFont(*mxVirDev, aFont);
    pDrawingArea->set_size_request(mxVirDev->approximate_digit_width() * 2,
                                   mxVirDev->GetTextHeight());
}

void SvxCharView::GetFocus()
{
    Invalidate();
    if (maFocusInHdl.IsSet())
        maFocusInHdl.Call(CharAndFont(GetText(), GetFontFamilyName()));
}

void SvxCharView::LoseFocus() { Invalidate(); }

OUString SvxCharView::RequestHelp(tools::Rectangle& rHelpRect)
{
    // Gtk3 requires a rectangle be specified for the tooltip to display, X11 does not.
    rHelpRect = tools::Rectangle(Point(), GetOutputSizePixel());
    return m_sToolTip;
}

bool SvxCharView::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeft())
    {
        if (!(rMEvt.GetClicks() % 2) && maHasInsert)
        {
            InsertCharToDoc();
        }
        GrabFocus();
        Invalidate();
        maMouseClickHdl.Call(GetCharAndFont());
        return true;
    }

    return CustomWidgetController::MouseButtonDown(rMEvt);
}

bool SvxCharView::KeyInput(const KeyEvent& rKEvt)
{
    bool bRet = false;
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    switch (aCode.GetCode())
    {
        case KEY_SPACE:
        case KEY_RETURN:
            InsertCharToDoc();
            bRet = true;
            break;
    }
    return bRet;
}

bool SvxCharView::Command(const CommandEvent& rCommandEvent)
{
    if (rCommandEvent.GetCommand() == CommandEventId::ContextMenu)
    {
        GrabFocus();
        Invalidate();
        maContextMenuHdl.Call(rCommandEvent);
        return true;
    }

    return weld::CustomWidgetController::Command(rCommandEvent);
}

void SvxCharView::InsertCharToDoc() { SfxCharmapContainer::InsertCharToDoc(GetCharAndFont()); }

void SvxCharView::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.SetFont(maFont);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color aWindowTextColor(rStyleSettings.GetFieldTextColor());
    Color aHighlightColor(rStyleSettings.GetHighlightColor());
    Color aHighlightTextColor(rStyleSettings.GetHighlightTextColor());
    Color aFillColor(rStyleSettings.GetWindowColor());
    Color aTextColor(rStyleSettings.GetWindowTextColor());
    Color aShadowColor(rStyleSettings.GetShadowColor());

    const OUString aText = GetText();

    Size aSize(GetOutputSizePixel());
    tools::Long nAvailWidth = aSize.Width();
    tools::Long nWinHeight = aSize.Height();

    bool bGotBoundary = true;
    bool bShrankFont = false;
    vcl::Font aOrigFont(rRenderContext.GetFont());
    Size aFontSize(aOrigFont.GetFontSize());
    ::tools::Rectangle aBoundRect;

    mxVirDev->Push(PUSH_ALLFONT);
    mxVirDev->SetFont(maFont);
    tools::Long nY = (nWinHeight - mxVirDev->GetTextHeight()) / 2;
    mxVirDev->Pop();

    for (tools::Long nFontHeight = aFontSize.Height(); nFontHeight > 0; nFontHeight -= 1)
    {
        if (!rRenderContext.GetTextBoundRect(aBoundRect, aText) || aBoundRect.IsEmpty())
        {
            bGotBoundary = false;
            break;
        }

        //only shrink in the single glyph large view mode
        tools::Long nTextWidth = aBoundRect.GetWidth();
        if (nAvailWidth > nTextWidth)
            break;
        vcl::Font aFont(aOrigFont);
        aFontSize.setHeight(nFontHeight);
        aFont.SetFontSize(aFontSize);
        rRenderContext.SetFont(aFont);
        nY = (nWinHeight - rRenderContext.GetTextHeight()) / 2;
        bShrankFont = true;
    }

    Point aPoint(2, nY);

    if (!bGotBoundary)
        aPoint.setX((aSize.Width() - rRenderContext.GetTextWidth(aText)) / 2);
    else
    {
        // adjust position
        aBoundRect += aPoint;

        // vertical adjustment
        int nYLDelta = aBoundRect.Top();
        int nYHDelta = aSize.Height() - aBoundRect.Bottom();
        if (nYLDelta <= 0)
            aPoint.AdjustY(-(nYLDelta - 1));
        else if (nYHDelta <= 0)
            aPoint.AdjustY(nYHDelta - 1);

        // centrally align glyph
        aPoint.setX(-aBoundRect.Left() + (aSize.Width() - aBoundRect.GetWidth()) / 2);
    }

    // tdf#111924 - don't lose focus on context menu
    if (HasFocus() || HasChildFocus())
    {
        rRenderContext.SetFillColor(aHighlightColor);
        rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), aSize));

        rRenderContext.SetTextColor(aHighlightTextColor);
        rRenderContext.DrawText(aPoint, aText);
    }
    else
    {
        rRenderContext.SetFillColor(aFillColor);
        rRenderContext.SetLineColor(aShadowColor);
        rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), aSize));

        rRenderContext.SetTextColor(aWindowTextColor);
        rRenderContext.DrawText(aPoint, aText);
    }
    rRenderContext.SetFillColor(aFillColor);
    rRenderContext.SetTextColor(aTextColor);

    if (bShrankFont)
        rRenderContext.SetFont(aOrigFont);
}

void SvxCharView::setFocusInHdl(const Link<const CharAndFont&, void>& rLink)
{
    maFocusInHdl = rLink;
}

void SvxCharView::setMouseClickHdl(const Link<const CharAndFont&, void>& rLink)
{
    maMouseClickHdl = rLink;
}

void SvxCharView::setContextMenuHdl(const Link<const CommandEvent&, void>& rLink)
{
    maContextMenuHdl = rLink;
}

void SvxCharView::UpdateFont(const OUString& rFontFamilyName)
{
    tools::Long nWinHeight = GetOutputSizePixel().Height();
    maFont.SetFamilyName(rFontFamilyName);
    maFont.SetWeight(WEIGHT_NORMAL);
    maFont.SetAlignment(ALIGN_TOP);
    maFont.SetFontSize(mxVirDev->PixelToLogic(Size(0, nWinHeight / 2)));
    maFont.SetTransparent(true);

    Invalidate();
}

void SvxCharView::Resize()
{
    // force recalculation of size
    UpdateFont(maFont.GetFamilyName());
}

void SvxCharView::SetText(const OUString& rText)
{
    m_sText = rText;
    Invalidate();
}

CharAndFont SvxCharView::GetCharAndFont() const
{
    return CharAndFont(GetText(), GetFontFamilyName());
}

void SvxCharView::SetHasInsert(bool bInsert) { maHasInsert = bInsert; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
