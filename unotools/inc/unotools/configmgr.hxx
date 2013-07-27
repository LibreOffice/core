/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

            static  ConfigManager*  pConfigManager;
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

            static ConfigManager*           GetConfigManager();
            static void                     RemoveConfigManager();
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
                OOOVENDOR,
                FULLPRODUCTNAME
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
