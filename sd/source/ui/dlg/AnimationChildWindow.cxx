/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AnimationChildWindow.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:56:34 $
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

#include "AnimationChildWindow.hxx"

#include "app.hrc"
#include "animobjs.hxx"
#include "animobjs.hrc"

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif
#ifndef _SFXDOCKWIN_HXX
#include <sfx2/dockwin.hxx>
#endif

namespace sd {

SFX_IMPL_DOCKINGWINDOW(AnimationChildWindow, SID_ANIMATION_OBJECTS)

/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Animator
|*
\************************************************************************/

AnimationChildWindow::AnimationChildWindow(
    ::Window* _pParent,
    USHORT nId,
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
    SetHideNotDelete( TRUE );
}

} // end of namespace sd
