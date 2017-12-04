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

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LOTUSWORDPROIMPORTFILTER_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LOTUSWORDPROIMPORTFILTER_HXX

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustrbuf.hxx>

/* This component will be instantiated for both import or export. Whether it calls
 * setSourceDocument or setTargetDocument determines which Impl function the filter
 * member calls */
class LotusWordProImportFilter final : public cppu::WeakImplHelper
<
    css::document::XFilter,
    css::document::XImporter,
    css::document::XExtendedFilterDetection,
    css::lang::XInitialization,
    css::lang::XServiceInfo
>
{
    // oo.org declares
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< css::lang::XComponent > mxDoc;

    /// @throws css::uno::RuntimeException
    bool importImpl( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor );

public:
    explicit LotusWordProImportFilter( const css::uno::Reference< css::uno::XComponentContext > &rxContext)
        : mxContext( rxContext )
    {
    }

    // XFilter
        virtual sal_Bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor ) override;
        virtual void SAL_CALL cancel(  ) override;

    // XImporter
        virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

    //XExtendedFilterDetection
    virtual OUString SAL_CALL detect( css::uno::Sequence< css::beans::PropertyValue >& Descriptor ) override;

    // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
