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
#include "sdresid.hxx"
#include "res_bmp.hrc"
#include <sfx2/dispatch.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/taskpanelist.hxx>
#include "framework/FrameworkHelper.hxx"

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

} // end of namespace ::sd
