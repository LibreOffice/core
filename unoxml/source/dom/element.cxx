/*************************************************************************
 *
 *  $RCSfile: element.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lo $ $Date: 2004-01-28 16:31:23 $
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

#include "element.hxx"
#include "attr.hxx"
#include "elementlist.hxx"
#include "attributesmap.hxx"

namespace DOM
{

    CElement::CElement(const xmlNodePtr aNodePtr)
    {
        m_aNodeType = NodeType_ELEMENT_NODE;
        init_node(aNodePtr);
    }

    /**
    Retrieves an attribute value by name.
    */
    OUString CElement::getAttribute(const OUString& name)
        throw (RuntimeException)
    {
        OUString aValue;
        // search properties
        if (m_aNodePtr != NULL)
        {
            OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
            xmlChar *xName = (xmlChar*)o1.getStr();
            aValue = OUString((sal_Char*)xmlGetProp(m_aNodePtr, xName), strlen((char*)xName), RTL_TEXTENCODING_UTF8);
        }
        return aValue;
    }

    /**
    Retrieves an attribute node by name.
    */
    Reference< XAttr > CElement::getAttributeNode(const OUString& name)
        throw (RuntimeException)
    {
        Reference< XAttr > aAttr;
        if (m_aNodePtr != NULL)
        {
            OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
            xmlChar *xName = (xmlChar*)o1.getStr();
            xmlAttrPtr pAttr = xmlHasProp(m_aNodePtr, xName);
            aAttr = Reference< XAttr >(static_cast< CAttr* >(CNode::get((xmlNodePtr)pAttr)));
        }
        return aAttr;
    }

    /**
    Retrieves an Attr node by local name and namespace URI.
    */
    Reference< XAttr > CElement::getAttributeNodeNS(
            const OUString& namespaceURI, const OUString& localName)
        throw (RuntimeException)
    {
        Reference< XAttr > aAttr;
        if (m_aNodePtr != NULL)
        {
            OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
            xmlChar *xName = (xmlChar*)o1.getStr();
            OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
            xmlChar *xNS = (xmlChar*)o2.getStr();
            xmlAttrPtr pAttr = xmlHasNsProp(m_aNodePtr, xName, xNS);
            aAttr = Reference< XAttr >(static_cast< CAttr* >(CNode::get((xmlNodePtr)pAttr)));
        }
        return aAttr;
    }

    /**
    Retrieves an attribute value by local name and namespace URI.
    */
    OUString CElement::getAttributeNS(const OUString& namespaceURI, const OUString& localName)
        throw (RuntimeException)
    {
        OUString aValue;
        // search properties
        if (m_aNodePtr != NULL)
        {
            OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
            xmlChar *xName = (xmlChar*)o1.getStr();
            OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
            xmlChar *xNS = (xmlChar*)o2.getStr();
            xmlChar *xValue = (xmlChar*)xmlGetNsProp(m_aNodePtr, xName, xNS);
            aValue = OUString((sal_Char*)xValue, strlen((char*)xValue), RTL_TEXTENCODING_UTF8);
        }
        return aValue;
    }

    /**
    Returns a NodeList of all descendant Elements with a given tag name, 
    in the order in which they are 
    encountered in a preorder traversal of this Element tree.
    */
    Reference< XNodeList > CElement::getElementsByTagName(const OUString& name)
        throw (RuntimeException)
    {
        Reference< XNodeList > aList = Reference< XNodeList >(new CElementList(this, name));
        return aList;
    }

    /**
    Returns a NodeList of all the descendant Elements with a given local 
    name and namespace URI in the order in which they are encountered in
    a preorder traversal of this Element tree.
    */
    Reference< XNodeList > CElement::getElementsByTagNameNS(const OUString& namespaceURI, 
            const OUString& localName)
        throw (RuntimeException)
    {
        Reference< XNodeList > aList = Reference< XNodeList >(new CElementList(this, localName, namespaceURI));
        return aList;
    }

    /**
    The name of the element.
    */
    OUString CElement::getTagName()
        throw (RuntimeException)
    {
        OUString aName;
        if (m_aNodePtr != NULL)
        {
            aName = OUString((sal_Char*)m_aNodePtr->name, strlen((char*)m_aNodePtr->name), RTL_TEXTENCODING_UTF8);
        }
        return aName;
    }


    /**
    Returns true when an attribute with a given name is specified on this
    element or has a default value, false otherwise.
    */
    sal_Bool CElement::hasAttribute(const OUString& name)
        throw (RuntimeException)
    {
        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        return (m_aNodePtr != NULL && xmlHasProp(m_aNodePtr, xName) != NULL);
    }

    /**
    Returns true when an attribute with a given local name and namespace
    URI is specified on this element or has a default value, false otherwise.
    */
    sal_Bool CElement::hasAttributeNS(const OUString& namespaceURI, const OUString& localName)
        throw (RuntimeException)
    {
        OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
        xmlChar *xNs = (xmlChar*)o2.getStr();
        return (m_aNodePtr != NULL && xmlHasNsProp(m_aNodePtr, xName, xNs) != NULL);
    }

    /**
    Removes an attribute by name.
    */
    void CElement::removeAttribute(const OUString& name)
        throw (RuntimeException)
    {
        xmlChar *xName = (xmlChar*)OUStringToOString(name, RTL_TEXTENCODING_UTF8).getStr();
        if (m_aNodePtr != NULL) {
            xmlUnsetProp(m_aNodePtr, xName);
        }
    }

    /**
    Removes an attribute by local name and namespace URI.
    */
    void CElement::removeAttributeNS(const OUString& namespaceURI, const OUString& localName)
        throw (RuntimeException)
    {
        OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
        xmlChar *xURI = (xmlChar*)o2.getStr();
        if (m_aNodePtr != NULL) {
            // XXX
            xmlNsPtr pNs = xmlSearchNsByHref(m_aNodePtr->doc, m_aNodePtr, xURI);
            xmlUnsetNsProp(m_aNodePtr, pNs, xName);
        }
    }

    /**
    Removes the specified attribute node.
    */
    Reference< XAttr > CElement::removeAttributeNode(const Reference< XAttr >& oldAttr)
        throw (RuntimeException)
    {
        Reference< XAttr > aAttr;
        if(m_aNodePtr != NULL)
        {
            Reference< XUnoTunnel > tunnel(oldAttr, UNO_QUERY);
            xmlAttrPtr pAttr = (xmlAttrPtr)tunnel->getSomething(Sequence< sal_Int8 >());
            xmlRemoveProp(pAttr);
            aAttr = Reference< XAttr >(static_cast< CAttr* >(CNode::get((xmlNodePtr)pAttr)));
        }
        return aAttr;
    }

    /**
    Adds a new attribute node.
    */
    Reference< XAttr > CElement::_setAttributeNode(const Reference< XAttr >& newAttr, sal_Bool bNS)
        throw (RuntimeException)
    {
        Reference< XAttr > aAttr;
        if (m_aNodePtr != NULL) 
        {
            // check whether the attrib belongs to this document
            Reference< XDocument > newDoc(newAttr->getOwnerDocument(), UNO_QUERY);
            Reference< XDocument > oldDoc(CNode::getOwnerDocument(), UNO_QUERY);
            if (newDoc != oldDoc) {
                throw RuntimeException();
            }
            
            // get the implementation
            Reference< XUnoTunnel > tunnel(newAttr, UNO_QUERY);
            xmlAttrPtr pAttr = (xmlAttrPtr)tunnel->getSomething(Sequence< sal_Int8 >());

            // check whether the attribute is not in use by another element
            xmlNsPtr pNs = NULL;
            if (pAttr->parent != NULL)
                if(strcmp((char*)pAttr->parent->name, "__private") == NULL 
                    && pNs && pAttr->ns != NULL)
                {
                    pNs = xmlSearchNs(m_aNodePtr->doc, m_aNodePtr, pAttr->ns->prefix);
                    if (pNs == NULL || strcmp((char*)pNs->href, (char*)pAttr->ns->href) !=0 )
                        pNs = xmlNewNs(m_aNodePtr, pAttr->ns->href, pAttr->ns->href);
                else
                    throw RuntimeException();
            }

            xmlAttrPtr res = NULL;
            
            if (bNS)
                res = xmlNewNsProp(m_aNodePtr, pNs, pAttr->name, pAttr->children->content);
            else
                res = xmlNewProp(m_aNodePtr, pAttr->name, pAttr->children->content);

            // free carrier node ...
            if(pAttr->parent != NULL && strcmp((char*)pAttr->parent->name, "__private")== 0)
                xmlFreeNode(pAttr->parent);
            // ... remove the old attr from the node cache
            CNode::remove((xmlNodePtr)pAttr);

            // get the new attr node
            aAttr = Reference< XAttr >(static_cast< CAttr*  >(CNode::get((xmlNodePtr)res)));
        }
        return aAttr;
    }

    Reference< XAttr > CElement::setAttributeNode(const Reference< XAttr >& newAttr)
        throw (RuntimeException)
    {
        return _setAttributeNode(newAttr, sal_False);
    }

    /**
    Adds a new attribute.
    */
    Reference< XAttr > CElement::setAttributeNodeNS(const Reference< XAttr >& newAttr)
        throw (RuntimeException)
    {
        return _setAttributeNode(newAttr, sal_True);
    }

    /**
    Adds a new attribute.
    */
    void CElement::setAttribute(const OUString& name, const OUString& value)
        throw (RuntimeException)
    {
        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        OString o2 = OUStringToOString(value, RTL_TEXTENCODING_UTF8);
        xmlChar *xValue = (xmlChar*)o2.getStr();
        if (m_aNodePtr != NULL)
        {
            xmlNewProp(m_aNodePtr, xName, xValue);
        }        
    }

    /**
    Adds a new attribute. 
    */
    void CElement::setAttributeNS(
            const OUString& namespaceURI, const OUString& qualifiedName, const OUString& value)
        throw (RuntimeException)
    {       
        if (namespaceURI.getLength() == 0) throw RuntimeException();
        
        OString o1, o2, o3, o4, o5;
        xmlChar *xPrefix = NULL;
        xmlChar *xLName = NULL;
        o1 = OUStringToOString(qualifiedName, RTL_TEXTENCODING_UTF8);
        xmlChar *xQName = (xmlChar*)o1.getStr();
        sal_Int32 idx = qualifiedName.indexOf(':');
        if (idx != -1)
        {            
            o2 = OUStringToOString(
                qualifiedName.copy(0,idx), 
                RTL_TEXTENCODING_UTF8);
            xPrefix = (xmlChar*)o2.getStr();
            o3 = OUStringToOString(
                qualifiedName.copy(idx+1), 
                RTL_TEXTENCODING_UTF8);
            xLName = (xmlChar*)o3.getStr();
        }  else {
            xPrefix = (xmlChar*)"";
            xLName = xQName;
        }
        o4 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
        o5 = OUStringToOString(value, RTL_TEXTENCODING_UTF8);
        xmlChar *xURI= (xmlChar*)o4.getStr();
        xmlChar *xValue = (xmlChar*)o5.getStr();
        if (m_aNodePtr != NULL)
        {
            //find the right namespace
            xmlNsPtr pNs = xmlSearchNs(m_aNodePtr->doc, m_aNodePtr, xPrefix);
            // if no namespace found, create a new one
            if (pNs == NULL)
                pNs = xmlNewNs(m_aNodePtr, xURI, xPrefix);

            if (strcmp((char*)pNs->href, (char*)xURI) == 0)
            {
                // found namespace matches
                xmlNewNsProp(m_aNodePtr, pNs, xLName, xValue);
            } else {
                // ambigious ns prefix
                throw RuntimeException();
            }
        }           
    }

    Reference< XNamedNodeMap > SAL_CALL CElement::getAttributes()throw (RuntimeException)
    {
        Reference< XNamedNodeMap > aMap;
        if (hasAttributes()) {
            aMap = Reference< XNamedNodeMap >(new CAttributesMap(this));
        }
        return aMap;
    }
    OUString SAL_CALL CElement::getNodeName()throw (RuntimeException)
    {
        OUString aName;
        if (m_aNodePtr != NULL)
        {
            const xmlChar* xName = m_aNodePtr->name;
            aName = OUString((const sal_Char*)xName, strlen((const char*)xName), RTL_TEXTENCODING_UTF8);
        }
        return aName;
    }
    OUString SAL_CALL CElement::getNodeValue() throw (RuntimeException)
    {
        return OUString();
    }
}
