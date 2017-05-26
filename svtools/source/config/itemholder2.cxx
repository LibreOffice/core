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
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>

#include <svtools/accessibilityoptions.hxx>
#include <svtools/apearcfg.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/fontsubstconfig.hxx>
#include <svtools/helpopt.hxx>
#include <svtools/printoptions.hxx>
#include <unotools/options.hxx>
#include <svtools/miscopt.hxx>

namespace svtools {

ItemHolder2::ItemHolder2()
    : ItemHolderMutexBase()
{
    try
    {
        css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        css::uno::Reference< css::lang::XComponent > xCfg(
            css::configuration::theDefaultProvider::get( xContext ),
            css::uno::UNO_QUERY_THROW );
        xCfg->addEventListener(static_cast< css::lang::XEventListener* >(this));
    }
    catch(const css::uno::RuntimeException&)
    {
        throw;
    }
#ifdef DBG_UTIL
    catch(const css::uno::Exception& rEx)
    {
        static bool bMessage = true;
        if(bMessage)
        {
            bMessage = false;
            SAL_WARN( "svtools", "CreateInstance with arguments exception: " << rEx.Message );
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

    TItems::const_iterator pIt;
    for (  pIt  = m_lItems.begin();
           pIt != m_lItems.end()  ;
         ++pIt                    )
    {
        const TItemInfo& rInfo = *pIt;
        if (rInfo.eItem == eItem)
            return;
    }

    TItemInfo aNewItem;
    aNewItem.eItem = eItem;
    impl_newItem(aNewItem);
    if (aNewItem.pItem)
        m_lItems.push_back(aNewItem);
}


void ItemHolder2::impl_releaseAllItems()
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    TItems::iterator pIt;
    for (  pIt  = m_lItems.begin();
           pIt != m_lItems.end()  ;
         ++pIt                    )
    {
        TItemInfo& rInfo = *pIt;
        impl_deleteItem(rInfo);
    }
    m_lItems.clear();
}


void ItemHolder2::impl_newItem(TItemInfo& rItem)
{
    switch(rItem.eItem)
    {
        case EItem::AccessibilityOptions :
            rItem.pItem = new SvtAccessibilityOptions();
            break;

        case EItem::ColorConfig :
            rItem.pItem = new ::svtools::ColorConfig();
            break;

        case EItem::HelpOptions :
            rItem.pItem = new SvtHelpOptions();
            break;

        case EItem::MenuOptions :
            rItem.pItem = new SvtMenuOptions();
            break;

        case EItem::PrintOptions :
            rItem.pItem = new SvtPrinterOptions();
            break;

        case EItem::PrintFileOptions :
            rItem.pItem = new SvtPrintFileOptions();
            break;

        case EItem::MiscOptions :
            rItem.pItem = new SvtMiscOptions();
            break;

        default:
            OSL_ASSERT(false);
            break;
    }
}


void ItemHolder2::impl_deleteItem(TItemInfo& rItem)
{
    if (rItem.pItem)
    {
        delete rItem.pItem;
        rItem.pItem = nullptr;
    }
}

} // namespace svtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
