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

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <officecfg/Setup.hxx>
#include <rtl/instance.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <unotools/configitem.hxx>
#include <unotools/configmgr.hxx>
#include <comphelper/processfactory.hxx>

namespace {

class RegisterConfigItemHelper {
public:
    RegisterConfigItemHelper(
        utl::ConfigManager & manager, utl::ConfigItem & item):
            manager_(manager), item_(&item)
    {
        manager.registerConfigItem(item_);
    }

    ~RegisterConfigItemHelper() {
        if (item_ != nullptr) {
            manager_.removeConfigItem(*item_);
        }
    }

    void keep() { item_ = nullptr; }

private:
    utl::ConfigManager & manager_;
    utl::ConfigItem * item_;

    RegisterConfigItemHelper(const RegisterConfigItemHelper&) = delete;
    RegisterConfigItemHelper& operator=(const RegisterConfigItemHelper&) = delete;
};

css::uno::Reference< css::lang::XMultiServiceFactory >
getConfigurationProvider() {
    return css::configuration::theDefaultProvider::get( comphelper::getProcessComponentContext() );
}

struct theConfigManager:
    public rtl::Static< utl::ConfigManager, theConfigManager >
{};

}

OUString utl::ConfigManager::getAboutBoxProductVersion() {
    return officecfg::Setup::Product::ooSetupVersionAboutBox::get();
}

OUString utl::ConfigManager::getAboutBoxProductVersionSuffix() {
    return officecfg::Setup::Product::ooSetupVersionAboutBoxSuffix::get();
}

OUString utl::ConfigManager::getDefaultCurrency() {
    return officecfg::Setup::L10N::ooSetupCurrency::get();
}

OUString utl::ConfigManager::getUILocale() {
    return officecfg::Setup::L10N::ooLocale::get();
}

OUString utl::ConfigManager::getWorkLocale() {
    return officecfg::Setup::L10N::ooSetupSystemLocale::get();
}

OUString utl::ConfigManager::getProductExtension() {
    return officecfg::Setup::Product::ooSetupExtension::get();
}

OUString utl::ConfigManager::getProductName() {
    return officecfg::Setup::Product::ooName::get();
}

OUString utl::ConfigManager::getProductVersion() {
    return officecfg::Setup::Product::ooSetupVersion::get();
}

OUString utl::ConfigManager::getVendor() {
    return officecfg::Setup::Product::ooVendor::get();
}

void utl::ConfigManager::storeConfigItems() {
    getConfigManager().doStoreConfigItems();
}

utl::ConfigManager & utl::ConfigManager::getConfigManager() {
    return theConfigManager::get();
}

css::uno::Reference< css::container::XHierarchicalNameAccess >
utl::ConfigManager::acquireTree(utl::ConfigItem const & item) {
    css::uno::Sequence< css::uno::Any > args(1);
    args[0] <<= css::beans::NamedValue(
        "nodepath",
        css::uno::makeAny("/org.openoffice." + item.GetSubTreeName()));
    if (item.GetMode() & ConfigItemMode::AllLocales) {
        args.realloc(2);
        args[1] <<= css::beans::NamedValue("locale", css::uno::makeAny(OUString("*")));
    }
    return css::uno::Reference< css::container::XHierarchicalNameAccess >(
        getConfigurationProvider()->createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationUpdateAccess",
            args),
        css::uno::UNO_QUERY_THROW);
}

css::uno::Reference< css::container::XHierarchicalNameAccess >
utl::ConfigManager::acquireTree(std::u16string_view rSubTreeName) {
    css::uno::Sequence< css::uno::Any > args(1);
    args[0] <<= css::beans::NamedValue(
        "nodepath",
        css::uno::makeAny(OUString::Concat(u"/org.openoffice.") + rSubTreeName));
    return css::uno::Reference< css::container::XHierarchicalNameAccess >(
        getConfigurationProvider()->createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationUpdateAccess",
            args),
        css::uno::UNO_QUERY_THROW);
}

utl::ConfigManager::ConfigManager() {}

utl::ConfigManager::~ConfigManager() {
    if (!items_.empty())
    {
        for (const auto& item : items_)
        {
            if (OUString itemName = item->GetSubTreeName(); itemName != "Office.Commands/Execute")
                SAL_WARN("unotools.config", "ConfigManager not empty, remaining item=" + itemName);
        }
    }
}

css::uno::Reference< css::container::XHierarchicalNameAccess >
utl::ConfigManager::addConfigItem(utl::ConfigItem & item) {
    RegisterConfigItemHelper reg(*this, item);
    css::uno::Reference< css::container::XHierarchicalNameAccess > tree(
        acquireTree(item));
    reg.keep();
    return tree;
}

void utl::ConfigManager::removeConfigItem(utl::ConfigItem & item) {
    items_.erase(std::remove(items_.begin(), items_.end(), &item), items_.end());
}

void utl::ConfigManager::registerConfigItem(utl::ConfigItem * item) {
    assert(item != nullptr);
    items_.push_back(item);
}

void utl::ConfigManager::doStoreConfigItems() {
    for (auto const& item : items_)
    {
        if (item->IsModified()) {
            item->Commit();
            item->ClearModified();
        }
    }
}

static bool bIsFuzzing = false;

#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
bool utl::ConfigManager::IsFuzzing()
{
    return bIsFuzzing;
}
#endif

void utl::ConfigManager::EnableFuzzing()
{
    bIsFuzzing = true;
    LanguageTag::disable_lt_tag_parse();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
