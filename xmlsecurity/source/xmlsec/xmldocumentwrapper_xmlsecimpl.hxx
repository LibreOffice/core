/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _XMLDOCUMENTWRAPPER_XMLSECIMPL_HXX
#define _XMLDOCUMENTWRAPPER_XMLSECIMPL_HXX

#include <com/sun/star/xml/wrapper/XXMLDocumentWrapper.hpp>
#include <com/sun/star/xml/csax/XCompressedDocumentHandler.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeper.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase4.hxx>

#include "saxhelper.hxx"

#define NODEPOSITION_NORMAL        1
#define NODEPOSITION_STARTELEMENT  2
#define NODEPOSITION_ENDELEMENT    3

#include <libxml/tree.h>

class XMLDocumentWrapper_XmlSecImpl : public cppu::WeakImplHelper4
<
    com::sun::star::xml::wrapper::XXMLDocumentWrapper,
    com::sun::star::xml::sax::XDocumentHandler,
    com::sun::star::xml::csax::XCompressedDocumentHandler,
    com::sun::star::lang::XServiceInfo
>
/****** XMLDocumentWrapper_XmlSecImpl.hxx/CLASS XMLDocumentWrapper_XmlSecImpl *
 *
 *   NAME
 *  XMLDocumentWrapper_XmlSecImpl -- Class to manipulate a libxml2
 *  document
 *
 *   FUNCTION
 *  Converts SAX events into a libxml2 document, converts the document back
 *  into SAX event stream, and manipulate nodes in the document.
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
private:
    /* the sax helper */
    SAXHelper saxHelper;

    /* the document used to convert SAX events to */
    xmlDocPtr m_pDocument;

    /* the root element */
    xmlNodePtr m_pRootElement;

    /*
     * the current active element. The next incoming SAX event will be
     * appended to this element
     */
    xmlNodePtr m_pCurrentElement;

    /*
     * This variable is used when converting the document or part of it into
     * SAX events. See getNextSAXEvent method.
     */
    sal_Int32 m_nCurrentPosition;

    /*
     * used for recursive deletion. See recursiveDelete method
     */
    xmlNodePtr m_pStopAtNode;
    xmlNodePtr m_pCurrentReservedNode;
    com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper > > m_aReservedNodes;
    sal_Int32 m_nReservedNodeIndex;

private:
    void getNextSAXEvent();

    void sendStartElement(
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xHandler,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xHandler2,
        const xmlNodePtr pNode) const
        throw (com::sun::star::xml::sax::SAXException);

    void sendEndElement(
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xHandler,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xHandler2,
        const xmlNodePtr pNode) const
        throw (com::sun::star::xml::sax::SAXException);

    void sendNode(
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xHandler,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xHandler2,
        const xmlNodePtr pNode) const
        throw (com::sun::star::xml::sax::SAXException);

    rtl::OString getNodeQName(const xmlNodePtr pNode) const;

    sal_Int32 recursiveDelete( const xmlNodePtr pNode);

    void getNextReservedNode();

    void removeNode( const xmlNodePtr pNode) const;

    xmlNodePtr checkElement(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper >& xXMLElement) const;

    void buildIDAttr( xmlNodePtr pNode ) const;
    void rebuildIDLink( xmlNodePtr pNode ) const;

public:
    XMLDocumentWrapper_XmlSecImpl();
    virtual ~XMLDocumentWrapper_XmlSecImpl();

    /* com::sun::star::xml::wrapper::XXMLDocumentWrapper */
    virtual com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper > SAL_CALL getCurrentElement(  )
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setCurrentElement( const com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper >& element )
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeCurrentElement(  )
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isCurrent( const com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper >& node )
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isCurrentElementEmpty(  )
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getNodeName( const com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper >& node )
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL clearUselessData(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper >& node,
        const com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper > >& reservedDescendants,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper >& stopAtNode )
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL collapse( const com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper >& node )
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL generateSAXEvents(
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& handler,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xEventKeeperHandler,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper >& startNode,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper >& endNode )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL getTree(
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& handler )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL rebuildIDLink(
        const com::sun::star::uno::Reference< com::sun::star::xml::wrapper::XXMLElementWrapper >& node )
        throw (com::sun::star::uno::RuntimeException);

    /* com::sun::star::xml::sax::XDocumentHandler */
    virtual void SAL_CALL startDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL endDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL startElement(
        const rtl::OUString& aName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttribs )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL endElement( const rtl::OUString& aName )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL characters( const rtl::OUString& aChars )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL ignorableWhitespace( const rtl::OUString& aWhitespaces )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL processingInstruction( const rtl::OUString& aTarget, const rtl::OUString& aData )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setDocumentLocator( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XLocator >& xLocator )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    /* com::sun::star::xml::csax::XCompressedDocumentHandler */
    virtual void SAL_CALL _startDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL _endDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL _startElement(
        const rtl::OUString& aName,
        const com::sun::star::uno::Sequence<
            com::sun::star::xml::csax::XMLAttribute >& aAttributes )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL _endElement( const rtl::OUString& aName )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL _characters( const rtl::OUString& aChars )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL _ignorableWhitespace( const rtl::OUString& aWhitespaces )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL _processingInstruction( const rtl::OUString& aTarget, const rtl::OUString& aData )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL _setDocumentLocator(
        sal_Int32 columnNumber,
        sal_Int32 lineNumber,
        const rtl::OUString& publicId,
        const rtl::OUString& systemId )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    /* com::sun::star::lang::XServiceInfo */
    virtual rtl::OUString SAL_CALL getImplementationName(  )
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName )
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (com::sun::star::uno::RuntimeException);
};

rtl::OUString XMLDocumentWrapper_XmlSecImpl_getImplementationName()
    throw ( com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL XMLDocumentWrapper_XmlSecImpl_supportsService( const rtl::OUString& ServiceName )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    XMLDocumentWrapper_XmlSecImpl_getSupportedServiceNames(  )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
SAL_CALL XMLDocumentWrapper_XmlSecImpl_createInstance(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr)
    throw ( com::sun::star::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
