/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: elementlist.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:01:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "elementlist.hxx"

namespace DOM
{

    CElementList::CElementList(const CElement* aElement, const OUString& aName)
        : m_pElement(aElement)
        , m_aName(aName)
        , xURI(0)
    {
        OString o1 = OUStringToOString(aName, RTL_TEXTENCODING_UTF8);
        xName = new xmlChar[o1.getLength()];
        strcpy((char*)xName, o1.getStr());
    }
    CElementList::CElementList(const CElement* aElement, const OUString& aName, const OUString& aURI)
        : m_pElement(aElement)
        , m_aName(aName)
        , m_aURI(aURI)
    {
        OString o1 = OUStringToOString(aName, RTL_TEXTENCODING_UTF8);
        xName = new xmlChar[o1.getLength()];
        strcpy((char*)xName, o1.getStr());
        OString o2 = OUStringToOString(aURI, RTL_TEXTENCODING_UTF8);
        xURI = new xmlChar[o2.getLength()];
        strcpy((char*)xURI, o2.getStr());
    }

    void CElementList::buildlist(nodevector& v, xmlNodePtr pNode, sal_Bool start)
    {
        while (pNode != NULL)
        {
            if (strcmp((char*)pNode->name, (char*)xName)==0)
            {
                if (xURI == NULL)
                    v.push_back(pNode);
                else
                    if (pNode->ns != NULL && strcmp((char*)pNode->ns->href, (char*)xURI) == 0)
                        v.push_back(pNode);
            }
            if (pNode->children != NULL) buildlist(v,pNode->children, sal_False);
            if (!start) pNode = pNode->next;
            else break;
        }
    }

    /**
    The number of nodes in the list.
    */
    sal_Int32 SAL_CALL CElementList::getLength() throw (RuntimeException)
    {
        // this has to be 'live'
        // XXX work with listeners at some point
        nodevector v;
        buildlist(v, static_cast<const CNode*>(m_pElement)->m_aNodePtr);
        return v.size();
    }
    /**
    Returns the indexth item in the collection.
    */
    Reference< XNode > SAL_CALL CElementList::item(sal_Int32 index) throw (RuntimeException)
    {
        if (index < 0) throw RuntimeException();
        nodevector v;
        buildlist(v, static_cast<const CNode*>(m_pElement)->m_aNodePtr);
        return Reference< XNode >(CNode::get(v[index]));
    }
}