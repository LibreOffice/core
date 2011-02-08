/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "TestMenu.hxx"

#include "taskpane/TaskPaneControlFactory.hxx"

#include <vcl/image.hxx>
#include <vcl/svapp.hxx>

namespace sd { namespace toolpanel {

#ifdef SHOW_COLOR_MENU
/** This factory class is used to create instances of ColorMenu.  It can be
    extended so that its constructor stores arguments that later are passed
    to new ColorMenu objects.
*/
class ColorMenuFactory
    : public ControlFactory
{
protected:
    virtual TreeNode* InternalCreateControl( ::Window& i_rParent )
    {
        return new ColorMenu (&i_rParent);
    }
};


ColorMenu::ColorMenu (::Window* i_pParent)
    : Window (i_pParent),
      TreeNode(NULL),
      maSet (this),
      mnPreferredColumnCount(2)
{
    WinBits aStyle =
        WB_ITEMBORDER
        | WB_DOUBLEBORDER
        | WB_NAMEFIELD
        | WB_FLATVALUESET
        | WB_TABSTOP
        | WB_VSCROLL;

    maSet.SetStyle (maSet.GetStyle() | aStyle);
    maSet.SetExtraSpacing(2);

    Fill ();
    maSet.Show();
    i_pParent->Resize();
}




ColorMenu::~ColorMenu (void)
{
}


::std::auto_ptr<ControlFactory> ColorMenu::CreateControlFactory (void)
{
    return ::std::auto_ptr<ControlFactory>(new ColorMenuFactory());
}


/** The preferred size depends on the preferred number of columns, the
    number of items, and the size of the items.
*/
Size ColorMenu::GetPreferredSize (void)
{
    Size aItemSize = maSet.CalcItemSizePixel (Size());
    Size aPreferredWindowSize = maSet.CalcWindowSizePixel (
        aItemSize,
        (sal_uInt16)mnPreferredColumnCount,
        (sal_uInt16)CalculateRowCount (aItemSize, (sal_uInt16)mnPreferredColumnCount));
    return aPreferredWindowSize;
}




sal_Int32 ColorMenu::GetPreferredWidth (sal_Int32 nHeight)
{
    sal_Int32 nPreferredWidth = 0;
    if (maSet.GetItemCount() > 0)
    {
        Image aImage = maSet.GetItemImage(maSet.GetItemId(0));
        Size aItemSize = maSet.CalcItemSizePixel (aImage.GetSizePixel());
        if (nHeight>0 && aItemSize.Height()>0)
        {
            int nRowCount = nHeight / aItemSize.Height();
            if (nRowCount <= 0)
                nRowCount = 1;
            int nColumnCount = (maSet.GetItemCount() + nRowCount-1)
                / nRowCount;
            nPreferredWidth = nColumnCount * aItemSize.Width();
        }
    }

    return nPreferredWidth;
}




sal_Int32 ColorMenu::GetPreferredHeight (sal_Int32 nWidth)
{
    sal_Int32 nPreferredHeight = 0;
    if (maSet.GetItemCount()>0)
    {
        Image aImage = maSet.GetItemImage(maSet.GetItemId(0));
        Size aItemSize = maSet.CalcItemSizePixel (aImage.GetSizePixel());
        if (nWidth>0 && aItemSize.Width()>0)
        {
            int nColumnCount = nWidth / aItemSize.Width();
            if (nColumnCount <= 0)
                nColumnCount = 1;
            else if (nColumnCount > 4)
                nColumnCount = 4;
            int nRowCount = (maSet.GetItemCount() + nColumnCount-1)
                / nColumnCount;
            nPreferredHeight = nRowCount * aItemSize.Height();
        }
    }
    return nPreferredHeight;
}




bool ColorMenu::IsResizable (void)
{
    return true;
}




::Window* ColorMenu::GetWindow (void)
{
    return this;
}




void ColorMenu::Resize (void)
{
    ::Window::Resize();
    Size aWindowSize = GetOutputSizePixel();
    maSet.SetPosSizePixel (Point(0,0), aWindowSize);
    if (IsVisible() && aWindowSize.Width() > 0)
    {
        //        maSet.SetPosSizePixel (
        //            Point (0,0),
        //            aWindowSize);

        // Calculate the number of rows and columns.
        if (maSet.GetItemCount() > 0)
        {
            Image aImage = maSet.GetItemImage(maSet.GetItemId(0));
            Size aItemSize = maSet.CalcItemSizePixel (
                aImage.GetSizePixel());
            int nColumnCount = aWindowSize.Width() / 30;
            if (nColumnCount < 1)
                nColumnCount = 1;
            else if (nColumnCount > 4)
                nColumnCount = 4;

            sal_uInt16 nRowCount = (sal_uInt16)CalculateRowCount (aItemSize, nColumnCount);

            maSet.SetColCount ((sal_uInt16)nColumnCount);
            maSet.SetLineCount (nRowCount);
        }
    }

}




int ColorMenu::CalculateRowCount (const Size&, int nColumnCount)
{
    int nRowCount = 0;

    if (maSet.GetItemCount()>0 && nColumnCount>0)
    {
        nRowCount = GetOutputSizePixel().Height() / 30;
        if (nRowCount < 1)
            nRowCount = 1;
    }

    return nRowCount;
}




void ColorMenu::Fill (void)
{
    const StyleSettings& rSettings (
        Application::GetSettings().GetStyleSettings());
    maSet.Clear();
    maSet.SetItemWidth (30);
    maSet.SetItemHeight (30);
    sal_uInt16 i = 0;
    maSet.InsertItem (++i, rSettings.GetFaceColor());
    maSet.SetItemText (i, String::CreateFromAscii("FaceColor"));
    maSet.InsertItem (++i, rSettings.GetCheckedColor());
    maSet.SetItemText (i, String::CreateFromAscii("CheckedColor"));
    maSet.InsertItem (++i, rSettings.GetLightColor());
    maSet.SetItemText (i, String::CreateFromAscii("LightColor"));
    maSet.InsertItem (++i, rSettings.GetLightBorderColor());
    maSet.SetItemText (i, String::CreateFromAscii("LightBorderColor"));
    maSet.InsertItem (++i, rSettings.GetShadowColor());
    maSet.SetItemText (i, String::CreateFromAscii("ShadowColor"));
    maSet.InsertItem (++i, rSettings.GetDarkShadowColor());
    maSet.SetItemText (i, String::CreateFromAscii("DarkShadowColor"));
    maSet.InsertItem (++i, rSettings.GetButtonTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("ButtonTextColor"));
    maSet.InsertItem (++i, rSettings.GetRadioCheckTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("RadioCheckTextColor"));
    maSet.InsertItem (++i, rSettings.GetGroupTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("GroupTextColor"));
    maSet.InsertItem (++i, rSettings.GetLabelTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("LabelTextColor"));
    maSet.InsertItem (++i, rSettings.GetInfoTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("InfoTextColor"));
    maSet.InsertItem (++i, rSettings.GetWindowColor());
    maSet.SetItemText (i, String::CreateFromAscii("WindowColor"));
    maSet.InsertItem (++i, rSettings.GetWindowTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("WindowTextColor"));
    maSet.InsertItem (++i, rSettings.GetDialogColor());
    maSet.SetItemText (i, String::CreateFromAscii("DialogColor"));
    maSet.InsertItem (++i, rSettings.GetDialogTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("DialogTextColor"));
    maSet.InsertItem (++i, rSettings.GetWorkspaceColor());
    maSet.SetItemText (i, String::CreateFromAscii("WorkspaceColor"));
    maSet.InsertItem (++i, rSettings.GetFieldColor());
    maSet.SetItemText (i, String::CreateFromAscii("FieldColor"));
    maSet.InsertItem (++i, rSettings.GetFieldTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("FieldTextColor"));
    maSet.InsertItem (++i, rSettings.GetActiveColor());
    maSet.SetItemText (i, String::CreateFromAscii("ActiveColor"));
    maSet.InsertItem (++i, rSettings.GetActiveColor2());
    maSet.SetItemText (i, String::CreateFromAscii("ActiveColor2"));
    maSet.InsertItem (++i, rSettings.GetActiveTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("ActiveTextColor"));
    maSet.InsertItem (++i, rSettings.GetActiveBorderColor());
    maSet.SetItemText (i, String::CreateFromAscii("ActiveBorderColor"));
    maSet.InsertItem (++i, rSettings.GetDeactiveColor());
    maSet.SetItemText (i, String::CreateFromAscii("DeactiveColor"));
    maSet.InsertItem (++i, rSettings.GetDeactiveColor2());
    maSet.SetItemText (i, String::CreateFromAscii("DeactiveColor2"));
    maSet.InsertItem (++i, rSettings.GetDeactiveTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("DeactiveTextColor"));
    maSet.InsertItem (++i, rSettings.GetDeactiveBorderColor());
    maSet.SetItemText (i, String::CreateFromAscii("DeactiveBorderColor"));
    maSet.InsertItem (++i, rSettings.GetHighlightColor());
    maSet.SetItemText (i, String::CreateFromAscii("HighlightColor"));
    maSet.InsertItem (++i, rSettings.GetHighlightTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("HighlightTextColor"));
    maSet.InsertItem (++i, rSettings.GetDisableColor());
    maSet.SetItemText (i, String::CreateFromAscii("DisableColor"));
    maSet.InsertItem (++i, rSettings.GetHelpColor());
    maSet.SetItemText (i, String::CreateFromAscii("HelpColor"));
    maSet.InsertItem (++i, rSettings.GetHelpTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("HelpTextColor"));
    maSet.InsertItem (++i, rSettings.GetMenuColor());
    maSet.SetItemText (i, String::CreateFromAscii("MenuColor"));
    maSet.InsertItem (++i, rSettings.GetMenuBarColor());
    maSet.SetItemText (i, String::CreateFromAscii("MenuBarColor"));
    maSet.InsertItem (++i, rSettings.GetMenuBorderColor());
    maSet.SetItemText (i, String::CreateFromAscii("MenuBorderColor"));
    maSet.InsertItem (++i, rSettings.GetMenuTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("MenuTextColor"));
    maSet.InsertItem (++i, rSettings.GetMenuHighlightColor());
    maSet.SetItemText (i, String::CreateFromAscii("MenuHighlightColor"));
    maSet.InsertItem (++i, rSettings.GetMenuHighlightTextColor());
    maSet.SetItemText (i, String::CreateFromAscii("MenuHighlightTextColor"));
    maSet.InsertItem (++i, rSettings.GetLinkColor());
    maSet.SetItemText (i, String::CreateFromAscii("LinkColor"));
    maSet.InsertItem (++i, rSettings.GetVisitedLinkColor());
    maSet.SetItemText (i, String::CreateFromAscii("VisitedLinkColor"));
    maSet.InsertItem (++i, rSettings.GetHighlightLinkColor());
    maSet.SetItemText (i, String::CreateFromAscii("HighlightLinkColor"));
    maSet.InsertItem (++i, rSettings.GetFontColor());
    maSet.SetItemText (i, String::CreateFromAscii("FontColor"));
}
#endif

} } // end of namespace ::sd::toolpanel
