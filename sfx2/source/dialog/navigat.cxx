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
#include "precompiled_sfx2.hxx"

// includes *******************************************************************

#include <sfx2/navigat.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/app.hxx>
#include "sfx2/sfxresid.hxx"
#include "helpid.hrc"

SFX_IMPL_DOCKINGWINDOW( SfxNavigatorWrapper , SID_NAVIGATOR );

SfxNavigatorWrapper::SfxNavigatorWrapper( Window* pParentWnd ,
                                                sal_uInt16 nId ,
                                                SfxBindings* pBindings ,
                                                SfxChildWinInfo* pInfo )
                    : SfxChildWindow( pParentWnd , nId )
{
    pWindow = new SfxNavigator( pBindings, this, pParentWnd,
        WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK | WB_ROLLABLE);
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pWindow->SetHelpId ( HID_NAVIGATOR_WINDOW );
    pWindow->SetOutputSizePixel( Size( 270, 240 ) );

    ( ( SfxDockingWindow* ) pWindow )->Initialize( pInfo );
    SetHideNotDelete( sal_True );
}

SfxNavigator::SfxNavigator( SfxBindings* pBind ,
                            SfxChildWindow* pChildWin ,
                            Window* pParent ,
                            WinBits nBits )
                        : SfxDockingWindow( pBind ,
                                            pChildWin ,
                                            pParent ,
                                            nBits )
                        , pWrapper( pChildWin )
{
    SetText( String( SfxResId( SID_NAVIGATOR ) ) );
}

void SfxNavigator::Resize()
{
    SfxDockingWindow::Resize();
    if ( pWrapper->GetContextWindow() )
        pWrapper->GetContextWindow()->SetSizePixel( GetOutputSizePixel() );
}

void SfxNavigator::Resizing( Size &rSize )
{
    SfxDockingWindow::Resizing( rSize );

    SfxChildWindowContext *pCon = GetChildWindow_Impl()->GetContext_Impl();
    DBG_ASSERT( pCon, "No Context!" );
    if ( pCon )
        pCon->Resizing( rSize );
}

sal_Bool SfxNavigator::Close()
{
    SfxChildWindowContext *pCon = GetChildWindow_Impl()->GetContext_Impl();
    DBG_ASSERT( pCon, "No Context!" );
    if ( !pCon || pCon->Close() )
        return SfxDockingWindow::Close();
    else
        return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
