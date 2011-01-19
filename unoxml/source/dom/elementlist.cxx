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

#include "elementlist.hxx"

#include <string.h>

namespace DOM
{

    CElementList::CElementList(const CElement* aElement, const OUString& aName)
        : m_pElement(aElement)
        , m_aName(aName)
        , xURI(0)
        , m_bRebuild(sal_True)
    {
        OString o1 = OUStringToOString(aName, RTL_TEXTENCODING_UTF8);
        xName = new xmlChar[o1.getLength()];
        strcpy((char*)xName, o1.getStr());
        registerListener(aElement);
    }

    CElementList::CElementList(const CElement* aElement, const OUString& aName, const OUString& aURI)
        : m_pElement(aElement)
        , m_aName(aName)
        , m_aURI(aURI)
        , m_bRebuild(sal_True)
    {
        OString o1 = OUStringToOString(aName, RTL_TEXTENCODING_UTF8);
        xName = new xmlChar[o1.getLength()];
        strcpy((char*)xName, o1.getStr());
        OString o2 = OUStringToOString(aURI, RTL_TEXTENCODING_UTF8);
        xURI = new xmlChar[o2.getLength()];
        strcpy((char*)xURI, o2.getStr());
        registerListener(aElement);
    }

    void CElementList::registerListener(const CElement* pElement)
    {
        try {
            // get the XNode
            Reference< XNode > const xNode( CNode::getCNode(
                    static_cast<const CNode*>(pElement)->m_aNodePtr).get() );
            Reference< XEventTarget > xTarget(xNode, UNO_QUERY_THROW);
            OUString aType = OUString::createFromAscii("DOMSubtreeModified");
            sal_Bool capture = sal_False;
            xTarget->addEventListener(aType, Reference< XEventListener >(this), capture);
        } catch (Exception &e){
            OString aMsg("Exception caught while registering NodeList as listener:\n");
            aMsg += OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US);
            OSL_ENSURE(sal_False, aMsg.getStr());
        }
    }

    void CElementList::buildlist(xmlNodePtr pNode, sal_Bool start)
    {
        // bail out if no rebuild is needed
        if (start) {
            if (!m_bRebuild)
            {
                return;
            } else {
                m_nodevector.erase(m_nodevector.begin(), m_nodevector.end());
                m_bRebuild = sal_False; // don't rebuild until tree is mutated
            }
        }

        while (pNode != NULL )
        {
            if (pNode->type == XML_ELEMENT_NODE &&
                strcmp((char*)pNode->name, (char*)xName)==0)
            {
                if (xURI == NULL)
                    m_nodevector.push_back(pNode);
                else
                    if (pNode->ns != NULL && strcmp((char*)pNode->ns->href, (char*)xURI) == 0)
                        m_nodevector.push_back(pNode);
            }
            if (pNode->children != NULL) buildlist(pNode->children, sal_False);

            if (!start) pNode = pNode->next;
            else break; // fold back
        }
    }

    /**
    The number of nodes in the list.
    */
    sal_Int32 SAL_CALL CElementList::getLength() throw (RuntimeException)
    {
        // this has to be 'live'
        buildlist(static_cast<const CNode*>(m_pElement)->m_aNodePtr);
        return m_nodevector.size();
    }
    /**
    Returns the indexth item in the collection.
    */
    Reference< XNode > SAL_CALL CElementList::item(sal_Int32 index) throw (RuntimeException)
    {
        if (index < 0) throw RuntimeException();
        buildlist(static_cast<const CNode*>(m_pElement)->m_aNodePtr);
        Reference< XNode > const xRet(
            CNode::getCNode(m_nodevector[index]).get());
        return xRet;
    }

    // tree mutations can change the list
    void SAL_CALL CElementList::handleEvent(const Reference< XEvent >& evt) throw (RuntimeException)
    {
      Reference< XEvent > aEvent = evt;
        m_bRebuild = sal_True;
    }
}
