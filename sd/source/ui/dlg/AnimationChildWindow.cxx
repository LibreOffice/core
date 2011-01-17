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

#include "AnimationChildWindow.hxx"

#include "app.hrc"
#include "animobjs.hxx"
#include "animobjs.hrc"
#include <sfx2/app.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/dockwin.hxx>

namespace sd {

SFX_IMPL_DOCKINGWINDOW(AnimationChildWindow, SID_ANIMATION_OBJECTS)

/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Animator
|*
\************************************************************************/

AnimationChildWindow::AnimationChildWindow(
    ::Window* _pParent,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo )
    : SfxChildWindow( _pParent, nId )
{
    AnimationWindow* pAnimWin = new AnimationWindow(
        pBindings, this, _pParent, SdResId( FLT_WIN_ANIMATION ) );
    pWindow = pAnimWin;

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pAnimWin->Initialize( pInfo );
    /*
    if ( pInfo->aSize.Width() != 0 && pInfo->aSize.Height() != 0 )
    {
        pWindow->SetPosSizePixel( pInfo->aPos, pInfo->aSize );
    }
    else
        pWindow->SetPosPixel(SFX_APPWINDOW->OutputToScreenPixel(
                                SFX_APPWINDOW->GetClientAreaPixel().TopLeft()));

    if ( pInfo->nFlags & SFX_CHILDWIN_ZOOMIN )
        pAnimWin->ZoomIn();

    pAnimWin->aFltWinSize = pWindow->GetSizePixel();
    */
    SetHideNotDelete( sal_True );
}

} // end of namespace sd
