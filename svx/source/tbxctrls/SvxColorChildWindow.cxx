/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/SvxColorChildWindow.hxx>
#include <sfx2/dockwin.hxx>
#include <svx/svxids.hrc>
#include <svx/strings.hrc>
#include <svx/colrctrl.hxx>
#include <svx/dialmgr.hxx>

SFX_IMPL_DOCKINGWINDOW_WITHID( SvxColorChildWindow, SID_COLOR_CONTROL )

// Derivation from SfxChildWindow as "container" for animator
SvxColorChildWindow::SvxColorChildWindow( vcl::Window* _pParent,
                                          sal_uInt16 nId,
                                          SfxBindings* pBindings,
                                          SfxChildWinInfo* pInfo ) :
    SfxChildWindow( _pParent, nId )
{
    VclPtr<SvxColorDockingWindow> pWin = VclPtr<SvxColorDockingWindow>::Create( pBindings, this,
                                        _pParent );

    SetWindow(pWin);

    SetAlignment(SfxChildAlignment::BOTTOM);

    pWin->Initialize( pInfo );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
