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

#ifndef INCLUDED_UNOTOOLS_CONFIGMGR_HXX
#define INCLUDED_UNOTOOLS_CONFIGMGR_HXX

#include <sal/config.h>

#include <string_view>
#include <vector>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <unotools/unotoolsdllapi.h>

namespace com::sun::star::container {
    class XHierarchicalNameAccess;
}
namespace utl { class ConfigItem; }
namespace com::sun::star::uno { template <class interface_type> class Reference; }

namespace utl {

class UNOTOOLS_DLLPUBLIC ConfigManager {
public:
    static OUString getAboutBoxProductVersion();

    static OUString getAboutBoxProductVersionSuffix();

    static OUString getDefaultCurrency();

    static OUString getUILocale();

    static OUString getWorkLocale();

    static OUString getProductExtension();

    static OUString getProductName();

    static OUString getProductVersion();

    static OUString getVendor();

    static void storeConfigItems();

    SAL_DLLPRIVATE static ConfigManager & getConfigManager();

    SAL_DLLPRIVATE static css::uno::Reference< css::container::XHierarchicalNameAccess>
    acquireTree(utl::ConfigItem const & item);

    static css::uno::Reference< css::container::XHierarchicalNameAccess>
    acquireTree(std::u16string_view rSubTreeName);

    SAL_DLLPRIVATE ConfigManager();

    SAL_DLLPRIVATE ~ConfigManager();

    SAL_DLLPRIVATE css::uno::Reference< css::container::XHierarchicalNameAccess >
    addConfigItem(utl::ConfigItem & item);

    SAL_DLLPRIVATE void removeConfigItem(utl::ConfigItem & item);

    SAL_DLLPRIVATE void registerConfigItem(utl::ConfigItem * item);

private:

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    void doStoreConfigItems();

    std::vector< ConfigItem * > items_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
