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

#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#include <svl/eitem.hxx>
#include <sfx2/app.hxx>

#define _SVX_MODCTRL_CXX

#include <svx/dialogs.hrc>

#include "svx/modctrl.hxx"
#include <svx/dialmgr.hxx>

SFX_IMPL_STATUSBAR_CONTROL(SvxModifyControl, SfxBoolItem);

// class SvxModifyControl ------------------------------------------------

SvxModifyControl::SvxModifyControl( sal_uInt16 _nSlotId,
                                    sal_uInt16 _nId,
                                    StatusBar& rStb ) :

    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    bState( sal_True )
{
}

// -----------------------------------------------------------------------

void SvxModifyControl::StateChanged( sal_uInt16, SfxItemState eState,
                                     const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState )
        GetStatusBar().SetItemText( GetId(), String() );
    else
    {
        DBG_ASSERT( dynamic_cast< const SfxBoolItem* >(pState), "invalid item type" );
        SfxBoolItem* pItem = (SfxBoolItem*)pState;
        bState = pItem->GetValue();
        DrawItemText_Impl();
    }
}

// -----------------------------------------------------------------------

void SvxModifyControl::Paint( const UserDrawEvent& )
{
    DrawItemText_Impl();
}

// -----------------------------------------------------------------------

void SvxModifyControl::DrawItemText_Impl()
{
    String sMode;

    if ( bState )
        sMode = '*';
    GetStatusBar().SetItemText( GetId(), sMode );
}

sal_uIntPtr SvxModifyControl::GetDefItemWidth(const StatusBar& rStb)
{
    return rStb.GetTextWidth(String::CreateFromAscii("XX"));
}


