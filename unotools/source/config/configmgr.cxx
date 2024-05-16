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
    static utl::ConfigManager theConfigManager;
    return theConfigManager;
}

css::uno::Reference< css::container::XHierarchicalNameAccess >
utl::ConfigManager::acquireTree(utl::ConfigItem const & item) {
    css::uno::Sequence< css::uno::Any > args{ css::uno::Any(css::beans::NamedValue(
        u"nodepath"_ustr,
        css::uno::Any("/org.openoffice." + item.GetSubTreeName()))) };
    if (item.GetMode() & ConfigItemMode::AllLocales) {
        args.realloc(2);
        args.getArray()[1] <<= css::beans::NamedValue(u"locale"_ustr, css::uno::Any(u"*"_ustr));
    }
    return css::uno::Reference< css::container::XHierarchicalNameAccess >(
        getConfigurationProvider()->createInstanceWithArguments(
            u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr,
            args),
        css::uno::UNO_QUERY_THROW);
}

css::uno::Reference< css::container::XHierarchicalNameAccess >
utl::ConfigManager::acquireTree(std::u16string_view rSubTreeName) {
    css::uno::Sequence< css::uno::Any > args{ css::uno::Any(css::beans::NamedValue(
        u"nodepath"_ustr,
        css::uno::Any(OUString::Concat(u"/org.openoffice.") + rSubTreeName))) };
    return css::uno::Reference< css::container::XHierarchicalNameAccess >(
        getConfigurationProvider()->createInstanceWithArguments(
            u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr,
            args),
        css::uno::UNO_QUERY_THROW);
}

utl::ConfigManager::ConfigManager() {}

utl::ConfigManager::~ConfigManager() {
    SAL_WARN_IF(!items_.empty(), "unotools.config", "ConfigManager not empty");
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
    std::erase(items_, &item);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
