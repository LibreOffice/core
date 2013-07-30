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

#include <tools/shl.hxx>
#include <vcl/status.hxx>
#include <svl/eitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>

#include <svx/dialogs.hrc>

#include "svx/insctrl.hxx"
#include <svx/dialmgr.hxx>

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
        DBG_ASSERT( pState->ISA( SfxBoolItem ), "invalid item type" );
        SfxBoolItem* pItem = (SfxBoolItem*)pState;
        bInsert = pItem->GetValue();

        if ( bInsert )
            GetStatusBar().SetQuickHelpText( GetId(), SVX_RESSTR( RID_SVXSTR_INSERT_HELPTEXT ) );
        else
            GetStatusBar().SetQuickHelpText( GetId(), SVX_RESSTR( RID_SVXSTR_OVERWRITE_HELPTEXT ) );

        DrawItemText_Impl();
    }
}

// -----------------------------------------------------------------------

void SvxInsertStatusBarControl::Click()
{
    if ( GetStatusBar().GetItemText( GetId() ).isEmpty() )
        return;
    bInsert = !bInsert;
    SfxBoolItem aIns( GetSlotId(), bInsert );

    ::com::sun::star::uno::Any a;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name = OUString( "InsertMode" );
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
    String aText;
    if ( !bInsert )
        aText = SVX_RESSTR( RID_SVXSTR_OVERWRITE_TEXT );

    GetStatusBar().SetItemText( GetId(), aText );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
