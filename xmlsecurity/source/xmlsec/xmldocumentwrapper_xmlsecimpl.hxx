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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_XMLDOCUMENTWRAPPER_XMLSECIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_XMLDOCUMENTWRAPPER_XMLSECIMPL_HXX

#include <com/sun/star/xml/wrapper/XXMLDocumentWrapper.hpp>
#include <com/sun/star/xml/csax/XCompressedDocumentHandler.hpp>
#include <com/sun/star/xml/crypto/sax/XSAXEventKeeper.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

#include "saxhelper.hxx"

#define NODEPOSITION_NORMAL        1
#define NODEPOSITION_STARTELEMENT  2
#define NODEPOSITION_ENDELEMENT    3

#include <libxml/tree.h>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

class XMLDocumentWrapper_XmlSecImpl : public cppu::WeakImplHelper
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

    static void sendStartElement(
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xHandler,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xHandler2,
        const xmlNodePtr pNode)
        throw (com::sun::star::xml::sax::SAXException);

    static void sendEndElement(
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xHandler,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xHandler2,
        const xmlNodePtr pNode)
        throw (com::sun::star::xml::sax::SAXException);

    static void sendNode(
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xHandler,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xHandler2,
        const xmlNodePtr pNode)
        throw (com::sun::star::xml::sax::SAXException);

    static OString getNodeQName(const xmlNodePtr pNode);

    sal_Int32 recursiveDelete( const xmlNodePtr pNode);

    void getNextReservedNode();

    void removeNode( const xmlNodePtr pNode) const;

    static xmlNodePtr checkElement(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper >& xXMLElement);

    void buildIDAttr( xmlNodePtr pNode ) const;
    void rebuildIDLink( xmlNodePtr pNode ) const;

public:
    XMLDocumentWrapper_XmlSecImpl();
    virtual ~XMLDocumentWrapper_XmlSecImpl();

    /* com::sun::star::xml::wrapper::XXMLDocumentWrapper */
    virtual com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper > SAL_CALL getCurrentElement(  )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setCurrentElement( const com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper >& element )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeCurrentElement(  )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isCurrent( const com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper >& node )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isCurrentElementEmpty(  )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getNodeName( const com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper >& node )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL clearUselessData(
        const com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper >& node,
        const com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper > >& reservedDescendants,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper >& stopAtNode )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL collapse( const com::sun::star::uno::Reference<
        com::sun::star::xml::wrapper::XXMLElementWrapper >& node )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL generateSAXEvents(
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& handler,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& xEventKeeperHandler,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper >& startNode,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::wrapper::XXMLElementWrapper >& endNode )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL getTree(
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler >& handler )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL rebuildIDLink(
        const com::sun::star::uno::Reference< com::sun::star::xml::wrapper::XXMLElementWrapper >& node )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    /* com::sun::star::xml::sax::XDocumentHandler */
    virtual void SAL_CALL startDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL endDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL startElement(
        const OUString& aName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttribs )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL endElement( const OUString& aName )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL characters( const OUString& aChars )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setDocumentLocator( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XLocator >& xLocator )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    /* com::sun::star::xml::csax::XCompressedDocumentHandler */
    virtual void SAL_CALL compressedStartDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedEndDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedStartElement(
        const OUString& aName,
        const com::sun::star::uno::Sequence<
            com::sun::star::xml::csax::XMLAttribute >& aAttributes )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedEndElement( const OUString& aName )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedCharacters( const OUString& aChars )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedIgnorableWhitespace( const OUString& aWhitespaces )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedProcessingInstruction( const OUString& aTarget, const OUString& aData )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedSetDocumentLocator(
        sal_Int32 columnNumber,
        sal_Int32 lineNumber,
        const OUString& publicId,
        const OUString& systemId )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException, std::exception) override;

    /* com::sun::star::lang::XServiceInfo */
    virtual OUString SAL_CALL getImplementationName(  )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;
};

OUString XMLDocumentWrapper_XmlSecImpl_getImplementationName()
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< OUString > SAL_CALL
    XMLDocumentWrapper_XmlSecImpl_getSupportedServiceNames(  )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
SAL_CALL XMLDocumentWrapper_XmlSecImpl_createInstance(
    const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > &)
    throw ( com::sun::star::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
