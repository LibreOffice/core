/*************************************************************************
 *
 *  $RCSfile: tbxdrctl.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:21:11 $
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

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#include <tools/ref.hxx>
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _AEITEM_HXX
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#include <sfx2/imagemgr.hxx>
#include <sfx2/viewfrm.hxx>

#pragma hdrstop

#include "dialmgr.hxx"
#include "dialogs.hrc"

#include "tbxctl.hxx"
#include "tbxdraw.hxx"
#include "tbxdraw.hrc"

SFX_IMPL_TOOLBOX_CONTROL(SvxTbxCtlDraw, SfxAllEnumItem);

/*************************************************************************
|*
|*
|*
\************************************************************************/

SvxTbxCtlDraw::SvxTbxCtlDraw( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :

    SfxToolBoxControl( nSlotId, nId, rTbx ),
    nLastAction( 0 )

{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}

/*************************************************************************
|*
|* Benachrichtigung, wenn sich der Applikationsstatus geaendert hat
|*
\************************************************************************/

void SvxTbxCtlDraw::StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState )
{

    GetToolBox().EnableItem( GetId(), ( eState != SFX_ITEM_DISABLED ) );
    if ( SFX_ITEM_AVAILABLE == eState )
    {
        USHORT nTemp = ( (SfxEnumItem*)pState )->GetValue();

        if( GetSlotId() == SID_INSERT_DRAW && nTemp != USHRT_MAX )
        {
            // Check whether we are in high contrast mode or not!
            BOOL bHiContrast = GetToolBox().GetBackground().GetColor().IsDark();

            nLastAction = nTemp;

            USHORT nImage = nLastAction ? nLastAction : GetSlotId();
            rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
            aSlotURL += rtl::OUString::valueOf( sal_Int32( nImage ));
            Image aImage = GetImage( m_xFrame,
                                     aSlotURL,
                                     hasBigImages(),
                                     GetToolBox().GetDisplayBackground().GetColor().IsDark() );

//            SfxViewFrame* pFrame = GetBindings().GetDispatcher()->GetFrame();
//            Image aImage = pFrame->GetImageManager()->GetImage( nImage, bHiContrast );
            GetToolBox().SetItemImage( GetId(), aImage );
        }
    }

    SfxToolBoxControl::StateChanged( nSID, eState, pState );
}

/*************************************************************************
|*
|* Wenn man ein PopupWindow erzeugen will
|*
\************************************************************************/

SfxPopupWindowType SvxTbxCtlDraw::GetPopupWindowType() const
{
    return( nLastAction == 0 ? SFX_POPUPWINDOW_ONCLICK : SFX_POPUPWINDOW_ONTIMEOUT);
}

/*************************************************************************
|*
|* Hier wird das Fenster erzeugt
|* Lage der Toolbox mit GetToolBox() abfragbar
|* rItemRect sind die Screen-Koordinaten
|*
\************************************************************************/

SfxPopupWindow* SvxTbxCtlDraw::CreatePopupWindow()
{
    rtl::OUString aSubTbxResName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/drawbar" ));
    createAndPositionSubToolBar( aSubTbxResName );

/*
    if ( GetId() == SID_INSERT_DRAW )
    {
        SvxPopupWindowTbxMgr* pWin =
            new SvxPopupWindowTbxMgr( GetId(), this,
                                      SVX_RES( RID_SVXTBX_DRAW ),
                                      SVX_RES( TBX_DRAW ),
                                      GetBindings() );
        pWin->StartPopupMode( &GetToolBox(), TRUE );
        pWin->StartSelection();
        pWin->Show();
        return pWin;
    }
*/
    return NULL;
}

// -----------------------------------------------------------------------

void SvxTbxCtlDraw::Select( BOOL bMod1 )
{
    if ( nLastAction )
    {
        SfxDispatcher* pDisp = SfxViewFrame::Current() ? SfxViewFrame::Current()->GetDispatcher() : NULL;
        if ( pDisp )
            pDisp->Execute( nLastAction, SFX_CALLMODE_SLOT, NULL, ( bMod1 ? KEY_MOD1 : 0 ) );
/*
        GetBindings().GetDispatcher()->Execute( nLastAction,
            SFX_CALLMODE_SLOT, NULL, ( bMod1 ? KEY_MOD1 : 0 ) );
*/
        if ( bMod1 )
        {
            //  #99013# if selected with control key, return focus to current view
            Window* pShellWnd = SfxViewShell::Current()->GetWindow();
            if ( pShellWnd )
                pShellWnd->GrabFocus();
        }
    }
}

