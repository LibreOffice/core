/*************************************************************************
 *
 *  $RCSfile: ViewTabBar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:53:50 $
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

#include "ViewTabBar.hxx"

#define USE_TAB_CONTROL

#include "ViewShell.hxx"
#include "PaneManager.hxx"
#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "FrameView.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "helpids.h"
#include <vcl/tabpage.hxx>

namespace {

enum ViewTabBarEntry {
    VTBE_FIRST = 1,
    VTBE_EDIT_VIEW = VTBE_FIRST,
    VTBE_OUTLINE_VIEW,
    VTBE_NOTES_VIEW,
    VTBE_HANDOUT_VIEW,
    VTBE_SLIDE_VIEW,
    VTBE_LAST = VTBE_SLIDE_VIEW
};

} // end of anonymous namespace




namespace sd {

class ViewTabPage : public TabPage
{
public:
    ViewTabPage (Window* pParent) : TabPage(pParent) {}
    virtual void Resize (void)
    { SetPosSizePixel(Point(0,0),GetParent()->GetOutputSizePixel()); }
};

ViewTabBar::ViewTabBar (ViewShellBase& rViewShellBase, Window* pParent)
    : TabControl (pParent),
      mrViewShellBase (rViewShellBase)
{
    //  SetMaxPageWidth(150);
    //    SetHelpId( HID_SD_TABBAR_PAGES );

    // Add tabs for the views that can be displayed in the center pane.
    InsertPage (VTBE_EDIT_VIEW,
        String (SdResId (STR_DRAW_MODE)));
    InsertPage (VTBE_OUTLINE_VIEW,
        String (SdResId (STR_OUTLINE_MODE)));
    InsertPage (VTBE_NOTES_VIEW,
        String (SdResId (STR_NOTES_MODE)));
    InsertPage (VTBE_HANDOUT_VIEW,
        String (SdResId (STR_HANDOUT_MODE)));
    InsertPage (VTBE_SLIDE_VIEW,
        String (SdResId (STR_SLIDE_MODE)));
    //    InsertPage (VTBE_OLD_SLIDE_VIEW,
    //        String (SdResId (STR_SLIDE_MODE)));

    // Set one new tab page for all tab entries.  We need it only to
    // determine the height of the tab bar.
    TabPage* pTabPage = new TabPage (this);
    for (USHORT nIndex=VTBE_FIRST; nIndex<=VTBE_LAST; nIndex++)
    {
        SetTabPage (nIndex, pTabPage);
        pTabPage->Hide();
    }

    // Set help texts.
    SetHelpId (VTBE_EDIT_VIEW, HID_SD_BTN_DRAW);
    SetHelpId (VTBE_SLIDE_VIEW, HID_SD_BTN_SLIDE);
    //    SetHelpId (VTBE_OLD_SLIDE_VIEW, HID_SD_BTN_SLIDE);
    SetHelpId (VTBE_OUTLINE_VIEW, HID_SD_BTN_OUTLINE);
    SetHelpId (VTBE_NOTES_VIEW, HID_SD_BTN_NOTES);
    SetHelpId (VTBE_HANDOUT_VIEW, HID_SD_BTN_HANDOUT);

    // Register as listener at the view shell base.
    mrViewShellBase.GetPaneManager().AddEventListener (
        LINK(this, ViewTabBar, ViewShellBaseEventHandler));
}




ViewTabBar::~ViewTabBar (void)
{
    // Set all references to the one tab page to NULL and delete the page.
    TabPage* pTabPage = GetTabPage (VTBE_FIRST);
    for (USHORT nIndex=VTBE_FIRST; nIndex<=VTBE_LAST; nIndex++)
    {
        SetTabPage (nIndex, NULL);
    }
    delete pTabPage;

    // Tell the view shell base that we are not able to listen anymore.
    mrViewShellBase.GetPaneManager().RemoveEventListener (
        LINK(this, ViewTabBar, ViewShellBaseEventHandler));
}




void ViewTabBar::ActivatePage (void)
{
    TabControl::ActivatePage ();
    ViewShell::ShellType eType (
        mrViewShellBase.GetPaneManager().GetViewShellType(
            PaneManager::PT_CENTER));
    PageKind ePageKind (PK_STANDARD);
    switch (GetCurPageId())
    {
        case VTBE_EDIT_VIEW:
            eType = ViewShell::ST_IMPRESS;
            ePageKind = PK_STANDARD;
            break;

        case VTBE_OUTLINE_VIEW:
            eType = ViewShell::ST_OUTLINE;
            break;

        case VTBE_NOTES_VIEW:
            eType = ViewShell::ST_NOTES;
            ePageKind = PK_NOTES;
            break;

        case VTBE_HANDOUT_VIEW:
            eType = ViewShell::ST_HANDOUT;
            ePageKind = PK_HANDOUT;
            break;

        case VTBE_SLIDE_VIEW:
            eType = ViewShell::ST_SLIDE_SORTER;
            break;

        default:
            eType = ViewShell::ST_NONE;
            break;
    }

    ViewShell* pViewShell = mrViewShellBase.GetMainViewShell();
    FrameView* pFrameView = pViewShell->GetFrameView();
    pFrameView->SetViewShEditMode (EM_PAGE, pFrameView->GetPageKind());
    DrawViewShell* pDrawViewShell = static_cast<DrawViewShell*>(pViewShell);
    if (pDrawViewShell != NULL)
    {
        pFrameView->SetLayerMode (pDrawViewShell->IsLayerModeActive());
        pFrameView->SetViewShEditMode(EM_PAGE, ePageKind);
    }
    mrViewShellBase.GetPaneManager().RequestMainViewShellChange (eType);
}




void ViewTabBar::Paint (const Rectangle& rRect)
{
    Color aOriginalFillColor (GetFillColor());
    Color aOriginalLineColor (GetLineColor());

    SetFillColor (GetSettings().GetStyleSettings().GetDialogColor());
    SetLineColor ();
    DrawRect (rRect);
    TabControl::Paint (rRect);

    SetFillColor (aOriginalFillColor);
    SetLineColor (aOriginalLineColor);
}




int ViewTabBar::GetHeight (void)
{
    int nHeight = 0;

    TabPage* pActivePage (GetTabPage(GetCurPageId()));
    if (pActivePage != NULL)
    {
        nHeight = pActivePage->GetPosPixel().Y();
        nHeight -= 2;
    }

    if (nHeight <= 0)
        nHeight = GetOutputSizePixel().Height();

    return nHeight;
}




IMPL_LINK(ViewTabBar, ViewShellBaseEventHandler, PaneManagerEvent*, pEvent)
{
    if (pEvent->meEventId == PaneManagerEvent::EID_VIEW_SHELL_ADDED
        && pEvent->mePane == PaneManager::PT_CENTER)
    {
        // Select the tab of the currently active view.
        ViewTabBarEntry eActiveView = VTBE_EDIT_VIEW;
        switch (mrViewShellBase.GetPaneManager().GetViewShellType (
            PaneManager::PT_CENTER))
        {
            case ViewShell::ST_DRAW:
            case ViewShell::ST_IMPRESS:
                eActiveView = VTBE_EDIT_VIEW;
                break;

            case ViewShell::ST_OUTLINE:
                eActiveView = VTBE_OUTLINE_VIEW;
                break;

            case ViewShell::ST_SLIDE_SORTER:
                eActiveView = VTBE_SLIDE_VIEW;
                break;

            case ViewShell::ST_NOTES:
                eActiveView = VTBE_NOTES_VIEW;
                break;

            case ViewShell::ST_HANDOUT:
                eActiveView = VTBE_HANDOUT_VIEW;
                break;
        }
        SetCurPageId (eActiveView);
    }

    return 0;
}


} // end of namespace sd
