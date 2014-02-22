/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
} 




ColorControl::ColorControl (
    Window* pParent,
    SfxBindings* /* pBindings */,
    const ResId& rControlResId,
    const ResId& rValueSetResId,
    const ::boost::function<Color(void)>& rNoColorGetter,
    const ::boost::function<void(OUString&,Color)>& rColorSetter,
    FloatingWindow* pFloatingWindow,
    const ResId* pNoColorStringResId) 
    : PopupControl(pParent, rControlResId),
      maVSColor(this, rValueSetResId),
      mpFloatingWindow(pFloatingWindow),
      msNoColorString(
          pNoColorStringResId
              ? pNoColorStringResId->toString()
              : OUString()),
      maNoColorGetter(rNoColorGetter),
      maColorSetter(rColorSetter)
{
    FreeResource();
    FillColors();
}



ColorControl::~ColorControl (void)
{
}




void ColorControl::FillColors (void)
{
    XColorListRef xColorTable (GetColorTable());

    if (xColorTable.is())
    {
        const long nColorCount(xColorTable->Count());
        if (nColorCount <= 0)
            return;

        const WinBits aWinBits(maVSColor.GetStyle() | WB_TABSTOP | WB_ITEMBORDER | WB_NAMEFIELD |
            WB_NO_DIRECTSELECT | WB_MENUSTYLEVALUESET);

        maVSColor.SetStyle(aWinBits);

        
        if(!msNoColorString.isEmpty())
        {
            maVSColor.SetStyle(maVSColor.GetStyle() | WB_NONEFIELD);
            maVSColor.SetText(msNoColorString);
        }

        const Size aNewSize(maVSColor.layoutAllVisible(nColorCount));
        maVSColor.SetOutputSizePixel(aNewSize);
        static sal_Int32 nAdd = 4;

        SetOutputSizePixel(Size(aNewSize.Width() + nAdd, aNewSize.Height() + nAdd));
        Link aLink = LINK(this, ColorControl, VSSelectHdl);
        maVSColor.SetSelectHdl(aLink);

        
        
        maVSColor.SetBackground(Theme::GetWallpaper(Theme::Paint_DropDownBackground));

        
        maVSColor.Clear();
        maVSColor.addEntriesForXColorList(*xColorTable);
    }

    maVSColor.Show();
}




void ColorControl::GetFocus (void)
{
    maVSColor.GrabFocus();
}




void ColorControl::SetCurColorSelect (Color aCol, bool bAvailable)
{

    short nCol = GetItemId_Imp( maVSColor, aCol );
    if(! bAvailable)
    {
        maVSColor.SetNoSelection();
        return;
    }

    
    if( nCol == -1)
    {
        maVSColor.SetNoSelection();
    }
    else
    {
        
        maVSColor.SetNoSelection();
        maVSColor.SelectItem(nCol);
    }
}




IMPL_LINK(ColorControl, VSSelectHdl, void *, pControl)
{
    if(pControl == &maVSColor)
    {
        sal_uInt16 iPos = maVSColor.GetSelectItemId();
        Color aColor = maVSColor.GetItemColor( iPos );
        OUString aTmpStr = maVSColor.GetItemText( iPos );

        
        if (aColor.GetColor() == 0 && aTmpStr.isEmpty())
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


} } 
