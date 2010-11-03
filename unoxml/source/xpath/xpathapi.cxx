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

#include "xpathapi.hxx"
#include "nodelist.hxx"
#include "xpathobject.hxx"
#include "../dom/node.hxx"

#include <rtl/ustrbuf.hxx>

#include <libxml/xmlerror.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <stdarg.h>
#include <string.h>


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
        m_nsmap.insert(nsmap_t::value_type(aPrefix, aURI));
    }

    void SAL_CALL CXPathAPI::unregisterNS(
            const OUString& aPrefix,
            const OUString& aURI)
        throw (RuntimeException)
    {
        if ((m_nsmap.find(aPrefix))->second.equals(aURI))
            m_nsmap.erase(aPrefix);
    }

    // register all namespaces stored in the namespace list for this object
    // with the current xpath evaluation context
    static void _registerNamespaces(
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
            i++;
        }
    }

    // get all ns decls on a node (and parent nodes, if any) and register them
    static void _collectNamespaces(
            CXPathAPI* pAPI,
            const Reference< XNode >&  namespaceNode)
    {
        // get namespace decls from node...
        xmlNodePtr pNode = DOM::CNode::getNodePtr(namespaceNode);
        while (pNode != 0) {
            xmlNsPtr curDef = pNode->nsDef;
            while (curDef != 0) {
                const xmlChar* xHref = curDef->href;
                OUString aURI((sal_Char*)xHref, strlen((char*)xHref), RTL_TEXTENCODING_UTF8);
                const xmlChar* xPre = curDef->prefix;
                OUString aPrefix((sal_Char*)xPre, strlen((char*)xPre), RTL_TEXTENCODING_UTF8);
                pAPI->registerNS(aPrefix, aURI);
                curDef = curDef->next;
            }
            pNode = pNode->parent;
        }
    }

    // register function and variable lookup functions with the current
    // xpath evaluation context
    static void _registerExtensions(
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
            i++;
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
        _collectNamespaces(this, namespaceNode);
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
        _collectNamespaces(this, namespaceNode);
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
                OUString::createFromAscii("libxml2 error:\n"));
            buf.appendAscii(str);
            OString msg = OUStringToOString(buf.makeStringAndClear(),
                RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(sal_False, msg.getStr());
        }

        static void structured_error_func(void * userData, xmlErrorPtr error)
        {
            (void) userData;
            ::rtl::OUStringBuffer buf(
                OUString::createFromAscii("libxml2 error:\n"));
            if (error) {
                buf.append(make_error_message(error));
            } else {
                buf.append(OUString::createFromAscii("no error argument!"));
            }
            OString msg = OUStringToOString(buf.makeStringAndClear(),
                RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(sal_False, msg.getStr());
        }

    } // extern "C"

    /**
     * evaluates an XPath string. relative XPath expressions are evaluated relative to
     * the context Node
     */
    Reference< XXPathObject > SAL_CALL CXPathAPI::eval(
            const Reference< XNode >& contextNode,
            const OUString& expr)
        throw (RuntimeException, XPathException)
    {
        xmlXPathContextPtr xpathCtx;
        xmlXPathObjectPtr xpathObj;

        // get the node and document
        xmlNodePtr pNode = DOM::CNode::getNodePtr(contextNode);
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
        xpathCtx = xmlXPathNewContext(pDoc);
        if (xpathCtx == NULL) throw XPathException();

        // set context node
        xpathCtx->node = pNode;
        // error handling
        xpathCtx->error = structured_error_func;
        xmlSetGenericErrorFunc(NULL, generic_error_func);

        // register namespaces and extension
        _registerNamespaces(xpathCtx, m_nsmap);
        _registerExtensions(xpathCtx, m_extensions);

        /* run the query */
        OString o1 = OUStringToOString(expr, RTL_TEXTENCODING_UTF8);
        xmlChar *xStr = (xmlChar*)o1.getStr();
        if ((xpathObj = xmlXPathEval(xStr, xpathCtx)) == NULL) {
            // OSL_ENSURE(xpathCtx->lastError == NULL, xpathCtx->lastError->message);
            xmlXPathFreeContext(xpathCtx);
            throw XPathException();
        }
        xmlXPathFreeContext(xpathCtx);
        Reference< XXPathObject > aObj(new CXPathObject(xpathObj, contextNode));
        return aObj;
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
        _collectNamespaces(this, namespaceNode);
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
        // get extension from service manager
        Reference< XXPathExtension > aExtension(m_aFactory->createInstance(aName), UNO_QUERY_THROW);
        m_extensions.push_back( aExtension );
    }

    /**
     * registers the given extension instance to be used by XPath evaluations performed through this
     * XPathAPI instance
     */
    void SAL_CALL CXPathAPI::registerExtensionInstance(
            const Reference< XXPathExtension>& aExtension)
        throw (RuntimeException)
    {
        if (aExtension.is()) {
            m_extensions.push_back( aExtension );
        } else {
            throw RuntimeException();
        }
    }
}
