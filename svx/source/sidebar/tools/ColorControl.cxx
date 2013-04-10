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

#include <svx/sidebar/ColorControl.hxx>
#include "svx/svxids.hrc"
#include "svx/drawitem.hxx"
#include "svx/xtable.hxx"
#include "svx/dialmgr.hxx"
#include "svx/xflclit.hxx"
#include <tools/resid.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/floatwin.hxx>
#include <unotools/pathoptions.hxx>
#include <editeng/editrids.hrc>

using ::sfx2::sidebar::Theme;

namespace svx { namespace sidebar {

namespace {
    short GetItemId_Imp( ValueSet& rValueSet, const Color& rCol )
    {
        if(rCol == COL_AUTO)
            return 0;

        bool    bFound = false;
        sal_uInt16  nCount = rValueSet.GetItemCount();
        sal_uInt16  n      = 1;

        while ( !bFound && n <= nCount )
        {
            Color aValCol = rValueSet.GetItemColor(n);

            bFound = (   aValCol.GetRed()   == rCol.GetRed()
                && aValCol.GetGreen() == rCol.GetGreen()
                && aValCol.GetBlue()  == rCol.GetBlue() );

            if ( !bFound )
                n++;
        }
        return bFound ? n : -1;
    }
    class JustReleaseDeleter {public:
            void operator() (XColorList*) const {/* release but don't delete pointer */}
    };
    XColorListRef GetColorTable (void)
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        DBG_ASSERT(pDocSh!=NULL, "DocShell not found!");
        if (pDocSh != NULL)
        {
            const SfxPoolItem* pItem = pDocSh->GetItem(SID_COLOR_TABLE);
            if (pItem != NULL)
            {
                XColorListRef xTable = ((SvxColorListItem*)pItem)->GetColorList();
                if (xTable.is())
                    return xTable;
            }
        }

        return XColorList::GetStdColorList();
    }
} // end of anonymous namespace




ColorControl::ColorControl (
    Window* pParent,
    SfxBindings* pBindings,
    const ResId& rControlResId,
    const ResId& rValueSetResId,
    const ::boost::function<Color(void)>& rNoColorGetter,
    const ::boost::function<void(String&,Color)>& rColorSetter,
    FloatingWindow* pFloatingWindow,
    const ResId* pNoColorStringResId) // const sal_uInt32 nNoColorStringResId)
    : PopupControl(pParent, rControlResId),
      mpBindings(pBindings),
      maVSColor(this, rValueSetResId),
      mpFloatingWindow(pFloatingWindow),
      msNoColorString(
          pNoColorStringResId
              ? String(*pNoColorStringResId)
              : String()),
      maNoColorGetter(rNoColorGetter),
      maColorSetter(rColorSetter)
{
    FreeResource();
}



ColorControl::~ColorControl (void)
{
}




void ColorControl::FillColors (void)
{
    XColorListRef xColorTable (GetColorTable());

    if (xColorTable.is())
    {
        const long nColorCount (xColorTable->Count());
        if (nColorCount <= 0)
            return;
        const sal_Int32 nColumnCount (ceil(sqrt(double(nColorCount))));
        const sal_Int32 nRowCount (ceil(double(nColorCount)/nColumnCount));
        maVSColor.SetColCount(nColumnCount);
        maVSColor.SetLineCount(nRowCount);
        const sal_Int32 nItemSize (15*12 / ::std::max<sal_Int32>(nColumnCount, nRowCount));
        maVSColor.CalcWindowSizePixel(Size(nItemSize,nItemSize));
        Link aLink =  LINK(this, ColorControl, VSSelectHdl);
        maVSColor.SetSelectHdl(aLink);
        maVSColor.SetStyle(
            (maVSColor.GetStyle()
                | WB_TABSTOP | WB_ITEMBORDER | WB_NAMEFIELD// | WB_FLATVALUESET
                | WB_NO_DIRECTSELECT | WB_MENUSTYLEVALUESET | WB_NO_DIRECTSELECT)
                        & ~WB_VSCROLL);

        if (msNoColorString.Len() > 0)
        {
            maVSColor.SetStyle(maVSColor.GetStyle() | WB_NONEFIELD);
            maVSColor.SetText(msNoColorString);
        }

        // Now, after all calls to SetStyle, we can change the
        // background color.
        maVSColor.SetBackground(Theme::GetWallpaper(Theme::Paint_DropDownBackground));

        maVSColor.Clear();
        XColorEntry* pEntry = NULL;
        for (sal_Int32 nIndex=0; nIndex<nColorCount; ++nIndex)
        {
            pEntry = xColorTable->GetColor(nIndex);
            maVSColor.InsertItem(nIndex+1, pEntry->GetColor(), pEntry->GetName() );
        }
    }

    maVSColor.Show();
}




void ColorControl::GetFocus (void)
{
    maVSColor.GrabFocus();
}




void ColorControl::SetCurColorSelect (Color aCol, bool bAvailable)
{
    FillColors();
    short nCol = GetItemId_Imp( maVSColor, aCol );
    if(! bAvailable)
    {
        maVSColor.SetNoSelection();
        return;
    }

    //if not found
    if( nCol == -1)
    {
        maVSColor.SetNoSelection();
    }
    else
    {
        maVSColor.SelectItem( nCol );
    }
}




IMPL_LINK(ColorControl, VSSelectHdl, void *, pControl)
{
    if(pControl == &maVSColor)
    {
        sal_uInt16 iPos = maVSColor.GetSelectItemId();
        Color aColor = maVSColor.GetItemColor( iPos );
        String aTmpStr = maVSColor.GetItemText( iPos );

        // react when the WB_NONEFIELD created entry is selected
        if (aColor.GetColor() == 0 && aTmpStr.Len() == 0)
        {
            if (maNoColorGetter)
                aColor = maNoColorGetter();
        }
        if (maColorSetter)
            maColorSetter(aTmpStr, aColor);

        if (mpFloatingWindow!=NULL && mpFloatingWindow->IsInPopupMode())
            mpFloatingWindow->EndPopupMode();
    }

    return 0;
}


} } // end of namespace svx::sidebar
