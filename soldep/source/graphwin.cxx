/*************************************************************************
 *
 *  $RCSfile: graphwin.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-01 14:18:34 $
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


#include "graphwin.hxx"

GraphWin::GraphWin( Window * pParent )
: ScrollableWindow( pParent, 0L, SCRWIN_DEFAULT | SCRWIN_VCENTER | SCRWIN_HCENTER )
, aBufferWindow( this )
{
    SetSizePixel( pParent->GetSizePixel());
    SetPosPixel( pParent->GetPosPixel());
//  SetPosSizePixel( PIXELS( 0, 0, 500, 500 ) );
    SetTotalSize( Size( 2000,2000 ));
//  SetTotalSize( pParent->GetSizePixel());

    aBufferWindow.SetBackground( Wallpaper( Color( COL_WHITE )) );

//  aBufferWindow.SetPosSizePixel( PIXELS( 0, 0, 238, 80 ) );
    aBufferWindow.SetPosSizePixel( PIXELS( 0, 0, 2000, 2000 ) );
    aBufferWindow.Show();

    Show();
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

