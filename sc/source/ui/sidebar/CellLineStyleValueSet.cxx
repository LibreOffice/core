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

#include <CellLineStyleValueSet.hxx>
#include <i18nlangtag/mslangid.hxx>

namespace sc { namespace sidebar {

CellLineStyleValueSet::CellLineStyleValueSet( Window* pParent, const ResId& rResId)
:   ValueSet( pParent, rResId ),
    pVDev(NULL),
    nSelItem(0)
{
    SetColCount( 1 );
    SetLineCount( 9);
}

CellLineStyleValueSet::~CellLineStyleValueSet()
{
    delete pVDev;
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

void CellLineStyleValueSet::SetImage(Image img)
{
    imgCus = img;
}

void CellLineStyleValueSet::UserDraw( const UserDrawEvent& rUDEvt )
{
    Rectangle aRect = rUDEvt.GetRect();
    OutputDevice*  pDev = rUDEvt.GetDevice();
    sal_uInt16  nItemId = rUDEvt.GetItemId();

    long nRectHeight = aRect.GetHeight();
    long nRectWidth = aRect.GetWidth();
    Point aBLPos = aRect.TopLeft();

    Font aOldFont = pDev->GetFont();
    Color aOldColor = pDev->GetLineColor();
    Color aOldFillColor = pDev->GetFillColor();

    Font aFont(OutputDevice::GetDefaultFont(DEFAULTFONT_UI_SANS, MsLangId::getSystemLanguage(), DEFAULTFONT_FLAGS_ONLYONE));
    Size aSize = aFont.GetSize();
    aSize.Height() = nRectHeight*3/5;
    aFont.SetSize( aSize );

    long  nTLX = aBLPos.X() + 5,  nTLY = aBLPos.Y() + ( nRectHeight - nItemId )/2;
    long  nTRX = aBLPos.X() + nRectWidth * 7 / 9 - 15, nTRY = aBLPos.Y() + ( nRectHeight - nItemId )/2;

    if( nSelItem ==  nItemId )
    {
        Color aBackColor(50,107,197);
        Rectangle aBackRect = aRect;
        aBackRect.Top() += 3;
        aBackRect.Bottom() -= 2;
        pDev->SetFillColor(aBackColor);
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
        aFont.SetColor(GetSettings().GetStyleSettings().GetFieldTextColor()); //high contrast

    pDev->SetFont(aFont);
    Point aStart(aBLPos.X() + nRectWidth * 7 / 9 - 5 , aBLPos.Y() + nRectHeight/6);
    pDev->DrawText(aStart, maStrUnit[nItemId - 1]); //can't set TEXT_DRAW_ENDELLIPSIS here, or the text will disapear

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
            pDev->DrawRect(Rectangle(nTLX, nTLY , nTRX, nTRY + nItemId * 2 - 1 ));
            break;
        case 5:
            pDev->DrawRect(Rectangle(nTLX, nTLY , nTRX, nTRY + 1 ));
            pDev->DrawRect(Rectangle(nTLX, nTLY + 3 , nTRX, nTRY + 4 ));
            break;
        case 6:
            pDev->DrawRect(Rectangle(nTLX, nTLY , nTRX, nTRY + 1 ));
            pDev->DrawRect(Rectangle(nTLX, nTLY + 5 , nTRX, nTRY + 6 ));
            break;
        case 7:
            pDev->DrawRect(Rectangle(nTLX, nTLY , nTRX, nTRY + 1 ));
            pDev->DrawRect(Rectangle(nTLX, nTLY + 3 , nTRX, nTRY + 6 ));
            break;
        case 8:
            pDev->DrawRect(Rectangle(nTLX, nTLY , nTRX, nTRY + 3 ));
            pDev->DrawRect(Rectangle(nTLX, nTLY + 5 , nTRX, nTRY + 6 ));
            break;
        case 9:
            pDev->DrawRect(Rectangle(nTLX, nTLY , nTRX, nTRY + 3 ));
            pDev->DrawRect(Rectangle(nTLX, nTLY + 5 , nTRX, nTRY + 8 ));
            break;
    }

    Invalidate( aRect );
    pDev->SetLineColor(aOldColor);
    pDev->SetFillColor(aOldFillColor);
    pDev->SetFont(aOldFont);

}

} } // end of namespace svx::sidebar

// eof
