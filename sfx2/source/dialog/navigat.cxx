/*************************************************************************
 *
 *  $RCSfile: navigat.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:31 $
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

// includes *******************************************************************

#pragma hdrstop

#include "navigat.hxx"
#include "sfx.hrc"
#include "app.hxx"
#include "sfxresid.hxx"
#include "helpid.hrc"
#include <segmentc.hxx>

SEG_EOFGLOBALS();

SFX_IMPL_DOCKINGWINDOW( SfxNavigatorWrapper , SID_NAVIGATOR );

SfxNavigatorWrapper::SfxNavigatorWrapper( Window* pParent ,
                                                USHORT nId ,
                                                SfxBindings* pBindings ,
                                                SfxChildWinInfo* pInfo )
                    : SfxChildWindow( pParent , nId )
{
    pWindow = new SfxNavigator( pBindings, this, pParent, WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pWindow->SetHelpId ( HID_NAVIGATOR_WINDOW );
    ((SfxDockingWindow*)pWindow)->SetFloatingSize( Size( 240, 240 ) );
    pWindow->SetSizePixel( Size( 240, 240 ) );

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


