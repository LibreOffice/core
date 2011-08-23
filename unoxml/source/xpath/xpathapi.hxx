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

#ifndef _XPATHAPI_HXX
#define _XPATHAPI_HXX

#include <map>
#include <vector>

#include <sal/types.h>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/xml/xpath/XXPathExtension.hpp>
#include <com/sun/star/xml/xpath/Libxml2ExtensionHandle.hpp>
#include <com/sun/star/xml/xpath/XPathException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "libxml/tree.h"

using ::rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::xpath;

namespace XPath
{
    typedef std::map<OUString, OUString> nsmap_t;
    typedef std::vector< Reference<XXPathExtension> > extensions_t;

    class  CXPathAPI
        : public ::cppu::WeakImplHelper2< XXPathAPI, XServiceInfo >
    {

    private:
        nsmap_t m_nsmap;
        const Reference < XMultiServiceFactory > m_aFactory;
        extensions_t m_extensions;

    public:
        // ctor
        CXPathAPI(const Reference< XMultiServiceFactory >& rSMgr);

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


        // --- XXPathAPI ---

        virtual void SAL_CALL registerNS(const OUString& aPrefix, const OUString& aURI)
            throw (RuntimeException);

        virtual void SAL_CALL unregisterNS(const OUString& aPrefix, const OUString& aURI)
            throw (RuntimeException);

        /**
        Use an XPath string to select a nodelist.
        */
        virtual Reference< XNodeList > SAL_CALL selectNodeList(const Reference< XNode >& contextNode, const OUString& str)
            throw (RuntimeException, XPathException);

        /**
        Use an XPath string to select a nodelist.
        */
        virtual Reference< XNodeList > SAL_CALL selectNodeListNS(const Reference< XNode >& contextNode, const OUString& str, const Reference< XNode >&  namespaceNode)
            throw (RuntimeException, XPathException);

        /**
        Use an XPath string to select a single node.
        */
        virtual Reference< XNode > SAL_CALL selectSingleNode(const Reference< XNode >& contextNode, const OUString& str)
            throw (RuntimeException, XPathException);

        /**
        Use an XPath string to select a single node.
        */
        virtual Reference< XNode > SAL_CALL selectSingleNodeNS(const Reference< XNode >& contextNode, const OUString& str, const Reference< XNode >&  namespaceNode)
            throw (RuntimeException, XPathException);

        virtual Reference< XXPathObject > SAL_CALL eval(const Reference< XNode >& contextNode, const OUString& str)
            throw (RuntimeException, XPathException);

        virtual Reference< XXPathObject > SAL_CALL evalNS(const Reference< XNode >& contextNode, const OUString& str, const Reference< XNode >&  namespaceNode)
            throw (RuntimeException, XPathException);

        virtual void SAL_CALL registerExtension(const OUString& aName) throw (RuntimeException);
        virtual void SAL_CALL registerExtensionInstance(const Reference< XXPathExtension>& aExtension) throw (RuntimeException);

    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
