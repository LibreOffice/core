/*************************************************************************
 *
 *  $RCSfile: scroll.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:44:44 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "swtypes.hxx"
#include "swrect.hxx"
#include "scroll.hxx"

#define SCROLL_LINE_SIZE 250


SwScrollbar::SwScrollbar( Window *pWin, int bHoriz ) :
    ScrollBar( pWin,
    WinBits( WB_3DLOOK | WB_HIDE | ( bHoriz ? WB_HSCROLL : WB_VSCROLL)  ) ),
    bHori( bHoriz ),
    bAuto( FALSE ),
    bThumbEnabled( TRUE ),
    bVisible(FALSE),
    bSizeSet(FALSE)
{
    // SSA: --- RTL --- no mirroring for horizontal scrollbars
    if( bHoriz )
        EnableRTL( FALSE );
}


 SwScrollbar::~SwScrollbar() {}

/*------------------------------------------------------------------------
 Beschreibung:  wird nach einer Aenderung der Dokumentgroesse gerufen, um den
                Range des Scrollbars neu einzustellen.
------------------------------------------------------------------------*/

void SwScrollbar::DocSzChgd( const Size &rSize )
{
    aDocSz = rSize;
    SetRange( Range( 0, bHori ? rSize.Width() : rSize.Height()) );
    const ULONG nVisSize = GetVisibleSize();
    SetLineSize( SCROLL_LINE_SIZE );
//    SetLineSize( nVisSize * 10 / 100 );
    SetPageSize( nVisSize * 77 / 100 );
}

/*------------------------------------------------------------------------
 Beschreibung:  wird nach einer Veraenderung des sichtbaren Ausschnittes
                gerufen.
------------------------------------------------------------------------*/


void SwScrollbar::ViewPortChgd( const Rectangle &rRect )
{
    long nThumb, nVisible;
    if( bHori )
    {
        nThumb = rRect.Left();
        nVisible = rRect.GetWidth();
    }
    else
    {
        nThumb = rRect.Top();
        nVisible = rRect.GetHeight();
    }

    SetVisibleSize( nVisible );
    DocSzChgd(aDocSz);
    if ( bThumbEnabled )
        SetThumbPos( nThumb );
    long nVis = GetVisibleSize();
    long nLen = GetRange().Len();
    if(bAuto)
        AutoShow();
}

void SwScrollbar::EnableThumbPos( BOOL bEnable, const SwRect &rVisArea )
{
    bThumbEnabled = bEnable;
    if ( bEnable )
    {
        long nThumb = bHori ? rVisArea.Left() : rVisArea.Top();
        SetThumbPos( nThumb );
    }
}

/*-----------------10/21/97 02:48pm-----------------

--------------------------------------------------*/
void SwScrollbar::Show( BOOL bSet )
{
    bVisible = bSet;
    if( (!bSet ||  !bAuto) && IsUpdateMode() && bSizeSet)
        ScrollBar::Show(bSet);
}

/*-----------------10/21/97 03:23pm-----------------

--------------------------------------------------*/
void SwScrollbar::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    ScrollBar::SetPosSizePixel(rNewPos, rNewSize);
    bSizeSet = TRUE;
    if(bVisible)
        Show();

}

/*-----------------10/21/97 04:47pm-----------------

--------------------------------------------------*/
void SwScrollbar::SetUpdateMode( BOOL bUpdate )
{
    ScrollBar::SetUpdateMode( bUpdate );
    if(bUpdate &&
        bVisible != ScrollBar::IsVisible())
    {
        if(bAuto)
            AutoShow();
        else
            ScrollBar::Show(bVisible);
    }
}
/*-----------------14.04.98 11:38-------------------

--------------------------------------------------*/
void SwScrollbar::SetAuto(BOOL bSet)
{
    if(bAuto != bSet)
    {
        bAuto = bSet;

        // automatisch versteckt - dann anzeigen
        if(!bAuto && bVisible && !ScrollBar::IsVisible())
            Show(TRUE);
        else if(bAuto)
            AutoShow(); // oder automatisch verstecken
    }
}
/*-----------------14.04.98 11:43-------------------

--------------------------------------------------*/
void SwScrollbar::AutoShow()
{
    long nVis = GetVisibleSize();
    long nLen = GetRange().Len();
    {
        if( nVis >= nLen - 1)
        {
            if(ScrollBar::IsVisible())
                ScrollBar::Show(FALSE);
        }
        else if ( !ScrollBar::IsVisible() &&
                  (!bHori || nVis) )        //Optimierung fuer Browser.
                                            //Horizontaler Scrollbar per
                                            //default aus.
        {
            ScrollBar::Show(TRUE);
        }
    }
}
