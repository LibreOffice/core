/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: element.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:56:41 $
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

#include "node.hxx"
#include "element.hxx"
#include "attr.hxx"
#include "elementlist.hxx"
#include "attributesmap.hxx"
#include "../events/mutationevent.hxx"

#include <string.h>

namespace DOM
{

    CElement::CElement(const xmlNodePtr aNodePtr)
    {
        m_aNodeType = NodeType_ELEMENT_NODE;
        init_node(aNodePtr);
    }

    /**
        Retrieves an attribute value by name.
        return empty string if attribute is not set
    */
    OUString CElement::getAttribute(const OUString& name)
        throw (RuntimeException)
    {
        OUString aValue;
        // search properties
        if (m_aNodePtr != NULL)
        {
            OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
            xmlChar *xValue = xmlGetProp(m_aNodePtr, (xmlChar*)o1.getStr());
            if (xValue != NULL) {
                aValue = OUString((sal_Char*)xValue, strlen((char*)xValue), RTL_TEXTENCODING_UTF8);
            }
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
    return empty string if attribute is not set
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
            if (xValue != NULL) {
                aValue = OUString((sal_Char*)xValue, strlen((char*)xValue), RTL_TEXTENCODING_UTF8);
            }
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
        throw (RuntimeException, DOMException)
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
        throw (RuntimeException, DOMException)
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
        throw (RuntimeException, DOMException)
    {
        Reference< XAttr > aAttr;
        if(m_aNodePtr != NULL)
        {
        xmlAttrPtr pAttr = (xmlAttrPtr) CNode::getNodePtr(oldAttr.get());

            if (pAttr->parent != m_aNodePtr)
            {
                DOMException e;
                e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
                throw e;
            }
            if (pAttr->doc != m_aNodePtr->doc)
            {
                DOMException e;
                e.Code = DOMExceptionType_WRONG_DOCUMENT_ERR;
                throw e;
            }

            if (oldAttr->getNamespaceURI().getLength() > 0)
                aAttr = oldAttr->getOwnerDocument()->createAttributeNS(
                    oldAttr->getNamespaceURI(), oldAttr->getName());
            else
                aAttr = oldAttr->getOwnerDocument()->createAttribute(oldAttr->getName());
            aAttr->setValue(oldAttr->getValue());
            xmlRemoveProp(pAttr);

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
        xmlAttrPtr pAttr = (xmlAttrPtr) CNode::getNodePtr(newAttr.get());

            // check whether the attribute is not in use by another element
            xmlNsPtr pNs = NULL;
            if (pAttr->parent != NULL)
                if(strcmp((char*)pAttr->parent->name, "__private") == 0
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

        if (aAttr.is())
        {
            // attribute adition event
            // dispatch DOMAttrModified event
            Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
            Reference< XMutationEvent > event(docevent->createEvent(
                OUString::createFromAscii("DOMAttrModified")), UNO_QUERY);
            event->initMutationEvent(OUString::createFromAscii("DOMAttrModified"),
                sal_True, sal_False, Reference< XNode >(aAttr, UNO_QUERY),
                OUString(), aAttr->getValue(), aAttr->getName(), AttrChangeType_ADDITION);
            dispatchEvent(Reference< XEvent >(event, UNO_QUERY));
            dispatchSubtreeModified();
        }
        return aAttr;
    }

    Reference< XAttr > CElement::setAttributeNode(const Reference< XAttr >& newAttr)
        throw (RuntimeException, DOMException)
    {
        return _setAttributeNode(newAttr, sal_False);
    }

    /**
    Adds a new attribute.
    */
    Reference< XAttr > CElement::setAttributeNodeNS(const Reference< XAttr >& newAttr)
        throw (RuntimeException, DOMException)
    {
        return _setAttributeNode(newAttr, sal_True);
    }

    /**
    Adds a new attribute.
    */
    void CElement::setAttribute(const OUString& name, const OUString& value)
        throw (RuntimeException, DOMException)
    {
        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        OString o2 = OUStringToOString(value, RTL_TEXTENCODING_UTF8);
        xmlChar *xValue = (xmlChar*)o2.getStr();
        if (m_aNodePtr != NULL)
        {
            OUString oldValue;
            AttrChangeType aChangeType = AttrChangeType_MODIFICATION;
            xmlChar *xOld = xmlGetProp(m_aNodePtr, xName);
            if (xOld == NULL)
            {
                aChangeType = AttrChangeType_ADDITION;
                xmlNewProp(m_aNodePtr, xName, xValue);
            }
            else
            {
                oldValue = OUString((char*)xOld, strlen((char*)xOld), RTL_TEXTENCODING_UTF8);
                xmlSetProp(m_aNodePtr, xName, xValue);
            }

            // dispatch DOMAttrModified event

            Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
            Reference< XMutationEvent > event(docevent->createEvent(
                OUString::createFromAscii("DOMAttrModified")), UNO_QUERY);
            event->initMutationEvent(OUString::createFromAscii("DOMAttrModified"),
                sal_True, sal_False, Reference< XNode >(getAttributeNode(name), UNO_QUERY),
                oldValue, value, name, aChangeType);
            dispatchEvent(Reference< XEvent >(event, UNO_QUERY));
            dispatchSubtreeModified();
        }
    }

    /**
    Adds a new attribute.
    */
    void CElement::setAttributeNS(
            const OUString& namespaceURI, const OUString& qualifiedName, const OUString& value)
        throw (RuntimeException, DOMException)
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

                OUString oldValue;
                AttrChangeType aChangeType = AttrChangeType_MODIFICATION;
                xmlChar *xOld = xmlGetNsProp(m_aNodePtr, xLName, pNs->href);
                if (xOld == NULL)
                {
                    aChangeType = AttrChangeType_ADDITION;
                    xmlNewNsProp(m_aNodePtr, pNs, xLName, xValue);
                }
                else
                {
                    oldValue = OUString((char *)xOld, strlen((char *)xOld), RTL_TEXTENCODING_UTF8);
                    xmlSetNsProp(m_aNodePtr, pNs, xLName, xValue);
                }
                // dispatch DOMAttrModified event
                Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
                Reference< XMutationEvent > event(docevent->createEvent(
                    OUString::createFromAscii("DOMAttrModified")), UNO_QUERY);
                event->initMutationEvent(OUString::createFromAscii("DOMAttrModified"), sal_True, sal_False,
                    Reference< XNode >(getAttributeNodeNS(namespaceURI, OUString((char*)xLName, strlen((char*)xLName), RTL_TEXTENCODING_UTF8)), UNO_QUERY),
                    oldValue, value, qualifiedName, aChangeType);
                dispatchEvent(Reference< XEvent >(event, UNO_QUERY));
                dispatchSubtreeModified();

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

    void SAL_CALL CElement::setElementName(const OUString& aName) throw (RuntimeException, DOMException)
    {
        if (aName.getLength() > 0 && aName.indexOf(OUString::createFromAscii(":")) < 0)
        {
            OString oName = OUStringToOString(aName, RTL_TEXTENCODING_UTF8);
            xmlChar *xName = (xmlChar*)oName.getStr();
            // xmlFree((void*)m_aNodePtr->name);
            m_aNodePtr->name = xmlStrdup(xName);
        }
        else
        {
            DOMException e;
            e.Code = DOMExceptionType_INVALID_CHARACTER_ERR;
            throw e;
        }
    }

}
