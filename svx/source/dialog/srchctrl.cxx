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
#include <tools/pstm.hxx>
#include <svl/intitem.hxx>
#include <sfx2/objsh.hxx>

#include <svx/svxids.hrc>

#define _SVX_SRCHDLG_CXX // damit private-Methoden vom SrchDlg bekannt sind



#include "srchctrl.hxx"
#include "svx/srchdlg.hxx"
#include <svl/srchitem.hxx>

// class SvxSearchFamilyControllerItem -----------------------------------

SvxSearchController::SvxSearchController
(
    sal_uInt16 _nId,
    SfxBindings& rBind,
    SvxSearchDialog& rDlg
) :
    SfxControllerItem( _nId, rBind ),

    rSrchDlg( rDlg )
{
}

// -----------------------------------------------------------------------

void SvxSearchController::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                        const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE == eState )
    {
        if ( SID_STYLE_FAMILY1 <= nSID && nSID <= SID_STYLE_FAMILY4 )
        {
            SfxObjectShell* pShell = SfxObjectShell::Current();

            if ( pShell && pShell->GetStyleSheetPool() )
                rSrchDlg.TemplatesChanged_Impl( *pShell->GetStyleSheetPool() );
        }
        else if ( SID_SEARCH_OPTIONS == nSID )
        {
            const SfxUInt16Item* pSfxUInt16Item = dynamic_cast< const SfxUInt16Item* >(pState);
            DBG_ASSERT( pSfxUInt16Item, "wrong item type" );
            sal_uInt16 nFlags = (sal_uInt16 )pSfxUInt16Item->GetValue();
            rSrchDlg.EnableControls_Impl( nFlags );
        }
        else if ( SID_SEARCH_ITEM == nSID )
        {
            const SvxSearchItem* pSvxSearchItem = dynamic_cast< const SvxSearchItem* >(pState);
            DBG_ASSERT( pSvxSearchItem, "wrong item type" );
            rSrchDlg.SetItem_Impl( pSvxSearchItem );
        }
    }
    else if ( SID_SEARCH_OPTIONS == nSID || SID_SEARCH_ITEM == nSID )
        rSrchDlg.EnableControls_Impl( 0 );
}


