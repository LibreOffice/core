/*************************************************************************
 *
 *  $RCSfile: hintwin.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mh $ $Date: 2001-10-23 11:13:01 $
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

// System - Includes -----------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif


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
    aTextFont.SetTransparent( TRUE );
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


void __EXPORT ScHintWindow::Paint( const Rectangle& rRect )
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
