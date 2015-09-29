/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svl/intitem.hxx>
#include <sfx2/objsh.hxx>

#include <svx/svxids.hrc>

#include "srchctrl.hxx"
#include "svx/srchdlg.hxx"
#include <svl/srchitem.hxx>

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



void SvxSearchController::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                        const SfxPoolItem* pState )
{
    if ( SfxItemState::DEFAULT == eState )
    {
        if ( SID_STYLE_FAMILY1 <= nSID && nSID <= SID_STYLE_FAMILY4 )
        {
            SfxObjectShell* pShell = SfxObjectShell::Current();

            if ( pShell && pShell->GetStyleSheetPool() )
                rSrchDlg.TemplatesChanged_Impl( *pShell->GetStyleSheetPool() );
        }
        else if ( SID_SEARCH_OPTIONS == nSID )
        {
            DBG_ASSERT( dynamic_cast<const SfxUInt16Item* >(pState) !=  nullptr, "wrong item type" );
            SearchOptionFlags nFlags = (SearchOptionFlags) static_cast<const SfxUInt16Item*>(pState)->GetValue();
            rSrchDlg.EnableControls_Impl( nFlags );
        }
        else if ( SID_SEARCH_ITEM == nSID )
        {
            DBG_ASSERT( dynamic_cast<const SvxSearchItem*>( pState) !=  nullptr, "wrong item type" );
            rSrchDlg.SetItem_Impl( static_cast<const SvxSearchItem*>(pState) );
        }
    }
    else if ( SID_SEARCH_OPTIONS == nSID || SID_SEARCH_ITEM == nSID )
        rSrchDlg.EnableControls_Impl( SearchOptionFlags::NONE );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
