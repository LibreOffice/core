/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_UNOTOOLS_CONFIGMGR_HXX
#define INCLUDED_UNOTOOLS_CONFIGMGR_HXX

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
