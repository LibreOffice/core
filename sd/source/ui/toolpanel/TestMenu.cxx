/*************************************************************************
 *
 *  $RCSfile: TestMenu.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:38:17 $
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

#include "TestMenu.hxx"

#include <vcl/image.hxx>
#include <vcl/svapp.hxx>

namespace sd { namespace toolpanel {


TestMenu::TestMenu (TreeNode* pParent)
    : Window (pParent->GetWindow()),
      TreeNode(pParent),
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
    pParent->RequestResize();
}




TestMenu::~TestMenu (void)
{
}




/** The preferred size depends on the preferred number of columns, the
    number of items, and the size of the items.
*/
Size TestMenu::GetPreferredSize (void)
{
    Size aItemSize = maSet.CalcItemSizePixel (Size());
    Size aPreferredWindowSize = maSet.CalcWindowSizePixel (
        aItemSize,
        mnPreferredColumnCount,
        CalculateRowCount (aItemSize, mnPreferredColumnCount));
    return aPreferredWindowSize;
}




sal_Int32 TestMenu::GetPreferredWidth (sal_Int32 nHeight)
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




sal_Int32 TestMenu::GetPreferredHeight (sal_Int32 nWidth)
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




bool TestMenu::IsResizable (void)
{
    return true;
}




::Window* TestMenu::GetWindow (void)
{
    return this;
}




void TestMenu::Resize (void)
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

            int nRowCount = CalculateRowCount (aItemSize, nColumnCount);

            maSet.SetColCount (nColumnCount);
            maSet.SetLineCount (nRowCount);
        }
    }

}




int TestMenu::CalculateRowCount (const Size& rItemSize, int nColumnCount)
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




void TestMenu::Fill (void)
{
    const StyleSettings& rSettings (
        Application::GetSettings().GetStyleSettings());
    maSet.Clear();
    maSet.SetItemWidth (30);
    maSet.SetItemHeight (30);
    int i = 0;
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

} } // end of namespace ::sd::toolpanel
