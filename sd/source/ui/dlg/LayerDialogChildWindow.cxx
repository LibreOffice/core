/*************************************************************************
 *
 *  $RCSfile: LayerDialogChildWindow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 13:48:56 $
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
    ::Window* pParent,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : SfxChildWindow (pParent, nId)
{
    ViewShellBase& rBase (*ViewShellBase::GetViewShellBase(
        pBindings->GetDispatcher()->GetFrame()));
    LayerDialogContent* pContent = new LayerDialogContent (
        pBindings,
        this,
        pParent,
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
