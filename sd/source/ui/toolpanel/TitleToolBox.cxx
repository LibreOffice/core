/*************************************************************************
 *
 *  $RCSfile: TitleToolBox.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:39:47 $
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
