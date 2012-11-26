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
    else if( dynamic_cast< const SfxUInt16Item* >(pState) )
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

