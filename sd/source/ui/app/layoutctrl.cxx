/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tmplctrl.cxx,v $
 *
 * $Revision: 1.3 $
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

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/menu.hxx>
#include <vcl/status.hxx>
#include <vcl/toolbox.hxx>

#include <svl/languageoptions.hxx>
#include <svl/style.hxx>
#include <svl/stritem.hxx>
#include <svtools/miscopt.hxx>

#include <sfx2/dispatch.hxx>

#include <svx/toolbarmenu.hxx>

#include "layoutdialog.hxx"
#include "sdresid.hxx"
#include "res_bmp.hrc"
#include "strings.hrc"
#include "layoutctrl.hxx"
#include "ViewShellBase.hxx"
#include "drawdoc.hxx"
#include "sdattr.hrc"
#include "app.hrc"
#include "glob.hrc"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

SFX_IMPL_TOOLBOX_CONTROL( SdLayoutControl, SfxVoidItem );

// class SdLayoutControl ------------------------------------------

SdLayoutControl::SdLayoutControl( USHORT _nSlotId, USHORT _nId, ToolBox& rTbx )
: SfxToolBoxControl( _nSlotId, _nId, rTbx )
{
    rTbx.SetItemBits( _nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( _nId ) );
    rTbx.Invalidate();
}

// -----------------------------------------------------------------------

SdLayoutControl::~SdLayoutControl()
{
}

// -----------------------------------------------------------------------

void SdLayoutControl::Select( BOOL bMod1 )
{
    (void)bMod1;
}

// -----------------------------------------------------------------------

void SdLayoutControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    SfxToolBoxControl::StateChanged( nSID, eState, pState );
}

// -----------------------------------------------------------------------

SfxPopupWindowType SdLayoutControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SdLayoutControl::CreatePopupWindow()
{
    Window* pWin = 0;

    ToolBox& rTbx = GetToolBox();
    sd::ViewShellBase* pViewShellBase = sd::ViewShellBase::GetViewShellBase( SfxViewFrame::Current() );
    if( pViewShellBase )
    {
        pWin = pWin = new sd::SdLayoutDialogContent( *pViewShellBase, &rTbx );
//          pWin->StartPopupMode( &rTbx, TRUE );
        if( pWin )
        {
            StartPopupMode( pWin );
/*
            pWin->EnableDocking(true);
            Window::GetDockingManager()->StartPopupMode( &rTbx, pWin );

            pWin->AddEventListener( LINK( this, SdLayoutControl, WindowEventListener ) );
*/
        }
     }

//    SetPopupWindow( pWin );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SdLayoutControl, WindowEventListener, VclSimpleEvent*, pEvent )
{
    VclWindowEvent* pWindowEvent = dynamic_cast< VclWindowEvent* >( pEvent );
    if ( pWindowEvent && pWindowEvent->GetWindow() )
    {
        Window* pWindow = pWindowEvent->GetWindow();
        switch( pWindowEvent->GetId() )
        {
        case VCLEVENT_WINDOW_ENDPOPUPMODE:
            {
                EndPopupModeData *pData = static_cast<EndPopupModeData*>(pWindowEvent->GetData());
                if( pData && pData->mbTearoff );
                {
                    pWindow->SetPosPixel( Point( pData->maFloatingPos.X(), pData->maFloatingPos.Y() ) );
                    pWindow->Show();
                }
            }
            // fall through!
        case VCLEVENT_WINDOW_CLOSE:
        case VCLEVENT_OBJECT_DYING:
            pWindow->RemoveEventListener( LINK( this, SdLayoutControl, WindowEventListener ) );
//          static_cast<SfxPopupWindow*>(pWindow)->PopupModeEnd();
            break;
        }
    }
    return 0;
}
