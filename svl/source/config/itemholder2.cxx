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


#include "itemholder2.hxx"

#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>

#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <svl/languageoptions.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/options.hxx>

ItemHolder2::ItemHolder2()
    : ItemHolderMutexBase()
{
    try
    {
        css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        css::uno::Reference< css::lang::XComponent > xCfg( css::configuration::theDefaultProvider::get(xContext), css::uno::UNO_QUERY_THROW );
        xCfg->addEventListener(static_cast< css::lang::XEventListener* >(this));
    }
    catch(const css::uno::RuntimeException&)
    {
        throw;
    }
#ifdef DBG_UTIL
    catch(const css::uno::Exception&)
    {
        static bool bMessage = true;
        if(bMessage)
        {
            bMessage = false;
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "svl", "CreateInstance with arguments exception: " << exceptionToString(ex));
        }
    }
#else
    catch(css::uno::Exception&){}
#endif
}

ItemHolder2::~ItemHolder2()
{
    impl_releaseAllItems();
}

void ItemHolder2::holdConfigItem(EItem eItem)
{
    static ItemHolder2* pHolder = new ItemHolder2();
    pHolder->impl_addItem(eItem);
}

void SAL_CALL ItemHolder2::disposing(const css::lang::EventObject&)
{
    impl_releaseAllItems();
}

void ItemHolder2::impl_addItem(EItem eItem)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    for ( auto const & rInfo : m_lItems )
    {
        if (rInfo.eItem == eItem)
            return;
    }

    TItemInfo aNewItem;
    aNewItem.eItem = eItem;
    impl_newItem(aNewItem);
    if (aNewItem.pItem)
        m_lItems.emplace_back(std::move(aNewItem));
}

void ItemHolder2::impl_releaseAllItems()
{
    std::vector< TItemInfo > items;
    {
        ::osl::MutexGuard aLock(m_aLock);
        items.swap(m_lItems);
    }

    // items will be freed when the block exits
}

void ItemHolder2::impl_newItem(TItemInfo& rItem)
{
    switch(rItem.eItem)
    {
        case EItem::CJKOptions :
            rItem.pItem.reset( new SvtCJKOptions() );
            break;

        case EItem::CTLOptions :
            rItem.pItem.reset( new SvtCTLOptions() );
            break;

        default:
            OSL_ASSERT(false);
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
