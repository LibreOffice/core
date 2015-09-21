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

#include <sal/config.h>

#include <boost/noncopyable.hpp>
#include "itemdel.hxx"
#include <vcl/svapp.hxx>
#include <tools/errcode.hxx>
#include <limits.h>
#include <vector>

#include <svl/itempool.hxx>

class SfxItemDisruptor_Impl: private boost::noncopyable
{
    SfxPoolItem *           pItem;
    Link<Application*,void> aLink;

private:
    DECL_LINK_TYPED( Delete, Application*, void );

public:
    explicit SfxItemDisruptor_Impl(SfxPoolItem *pItemToDesrupt);
    void LaunchDeleteOnIdle();
    ~SfxItemDisruptor_Impl();
};

SfxItemDisruptor_Impl::SfxItemDisruptor_Impl( SfxPoolItem *pItemToDesrupt ):
    pItem(pItemToDesrupt),
    aLink( LINK(this, SfxItemDisruptor_Impl, Delete) )
{

    DBG_ASSERT( 0 == pItem->GetRefCount(), "disrupting pooled item" );
    pItem->SetKind( SFX_ITEMS_DELETEONIDLE );
}

void SfxItemDisruptor_Impl::LaunchDeleteOnIdle()
{
    // process in Idle
    Application::InsertIdleHdl( aLink, 1 );
}

SfxItemDisruptor_Impl::~SfxItemDisruptor_Impl()
{

    // remove from Idle-Handler
    Application::RemoveIdleHdl( aLink );

    // reset RefCount (was set to SFX_ITEMS_SPECIAL before!)
    pItem->SetRefCount( 0 );

    delete pItem;
}

IMPL_LINK_NOARG_TYPED(SfxItemDisruptor_Impl, Delete, Application*, void)
{
    delete this;
}

void DeleteItemOnIdle(SfxPoolItem* pItem)
{
    DBG_ASSERT( 0 == pItem->GetRefCount(), "deleting item in use" );
    SfxItemDisruptor_Impl *pDesruptor = new SfxItemDisruptor_Impl(pItem);
    pDesruptor->LaunchDeleteOnIdle();
    // coverity[leaked_storage] pDesruptor takes care of its own destruction at idle time
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
