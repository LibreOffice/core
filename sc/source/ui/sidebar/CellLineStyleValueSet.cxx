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

#include "CellLineStyleValueSet.hxx"
#include <i18nlangtag/mslangid.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>

namespace sc { namespace sidebar {

CellLineStyleValueSet::CellLineStyleValueSet(vcl::Window* pParent)
    : ValueSet(pParent, WB_TABSTOP)
    , nSelItem(0)
{
    SetColCount();
    SetLineCount( 9);
}

CellLineStyleValueSet::~CellLineStyleValueSet()
{
    disposeOnce();
}

Size CellLineStyleValueSet::GetOptimalSize() const
{
    return LogicToPixel(Size(80, 12 * 9), MapMode(MapUnit::MapAppFont));
}

void CellLineStyleValueSet::SetUnit(const OUString* str)
{
    for (int i = 0; i < CELL_LINE_STYLE_ENTRIES; ++i)
    {
        maStrUnit[i] = str[i];
    }
}

void CellLineStyleValueSet::SetSelItem(sal_uInt16 nSel)
{
    nSelItem = nSel;
    if(nSel == 0)
    {
        SelectItem(1);
        SetNoSelection();
    }
    else
    {
        SelectItem(nSelItem);
        GrabFocus();
    }
}

void CellLineStyleValueSet::UserDraw( const UserDrawEvent& rUDEvt )
{
    tools::Rectangle aRect = rUDEvt.GetRect();
    vcl::RenderContext* pDev = rUDEvt.GetRenderContext();
    sal_uInt16  nItemId = rUDEvt.GetItemId();

    long nRectHeight = aRect.GetHeight();
    long nRectWidth = aRect.GetWidth();
    Point aBLPos = aRect.TopLeft();

    vcl::Font aOldFont = pDev->GetFont();
    Color aOldColor = pDev->GetLineColor();
    Color aOldFillColor = pDev->GetFillColor();

    vcl::Font aFont(OutputDevice::GetDefaultFont(DefaultFontType::UI_SANS, MsLangId::getSystemLanguage(), GetDefaultFontFlags::OnlyOne));
    Size aSize = aFont.GetFontSize();
    aSize.setHeight( nRectHeight*3/5 );
    aFont.SetFontSize( aSize );

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
    if (nSelItem ==  nItemId )
        aFont.SetColor(COL_WHITE);
    else
        aFont.SetColor(GetSettings().GetStyleSettings().GetFieldTextColor()); //high contrast

    pDev->SetFont(aFont);
    long nTextWidth = pDev->GetTextWidth(maStrUnit[nItemId - 1]);
    long nTLX = aBLPos.X() + 5,  nTLY = aBLPos.Y() + ( nRectHeight - nItemId )/2;
    long nTRX = aBLPos.X() + nRectWidth - nTextWidth - 15, nTRY = aBLPos.Y() + ( nRectHeight - nItemId )/2;
    Point aStart(aBLPos.X() + nRectWidth - nTextWidth - 5 , aBLPos.Y() + nRectHeight/6);
    pDev->DrawText(aStart, maStrUnit[nItemId - 1]); //can't set DrawTextFlags::EndEllipsis here, or the text will disappear

    //draw line
    if( nSelItem ==  nItemId )
    {
        pDev->SetFillColor(COL_WHITE);
        pDev->SetLineColor(COL_WHITE);
    }
    else
    {
        pDev->SetFillColor(GetSettings().GetStyleSettings().GetFieldTextColor());   //high contrast
        pDev->SetLineColor(GetSettings().GetStyleSettings().GetFieldTextColor());   //high contrast
    }

    switch( nItemId )
    {
        case 1:
        case 2:
        case 3:
        case 4:
            pDev->DrawRect(tools::Rectangle(nTLX, nTLY , nTRX, nTRY + nItemId * 2 - 1 ));
            break;
        case 5:
            pDev->DrawRect(tools::Rectangle(nTLX, nTLY , nTRX, nTRY + 1 ));
            pDev->DrawRect(tools::Rectangle(nTLX, nTLY + 3 , nTRX, nTRY + 4 ));
            break;
        case 6:
            pDev->DrawRect(tools::Rectangle(nTLX, nTLY , nTRX, nTRY + 1 ));
            pDev->DrawRect(tools::Rectangle(nTLX, nTLY + 5 , nTRX, nTRY + 6 ));
            break;
        case 7:
            pDev->DrawRect(tools::Rectangle(nTLX, nTLY , nTRX, nTRY + 1 ));
            pDev->DrawRect(tools::Rectangle(nTLX, nTLY + 3 , nTRX, nTRY + 6 ));
            break;
        case 8:
            pDev->DrawRect(tools::Rectangle(nTLX, nTLY , nTRX, nTRY + 3 ));
            pDev->DrawRect(tools::Rectangle(nTLX, nTLY + 5 , nTRX, nTRY + 6 ));
            break;
        case 9:
            pDev->DrawRect(tools::Rectangle(nTLX, nTLY , nTRX, nTRY + 3 ));
            pDev->DrawRect(tools::Rectangle(nTLX, nTLY + 5 , nTRX, nTRY + 8 ));
            break;
    }

    Invalidate( aRect );
    pDev->SetLineColor(aOldColor);
    pDev->SetFillColor(aOldFillColor);
    pDev->SetFont(aOldFont);

}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
