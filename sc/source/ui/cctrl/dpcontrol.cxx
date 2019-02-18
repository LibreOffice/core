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
#include <scitems.hxx>
#include <document.hxx>
#include <docpool.hxx>
#include <patattr.hxx>

ScDPFieldButton::ScDPFieldButton(OutputDevice* pOutDev, const StyleSettings* pStyle, const Fraction* pZoomY, ScDocument* pDoc) :
    mpDoc(pDoc),
    mpOutDev(pOutDev),
    mpStyle(pStyle),
    mbBaseButton(true),
    mbPopupButton(false),
    mbHasHiddenMember(false),
    mbPopupPressed(false),
    mbPopupLeft(false)
{
    if (pZoomY)
        maZoomY = *pZoomY;
    else
        maZoomY = Fraction(1, 1);
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
    if (mbBaseButton)
    {
        // Background
        tools::Rectangle aRect(maPos, maSize);
        mpOutDev->SetLineColor(mpStyle->GetFaceColor());
        mpOutDev->SetFillColor(mpStyle->GetFaceColor());
        mpOutDev->DrawRect(aRect);

        // Border lines
        mpOutDev->SetLineColor(mpStyle->GetLightColor());
        mpOutDev->DrawLine(maPos, Point(maPos.X(), maPos.Y()+maSize.Height()-1));
        mpOutDev->DrawLine(maPos, Point(maPos.X()+maSize.Width()-1, maPos.Y()));

        mpOutDev->SetLineColor(mpStyle->GetShadowColor());
        mpOutDev->DrawLine(Point(maPos.X(), maPos.Y()+maSize.Height()-1),
                           Point(maPos.X()+maSize.Width()-1, maPos.Y()+maSize.Height()-1));
        mpOutDev->DrawLine(Point(maPos.X()+maSize.Width()-1, maPos.Y()),
                           Point(maPos.X()+maSize.Width()-1, maPos.Y()+maSize.Height()-1));

        // Field name.
        // Get the font and size the same way as in scenario selection (lcl_DrawOneFrame in gridwin4.cxx)
        vcl::Font aTextFont( mpStyle->GetAppFont() );
        if ( mpDoc )
        {
            //  use ScPatternAttr::GetFont only for font size
            vcl::Font aAttrFont;
            mpDoc->GetPool()->GetDefaultItem(ATTR_PATTERN).
                GetFont( aAttrFont, SC_AUTOCOL_BLACK, mpOutDev, &maZoomY );
            aTextFont.SetFontSize( aAttrFont.GetFontSize() );
        }
        mpOutDev->SetFont(aTextFont);
        mpOutDev->SetTextColor(mpStyle->GetButtonTextColor());

        Point aTextPos = maPos;
        long nTHeight = mpOutDev->GetTextHeight();
        aTextPos.setX(maPos.getX() + 2); // 2 = Margin
        aTextPos.setY(maPos.getY() + (maSize.Height()-nTHeight)/2);

        mpOutDev->Push(PushFlags::CLIPREGION);
        mpOutDev->IntersectClipRegion(aRect);
        mpOutDev->DrawText(aTextPos, maText);
        mpOutDev->Pop();
    }

    if (mbPopupButton)
        drawPopupButton();
}

void ScDPFieldButton::getPopupBoundingBox(Point& rPos, Size& rSize) const
{
    float fScaleFactor = mpOutDev->GetDPIScaleFactor();

    long nMaxSize = 18 * fScaleFactor; // Button max size in either dimension

    long nW = std::min(maSize.getWidth() / 2, nMaxSize);
    long nH = std::min(maSize.getHeight(),    nMaxSize);

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

void ScDPFieldButton::drawPopupButton()
{
    Point aPos;
    Size aSize;
    getPopupBoundingBox(aPos, aSize);

    float fScaleFactor = mpOutDev->GetDPIScaleFactor();

    // Background & outer black border
    mpOutDev->SetLineColor(COL_BLACK);
    Color aBackgroundColor = mbPopupPressed ? mpStyle->GetShadowColor() : mpStyle->GetFaceColor();
    mpOutDev->SetFillColor(aBackgroundColor);
    mpOutDev->DrawRect(tools::Rectangle(aPos, aSize));

    // the arrowhead
    Color aArrowColor = mbHasHiddenMember ? mpStyle->GetHighlightLinkColor() : mpStyle->GetButtonTextColor();
    mpOutDev->SetLineColor(aArrowColor);
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
        // tiny little box to display in presence of hidden member(s).
        Point aBoxPos(aPos.X() + aSize.Width() - 5 * fScaleFactor, aPos.Y() + aSize.Height() - 5 * fScaleFactor);
        Size aBoxSize(3 * fScaleFactor, 3 * fScaleFactor);
        mpOutDev->DrawRect(tools::Rectangle(aBoxPos, aBoxSize));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
