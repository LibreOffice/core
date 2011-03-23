/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef DOM_DOCUMENTBUILDER_HXX
#define DOM_DOCUMENTBUILDER_HXX

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


using ::rtl::OUString;
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
            throw (RuntimeException);
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
            throw (RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames ()
            throw (RuntimeException);

        /**
        Obtain an instance of a DOMImplementation object.
        */
        virtual Reference< XDOMImplementation > SAL_CALL getDOMImplementation()
            throw (RuntimeException);

        /**
        Indicates whether or not this parser is configured to understand
        namespaces.
        */
        virtual sal_Bool SAL_CALL isNamespaceAware()
            throw (RuntimeException);

        /**
        Indicates whether or not this parser is configured to validate XML
        documents.
        */
        virtual sal_Bool SAL_CALL isValidating()
            throw (RuntimeException);

        /**
        Obtain a new instance of a DOM Document object to build a DOM tree
        with.
        */
        virtual Reference< XDocument > SAL_CALL newDocument()
            throw (RuntimeException);

        /**
        Parse the content of the given InputStream as an XML document and
        return a new DOM Document object.
        */
        virtual Reference< XDocument > SAL_CALL parse(const Reference< XInputStream >& is)
            throw (RuntimeException, SAXParseException, IOException);

        /**
        Parse the content of the given URI as an XML document and return
        a new DOM Document object.
        */
        virtual Reference< XDocument > SAL_CALL parseURI(const OUString& uri)
            throw (RuntimeException, SAXParseException, IOException);

        /**
        Specify the EntityResolver to be used to resolve entities present
        in the XML document to be parsed.
        */
        virtual void SAL_CALL setEntityResolver(const Reference< XEntityResolver >& er)
            throw (RuntimeException);

        virtual Reference< XEntityResolver > SAL_CALL getEntityResolver()
            throw (RuntimeException);


        /**
        Specify the ErrorHandler to be used to report errors present in
        the XML document to be parsed.
        */
        virtual void SAL_CALL setErrorHandler(const Reference< XErrorHandler >& eh)
            throw (RuntimeException);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
