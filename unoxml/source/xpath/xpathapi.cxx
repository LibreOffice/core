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

#include <xpathapi.hxx>

#include <stdarg.h>
#include <string.h>

#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <rtl/ustrbuf.hxx>

#include <nodelist.hxx>
#include <xpathobject.hxx>

#include "../dom/node.hxx"
#include "../dom/document.hxx"


using ::com::sun::star::lang::XMultiServiceFactory;


namespace XPath
{
    // factory
    Reference< XInterface > CXPathAPI::_getInstance(const Reference< XMultiServiceFactory >& rSMgr)
    {
        return Reference< XInterface >(static_cast<XXPathAPI*>(new CXPathAPI(rSMgr)));
    }

    // ctor
    CXPathAPI::CXPathAPI(const Reference< XMultiServiceFactory >& rSMgr)
        : m_aFactory(rSMgr)
    {
    }

    const char* CXPathAPI::aImplementationName = "com.sun.star.comp.xml.xpath.XPathAPI";
    const char* CXPathAPI::aSupportedServiceNames[] = {
        "com.sun.star.xml.xpath.XPathAPI",
        NULL
    };

    OUString CXPathAPI::_getImplementationName()
    {
        return OUString::createFromAscii(aImplementationName);
    }

    Sequence<OUString> CXPathAPI::_getSupportedServiceNames()
    {
        Sequence<OUString> aSequence;
        for (int i=0; aSupportedServiceNames[i]!=NULL; i++) {
            aSequence.realloc(i+1);
            aSequence[i]=(OUString::createFromAscii(aSupportedServiceNames[i]));
        }
        return aSequence;
    }

    Sequence< OUString > SAL_CALL CXPathAPI::getSupportedServiceNames()
        throw (RuntimeException)
    {
        return CXPathAPI::_getSupportedServiceNames();
    }

    OUString SAL_CALL CXPathAPI::getImplementationName()
        throw (RuntimeException)
    {
        return CXPathAPI::_getImplementationName();
    }

    sal_Bool SAL_CALL CXPathAPI::supportsService(const OUString& aServiceName)
        throw (RuntimeException)
    {
        Sequence< OUString > supported = CXPathAPI::_getSupportedServiceNames();
        for (sal_Int32 i=0; i<supported.getLength(); i++)
        {
            if (supported[i] == aServiceName) return sal_True;
        }
        return sal_False;
    }

    // -------------------------------------------------------------------

    void SAL_CALL CXPathAPI::registerNS(
            const OUString& aPrefix,
            const OUString& aURI)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);

        m_nsmap.insert(nsmap_t::value_type(aPrefix, aURI));
    }

    void SAL_CALL CXPathAPI::unregisterNS(
            const OUString& aPrefix,
            const OUString& aURI)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);

        if ((m_nsmap.find(aPrefix))->second.equals(aURI)) {
            m_nsmap.erase(aPrefix);
        }
    }

    // register all namespaces stored in the namespace list for this object
    // with the current xpath evaluation context
    static void lcl_registerNamespaces(
            xmlXPathContextPtr ctx,
            const nsmap_t& nsmap)
    {
        nsmap_t::const_iterator i = nsmap.begin();
        OString oprefix, ouri;
        xmlChar *p, *u;
        while (i != nsmap.end())
        {
            oprefix = OUStringToOString(i->first,  RTL_TEXTENCODING_UTF8);
            ouri    = OUStringToOString(i->second, RTL_TEXTENCODING_UTF8);
            p = (xmlChar*)oprefix.getStr();
            u = (xmlChar*)ouri.getStr();
            xmlXPathRegisterNs(ctx, p, u);
            ++i;
        }
    }

    // get all ns decls on a node (and parent nodes, if any)
    static void lcl_collectNamespaces(
            nsmap_t & rNamespaces, Reference< XNode > const& xNamespaceNode)
    {
        DOM::CNode *const pCNode(DOM::CNode::GetImplementation(xNamespaceNode));
        if (!pCNode) { throw RuntimeException(); }

        ::osl::MutexGuard const g(pCNode->GetOwnerDocument().GetMutex());

        xmlNodePtr pNode = pCNode->GetNodePtr();
        while (pNode != 0) {
            xmlNsPtr curDef = pNode->nsDef;
            while (curDef != 0) {
                const xmlChar* xHref = curDef->href;
                OUString aURI((sal_Char*)xHref, strlen((char*)xHref), RTL_TEXTENCODING_UTF8);
                const xmlChar* xPre = curDef->prefix;
                OUString aPrefix((sal_Char*)xPre, strlen((char*)xPre), RTL_TEXTENCODING_UTF8);
                // we could already have this prefix from a child node
                if (rNamespaces.find(aPrefix) == rNamespaces.end())
                {
                    rNamespaces.insert(::std::make_pair(aPrefix, aURI));
                }
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
        for (nsmap_t::const_iterator iter = namespaces.begin();
                iter != namespaces.end(); ++iter)
        {
            rAPI.registerNS(iter->first, iter->second);
        }
    }

    // register function and variable lookup functions with the current
    // xpath evaluation context
    static void lcl_registerExtensions(
            xmlXPathContextPtr ctx,
            const extensions_t& extensions)
    {
        extensions_t::const_iterator i = extensions.begin();
        while (i != extensions.end())
        {
            Libxml2ExtensionHandle aHandle = (*i)->getLibxml2ExtensionHandle();
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
            ++i;
        }
    }

    /**
     * Use an XPath string to select a nodelist.
     */
    Reference< XNodeList > SAL_CALL CXPathAPI::selectNodeList(
            const Reference< XNode >& contextNode,
            const OUString& expr)
        throw (RuntimeException, XPathException)
    {
        Reference< XXPathObject > xobj = eval(contextNode, expr);
        return xobj->getNodeList();
    }

    /**
     * same as selectNodeList but registers all name space decalratiosn found on namespaceNode
     */
    Reference< XNodeList > SAL_CALL CXPathAPI::selectNodeListNS(
            const Reference< XNode >&  contextNode,
            const OUString& expr,
            const Reference< XNode >&  namespaceNode)
        throw (RuntimeException, XPathException)
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
        throw (RuntimeException, XPathException)
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
        throw (RuntimeException, XPathException)
    {
        lcl_collectRegisterNamespaces(*this, namespaceNode);
        return selectSingleNode(contextNode, expr);
    }

    static OUString make_error_message(xmlErrorPtr pError)
    {
        ::rtl::OUStringBuffer buf;
        if (pError->message) {
            buf.appendAscii(pError->message);
        }
        int line = pError->line;
        if (line) {
            buf.appendAscii("Line: ");
            buf.append(static_cast<sal_Int32>(line));
            buf.appendAscii("\n");
        }
        int column = pError->int2;
        if (column) {
            buf.appendAscii("Column: ");
            buf.append(static_cast<sal_Int32>(column));
            buf.appendAscii("\n");
        }
        OUString msg = buf.makeStringAndClear();
        return msg;
    }

    extern "C" {

        static void generic_error_func(void *userData, const char *format, ...)
        {
            (void) userData;
            char str[1000];
            va_list args;

            va_start(args, format);
#ifdef _WIN32
#define vsnprintf _vsnprintf
#endif
            vsnprintf(str, sizeof(str), format, args);
            va_end(args);

            ::rtl::OUStringBuffer buf(
                OUString(RTL_CONSTASCII_USTRINGPARAM("libxml2 error:\n")));
            buf.appendAscii(str);
            OString msg = OUStringToOString(buf.makeStringAndClear(),
                RTL_TEXTENCODING_ASCII_US);
            OSL_FAIL(msg.getStr());
        }

        static void structured_error_func(void * userData, xmlErrorPtr error)
        {
            (void) userData;
            ::rtl::OUStringBuffer buf(
                OUString(RTL_CONSTASCII_USTRINGPARAM("libxml2 error:\n")));
            if (error) {
                buf.append(make_error_message(error));
            } else {
                buf.append(OUString(RTL_CONSTASCII_USTRINGPARAM("no error argument!")));
            }
            OString msg = OUStringToOString(buf.makeStringAndClear(),
                RTL_TEXTENCODING_ASCII_US);
            OSL_FAIL(msg.getStr());
        }

    } // extern "C"

    /**
     * evaluates an XPath string. relative XPath expressions are evaluated relative to
     * the context Node
     */
    Reference< XXPathObject > SAL_CALL CXPathAPI::eval(
            Reference< XNode > const& xContextNode,
            const OUString& expr)
        throw (RuntimeException, XPathException)
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
                dynamic_cast<DOM::CDocument*>( DOM::CNode::GetImplementation(
                        xContextNode->getOwnerDocument())));
        if (!pCDoc.is()) { throw RuntimeException(); }

        DOM::CNode *const pCNode = DOM::CNode::GetImplementation(xContextNode);
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
        ::boost::shared_ptr<xmlXPathContext> const xpathCtx(
                xmlXPathNewContext(pDoc), xmlXPathFreeContext);
        if (xpathCtx == NULL) { throw XPathException(); }

        // set context node
        xpathCtx->node = pNode;
        // error handling
        xpathCtx->error = structured_error_func;
        xmlSetGenericErrorFunc(NULL, generic_error_func);

        // register namespaces and extension
        lcl_registerNamespaces(xpathCtx.get(), nsmap);
        lcl_registerExtensions(xpathCtx.get(), extensions);

        /* run the query */
        OString o1 = OUStringToOString(expr, RTL_TEXTENCODING_UTF8);
        xmlChar *xStr = (xmlChar*)o1.getStr();
        ::boost::shared_ptr<xmlXPathObject> const xpathObj(
                xmlXPathEval(xStr, xpathCtx.get()), xmlXPathFreeObject);
        if (0 == xpathObj) {
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
        throw (RuntimeException, XPathException)
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
        throw (RuntimeException)
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
        throw (RuntimeException)
    {
        if (!xExtension.is()) {
            throw RuntimeException();
        }
        ::osl::MutexGuard const g(m_Mutex);
        m_extensions.push_back( xExtension );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
