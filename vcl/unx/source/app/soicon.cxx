/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: soicon.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 15:01:23 $
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

#include <salunx.h>

#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_IMPBMP_HXX
#include <impbmp.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmapex.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <graph.hxx>
#endif
#ifndef _SV_SOICON_HXX
#include <soicon.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SALBMP_H
#include <salbmp.h>
#endif
#include <svids.hrc>


BOOL SelectAppIconPixmap( SalDisplay *pDisplay, USHORT nIcon, USHORT iconSize,
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

    BitmapEx aIcon( ResId(nIconSizeOffset + nIcon, ImplGetResMgr()));
    if( TRUE == aIcon.IsEmpty() )
        return FALSE;

    SalTwoRect aRect;
    aRect.mnSrcX = 0; aRect.mnSrcY = 0;
    aRect.mnSrcWidth = iconSize; aRect.mnSrcHeight = iconSize;
    aRect.mnDestX = 0; aRect.mnDestY = 0;
    aRect.mnDestWidth = iconSize; aRect.mnDestHeight = iconSize;

    X11SalBitmap *pBitmap = static_cast < X11SalBitmap * >
        (aIcon.ImplGetBitmapImpBitmap()->ImplGetSalBitmap());

    icon_pixmap = XCreatePixmap( pDisplay->GetDisplay(), pDisplay->GetRootWindow(),
        iconSize, iconSize, pDisplay->GetRootVisual()->GetDepth());

    pBitmap->ImplDraw(icon_pixmap, pDisplay->GetRootVisual()->GetDepth(),
        aRect, DefaultGC(pDisplay->GetDisplay(), pDisplay->GetScreenNumber()), false);

    icon_mask = None;

    if( TRANSPARENT_BITMAP == aIcon.GetTransparentType() )
    {
        icon_mask = XCreatePixmap( pDisplay->GetDisplay(),
            pDisplay->GetRootWindow(), iconSize, iconSize, 1);

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

        pMask->ImplDraw(icon_mask, 1, aRect, aMonoGC, false);
        XFreeGC( pDisplay->GetDisplay(), aMonoGC );
    }

    return TRUE;
}

