/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "unx/pixmap.hxx"

X11Pixmap::X11Pixmap()
: mpDisplay( NULL )
, mnScreen( 0 )
, mpPixmap( 0 )
, mbDeletePixmap( false )
, mnWidth( -1 )
, mnHeight( -1 )
, mnDepth( 0 )
{
}

X11Pixmap::X11Pixmap( Display* pDisplay, SalX11Screen nScreen, int nWidth, int nHeight, int nDepth )
: mpDisplay( pDisplay )
, mnScreen( nScreen )
, mbDeletePixmap( true )
, mnWidth( nWidth )
, mnHeight( nHeight )
, mnDepth( nDepth )
{
    Window root = RootWindow( pDisplay, 0 );
    mpPixmap = XCreatePixmap( pDisplay, root, nWidth, nHeight, nDepth );
}

X11Pixmap::X11Pixmap( X11Pixmap& rOther )
: mpDisplay( rOther.mpDisplay )
, mnScreen( rOther.mnScreen )
, mbDeletePixmap( rOther.mbDeletePixmap )
, mnWidth( rOther.mnWidth )
, mnHeight( rOther.mnHeight )
, mnDepth( rOther.mnDepth )
{
    mpPixmap = rOther.mpPixmap;
    rOther.mpPixmap = 0;
    rOther.mbDeletePixmap = false;
}

X11Pixmap::~X11Pixmap()
{
    if (mbDeletePixmap && mpPixmap)
        XFreePixmap( mpDisplay, mpPixmap );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
