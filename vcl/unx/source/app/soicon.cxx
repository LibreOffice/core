/*************************************************************************
 *
 *  $RCSfile: soicon.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $ $Date: 2005-03-03 19:58:00 $
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

