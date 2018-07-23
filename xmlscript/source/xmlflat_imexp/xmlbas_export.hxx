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

#ifndef INCLUDED_XMLSCRIPT_SOURCE_XMLFLAT_IMEXP_XMLBAS_EXPORT_HXX
#define INCLUDED_XMLSCRIPT_SOURCE_XMLFLAT_IMEXP_XMLBAS_EXPORT_HXX

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/document/XXMLBasicExporter.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>

namespace xmlscript
{

    // class XMLBasicExporterBase

    typedef ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::lang::XInitialization,
        css::document::XXMLBasicExporter > XMLBasicExporterBase_BASE;

    class XMLBasicExporterBase : public XMLBasicExporterBase_BASE
    {
    private:
        ::osl::Mutex                                              m_aMutex;
        css::uno::Reference< css::xml::sax::XDocumentHandler >    m_xHandler;
        css::uno::Reference< css::frame::XModel >                 m_xModel;
        bool const                                                m_bOasis;

    public:
        explicit XMLBasicExporterBase(bool bOasis);
        virtual ~XMLBasicExporterBase() override;

        // XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XExporter
        virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& rxDoc ) override;

        // XFilter
        virtual sal_Bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor ) override;
        virtual void SAL_CALL cancel() override;
    };

    // class XMLBasicExporter

    class XMLBasicExporter : public XMLBasicExporterBase
    {
    public:
        explicit XMLBasicExporter();
        virtual ~XMLBasicExporter() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
    };

    // class XMLOasisBasicExporter

    class XMLOasisBasicExporter : public XMLBasicExporterBase
    {
    public:
        explicit XMLOasisBasicExporter();
        virtual ~XMLOasisBasicExporter() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
    };

}   // namespace xmlscript

#endif // INCLUDED_XMLSCRIPT_SOURCE_XMLFLAT_IMEXP_XMLBAS_EXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
