/*************************************************************************
 *
 *  $RCSfile: cbuttonw.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <tools/svwin.h>
#include <vcl/window.hxx>
#include <vcl/decoview.hxx>

#include "cbutton.hxx"


//========================================================================
//  class ScDDComboBoxButton
//========================================================================

ScDDComboBoxButton::ScDDComboBoxButton( Window* pOutputDevice )
    :   pOut( pOutputDevice )
{
    SetOptSizePixel();
}

// -------------------------------------------------------------------------

__EXPORT ScDDComboBoxButton::~ScDDComboBoxButton()
{
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
                               const Color& rArrowCol,
                               BOOL         bBtnIn  /* = FALSE */ )
{
    // save old state
    BOOL        bHadFill   = pOut->IsFillColor();
    Color       aOldFill   = pOut->GetFillColor();
    BOOL        bHadLine   = pOut->IsLineColor();
    Color       aOldLine   = pOut->GetLineColor();
    BOOL        bOldEnable = pOut->IsMapModeEnabled();

    Size        aLogPix( 1, 1 );
    Rectangle   aBtnRect( rAt, rSize );
    Rectangle   aInnerRect = aBtnRect;

    pOut->EnableMapMode( FALSE );

    DecorationView aDecoView( pOut);

    USHORT nButtonStyle = BUTTON_DRAW_DEFAULT;
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

    ImpDrawArrow( aInnerRect, rArrowCol );


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
                                       const Color&     rColor )
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

    pOut->SetFillColor( rColor );
    pOut->SetLineColor( rColor );

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





