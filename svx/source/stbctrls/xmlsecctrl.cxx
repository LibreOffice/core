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
#include "precompiled_svx.hxx"

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
#include <sfx2/signaturestate.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>

#include <svl/intitem.hxx>

#include <svl/eitem.hxx>

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include "svx/xmlsecctrl.hxx"
#include <tools/urlobj.hxx>

#define PAINT_OFFSET    5

//#include <editeng/sizeitem.hxx>
//#include <svx/dialmgr.hxx>
//#include "svx/dlgutil.hxx"
//#include "stbctrls.h"

//#include <svx/dialogs.hrc>

/*#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif*/



SFX_IMPL_STATUSBAR_CONTROL( XmlSecStatusBarControl, SfxUInt16Item );

struct XmlSecStatusBarControl::XmlSecStatusBarControl_Impl
{
    Point       maPos;
    Size        maSize;
    sal_uInt16      mnState;
    Image       maImage;
    Image       maImageBroken;
    Image       maImageNotValidated;
};


XmlSecStatusBarControl::XmlSecStatusBarControl( sal_uInt16 _nSlotId,  sal_uInt16 _nId, StatusBar& _rStb )
    :SfxStatusBarControl( _nSlotId, _nId, _rStb )

    ,mpImpl( new XmlSecStatusBarControl_Impl )
{
    mpImpl->mnState = (sal_uInt16)SIGNATURESTATE_UNKNOWN;

    sal_Bool bHC = GetStatusBar().GetSettings().GetStyleSettings().GetHighContrastMode();
    mpImpl->maImage = Image( SVX_RES( bHC ? RID_SVXBMP_SIGNET_H : RID_SVXBMP_SIGNET ) );
    mpImpl->maImageBroken =
        Image( SVX_RES( bHC ? RID_SVXBMP_SIGNET_BROKEN_H : RID_SVXBMP_SIGNET_BROKEN ) );
    mpImpl->maImageNotValidated =
        Image( SVX_RES( bHC ? RID_SVXBMP_SIGNET_NOTVALIDATED_H : RID_SVXBMP_SIGNET_NOTVALIDATED ) );
}

XmlSecStatusBarControl::~XmlSecStatusBarControl()
{
    delete mpImpl;
}

void XmlSecStatusBarControl::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )
{
    if( SFX_ITEM_AVAILABLE != eState )
    {
        mpImpl->mnState = (sal_uInt16)SIGNATURESTATE_UNKNOWN;
    }
    else if( pState->ISA( SfxUInt16Item ) )
    {
//      mpImpl->mbSigned = ( ( SfxUInt16Item* ) pState )->GetValue() == 1 /* SIGNED*/ ;
        mpImpl->mnState = ( ( SfxUInt16Item* ) pState )->GetValue();
    }
    else
    {
        DBG_ERRORFILE( "+XmlSecStatusBarControl::StateChanged(): invalid item type" );
        mpImpl->mnState = (sal_uInt16)SIGNATURESTATE_UNKNOWN;
    }

    if( GetStatusBar().AreItemsVisible() )              // necessary ?
        GetStatusBar().SetItemData( GetId(), 0 );

    GetStatusBar().SetItemText( GetId(), String() );    // necessary ?

    sal_uInt16 nResId = RID_SVXSTR_XMLSEC_NO_SIG;
    if ( mpImpl->mnState == SIGNATURESTATE_SIGNATURES_OK )
        nResId = RID_SVXSTR_XMLSEC_SIG_OK;
    else if ( mpImpl->mnState == SIGNATURESTATE_SIGNATURES_BROKEN )
        nResId = RID_SVXSTR_XMLSEC_SIG_NOT_OK;
    else if ( mpImpl->mnState == SIGNATURESTATE_SIGNATURES_NOTVALIDATED )
        nResId = RID_SVXSTR_XMLSEC_SIG_OK_NO_VERIFY;
    else if ( mpImpl->mnState == SIGNATURESTATE_SIGNATURES_PARTIAL_OK )
        nResId = RID_SVXSTR_XMLSEC_SIG_CERT_OK_PARTIAL_SIG;

    GetStatusBar().SetQuickHelpText( GetId(), SVX_RESSTR( nResId ) );
}

void XmlSecStatusBarControl::Command( const CommandEvent& rCEvt )
{
    if( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        PopupMenu aPopupMenu( ResId( RID_SVXMNU_XMLSECSTATBAR, DIALOG_MGR() ) );
        if( aPopupMenu.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel() ) )
        {
            ::com::sun::star::uno::Any a;
            SfxUInt16Item aState( GetSlotId(), 0 );
            INetURLObject aObj( m_aCommandURL );

            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
            aArgs[0].Name  = aObj.GetURLPath();
            aState.QueryValue( a );
            aArgs[0].Value = a;

            execute( aArgs );
        }
    }
    else
        SfxStatusBarControl::Command( rCEvt );
}

void XmlSecStatusBarControl::Paint( const UserDrawEvent& rUsrEvt )
{
    OutputDevice*       pDev = rUsrEvt.GetDevice();
    DBG_ASSERT( pDev, "-XmlSecStatusBarControl::Paint(): no Output Device... this will lead to nirvana..." );
    Rectangle           aRect = rUsrEvt.GetRect();
    StatusBar&          rBar = GetStatusBar();
    Point               aItemPos = rBar.GetItemTextPos( GetId() );
    Color               aOldLineColor = pDev->GetLineColor();
    Color               aOldFillColor = pDev->GetFillColor();

    pDev->SetLineColor();
    pDev->SetFillColor( pDev->GetBackground().GetColor() );

    if( mpImpl->mnState == SIGNATURESTATE_SIGNATURES_OK )
    {
        ++aRect.Top();
        pDev->DrawImage( aRect.TopLeft(), mpImpl->maImage );
    }
    else if( mpImpl->mnState == SIGNATURESTATE_SIGNATURES_BROKEN )
    {
        ++aRect.Top();
        pDev->DrawImage( aRect.TopLeft(), mpImpl->maImageBroken );
    }
    else if( mpImpl->mnState == SIGNATURESTATE_SIGNATURES_NOTVALIDATED
        || mpImpl->mnState == SIGNATURESTATE_SIGNATURES_PARTIAL_OK)
    {
        ++aRect.Top();
        pDev->DrawImage( aRect.TopLeft(), mpImpl->maImageNotValidated );
    }
    else
        pDev->DrawRect( aRect );

    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}

long XmlSecStatusBarControl::GetDefItemWidth( StatusBar& )
{
    return 16;
}

