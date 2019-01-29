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
#include "LineWidthValueSet.hxx"

#include <i18nlangtag/mslangid.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>

namespace svx { namespace sidebar {

LineWidthValueSet::LineWidthValueSet(vcl::Window* pParent)
    : ValueSet(pParent, WB_TABSTOP)
    , nSelItem(0)
    , bCusEnable(false)
{
}

void LineWidthValueSet::Resize()
{
    SetColCount();
    SetLineCount(9);
    ValueSet::Resize();
}

LineWidthValueSet::~LineWidthValueSet()
{
    disposeOnce();
}

void LineWidthValueSet::SetUnit(std::array<OUString,9> const & strUnits)
{
    maStrUnits = strUnits;
}

void LineWidthValueSet::SetSelItem(sal_uInt16 nSel)
{
    nSelItem = nSel;
    if(nSel == 0)
    {
        SelectItem(1); // ,false); // 'false' nut supported by AOO
        SetNoSelection();
    }
    else
    {
        SelectItem(nSelItem);
        GrabFocus();
    }
}

void LineWidthValueSet::SetImage(const Image& img)
{
    imgCus = img;
}

void LineWidthValueSet::SetCusEnable(bool bEnable)
{
    bCusEnable = bEnable;
}

void  LineWidthValueSet::UserDraw( const UserDrawEvent& rUDEvt )
{
    tools::Rectangle aRect = rUDEvt.GetRect();
    vcl::RenderContext* pDev = rUDEvt.GetRenderContext();
    sal_uInt16  nItemId = rUDEvt.GetItemId();

    long nRectHeight = aRect.GetHeight();
    long nRectWidth = aRect.GetWidth();
    Point aBLPos = aRect.TopLeft();

    //const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    //Color aBackColor(0,0,200);
    //const Color aTextColor = rStyleSettings.GetFieldTextColor();
    vcl::Font aOldFont = pDev->GetFont();
    Color aOldColor = pDev->GetLineColor();
    Color aOldFillColor = pDev->GetFillColor();

    vcl::Font aFont(OutputDevice::GetDefaultFont(DefaultFontType::UI_SANS, MsLangId::getSystemLanguage(), GetDefaultFontFlags::OnlyOne));
    Size aSize = aFont.GetFontSize();
    aSize.setHeight( nRectHeight*3/5 );
    aFont.SetFontSize( aSize );

    Point aLineStart(aBLPos.X() + 5,            aBLPos.Y() + ( nRectHeight - nItemId )/2);
    Point aLineEnd(aBLPos.X() + nRectWidth * 7 / 9 - 10, aBLPos.Y() + ( nRectHeight - nItemId )/2);
    if(nItemId == 9)
    {
        Point aImgStart(aBLPos.X() + 5,         aBLPos.Y() + ( nRectHeight - 23 ) / 2);
        pDev->DrawImage(aImgStart, imgCus);
    //  Point aStart(aImgStart.X() + 14 + 20 , aBLPos.Y() + nRectHeight/6);
        tools::Rectangle aStrRect = aRect;
        aStrRect.AdjustTop(nRectHeight/6 );
        aStrRect.AdjustBottom( -(nRectHeight/6) );
        aStrRect.AdjustLeft(imgCus.GetSizePixel().Width() + 20 );
        if(bCusEnable)
            aFont.SetColor(GetSettings().GetStyleSettings().GetFieldTextColor());
        else
            aFont.SetColor(GetSettings().GetStyleSettings().GetDisableColor());

        pDev->SetFont(aFont);
        pDev->DrawText(aStrRect, maStrUnits[ nItemId - 1 ], DrawTextFlags::EndEllipsis);
    }
    else
    {
        if( nSelItem ==  nItemId )
        {
            tools::Rectangle aBackRect = aRect;
            aBackRect.AdjustTop(3 );
            aBackRect.AdjustBottom( -2 );
            pDev->SetFillColor(Color(50,107,197));
            pDev->DrawRect(aBackRect);
        }
        else
        {
            pDev->SetFillColor( COL_TRANSPARENT );
            pDev->DrawRect(aRect);
        }

        //draw text
        if(nSelItem ==  nItemId )
            aFont.SetColor(COL_WHITE);
        else
            aFont.SetColor(GetSettings().GetStyleSettings().GetFieldTextColor());
        pDev->SetFont(aFont);
        Point aStart(aBLPos.X() + nRectWidth * 7 / 9 , aBLPos.Y() + nRectHeight/6);
        pDev->DrawText(aStart, maStrUnits[ nItemId - 1 ]);  //can't set DrawTextFlags::EndEllipsis here ,or the text will disappear

        //draw line
        if( nSelItem ==  nItemId )
            pDev->SetLineColor(COL_WHITE);
        else
            pDev->SetLineColor(GetSettings().GetStyleSettings().GetFieldTextColor());

        for(sal_uInt16 i = 1; i <= nItemId; i++)
        {
            pDev->DrawLine(aLineStart,aLineEnd );
            aLineStart.setY(aLineStart.getY() + 1);
            aLineEnd.setY  (aLineEnd.getY() + 1);
        }
    }

    Invalidate( aRect );
    pDev->SetLineColor(aOldColor);
    pDev->SetFillColor(aOldFillColor);
    pDev->SetFont(aOldFont);
}

Size LineWidthValueSet::GetOptimalSize() const
{
    return LogicToPixel(Size(80, 12 * 9), MapMode(MapUnit::MapAppFont));
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
