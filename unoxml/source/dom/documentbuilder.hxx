/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: documentbuilder.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _DOCUMENTBUILDER_HXX
#define _DOCUMENTBUILDER_HXX

#include <sal/types.h>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XDOMImplementation.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "libxml/tree.h"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::io;

namespace DOM
{
    class  CDocumentBuilder
        : public ::cppu::WeakImplHelper2< XDocumentBuilder, XServiceInfo >
    {
    private:
        Reference< XMultiServiceFactory > m_aFactory;

    public:

        // ctor
        CDocumentBuilder(const Reference< XMultiServiceFactory >& xFactory);

        // call for factory
        static Reference< XInterface > getInstance(const Reference < XMultiServiceFactory >& xFactory);

        // static helpers for service info and component management
        static const char* aImplementationName;
        static const char* aSupportedServiceNames[];
        static OUString _getImplementationName();
        static Sequence< OUString > _getSupportedServiceNames();
        static Reference< XInterface > _getInstance(const Reference< XMultiServiceFactory >& rSMgr);

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
            throw (RuntimeException);

        /**
        Parse the content of the given URI as an XML document and return
        a new DOM Document object.
        */
        //virtual XDocument SAL_CALL parse(const string& uri);

        /**
        Specify the EntityResolver to be used to resolve entities present
        in the XML document to be parsed.
        */
        // virtual void SAL_CALL setEntityResolver(const XEntityResolver& er);

        /**
        Specify the ErrorHandler to be used to report errors present in
        the XML document to be parsed.
        */
        //virtual void SAL_CALL setErrorHandler(const XErrorHandler& eh);
    };
}

#endif
