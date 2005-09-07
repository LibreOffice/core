/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: navigat.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:26:49 $
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

// includes *******************************************************************

#ifndef GCC
#pragma hdrstop
#endif

#include "navigat.hxx"
#include "sfx.hrc"
#include "app.hxx"
#include "sfxresid.hxx"
#include "helpid.hrc"

SFX_IMPL_DOCKINGWINDOW( SfxNavigatorWrapper , SID_NAVIGATOR );

SfxNavigatorWrapper::SfxNavigatorWrapper( Window* pParent ,
                                                USHORT nId ,
                                                SfxBindings* pBindings ,
                                                SfxChildWinInfo* pInfo )
                    : SfxChildWindow( pParent , nId )
{
    pWindow = new SfxNavigator( pBindings, this, pParent,
        WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK | WB_ROLLABLE);
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pWindow->SetHelpId ( HID_NAVIGATOR_WINDOW );
    pWindow->SetOutputSizePixel( Size( 270, 240 ) );

    ( ( SfxDockingWindow* ) pWindow )->Initialize( pInfo );
    SetHideNotDelete( TRUE );
}

SfxNavigator::SfxNavigator( SfxBindings* pBindings ,
                                        SfxChildWindow* pChildWin ,
                                        Window* pParent ,
                                        WinBits nBits )
                        : SfxDockingWindow( pBindings ,
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
    DBG_ASSERT( pCon, "Kein Context!" );
    if ( pCon )
        pCon->Resizing( rSize );
}

BOOL SfxNavigator::Close()
{
    SfxChildWindowContext *pCon = GetChildWindow_Impl()->GetContext_Impl();
    DBG_ASSERT( pCon, "Kein Context!" );
    if ( !pCon || pCon->Close() )
        return SfxDockingWindow::Close();
    else
        return FALSE;
}

