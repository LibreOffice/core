/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: graphwin.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-04-20 15:15:01 $
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

#include "graphwin.hxx"
//#include "depapp.hxx"
#include "soldep.hxx"


GraphWin::GraphWin( Window * pParent, void * pClass )
: ScrollableWindow( pParent, 0L, SCRWIN_DEFAULT | SCRWIN_VCENTER | SCRWIN_HCENTER )
, aBufferWindow( this )
{
    mpClass = pClass;
    SetSizePixel( pParent->GetSizePixel());
    SetPosPixel( pParent->GetPosPixel());
    SetTotalSize( Size( 2000,2000 ));

    aBufferWindow.SetBackground( Wallpaper( Color( COL_WHITE )));
    aBufferWindow.SetPosSizePixel( PIXELS( 0, 0, 2000, 2000 ) );
    aBufferWindow.Show(); // Content
    Show();               // Window with Scrollbars
}

void GraphWin::EndScroll( long nDeltaX, long nDeltaY )
{
    // get the visible area
    Rectangle aArea( GetVisibleArea());
    long nX = aArea.Right();
    long nY = aArea.Bottom();

    // set the new pos and size by using LogicToPixel (this is mandatory)
    aBufferWindow.SetPosSizePixel( LogicToPixel( Point( 0, 0 )),
        LogicToPixel( Size( nX, nY )));
    aBufferWindow.Invalidate();
}

void GraphWin::Resize()
{
    // get the visible area
    ScrollableWindow::Resize();
    Rectangle aArea( GetVisibleArea());
    long nX = aArea.Right();
    long nY = aArea.Bottom();

    // set the new pos and size by using LogicToPixel (this is mandatory)
    aBufferWindow.SetPosSizePixel( LogicToPixel( Point( 0, 0 )),
        LogicToPixel( Size( nX, nY )));
}

void GraphWin::Command( const CommandEvent& rEvent)
{
    ScrollableWindow::Command( rEvent );
}
