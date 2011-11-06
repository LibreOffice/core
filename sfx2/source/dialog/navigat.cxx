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
#include "precompiled_sfx2.hxx"

// includes *******************************************************************

#ifndef GCC
#endif

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
    DBG_ASSERT( pCon, "Kein Context!" );
    if ( pCon )
        pCon->Resizing( rSize );
}

sal_Bool SfxNavigator::Close()
{
    SfxChildWindowContext *pCon = GetChildWindow_Impl()->GetContext_Impl();
    DBG_ASSERT( pCon, "Kein Context!" );
    if ( !pCon || pCon->Close() )
        return SfxDockingWindow::Close();
    else
        return sal_False;
}

