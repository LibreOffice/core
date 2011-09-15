/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include "dpcontrol.hxx"

#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include "global.hxx"
#include "scitems.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "patattr.hxx"

using rtl::OUString;

ScDPFieldButton::ScDPFieldButton(OutputDevice* pOutDev, const StyleSettings* pStyle, const Fraction* pZoomX, const Fraction* pZoomY, ScDocument* pDoc) :
    mpDoc(pDoc),
    mpOutDev(pOutDev),
    mpStyle(pStyle),
    mbBaseButton(true),
    mbPopupButton(false),
    mbHasHiddenMember(false),
    mbPopupPressed(false),
    mbPopupLeft(false)
{
    if (pZoomX)
        maZoomX = *pZoomX;
    else
        maZoomX = Fraction(1, 1);

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
        maPos.X() -= maSize.Width() - 1;
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
    const long nMargin = 2;
    bool bOldMapEnablaed = mpOutDev->IsMapModeEnabled();
    mpOutDev->EnableMapMode(false);

    if (mbBaseButton)
    {
        // Background
        Rectangle aRect(maPos, maSize);
        mpOutDev->SetLineColor(mpStyle->GetFaceColor());
        mpOutDev->SetFillColor(mpStyle->GetFaceColor());
        mpOutDev->DrawRect(aRect);

        // Border lines
        mpOutDev->SetLineColor(mpStyle->GetLightColor());
        mpOutDev->DrawLine(Point(maPos), Point(maPos.X(), maPos.Y()+maSize.Height()-1));
        mpOutDev->DrawLine(Point(maPos), Point(maPos.X()+maSize.Width()-1, maPos.Y()));

        mpOutDev->SetLineColor(mpStyle->GetShadowColor());
        mpOutDev->DrawLine(Point(maPos.X(), maPos.Y()+maSize.Height()-1),
                           Point(maPos.X()+maSize.Width()-1, maPos.Y()+maSize.Height()-1));
        mpOutDev->DrawLine(Point(maPos.X()+maSize.Width()-1, maPos.Y()),
                           Point(maPos.X()+maSize.Width()-1, maPos.Y()+maSize.Height()-1));

        // Field name.
        // Get the font and size the same way as in scenario selection (lcl_DrawOneFrame in gridwin4.cxx)
        Font aTextFont( mpStyle->GetAppFont() );
        if ( mpDoc )
        {
            //  use ScPatternAttr::GetFont only for font size
            Font aAttrFont;
            static_cast<const ScPatternAttr&>(mpDoc->GetPool()->GetDefaultItem(ATTR_PATTERN)).
                GetFont( aAttrFont, SC_AUTOCOL_BLACK, mpOutDev, &maZoomY );
            aTextFont.SetSize( aAttrFont.GetSize() );
        }
        mpOutDev->SetFont(aTextFont);
        mpOutDev->SetTextColor(mpStyle->GetButtonTextColor());

        Point aTextPos = maPos;
        long nTHeight = mpOutDev->GetTextHeight();
        aTextPos.setX(maPos.getX() + nMargin);
        aTextPos.setY(maPos.getY() + (maSize.Height()-nTHeight)/2);

        mpOutDev->Push(PUSH_CLIPREGION);
        mpOutDev->IntersectClipRegion(aRect);
        mpOutDev->DrawText(aTextPos, maText);
        mpOutDev->Pop();
    }

    if (mbPopupButton)
        drawPopupButton();

    mpOutDev->EnableMapMode(bOldMapEnablaed);
}

void ScDPFieldButton::getPopupBoundingBox(Point& rPos, Size& rSize) const
{
    long nW = maSize.getWidth() / 2;
    long nH = maSize.getHeight();
    if (nW > 18)
        nW = 18;
    if (nH > 18)
        nH = 18;

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

    // Background & outer black border
    mpOutDev->SetLineColor(COL_BLACK);
    mpOutDev->SetFillColor(mpStyle->GetFaceColor());
    mpOutDev->DrawRect(Rectangle(aPos, aSize));

    if (!mbPopupPressed)
    {
        // border lines
        mpOutDev->SetLineColor(mpStyle->GetLightColor());
        mpOutDev->DrawLine(Point(aPos.X()+1, aPos.Y()+1), Point(aPos.X()+1, aPos.Y()+aSize.Height()-2));
        mpOutDev->DrawLine(Point(aPos.X()+1, aPos.Y()+1), Point(aPos.X()+aSize.Width()-2, aPos.Y()+1));

        mpOutDev->SetLineColor(mpStyle->GetShadowColor());
        mpOutDev->DrawLine(Point(aPos.X()+1, aPos.Y()+aSize.Height()-2),
                           Point(aPos.X()+aSize.Width()-2, aPos.Y()+aSize.Height()-2));
        mpOutDev->DrawLine(Point(aPos.X()+aSize.Width()-2, aPos.Y()+1),
                           Point(aPos.X()+aSize.Width()-2, aPos.Y()+aSize.Height()-2));
    }

    // the arrowhead
    Color aArrowColor = mbHasHiddenMember ? mpStyle->GetHighlightLinkColor() : mpStyle->GetButtonTextColor();
    mpOutDev->SetLineColor(aArrowColor);
    mpOutDev->SetFillColor(aArrowColor);
    Point aCenter(aPos.X() + (aSize.Width() >> 1), aPos.Y() + (aSize.Height() >> 1));
    Point aPos1, aPos2;
    aPos1.X() = aCenter.X() - 4;
    aPos2.X() = aCenter.X() + 4;
    aPos1.Y() = aCenter.Y() - 3;
    aPos2.Y() = aCenter.Y() - 3;

    if (mbPopupPressed)
    {
        aPos1.X() += 1;
        aPos2.X() += 1;
        aPos1.Y() += 1;
        aPos2.Y() += 1;
    }

    do
    {
        ++aPos1.X();
        --aPos2.X();
        ++aPos1.Y();
        ++aPos2.Y();
        mpOutDev->DrawLine(aPos1, aPos2);
    }
    while (aPos1 != aPos2);

    if (mbHasHiddenMember)
    {
        // tiny little box to display in presence of hidden member(s).
        Point aBoxPos(aPos.X() + aSize.Width() - 5, aPos.Y() + aSize.Height() - 5);
        if (mbPopupPressed)
        {
            aBoxPos.X() += 1;
            aBoxPos.Y() += 1;
        }
        Size aBoxSize(3, 3);
        mpOutDev->DrawRect(Rectangle(aBoxPos, aBoxSize));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
