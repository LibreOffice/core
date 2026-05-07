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

#include <dpcontrol.hxx>

#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <comphelper/lok.hxx>
#include <scitems.hxx>
#include <document.hxx>
#include <docpool.hxx>
#include <patattr.hxx>
#include <svtools/colorcfg.hxx>
#include <tools/mapunit.hxx>

ScDPFieldButton::ScDPFieldButton(OutputDevice* pOutDev, const StyleSettings& rStyle, double fZoomY, ScDocument& rDoc):
    mfZoomY(fZoomY),
    mrDoc(rDoc),
    mpOutDev(pOutDev),
    mrStyle(rStyle),
    mnToggleIndent(0),
    mbBaseButton(true),
    mbPopupButton(false),
    mbPopupButtonMulti(false),
    mbToggleButton(false),
    mbToggleCollapse(false),
    mbHasHiddenMember(false),
    mbPopupPressed(false),
    mbPopupLeft(false)
{
}

ScDPFieldButton::~ScDPFieldButton()
{
}

void ScDPFieldButton::setText(const OUString& rText)
{
    maText = rText;
}

void ScDPFieldButton::setBoundingBox(const Point& rPos, const Size& rSize, bool bLayoutRTL)
{
    maPos = rPos;
    maSize = rSize;
    if (bLayoutRTL)
    {
        // rPos is the logical-left position, adjust maPos to visual-left (inside the cell border)
        maPos.AdjustX( -(maSize.Width() - 1) );
    }
}

void ScDPFieldButton::setDrawBaseButton(bool b)
{
    mbBaseButton = b;
}

void ScDPFieldButton::setDrawPopupButton(bool b)
{
    mbPopupButton = b;
}

void ScDPFieldButton::setDrawPopupButtonMulti(bool b)
{
    mbPopupButtonMulti = b;
}

void ScDPFieldButton::setDrawToggleButton(bool b, bool bCollapse, sal_Int32 nIndent)
{
    mbToggleButton = b;
    mbToggleCollapse = bCollapse;
    mnToggleIndent = nIndent;
}

void ScDPFieldButton::setHasHiddenMember(bool b)
{
    mbHasHiddenMember = b;
}

void ScDPFieldButton::setPopupPressed(bool b)
{
    mbPopupPressed = b;
}

void ScDPFieldButton::setPopupLeft(bool b)
{
    mbPopupLeft = b;
}

void ScDPFieldButton::draw()
{
    bool bOldMapEnabled = mpOutDev->IsMapModeEnabled();

    if (mpOutDev->GetMapMode().GetMapUnit() != MapUnit::MapPixel)
        mpOutDev->EnableMapMode(false);

    if (mbBaseButton)
    {
        // For pivot field cells, the cell content is rendered by
        // ScOutputData::DrawEditStandard / DrawStrings using the cell's
        // pattern (font, color, background, alignment, wrap). Painting a
        // button-face fill + single-line caption here would overwrite that
        // and ignore wrap and vertical alignment from the pattern. Excel
        // doesn't draw a 3D button-face for pivot fields either; it lets
        // the cell-style framework (dxf / pivotTableStyleInfo) drive the
        // appearance, and only overlays the dropdown arrow.
        // Keep just the 3D border lines so the cell still reads as a
        // clickable field button.
        mpOutDev->SetLineColor(mrStyle.GetLightColor());
        mpOutDev->DrawLine(maPos, Point(maPos.X(), maPos.Y()+maSize.Height()-1));
        mpOutDev->DrawLine(maPos, Point(maPos.X()+maSize.Width()-1, maPos.Y()));

        mpOutDev->SetLineColor(mrStyle.GetShadowColor());
        mpOutDev->DrawLine(Point(maPos.X(), maPos.Y()+maSize.Height()-1),
                           Point(maPos.X()+maSize.Width()-1, maPos.Y()+maSize.Height()-1));
        mpOutDev->DrawLine(Point(maPos.X()+maSize.Width()-1, maPos.Y()),
                           Point(maPos.X()+maSize.Width()-1, maPos.Y()+maSize.Height()-1));
    }

    if (mbPopupButton || mbPopupButtonMulti)
        drawPopupButton();

    if (mbToggleButton)
        drawToggleButton();

    mpOutDev->EnableMapMode(bOldMapEnabled);
}

void ScDPFieldButton::getPopupBoundingBox(Point& rPos, Size& rSize) const
{
    float fScaleFactor = mpOutDev->GetDPIScaleFactor();

    tools::Long nMaxSize = 18 * fScaleFactor; // Button max size in either dimension

    tools::Long nW = std::min(maSize.getWidth() / 2, nMaxSize);
    tools::Long nH = std::min(maSize.getHeight(),    nMaxSize);

    double fZoom = mfZoomY > 1.0 ? mfZoomY : 1.0;
    if (fZoom > 1.0)
    {
        nW = fZoom * (nW - 1);
        nH = fZoom * (nH - 1);
    }

    // #i114944# AutoFilter button is left-aligned in RTL.
    // DataPilot button is always right-aligned for now, so text output isn't affected.
    if (mbPopupLeft)
        rPos.setX(maPos.getX());
    else
        rPos.setX(maPos.getX() + maSize.getWidth() - nW);

    rPos.setY(maPos.getY() + maSize.getHeight() - nH);
    rSize.setWidth(nW);
    rSize.setHeight(nH);
}

void ScDPFieldButton::getToggleBoundingBox(Point& rPos, Size& rSize) const
{
    const float fScaleFactor = mpOutDev->GetDPIScaleFactor();

    tools::Long nMaxSize = 13 * fScaleFactor; // Button max size in either dimension
    tools::Long nMargin = 3 * fScaleFactor;

    tools::Long nIndent = fScaleFactor * o3tl::convert(mnToggleIndent, o3tl::Length::twip, o3tl::Length::px);
    tools::Long nW = std::min(maSize.getWidth() / 2, nMaxSize);
    tools::Long nH = std::min(maSize.getHeight(),    nMaxSize);
    nIndent = std::min(nIndent, maSize.getWidth());

    double fZoom = mfZoomY > 1.0 ? mfZoomY : 1.0;
    if (fZoom > 1.0)
    {
        nW = fZoom * (nW - 1);
        nH = fZoom * (nH - 1);
        nIndent = fZoom * (nIndent -1);
        nMargin = fZoom * (nMargin - 1);
    }

    // FIXME: RTL case ?
    rPos.setX(maPos.getX() + nIndent - nW + nMargin);
    rPos.setY(maPos.getY() + maSize.getHeight() / 2 - nH / 2 + nMargin);
    rSize.setWidth(nW - nMargin - 1);
    rSize.setHeight(nH - nMargin - 1);
}

void ScDPFieldButton::drawPopupButton()
{
    Point aPos;
    Size aSize;
    getPopupBoundingBox(aPos, aSize);

    float fScaleFactor = mpOutDev->GetDPIScaleFactor();

    // Button background color
    Color aFaceColor = mrStyle.GetFaceColor();
    Color aBackgroundColor
        = mbHasHiddenMember ? mrStyle.GetHighlightColor()
                            : mbPopupPressed ? mrStyle.GetShadowColor() : aFaceColor;

    // Button line color
    mpOutDev->SetLineColor(mrStyle.GetLabelTextColor());
    // If the document background is light and face color is dark, use ShadowColor instead
    Color aDocColor = svtools::ColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
    if (aDocColor.IsBright() && aFaceColor.IsDark())
        mpOutDev->SetLineColor(mrStyle.GetShadowColor());

    mpOutDev->SetFillColor(aBackgroundColor);
    mpOutDev->DrawRect(tools::Rectangle(aPos, aSize));

    // the arrowhead
    Color aArrowColor = mbHasHiddenMember ? mrStyle.GetHighlightTextColor() : mrStyle.GetButtonTextColor();
    // FIXME: HACK: The following DrawPolygon draws twice in lok rtl mode for some reason.
    // => one at the correct location with fill (possibly no outline)
    // => and the other at an x offset with outline and without fill
    // eg. Replacing this with a DrawRect() does not have any such problems.
    comphelper::LibreOfficeKit::isActive() ? mpOutDev->SetLineColor() : mpOutDev->SetLineColor(aArrowColor);
    mpOutDev->SetFillColor(aArrowColor);

    Point aCenter(aPos.X() + (aSize.Width() / 2), aPos.Y() + (aSize.Height() / 2));

    Size aArrowSize(4 * fScaleFactor, 2 * fScaleFactor);

    tools::Polygon aPoly(3);
    aPoly.SetPoint(Point(aCenter.X() - aArrowSize.Width(), aCenter.Y() - aArrowSize.Height()), 0);
    aPoly.SetPoint(Point(aCenter.X() + aArrowSize.Width(), aCenter.Y() - aArrowSize.Height()), 1);
    aPoly.SetPoint(Point(aCenter.X(),                      aCenter.Y() + aArrowSize.Height()), 2);
    mpOutDev->DrawPolygon(aPoly);

    if (mbHasHiddenMember)
    {
        // Visible filter-active indicator at the bottom-right of the popup
        // button. Drawn as a Unicode outlined-triangle glyph (U+25BD ▽) in
        // Excel-blue — fixed colour regardless of OS theme so users can spot
        // filtered fields at a glance, matching Excel's pivot-button cue.
        const OUString sFilterGlyph(static_cast<sal_Unicode>(0x25BD));
        const Color aFilterCueColor(0x00, 0x70, 0xC0); // Excel filter blue
        mpOutDev->Push(vcl::PushFlags::FONT | vcl::PushFlags::TEXTCOLOR);
        vcl::Font aGlyphFont = mpOutDev->GetFont();
        aGlyphFont.SetFontHeight(static_cast<tools::Long>(8 * fScaleFactor));
        aGlyphFont.SetWeight(WEIGHT_BOLD);
        mpOutDev->SetFont(aGlyphFont);
        mpOutDev->SetTextColor(aFilterCueColor);
        const Size aGlyphSize(mpOutDev->GetTextWidth(sFilterGlyph),
                              mpOutDev->GetTextHeight());
        const Point aGlyphPos(
            aPos.X() + aSize.Width()  - aGlyphSize.Width()  - static_cast<tools::Long>(fScaleFactor),
            aPos.Y() + aSize.Height() - aGlyphSize.Height());
        mpOutDev->DrawText(aGlyphPos, sFilterGlyph);
        mpOutDev->Pop();
    }
}

void ScDPFieldButton::drawToggleButton()
{
    Point aPos;
    Size aSize;
    getToggleBoundingBox(aPos, aSize);

    // Background & outer black border
    mpOutDev->SetLineColor(COL_BLACK);
    mpOutDev->SetFillColor();
    mpOutDev->DrawRect(tools::Rectangle(aPos, aSize));

    Point aCenter(aPos.X() + aSize.getWidth() / 2, aPos.Y() + aSize.getHeight() / 2);

    mpOutDev->DrawLine(
        Point(aPos.X() + 2, aCenter.Y()),
        Point(aPos.X() + aSize.getWidth() - 2, aCenter.Y()));

    if (!mbToggleCollapse)
    {
        mpOutDev->DrawLine(
            Point(aCenter.X(), aPos.Y() + 2),
            Point(aCenter.X(), aPos.Y() + aSize.getHeight() - 2));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
