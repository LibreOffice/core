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
                               sal_Bool         bState,
                               sal_Bool         bBtnIn  /* = sal_False */ )
{
    if ( rSize.Width() == 0 || rSize.Height() == 0 )
        return;     // #i43092# rectangle with size 0 would have RECT_EMPTY as end position

    // save old state
    sal_Bool        bHadFill   = pOut->IsFillColor();
    Color       aOldFill   = pOut->GetFillColor();
    sal_Bool        bHadLine   = pOut->IsLineColor();
    Color       aOldLine   = pOut->GetLineColor();
    sal_Bool        bOldEnable = pOut->IsMapModeEnabled();

    Size        aLogPix( 1, 1 );
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
                                       sal_Bool             bState )
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
