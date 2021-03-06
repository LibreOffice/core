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

#pragma once


#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>


enum FilterType
{
    FILTER_IMPORT,
    FILTER_EXPORT
};

/* This component will be instantiated for both import or export. Whether it calls
 * setSourceDocument or setTargetDocument determines which Impl function the filter
 * member calls */

class XmlFilterAdaptor final : public cppu::WeakImplHelper
<
    css::document::XFilter,
    css::document::XExporter,
    css::document::XImporter,
    css::lang::XInitialization,
    css::lang::XServiceInfo
>
{
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< css::lang::XComponent > mxDoc;
    OUString msFilterName;
    css::uno::Sequence< OUString > msUserData;
    OUString msTemplateName;
    FilterType meType;

    /// @throws css::uno::RuntimeException
    bool exportImpl( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor );

    /// @throws css::uno::RuntimeException
    bool importImpl( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor );


public:

    explicit XmlFilterAdaptor( const css::uno::Reference< css::uno::XComponentContext > & rxContext)
        : mxContext(rxContext)
        , meType(FILTER_IMPORT)
    {
    }

    // XFilter

    virtual sal_Bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor ) override;

    virtual void SAL_CALL cancel(  ) override;


    // XExporter

    virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;


    // XImporter

    virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;


    // XInitialization

    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;


    // XServiceInfo

    virtual OUString SAL_CALL getImplementationName(  ) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
