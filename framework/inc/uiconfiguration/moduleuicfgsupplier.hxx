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
#include <com/sun/star/frame/XModuleManager.hpp>
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

            ModuleUIConfigurationManagerSupplier( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager );
            virtual ~ModuleUIConfigurationManagerSupplier();

            // XComponent
            virtual void SAL_CALL dispose()
                throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
                throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener )
                throw (::com::sun::star::uno::RuntimeException);

            // XModuleUIConfigurationManagerSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager > SAL_CALL getUIConfigurationManager( const ::rtl::OUString& ModuleIdentifier )
                throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

        private:
            typedef ::boost::unordered_map< rtl::OUString, com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >, OUStringHashCode, ::std::equal_to< rtl::OUString > > ModuleToModuleCfgMgr;

//TODO_AS            void impl_initStorages();

            // private methods
            ModuleToModuleCfgMgr                                                                m_aModuleToModuleUICfgMgrMap;
            bool                                                                                m_bDisposed;
// TODO_AS            bool                                                                                m_bInit;
            rtl::OUString                                                                       m_aDefaultConfigURL;
            rtl::OUString                                                                       m_aUserConfigURL;
            com::sun::star::uno::Reference< com::sun::star::embed::XStorage >                   m_xDefaultCfgRootStorage;
            com::sun::star::uno::Reference< com::sun::star::embed::XStorage >                   m_xUserCfgRootStorage;
            com::sun::star::uno::Reference< com::sun::star::embed::XTransactedObject >          m_xUserRootCommit;
            com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager >   m_xModuleMgr;
            com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >        m_xServiceManager;
            ::cppu::OMultiTypeInterfaceContainerHelper                                          m_aListenerContainer;   /// container for ALL Listener
   };
}

#endif // __FRAMEWORK_UICONFIGURATION_MODULEUICFGSUPPLIER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
