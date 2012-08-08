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

#ifndef __FRAMEWORK_UIFACTORY_TOOLBARCONTROLLERFACTORY_HXX_
#define __FRAMEWORK_UIFACTORY_TOOLBARCONTROLLERFACTORY_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XUIControllerRegistration.hpp>

#include <cppuhelper/implbase3.hxx>
#include <rtl/ustring.hxx>

namespace framework
{

class ConfigurationAccess_ControllerFactory;
class ToolbarControllerFactory :  protected ThreadHelpBase                                          ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                                  public ::cppu::WeakImplHelper3<   com::sun::star::lang::XServiceInfo,
                                                                    com::sun::star::lang::XMultiComponentFactory,
                                                                    com::sun::star::frame::XUIControllerRegistration>
{
    public:
        ToolbarControllerFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
        virtual ~ToolbarControllerFactory();

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO

        // XMultiComponentFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithContext( const ::rtl::OUString& aServiceSpecifier, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArgumentsAndContext( const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames() throw (::com::sun::star::uno::RuntimeException);

        // XUIControllerRegistration
        virtual sal_Bool SAL_CALL hasController( const ::rtl::OUString& aCommandURL, const rtl::OUString& aModuleName ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL registerController( const ::rtl::OUString& aCommandURL, const rtl::OUString& aModuleName, const ::rtl::OUString& aControllerImplementationName ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deregisterController( const ::rtl::OUString& aCommandURL, const rtl::OUString& aModuleName ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        ToolbarControllerFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager,bool  );
        sal_Bool                                                                         m_bConfigRead;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;
        ConfigurationAccess_ControllerFactory*                                           m_pConfigAccess;
};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_TOOLBARCONTROLLERFACTORY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
