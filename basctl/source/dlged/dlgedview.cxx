/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedview.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:15:45 $
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


#ifndef _BASCTL_DLGEDVIEW_HXX
#include "dlgedview.hxx"
#endif

#ifndef _BASCTL_DLGED_HXX
#include "dlged.hxx"
#endif

#ifndef _BASCTL_DLGEDPAGE_HXX
#include <dlgedpage.hxx>
#endif

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#include <basidesh.hxx>
#include <iderdll.hxx>

TYPEINIT1( DlgEdView, SdrView );

//----------------------------------------------------------------------------

DlgEdView::DlgEdView( SdrModel* pModel, OutputDevice* pOut, DlgEditor* pEditor )
    :SdrView( pModel, pOut )
    ,pDlgEditor( pEditor )
{
}

//----------------------------------------------------------------------------

DlgEdView::~DlgEdView()
{
}

//----------------------------------------------------------------------------

void DlgEdView::MarkListHasChanged()
{
    SdrView::MarkListHasChanged();

    DlgEdHint aHint( DLGED_HINT_SELECTIONCHANGED );
    if ( pDlgEditor )
    {
        pDlgEditor->Broadcast( aHint );
        pDlgEditor->UpdatePropertyBrowserDelayed();
    }
}

//----------------------------------------------------------------------------

void DlgEdView::MakeVisible( const Rectangle& rRect, Window& rWin )
{
    // visible area
    MapMode aMap( rWin.GetMapMode() );
    Point aOrg( aMap.GetOrigin() );
    Size aVisSize( rWin.GetOutputSize() );
    Rectangle RectTmp( Point(-aOrg.X(),-aOrg.Y()), aVisSize );
    Rectangle aVisRect( RectTmp );

    // check, if rectangle is inside visible area
    if ( !aVisRect.IsInside( rRect ) )
    {
        // calculate scroll distance; the rectangle must be inside the visible area
        sal_Int32 nScrollX = 0, nScrollY = 0;

        sal_Int32 nVisLeft   = aVisRect.Left();
        sal_Int32 nVisRight  = aVisRect.Right();
        sal_Int32 nVisTop    = aVisRect.Top();
        sal_Int32 nVisBottom = aVisRect.Bottom();

        sal_Int32 nDeltaX = pDlgEditor->GetHScroll()->GetLineSize();
        sal_Int32 nDeltaY = pDlgEditor->GetVScroll()->GetLineSize();

        while ( rRect.Right() > nVisRight + nScrollX )
            nScrollX += nDeltaX;

        while ( rRect.Left() < nVisLeft + nScrollX )
            nScrollX -= nDeltaX;

        while ( rRect.Bottom() > nVisBottom + nScrollY )
            nScrollY += nDeltaY;

        while ( rRect.Top() < nVisTop + nScrollY )
            nScrollY -= nDeltaY;

        // don't scroll beyond the page size
        Size aPageSize = pDlgEditor->GetPage()->GetSize();
        sal_Int32 nPageWidth  = aPageSize.Width();
        sal_Int32 nPageHeight = aPageSize.Height();

        if ( nVisRight + nScrollX > nPageWidth )
            nScrollX = nPageWidth - nVisRight;

        if ( nVisLeft + nScrollX < 0 )
            nScrollX = -nVisLeft;

        if ( nVisBottom + nScrollY > nPageHeight )
            nScrollY = nPageHeight - nVisBottom;

        if ( nVisTop + nScrollY < 0 )
            nScrollY = -nVisTop;

        // scroll window
        rWin.Update();
        rWin.Scroll( -nScrollX, -nScrollY );
        aMap.SetOrigin( Point( aOrg.X() - nScrollX, aOrg.Y() - nScrollY ) );
        rWin.SetMapMode( aMap );
        rWin.Update();
        rWin.Invalidate();

        // update scroll bars
        if ( pDlgEditor )
            pDlgEditor->UpdateScrollBars();

        DlgEdHint aHint( DLGED_HINT_WINDOWSCROLLED );
        if ( pDlgEditor )
            pDlgEditor->Broadcast( aHint );
    }
}

//----------------------------------------------------------------------------

