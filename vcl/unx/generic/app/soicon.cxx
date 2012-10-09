/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <unx/salunx.h>
#include <unx/saldisp.hxx>
#include <unx/salbmp.h>
#include <unx/soicon.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/graph.hxx>

#include <svdata.hxx>
#include <svids.hrc>
#include <salbmp.hxx>
#include <impbmp.hxx>


sal_Bool SelectAppIconPixmap( SalDisplay *pDisplay, SalX11Screen nXScreen,
                              sal_uInt16 nIcon, sal_uInt16 iconSize,
                              Pixmap& icon_pixmap, Pixmap& icon_mask)
{
    if( ! ImplGetResMgr() )
        return sal_False;

    sal_uInt16 nIconSizeOffset;

    if( iconSize >= 48 )
        nIconSizeOffset = SV_ICON_SIZE48_START;
    else if( iconSize >= 32 )
        nIconSizeOffset = SV_ICON_SIZE32_START;
    else if( iconSize >= 16 )
        nIconSizeOffset = SV_ICON_SIZE16_START;
    else
        return sal_False;

    BitmapEx aIcon( ResId(nIconSizeOffset + nIcon, *ImplGetResMgr()));
    if( sal_True == aIcon.IsEmpty() )
        return sal_False;

    SalTwoRect aRect;
    aRect.mnSrcX = 0; aRect.mnSrcY = 0;
    aRect.mnSrcWidth = iconSize; aRect.mnSrcHeight = iconSize;
    aRect.mnDestX = 0; aRect.mnDestY = 0;
    aRect.mnDestWidth = iconSize; aRect.mnDestHeight = iconSize;

    X11SalBitmap *pBitmap = static_cast < X11SalBitmap * >
        (aIcon.ImplGetBitmapImpBitmap()->ImplGetSalBitmap());

    icon_pixmap = XCreatePixmap( pDisplay->GetDisplay(),
                                 pDisplay->GetRootWindow( nXScreen ),
                                 iconSize, iconSize,
                                 DefaultDepth( pDisplay->GetDisplay(),
                                               nXScreen.getXScreen() )
                                 );

    pBitmap->ImplDraw( icon_pixmap,
                       nXScreen,
                       DefaultDepth( pDisplay->GetDisplay(),
                                     nXScreen.getXScreen() ),
                       aRect,
                       DefaultGC( pDisplay->GetDisplay(),
                                  nXScreen.getXScreen() ) );

    icon_mask = None;

    if( TRANSPARENT_BITMAP == aIcon.GetTransparentType() )
    {
        icon_mask = XCreatePixmap( pDisplay->GetDisplay(),
                                   pDisplay->GetRootWindow( pDisplay->GetDefaultXScreen() ),
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

        pMask->ImplDraw(icon_mask, nXScreen, 1, aRect, aMonoGC);
        XFreeGC( pDisplay->GetDisplay(), aMonoGC );
    }

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
