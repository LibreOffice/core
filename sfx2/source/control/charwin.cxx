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
#include <vcl/weldutils.hxx>
#include <sfx2/charwin.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertyvalue.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <o3tl/temporary.hxx>
#include <unicode/uchar.h>
#include <unicode/utypes.h>

using namespace com::sun::star;

SvxCharView::SvxCharView(const VclPtr<VirtualDevice>& rVirDev)
    : mxVirDev(rVirDev)
    , mnY(0)
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
    mxVirDev->Push(PUSH_ALLFONT);
    weld::SetPointFont(*mxVirDev, aFont);
    pDrawingArea->set_size_request(mxVirDev->approximate_digit_width() * 2,
                                   mxVirDev->GetTextHeight());
    mxVirDev->Pop();
}

void SvxCharView::GetFocus()
{
    Invalidate();
    if (maFocusInHdl.IsSet())
        maFocusInHdl.Call(this);
}

void SvxCharView::LoseFocus() { Invalidate(); }

bool SvxCharView::GetDecimalValueAndCharName(sal_UCS4& rDecimalValue, OUString& rCharName)
{
    OUString charValue = GetText();
    if (charValue.isEmpty())
        return false;

    sal_UCS4 nDecimalValue = charValue.iterateCodePoints(&o3tl::temporary(sal_Int32(1)), -1);
    /* get the character name */
    UErrorCode errorCode = U_ZERO_ERROR;
    // icu has a private uprv_getMaxCharNameLength function which returns the max possible
    // length of this property. Unicode 3.2 max char name length was 83
    char buffer[100];
    u_charName(nDecimalValue, U_UNICODE_CHAR_NAME, buffer, sizeof(buffer), &errorCode);
    if (U_SUCCESS(errorCode))
    {
        rDecimalValue = nDecimalValue;
        rCharName = OUString::createFromAscii(buffer);
        return true;
    }
    return false;
}

OUString SvxCharView::GetCharInfoText()
{
    sal_UCS4 nDecimalValue = 0;
    OUString sCharName;
    const bool bSuccess = GetDecimalValueAndCharName(nDecimalValue, sCharName);
    if (bSuccess)
    {
        OUString aHexText = OUString::number(nDecimalValue, 16).toAsciiUpperCase();
        return GetText() + u" " + sCharName + u" U+" + aHexText;
    }
    return OUString();
}

OUString SvxCharView::RequestHelp(tools::Rectangle& rHelpRect)
{
    // Gtk3 requires a rectangle be specified for the tooltip to display, X11 does not.
    rHelpRect = tools::Rectangle(Point(), GetOutputSizePixel());
    return GetCharInfoText();
}

bool SvxCharView::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeft())
    {
        if (!(rMEvt.GetClicks() % 2) && maHasInsert)
        {
            InsertCharToDoc();
        }

        maMouseClickHdl.Call(this);
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
        createContextMenu(rCommandEvent.GetMousePosPixel());
        return true;
    }

    return weld::CustomWidgetController::Command(rCommandEvent);
}

void SvxCharView::InsertCharToDoc()
{
    if (GetText().isEmpty())
        return;

    uno::Sequence<beans::PropertyValue> aArgs{
        comphelper::makePropertyValue(u"Symbols"_ustr, GetText()),
        comphelper::makePropertyValue(u"FontName"_ustr, maFont.GetFamilyName())
    };

    comphelper::dispatchCommand(u".uno:InsertSymbol"_ustr, aArgs);
}

void SvxCharView::createContextMenu(const Point& rPosition)
{
    weld::DrawingArea* pDrawingArea = GetDrawingArea();
    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(pDrawingArea, u"sfx/ui/charviewmenu.ui"_ustr));
    std::unique_ptr<weld::Menu> xItemMenu(xBuilder->weld_menu(u"charviewmenu"_ustr));
    ContextMenuSelect(
        xItemMenu->popup_at_rect(pDrawingArea, tools::Rectangle(rPosition, Size(1, 1))));
    Invalidate();
}

void SvxCharView::ContextMenuSelect(std::u16string_view rMenuId)
{
    if (rMenuId == u"clearchar")
        maClearClickHdl.Call(this);
    else if (rMenuId == u"clearallchar")
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

void SvxCharView::setFocusInHdl(const Link<SvxCharView*, void>& rLink) { maFocusInHdl = rLink; }

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

    OUString sName;
    if (GetDecimalValueAndCharName(o3tl::temporary(sal_UCS4()), sName))
        SetAccessibleName(sName);
    else
        SetAccessibleName(OUString());
}

void SvxCharView::SetHasInsert(bool bInsert) { maHasInsert = bInsert; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
