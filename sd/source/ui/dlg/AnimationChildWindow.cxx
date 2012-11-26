/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    SetHideNotDelete( true );
}

} // end of namespace sd
