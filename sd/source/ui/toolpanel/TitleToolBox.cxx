/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TitleToolBox.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:16:06 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "TitleToolBox.hxx"

#include "res_bmp.hrc"

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _SDRESID_HXX
#include "sdresid.hxx"
#endif
#include <rtl/ustring.hxx>

namespace sd { namespace toolpanel {


TitleToolBox::TitleToolBox (::Window* pParent, WinBits nStyle )
    : ToolBox( pParent, nStyle )
{
    lastSize = -1;

    Bitmap aBitmap (SdResId (BMP_CLOSE_DOC));
    Bitmap aBitmapHC (SdResId (BMP_CLOSE_DOC_H));
    Bitmap aTriangleRight (SdResId (BMP_TRIANGLE_RIGHT));
    Bitmap aTriangleDown (SdResId (BMP_TRIANGLE_DOWN));

    maImage = Image (aBitmap, Color (COL_LIGHTMAGENTA));
    maImageHC = Image (aBitmapHC, Color (BMP_COLOR_HIGHCONTRAST));
    maTriangleRight = Image (aTriangleRight, Color (COL_LIGHTMAGENTA));
    maTriangleDown = Image (aTriangleDown, Color (COL_LIGHTMAGENTA));

    SetOutStyle (TOOLBOX_STYLE_FLAT);
    SetBackground (Wallpaper (
        GetSettings().GetStyleSettings().GetDialogColor()));
}




void TitleToolBox::AddItem (ToolBoxId aId)
{
    switch (aId)
    {
        case TBID_PANEL_MENU:
            InsertItem (TBID_PANEL_MENU,
                String::CreateFromAscii ("Panel"),
                TIB_DROPDOWN);
            break;

        case TBID_DOCUMENT_CLOSE:
            InsertItem (TBID_DOCUMENT_CLOSE,
                GetSettings().GetStyleSettings().GetMenuBarColor().IsDark()
                ? maImageHC : maImage, 0 );
            break;

        case TBID_TRIANGLE_RIGHT:
            InsertItem (TBID_TRIANGLE_RIGHT,
                maTriangleRight, 0);
            break;

        case TBID_TRIANGLE_DOWN:
            InsertItem (TBID_TRIANGLE_DOWN,
                maTriangleDown, 0);
            break;

        case TBID_TEST:
            InsertItem (TBID_TEST,
                String::CreateFromAscii ("Test"),
                TIB_DROPDOWN);
            break;
    }
}




void TitleToolBox::DataChanged (const DataChangedEvent& rDCEvt)
{
    Window::DataChanged (rDCEvt);

    if (rDCEvt.GetFlags() & SETTINGS_STYLE)
    {
        SetBackground (
            Wallpaper (GetSettings().GetStyleSettings().GetDialogColor()));
    }
}




} } // end of namespace ::sd::toolbox
