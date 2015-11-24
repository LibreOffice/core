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

#ifndef INCLUDED_XMLHELP_SOURCE_CXXHELP_INC_TVFACTORY_HXX
#define INCLUDED_XMLHELP_SOURCE_CXXHELP_INC_TVFACTORY_HXX

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>


namespace treeview {

class TVFactory: public cppu::WeakImplHelper <
    css::lang::XServiceInfo,
    css::lang::XMultiServiceFactory >
    {
    public:

        TVFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext );

        virtual ~TVFactory();

        // XServiceInfo
        virtual OUString SAL_CALL
        getImplementationName(
            void )
            throw( css::uno::RuntimeException, std::exception ) override;

        virtual sal_Bool SAL_CALL
        supportsService(
            const OUString& ServiceName )
            throw(css::uno::RuntimeException, std::exception ) override;

        virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames(
            void )
            throw( css::uno::RuntimeException, std::exception ) override;

        // XMultiServiceFactory

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
        createInstance(
            const OUString& aServiceSpecifier )
            throw( css::uno::Exception,
                   css::uno::RuntimeException, std::exception ) override;

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
        createInstanceWithArguments(
            const OUString& ServiceSpecifier,
            const css::uno::Sequence< css::uno::Any >& Arguments )
            throw( css::uno::Exception,
                   css::uno::RuntimeException, std::exception) override;

        virtual css::uno::Sequence< OUString > SAL_CALL
        getAvailableServiceNames( )
            throw( css::uno::RuntimeException, std::exception ) override;

        // Other

        static OUString SAL_CALL getImplementationName_static();

        static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static();

        static css::uno::Reference< css::lang::XSingleServiceFactory > SAL_CALL
        createServiceFactory(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& rxServiceMgr );

        static css::uno::Reference< css::uno::XInterface > SAL_CALL
        CreateInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& xMultiServiceFactory );


    private:

        // Members
        css::uno::Reference< css::uno::XComponentContext >      m_xContext;
        css::uno::Reference< css::uno::XInterface >             m_xHDS;
    };

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
