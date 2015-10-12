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

#ifndef INCLUDED_UNOXML_SOURCE_DOM_DOCUMENTBUILDER_HXX
#define INCLUDED_UNOXML_SOURCE_DOM_DOCUMENTBUILDER_HXX

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
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/IOException.hpp>
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
        css::uno::Reference< css::lang::XMultiServiceFactory > const
            m_xFactory;
        css::uno::Reference< css::xml::sax::XEntityResolver > m_xEntityResolver;
        css::uno::Reference< css::xml::sax::XErrorHandler > m_xErrorHandler;

    public:

        // ctor
        explicit CDocumentBuilder(
            css::uno::Reference< css::lang::XMultiServiceFactory > const&
                xFactory);

        // static helpers for service info and component management
        static const char* aImplementationName;
        static const char* aSupportedServiceNames[];
        static OUString _getImplementationName();
        static css::uno::Sequence< OUString > _getSupportedServiceNames();
        static css::uno::Reference< XInterface > _getInstance(
            css::uno::Reference< css::lang::XMultiServiceFactory > const&
                rSMgr);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
            throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames ()
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Obtain an instance of a DOMImplementation object.
        */
        virtual css::uno::Reference< css::xml::dom::XDOMImplementation > SAL_CALL getDOMImplementation()
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Indicates whether or not this parser is configured to understand
        namespaces.
        */
        virtual sal_Bool SAL_CALL isNamespaceAware()
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Indicates whether or not this parser is configured to validate XML
        documents.
        */
        virtual sal_Bool SAL_CALL isValidating()
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Obtain a new instance of a DOM Document object to build a DOM tree
        with.
        */
        virtual css::uno::Reference< css::xml::dom::XDocument > SAL_CALL newDocument()
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Parse the content of the given InputStream as an XML document and
        return a new DOM Document object.
        */
        virtual css::uno::Reference< css::xml::dom::XDocument > SAL_CALL parse(const css::uno::Reference< css::io::XInputStream >& is)
            throw (css::uno::RuntimeException, css::xml::sax::SAXParseException, css::io::IOException, std::exception) override;

        /**
        Parse the content of the given URI as an XML document and return
        a new DOM Document object.
        */
        virtual css::uno::Reference< css::xml::dom::XDocument > SAL_CALL parseURI(const OUString& uri)
            throw (css::uno::RuntimeException, css::xml::sax::SAXParseException, css::io::IOException, std::exception) override;

        /**
        Specify the EntityResolver to be used to resolve entities present
        in the XML document to be parsed.
        */
        virtual void SAL_CALL setEntityResolver(const css::uno::Reference< css::xml::sax::XEntityResolver >& er)
            throw (css::uno::RuntimeException, std::exception) override;

        css::uno::Reference< css::xml::sax::XEntityResolver > SAL_CALL getEntityResolver()
            throw (css::uno::RuntimeException);


        /**
        Specify the ErrorHandler to be used to report errors present in
        the XML document to be parsed.
        */
        virtual void SAL_CALL setErrorHandler(const css::uno::Reference< css::xml::sax::XErrorHandler >& eh)
            throw (css::uno::RuntimeException, std::exception) override;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
