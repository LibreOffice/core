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

#include <sal/types.h>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XDOMImplementation.hpp>
#include <com/sun/star/xml/sax/XEntityResolver.hpp>
#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace DOM
{
    typedef ::cppu::WeakImplHelper
        < css::xml::dom::XDocumentBuilder
        , css::lang::XServiceInfo
        > CDocumentBuilder_Base;

    class CDocumentBuilder
        : public CDocumentBuilder_Base
    {
    private:
        ::osl::Mutex m_Mutex;
        css::uno::Reference< css::xml::sax::XEntityResolver > m_xEntityResolver;
        css::uno::Reference< css::xml::sax::XErrorHandler > m_xErrorHandler;

    public:

        // ctor
        explicit CDocumentBuilder();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames () override;

        /**
        Obtain an instance of a DOMImplementation object.
        */
        virtual css::uno::Reference< css::xml::dom::XDOMImplementation > SAL_CALL getDOMImplementation() override;

        /**
        Indicates whether or not this parser is configured to understand
        namespaces.
        */
        virtual sal_Bool SAL_CALL isNamespaceAware() override;

        /**
        Indicates whether or not this parser is configured to validate XML
        documents.
        */
        virtual sal_Bool SAL_CALL isValidating() override;

        /**
        Obtain a new instance of a DOM Document object to build a DOM tree
        with.
        */
        virtual css::uno::Reference< css::xml::dom::XDocument > SAL_CALL newDocument() override;

        /**
        Parse the content of the given InputStream as an XML document and
        return a new DOM Document object.
        */
        virtual css::uno::Reference< css::xml::dom::XDocument > SAL_CALL parse(const css::uno::Reference< css::io::XInputStream >& is) override;

        /**
        Parse the content of the given URI as an XML document and return
        a new DOM Document object.
        */
        virtual css::uno::Reference< css::xml::dom::XDocument > SAL_CALL parseURI(const OUString& uri) override;

        /**
        Specify the EntityResolver to be used to resolve entities present
        in the XML document to be parsed.
        */
        virtual void SAL_CALL setEntityResolver(const css::uno::Reference< css::xml::sax::XEntityResolver >& er) override;

        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::xml::sax::XEntityResolver > getEntityResolver();


        /**
        Specify the ErrorHandler to be used to report errors present in
        the XML document to be parsed.
        */
        virtual void SAL_CALL setErrorHandler(const css::uno::Reference< css::xml::sax::XErrorHandler >& eh) override;

        /*
        Get the ErrorHandler to be used to report errors present in
        the XML document to be parsed.
        */

        const css::uno::Reference< css::xml::sax::XErrorHandler >& getErrorHandler() const
        {
            return m_xErrorHandler;
        }

    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
