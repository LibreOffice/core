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
#include "precompiled_xmlsecurity.hxx"

// include ---------------------------------------------------------------
#include <tools/shl.hxx>
#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#include <vcl/image.hxx>
//#ifndef _SFXITEMPOOL_HXX
//#include <svl/itempool.hxx>
//#endif
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>

#include <svl/eitem.hxx>


#include <xmlsecurity/stbcontrl.hxx>

#define PAINT_OFFSET    5

//#include "sizeitem.hxx"
//#include "dialmgr.hxx"
//#include "dlgutil.hxx"
//#include "stbctrls.h"

//#include "dialogs.hrc"

/*#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif*/



SFX_IMPL_STATUSBAR_CONTROL( XmlSecStatusBarControl, SfxBoolItem );

/*
class FunctionPopup_Impl : public PopupMenu
{
public:
    FunctionPopup_Impl( sal_uInt16 nCheck );

    sal_uInt16          GetSelected() const { return nSelected; }

private:
    sal_uInt16          nSelected;

    virtual void    Select();
};

// -----------------------------------------------------------------------

FunctionPopup_Impl::FunctionPopup_Impl( sal_uInt16 nCheck ) :
    PopupMenu( ResId( RID_SVXMNU_PSZ_FUNC, DIALOG_MGR() ) ),
    nSelected( 0 )
{
    if (nCheck)
        CheckItem( nCheck );
}

// -----------------------------------------------------------------------

void FunctionPopup_Impl::Select()
{
    nSelected = GetCurItemId();
}
*/



struct XmlSecStatusBarControl::XmlSecStatusBarControl_Impl
{
    Point       maPos;
    Size        maSize;
    bool        mbSigned;
    Image       maImage;
};


XmlSecStatusBarControl::XmlSecStatusBarControl( sal_uInt16 _nId, StatusBar& _rStb, SfxBindings& _rBind )
    :SfxStatusBarControl( _nId, _rStb, _rBind )

    ,mpImpl( new XmlSecStatusBarControl_Impl )
{
    mpImpl->mbSigned = false;
//  pImp->maImage = Image( ResId( RID_SVXBMP_POSITION, DIALOG_MGR() ) );
}

XmlSecStatusBarControl::~XmlSecStatusBarControl()
{
    delete mpImpl;
}

void XmlSecStatusBarControl::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    GetStatusBar().SetHelpText( GetId(), String() );    // necessary ?
    GetStatusBar().SetHelpId( GetId(), nSID );          // necessary ?

    if( SFX_ITEM_AVAILABLE != eState )
    {
        mpImpl->mbSigned = false;
    }
    else if( pState->ISA( SfxBoolItem ) )
    {
        mpImpl->mbSigned = ( ( SfxBoolItem* ) pState )->GetValue();
    }
    else
    {
        DBG_ERRORFILE( "+XmlSecStatusBarControl::StateChanged(): invalid item type" );
        mpImpl->mbSigned = false;
    }

    if( GetStatusBar().AreItemsVisible() )              // necessary ?
        GetStatusBar().SetItemData( GetId(), 0 );

    GetStatusBar().SetItemText( GetId(), String() );    // necessary ?
}

void XmlSecStatusBarControl::Command( const CommandEvent& rCEvt )
{
    // can / has to be done when integrated in Office!
//  if( rCEvt.GetCommand() == .... )
    if( false )
    {
//      GetBindings().GetDispatcher()->Execute( SID_PSZ_FUNCTION, SFX_CALLMODE_RECORD, &aItem, 0L );
    }
    else
        SfxStatusBarControl::Command( rCEvt );
}

void XmlSecStatusBarControl::Paint( const UserDrawEvent& rUsrEvt )
{
    OutputDevice*       pDev = rUsrEvt.GetDevice();
    DBG_ASSERT( pDev, "-XmlSecStatusBarControl::Paint(): no Output Device... this will lead to nirvana..." );
    const Rectangle&    rRect = rUsrEvt.GetRect();
    StatusBar&          rBar = GetStatusBar();
    Point               aItemPos = rBar.GetItemTextPos( GetId() );
    Color               aOldLineColor = pDev->GetLineColor();
    Color               aOldFillColor = pDev->GetFillColor();

    // just 4 testing until we've got a bitmap
    pDev->SetLineColor();
    pDev->SetFillColor( pDev->GetBackground().GetColor() );

    String              s( String::CreateFromAscii( mpImpl->mbSigned? "X" : "-" ) );
    pDev->DrawRect( rRect );
    pDev->DrawText( Point( rRect.Left() + rRect.GetWidth() / 2 - pDev->GetTextWidth( s ) / 2, aItemPos.Y() ), s );

    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}

