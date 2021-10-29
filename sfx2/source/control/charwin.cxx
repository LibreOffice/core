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
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/charwin.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertyvalue.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

using namespace com::sun::star;

SvxCharView::SvxCharView(const VclPtr<VirtualDevice>& rVirDev)
    : mxVirDev(rVirDev)
    , mnY(0)
    , maPosition(0, 0)
    , maHasInsert(true)
{
}

void SvxCharView::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    vcl::Font aFont = rStyleSettings.GetLabelFont();
    const Size aFontSize = aFont.GetFontSize();
    aFont.SetFontSize(Size(aFontSize.Width() * 2.5, aFontSize.Height() * 2.5));
    mxVirDev->Push(PUSH_ALLFONT);
    mxVirDev->SetFont(aFont);
    pDrawingArea->set_size_request(mxVirDev->approximate_digit_width() * 2,
                                   mxVirDev->GetTextHeight());
    mxVirDev->Pop();
}

void SvxCharView::GetFocus() { Invalidate(); }

void SvxCharView::LoseFocus() { Invalidate(); }

bool SvxCharView::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeft())
    {
        if (!(rMEvt.GetClicks() % 2) && maHasInsert)
        {
            InsertCharToDoc();
        }

        maMouseClickHdl.Call(this);
    }

    if (rMEvt.IsRight())
    {
        Point aPosition(rMEvt.GetPosPixel());
        maPosition = aPosition;
        GrabFocus();
        Invalidate();
        createContextMenu();
    }

    return true;
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

void SvxCharView::InsertCharToDoc()
{
    if (GetText().isEmpty())
        return;

    uno::Sequence<beans::PropertyValue> aArgs{ comphelper::makePropertyValue("Symbols", GetText()),
                                               comphelper::makePropertyValue(
                                                   "FontName", maFont.GetFamilyName()) };

    comphelper::dispatchCommand(".uno:InsertSymbol", aArgs);
}

void SvxCharView::createContextMenu()
{
    weld::DrawingArea* pDrawingArea = GetDrawingArea();
    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(pDrawingArea, "sfx/ui/charviewmenu.ui"));
    std::unique_ptr<weld::Menu> xItemMenu(xBuilder->weld_menu("charviewmenu"));
    ContextMenuSelect(
        xItemMenu->popup_at_rect(pDrawingArea, tools::Rectangle(maPosition, Size(1, 1))));
    Invalidate();
}

void SvxCharView::ContextMenuSelect(std::string_view rMenuId)
{
    if (rMenuId == "clearchar")
        maClearClickHdl.Call(this);
    else if (rMenuId == "clearallchar")
        maClearAllClickHdl.Call(this);
}

void SvxCharView::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.SetFont(maFont);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color aWindowTextColor(rStyleSettings.GetFieldTextColor());
    Color aHighlightColor(rStyleSettings.GetHighlightColor());
    Color aHighlightTextColor(rStyleSettings.GetHighlightTextColor());
    Color aFillColor(rStyleSettings.GetWindowColor());
    Color aTextColor(rStyleSettings.GetWindowTextColor());

    const OUString aText = GetText();

    Size aSize(GetOutputSizePixel());
    tools::Long nAvailWidth = aSize.Width();
    tools::Long nWinHeight = aSize.Height();

    bool bGotBoundary = true;
    bool bShrankFont = false;
    vcl::Font aOrigFont(rRenderContext.GetFont());
    Size aFontSize(aOrigFont.GetFontSize());
    ::tools::Rectangle aBoundRect;

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
        mnY = (nWinHeight - rRenderContext.GetTextHeight()) / 2;
        bShrankFont = true;
    }

    Point aPoint(2, mnY);

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

    if (HasFocus())
    {
        rRenderContext.SetFillColor(aHighlightColor);
        rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), aSize));

        rRenderContext.SetTextColor(aHighlightTextColor);
        rRenderContext.DrawText(aPoint, aText);
    }
    else
    {
        rRenderContext.SetFillColor(aFillColor);
        rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), aSize));

        rRenderContext.SetTextColor(aWindowTextColor);
        rRenderContext.DrawText(aPoint, aText);
    }
    rRenderContext.SetFillColor(aFillColor);
    rRenderContext.SetTextColor(aTextColor);

    if (bShrankFont)
        rRenderContext.SetFont(aOrigFont);
}

void SvxCharView::setMouseClickHdl(const Link<SvxCharView*, void>& rLink)
{
    maMouseClickHdl = rLink;
}

void SvxCharView::setClearClickHdl(const Link<SvxCharView*, void>& rLink)
{
    maClearClickHdl = rLink;
}

void SvxCharView::setClearAllClickHdl(const Link<SvxCharView*, void>& rLink)
{
    maClearAllClickHdl = rLink;
}

void SvxCharView::SetFont(const vcl::Font& rFont)
{
    tools::Long nWinHeight = GetOutputSizePixel().Height();
    maFont = rFont;
    maFont.SetWeight(WEIGHT_NORMAL);
    maFont.SetAlignment(ALIGN_TOP);
    maFont.SetFontSize(mxVirDev->PixelToLogic(Size(0, nWinHeight / 2)));
    maFont.SetTransparent(true);

    mxVirDev->Push(PUSH_ALLFONT);
    mxVirDev->SetFont(maFont);
    mnY = (nWinHeight - mxVirDev->GetTextHeight()) / 2;
    mxVirDev->Pop();

    Invalidate();
}

void SvxCharView::Resize()
{
    SetFont(GetFont()); //force recalculation of size
}

void SvxCharView::SetText(const OUString& rText)
{
    m_sText = rText;
    Invalidate();
}

void SvxCharView::SetHasInsert(bool bInsert) { maHasInsert = bInsert; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
