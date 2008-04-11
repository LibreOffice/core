/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PaneDockingWindow.cxx,v $
 * $Revision: 1.17 $
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

#include "PaneDockingWindow.hxx"
#include "Window.hxx"
#include "ViewShellBase.hxx"
#include "sdresid.hxx"
#include "res_bmp.hrc"
#include <sfx2/dispatch.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/taskpanelist.hxx>
#include "framework/FrameworkHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd {

PaneDockingWindow::PaneDockingWindow (
    SfxBindings *_pBindings,
    SfxChildWindow *pChildWindow,
    ::Window* pParent,
    const ResId& rResId,
    const ::rtl::OUString& rsPaneURL,
    const ::rtl::OUString& rsTitle)
    : SfxDockingWindow (
        _pBindings,
        pChildWindow,
        pParent,
        rResId
        ),
      msPaneURL(rsPaneURL),
      msTitle(rsTitle),
      mpTitleToolBox(NULL),
      maBorder (3,1,3,3),
      mnChildWindowId(pChildWindow->GetType()),
      mpContentWindow(new ::Window(this)),
      mbIsLayoutPending(false)
{
    SetBackground (Wallpaper());

    InitializeTitleToolBox();

    // Tell the system window about the new docking window so that it can be
    // reached via the keyboard.
    SystemWindow* pSystemWindow = GetSystemWindow();
    if (pSystemWindow != NULL)
        pSystemWindow->GetTaskPaneList()->AddWindow(this);

    mpContentWindow->Show();
}




PaneDockingWindow::~PaneDockingWindow (void)
{
    // Tell the next system window that the docking window is no longer
    // available.
    SystemWindow* pSystemWindow = GetSystemWindow();
    if (pSystemWindow != NULL)
        pSystemWindow->GetTaskPaneList()->RemoveWindow(this);
    mpTitleToolBox.reset();
}




void PaneDockingWindow::SetTitle (const String& rsTitle)
{
    msTitle = rsTitle;
    Invalidate();
}




void PaneDockingWindow::Resize (void)
{
    SfxDockingWindow::Resize();
    mbIsLayoutPending = true;
}




void PaneDockingWindow::Layout (void)
{
    mbIsLayoutPending = false;

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

    // Place the content window.
    if (nTitleBarHeight < aToolBoxSize.Height())
        nTitleBarHeight = aToolBoxSize.Height();
    aWindowSize.Height() -= nTitleBarHeight;
    mpContentWindow->SetPosSizePixel(
        Point(maBorder.Left(),nTitleBarHeight+maBorder.Top()),
        Size (aWindowSize.Width()-maBorder.Left()-maBorder.Right(),
            aWindowSize.Height()-maBorder.Top()-maBorder.Bottom()));
}




void PaneDockingWindow::Paint (const Rectangle& rRectangle)
{
    if (mbIsLayoutPending)
        Layout();

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




void PaneDockingWindow::InitializeTitleToolBox (void)
{
    if (mpTitleToolBox.get() == NULL)
    {
        // Initialize the title tool box.
        mpTitleToolBox.reset (new ToolBox(this));
        mpTitleToolBox->SetSelectHdl (
            LINK(this, PaneDockingWindow, ToolboxSelectHandler));
        mpTitleToolBox->SetOutStyle (TOOLBOX_STYLE_FLAT);
        mpTitleToolBox->SetBackground (Wallpaper (
            GetSettings().GetStyleSettings().GetDialogColor()));
        mpTitleToolBox->Show();
    }
    else
        mpTitleToolBox->Clear();

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




USHORT PaneDockingWindow::AddMenu (
    const String& rsMenuName,
    ULONG nHelpId,
    const Link& rCallback)
{
    // Add the menu before the closer button.
    USHORT nItemCount (mpTitleToolBox->GetItemCount());
    USHORT nItemId (nItemCount+1);
    mpTitleToolBox->InsertItem (
        nItemId,
        rsMenuName,
        TIB_DROPDOWNONLY,
        nItemCount>0 ? nItemCount-1 : (USHORT)-1);
    mpTitleToolBox->SetHelpId( nItemId, nHelpId );
    mpTitleToolBox->SetClickHdl (rCallback);
    mpTitleToolBox->SetDropdownClickHdl (rCallback);

    // The tool box has likely changed its size. The title bar has to be
    // resized.
    Resize();
    Invalidate();

    return nItemCount+1;
}




IMPL_LINK(PaneDockingWindow, ToolboxSelectHandler, ToolBox*, pToolBox)
{
    USHORT nId = pToolBox->GetCurItemId();

    if (nId == 1)
    {
        EndTracking();
        SfxBoolItem aVisibility (mnChildWindowId, FALSE);
        GetBindings().GetDispatcher()->Execute (
            mnChildWindowId,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
            &aVisibility,
            NULL);
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

        case STATE_CHANGE_VISIBLE:
            // The visibility of the docking window has changed.  Tell the
            // ConfigurationController so that it can activate or deactivate
            // a/the view for the pane.
            // Without this the side panes remain empty after closing an
            // in-place slide show.
            ViewShellBase* pBase = ViewShellBase::GetViewShellBase(
                GetBindings().GetDispatcher()->GetFrame());
            if (pBase != NULL)
            {
                framework::FrameworkHelper::Instance(*pBase)->UpdateConfiguration();
            }
            break;
    }
    SfxDockingWindow::StateChanged (nType);
}




void PaneDockingWindow::DataChanged (const DataChangedEvent& rEvent)
{
    SfxDockingWindow::DataChanged (rEvent);

    switch (rEvent.GetType())
    {
        case DATACHANGED_SETTINGS:
            if ((rEvent.GetFlags() & SETTINGS_STYLE) == 0)
                break;
            // else fall through.
        case DATACHANGED_FONTS:
        case DATACHANGED_FONTSUBSTITUTION:
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

            // Font.
            Font aFont = rStyleSettings.GetAppFont();
            if (IsControlFont())
                aFont.Merge(GetControlFont());
            SetZoomedPointFont(aFont);

            // Color.
            Color aColor;
            if (IsControlForeground())
                aColor = GetControlForeground();
            else
                aColor = rStyleSettings.GetButtonTextColor();
            SetTextColor(aColor);
            SetTextFillColor();

            Resize();
            Invalidate();
        }
        break;
    }
}




::Window* PaneDockingWindow::GetContentWindow (void)
{
    return mpContentWindow.get();
}

} // end of namespace ::sd
