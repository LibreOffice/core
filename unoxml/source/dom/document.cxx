/*************************************************************************
 *
 *  $RCSfile: document.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: lo $ $Date: 2004-02-26 14:43:14 $
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

#include "document.hxx"
#include "attr.hxx"
#include "element.hxx"
#include "cdatasection.hxx"
#include "documentfragment.hxx"
#include "text.hxx"
#include "cdatasection.hxx"
#include "comment.hxx"
#include "processinginstruction.hxx"
#include "entityreference.hxx"
#include "documenttype.hxx"
#include "elementlist.hxx"
#include "domimplementation.hxx"

#include "../events/mutationevent.hxx"

namespace DOM
{
    

    void CDocument::addnode(xmlNodePtr aNode)
    {
        if (aNode != (xmlNodePtr)m_aDocPtr)
        {
            Reference< XNode >* nref = new Reference< XNode >(CNode::get(aNode));
            m_aNodeRefList.push_back(nref);
        }
    }

    CDocument::~CDocument()
    {                
        Reference< XNode >* pRef;
        nodereflist_t::const_iterator r = m_aNodeRefList.begin();
        while (r!=m_aNodeRefList.end())
        {
            pRef = *r;
            delete pRef;
            r++;
        }

        // get rid of leftover instances, if anybody still holds a
        // reference to one of these, it will be invalid!
        /*
        CNode* aNode = 0;
        nodelist_t::const_iterator i = m_aNodeList.begin();
        while (i!=m_aNodeList.end())
        {
            aNode = CNode::get(*i, sal_False);
            if (aNode != 0)
            {
                // CNode::remove(*i);
                // delete will remove
                delete aNode;
            }
            i++;
        }
        */

        xmlFreeDoc(m_aDocPtr);

    }

    CDocument::CDocument(xmlDocPtr aDocPtr)
    {   
        m_aDocPtr = aDocPtr;
        m_aNodeType = NodeType_DOCUMENT_NODE;
        // init node base
        init_node((xmlNodePtr)m_aDocPtr);
    }

    // Creates an Attr of the given name.
    Reference< XAttr > SAL_CALL CDocument::createAttribute(const OUString& name)
        throw (DOMException)
    {
        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        return Reference< XAttr >(static_cast< CAttr* >(
                    CNode::get((xmlNodePtr)xmlNewDocProp(m_aDocPtr, xName, NULL))));
    };

    // Creates an attribute of the given qualified name and namespace URI.
    Reference< XAttr > SAL_CALL CDocument::createAttributeNS(
            const OUString& ns, const OUString& qname)
        throw (DOMException)
    {

        // libxml does not allow a NS definition to be attached to an
        // attribute node - which is a good thing, since namespaces are
        // only defined as parts of element nodes
        // thus, we create a temporary element node which carries the ns definition
        // and is removed/merged as soon as the attribute gets append to it's
        // actual parent
        sal_Int32 i = qname.indexOf(':');
        OString oPrefix, oName, oUri;
        xmlChar *xPrefix, *xName, *xUri;
        if (i != -1)
        {
            oPrefix = OUStringToOString(qname.copy(0, i), RTL_TEXTENCODING_UTF8);
            xPrefix = (xmlChar*)oPrefix.getStr();
            oName = OUStringToOString(qname.copy(i+1, qname.getLength()-i-1), RTL_TEXTENCODING_UTF8);
        }
        else
        {
            xPrefix = (xmlChar*)"";
            oName = OUStringToOString(qname, RTL_TEXTENCODING_UTF8);
        }
        xName = (xmlChar*)oName.getStr();
        oUri = OUStringToOString(ns, RTL_TEXTENCODING_UTF8);
        xUri = (xmlChar*)oUri.getStr();

        // create the carrier node
        xmlNodePtr pNode = xmlNewDocNode(m_aDocPtr, NULL, (xmlChar*)"__private", NULL);
        xmlNsPtr pNs = xmlNewNs(pNode, xUri, xPrefix);
        xmlAttrPtr pAttr = xmlNewNsProp(pNode, pNs, xName, NULL);        
        return Reference< XAttr >(static_cast< CAttr* >(CNode::get((xmlNodePtr)pAttr)));
    };

    // Creates a CDATASection node whose value is the specified string.
    Reference< XCDATASection > SAL_CALL CDocument::createCDATASection(const OUString& data)
        throw (RuntimeException)
    {
        xmlChar *xData = (xmlChar*)OUStringToOString(data, RTL_TEXTENCODING_UTF8).getStr();
        xmlNodePtr pText = xmlNewCDataBlock(m_aDocPtr, xData, strlen((char*)xData));
        return Reference< XCDATASection >(static_cast< CCDATASection* >(CNode::get(pText)));
    }

    // Creates a Comment node given the specified string.
    Reference< XComment > SAL_CALL CDocument::createComment(const OUString& data)
        throw (RuntimeException)
    {
        OString o1 = OUStringToOString(data, RTL_TEXTENCODING_UTF8);
        xmlChar *xData = (xmlChar*)o1.getStr();
        xmlNodePtr pComment = xmlNewDocComment(m_aDocPtr, xData);
        return Reference< XComment >(static_cast< CComment* >(CNode::get(pComment)));
    }

    //Creates an empty DocumentFragment object.
    Reference< XDocumentFragment > SAL_CALL CDocument::createDocumentFragment()
        throw (RuntimeException)
    {
        xmlNodePtr pFrag = xmlNewDocFragment(m_aDocPtr);
        return Reference< XDocumentFragment >(static_cast< CDocumentFragment* >(CNode::get(pFrag)));
    }

    // Creates an element of the type specified.
    Reference< XElement > SAL_CALL CDocument::createElement(const OUString& tagName)
        throw (DOMException)
    {
        OString o1 = OUStringToOString(tagName, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        xmlNodePtr aNodePtr = xmlNewDocNode(m_aDocPtr, NULL, xName, NULL);
        return Reference< XElement >(static_cast< CElement* >(CNode::get(aNodePtr)));
    }

    // Creates an element of the given qualified name and namespace URI.
    Reference< XElement > SAL_CALL CDocument::createElementNS(
            const OUString& ns, const OUString& qname)
        throw (DOMException)
    {        
        sal_Int32 i = qname.indexOf(':');
        if (ns.getLength() == 0) throw RuntimeException();
        xmlChar *xPrefix;
        xmlChar *xName;
        OString o1, o2, o3;
        if ( i != -1) {
            o1 = OUStringToOString(qname.copy(0, i), RTL_TEXTENCODING_UTF8);
            xPrefix = (xmlChar*)o1.getStr();
            o2 = OUStringToOString(qname.copy(i+1, qname.getLength()-i-1), RTL_TEXTENCODING_UTF8);
            xName = (xmlChar*)o2.getStr();
        } else {
            // default prefix
            xPrefix = (xmlChar*)"";
            o2 = OUStringToOString(qname, RTL_TEXTENCODING_UTF8);
            xName = (xmlChar*)o2.getStr();
        }
        o3 = OUStringToOString(ns, RTL_TEXTENCODING_UTF8);
        xmlChar *xUri = (xmlChar*)o3.getStr();

        // xmlNsPtr aNsPtr = xmlNewReconciledNs?
        // xmlNsPtr aNsPtr = xmlNewGlobalNs?
        xmlNodePtr aNodePtr = xmlNewDocNode(m_aDocPtr, NULL, xName, NULL);
        xmlNsPtr pNs = xmlNewNs(aNodePtr, xUri, xPrefix);
        xmlSetNs(aNodePtr, pNs);
        return Reference< XElement >(static_cast< CElement* >(CNode::get(aNodePtr)));
    }

    //Creates an EntityReference object.
    Reference< XEntityReference > SAL_CALL CDocument::createEntityReference(const OUString& name)
        throw (DOMException)
    {
        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        xmlNodePtr aNodePtr = xmlNewReference(m_aDocPtr, xName);
        return Reference< XEntityReference >(static_cast< CEntityReference* >(CNode::get(aNodePtr)));
    }

    // Creates a ProcessingInstruction node given the specified name and 
    // data strings.
    Reference< XProcessingInstruction > SAL_CALL CDocument::createProcessingInstruction(
            const OUString& target, const OUString& data)
        throw (DOMException)
    {
        OString o1 = OUStringToOString(target, RTL_TEXTENCODING_UTF8);
        xmlChar *xTarget = (xmlChar*)o1.getStr();
        OString o2 = OUStringToOString(data, RTL_TEXTENCODING_UTF8);
        xmlChar *xData = (xmlChar*)o2.getStr();
        xmlNodePtr aNodePtr = xmlNewPI(xTarget, xData);
        aNodePtr->doc = m_aDocPtr;
        return Reference< XProcessingInstruction >(static_cast< CProcessingInstruction* >(CNode::get(aNodePtr)));
    }

    // Creates a Text node given the specified string.
    Reference< XText > SAL_CALL CDocument::createTextNode(const OUString& data)
        throw (RuntimeException)
    {
        OString o1 = OUStringToOString(data, RTL_TEXTENCODING_UTF8);
        xmlChar *xData = (xmlChar*)o1.getStr();
        xmlNodePtr aNodePtr = xmlNewDocText(m_aDocPtr, xData);
        return Reference< XText >(static_cast< CText* >(CNode::get(aNodePtr)));
    }

    // The Document Type Declaration (see DocumentType) associated with this
    // document.
    Reference< XDocumentType > SAL_CALL CDocument::getDoctype()
        throw (RuntimeException)
    {
        // find the doc type
        xmlNodePtr cur = m_aDocPtr->children;
        while (cur != NULL)
        {
            if (cur->type == XML_DOCUMENT_TYPE_NODE || cur->type == XML_DTD_NODE)
                break;
        }
        return Reference< XDocumentType >(static_cast< CDocumentType* >(CNode::get(cur)));
    }

    // This is a convenience attribute that allows direct access to the child
    // node that is the root element of the document.
    Reference< XElement > SAL_CALL CDocument::getDocumentElement()
        throw (RuntimeException)
    {
        // find the document element
        xmlNodePtr cur = m_aDocPtr->children;
        while (cur != NULL)
        {
            if (cur->type == XML_ELEMENT_NODE)
                break;
            cur = cur->next;
        }
        return Reference< XElement >(static_cast< CElement* >(CNode::get(cur)));
    }

    static xmlNodePtr _search_element_by_id(const xmlNodePtr cur, const xmlChar* id)
    {
        
        if (cur == NULL)
            return NULL;
        // look in current node
        if (cur->type == XML_ELEMENT_NODE)
        {
            xmlAttrPtr a = cur->properties;
            while (a != NULL)
            {
                if (a->atype == XML_ATTRIBUTE_ID) {
                    if (strcmp((char*)a->children->content, (char*)id) == NULL)
                        return cur;
                }
                a = a->next;
            }
        }
        // look in children
        xmlNodePtr result = _search_element_by_id(cur->children, id);
        if (result != NULL)
            return result;
        result = _search_element_by_id(cur->next, id);
            return result;
    }

    // Returns the Element whose ID is given by elementId.
    Reference< XElement > SAL_CALL CDocument::getElementById(const OUString& elementId)
        throw (RuntimeException)
    {
        // search the tree for an element with the given ID
        OString o1 = OUStringToOString(elementId, RTL_TEXTENCODING_UTF8);
        xmlChar *xId = (xmlChar*)o1.getStr();
        Reference< XUnoTunnel > tunnel(getDocumentElement(), UNO_QUERY);
        xmlNodePtr pStart = (xmlNodePtr)tunnel->getSomething(Sequence< sal_Int8>());
        xmlNodePtr aNodePtr = _search_element_by_id(pStart, xId);
        return Reference< XElement >(static_cast< CElement* >(CNode::get(aNodePtr)));
    }


    Reference< XNodeList > SAL_CALL CDocument::getElementsByTagName(const OUString& tagname)
            throw (RuntimeException)
    {
        // build a list
        return Reference< XNodeList >(
            new CElementList(static_cast< CElement* >(
            this->getDocumentElement().get()), tagname));
    }

    Reference< XNodeList > SAL_CALL CDocument::getElementsByTagNameNS(
            const OUString& namespaceURI, const OUString& localName)
        throw (RuntimeException)
    {        
        return Reference< XNodeList >(
            new CElementList(static_cast< CElement* >(
            this->getDocumentElement().get()), namespaceURI, localName));
    }

    Reference< XDOMImplementation > SAL_CALL CDocument::getImplementation()
        throw (RuntimeException)
    {
        // XXX
        return Reference< XDOMImplementation >(CDOMImplementation::get());
    }

    // helper function to recall import for siblings
    static Reference< XNode > _import_siblings (
        const Reference< XNode > aNode, const Reference< XNode> parent, CDocument* pTarget)
    {
        Reference< XNode > sibling = aNode;
        Reference< XNode > tmp;
        Reference< XNode > firstImported;
        while (sibling.is())
        {
            tmp = pTarget->importNode(sibling, sal_True);
            parent->appendChild(tmp);
            if (!firstImported.is())
                firstImported = tmp;
            sibling = sibling->getNextSibling();
        }
        return firstImported;
    }

    Reference< XNode > SAL_CALL CDocument::importNode(
            const Reference< XNode >& importedNode, sal_Bool deep)
        throw (DOMException)
    {
        // this node could be from another memory model
        // only use uno interfaces to access is!!!

        // allready in doc?
        if ( importedNode->getOwnerDocument() == 
            Reference< XDocument>(static_cast< CDocument* >(CNode::get((xmlNodePtr)m_aDocPtr))))
            return importedNode;

        Reference< XNode > aNode;
        switch (importedNode->getNodeType())
        {
        case NodeType_ATTRIBUTE_NODE:
        {
            Reference< XAttr > attr(importedNode, UNO_QUERY);
            Reference< XAttr > newAttr = createAttribute(attr->getName());
            newAttr->setValue(attr->getValue());
            aNode.set(newAttr, UNO_QUERY);
            break;
        }
        case NodeType_CDATA_SECTION_NODE:
        {
            Reference< XCDATASection > cdata(importedNode, UNO_QUERY);
            Reference< XCDATASection > newCdata = createCDATASection(cdata->getData());
            aNode.set(newCdata, UNO_QUERY);
            break;
        }
        case NodeType_COMMENT_NODE:
        {
            Reference< XComment > comment(importedNode, UNO_QUERY);
            Reference< XComment > newComment = createComment(comment->getData());
            aNode.set(newComment, UNO_QUERY);
            break;
        }
        case NodeType_DOCUMENT_FRAGMENT_NODE:
        {
            Reference< XDocumentFragment > frag(importedNode, UNO_QUERY);
            Reference< XDocumentFragment > newFrag = createDocumentFragment();
            aNode.set(newFrag, UNO_QUERY);
            break;
        }
        case NodeType_ELEMENT_NODE:
        {
            Reference< XElement > element(importedNode, UNO_QUERY);
            Reference< XElement > newElement(createElement(element->getTagName()));
            // get attributes
            if (element->hasAttributes())
            {
                Reference< XNamedNodeMap > attribs = element->getAttributes();
                Reference< XAttr > curAttr;
                for (sal_Int32 i = 0; i < attribs->getLength(); i++)
                {
                    curAttr = Reference< XAttr >(attribs->item(i), UNO_QUERY);
                    newElement->setAttributeNS(curAttr->getNamespaceURI(), curAttr->getNodeName(), curAttr->getValue());
                }
            }
            aNode.set(newElement, UNO_QUERY);
            break;
        }
        case NodeType_ENTITY_REFERENCE_NODE:
        {
            Reference< XEntityReference > ref(importedNode, UNO_QUERY);
            Reference< XEntityReference > newRef(createEntityReference(ref->getNodeName()));
            aNode.set(newRef, UNO_QUERY);
            break;
        }
        case NodeType_PROCESSING_INSTRUCTION_NODE:
        {
            Reference< XProcessingInstruction > pi(importedNode, UNO_QUERY);
            Reference< XProcessingInstruction > newPi(
                createProcessingInstruction(pi->getTarget(), pi->getData()));
            aNode.set(newPi, UNO_QUERY);
            break;
        }
        case NodeType_TEXT_NODE:
        {
            Reference< XText > text(importedNode, UNO_QUERY);
            Reference< XText > newText(createTextNode(text->getData()));
            aNode.set(newText, UNO_QUERY);
            break;
        }
        case NodeType_ENTITY_NODE:
        case NodeType_DOCUMENT_NODE:
        case NodeType_DOCUMENT_TYPE_NODE:
        case NodeType_NOTATION_NODE:
        default:
            // can't be imported
            throw RuntimeException();
            break;

        }
        if (deep)
        {
            // get children and import them
            Reference< XNode > child = importedNode->getFirstChild();
            if (child.is()) 
            {
                Reference< XNode > ic = _import_siblings(child, aNode, this);                
                aNode->appendChild(ic);
            }
        }        

        /* DOMNodeInsertedIntoDocument
         * Fired when a node is being inserted into a document, 
         * either through direct insertion of the Node or insertion of a 
         * subtree in which it is contained. This event is dispatched after 
         * the insertion has taken place. The target of this event is the node 
         * being inserted. If the Node is being directly inserted the DOMNodeInserted 
         * event will fire before the DOMNodeInsertedIntoDocument event.
         *   Bubbles: No
         *   Cancelable: No
         *   Context Info: None
         */
        if (aNode.is())
        {
            events::CMutationEvent *pEvent = new events::CMutationEvent;
            pEvent->initMutationEvent(EventType_DOMNodeInsertedIntoDocument, sal_True, 
                sal_False, Reference< XNode >(),
                OUString(), OUString(), OUString(), (AttrChangeType)0 );
            pEvent->m_target = Reference< XEventTarget >(aNode, UNO_QUERY);
            dispatchEvent(Reference< XEvent >(static_cast< events::CEvent* >(pEvent)));
        }

        return aNode;
    }
    OUString SAL_CALL CDocument::getNodeName()throw (RuntimeException)
    {
        return OUString::createFromAscii("#document");
    }
    OUString SAL_CALL CDocument::getNodeValue() throw (RuntimeException)
    {
        return OUString();
    }
}
