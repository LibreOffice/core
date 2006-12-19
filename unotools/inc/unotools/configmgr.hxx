/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configmgr.hxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-19 18:34:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _UTL_CONFIGMGR_HXX_
#define _UTL_CONFIGMGR_HXX_

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

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
                ABOUTBOXPRODUCTVERSION
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
