/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <svtools/itemdel.hxx>
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

typedef std::vector<SfxItemDesruptor_Impl*> SfxItemDesruptorList_Impl;

static SfxItemDesruptorList_Impl *pItemDesruptList = NULL;

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

    // und in Liste eintragen (damit geflusht werden kann)
    SfxItemDesruptorList_Impl* &rpList = pItemDesruptList;
    if ( !rpList )
        rpList = new SfxItemDesruptorList_Impl;
    SfxItemDesruptor_Impl *pThis = this;
    rpList->push_back( pThis );
}

// ------------------------------------------------------------------------
SfxItemDesruptor_Impl::~SfxItemDesruptor_Impl()
{
    DBG_DTOR(SfxItemDesruptor_Impl, 0);

    // aus Idle-Handler austragen
    GetpApp()->RemoveIdleHdl( aLink );

    // und aus Liste austragen
    SfxItemDesruptorList_Impl* &rpList = pItemDesruptList;
    DBG_ASSERT( rpList, "no DesruptorList" );
    const SfxItemDesruptor_Impl *pThis = this;
    if ( rpList ) HACK(warum?)
        rpList->erase( std::find( rpList->begin(), rpList->end(), pThis ) );

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
