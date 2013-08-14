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


#include <itemdel.hxx>
#include <vcl/svapp.hxx>
#include <tools/errcode.hxx>
#include <limits.h>
#include <vector>

#include <svl/itempool.hxx>

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxItemDesruptor_Impl);

// -----------------------------------------------------------------------

class SfxItemDesruptor_Impl
{
    SfxPoolItem *pItem;
    Link         aLink;

private:
                 DECL_LINK( Delete, void * );
                 SfxItemDesruptor_Impl( const SfxItemDesruptor_Impl& ); // n.i.

public:
                 SfxItemDesruptor_Impl( SfxPoolItem *pItemToDesrupt );
                 ~SfxItemDesruptor_Impl();
};

// ------------------------------------------------------------------------
SfxItemDesruptor_Impl::SfxItemDesruptor_Impl( SfxPoolItem *pItemToDesrupt ):
    pItem(pItemToDesrupt),
    aLink( LINK(this, SfxItemDesruptor_Impl, Delete) )
{
    DBG_CTOR(SfxItemDesruptor_Impl, 0);

    DBG_ASSERT( 0 == pItem->GetRefCount(), "desrupting pooled item" );
    pItem->SetKind( SFX_ITEMS_DELETEONIDLE );

    // im Idle abarbeiten
    GetpApp()->InsertIdleHdl( aLink, 1 );
}

// ------------------------------------------------------------------------
SfxItemDesruptor_Impl::~SfxItemDesruptor_Impl()
{
    DBG_DTOR(SfxItemDesruptor_Impl, 0);

    // aus Idle-Handler austragen
    GetpApp()->RemoveIdleHdl( aLink );

    // reset RefCount (was set to SFX_ITEMS_SPECIAL before!)
    pItem->SetRefCount( 0 );
    //DBG_CHKOBJ( pItem, SfxPoolItem, 0 );
    delete pItem;
}

// ------------------------------------------------------------------------
IMPL_LINK_NOARG(SfxItemDesruptor_Impl, Delete)
{
    {DBG_CHKTHIS(SfxItemDesruptor_Impl, 0);}
    delete this;
    return 0;
}

// ------------------------------------------------------------------------
SfxPoolItem* DeleteItemOnIdle( SfxPoolItem* pItem )
{
    DBG_ASSERT( 0 == pItem->GetRefCount(), "deleting item in use" );
    new SfxItemDesruptor_Impl( pItem );
    return pItem;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
