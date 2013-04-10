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
#include "LineStyleValueSet.hxx"

#include <i18nlangtag/mslangid.hxx>


namespace svx { namespace sidebar {

LineStyleValueSet::LineStyleValueSet (
    Window* pParent,
    const ResId& rResId)
    : ValueSet( pParent, rResId ),
      pVDev(NULL),
      mbSelectFirst(true)
{
    SetColCount( 1 );
}




LineStyleValueSet::~LineStyleValueSet (void)
{
    delete pVDev;
}




void LineStyleValueSet::SetFirstString(XubString str)
{
    strNone = str;
}




void  LineStyleValueSet::UserDraw( const UserDrawEvent& rUDEvt )
{
    Rectangle aRect = rUDEvt.GetRect();
    OutputDevice*  pDev = rUDEvt.GetDevice();
    sal_uInt16  nItemId = rUDEvt.GetItemId();

    long nRectHeight = aRect.GetHeight();
    //Point aBLPos = aRect.TopLeft();
    if(nItemId == 1)
    {
        Color aOldFillColor = pDev->GetFillColor();

        //draw back
        if( mbSelectFirst )
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
        pDev->SetFillColor(aOldFillColor);

        //draw text
        Font aOldFont = pDev->GetFont();
        Color aOldColor = pDev->GetLineColor();
        Font aFont(OutputDevice::GetDefaultFont(
                    DEFAULTFONT_UI_SANS, MsLangId::getSystemLanguage(), DEFAULTFONT_FLAGS_ONLYONE));
        Size aSize = aFont.GetSize();
        aSize.Height() = nRectHeight*3/5;
        if( mbSelectFirst )
            aFont.SetColor(COL_WHITE);
        else
            //aFont.SetColor(COL_BLACK);
            aFont.SetColor(GetSettings().GetStyleSettings().GetFieldTextColor()); //high contrast
        aFont.SetFillColor(COL_BLUE);
        aFont.SetSize( aSize );
        pDev->SetFont(aFont);
//      String sText("None", 9, RTL_TEXTENCODING_ASCII_US);

    //  Point aStart(aBLPos.X() + 8 , aBLPos.Y() + nRectHeight/6);
        Rectangle aStrRect = aRect;
        aStrRect.Top() += nRectHeight/6;
        aStrRect.Bottom() -= nRectHeight/6;
        aStrRect.Left() += 8;
        pDev->DrawText(aStrRect, strNone, TEXT_DRAW_ENDELLIPSIS);

        pDev->SetFont(aOldFont);
        pDev->SetLineColor(aOldColor);


    }
    Invalidate( aRect );
}




void  LineStyleValueSet::SetFirstSelect(bool bSel)
{
    mbSelectFirst = bSel;
}



} } // end of namespace svx::sidebar

// eof
