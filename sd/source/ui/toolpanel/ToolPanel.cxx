/*************************************************************************
 *
 *  $RCSfile: ToolPanel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:40:40 $
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

#include "ToolPanel.hxx"

#include "TaskPaneFocusManager.hxx"
#include "TitleBar.hxx"
#include "TitledControl.hxx"
#include "ControlContainer.hxx"
#include "TaskPaneViewShell.hxx"

#ifndef _SV_DECOVIEW_HXX
#include <vcl/decoview.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif


namespace sd { namespace toolpanel {


/** Use WB_DIALOGCONTROL as argument for the Control constructor to
    let VCL handle focus traveling.  In addition the control
    descriptors have to use WB_TABSTOP.
*/
ToolPanel::ToolPanel (
    Window* pParentWindow,
    TaskPaneViewShell& rViewShell)
    : Control (pParentWindow, WB_DIALOGCONTROL),
      mrViewShell(rViewShell),
      TreeNode (NULL),
      mbRearrangeActive(false)
{
    SetBackground (Wallpaper ());
    SetAccessibleName (String::CreateFromAscii("Task Pane"));
    SetAccessibleDescription (String::CreateFromAscii("Impress task pane"));
}




ToolPanel::~ToolPanel (void)
{
}




void ToolPanel::AddControl (
    ::std::auto_ptr<TreeNode> pControl,
    const String& rTitle)
{
    ::std::auto_ptr<TreeNode> pChild (new TitledControl (
        this,
        pControl,
        rTitle,
        TitleBar::TBT_CONTROL_TITLE));

    // Add a down link only for the first control so that when
    // entering the sub tool panel the focus is set to the first control.
    if (mpControlContainer->GetControlCount() == 1)
        FocusManager::Instance().RegisterLink (
            GetParent(),
            pChild->GetWindow());
    else
        FocusManager::Instance().RegisterUpLink (
            pChild->GetWindow(),
            GetParent());

    mpControlContainer->AddControl (pChild);
}




void ToolPanel::ListHasChanged (void)
{
    mpControlContainer->ListHasChanged ();
    Rearrange ();
}




void ToolPanel::Resize (void)
{
    Control::Resize();
    Rearrange ();
}




void ToolPanel::RequestResize (void)
{
    Invalidate();
    Rearrange ();
}




/** Subtract the space for the title bars from the available space and
    give the remaining space to the active control.
*/
void ToolPanel::Rearrange (void)
{
    // Prevent recursive calls.
    if ( ! mbRearrangeActive && mpControlContainer->GetVisibleControlCount()>0)
    {
        mbRearrangeActive = true;

        SetBackground (Wallpaper ());

        // Make the area that is covered by the children a little bit
        // smaller so that a frame is visible arround them.
        Rectangle aAvailableArea (Point(0,0), GetOutputSizePixel());

        int nWidth = aAvailableArea.GetWidth();
        sal_uInt32 nControlCount (mpControlContainer->GetControlCount());
        sal_uInt32 nActiveControlIndex (
            mpControlContainer->GetActiveControlIndex());

        // Place title bars of controls above the active control and thereby
        // determine the top of the active control.
        sal_uInt32 nIndex;
        for (nIndex=mpControlContainer->GetFirstIndex();
             nIndex<nActiveControlIndex;
             nIndex=mpControlContainer->GetNextIndex(nIndex))
        {
            TreeNode* pChild = mpControlContainer->GetControl(nIndex);
            if (pChild != NULL)
            {
                sal_uInt32 nHeight = pChild->GetPreferredHeight (nWidth);
                pChild->GetWindow()->SetPosSizePixel (
                    aAvailableArea.TopLeft(),
                    Size(nWidth, nHeight));
                aAvailableArea.Top() += nHeight;
            }
        }

        // Place title bars of controls below the active control and thereby
        // determine the bottom of the active control.
        for (nIndex=mpControlContainer->GetLastIndex();
             nIndex<nControlCount && nIndex!=nActiveControlIndex;
             nIndex=mpControlContainer->GetPreviousIndex(nIndex))
        {
            TreeNode* pChild = mpControlContainer->GetControl(nIndex);
            if (pChild != NULL)
            {
                sal_uInt32 nHeight = pChild->GetPreferredHeight (nWidth);
                pChild->GetWindow()->SetPosSizePixel (
                    Point(aAvailableArea.Left(),
                        aAvailableArea.Bottom()-nHeight+1),
                        Size(nWidth, nHeight));
                aAvailableArea.Bottom() -= nHeight;
            }
        }

        // Finally place the active control.
        TreeNode* pChild = mpControlContainer->GetControl(nActiveControlIndex);
        if (pChild != NULL)
            pChild->GetWindow()->SetPosSizePixel (
                aAvailableArea.TopLeft(),
                aAvailableArea.GetSize());

        mbRearrangeActive = false;
    }
    else
        SetBackground (
            Application::GetSettings().GetStyleSettings().GetDialogColor());
}




Size ToolPanel::GetPreferredSize (void)
{
    return Size(300,300);
}




sal_Int32 ToolPanel::GetPreferredWidth (sal_Int32 nHeight)
{
    return 300;
}




sal_Int32 ToolPanel::GetPreferredHeight (sal_Int32 nWidth)
{
    return 300;
}




bool ToolPanel::IsResizable (void)
{
    return true;
}




::Window* ToolPanel::GetWindow (void)
{
    return this;
}




TaskPaneShellManager* ToolPanel::GetShellManager (void)
{
    return &mrViewShell.GetSubShellManager();
}

} } // end of namespace ::sd::toolpanel
