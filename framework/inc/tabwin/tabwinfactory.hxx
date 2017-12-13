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

#ifndef INCLUDED_FRAMEWORK_INC_TABWIN_TABWINFACTORY_HXX
#define INCLUDED_FRAMEWORK_INC_TABWIN_TABWINFACTORY_HXX

#include <stdtypes.h>
#include <macros/xserviceinfo.hxx>
#include <services.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/XToolkit2.hpp>

#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>

namespace framework
{

class TabWinFactory :  public ::cppu::WeakImplHelper< css::lang::XSingleComponentFactory, css::lang::XServiceInfo>
{
    public:
        TabWinFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext );
        virtual ~TabWinFactory() override;

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO_NOFACTORY
        /* Helper for registry */
        /// @throws css::uno::Exception
        static css::uno::Reference< css::uno::XInterface >             SAL_CALL impl_createInstance                ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );
        static css::uno::Reference< css::lang::XSingleServiceFactory > impl_createFactory                 ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );

        // XSingleComponentFactory
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithContext( const css::uno::Reference< css::uno::XComponentContext >& Context ) override;
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArgumentsAndContext( const css::uno::Sequence< css::uno::Any >& Arguments, const css::uno::Reference< css::uno::XComponentContext >& Context ) override;

    private:
        css::uno::Reference< css::uno::XComponentContext >     m_xContext;
        css::uno::Reference< css::awt::XToolkit2 >             m_xToolkit;
};

}

#endif // INCLUDED_FRAMEWORK_INC_TABWIN_TABWINFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
