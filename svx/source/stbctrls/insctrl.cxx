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
#include <svl/eitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>

#define _SVX_INSCTRL_CXX

#include <svx/dialogs.hrc>

#include "svx/insctrl.hxx"
#include <svx/dialmgr.hxx>

#define PAINT_OFFSET    5

SFX_IMPL_STATUSBAR_CONTROL(SvxInsertStatusBarControl, SfxBoolItem);

// class SvxInsertStatusBarControl ---------------------------------------

SvxInsertStatusBarControl::SvxInsertStatusBarControl( sal_uInt16 _nSlotId,
                                                      sal_uInt16 _nId,
                                                      StatusBar& rStb ) :

    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    bInsert( sal_True )
{
}

// -----------------------------------------------------------------------

SvxInsertStatusBarControl::~SvxInsertStatusBarControl()
{
}

// -----------------------------------------------------------------------

void SvxInsertStatusBarControl::StateChanged( sal_uInt16 , SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState )
        GetStatusBar().SetItemText( GetId(), String() );
    else
    {
        DBG_ASSERT( dynamic_cast< const SfxBoolItem* >(pState), "invalid item type" );
        SfxBoolItem* pItem = (SfxBoolItem*)pState;
        bInsert = pItem->GetValue();
        DrawItemText_Impl();
    }
}

// -----------------------------------------------------------------------

void SvxInsertStatusBarControl::Click()
{
    if ( !GetStatusBar().GetItemText( GetId() ).Len() )
        return;
    bInsert = !bInsert;
    SfxBoolItem aIns( GetSlotId(), bInsert );

    ::com::sun::star::uno::Any a;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InsertMode" ));
    aIns.QueryValue( a );
    aArgs[0].Value = a;

    execute( aArgs );
}

// -----------------------------------------------------------------------

void SvxInsertStatusBarControl::Paint( const UserDrawEvent& )
{
    DrawItemText_Impl();
}

// -----------------------------------------------------------------------

void SvxInsertStatusBarControl::DrawItemText_Impl()
{
    sal_uInt16 _nId = RID_SVXSTR_OVERWRITE_TEXT;

    if ( bInsert )
        _nId = RID_SVXSTR_INSERT_TEXT;
    GetStatusBar().SetItemText( GetId(), SVX_RESSTR( _nId ) );
}

sal_uIntPtr SvxInsertStatusBarControl::GetDefItemWidth(const StatusBar& rStb)
{
    long nWidth1 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_OVERWRITE_TEXT));
    long nWidth2 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_INSERT_TEXT));

    if(nWidth1<nWidth2)
        nWidth1=nWidth2;

    return nWidth1+PAINT_OFFSET;
}


