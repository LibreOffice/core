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
#include "precompiled_svx.hxx"

#include "ColorPanel.hxx"

#include <vcl/image.hxx>
#include <vcl/svapp.hxx>

namespace svx { namespace sidebar {

ColorPanel::ColorPanel (::Window* pParent)
    : ValueSet(pParent),
      mnPreferredColumnCount(2)
{
    WinBits aStyle =
        WB_ITEMBORDER
        | WB_DOUBLEBORDER
        | WB_NAMEFIELD
        | WB_FLATVALUESET
        | WB_TABSTOP
        | WB_VSCROLL;

    SetStyle(GetStyle() | aStyle);
    SetExtraSpacing(2);

    Fill ();
    Show();
}




ColorPanel::~ColorPanel (void)
{
}

void ColorPanel::Resize (void)
{
    ::Window::Resize();
    Size aWindowSize = GetOutputSizePixel();
    SetPosSizePixel(Point(0,0), aWindowSize);
    if (IsVisible() && aWindowSize.Width() > 0)
    {
        // Calculate the number of rows and columns.
        if (GetItemCount() > 0)
        {
            Image aImage = GetItemImage(GetItemId(0));
            Size aItemSize = CalcItemSizePixel (
                aImage.GetSizePixel());
            int nColumnCount = aWindowSize.Width() / 30;
            if (nColumnCount < 1)
                nColumnCount = 1;
            else if (nColumnCount > 4)
                nColumnCount = 4;

            sal_uInt16 nRowCount = (sal_uInt16)CalculateRowCount(aItemSize, nColumnCount);

            SetColCount((sal_uInt16)nColumnCount);
            SetLineCount(nRowCount);
        }
    }

}




int ColorPanel::CalculateRowCount (const Size&, int nColumnCount)
{
    int nRowCount = 0;

    if (GetItemCount()>0 && nColumnCount>0)
    {
        nRowCount = GetOutputSizePixel().Height() / 30;
        if (nRowCount < 1)
            nRowCount = 1;
    }

    return nRowCount;
}




void ColorPanel::DataChanged (const DataChangedEvent& rEvent)
{
    Fill();
}




void ColorPanel::Fill (void)
{
    const StyleSettings& rSettings (
        Application::GetSettings().GetStyleSettings());
    Clear();
    SetItemWidth (30);
    SetItemHeight (30);
    sal_uInt16 i = 0;
    InsertItem (++i, rSettings.GetFaceColor());
    SetItemText (i, String::CreateFromAscii("FaceColor"));
    InsertItem (++i, rSettings.GetCheckedColor());
    SetItemText (i, String::CreateFromAscii("CheckedColor"));
    InsertItem (++i, rSettings.GetLightColor());
    SetItemText (i, String::CreateFromAscii("LightColor"));
    InsertItem (++i, rSettings.GetLightBorderColor());
    SetItemText (i, String::CreateFromAscii("LightBorderColor"));
    InsertItem (++i, rSettings.GetShadowColor());
    SetItemText (i, String::CreateFromAscii("ShadowColor"));
    InsertItem (++i, rSettings.GetDarkShadowColor());
    SetItemText (i, String::CreateFromAscii("DarkShadowColor"));
    InsertItem (++i, rSettings.GetButtonTextColor());
    SetItemText (i, String::CreateFromAscii("ButtonTextColor"));
    InsertItem (++i, rSettings.GetRadioCheckTextColor());
    SetItemText (i, String::CreateFromAscii("RadioCheckTextColor"));
    InsertItem (++i, rSettings.GetGroupTextColor());
    SetItemText (i, String::CreateFromAscii("GroupTextColor"));
    InsertItem (++i, rSettings.GetLabelTextColor());
    SetItemText (i, String::CreateFromAscii("LabelTextColor"));
    InsertItem (++i, rSettings.GetInfoTextColor());
    SetItemText (i, String::CreateFromAscii("InfoTextColor"));
    InsertItem (++i, rSettings.GetWindowColor());
    SetItemText (i, String::CreateFromAscii("WindowColor"));
    InsertItem (++i, rSettings.GetWindowTextColor());
    SetItemText (i, String::CreateFromAscii("WindowTextColor"));
    InsertItem (++i, rSettings.GetDialogColor());
    SetItemText (i, String::CreateFromAscii("DialogColor"));
    InsertItem (++i, rSettings.GetDialogTextColor());
    SetItemText (i, String::CreateFromAscii("DialogTextColor"));
    InsertItem (++i, rSettings.GetWorkspaceColor());
    SetItemText (i, String::CreateFromAscii("WorkspaceColor"));
    InsertItem (++i, rSettings.GetFieldColor());
    SetItemText (i, String::CreateFromAscii("FieldColor"));
    InsertItem (++i, rSettings.GetFieldTextColor());
    SetItemText (i, String::CreateFromAscii("FieldTextColor"));
    InsertItem (++i, rSettings.GetActiveColor());
    SetItemText (i, String::CreateFromAscii("ActiveColor"));
    InsertItem (++i, rSettings.GetActiveColor2());
    SetItemText (i, String::CreateFromAscii("ActiveColor2"));
    InsertItem (++i, rSettings.GetActiveTextColor());
    SetItemText (i, String::CreateFromAscii("ActiveTextColor"));
    InsertItem (++i, rSettings.GetActiveBorderColor());
    SetItemText (i, String::CreateFromAscii("ActiveBorderColor"));
    InsertItem (++i, rSettings.GetDeactiveColor());
    SetItemText (i, String::CreateFromAscii("DeactiveColor"));
    InsertItem (++i, rSettings.GetDeactiveColor2());
    SetItemText (i, String::CreateFromAscii("DeactiveColor2"));
    InsertItem (++i, rSettings.GetDeactiveTextColor());
    SetItemText (i, String::CreateFromAscii("DeactiveTextColor"));
    InsertItem (++i, rSettings.GetDeactiveBorderColor());
    SetItemText (i, String::CreateFromAscii("DeactiveBorderColor"));
    InsertItem (++i, rSettings.GetHighlightColor());
    SetItemText (i, String::CreateFromAscii("HighlightColor"));
    InsertItem (++i, rSettings.GetHighlightTextColor());
    SetItemText (i, String::CreateFromAscii("HighlightTextColor"));
    InsertItem (++i, rSettings.GetDisableColor());
    SetItemText (i, String::CreateFromAscii("DisableColor"));
    InsertItem (++i, rSettings.GetHelpColor());
    SetItemText (i, String::CreateFromAscii("HelpColor"));
    InsertItem (++i, rSettings.GetHelpTextColor());
    SetItemText (i, String::CreateFromAscii("HelpTextColor"));
    InsertItem (++i, rSettings.GetMenuColor());
    SetItemText (i, String::CreateFromAscii("MenuColor"));
    InsertItem (++i, rSettings.GetMenuBarColor());
    SetItemText (i, String::CreateFromAscii("MenuBarColor"));
    InsertItem (++i, rSettings.GetMenuBorderColor());
    SetItemText (i, String::CreateFromAscii("MenuBorderColor"));
    InsertItem (++i, rSettings.GetMenuTextColor());
    SetItemText (i, String::CreateFromAscii("MenuTextColor"));
    InsertItem (++i, rSettings.GetMenuHighlightColor());
    SetItemText (i, String::CreateFromAscii("MenuHighlightColor"));
    InsertItem (++i, rSettings.GetMenuHighlightTextColor());
    SetItemText (i, String::CreateFromAscii("MenuHighlightTextColor"));
    InsertItem (++i, rSettings.GetLinkColor());
    SetItemText (i, String::CreateFromAscii("LinkColor"));
    InsertItem (++i, rSettings.GetVisitedLinkColor());
    SetItemText (i, String::CreateFromAscii("VisitedLinkColor"));
    InsertItem (++i, rSettings.GetHighlightLinkColor());
    SetItemText (i, String::CreateFromAscii("HighlightLinkColor"));
    InsertItem (++i, rSettings.GetFontColor());
    SetItemText (i, String::CreateFromAscii("FontColor"));
}

} } // end of namespace ::svx::sidebar
