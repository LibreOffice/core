/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TitleToolBox.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_sd.hxx"

#include "TitleToolBox.hxx"

#include "res_bmp.hrc"
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include "sdresid.hxx"
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
                GetSettings().GetStyleSettings().GetHighContrastMode()
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
