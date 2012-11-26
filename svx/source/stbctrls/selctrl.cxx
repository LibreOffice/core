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
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/urlobj.hxx>

#define _SVX_SELCTRL_CXX

#include "svx/selctrl.hxx"
#include <svx/dialmgr.hxx>

#include <svx/dialogs.hrc>

#define PAINT_OFFSET    5

SFX_IMPL_STATUSBAR_CONTROL(SvxSelectionModeControl, SfxUInt16Item);

// class SvxSelectionModeControl -----------------------------------------

SvxSelectionModeControl::SvxSelectionModeControl( sal_uInt16 _nSlotId,
                                                  sal_uInt16 _nId,
                                                  StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    nState( 0 )
{
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::StateChanged( sal_uInt16, SfxItemState eState,
                                            const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState )
        GetStatusBar().SetItemText( GetId(), String() );
    else
    {
        DBG_ASSERT( dynamic_cast< const SfxUInt16Item* >(pState), "invalid item type" );
        SfxUInt16Item* pItem = (SfxUInt16Item*)pState;
        nState = pItem->GetValue();
        DrawItemText_Impl();
    }
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::Click()
{
    if ( !GetStatusBar().GetItemText( GetId() ).Len() )
        return;
    nState++;
    if ( nState > 3 )
        nState = 0;

    ::com::sun::star::uno::Any a;
    SfxUInt16Item aState( GetSlotId(), nState );
    INetURLObject aObj( m_aCommandURL );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name  = aObj.GetURLPath();
    aState.QueryValue( a );
    aArgs[0].Value = a;

    execute( aArgs );
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::Paint( const UserDrawEvent& )
{
    DrawItemText_Impl();
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::DrawItemText_Impl()
{
    String sTxt;
    sal_uInt16 _nId = 0;

    switch ( nState )
    {
        case 0:
            _nId = RID_SVXSTR_SELMODE_STD;
            break;
        case 1:
            _nId = RID_SVXSTR_SELMODE_ER;
            break;
        case 2:
            _nId = RID_SVXSTR_SELMODE_ERG;
            break;
        case 3:
            _nId = RID_SVXSTR_SELMODE_BLK;
            break;
        default: DBG_ERROR( "invalid selection mode!" );
    }

    if ( _nId )
        sTxt = SVX_RESSTR( _nId );
    GetStatusBar().SetItemText( GetId(), sTxt );
}

sal_uIntPtr SvxSelectionModeControl::GetDefItemWidth(const StatusBar& rStb)
{
    long nWidth1 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_STD));
    long nWidth2 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_ER));
    long nWidth3 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_ERG));
    long nWidth4 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_BLK));

    if(nWidth1<nWidth2)
        nWidth1=nWidth2;

    if(nWidth1<nWidth3)
        nWidth1=nWidth3;

    if(nWidth1<nWidth4)
        nWidth1=nWidth4;

    return nWidth1+PAINT_OFFSET;
}


