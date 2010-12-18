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

#ifndef _UTL_CONFIGMGR_HXX_
#define _UTL_CONFIGMGR_HXX_

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>
#include "unotools/unotoolsdllapi.h"

//-----------------------------------------------------------------------------
namespace com{ namespace sun{ namespace star{
    namespace lang{
        class XMultiServiceFactory;
    }
    namespace container{
        class XHierarchicalNameAccess;
    }
}}}

//-----------------------------------------------------------------------------
namespace utl
{
    struct ConfigMgr_Impl;
    class ConfigItem;
    class UNOTOOLS_DLLPUBLIC ConfigManager
    {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                xConfigurationProvider;
            ConfigMgr_Impl*     pMgrImpl;

            static void getBasisAboutBoxProductVersion( rtl::OUString& rVersion );

        public:
            ConfigManager();
            ConfigManager(com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xConfigProvider);
            ~ConfigManager();

            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                GetConfigurationProvider();

            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                GetLocalConfigurationProvider();

            com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess>
                AddConfigItem(utl::ConfigItem& rCfgItem);

            void RegisterConfigItem(utl::ConfigItem& rCfgItem);
            com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess>
                AcquireTree(utl::ConfigItem& rCfgItem);


            void RemoveConfigItem(utl::ConfigItem& rCfgItem);

            void StoreConfigItems();

            static ConfigManager&           GetConfigManager();
            static rtl::OUString            GetConfigBaseURL();

            enum ConfigProperty
            {
                INSTALLPATH,        // deprecated. don't use
                LOCALE,
                OFFICEINSTALL,      // deprecated. don't use
                USERINSTALLURL,     // deprecated. don't use
                OFFICEINSTALLURL,   // deprecated. don't use
                PRODUCTNAME,
                PRODUCTVERSION,
                PRODUCTEXTENSION,
                DEFAULTCURRENCY,
                PRODUCTXMLFILEFORMATNAME,
                PRODUCTXMLFILEFORMATVERSION,
                WRITERCOMPATIBILITYVERSIONOOO11,
                OPENSOURCECONTEXT,
                ABOUTBOXPRODUCTVERSION,
                OOOVENDOR
            };
            //direct readonly access to some special configuration elements
            static com::sun::star::uno::Any GetDirectConfigProperty(ConfigProperty eProp);

            sal_Bool        IsLocalConfigProvider();
            com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess>
                GetHierarchyAccess(const rtl::OUString& rFullPath);
            com::sun::star::uno::Any GetLocalProperty(const rtl::OUString& rProperty);
            void PutLocalProperty(const rtl::OUString& , const com::sun::star::uno::Any& rValue);

    };
}//namespace utl
#endif //_UTL_CONFIGMGR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
