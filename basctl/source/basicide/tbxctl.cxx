/*************************************************************************
 *
 *  $RCSfile: tbxctl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2000-09-29 11:02:37 $
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


#include <ide_pch.hxx>

#pragma hdrstop

#define _BASIDE_POPUPWINDOWTBX
#include <tbxctl.hxx>
#include <svx/svxids.hrc>
#include <iderid.hxx>
#include <tbxctl.hrc>
#include <idetemp.hxx>

SFX_IMPL_TOOLBOX_CONTROL( TbxControls, SfxEnumItem )

/*************************************************************************
|*
|* WorkWindow Alignment
|*
\************************************************************************/

IMPL_LINK( PopupWindowTbx, SelectHdl, void*, EMPTYARG )
{
#ifdef VCL
    if ( IsInPopupMode() )
        EndPopupMode();
#endif

    aSelectLink.Call( &aTbx.GetToolBox() );

    return 0;
}

PopupWindowTbx::PopupWindowTbx( USHORT nId, ToolBoxAlign eAlign,
                                ResId aRIdWin, ResId aRIdTbx,
                                SfxBindings& rBind ) :
                SfxPopupWindow  ( nId, aRIdWin, rBind ),
                aTbx            ( this, GetBindings(), aRIdTbx )
{
    FreeResource();
    aTbx.Initialize();

    ToolBox& rBox = aTbx.GetToolBox();
    rBox.SetAlign( eAlign );
    if( eAlign == BOXALIGN_LEFT )
        SetText( String() );

    Size aSize = aTbx.CalcWindowSizePixel();
    rBox.SetSizePixel( aSize );
    SetOutputSizePixel( aSize );
    aSelectLink = rBox.GetSelectHdl();
    rBox.SetSelectHdl( LINK( this, PopupWindowTbx, SelectHdl ) );
}

SfxPopupWindow* PopupWindowTbx::Clone() const
{
    return new PopupWindowTbx( GetId(), aTbx.GetAlign(),
                        IDEResId( RID_TBXCONTROLS ),
                        IDEResId( RID_TOOLBOX ),
                        (SfxBindings&) GetBindings() );
}

void PopupWindowTbx::PopupModeEnd()
{
    aTbx.GetToolBox().EndSelection();
    SfxPopupWindow::PopupModeEnd();
}

void PopupWindowTbx::Update()
{
    ToolBox *pBox = &aTbx.GetToolBox();
    aTbx.Activate( pBox );
    aTbx.Deactivate( pBox );
}

PopupWindowTbx::~PopupWindowTbx()
{
}

/*************************************************************************
|*
|* Klasse fuer Toolbox
|*
\************************************************************************/

TbxControls::TbxControls( USHORT nId, ToolBox& rTbx, SfxBindings& rBind ) :
        SfxToolBoxControl( nId, rTbx, rBind )
{
    nLastSlot = USHRT_MAX;
}

/*************************************************************************
|*
|* Wenn man ein PopupWindow erzeugen will
|*
\************************************************************************/
SfxPopupWindowType TbxControls::GetPopupWindowType() const
{
    if( nLastSlot == USHRT_MAX )
        return(SFX_POPUPWINDOW_ONCLICK);
    return(SFX_POPUPWINDOW_ONTIMEOUT);
}

void TbxControls::StateChanged( USHORT nSID, SfxItemState eState,
  const SfxPoolItem* pState )
{
    if( pState )
    {
        SfxAllEnumItem* pItem = PTR_CAST(SfxAllEnumItem, pState);
        if( pItem )
        {
            USHORT nLastEnum = pItem->GetValue();
            USHORT nTemp = 0;
            switch( nLastEnum )
            {
                case SVX_SNAP_PUSHBUTTON:   nTemp = SID_INSERT_PUSHBUTTON; break;
                case SVX_SNAP_CHECKBOX:     nTemp = SID_INSERT_CHECKBOX; break;
                case SVX_SNAP_RADIOBUTTON:  nTemp = SID_INSERT_RADIOBUTTON; break;
                case SVX_SNAP_SPINBUTTON:   nTemp = SID_INSERT_SPINBUTTON; break;
                case SVX_SNAP_FIXEDTEXT:    nTemp = SID_INSERT_FIXEDTEXT; break;
                case SVX_SNAP_GROUPBOX:     nTemp = SID_INSERT_GROUPBOX; break;
                case SVX_SNAP_LISTBOX:      nTemp = SID_INSERT_LISTBOX; break;
                case SVX_SNAP_COMBOBOX:     nTemp = SID_INSERT_COMBOBOX; break;
                case SVX_SNAP_EDIT:         nTemp = SID_INSERT_EDIT; break;
                case SVX_SNAP_HSCROLLBAR:   nTemp = SID_INSERT_HSCROLLBAR; break;
                case SVX_SNAP_VSCROLLBAR:   nTemp = SID_INSERT_VSCROLLBAR; break;
                case SVX_SNAP_PREVIEW:      nTemp = SID_INSERT_PREVIEW; break;
                case SVX_SNAP_URLBUTTON:    nTemp = SID_INSERT_URLBUTTON; break;
            }
            if( nTemp )
            {
                Image aImage = SFX_IMAGEMANAGER()->GetImage( nTemp );
                GetToolBox().SetItemImage( SID_CHOOSE_CONTROLS, aImage );
                nLastSlot = nLastEnum;
            }
        }
    }
    SfxToolBoxControl::StateChanged( nSID, eState,pState );
}

void TbxControls::Select( USHORT nModifier )
{
    SfxAllEnumItem aItem( SID_CHOOSE_CONTROLS, nLastSlot );
    SfxViewFrame* pCurFrame = SfxViewFrame::Current();
    DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
    SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
    if( pDispatcher )
    {
        pDispatcher->Execute( SID_CHOOSE_CONTROLS, SFX_CALLMODE_SYNCHRON, &aItem, 0L );
    }
}

/*************************************************************************
|*
|* Hier wird das Fenster erzeugt
|* Lage der Toolbox mit GetToolBox() abfragbar
|* rItemRect sind die Screen-Koordinaten
|*
\************************************************************************/
SfxPopupWindow* TbxControls::CreatePopupWindow()
{
    if (GetId() == SID_CHOOSE_CONTROLS)
    {
        PopupWindowTbx *pWin =
            new PopupWindowTbx( GetId(),
                                GetToolBox().IsHorizontal() ?
                                    BOXALIGN_LEFT : BOXALIGN_TOP,
                                IDEResId( RID_TBXCONTROLS ),
                                IDEResId( RID_TOOLBOX ),
                                GetBindings() );
        pWin->StartPopupMode(&GetToolBox(), TRUE);
        pWin->Update();
        pWin->StartSelection();
        pWin->Show();
        return(pWin);
    }

    return(0);
}


