/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: soicon.cxx,v $
 * $Revision: 1.25 $
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
#include "precompiled_vcl.hxx"

#include <salunx.h>
#include <saldisp.hxx>
#include <vcl/salbmp.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/impbmp.hxx>
#include <vcl/bitmap.hxx>
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmapex.hxx>
#endif
#include <vcl/graph.hxx>
#include <soicon.hxx>
#include <vcl/svdata.hxx>
#include <salbmp.h>
#include <vcl/svids.hrc>

BOOL SelectAppIconPixmap( SalDisplay *pDisplay, int nScreen,USHORT nIcon, USHORT iconSize,
                          Pixmap& icon_pixmap, Pixmap& icon_mask)
{
    if( ! ImplGetResMgr() )
        return FALSE;

    USHORT nIconSizeOffset;

    if( iconSize >= 48 )
        nIconSizeOffset = SV_ICON_SIZE48_START;
    else if( iconSize >= 32 )
        nIconSizeOffset = SV_ICON_SIZE32_START;
    else if( iconSize >= 16 )
        nIconSizeOffset = SV_ICON_SIZE16_START;
    else
        return FALSE;

    BitmapEx aIcon( ResId(nIconSizeOffset + nIcon, *ImplGetResMgr()));
    if( TRUE == aIcon.IsEmpty() )
        return FALSE;

    SalTwoRect aRect;
    aRect.mnSrcX = 0; aRect.mnSrcY = 0;
    aRect.mnSrcWidth = iconSize; aRect.mnSrcHeight = iconSize;
    aRect.mnDestX = 0; aRect.mnDestY = 0;
    aRect.mnDestWidth = iconSize; aRect.mnDestHeight = iconSize;

    X11SalBitmap *pBitmap = static_cast < X11SalBitmap * >
        (aIcon.ImplGetBitmapImpBitmap()->ImplGetSalBitmap());

    icon_pixmap = XCreatePixmap( pDisplay->GetDisplay(),
                                 pDisplay->GetRootWindow( nScreen ),
                                 iconSize, iconSize,
                                 DefaultDepth( pDisplay->GetDisplay(), nScreen )
                                 );

    pBitmap->ImplDraw( icon_pixmap,
                       nScreen,
                       DefaultDepth( pDisplay->GetDisplay(), nScreen ),
                       aRect,
                       DefaultGC(pDisplay->GetDisplay(), nScreen ) );

    icon_mask = None;

    if( TRANSPARENT_BITMAP == aIcon.GetTransparentType() )
    {
        icon_mask = XCreatePixmap( pDisplay->GetDisplay(),
                                   pDisplay->GetRootWindow( pDisplay->GetDefaultScreenNumber() ),
                                   iconSize, iconSize, 1);

        XGCValues aValues;
        aValues.foreground = 0xffffffff;
        aValues.background = 0;
        aValues.function = GXcopy;
        GC aMonoGC = XCreateGC( pDisplay->GetDisplay(), icon_mask,
            GCFunction|GCForeground|GCBackground, &aValues );

        Bitmap aMask = aIcon.GetMask();
        aMask.Invert();

        X11SalBitmap *pMask = static_cast < X11SalBitmap * >
            (aMask.ImplGetImpBitmap()->ImplGetSalBitmap());

        pMask->ImplDraw(icon_mask, nScreen, 1, aRect, aMonoGC);
        XFreeGC( pDisplay->GetDisplay(), aMonoGC );
    }

    return TRUE;
}

