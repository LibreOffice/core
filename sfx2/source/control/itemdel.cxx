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

#include <itemdel.hxx>
#include <vcl/idle.hxx>

#include <svl/itempool.hxx>

class SfxItemDisruptor_Impl
{
    std::unique_ptr<SfxPoolItem> pItem;
    Idle m_Idle;

private:
    DECL_LINK( Delete, Timer*, void );

public:
    explicit SfxItemDisruptor_Impl(SfxPoolItem *pItemToDesrupt);
    void LaunchDeleteOnIdle();
    ~SfxItemDisruptor_Impl();
    SfxItemDisruptor_Impl(const SfxItemDisruptor_Impl&) = delete;
    SfxItemDisruptor_Impl& operator=(const SfxItemDisruptor_Impl&) = delete;
};

SfxItemDisruptor_Impl::SfxItemDisruptor_Impl(SfxPoolItem *const pItemToDisrupt)
    : pItem(pItemToDisrupt)
    , m_Idle("sfx SfxItemDisruptor_Impl::Delete")
{
    m_Idle.SetInvokeHandler(LINK(this, SfxItemDisruptor_Impl, Delete));
    m_Idle.SetPriority(TaskPriority::DEFAULT_IDLE);
    m_Idle.SetDebugName("sfx::SfxItemDisruptor_Impl m_Idle");

    DBG_ASSERT( 0 == pItem->GetRefCount(), "disrupting pooled item" );
    pItem->SetKind(SfxItemKind::DeleteOnIdle);
}

void SfxItemDisruptor_Impl::LaunchDeleteOnIdle()
{
    m_Idle.Start();
}

SfxItemDisruptor_Impl::~SfxItemDisruptor_Impl()
{
    m_Idle.Stop();

    // reset RefCount (was set to SFX_ITEMS_SPECIAL before!)
    pItem->SetRefCount( 0 );

    pItem.reset();
}

IMPL_LINK_NOARG(SfxItemDisruptor_Impl, Delete, Timer*, void)
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
