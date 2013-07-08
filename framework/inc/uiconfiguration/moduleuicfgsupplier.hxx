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

#ifndef __FRAMEWORK_UICONFIGURATION_MODULEUICFGSUPPLIER_HXX_
#define __FRAMEWORK_UICONFIGURATION_MODULEUICFGSUPPLIER_HXX_


/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <list>
#include <boost/unordered_map.hpp>

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManager2.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <rtl/ustring.hxx>


namespace framework
{
    class ModuleUIConfigurationManagerSupplier : public com::sun::star::lang::XTypeProvider ,
                                                 public com::sun::star::lang::XServiceInfo  ,
                                                 public com::sun::star::lang::XComponent    ,
                                                 public ::com::sun::star::ui::XModuleUIConfigurationManagerSupplier      ,
                                                 private ThreadHelpBase                     ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                                                 public ::cppu::OWeakObject
    {
        public:
            //  XInterface, XTypeProvider, XServiceInfo
            FWK_DECLARE_XINTERFACE
            FWK_DECLARE_XTYPEPROVIDER
            DECLARE_XSERVICEINFO

            ModuleUIConfigurationManagerSupplier( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext );
            virtual ~ModuleUIConfigurationManagerSupplier();

            // XComponent
            virtual void SAL_CALL dispose()
                throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
                throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener )
                throw (::com::sun::star::uno::RuntimeException);

            // XModuleUIConfigurationManagerSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager > SAL_CALL getUIConfigurationManager( const OUString& ModuleIdentifier )
                throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

        private:
            typedef ::boost::unordered_map< OUString, com::sun::star::uno::Reference< ::com::sun::star::ui::XModuleUIConfigurationManager2 >, OUStringHash, ::std::equal_to< OUString > > ModuleToModuleCfgMgr;

//TODO_AS            void impl_initStorages();

            // private methods
            ModuleToModuleCfgMgr                                                                m_aModuleToModuleUICfgMgrMap;
            bool                                                                                m_bDisposed;
// TODO_AS            bool                                                                                m_bInit;
            OUString                                                                       m_aDefaultConfigURL;
            OUString                                                                       m_aUserConfigURL;
            com::sun::star::uno::Reference< com::sun::star::embed::XStorage >                   m_xDefaultCfgRootStorage;
            com::sun::star::uno::Reference< com::sun::star::embed::XStorage >                   m_xUserCfgRootStorage;
            com::sun::star::uno::Reference< com::sun::star::embed::XTransactedObject >          m_xUserRootCommit;
            com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager2 >          m_xModuleMgr;
            com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >            m_xContext;
            ::cppu::OMultiTypeInterfaceContainerHelper                                          m_aListenerContainer;   /// container for ALL Listener
   };
}

#endif // __FRAMEWORK_UICONFIGURATION_MODULEUICFGSUPPLIER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
