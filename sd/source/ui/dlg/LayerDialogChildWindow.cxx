/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LayerDialogChildWindow.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:56:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "LayerDialogChildWindow.hxx"
#ifndef _SFXDOCKWIN_HXX
#include <sfx2/dockwin.hxx>
#endif
#include "app.hrc"
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif

// Instantiate the implementation of the docking window before files
// are included that define ::sd::Window.  The ... macros are not really
// namespace proof.
namespace sd {
SFX_IMPL_DOCKINGWINDOW(LayerDialogChildWindow, SID_LAYER_DIALOG_WIN)
}

#include "LayerDialog.hrc"

#ifndef SD_LAYER_DIALOG_CONTENT_HXX
#include "LayerDialogContent.hxx"
#endif


#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif

namespace sd {


LayerDialogChildWindow::LayerDialogChildWindow (
    ::Window* _pParent,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : SfxChildWindow (_pParent, nId)
{
    ViewShellBase& rBase (*ViewShellBase::GetViewShellBase(
        pBindings->GetDispatcher()->GetFrame()));
    LayerDialogContent* pContent = new LayerDialogContent (
        pBindings,
        this,
        _pParent,
        SdResId( FLT_WIN_LAYER_DIALOG),
        rBase);
    pWindow = pContent;

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pContent->Initialize(pInfo);
}



LayerDialogChildWindow::~LayerDialogChildWindow (void)
{
}



} // end of namespace sd
