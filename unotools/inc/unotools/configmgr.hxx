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

#pragma once
#if 1

#include "sal/config.h"

#include <list>

#include "boost/noncopyable.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "sal/types.h"
#include "unotools/unotoolsdllapi.h"

namespace com { namespace sun { namespace star {
    namespace container{ class XHierarchicalNameAccess; }
} } }
namespace rtl { class OUString; }
namespace utl { class ConfigItem; }

namespace utl {

class UNOTOOLS_DLLPUBLIC ConfigManager: private boost::noncopyable {
public:
    static rtl::OUString getAboutBoxProductVersion();

    static rtl::OUString getAboutBoxProductVersionSuffix();

    static rtl::OUString getDefaultCurrency();

    static rtl::OUString getLocale();

    static rtl::OUString getProductExtension();

    static rtl::OUString getProductName();

    static rtl::OUString getProductXmlFileFormat();

    static rtl::OUString getProductXmlFileFormatVersion();

    static rtl::OUString getProductVersion();

    static rtl::OUString getVendor();

    static rtl::OUString getWriterCompatibilityVersionOOo_1_1();

    static void storeConfigItems();

    SAL_DLLPRIVATE static ConfigManager & getConfigManager();

    SAL_DLLPRIVATE static com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess>
    acquireTree(utl::ConfigItem & item);

    SAL_DLLPRIVATE ConfigManager();

    SAL_DLLPRIVATE ~ConfigManager();

    SAL_DLLPRIVATE com::sun::star::uno::Reference<
        com::sun::star::container::XHierarchicalNameAccess >
    addConfigItem(utl::ConfigItem & item);

    SAL_DLLPRIVATE void removeConfigItem(utl::ConfigItem & item);

    SAL_DLLPRIVATE void registerConfigItem(utl::ConfigItem * item);

private:
    void doStoreConfigItems();

    std::list< ConfigItem * > items_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
