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

#include "itemholder1.hxx"

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>

#include <unotools/misccfg.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/cmdoptions.hxx>
#include <unotools/compatibility.hxx>
#include <unotools/compatibilityviewoptions.hxx>
#include <unotools/defaultoptions.hxx>
#include <unotools/dynamicmenuoptions.hxx>
#include <unotools/eventcfg.hxx>
#include <unotools/extendedsecurityoptions.hxx>
#include <unotools/fltrcfg.hxx>
#include <unotools/fontoptions.hxx>
#include <unotools/historyoptions.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/printwarningoptions.hxx>
#include <unotools/optionsdlg.hxx>
#include <unotools/searchopt.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <unotools/options.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>

ItemHolder1::ItemHolder1()
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
#ifdef DBG_UTIL
    catch(const css::uno::Exception&)
    {
        static bool bMessage = true;
        if(bMessage)
        {
            bMessage = false;
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "unotools", "CreateInstance with arguments exception: " << exceptionToString(ex));
        }
    }
#else
    catch(css::uno::Exception&){}
#endif
}

ItemHolder1::~ItemHolder1()
{
    impl_releaseAllItems();
}

void ItemHolder1::holdConfigItem(EItem eItem)
{
    static ItemHolder1* pHolder = new ItemHolder1();
    pHolder->impl_addItem(eItem);
}

void SAL_CALL ItemHolder1::disposing(const css::lang::EventObject&)
{
    css::uno::Reference< css::uno::XInterface > xSelfHold(static_cast< css::lang::XEventListener* >(this), css::uno::UNO_QUERY);
    impl_releaseAllItems();
}

void ItemHolder1::impl_addItem(EItem eItem)
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

void ItemHolder1::impl_releaseAllItems()
{
    std::vector< TItemInfo > items;
    {
        ::osl::MutexGuard aLock(m_aLock);
        items.swap(m_lItems);
    }

    // items will be freed when the block exits
}

void ItemHolder1::impl_newItem(TItemInfo& rItem)
{
    switch(rItem.eItem)
    {
        case EItem::CmdOptions :
            rItem.pItem.reset( new SvtCommandOptions() );
            break;

        case EItem::Compatibility :
            rItem.pItem.reset( new SvtCompatibilityOptions() );
            break;

        case EItem::CompatibilityView :
            rItem.pItem.reset( new SvtCompatibilityViewOptions() );
            break;

        case EItem::DefaultOptions :
            rItem.pItem.reset( new SvtDefaultOptions() );
            break;

        case EItem::DynamicMenuOptions :
            rItem.pItem.reset( new SvtDynamicMenuOptions() );
            break;

        case EItem::EventConfig :
            //rItem.pItem.reset( new GlobalEventConfig() );
            break;

        case EItem::ExtendedSecurityOptions :
            rItem.pItem.reset( new SvtExtendedSecurityOptions() );
            break;

        case EItem::FontOptions :
            rItem.pItem.reset( new SvtFontOptions() );
            break;

        case EItem::HistoryOptions :
            rItem.pItem.reset( new SvtHistoryOptions() );
            break;

        case EItem::LinguConfig :
            rItem.pItem.reset( new SvtLinguConfig() );
            break;

        case EItem::ModuleOptions :
            rItem.pItem.reset( new SvtModuleOptions() );
            break;

        case EItem::OptionsDialogOptions :
            rItem.pItem.reset( new SvtOptionsDialogOptions() );
            break;

        case EItem::PathOptions :
            rItem.pItem.reset( new SvtPathOptions() );
            break;

        case EItem::PrintWarningOptions :
            rItem.pItem.reset( new SvtPrintWarningOptions() );
            break;

        case EItem::MiscConfig :
            rItem.pItem.reset( new ::utl::MiscCfg() );
            break;

        case EItem::SecurityOptions :
            rItem.pItem.reset( new SvtSecurityOptions() );
            break;

        case EItem::ViewOptionsDialog :
            rItem.pItem.reset( new SvtViewOptions(EViewType::Dialog, OUString()) );
            break;

        case EItem::ViewOptionsTabDialog :
            rItem.pItem.reset( new SvtViewOptions(EViewType::TabDialog, OUString()) );
            break;

        case EItem::ViewOptionsTabPage :
            rItem.pItem.reset( new SvtViewOptions(EViewType::TabPage, OUString()) );
            break;

        case EItem::ViewOptionsWindow :
            rItem.pItem.reset( new SvtViewOptions(EViewType::Window, OUString()) );
            break;

        case EItem::UserOptions :
            rItem.pItem.reset( new SvtUserOptions() );
            break;

        case EItem::SysLocaleOptions :
            rItem.pItem.reset( new SvtSysLocaleOptions() );
            break;

        default:
            OSL_FAIL( "unknown item type" );
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
