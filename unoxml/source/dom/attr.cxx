/*************************************************************************
 *
 *  $RCSfile: attr.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lo $ $Date: 2004-01-28 16:30:50 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "attr.hxx"
#include "element.hxx"
#include <com/sun/star/xml/dom/DOMException.hdl>

namespace DOM
{
    CAttr::CAttr(const xmlAttrPtr pAttr)
    {
        m_aAttrPtr = pAttr;
        m_aNodeType = NodeType_ATTRIBUTE_NODE;
        init_node((xmlNodePtr)pAttr);
    }

    OUString SAL_CALL CAttr::getNodeName()
        throw (RuntimeException)
    {
        return getName();
    }
    OUString SAL_CALL CAttr::getNodeValue()
        throw (RuntimeException)
    {
        return getValue();
    }


    /**
    Returns the name of this attribute.
    */
    OUString SAL_CALL CAttr::getName() throw (RuntimeException)
    {
        OUString aName;
        if (m_aAttrPtr != NULL)
        {
            aName = OUString((char*)m_aAttrPtr->name, strlen((char*)m_aAttrPtr->name), RTL_TEXTENCODING_UTF8);
        }
        return aName;
    }

    /**
    The Element node this attribute is attached to or null if this 
    attribute is not in use.
    */
    Reference< XElement > SAL_CALL CAttr::getOwnerElement()
        throw (RuntimeException)
    {
        Reference< XElement > aElement;
        if (m_aAttrPtr != NULL && m_aAttrPtr->parent != NULL)
        {
            aElement = Reference< XElement >(static_cast< CElement* >(CNode::get(m_aAttrPtr->parent)));
        }
        return aElement;
    }

    /**
    If this attribute was explicitly given a value in the original 
    document, this is true; otherwise, it is false.
    */
    sal_Bool SAL_CALL CAttr::getSpecified()
        throw (RuntimeException)
    {
        // XXX what is this supposed do exactly?
        return sal_False;
    }

    /**
    On retrieval, the value of the attribute is returned as a string.
    */
    OUString SAL_CALL CAttr::getValue()
        throw (RuntimeException)
    {
        OUString aName;
        if (m_aAttrPtr != NULL && m_aAttrPtr->children != NULL)
        {
            aName = OUString((char*)m_aAttrPtr->children->content, strlen((char*)m_aAttrPtr->children->content),
                RTL_TEXTENCODING_UTF8);
        }
        return aName;
    }

    /**
    Sets the value of the attribute from a string.
    */
    void SAL_CALL CAttr::setValue(const OUString& value)
        throw (RuntimeException)
    {   
        OString o1 = OUStringToOString(value, RTL_TEXTENCODING_UTF8);
        xmlChar* xValue = (xmlChar*)o1.getStr();
        // xmlChar* xName = OUStringToOString(m_aAttrPtr->name, RTL_TEXTENCODING_UTF8).getStr();
        // this does not work if the attribute was created anew
        // xmlNodePtr pNode = m_aAttrPtr->parent;
        // xmlSetProp(pNode, m_aAttrPtr->name, xValue);
        xmlChar *buffer = xmlEncodeEntitiesReentrant(m_aAttrPtr->doc, xValue);
        m_aAttrPtr->children = xmlStringGetNodeList(m_aAttrPtr->doc, buffer);
        xmlNodePtr tmp = m_aAttrPtr->children;
        while (tmp != NULL) {
            tmp->parent = (xmlNodePtr) m_aNodePtr;
            tmp->doc = m_aAttrPtr->doc;
            if (tmp->next == NULL)
                m_aNodePtr->last = tmp;
            tmp = tmp->next;
        }
        xmlFree(buffer);            
    }

}
