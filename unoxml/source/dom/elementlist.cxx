/*************************************************************************
 *
 *  $RCSfile: elementlist.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lo $ $Date: 2004-01-28 16:31:27 $
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