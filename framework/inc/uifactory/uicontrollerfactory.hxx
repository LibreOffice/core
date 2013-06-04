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

#ifndef __FRAMEWORK_UICONTROLLERFACTORY_HXX_
#define __FRAMEWORK_UICONTROLLERFACTORY_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>

#include <cppuhelper/implbase2.hxx>

namespace framework
{

class ConfigurationAccess_ControllerFactory;
class UIControllerFactory :  protected ThreadHelpBase, // Struct for right initalization of mutex member! Must be first of baseclasses.
                             public ::cppu::WeakImplHelper2<
                                 com::sun::star::lang::XServiceInfo,
                                 com::sun::star::frame::XUIControllerFactory >
{
    public:
        virtual ~UIControllerFactory();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException) = 0;

        // XMultiComponentFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithContext( const OUString& aServiceSpecifier, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArgumentsAndContext( const OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() throw (::com::sun::star::uno::RuntimeException);

        // XUIControllerRegistration
        virtual sal_Bool SAL_CALL hasController( const OUString& aCommandURL, const OUString& aModuleName ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL registerController( const OUString& aCommandURL, const OUString& aModuleName, const OUString& aControllerImplementationName ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deregisterController( const OUString& aCommandURL, const OUString& aModuleName ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        UIControllerFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext, const rtl::OUString &rUINode  );
        sal_Bool                                                                         m_bConfigRead;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >     m_xContext;
        ConfigurationAccess_ControllerFactory*                                           m_pConfigAccess;
};

class PopupMenuControllerFactory :  public UIControllerFactory
{
    public:
        PopupMenuControllerFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO
};

class ToolbarControllerFactory :  public UIControllerFactory
{
    public:
        ToolbarControllerFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO
};

class StatusbarControllerFactory :  public UIControllerFactory
{
    public:
        StatusbarControllerFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO
};

}

#endif // __FRAMEWORK_UICONTROLLERFACTORY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
