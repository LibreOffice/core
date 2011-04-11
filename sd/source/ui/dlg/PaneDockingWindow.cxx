/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "PaneDockingWindow.hxx"
#include "Window.hxx"
#include "ViewShellBase.hxx"
#include "framework/FrameworkHelper.hxx"
#include "sdresid.hxx"
#include "res_bmp.hrc"
#include <sfx2/dispatch.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/svapp.hxx>
#include <tools/wintypes.hxx>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sfx2::TitledDockingWindow;

namespace sd {

PaneDockingWindow::PaneDockingWindow(
        SfxBindings *_pBindings, SfxChildWindow *pChildWindow, ::Window* pParent,
        const ResId& rResId, const ::rtl::OUString& rsTitle )
        :TitledDockingWindow( _pBindings, pChildWindow, pParent, rResId )
{
    SetTitle( rsTitle );
}

PaneDockingWindow::~PaneDockingWindow (void)
{
}

void PaneDockingWindow::StateChanged( StateChangedType nType )
{
    switch (nType)
    {
        case STATE_CHANGE_INITSHOW:
            Resize();
            GetContentWindow().SetStyle(GetContentWindow().GetStyle() | WB_DIALOGCONTROL);
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

void PaneDockingWindow::MouseButtonDown (const MouseEvent& rEvent)
{
    if (rEvent.GetButtons() == MOUSE_LEFT)
    {
        // For some strange reason we have to set the WB_DIALOGCONTROL at
        // the content window in order to have it pass focus to its content
        // window.  Without setting this flag here that works only on views
        // that have not been taken from the cash and relocated to this pane
        // docking window.
        GetContentWindow().SetStyle(GetContentWindow().GetStyle() | WB_DIALOGCONTROL);
        GetContentWindow().GrabFocus();
    }
    SfxDockingWindow::MouseButtonDown(rEvent);
}








void PaneDockingWindow::SetValidSizeRange (const Range aValidSizeRange)
{
    SplitWindow* pSplitWindow = dynamic_cast<SplitWindow*>(GetParent());
    if (pSplitWindow != NULL)
    {
        const sal_uInt16 nId (pSplitWindow->GetItemId(static_cast< ::Window*>(this)));
        const sal_uInt16 nSetId (pSplitWindow->GetSet(nId));
        // Because the PaneDockingWindow paints its own decoration, we have
        // to compensate the valid size range for that.
        const SvBorder aBorder (GetDecorationBorder());
        sal_Int32 nCompensation (pSplitWindow->IsHorizontal()
            ? mnTitleBarHeight + aBorder.Top() + aBorder.Bottom()
            : aBorder.Left() + aBorder.Right());
        pSplitWindow->SetItemSizeRange(
            nSetId,
            Range(
                aValidSizeRange.Min() + nCompensation,
                aValidSizeRange.Max() + nCompensation));
    }
}




PaneDockingWindow::Orientation PaneDockingWindow::GetOrientation (void) const
{
    SplitWindow* pSplitWindow = dynamic_cast<SplitWindow*>(GetParent());
    if (pSplitWindow == NULL)
        return UnknownOrientation;
    else if (pSplitWindow->IsHorizontal())
        return HorizontalOrientation;
    else
        return VerticalOrientation;
}

} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
