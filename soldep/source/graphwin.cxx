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

#include <soldep/graphwin.hxx>
//#include "depapp.hxx"
#include <soldep/soldep.hxx>


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
