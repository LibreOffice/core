/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stbcontrl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:13:48 $
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

// include ---------------------------------------------------------------

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
//#ifndef _SFXITEMPOOL_HXX
//#include <svtools/itempool.hxx>
//#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif

#include <svtools/eitem.hxx>


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
    FunctionPopup_Impl( USHORT nCheck );

    USHORT          GetSelected() const { return nSelected; }

private:
    USHORT          nSelected;

    virtual void    Select();
};

// -----------------------------------------------------------------------

FunctionPopup_Impl::FunctionPopup_Impl( USHORT nCheck ) :
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


XmlSecStatusBarControl::XmlSecStatusBarControl( USHORT _nId, StatusBar& _rStb, SfxBindings& _rBind )
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

void XmlSecStatusBarControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
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

