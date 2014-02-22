/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/SvxColorChildWindow.hxx>
#include <sfx2/dockwin.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/colrctrl.hxx>
#include <svx/dialmgr.hxx>

SFX_IMPL_DOCKINGWINDOW_WITHID( SvxColorChildWindow, SID_COLOR_CONTROL )

/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Animator
|*
\************************************************************************/

SvxColorChildWindow::SvxColorChildWindow( Window* _pParent,
                                          sal_uInt16 nId,
                                          SfxBindings* pBindings,
                                          SfxChildWinInfo* pInfo ) :
    SfxChildWindow( _pParent, nId )
{
    SvxColorDockingWindow* pWin = new SvxColorDockingWindow( pBindings, this,
                                        _pParent, SVX_RES( RID_SVXCTRL_COLOR ) );
    pWindow = pWin;

    eChildAlignment = SFX_ALIGN_BOTTOM;

    pWin->Initialize( pInfo );
}
