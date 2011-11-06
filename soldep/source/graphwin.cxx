/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
