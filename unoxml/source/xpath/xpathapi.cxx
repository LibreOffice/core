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

#include "xpathapi.hxx"

#include <stdarg.h>
#include <string.h>

#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlIO.h>

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include "nodelist.hxx"
#include "xpathobject.hxx"

#include <node.hxx>
#include "../dom/document.hxx"

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace css::io;
using namespace css::uno;
using namespace css::xml::dom;
using namespace css::xml::xpath;
using css::lang::XMultiServiceFactory;

namespace XPath
{
    // factory
    Reference< XInterface > CXPathAPI::_getInstance(const Reference< XMultiServiceFactory >& rSMgr)
    {
        return static_cast<XXPathAPI*>(new CXPathAPI(rSMgr));
    }

    // ctor
    CXPathAPI::CXPathAPI(const Reference< XMultiServiceFactory >& rSMgr)
        : m_aFactory(rSMgr)
    {
    }

    OUString CXPathAPI::_getImplementationName()
    {
        return "com.sun.star.comp.xml.xpath.XPathAPI";
    }

    Sequence<OUString> CXPathAPI::_getSupportedServiceNames()
    {
        return { "com.sun.star.xml.xpath.XPathAPI" };
    }

    Sequence< OUString > SAL_CALL CXPathAPI::getSupportedServiceNames()
    {
        return CXPathAPI::_getSupportedServiceNames();
    }

    OUString SAL_CALL CXPathAPI::getImplementationName()
    {
        return CXPathAPI::_getImplementationName();
    }

    sal_Bool SAL_CALL CXPathAPI::supportsService(const OUString& aServiceName)
    {
        return cppu::supportsService(this, aServiceName);
    }

    void SAL_CALL CXPathAPI::registerNS(
            const OUString& aPrefix,
            const OUString& aURI)
    {
        ::osl::MutexGuard const g(m_Mutex);

        m_nsmap.emplace(aPrefix, aURI);
    }

    void SAL_CALL CXPathAPI::unregisterNS(
            const OUString& aPrefix,
            const OUString& aURI)
    {
        ::osl::MutexGuard const g(m_Mutex);

        if ((m_nsmap.find(aPrefix))->second == aURI) {
            m_nsmap.erase(aPrefix);
        }
    }

    // register all namespaces stored in the namespace list for this object
    // with the current xpath evaluation context
    static void lcl_registerNamespaces(
            xmlXPathContextPtr ctx,
            const nsmap_t& nsmap)
    {
        OString oprefix, ouri;
        for (const auto& rEntry : nsmap)
        {
            oprefix = OUStringToOString(rEntry.first,  RTL_TEXTENCODING_UTF8);
            ouri    = OUStringToOString(rEntry.second, RTL_TEXTENCODING_UTF8);
            xmlChar const *p = reinterpret_cast<xmlChar const *>(oprefix.getStr());
            xmlChar const *u = reinterpret_cast<xmlChar const *>(ouri.getStr());
            (void)xmlXPathRegisterNs(ctx, p, u);
        }
    }

    // get all ns decls on a node (and parent nodes, if any)
    static void lcl_collectNamespaces(
            nsmap_t & rNamespaces, Reference< XNode > const& xNamespaceNode)
    {
        DOM::CNode *const pCNode(comphelper::getUnoTunnelImplementation<DOM::CNode>(xNamespaceNode));
        if (!pCNode) { throw RuntimeException(); }

        ::osl::MutexGuard const g(pCNode->GetOwnerDocument().GetMutex());

        xmlNodePtr pNode = pCNode->GetNodePtr();
        while (pNode != nullptr) {
            xmlNsPtr curDef = pNode->nsDef;
            while (curDef != nullptr) {
                const xmlChar* pHref = curDef->href;
                OUString aURI(reinterpret_cast<char const *>(pHref), strlen(reinterpret_cast<char const *>(pHref)), RTL_TEXTENCODING_UTF8);
                const xmlChar* pPre = curDef->prefix;
                OUString aPrefix(reinterpret_cast<char const *>(pPre), strlen(reinterpret_cast<char const *>(pPre)), RTL_TEXTENCODING_UTF8);
                // we could already have this prefix from a child node
                rNamespaces.emplace(aPrefix, aURI);
                curDef = curDef->next;
            }
            pNode = pNode->parent;
        }
    }

    static void lcl_collectRegisterNamespaces(
            CXPathAPI & rAPI, Reference< XNode > const& xNamespaceNode)
    {
        nsmap_t namespaces;
        lcl_collectNamespaces(namespaces, xNamespaceNode);
        for (const auto& rEntry : namespaces)
        {
            rAPI.registerNS(rEntry.first, rEntry.second);
        }
    }

    // register function and variable lookup functions with the current
    // xpath evaluation context
    static void lcl_registerExtensions(
            xmlXPathContextPtr ctx,
            const extensions_t& extensions)
    {
        for (const auto& rExtensionRef : extensions)
        {
            Libxml2ExtensionHandle aHandle = rExtensionRef->getLibxml2ExtensionHandle();
            if ( aHandle.functionLookupFunction != 0 )
            {
                xmlXPathRegisterFuncLookup(ctx,
                    reinterpret_cast<xmlXPathFuncLookupFunc>(
                        sal::static_int_cast<sal_IntPtr>(aHandle.functionLookupFunction)),
                    reinterpret_cast<void*>(
                        sal::static_int_cast<sal_IntPtr>(aHandle.functionData)));
            }
            if ( aHandle.variableLookupFunction != 0 )
            {
                xmlXPathRegisterVariableLookup(ctx,
                    reinterpret_cast<xmlXPathVariableLookupFunc>(
                        sal::static_int_cast<sal_IntPtr>(aHandle.variableLookupFunction)),
                    reinterpret_cast<void*>(
                        sal::static_int_cast<sal_IntPtr>(aHandle.variableData)));
            }
        }
    }

    /**
     * Use an XPath string to select a nodelist.
     */
    Reference< XNodeList > SAL_CALL CXPathAPI::selectNodeList(
            const Reference< XNode >& contextNode,
            const OUString& expr)
    {
        Reference< XXPathObject > xobj = eval(contextNode, expr);
        return xobj->getNodeList();
    }

    /**
     * same as selectNodeList but registers all name space declarations found on namespaceNode
     */
    Reference< XNodeList > SAL_CALL CXPathAPI::selectNodeListNS(
            const Reference< XNode >&  contextNode,
            const OUString& expr,
            const Reference< XNode >&  namespaceNode)
    {
        lcl_collectRegisterNamespaces(*this, namespaceNode);
        return selectNodeList(contextNode, expr);
    }

    /**
     * Same as selectNodeList but returns the first node (if any)
     */
    Reference< XNode > SAL_CALL CXPathAPI::selectSingleNode(
            const Reference< XNode >& contextNode,
            const OUString& expr)
    {
        Reference< XNodeList > aList = selectNodeList(contextNode, expr);
        Reference< XNode > aNode = aList->item(0);
        return aNode;
    }

    /**
     * Same as selectSingleNode but registers all namespaces declared on
     * namespaceNode
     */
    Reference< XNode > SAL_CALL CXPathAPI::selectSingleNodeNS(
            const Reference< XNode >& contextNode,
            const OUString& expr,
            const Reference< XNode >&  namespaceNode )
    {
        lcl_collectRegisterNamespaces(*this, namespaceNode);
        return selectSingleNode(contextNode, expr);
    }

    static OUString make_error_message(const xmlError* pError)
    {
        OUStringBuffer buf;
        if (pError) {
            if (pError->message) {
                buf.appendAscii(pError->message);
            }
            int line = pError->line;
            if (line) {
                buf.append("Line: ");
                buf.append(static_cast<sal_Int32>(line));
                buf.append("\n");
            }
            int column = pError->int2;
            if (column) {
                buf.append("Column: ");
                buf.append(static_cast<sal_Int32>(column));
                buf.append("\n");
            }
        } else {
            buf.append("no error argument!");
        }
        OUString msg = buf.makeStringAndClear();
        return msg;
    }

    extern "C" {

#if defined __GNUC__
        __attribute__ ((format (printf, 2, 3)))
#endif
        static void generic_error_func(void *, const char *format, ...)
        {
            char str[1000];
            va_list args;

            va_start(args, format);
#ifdef _WIN32
#define vsnprintf _vsnprintf
#endif
            vsnprintf(str, sizeof(str), format, args);
            va_end(args);

            SAL_WARN("unoxml", "libxml2 error: " << str);
        }

#if LIBXML_VERSION >= 21200
        static void structured_error_func(void *, const xmlError* error)
#else
        static void structured_error_func(void *, xmlErrorPtr error)
#endif
        {
            SAL_WARN("unoxml", "libxml2 error: " << make_error_message(error));
        }

    } // extern "C"

    /**
     * evaluates an XPath string. relative XPath expressions are evaluated relative to
     * the context Node
     */
    Reference< XXPathObject > SAL_CALL CXPathAPI::eval(
            Reference< XNode > const& xContextNode,
            const OUString& expr)
    {
        if (!xContextNode.is()) { throw RuntimeException(); }

        nsmap_t nsmap;
        extensions_t extensions;

        {
            ::osl::MutexGuard const g(m_Mutex);
            nsmap = m_nsmap;
            extensions = m_extensions;
        }

        // get the node and document
        ::rtl::Reference<DOM::CDocument> const pCDoc(
                dynamic_cast<DOM::CDocument*>( comphelper::getUnoTunnelImplementation<DOM::CNode>(
                        xContextNode->getOwnerDocument())));
        if (!pCDoc.is()) { throw RuntimeException(); }

        DOM::CNode *const pCNode = comphelper::getUnoTunnelImplementation<DOM::CNode>(xContextNode);
        if (!pCNode) { throw RuntimeException(); }

        ::osl::MutexGuard const g(pCDoc->GetMutex()); // lock the document!

        xmlNodePtr const pNode = pCNode->GetNodePtr();
        if (!pNode) { throw RuntimeException(); }
        xmlDocPtr pDoc = pNode->doc;

        /* NB: workaround for #i87252#:
           libxml < 2.6.17 considers it an error if the context
           node is the empty document (i.e. its xpathCtx->doc has no
           children). libxml 2.6.17 does not consider it an error.
           Unfortunately, old libxml prints an error message to stderr,
           which (afaik) cannot be turned off in this case, so we handle it.
        */
        if (!pDoc->children) {
            throw XPathException();
        }

        /* Create xpath evaluation context */
        std::shared_ptr<xmlXPathContext> const xpathCtx(
                xmlXPathNewContext(pDoc), xmlXPathFreeContext);
        if (xpathCtx == nullptr) { throw XPathException(); }

        // set context node
        xpathCtx->node = pNode;
        // error handling
        xpathCtx->error = structured_error_func;
        xmlSetGenericErrorFunc(nullptr, generic_error_func);

        // register namespaces and extension
        lcl_registerNamespaces(xpathCtx.get(), nsmap);
        lcl_registerExtensions(xpathCtx.get(), extensions);

        /* run the query */
        OString o1 = OUStringToOString(expr, RTL_TEXTENCODING_UTF8);
        xmlChar const *pStr = reinterpret_cast<xmlChar const *>(o1.getStr());
        std::shared_ptr<xmlXPathObject> const xpathObj(
                xmlXPathEval(pStr, xpathCtx.get()), xmlXPathFreeObject);
        xmlSetGenericErrorFunc(nullptr, nullptr);
        if (nullptr == xpathObj) {
            // OSL_ENSURE(xpathCtx->lastError == NULL, xpathCtx->lastError->message);
            throw XPathException();
        }
        Reference<XXPathObject> const xObj(
                new CXPathObject(pCDoc, pCDoc->GetMutex(), xpathObj));
        return xObj;
    }

    /**
     * same as eval but registers all namespace declarations found on namespaceNode
     */
    Reference< XXPathObject > SAL_CALL CXPathAPI::evalNS(
            const Reference< XNode >& contextNode,
            const OUString& expr,
            const Reference< XNode >& namespaceNode)
    {
        lcl_collectRegisterNamespaces(*this, namespaceNode);
        return eval(contextNode, expr);
    }

    /**
     * uses the service manager to create an instance of the service denoted by aName.
     * If the returned object implements the XXPathExtension interface, it is added to the list
     * of extensions that are used when evaluating XPath strings with this XPathAPI instance
     */
    void SAL_CALL CXPathAPI::registerExtension(
            const OUString& aName)
    {
        ::osl::MutexGuard const g(m_Mutex);

        // get extension from service manager
        Reference< XXPathExtension > const xExtension(
                m_aFactory->createInstance(aName), UNO_QUERY_THROW);
        m_extensions.push_back(xExtension);
    }

    /**
     * registers the given extension instance to be used by XPath evaluations performed through this
     * XPathAPI instance
     */
    void SAL_CALL CXPathAPI::registerExtensionInstance(
            Reference< XXPathExtension> const& xExtension)
    {
        if (!xExtension.is()) {
            throw RuntimeException();
        }
        ::osl::MutexGuard const g(m_Mutex);
        m_extensions.push_back( xExtension );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
