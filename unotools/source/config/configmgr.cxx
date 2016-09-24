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

#include <list>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/diagnose.h>
#include <rtl/instance.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
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

OUString getConfigurationString(OUString const & module, OUString const & path)
{
    css::uno::Sequence< css::uno::Any > args(1);
    args[0] <<= css::beans::NamedValue(
        OUString("nodepath"),
        css::uno::makeAny(module));
    return
        css::uno::Reference< css::container::XHierarchicalNameAccess >(
            getConfigurationProvider()->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess",
                args),
            css::uno::UNO_QUERY_THROW)->
        getByHierarchicalName(path).get< OUString >();
}

struct theConfigManager:
    public rtl::Static< utl::ConfigManager, theConfigManager >
{};

}

OUString utl::ConfigManager::getAboutBoxProductVersion() {
    return getConfigurationString(
        "/org.openoffice.Setup",
        "Product/ooSetupVersionAboutBox");
}

OUString utl::ConfigManager::getAboutBoxProductVersionSuffix() {
    return getConfigurationString(
        "/org.openoffice.Setup",
        "Product/ooSetupVersionAboutBoxSuffix");
}

OUString utl::ConfigManager::getDefaultCurrency() {
    return getConfigurationString(
        "/org.openoffice.Setup",
        "L10N/ooSetupCurrency");
}

OUString utl::ConfigManager::getLocale() {
    return getConfigurationString(
        "/org.openoffice.Setup",
        "L10N/ooLocale");
}

OUString utl::ConfigManager::getProductExtension() {
    return getConfigurationString(
        "/org.openoffice.Setup",
        "Product/ooSetupExtension");
}

OUString utl::ConfigManager::getProductName() {
    return getConfigurationString(
        "/org.openoffice.Setup",
        "Product/ooName");
}

OUString utl::ConfigManager::getProductVersion() {
    return getConfigurationString(
        "/org.openoffice.Setup",
        "Product/ooSetupVersion");
}

OUString utl::ConfigManager::getVendor() {
    return getConfigurationString(
        "/org.openoffice.Setup",
        "Product/ooVendor");
}

void utl::ConfigManager::storeConfigItems() {
    getConfigManager().doStoreConfigItems();
}

utl::ConfigManager & utl::ConfigManager::getConfigManager() {
    return theConfigManager::get();
}

css::uno::Reference< css::container::XHierarchicalNameAccess >
utl::ConfigManager::acquireTree(utl::ConfigItem & item) {
    css::uno::Sequence< css::uno::Any > args(1);
    args[0] <<= css::beans::NamedValue(
        OUString("nodepath"),
        css::uno::makeAny("/org.openoffice." + item.GetSubTreeName()));
    if (item.GetMode() & ConfigItemMode::AllLocales) {
        args.realloc(2);
        args[1] <<= css::beans::NamedValue(OUString("locale"), css::uno::makeAny(OUString("*")));
    }
    return css::uno::Reference< css::container::XHierarchicalNameAccess >(
        getConfigurationProvider()->createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationUpdateAccess",
            args),
        css::uno::UNO_QUERY_THROW);
}

utl::ConfigManager::ConfigManager() {}

utl::ConfigManager::~ConfigManager() {
    OSL_ASSERT(items_.empty());
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
    for (std::list< ConfigItem * >::iterator i(items_.begin());
         i != items_.end(); ++i)
    {
        if (*i == &item) {
            items_.erase(i);
            break;
        }
    }
}

void utl::ConfigManager::registerConfigItem(utl::ConfigItem * item) {
    OSL_ASSERT(item != nullptr);
    items_.push_back(item);
}

void utl::ConfigManager::doStoreConfigItems() {
    for (std::list< ConfigItem * >::iterator i(items_.begin());
         i != items_.end(); ++i)
    {
        if ((*i)->IsModified()) {
            (*i)->Commit();
            (*i)->ClearModified();
        }
    }
}

static bool bIsAvoidConfig = false;

bool utl::ConfigManager::IsAvoidConfig()
{
    return bIsAvoidConfig;
}

void utl::ConfigManager::EnableAvoidConfig()
{
    bIsAvoidConfig = true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
