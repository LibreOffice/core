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

#ifndef INCLUDED_SDEXT_SOURCE_PDFIMPORT_PDFIADAPTOR_HXX
#define INCLUDED_SDEXT_SOURCE_PDFIMPORT_PDFIADAPTOR_HXX

#include "xmlemitter.hxx"
#include "treevisitorfactory.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>


namespace pdfi
{
    typedef ::cppu::WeakComponentImplHelper<
        css::document::XFilter,
        css::document::XImporter,
        css::lang::XServiceInfo> PDFIHybridAdaptorBase;

    class PDFIHybridAdaptor : private cppu::BaseMutex,
                              public PDFIHybridAdaptorBase
    {
    private:
        css::uno::Reference<
            css::uno::XComponentContext >  m_xContext;
        css::uno::Reference<
            css::frame::XModel >           m_xModel;

    public:
        explicit PDFIHybridAdaptor( const css::uno::Reference<
                                          css::uno::XComponentContext >& xContext );

        // XFilter
        virtual sal_Bool SAL_CALL filter( const css::uno::Sequence<css::beans::PropertyValue>& rFilterData ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL cancel() throw(std::exception) override;

        // XImporter
        virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDocument )
            throw( css::lang::IllegalArgumentException, std::exception ) override;

        OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) override;

        sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
            throw (css::uno::RuntimeException, std::exception) override;

        css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException, std::exception) override;
    };

    typedef ::cppu::WeakComponentImplHelper<
        css::xml::XImportFilter,
        css::document::XImporter,
        css::lang::XServiceInfo> PDFIAdaptorBase;

    /** Adapts raw pdf import to XImportFilter interface
     */
    class PDFIRawAdaptor : private cppu::BaseMutex,
                           public PDFIAdaptorBase
    {
    private:
        OUString const m_implementationName;
        css::uno::Reference<
            css::uno::XComponentContext >  m_xContext;
        css::uno::Reference<
            css::frame::XModel >           m_xModel;
        TreeVisitorFactorySharedPtr                   m_pVisitorFactory;
        bool                                          m_bEnableToplevelText;

        bool parse( const css::uno::Reference<css::io::XInputStream>&       xInput,
                    const css::uno::Reference<css::task::XInteractionHandler>& xIHdl,
                    const OUString&                                                          rPwd,
                    const css::uno::Reference<css::task::XStatusIndicator>& xStatus,
                    const XmlEmitterSharedPtr&                                                    rEmitter,
                    const OUString&                                                          rURL,
                    const OUString&                                         rFilterOptions = OUString());

    public:
        explicit PDFIRawAdaptor( OUString const & implementationName,
                                 const css::uno::Reference<
                                       css::uno::XComponentContext >& xContext );

        /** Set factory object used to create the tree visitors

            Used for customizing the tree to the specific output
            format (writer, draw, etc)
         */
        void setTreeVisitorFactory(const TreeVisitorFactorySharedPtr& rVisitorFactory);

        /// TEMP - enable writer-like text:p on doc level
        void enableToplevelText() { m_bEnableToplevelText=true; }

        /** Export pdf document to ODG

            @param xOutput
            Stream to write the flat xml file to

            @param xStatus
            Optional status indicator
         */
        bool odfConvert( const OUString&                                                          rURL,
                         const css::uno::Reference<css::io::XOutputStream>&      xOutput,
                         const css::uno::Reference<css::task::XStatusIndicator>& xStatus );

        // XImportFilter
        virtual sal_Bool SAL_CALL importer( const css::uno::Sequence< css::beans::PropertyValue >& rSourceData,
                                            const css::uno::Reference< css::xml::sax::XDocumentHandler >& rHdl,
                                            const css::uno::Sequence< OUString >& rUserData ) throw( css::uno::RuntimeException, std::exception ) override;

        // XImporter
        virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDocument )
            throw( css::lang::IllegalArgumentException, std::exception ) override;

        OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) override;

        sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
            throw (css::uno::RuntimeException, std::exception) override;

        css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException, std::exception) override;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
