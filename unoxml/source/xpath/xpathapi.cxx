/*************************************************************************
 *
 *  $RCSfile: xpathapi.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lo $ $Date: 2004-01-28 16:32:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "xpathapi.hxx"
#include "nodelist.hxx"
#include "xpathobject.hxx"

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

namespace XPath
{

    Reference< XInterface > CXPathAPI::_getInstance(const Reference< XMultiServiceFactory >& rSMgr)
    {
        // XXX
        // return static_cast< XXPathAPI* >(new CXPathAPI());
        return Reference< XInterface >(static_cast<XXPathAPI*>(new CXPathAPI()));
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

    void SAL_CALL CXPathAPI::registerNS(const OUString& aPrefix, const OUString& aURI)
        throw (RuntimeException)
    {
        m_nsmap.insert(nsmap_t::value_type(aPrefix, aURI));                
    }

    void SAL_CALL CXPathAPI::unregisterNS(const OUString& aPrefix, const OUString& aURI)
        throw (RuntimeException)
    {
        if ((m_nsmap.find(aPrefix))->second.equals(aURI))
            m_nsmap.erase(aPrefix);
        
    }
    
    static void _registerNamespaces(xmlXPathContextPtr ctx, const nsmap_t& nsmap)
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

    /**
    Use an XPath string to select a nodelist.
    */
    Reference< XNodeList > SAL_CALL CXPathAPI::selectNodeList(
            const Reference< XNode >& contextNode, 
            const OUString& str)
        throw (RuntimeException)
    {

        xmlXPathContextPtr xpathCtx; 
        xmlXPathObjectPtr xpathObj; 

        // get the node and document
        Reference< XUnoTunnel > tunnel(contextNode, UNO_QUERY);
        xmlNodePtr pNode = (xmlNodePtr)tunnel->getSomething(Sequence< sal_Int8 >());
        xmlDocPtr pDoc = pNode->doc;

        /* Create xpath evaluation context */
        xpathCtx = xmlXPathNewContext(pDoc);
        if (xpathCtx == NULL)throw RuntimeException();
        _registerNamespaces(xpathCtx, m_nsmap);

        OString o1 = OUStringToOString(str, RTL_TEXTENCODING_UTF8);
        xmlChar *xStr = (xmlChar*)o1.getStr();
        
        /* run the query */
        if ((xpathObj = xmlXPathEval(xStr, xpathCtx)) == NULL)
        {
            xmlXPathFreeContext(xpathCtx);
            throw RuntimeException();
        }
        Reference< XNodeList > aList(new CNodeList(xpathObj));
        xmlXPathFreeContext(xpathCtx);        
        return aList;
    }

    /**
    Use an XPath string to select a nodelist.
    */
    Reference< XNodeList > SAL_CALL CXPathAPI::selectNodeListNS(
            const Reference< XNode >& contextNode, 
            const OUString& str, 
            const Reference< XNode >&  namespaceNode)
        throw (RuntimeException)
    {
        return Reference< XNodeList>();
    }

    /**
    Use an XPath string to select a single node.
    */
    Reference< XNode > SAL_CALL CXPathAPI::selectSingleNode(
            const Reference< XNode >& contextNode, 
            const OUString& str)
        throw (RuntimeException)
    {
        Reference< XNodeList > aList = selectNodeList(contextNode, str);
        Reference< XNode > aNode = aList->item(0);
        return aNode;
    }

    /**
    Use an XPath string to select a single node.
    */
    Reference< XNode > SAL_CALL CXPathAPI::selectSingleNodeNS(
            const Reference< XNode >& contextNode, 
            const OUString& str, 
            const Reference< XNode >&  namespaceNode)
        throw (RuntimeException)
    {
        return Reference< XNode >();
    }


    Reference< XXPathObject > SAL_CALL CXPathAPI::eval(const Reference< XNode >& contextNode, const OUString& str)
            throw (RuntimeException)
    {
        xmlXPathContextPtr xpathCtx; 
        xmlXPathObjectPtr xpathObj; 

        // get the node and document
        Reference< XUnoTunnel > tunnel(contextNode, UNO_QUERY);
        xmlNodePtr pNode = (xmlNodePtr)tunnel->getSomething(Sequence< sal_Int8 >());
        xmlDocPtr pDoc = pNode->doc;

        /* Create xpath evaluation context */
        xpathCtx = xmlXPathNewContext(pDoc);
        if (xpathCtx == NULL)throw RuntimeException();

        _registerNamespaces(xpathCtx, m_nsmap);
        
        OString o1 = OUStringToOString(str, RTL_TEXTENCODING_UTF8);
        xmlChar *xStr = (xmlChar*)o1.getStr();


        /* run the query */
        if ((xpathObj = xmlXPathEval(xStr, xpathCtx)) == NULL) {            
            // OSL_ENSURE(xpathCtx->lastError == NULL, xpathCtx->lastError->message);
            xmlXPathFreeContext(xpathCtx);
            throw RuntimeException();
        }
        xmlXPathFreeContext(xpathCtx);

        Reference< XXPathObject > aObj(new CXPathObject(xpathObj));        
        return aObj;
    }

    Reference< XXPathObject > SAL_CALL CXPathAPI::evalNS(const Reference< XNode >& contextNode, const OUString& str, const Reference< XNode >&  namespaceNode)
            throw (RuntimeException)
    {
        return Reference< XXPathObject>();
    }



}
