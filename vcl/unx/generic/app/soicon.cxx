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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <unx/salunx.h>
#include <unx/saldisp.hxx>
#include <unx/salbmp.h>
#include <unx/soicon.hxx>

#include <vcl/salbtype.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/graph.hxx>

#include <svdata.hxx>
#include <svids.hrc>
#include <salbmp.hxx>
#include <impbmp.hxx>


sal_Bool SelectAppIconPixmap( SalDisplay *pDisplay, int nScreen,sal_uInt16 nIcon, sal_uInt16 iconSize,
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

    return sal_True;
}

