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

#include <element.hxx>


namespace DOM
{

    static xmlChar* lcl_initXmlString(::rtl::OUString const& rString)
    {
        ::rtl::OString const os =
            ::rtl::OUStringToOString(rString, RTL_TEXTENCODING_UTF8);
        xmlChar *const pRet = new xmlChar[os.getLength() + 1];
        strcpy(reinterpret_cast<char*>(pRet), os.getStr());
        return pRet;
    }

    CElementList::CElementList(::rtl::Reference<CElement> const& pElement,
            OUString const& rName, OUString const*const pURI)
        : m_pElement(pElement)
        , m_pName(lcl_initXmlString(rName))
        , m_pURI((pURI) ? lcl_initXmlString(*pURI) : 0)
        , m_bRebuild(true)
    {
        registerListener(*m_pElement);
    }

    void CElementList::registerListener(CElement & rElement)
    {
        try {
            Reference< XEventTarget > const xTarget(
                    static_cast<XElement*>(& rElement), UNO_QUERY_THROW);
            OUString aType = OUString::createFromAscii("DOMSubtreeModified");
            sal_Bool capture = sal_False;
            xTarget->addEventListener(aType,
                    Reference< XEventListener >(this), capture);
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
                m_bRebuild = false; // don't rebuild until tree is mutated
            }
        }

        while (pNode != NULL )
        {
            if (pNode->type == XML_ELEMENT_NODE &&
                (strcmp((char*)pNode->name, (char*)m_pName.get()) == 0))
            {
                if (!m_pURI) {
                    m_nodevector.push_back(pNode);
                } else {
                    if (pNode->ns != NULL && (0 ==
                         strcmp((char*)pNode->ns->href, (char*)m_pURI.get())))
                    {
                        m_nodevector.push_back(pNode);
                    }
                }
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
        buildlist(static_cast<const CNode*>(m_pElement.get())->m_aNodePtr);
        return m_nodevector.size();
    }
    /**
    Returns the indexth item in the collection.
    */
    Reference< XNode > SAL_CALL CElementList::item(sal_Int32 index)
        throw (RuntimeException)
    {
        if (index < 0) throw RuntimeException();
        buildlist(static_cast<const CNode*>(m_pElement.get())->m_aNodePtr);
        if (m_nodevector.size() <= static_cast<size_t>(index)) {
            throw RuntimeException();
        }
        Reference< XNode > const xRet(
            CNode::getCNode(m_nodevector[index]).get());
        return xRet;
    }

    // tree mutations can change the list
    void SAL_CALL CElementList::handleEvent(Reference< XEvent > const&)
        throw (RuntimeException)
    {
        m_bRebuild = true;
    }
}
