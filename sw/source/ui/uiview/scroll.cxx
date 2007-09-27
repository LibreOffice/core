/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scroll.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:35:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include "swtypes.hxx"
#include "swrect.hxx"
#include "scroll.hxx"

#define SCROLL_LINE_SIZE 250


SwScrollbar::SwScrollbar( Window *pWin, BOOL bHoriz ) :
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
    if(bAuto)
        AutoShow();
}

/*-----------------10/21/97 02:48pm-----------------

--------------------------------------------------*/
void SwScrollbar::ExtendedShow( BOOL bSet )
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
        ExtendedShow();

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
            ExtendedShow(TRUE);
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
