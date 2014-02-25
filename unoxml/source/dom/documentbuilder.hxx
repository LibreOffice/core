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

#include <cppuhelper/implbase2.hxx>

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


using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::sax;
using namespace com::sun::star::io;

namespace DOM
{
    typedef ::cppu::WeakImplHelper2
        < XDocumentBuilder
        , ::com::sun::star::lang::XServiceInfo
        > CDocumentBuilder_Base;

    class CDocumentBuilder
        : public CDocumentBuilder_Base
    {
    private:
        ::osl::Mutex m_Mutex;
        Reference< ::com::sun::star::lang::XMultiServiceFactory > const
            m_xFactory;
        Reference< XEntityResolver > m_xEntityResolver;
        Reference< XErrorHandler > m_xErrorHandler;

    public:

        // ctor
        CDocumentBuilder(
            Reference< ::com::sun::star::lang::XMultiServiceFactory > const&
                xFactory);

        // call for factory
        static Reference< XInterface > getInstance(
            Reference< ::com::sun::star::lang::XMultiServiceFactory > const&
                xFactory);

        // static helpers for service info and component management
        static const char* aImplementationName;
        static const char* aSupportedServiceNames[];
        static OUString _getImplementationName();
        static Sequence< OUString > _getSupportedServiceNames();
        static Reference< XInterface > _getInstance(
            Reference< ::com::sun::star::lang::XMultiServiceFactory > const&
                rSMgr);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw (RuntimeException, std::exception);
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
            throw (RuntimeException, std::exception);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames ()
            throw (RuntimeException, std::exception);

        /**
        Obtain an instance of a DOMImplementation object.
        */
        virtual Reference< XDOMImplementation > SAL_CALL getDOMImplementation()
            throw (RuntimeException, std::exception);

        /**
        Indicates whether or not this parser is configured to understand
        namespaces.
        */
        virtual sal_Bool SAL_CALL isNamespaceAware()
            throw (RuntimeException, std::exception);

        /**
        Indicates whether or not this parser is configured to validate XML
        documents.
        */
        virtual sal_Bool SAL_CALL isValidating()
            throw (RuntimeException, std::exception);

        /**
        Obtain a new instance of a DOM Document object to build a DOM tree
        with.
        */
        virtual Reference< XDocument > SAL_CALL newDocument()
            throw (RuntimeException, std::exception);

        /**
        Parse the content of the given InputStream as an XML document and
        return a new DOM Document object.
        */
        virtual Reference< XDocument > SAL_CALL parse(const Reference< XInputStream >& is)
            throw (RuntimeException, SAXParseException, IOException, std::exception);

        /**
        Parse the content of the given URI as an XML document and return
        a new DOM Document object.
        */
        virtual Reference< XDocument > SAL_CALL parseURI(const OUString& uri)
            throw (RuntimeException, SAXParseException, IOException, std::exception);

        /**
        Specify the EntityResolver to be used to resolve entities present
        in the XML document to be parsed.
        */
        virtual void SAL_CALL setEntityResolver(const Reference< XEntityResolver >& er)
            throw (RuntimeException, std::exception);

        virtual Reference< XEntityResolver > SAL_CALL getEntityResolver()
            throw (RuntimeException);


        /**
        Specify the ErrorHandler to be used to report errors present in
        the XML document to be parsed.
        */
        virtual void SAL_CALL setErrorHandler(const Reference< XErrorHandler >& eh)
            throw (RuntimeException, std::exception);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
