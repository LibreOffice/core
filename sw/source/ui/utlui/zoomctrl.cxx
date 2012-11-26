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
#include "precompiled_sw.hxx"



#include "hintids.hxx"

#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#include <sfx2/app.hxx>
#include <svl/stritem.hxx>
#include <svx/zoomitem.hxx>

#include "swtypes.hxx"
#include "zoomctrl.hxx"


SFX_IMPL_STATUSBAR_CONTROL( SwZoomControl, SvxZoomItem );


SwZoomControl::SwZoomControl( sal_uInt16 _nSlotId,
                              sal_uInt16 _nId,
                              StatusBar& rStb ) :
    SvxZoomStatusBarControl( _nSlotId, _nId, rStb )
{
}

// -----------------------------------------------------------------------

SwZoomControl::~SwZoomControl()
{
}

// -----------------------------------------------------------------------

void SwZoomControl::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState )
{
    if(SFX_ITEM_AVAILABLE == eState && dynamic_cast< const SfxStringItem* >(pState))
    {
        sPreviewZoom = ((const SfxStringItem*)pState)->GetValue();
        GetStatusBar().SetItemText( GetId(), sPreviewZoom );
    }
    else
    {
        sPreviewZoom = aEmptyStr;
        SvxZoomStatusBarControl::StateChanged(nSID, eState, pState);
    }
}

// -----------------------------------------------------------------------

void SwZoomControl::Paint( const UserDrawEvent& rUsrEvt )
{
    if(!sPreviewZoom.Len())
        SvxZoomStatusBarControl::Paint(rUsrEvt);
    else
        GetStatusBar().SetItemText( GetId(), sPreviewZoom );
}

// -----------------------------------------------------------------------

void SwZoomControl::Command( const CommandEvent& rCEvt )
{
    if(!sPreviewZoom.Len())
        SvxZoomStatusBarControl::Command(rCEvt);
}




