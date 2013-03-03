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

#ifndef XPATH_XPATHAPI_HXX
#define XPATH_XPATHAPI_HXX

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
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/xml/xpath/XXPathExtension.hpp>
#include <com/sun/star/xml/xpath/Libxml2ExtensionHandle.hpp>
#include <com/sun/star/xml/xpath/XPathException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>


using ::rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::xpath;

namespace XPath
{
    typedef std::map<OUString, OUString> nsmap_t;
    typedef std::vector< Reference<XXPathExtension> > extensions_t;

    typedef ::cppu::WeakImplHelper2
        <   XXPathAPI
        ,   ::com::sun::star::lang::XServiceInfo
        > CXPathAPI_Base;

    class  CXPathAPI
        : public CXPathAPI_Base
    {

    private:
        ::osl::Mutex m_Mutex;
        nsmap_t m_nsmap;
        const Reference< ::com::sun::star::lang::XMultiServiceFactory > m_aFactory;
        extensions_t m_extensions;

    public:
        // ctor
        CXPathAPI(
            const Reference< ::com::sun::star::lang::XMultiServiceFactory >&
                rSMgr);

        // call for factory
        static Reference< XInterface > getInstance(
            const Reference < ::com::sun::star::lang::XMultiServiceFactory >&
                xFactory);

        // static helpers for service info and component management
        static const char* aImplementationName;
        static const char* aSupportedServiceNames[];
        static OUString _getImplementationName();
        static Sequence< OUString > _getSupportedServiceNames();
        static Reference< XInterface > _getInstance(
            const Reference< ::com::sun::star::lang::XMultiServiceFactory >&
                rSMgr);

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
