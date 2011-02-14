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

// System - Includes -----------------------------------------------------



// INCLUDE ---------------------------------------------------------------

#include "hintwin.hxx"
#include "global.hxx"

#define HINT_LINESPACE  2
#define HINT_INDENT     3
#define HINT_MARGIN     4

//==================================================================

ScHintWindow::ScHintWindow( Window* pParent, const String& rTit, const String& rMsg ) :
    Window( pParent, WinBits( WB_BORDER ) ),
    aTitle( rTit ),
    aMessage( rMsg )
{
    aMessage.ConvertLineEnd( LINEEND_CR );

    //  Hellgelb, wie Notizen in detfunc.cxx
    Color aYellow( 255,255,192 );           // hellgelb
    SetBackground( aYellow );

    aTextFont = GetFont();
    aTextFont.SetTransparent( sal_True );
    aTextFont.SetWeight( WEIGHT_NORMAL );
    aHeadFont = aTextFont;
    aHeadFont.SetWeight( WEIGHT_BOLD );

    SetFont( aHeadFont );
    Size aHeadSize( GetTextWidth( aTitle ), GetTextHeight() );
    SetFont( aTextFont );

    Size aTextSize;
    xub_StrLen nIndex = 0;
    while ( nIndex != STRING_NOTFOUND )
    {
        String aLine = aMessage.GetToken( 0, CHAR_CR, nIndex );
        Size aLineSize( GetTextWidth( aLine ), GetTextHeight() );
        nTextHeight = aLineSize.Height();
        aTextSize.Height() += nTextHeight;
        if ( aLineSize.Width() > aTextSize.Width() )
            aTextSize.Width() = aLineSize.Width();
    }
    aTextSize.Width() += HINT_INDENT;

    aTextStart = Point( HINT_MARGIN + HINT_INDENT,
                        aHeadSize.Height() + HINT_MARGIN + HINT_LINESPACE );

    Size aWinSize( Max( aHeadSize.Width(), aTextSize.Width() ) + 2 * HINT_MARGIN + 1,
                    aHeadSize.Height() + aTextSize.Height() + HINT_LINESPACE + 2 * HINT_MARGIN + 1 );
    SetOutputSizePixel( aWinSize );
}


ScHintWindow::~ScHintWindow()
{
}


void __EXPORT ScHintWindow::Paint( const Rectangle& /* rRect */ )
{
    SetFont( aHeadFont );
    DrawText( Point(HINT_MARGIN,HINT_MARGIN), aTitle );

    SetFont( aTextFont );
    xub_StrLen nIndex = 0;
    Point aLineStart = aTextStart;
    while ( nIndex != STRING_NOTFOUND )
    {
        String aLine = aMessage.GetToken( 0, CHAR_CR, nIndex );
        DrawText( aLineStart, aLine );
        aLineStart.Y() += nTextHeight;
    }
}
