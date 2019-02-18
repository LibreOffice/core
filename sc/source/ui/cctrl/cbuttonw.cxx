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

#include <comphelper/lok.hxx>
#include <vcl/outdev.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <cbutton.hxx>

//  class ScDDComboBoxButton

ScDDComboBoxButton::ScDDComboBoxButton( OutputDevice* pOutputDevice )
    :   pOut( pOutputDevice )
{
    SetOptSizePixel();
}

ScDDComboBoxButton::~ScDDComboBoxButton()
{
}

void ScDDComboBoxButton::SetOutputDevice( OutputDevice* pOutputDevice )
{
    pOut = pOutputDevice;
}

void ScDDComboBoxButton::SetOptSizePixel()
{
    aBtnSize = pOut->LogicToPixel(Size(8, 11), MapMode(MapUnit::MapAppFont));
    aBtnSize.setWidth( std::max(aBtnSize.Width(), pOut->GetSettings().GetStyleSettings().GetScrollBarSize()) );
}

void ScDDComboBoxButton::Draw( const Point& rAt,
                               const Size&  rSize )
{
    if ( rSize.Width() == 0 || rSize.Height() == 0 )
        return;

    // save old state
    bool        bHadFill   = pOut->IsFillColor();
    Color       aOldFill   = pOut->GetFillColor();
    bool        bHadLine   = pOut->IsLineColor();
    Color       aOldLine   = pOut->GetLineColor();
    bool        bOldEnable = pOut->IsMapModeEnabled();

    tools::Rectangle   aBtnRect( rAt, rSize );

    if (!comphelper::LibreOfficeKit::isActive())
        pOut->EnableMapMode(false);

    DecorationView aDecoView( pOut);

    tools::Rectangle aInnerRect=aDecoView.DrawButton( aBtnRect, DrawButtonFlags::Default );

    aInnerRect.AdjustLeft(1 );
    aInnerRect.AdjustTop(1 );
    aInnerRect.AdjustRight( -1 );
    aInnerRect.AdjustBottom( -1 );

    Size  aInnerSize   = aInnerRect.GetSize();
    Point aInnerCenter = aInnerRect.Center();

    aInnerRect.SetTop( aInnerCenter.Y() - (aInnerSize.Width()>>1) );
    aInnerRect.SetBottom( aInnerCenter.Y() + (aInnerSize.Width()>>1) );

    ImpDrawArrow( aInnerRect );

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

void ScDDComboBoxButton::ImpDrawArrow( const tools::Rectangle& rRect )
{
    // no need to save old line and fill color here (is restored after the call)

    tools::Rectangle   aPixRect = rRect;
    Point       aCenter  = aPixRect.Center();
    Size        aSize    = aPixRect.GetSize();

    Size aSize3;
    aSize3.setWidth( aSize.Width() >> 1 );
    aSize3.setHeight( aSize.Height() >> 1 );

    Size aSize4;
    aSize4.setWidth( aSize.Width() >> 2 );
    aSize4.setHeight( aSize.Height() >> 2 );

    tools::Rectangle aTempRect = aPixRect;

    const StyleSettings& rSett = Application::GetSettings().GetStyleSettings();
    Color aColor( rSett.GetButtonTextColor() );
    pOut->SetFillColor( aColor );
    pOut->SetLineColor( aColor );

    aTempRect.SetLeft( aCenter.X() - aSize4.Width() );
    aTempRect.SetRight( aCenter.X() + aSize4.Width() );
    aTempRect.SetTop( aCenter.Y() - aSize3.Height() );
    aTempRect.SetBottom( aCenter.Y() - 1 );

    pOut->DrawRect( aTempRect );

    Point aPos1( aCenter.X()-aSize3.Width(), aCenter.Y() );
    Point aPos2( aCenter.X()+aSize3.Width(), aCenter.Y() );
    while( aPos1.X() <= aPos2.X() )
    {
        pOut->DrawLine( aPos1, aPos2 );
        aPos1.AdjustX( 1 ); aPos2.AdjustX( -1 );
        aPos1.AdjustY( 1 ); aPos2.AdjustY( 1 );
    }

    pOut->DrawLine( Point( aCenter.X() - aSize3.Width(), aPos1.Y()+1 ),
                    Point( aCenter.X() + aSize3.Width(), aPos1.Y()+1 ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
