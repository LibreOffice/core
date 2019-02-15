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

#include <sal/config.h>

#include <sfx2/bindings.hxx>
#include <sfx2/navigat.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <helpids.h>
#include <tools/debug.hxx>

SFX_IMPL_DOCKINGWINDOW( SfxNavigatorWrapper , SID_NAVIGATOR );

SfxNavigatorWrapper::SfxNavigatorWrapper( vcl::Window* pParentWnd ,
                                                sal_uInt16 nId ,
                                                SfxBindings* pBindings ,
                                                SfxChildWinInfo* pInfo )
                    : SfxChildWindow( pParentWnd , nId )
{
    SetWindow( VclPtr<SfxNavigator>::Create( pBindings, this, pParentWnd,
        WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK | WB_ROLLABLE) );

    GetWindow()->SetHelpId ( HID_NAVIGATOR_WINDOW );
    GetWindow()->SetOutputSizePixel( Size( 270, 240 ) );

    static_cast<SfxDockingWindow*>( GetWindow() )->Initialize( pInfo );
    SetHideNotDelete( true );
}

SfxNavigator::SfxNavigator( SfxBindings* pBind ,
                            SfxChildWindow* pChildWin ,
                            vcl::Window* pParent ,
                            WinBits nBits )
                        : SfxDockingWindow( pBind ,
                                            pChildWin ,
                                            pParent ,
                                            nBits )
                        , pWrapper( pChildWin )
{
    SetText(SfxResId(STR_SID_NAVIGATOR));
}

void SfxNavigator::Resize()
{
    SfxDockingWindow::Resize();
    if ( pWrapper->GetContextWindow() )
        pWrapper->GetContextWindow()->SetSizePixel( GetOutputSizePixel() );
}

bool SfxNavigator::Close()
{
    DBG_ASSERT( GetChildWindow_Impl()->GetContext_Impl(), "No Context!" );
    return SfxDockingWindow::Close();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
