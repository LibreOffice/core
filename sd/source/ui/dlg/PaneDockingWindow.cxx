/*************************************************************************
 *
 *  $RCSfile: PaneDockingWindow.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-04 08:53:51 $
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

#include "PaneDockingWindow.hxx"
#include "ViewShellBase.hxx"
#include "Window.hxx"
#include "sdresid.hxx"
#include "res_bmp.hrc"
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#include <vcl/toolbox.hxx>

namespace sd {

PaneDockingWindow::PaneDockingWindow (
    SfxBindings *pBindings,
    SfxChildWindow *pChildWindow,
    ::Window* pParent,
    const ResId& rResId,
    PaneManager::PaneType ePane,
    const String& rsTitle)
    : SfxDockingWindow (
        pBindings,
        pChildWindow,
        pParent,
        rResId
        ),
      mePane(ePane),
      msTitle(rsTitle),
      mpTitleToolBox(NULL),
      maBorder (3,1,3,3),
      mnChildWindowId(pChildWindow->GetType())
{
    SetBackground (Wallpaper());

    ViewShellBase& rBase (*ViewShellBase::GetViewShellBase(
        pBindings->GetDispatcher()->GetFrame()));
    rBase.GetPaneManager().SetWindow (mePane, this);

    // Initialize the title tool box.
    mpTitleToolBox.reset (new ToolBox(this));
    mpTitleToolBox->SetSelectHdl (
        LINK(this, PaneDockingWindow, ToolboxSelectHandler));
    mpTitleToolBox->SetOutStyle (TOOLBOX_STYLE_FLAT);
    mpTitleToolBox->SetBackground (Wallpaper (
        GetSettings().GetStyleSettings().GetDialogColor()));
    mpTitleToolBox->Show();

    // Get the closer bitmap and set it as right most button.
    Bitmap aBitmap (SdResId (BMP_CLOSE_DOC));
    Bitmap aBitmapHC (SdResId (BMP_CLOSE_DOC_H));
    Image aImage = Image (aBitmap, Color (COL_LIGHTMAGENTA));
    Image aImageHC = Image (aBitmapHC, Color (BMP_COLOR_HIGHCONTRAST));
    mpTitleToolBox->InsertItem (1,
        GetSettings().GetStyleSettings().GetMenuBarColor().IsDark()
        ? aImageHC
        : aImage);
    mpTitleToolBox->ShowItem (1);
}




PaneDockingWindow::~PaneDockingWindow (void)
{
    ViewShellBase& rBase (*ViewShellBase::GetViewShellBase(
        GetBindings().GetDispatcher()->GetFrame()));
    // Tell the ViewShellBase that the window of this slide sorter is not
    // available anymore.
    rBase.GetPaneManager().SetWindow (mePane, NULL);
}




void PaneDockingWindow::Resize (void)
{
    SfxDockingWindow::Resize();
    Size aWindowSize (GetOutputSizePixel());
    Size aToolBoxSize (0,0);
    int nTitleBarHeight (GetSettings().GetStyleSettings().GetTitleHeight());

    // Place the title tool box.
    if (mpTitleToolBox.get() != NULL)
    {
        if (IsFloatingMode())
            mpTitleToolBox->HideItem (1);
        else
            mpTitleToolBox->ShowItem (1);

        aToolBoxSize = mpTitleToolBox->CalcWindowSizePixel();
        if (aToolBoxSize.Height() > nTitleBarHeight)
            nTitleBarHeight = aToolBoxSize.Height();
        mpTitleToolBox->SetPosSizePixel (
            Point(aWindowSize.Width()-aToolBoxSize.Width(),
                (nTitleBarHeight-aToolBoxSize.Height())/2),
            aToolBoxSize);
    }

    // Place the view shell.
    ViewShellBase& rBase (*ViewShellBase::GetViewShellBase(
        GetBindings().GetDispatcher()->GetFrame()));
    ViewShell* pViewShell = rBase.GetPaneManager().GetViewShell (mePane);
    if (pViewShell != NULL)
    {
        ::sd::Window* pWindow = pViewShell->GetActiveWindow();
        if (nTitleBarHeight < aToolBoxSize.Height())
            nTitleBarHeight = aToolBoxSize.Height();
        aWindowSize.Height() -= nTitleBarHeight;
        pViewShell->Resize(
            Point(maBorder.Left(),nTitleBarHeight+maBorder.Top()),
            Size (aWindowSize.Width()-maBorder.Left()-maBorder.Right(),
                aWindowSize.Height()-maBorder.Top()-maBorder.Bottom()));
    }
}




void PaneDockingWindow::Paint (const Rectangle& rRectangle)
{
    SfxDockingWindow::Paint (rRectangle);
    int nTitleBarHeight (GetSettings().GetStyleSettings().GetTitleHeight());
    Size aToolBoxSize = mpTitleToolBox->CalcWindowSizePixel();
    if (aToolBoxSize.Height() > nTitleBarHeight)
        nTitleBarHeight = aToolBoxSize.Height();
    Color aOriginalLineColor (GetLineColor());
    Color aOriginalFillColor (GetFillColor());
    SetFillColor (GetSettings().GetStyleSettings().GetDialogColor());
    SetLineColor ();

    // Make font bold.
    const Font& rOriginalFont (GetFont());
    Font aFont (rOriginalFont);
    aFont.SetWeight (WEIGHT_BOLD);
    SetFont (aFont);

    // Set border values.
    Size aWindowSize (GetOutputSizePixel());
    int nOuterLeft = 0;
    int nInnerLeft = nOuterLeft + maBorder.Left() - 1;
    int nOuterRight = aWindowSize.Width() - 1;
    int nInnerRight = nOuterRight - maBorder.Right() + 1;
    int nInnerTop = nTitleBarHeight + maBorder.Top() - 1;
    int nOuterBottom = aWindowSize.Height() - 1;
    int nInnerBottom = nOuterBottom - maBorder.Bottom() + 1;

    // Paint title bar background.
    Rectangle aTitleBarBox (Rectangle(
        nOuterLeft,
        0,
        nOuterRight,
        nInnerTop-1));
    DrawRect (aTitleBarBox);

    if (nInnerLeft > nOuterLeft)
        DrawRect (
            Rectangle (nOuterLeft, nInnerTop, nInnerLeft, nInnerBottom));
    if (nOuterRight > nInnerRight)
        DrawRect (
            Rectangle (nInnerRight, nInnerTop, nOuterRight, nInnerBottom));
    if (nInnerBottom < nOuterBottom)
        DrawRect (
            Rectangle (nOuterLeft, nInnerBottom, nOuterRight, nOuterBottom));

    // Paint bevel border.
    SetFillColor ();
    SetLineColor (GetSettings().GetStyleSettings().GetShadowColor());
    if (maBorder.Top() > 0)
        DrawLine (
            Point(nInnerLeft,nInnerTop),
            Point(nInnerLeft,nInnerBottom));
    if (maBorder.Left() > 0)
        DrawLine (
            Point(nInnerLeft,nInnerTop),
            Point(nInnerRight,nInnerTop));
    SetLineColor (GetSettings().GetStyleSettings().GetLightColor());
    if (maBorder.Bottom() > 0)
        DrawLine (
            Point(nInnerRight,nInnerBottom),
            Point(nInnerLeft,nInnerBottom));
    if (maBorder.Right() > 0)
        DrawLine (
            Point(nInnerRight,nInnerBottom),
            Point(nInnerRight,nInnerTop));

    // Paint title bar text.
    SetLineColor (GetSettings().GetStyleSettings().GetActiveTextColor());
    SetFillColor ();
    aTitleBarBox.Left() += 3;
    DrawText (aTitleBarBox, msTitle,
        TEXT_DRAW_LEFT
        | TEXT_DRAW_VCENTER
        | TEXT_DRAW_MULTILINE
        | TEXT_DRAW_WORDBREAK);

    // Restore original values of the output device.
    SetFont (rOriginalFont);
    SetFillColor (aOriginalFillColor);
}




USHORT PaneDockingWindow::AddMenu (
    const String& rsMenuName,
    const Link& rCallback)
{
    // Add the menu before the closer button.
    int nItemCount (mpTitleToolBox->GetItemCount());
    mpTitleToolBox->InsertItem (
        nItemCount+1,
        rsMenuName,
        TIB_DROPDOWN,
        nItemCount-1);
    mpTitleToolBox->SetClickHdl (rCallback);
    mpTitleToolBox->SetDropdownClickHdl (rCallback);

    return nItemCount+1;
}




IMPL_LINK(PaneDockingWindow, ToolboxSelectHandler, ToolBox*, pToolBox)
{
    USHORT nId = pToolBox->GetCurItemId();

    if (nId == 1)
    {
        EndTracking();
        GetBindings().GetDispatcher()->Execute (
            mnChildWindowId,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
    }

    return 0;
}




long PaneDockingWindow::Notify( NotifyEvent& rNEvt )
{
    return SfxDockingWindow::Notify (rNEvt);
}




void PaneDockingWindow::StateChanged( StateChangedType nType )
{
    switch (nType)
    {
        case STATE_CHANGE_INITSHOW:
            Resize();
            break;
    }
    SfxDockingWindow::StateChanged (nType);
}




void PaneDockingWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxDockingWindow::DataChanged (rDCEvt);
}



} // end of namespace ::sd
