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

#include "ColorPanel.hxx"

#include <vcl/image.hxx>
#include <vcl/svapp.hxx>

namespace svx { namespace sidebar {

ColorPanel::ColorPanel (::Window* pParent)
    : ValueSet(pParent, WB_ITEMBORDER)
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




void ColorPanel::DataChanged (const DataChangedEvent& /*rEvent*/)
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
    SetItemText (i, OUString("FaceColor"));
    InsertItem (++i, rSettings.GetCheckedColor());
    SetItemText (i, OUString("CheckedColor"));
    InsertItem (++i, rSettings.GetLightColor());
    SetItemText (i, OUString("LightColor"));
    InsertItem (++i, rSettings.GetLightBorderColor());
    SetItemText (i, OUString("LightBorderColor"));
    InsertItem (++i, rSettings.GetShadowColor());
    SetItemText (i, OUString("ShadowColor"));
    InsertItem (++i, rSettings.GetDarkShadowColor());
    SetItemText (i, OUString("DarkShadowColor"));
    InsertItem (++i, rSettings.GetButtonTextColor());
    SetItemText (i, OUString("ButtonTextColor"));
    InsertItem (++i, rSettings.GetRadioCheckTextColor());
    SetItemText (i, OUString("RadioCheckTextColor"));
    InsertItem (++i, rSettings.GetGroupTextColor());
    SetItemText (i, OUString("GroupTextColor"));
    InsertItem (++i, rSettings.GetLabelTextColor());
    SetItemText (i, OUString("LabelTextColor"));
    InsertItem (++i, rSettings.GetInfoTextColor());
    SetItemText (i, OUString("InfoTextColor"));
    InsertItem (++i, rSettings.GetWindowColor());
    SetItemText (i, OUString("WindowColor"));
    InsertItem (++i, rSettings.GetWindowTextColor());
    SetItemText (i, OUString("WindowTextColor"));
    InsertItem (++i, rSettings.GetDialogColor());
    SetItemText (i, OUString("DialogColor"));
    InsertItem (++i, rSettings.GetDialogTextColor());
    SetItemText (i, OUString("DialogTextColor"));
    InsertItem (++i, rSettings.GetWorkspaceColor());
    SetItemText (i, OUString("WorkspaceColor"));
    InsertItem (++i, rSettings.GetFieldColor());
    SetItemText (i, OUString("FieldColor"));
    InsertItem (++i, rSettings.GetFieldTextColor());
    SetItemText (i, OUString("FieldTextColor"));
    InsertItem (++i, rSettings.GetActiveColor());
    SetItemText (i, OUString("ActiveColor"));
    InsertItem (++i, rSettings.GetActiveColor2());
    SetItemText (i, OUString("ActiveColor2"));
    InsertItem (++i, rSettings.GetActiveTextColor());
    SetItemText (i, OUString("ActiveTextColor"));
    InsertItem (++i, rSettings.GetActiveBorderColor());
    SetItemText (i, OUString("ActiveBorderColor"));
    InsertItem (++i, rSettings.GetDeactiveColor());
    SetItemText (i, OUString("DeactiveColor"));
    InsertItem (++i, rSettings.GetDeactiveColor2());
    SetItemText (i, OUString("DeactiveColor2"));
    InsertItem (++i, rSettings.GetDeactiveTextColor());
    SetItemText (i, OUString("DeactiveTextColor"));
    InsertItem (++i, rSettings.GetDeactiveBorderColor());
    SetItemText (i, OUString("DeactiveBorderColor"));
    InsertItem (++i, rSettings.GetHighlightColor());
    SetItemText (i, OUString("HighlightColor"));
    InsertItem (++i, rSettings.GetHighlightTextColor());
    SetItemText (i, OUString("HighlightTextColor"));
    InsertItem (++i, rSettings.GetDisableColor());
    SetItemText (i, OUString("DisableColor"));
    InsertItem (++i, rSettings.GetHelpColor());
    SetItemText (i, OUString("HelpColor"));
    InsertItem (++i, rSettings.GetHelpTextColor());
    SetItemText (i, OUString("HelpTextColor"));
    InsertItem (++i, rSettings.GetMenuColor());
    SetItemText (i, OUString("MenuColor"));
    InsertItem (++i, rSettings.GetMenuBarColor());
    SetItemText (i, OUString("MenuBarColor"));
    InsertItem (++i, rSettings.GetMenuBorderColor());
    SetItemText (i, OUString("MenuBorderColor"));
    InsertItem (++i, rSettings.GetMenuTextColor());
    SetItemText (i, OUString("MenuTextColor"));
    InsertItem (++i, rSettings.GetMenuHighlightColor());
    SetItemText (i, OUString("MenuHighlightColor"));
    InsertItem (++i, rSettings.GetMenuHighlightTextColor());
    SetItemText (i, OUString("MenuHighlightTextColor"));
    InsertItem (++i, rSettings.GetLinkColor());
    SetItemText (i, OUString("LinkColor"));
    InsertItem (++i, rSettings.GetVisitedLinkColor());
    SetItemText (i, OUString("VisitedLinkColor"));
    InsertItem (++i, rSettings.GetHighlightLinkColor());
    SetItemText (i, OUString("HighlightLinkColor"));
    InsertItem (++i, rSettings.GetFontColor());
    SetItemText (i, OUString("FontColor"));
}

} } // end of namespace ::svx::sidebar
