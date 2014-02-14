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


#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include "cbutton.hxx"

//========================================================================
//  class ScDDComboBoxButton
//========================================================================

ScDDComboBoxButton::ScDDComboBoxButton( OutputDevice* pOutputDevice )
    :   pOut( pOutputDevice )
{
    SetOptSizePixel();
}

// -------------------------------------------------------------------------

ScDDComboBoxButton::~ScDDComboBoxButton()
{
}

// -------------------------------------------------------------------------

void ScDDComboBoxButton::SetOutputDevice( OutputDevice* pOutputDevice )
{
    pOut = pOutputDevice;
}

// -------------------------------------------------------------------------

void ScDDComboBoxButton::SetOptSizePixel()
{
    aBtnSize = pOut->LogicToPixel( Size(0,11), MAP_APPFONT );
    //aBtnSize.Width() = GetSystemMetrics( SM_CXVSCROLL ) - 1; // Win SDK-Funktion
    aBtnSize.Width() = pOut->GetSettings().GetStyleSettings().GetScrollBarSize();
}

// -------------------------------------------------------------------------

void ScDDComboBoxButton::Draw( const Point& rAt,
                               const Size&  rSize,
                               bool         bState,
                               bool         bBtnIn  /* = false */ )
{
    if ( rSize.Width() == 0 || rSize.Height() == 0 )
        return;     // #i43092# rectangle with size 0 would have RECT_EMPTY as end position

    // save old state
    sal_Bool        bHadFill   = pOut->IsFillColor();
    Color       aOldFill   = pOut->GetFillColor();
    sal_Bool        bHadLine   = pOut->IsLineColor();
    Color       aOldLine   = pOut->GetLineColor();
    sal_Bool        bOldEnable = pOut->IsMapModeEnabled();

    Rectangle   aBtnRect( rAt, rSize );
    Rectangle   aInnerRect = aBtnRect;

    pOut->EnableMapMode( false );

    DecorationView aDecoView( pOut);

    sal_uInt16 nButtonStyle = BUTTON_DRAW_DEFAULT;
    if( bBtnIn )    // gedrueckt?
    {
        nButtonStyle = BUTTON_DRAW_PRESSED;
    }

    aInnerRect=aDecoView.DrawButton( aBtnRect, nButtonStyle );


    aInnerRect.Left()   += 1;
    aInnerRect.Top()    += 1;
    aInnerRect.Right()  -= 1;
    aInnerRect.Bottom() -= 1;

    Size  aInnerSize   = aInnerRect.GetSize();
    Point aInnerCenter = aInnerRect.Center();

    aInnerRect.Top()   = aInnerCenter.Y() - (aInnerSize.Width()>>1);
    aInnerRect.Bottom()= aInnerCenter.Y() + (aInnerSize.Width()>>1);

    ImpDrawArrow( aInnerRect, bState );


    // restore old state
    pOut->EnableMapMode( bOldEnable );
    if (bHadLine)
        pOut->SetLineColor(aOldLine);
    else
        pOut->SetLineColor();
    if (bHadFill)
        pOut->SetFillColor(aOldFill);
    else
        pOut->SetFillColor();
}

//------------------------------------------------------------------------

void ScDDComboBoxButton::ImpDrawArrow( const Rectangle& rRect,
                                       bool             bState )
{
    // no need to save old line and fill color here (is restored after the call)

    Rectangle   aPixRect = rRect;
    Point       aCenter  = aPixRect.Center();
    Size        aSize    = aPixRect.GetSize();

    Size aSize3;
    aSize3.Width() = aSize.Width() >> 1;
    aSize3.Height() = aSize.Height() >> 1;

    Size aSize4;
    aSize4.Width() = aSize.Width() >> 2;
    aSize4.Height() = aSize.Height() >> 2;

    Rectangle aTempRect = aPixRect;

    const StyleSettings& rSett = Application::GetSettings().GetStyleSettings();
    Color aColor( bState ? COL_LIGHTBLUE : rSett.GetButtonTextColor().GetColor() );
    pOut->SetFillColor( aColor );
    pOut->SetLineColor( aColor );

    aTempRect.Left()   = aCenter.X() - aSize4.Width();
    aTempRect.Right()  = aCenter.X() + aSize4.Width();
    aTempRect.Top()    = aCenter.Y() - aSize3.Height();
    aTempRect.Bottom() = aCenter.Y() - 1;

    pOut->DrawRect( aTempRect );

    Point aPos1( aCenter.X()-aSize3.Width(), aCenter.Y() );
    Point aPos2( aCenter.X()+aSize3.Width(), aCenter.Y() );
    while( aPos1.X() <= aPos2.X() )
    {
        pOut->DrawLine( aPos1, aPos2 );
        aPos1.X()++; aPos2.X()--;
        aPos1.Y()++; aPos2.Y()++;
    }

    pOut->DrawLine( Point( aCenter.X() - aSize3.Width(), aPos1.Y()+1 ),
                    Point( aCenter.X() + aSize3.Width(), aPos1.Y()+1 ) );
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
