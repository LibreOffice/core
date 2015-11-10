/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <osl/diagnose.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include "xmldocumentwrapper_xmlsecimpl.hxx"

#include <xmloff/attrlist.hxx>
#include "xmlelementwrapper_xmlsecimpl.hxx"

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <vector>

#ifdef UNX
#define stricmp strcasecmp
#endif

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxcsax = com::sun::star::xml::csax;
namespace cssxs = com::sun::star::xml::sax;
namespace cssxw = com::sun::star::xml::wrapper;

#define IMPLEMENTATION_NAME "com.sun.star.xml.security.bridge.xmlsec.XMLDocumentWrapper_XmlSecImpl"

#define STRXMLNS "xmlns"

#define C2U( asciiStr ) asciiStr, strlen( asciiStr ), RTL_TEXTENCODING_UTF8

/* used by the recursiveDelete method */
#define NODE_REMOVED    0
#define NODE_NOTREMOVED 1
#define NODE_STOPED     2

XMLDocumentWrapper_XmlSecImpl::XMLDocumentWrapper_XmlSecImpl()
    : m_nCurrentPosition(0)
    , m_pStopAtNode(nullptr)
    , m_pCurrentReservedNode(nullptr)
    , m_nReservedNodeIndex(0)
{
    saxHelper.startDocument();
    m_pDocument = saxHelper.getDocument();

    /*
     * creates the virtual root element
     */
    saxHelper.startElement(OUString(C2U( "root" )), cssu::Sequence<cssxcsax::XMLAttribute>());

    m_pRootElement = saxHelper.getCurrentNode();
    m_pCurrentElement = m_pRootElement;
}

XMLDocumentWrapper_XmlSecImpl::~XMLDocumentWrapper_XmlSecImpl()
{
    saxHelper.endDocument();
    xmlFreeDoc(m_pDocument);
}

void XMLDocumentWrapper_XmlSecImpl::getNextSAXEvent()
/****** XMLDocumentWrapper_XmlSecImpl/getNextSAXEvent *************************
 *
 *   NAME
 *  getNextSAXEvent -- Prepares the next SAX event to be manipulate
 *
 *   SYNOPSIS
 *  getNextSAXEvent();
 *
 *   FUNCTION
 *  When converting the document into SAX events, this method is used to
 *  decide the next SAX event to be generated.
 *  Two member variables are checked to make the decision, the
 *  m_pCurrentElement and the m_nCurrentPosition.
 *  The m_pCurrentElement represents the node which have been covered, and
 *  the m_nCurrentPosition represents the event which have been sent.
 *  For example, suppose that the m_pCurrentElement
 *  points to element A, and the m_nCurrentPosition equals to
 *  NODEPOSITION_STARTELEMENT, then the next SAX event should be the
 *  endElement for element A if A has no child, or startElement for the
 *  first child element of element A otherwise.
 *  The m_nCurrentPosition can be one of following values:
 *  NODEPOSITION_STARTELEMENT for startElement;
 *  NODEPOSITION_ENDELEMENT for endElement;
 *  NODEPOSITION_NORMAL for other SAX events;
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    OSL_ASSERT( m_pCurrentElement != nullptr );

        /*
         * Get the next event through tree order.
         *
         * if the current event is a startElement, then the next
         * event depends on whether or not the current node has
         * children.
         */
    if (m_nCurrentPosition == NODEPOSITION_STARTELEMENT)
    {
            /*
             * If the current node has children, then its first child
             * should be next current node, and the next event will be
             * startElement or charaters(PI) based on that child's node
             * type. Otherwise, the endElement of current node is the
             * next event.
             */
        if (m_pCurrentElement->children != nullptr)
        {
            m_pCurrentElement = m_pCurrentElement->children;
            m_nCurrentPosition
                = (m_pCurrentElement->type == XML_ELEMENT_NODE)?
                    NODEPOSITION_STARTELEMENT:NODEPOSITION_NORMAL;
        }
        else
        {
            m_nCurrentPosition = NODEPOSITION_ENDELEMENT;
        }
    }
        /*
         * if the current event is a not startElement, then the next
         * event depends on whether or not the current node has
         * following sibling.
         */
    else if (m_nCurrentPosition == NODEPOSITION_ENDELEMENT || m_nCurrentPosition == NODEPOSITION_NORMAL)
    {
        xmlNodePtr pNextSibling = m_pCurrentElement->next;

            /*
             * If the current node has following sibling, that sibling
             * should be next current node, and the next event will be
             * startElement or charaters(PI) based on that sibling's node
             * type. Otherwise, the endElement of current node's parent
             * becomes the next event.
             */
        if (pNextSibling != nullptr)
        {
            m_pCurrentElement = pNextSibling;
            m_nCurrentPosition
                = (m_pCurrentElement->type == XML_ELEMENT_NODE)?
                    NODEPOSITION_STARTELEMENT:NODEPOSITION_NORMAL;
        }
        else
        {
            m_pCurrentElement = m_pCurrentElement->parent;
            m_nCurrentPosition = NODEPOSITION_ENDELEMENT;
        }
    }
}

void XMLDocumentWrapper_XmlSecImpl::sendStartElement(
    const cssu::Reference< cssxs::XDocumentHandler >& xHandler,
    const cssu::Reference< cssxs::XDocumentHandler >& xHandler2,
    const xmlNodePtr pNode)
    throw (cssxs::SAXException)
/****** XMLDocumentWrapper_XmlSecImpl/sendStartElement ************************
 *
 *   NAME
 *  sendStartElement -- Constructs a startElement SAX event
 *
 *   SYNOPSIS
 *  sendStartElement(xHandler, xHandler2, pNode);
 *
 *   FUNCTION
 *  Used when converting the document into SAX event stream.
 *  This method constructs a startElement SAX event for a particular
 *  element, then calls the startElement methods of the XDocumentHandlers.
 *
 *   INPUTS
 *  xHandler -  the first XDocumentHandler interface to receive the
 *          startElement SAX event. It can be NULL.
 *  xHandler2 - the second XDocumentHandler interface to receive the
 *          startElement SAX event. It can't be NULL.
 *  pNode -     the node on which the startElement should be generated.
 *          This node must be a element type.
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    SvXMLAttributeList* pAttributeList = new SvXMLAttributeList();
    cssu::Reference < cssxs::XAttributeList > xAttrList(pAttributeList);

    xmlNsPtr pNsDef = pNode->nsDef;

    while (pNsDef != nullptr)
    {
        const xmlChar* pNsPrefix = pNsDef->prefix;
        const xmlChar* pNsHref = pNsDef->href;

        if (pNsDef->prefix == nullptr)
        {
            pAttributeList->AddAttribute(
                OUString(C2U( STRXMLNS )),
                OUString(C2U( reinterpret_cast<char const *>(pNsHref) )));
        }
        else
        {
            pAttributeList->AddAttribute(
                OUString(C2U( STRXMLNS ))
                +OUString(C2U( ":" ))
                +OUString(C2U( reinterpret_cast<char const *>(pNsPrefix) )),
                OUString(C2U( reinterpret_cast<char const *>(pNsHref) )));
        }

        pNsDef = pNsDef->next;
    }

    xmlAttrPtr pAttr = pNode->properties;

    while (pAttr != nullptr)
    {
        const xmlChar* pAttrName = pAttr->name;
        xmlNsPtr pAttrNs = pAttr->ns;

        OUString ouAttrName;
        if (pAttrNs == nullptr)
        {
            ouAttrName = OUString(C2U( reinterpret_cast<char const *>(pAttrName) ));
        }
        else
        {
            ouAttrName = OUString(C2U( reinterpret_cast<char const *>(pAttrNs->prefix)))
                + ":" + OUString(C2U( reinterpret_cast<char const *>(pAttrName) ));
        }

        pAttributeList->AddAttribute(
            ouAttrName,
            OUString(C2U( reinterpret_cast<char*>(pAttr->children->content))));
        pAttr = pAttr->next;
    }

    OString sNodeName = getNodeQName(pNode);

    if (xHandler.is())
    {
        xHandler->startElement(
            OUString(C2U ( sNodeName.getStr() )),
            xAttrList);
    }

    xHandler2->startElement(
        OUString(C2U ( sNodeName.getStr() )),
        xAttrList);
}

void XMLDocumentWrapper_XmlSecImpl::sendEndElement(
    const cssu::Reference< cssxs::XDocumentHandler >& xHandler,
    const cssu::Reference< cssxs::XDocumentHandler >& xHandler2,
    const xmlNodePtr pNode)
    throw (cssxs::SAXException)
/****** XMLDocumentWrapper_XmlSecImpl/sendEndElement **************************
 *
 *   NAME
 *  sendEndElement -- Constructs a endElement SAX event
 *
 *   SYNOPSIS
 *  sendEndElement(xHandler, xHandler2, pNode);
 *
 *   FUNCTION
 *  Used when converting the document into SAX event stream.
 *  This method constructs a endElement SAX event for a particular
 *  element, then calls the endElement methods of the XDocumentHandlers.
 *
 *   INPUTS
 *  xHandler -  the first XDocumentHandler interface to receive the
 *          endElement SAX event. It can be NULL.
 *  xHandler2 - the second XDocumentHandler interface to receive the
 *          endElement SAX event. It can't be NULL.
 *  pNode -     the node on which the endElement should be generated.
 *          This node must be a element type.
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    OString sNodeName = getNodeQName(pNode);

    if (xHandler.is())
    {
        xHandler->endElement(OUString(C2U ( sNodeName.getStr() )));
    }

    xHandler2->endElement(OUString(C2U ( sNodeName.getStr() )));
}

void XMLDocumentWrapper_XmlSecImpl::sendNode(
    const cssu::Reference< cssxs::XDocumentHandler >& xHandler,
    const cssu::Reference< cssxs::XDocumentHandler >& xHandler2,
    const xmlNodePtr pNode)
    throw (cssxs::SAXException)
/****** XMLDocumentWrapper_XmlSecImpl/sendNode ********************************
 *
 *   NAME
 *  sendNode -- Constructs a characters SAX event or a
 *  processingInstruction SAX event
 *
 *   SYNOPSIS
 *  sendNode(xHandler, xHandler2, pNode);
 *
 *   FUNCTION
 *  Used when converting the document into SAX event stream.
 *  This method constructs a characters SAX event or a
 *  processingInstructionfor SAX event based on the type of a particular
 *  element, then calls the corresponding methods of the XDocumentHandlers.
 *
 *   INPUTS
 *  xHandler -  the first XDocumentHandler interface to receive the
 *          SAX event. It can be NULL.
 *  xHandler2 - the second XDocumentHandler interface to receive the
 *          SAX event. It can't be NULL.
 *  pNode -     the node on which the endElement should be generated.
 *          If it is a text node, then a characters SAX event is
 *          generated; if it is a PI node, then a
 *          processingInstructionfor SAX event is generated.
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    xmlElementType type = pNode->type;

    if (type == XML_TEXT_NODE)
    {
        if (xHandler.is())
        {
            xHandler->characters(OUString(C2U ( reinterpret_cast<char*>(pNode->content) )));
        }

        xHandler2->characters(OUString(C2U ( reinterpret_cast<char*>(pNode->content) )));
    }
    else if (type == XML_PI_NODE)
    {
        if (xHandler.is())
        {
            xHandler->processingInstruction(
                OUString(C2U ( reinterpret_cast<char const *>(pNode->name) )),
                OUString(C2U ( reinterpret_cast<char const *>(pNode->content) )));
        }

        xHandler2->processingInstruction(
            OUString(C2U ( reinterpret_cast<char const *>(pNode->name) )),
            OUString(C2U ( reinterpret_cast<char*>(pNode->content) )));
    }
}

OString XMLDocumentWrapper_XmlSecImpl::getNodeQName(const xmlNodePtr pNode)
/****** XMLDocumentWrapper_XmlSecImpl/getNodeQName ****************************
 *
 *   NAME
 *  getNodeQName -- Retrieves the qualified name of a node
 *
 *   SYNOPSIS
 *  name = getNodeQName(pNode);
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pNode - the node whose name will be retrieved
 *
 *   RESULT
 *  name - the node's qualified name
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    OString sNodeName(reinterpret_cast<const char*>(pNode->name));
    if (pNode->ns != nullptr)
    {
        xmlNsPtr pNs = pNode->ns;

        if (pNs->prefix != nullptr)
        {
            OString sPrefix(reinterpret_cast<const char*>(pNs->prefix));
            sNodeName = sPrefix+OString(":")+sNodeName;
        }
    }

    return sNodeName;
}

xmlNodePtr XMLDocumentWrapper_XmlSecImpl::checkElement( const cssu::Reference< cssxw::XXMLElementWrapper >& xXMLElement)
/****** XMLDocumentWrapper_XmlSecImpl/checkElement ****************************
 *
 *   NAME
 *  checkElement -- Retrieves the node wrapped by an XXMLElementWrapper
 *  interface
 *
 *   SYNOPSIS
 *  node = checkElement(xXMLElement);
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  xXMLElement -   the XXMLElementWrapper interface wraping a node
 *
 *   RESULT
 *  node - the node wrapped in the XXMLElementWrapper interface
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    xmlNodePtr rc = nullptr;

    if (xXMLElement.is())
    {
        cssu::Reference< cssl::XUnoTunnel > xNodTunnel( xXMLElement, cssu::UNO_QUERY ) ;
        if( !xNodTunnel.is() )
        {
            throw cssu::RuntimeException() ;
        }

        XMLElementWrapper_XmlSecImpl* pElement
            = reinterpret_cast<XMLElementWrapper_XmlSecImpl*>(
                sal::static_int_cast<sal_uIntPtr>(
                    xNodTunnel->getSomething(
                        XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() ))) ;

        if( pElement == nullptr ) {
            throw cssu::RuntimeException() ;
        }

        rc = pElement->getNativeElement();
    }

    return rc;
}

sal_Int32 XMLDocumentWrapper_XmlSecImpl::recursiveDelete(
    const xmlNodePtr pNode)
/****** XMLDocumentWrapper_XmlSecImpl/recursiveDelete *************************
 *
 *   NAME
 *  recursiveDelete -- Deletes a particular node with its branch.
 *
 *   SYNOPSIS
 *  result = recursiveDelete(pNode);
 *
 *   FUNCTION
 *  Deletes a particular node with its branch, while reserving the nodes
 *  (and their branches) listed in the m_aReservedNodes.
 *  The deletion process is preformed in the tree order, that is, a node
 *  is deleted after its previous sibling node is deleted, a parent node
 *  is deleted after its branch is deleted.
 *  During the deletion process when the m_pStopAtNode is reached, the
 *  progress is interrupted at once.
 *
 *   INPUTS
 *  pNode - the node to be deleted
 *
 *   RESULT
 *  result -    the result of the deletion process, can be one of following
 *          values:
 *          NODE_STOPED - the process is interrupted by meeting the
 *              m_pStopAtNode
 *          NODE_NOTREMOVED - the pNode is not completely removed
 *              because there is its descendant in the
 *              m_aReservedNodes list
 *          NODE_REMOVED - the pNode and its branch are completely
 *              removed
 *
 *   NOTES
 *  The node in the m_aReservedNodes list must be in the tree order, otherwise
 *  the result is unpredictable.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    if (pNode == m_pStopAtNode)
    {
        return NODE_STOPED;
    }

    if (pNode != m_pCurrentReservedNode)
    {
        xmlNodePtr pChild = pNode->children;

        xmlNodePtr pNextSibling;
        bool bIsRemoved = true;
        sal_Int32 nResult;

        while( pChild != nullptr )
        {
            pNextSibling = pChild->next;
            nResult = recursiveDelete(pChild);

            switch (nResult)
            {
            case NODE_STOPED:
                return NODE_STOPED;
            case NODE_NOTREMOVED:
                bIsRemoved = false;
                break;
            case NODE_REMOVED:
                removeNode(pChild);
                break;
            default:
                throw cssu::RuntimeException();
            }

            pChild = pNextSibling;
        }

        if (pNode == m_pCurrentElement)
        {
            bIsRemoved = false;
        }

        return bIsRemoved?NODE_REMOVED:NODE_NOTREMOVED;
    }
    else
    {
        getNextReservedNode();
        return NODE_NOTREMOVED;
    }
}

void XMLDocumentWrapper_XmlSecImpl::getNextReservedNode()
/****** XMLDocumentWrapper_XmlSecImpl/getNextReservedNode *********************
 *
 *   NAME
 *  getNextReservedNode -- Highlights the next reserved node in the
 *  reserved node list
 *
 *   SYNOPSIS
 *  getNextReservedNode();
 *
 *   FUNCTION
 *  The m_aReservedNodes array holds a node list, while the
 *  m_pCurrentReservedNode points to the one currently highlighted.
 *  This method is used to highlight the next node in the node list.
 *  This method is called at the time when the current highlighted node
 *  has been already processed, and the next node should be ready.
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    if (m_nReservedNodeIndex < m_aReservedNodes.getLength())
    {
        m_pCurrentReservedNode = checkElement( m_aReservedNodes[m_nReservedNodeIndex] );
        m_nReservedNodeIndex ++;
    }
    else
    {
        m_pCurrentReservedNode = nullptr;
    }
}

void XMLDocumentWrapper_XmlSecImpl::removeNode(const xmlNodePtr pNode) const
/****** XMLDocumentWrapper_XmlSecImpl/removeNode ******************************
 *
 *   NAME
 *  removeNode -- Deletes a node with its branch unconditionaly
 *
 *   SYNOPSIS
 *  removeNode( pNode );
 *
 *   FUNCTION
 *  Delete the node along with its branch from the document.
 *
 *   INPUTS
 *  pNode - the node to be deleted
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    /* you can't remove the current node */
    OSL_ASSERT( m_pCurrentElement != pNode );

    xmlAttrPtr pAttr = pNode->properties;

    while (pAttr != nullptr)
    {
        if (!stricmp(reinterpret_cast<char const *>(pAttr->name), "id"))
        {
            xmlRemoveID(m_pDocument, pAttr);
        }

        pAttr = pAttr->next;
    }

    xmlUnlinkNode(pNode);
    xmlFreeNode(pNode);
}

void XMLDocumentWrapper_XmlSecImpl::buildIDAttr(xmlNodePtr pNode) const
/****** XMLDocumentWrapper_XmlSecImpl/buildIDAttr *****************************
 *
 *   NAME
 *  buildIDAttr -- build the ID attribute of a node
 *
 *   SYNOPSIS
 *  buildIDAttr( pNode );
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pNode - the node whose id attribute will be built
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    xmlAttrPtr idAttr = xmlHasProp( pNode, reinterpret_cast<const unsigned char *>("id") );
    if (idAttr == nullptr)
    {
        idAttr = xmlHasProp( pNode, reinterpret_cast<const unsigned char *>("Id") );
    }

    if (idAttr != nullptr)
    {
        xmlChar* idValue = xmlNodeListGetString( m_pDocument, idAttr->children, 1 ) ;
        xmlAddID( nullptr, m_pDocument, idValue, idAttr );
    }
}

void XMLDocumentWrapper_XmlSecImpl::rebuildIDLink(xmlNodePtr pNode) const
/****** XMLDocumentWrapper_XmlSecImpl/rebuildIDLink ***************************
 *
 *   NAME
 *  rebuildIDLink -- rebuild the ID link for the branch
 *
 *   SYNOPSIS
 *  rebuildIDLink( pNode );
 *
 *   FUNCTION
 *  see NAME
 *
 *   INPUTS
 *  pNode - the node, from which the branch will be rebuilt
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    if (pNode != nullptr && pNode->type == XML_ELEMENT_NODE)
    {
        buildIDAttr( pNode );

        xmlNodePtr child = pNode->children;
        while (child != nullptr)
        {
            rebuildIDLink(child);
            child = child->next;
        }
    }
}

/* XXMLDocumentWrapper */
cssu::Reference< cssxw::XXMLElementWrapper > SAL_CALL XMLDocumentWrapper_XmlSecImpl::getCurrentElement(  )
    throw (cssu::RuntimeException, std::exception)
{
    XMLElementWrapper_XmlSecImpl* pElement = new XMLElementWrapper_XmlSecImpl(m_pCurrentElement);
    return cssu::Reference< cssxw::XXMLElementWrapper >(pElement);
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::setCurrentElement( const cssu::Reference< cssxw::XXMLElementWrapper >& element )
    throw (cssu::RuntimeException, std::exception)
{
    m_pCurrentElement = checkElement( element );
    saxHelper.setCurrentNode( m_pCurrentElement );
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::removeCurrentElement(  )
    throw (cssu::RuntimeException, std::exception)
{
    OSL_ASSERT( m_pCurrentElement != nullptr );

    xmlNodePtr pOldCurrentElement = m_pCurrentElement;

    /*
     * pop the top node in the parser context's
     * nodeTab stack, then the parent of that node will
     * automatically become the new stack top, and
     * the current node as well.
     */
    saxHelper.endElement(
        OUString(
            C2U (
                reinterpret_cast<char const *>(pOldCurrentElement->name)
            )));
    m_pCurrentElement = saxHelper.getCurrentNode();

    /*
     * remove the node
     */
    removeNode(pOldCurrentElement);
}

sal_Bool SAL_CALL XMLDocumentWrapper_XmlSecImpl::isCurrent( const cssu::Reference< cssxw::XXMLElementWrapper >& node )
    throw (cssu::RuntimeException, std::exception)
{
    xmlNodePtr pNode = checkElement(node);
    return (pNode == m_pCurrentElement);
}

sal_Bool SAL_CALL XMLDocumentWrapper_XmlSecImpl::isCurrentElementEmpty(  )
    throw (cssu::RuntimeException, std::exception)
{
    bool rc = false;

    if (m_pCurrentElement->children == nullptr)
    {
        rc = true;
    }

    return rc;
}

OUString SAL_CALL XMLDocumentWrapper_XmlSecImpl::getNodeName( const cssu::Reference< cssxw::XXMLElementWrapper >& node )
    throw (cssu::RuntimeException, std::exception)
{
    xmlNodePtr pNode = checkElement(node);
    return OUString(C2U ( reinterpret_cast<char const *>(pNode->name) ));
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::clearUselessData(
    const cssu::Reference< cssxw::XXMLElementWrapper >& node,
    const cssu::Sequence< cssu::Reference< cssxw::XXMLElementWrapper > >& reservedDescendants,
    const cssu::Reference< cssxw::XXMLElementWrapper >& stopAtNode )
    throw (cssu::RuntimeException, std::exception)
{
    xmlNodePtr pTargetNode = checkElement(node);

    m_pStopAtNode = checkElement(stopAtNode);
    m_aReservedNodes = reservedDescendants;
    m_nReservedNodeIndex = 0;

    getNextReservedNode();

    recursiveDelete(pTargetNode);
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::collapse( const cssu::Reference< cssxw::XXMLElementWrapper >& node )
    throw (cssu::RuntimeException, std::exception)
{
    xmlNodePtr pTargetNode = checkElement(node);
    xmlNodePtr pParent;

    while (pTargetNode != nullptr)
    {
        if (pTargetNode->children != nullptr || pTargetNode == m_pCurrentElement)
        {
            break;
        }

        pParent = pTargetNode->parent;
        removeNode(pTargetNode);
        pTargetNode = pParent;
    }
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::getTree( const cssu::Reference< cssxs::XDocumentHandler >& handler )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    if (m_pRootElement != nullptr)
    {
        xmlNodePtr pTempCurrentElement = m_pCurrentElement;
        sal_Int32 nTempCurrentPosition = m_nCurrentPosition;

        m_pCurrentElement = m_pRootElement;

        m_nCurrentPosition = NODEPOSITION_STARTELEMENT;
        cssu::Reference< cssxs::XDocumentHandler > xHandler = handler;

        while(true)
        {
            switch (m_nCurrentPosition)
            {
            case NODEPOSITION_STARTELEMENT:
                sendStartElement(nullptr, xHandler, m_pCurrentElement);
                break;
            case NODEPOSITION_ENDELEMENT:
                sendEndElement(nullptr, xHandler, m_pCurrentElement);
                break;
            case NODEPOSITION_NORMAL:
                sendNode(nullptr, xHandler, m_pCurrentElement);
                break;
            }

            if ( (m_pCurrentElement == m_pRootElement) && (m_nCurrentPosition == NODEPOSITION_ENDELEMENT ))
            {
                break;
            }

            getNextSAXEvent();
        }

        m_pCurrentElement = pTempCurrentElement;
        m_nCurrentPosition = nTempCurrentPosition;
    }
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::generateSAXEvents(
    const cssu::Reference< cssxs::XDocumentHandler >& handler,
    const cssu::Reference< cssxs::XDocumentHandler >& xEventKeeperHandler,
    const cssu::Reference< cssxw::XXMLElementWrapper >& startNode,
    const cssu::Reference< cssxw::XXMLElementWrapper >& endNode )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
        /*
         * The first SAX event is the startElement of the startNode
         * element.
         */
    bool bHasCurrentElementChild = (m_pCurrentElement->children != nullptr);

    xmlNodePtr pTempCurrentElement = m_pCurrentElement;

    m_pCurrentElement = checkElement(startNode);

    if (m_pCurrentElement->type == XML_ELEMENT_NODE)
    {
        m_nCurrentPosition = NODEPOSITION_STARTELEMENT;
    }
    else
    {
        m_nCurrentPosition = NODEPOSITION_NORMAL;
    }

    xmlNodePtr pEndNode = checkElement(endNode);

    cssu::Reference < cssxc::sax::XSAXEventKeeper > xSAXEventKeeper( xEventKeeperHandler, cssu::UNO_QUERY );

    cssu::Reference< cssxs::XDocumentHandler > xHandler = handler;

    while(true)
    {
        switch (m_nCurrentPosition)
        {
        case NODEPOSITION_STARTELEMENT:
            sendStartElement(xHandler, xEventKeeperHandler, m_pCurrentElement);
            break;
        case NODEPOSITION_ENDELEMENT:
            sendEndElement(xHandler, xEventKeeperHandler, m_pCurrentElement);
            break;
        case NODEPOSITION_NORMAL:
            sendNode(xHandler, xEventKeeperHandler, m_pCurrentElement);
            break;
        default:
            throw cssu::RuntimeException();
        }

        if (xSAXEventKeeper->isBlocking())
        {
            xHandler = nullptr;
        }

        if (pEndNode == nullptr &&
            ((bHasCurrentElementChild && m_pCurrentElement == xmlGetLastChild(pTempCurrentElement) && m_nCurrentPosition != NODEPOSITION_STARTELEMENT) ||
             (!bHasCurrentElementChild && m_pCurrentElement == pTempCurrentElement && m_nCurrentPosition == NODEPOSITION_STARTELEMENT)))
        {
            break;
        }

        getNextSAXEvent();

            /*
             * If there is an end point specified, then check whether
             * the current node equals to the end point. If so, stop
             * generating.
             */
        if (pEndNode != nullptr && m_pCurrentElement == pEndNode)
        {
            break;
        }
    }

    m_pCurrentElement = pTempCurrentElement;
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::rebuildIDLink(
    const com::sun::star::uno::Reference< com::sun::star::xml::wrapper::XXMLElementWrapper >& node )
    throw (com::sun::star::uno::RuntimeException, std::exception)
{
    xmlNodePtr pNode = checkElement( node );
    rebuildIDLink(pNode);
}


/* cssxs::XDocumentHandler */
void SAL_CALL XMLDocumentWrapper_XmlSecImpl::startDocument(  )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::endDocument(  )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::startElement( const OUString& aName, const cssu::Reference< cssxs::XAttributeList >& xAttribs )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    sal_Int32 nLength = xAttribs->getLength();
    cssu::Sequence< cssxcsax::XMLAttribute > aAttributes (nLength);

    for (int i = 0; i < nLength; ++i)
    {
        aAttributes[i].sName = xAttribs->getNameByIndex((short)i);
        aAttributes[i].sValue =xAttribs->getValueByIndex((short)i);
    }

    compressedStartElement(aName, aAttributes);
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::endElement( const OUString& aName )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    saxHelper.endElement(aName);
    m_pCurrentElement = saxHelper.getCurrentNode();
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::characters( const OUString& aChars )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    saxHelper.characters(aChars);
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::ignorableWhitespace( const OUString& aWhitespaces )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    saxHelper.ignorableWhitespace(aWhitespaces);
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::processingInstruction( const OUString& aTarget, const OUString& aData )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    saxHelper.processingInstruction(aTarget, aData);
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::setDocumentLocator( const cssu::Reference< cssxs::XLocator >& )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
}

/* XCompressedDocumentHandler */
void SAL_CALL XMLDocumentWrapper_XmlSecImpl::compressedStartDocument(  )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::compressedEndDocument(  )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::compressedStartElement( const OUString& aName, const cssu::Sequence< cssxcsax::XMLAttribute >& aAttributes )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    saxHelper.startElement(aName, aAttributes);
    m_pCurrentElement = saxHelper.getCurrentNode();

    buildIDAttr( m_pCurrentElement );
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::compressedEndElement( const OUString& aName )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    endElement( aName );
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::compressedCharacters( const OUString& aChars )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    characters( aChars );
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::compressedIgnorableWhitespace( const OUString& aWhitespaces )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    ignorableWhitespace( aWhitespaces );
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::compressedProcessingInstruction( const OUString& aTarget, const OUString& aData )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
    processingInstruction( aTarget, aData );
}

void SAL_CALL XMLDocumentWrapper_XmlSecImpl::compressedSetDocumentLocator( sal_Int32 /*columnNumber*/, sal_Int32 /*lineNumber*/, const OUString& /*publicId*/, const OUString& /*systemId*/ )
    throw (cssxs::SAXException, cssu::RuntimeException, std::exception)
{
}

OUString XMLDocumentWrapper_XmlSecImpl_getImplementationName ()
    throw (cssu::RuntimeException)
{
    return OUString ( IMPLEMENTATION_NAME );
}

cssu::Sequence< OUString > SAL_CALL XMLDocumentWrapper_XmlSecImpl_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.xml.wrapper.XMLDocumentWrapper";
    return aRet;
}

cssu::Reference< cssu::XInterface > SAL_CALL XMLDocumentWrapper_XmlSecImpl_createInstance(
    const cssu::Reference< cssu::XComponentContext > &)
    throw( cssu::Exception )
{
    return static_cast<cppu::OWeakObject*>(new XMLDocumentWrapper_XmlSecImpl( ));
}

/* XServiceInfo */
OUString SAL_CALL XMLDocumentWrapper_XmlSecImpl::getImplementationName(  )
    throw (cssu::RuntimeException, std::exception)
{
    return XMLDocumentWrapper_XmlSecImpl_getImplementationName();
}
sal_Bool SAL_CALL XMLDocumentWrapper_XmlSecImpl::supportsService( const OUString& rServiceName )
    throw (cssu::RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}
cssu::Sequence< OUString > SAL_CALL XMLDocumentWrapper_XmlSecImpl::getSupportedServiceNames(  )
    throw (cssu::RuntimeException, std::exception)
{
    return XMLDocumentWrapper_XmlSecImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
