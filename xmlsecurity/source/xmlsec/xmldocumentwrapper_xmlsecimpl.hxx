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
    css::xml::wrapper::XXMLDocumentWrapper,
    css::xml::sax::XDocumentHandler,
    css::xml::csax::XCompressedDocumentHandler,
    css::lang::XServiceInfo
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
    css::uno::Sequence< css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > > m_aReservedNodes;
    sal_Int32 m_nReservedNodeIndex;

private:
    void getNextSAXEvent();

    static void sendStartElement(
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xHandler,
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xHandler2,
        const xmlNodePtr pNode)
        throw (css::xml::sax::SAXException);

    static void sendEndElement(
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xHandler,
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xHandler2,
        const xmlNodePtr pNode)
        throw (css::xml::sax::SAXException);

    static void sendNode(
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xHandler,
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xHandler2,
        const xmlNodePtr pNode)
        throw (css::xml::sax::SAXException);

    static OString getNodeQName(const xmlNodePtr pNode);

    sal_Int32 recursiveDelete( const xmlNodePtr pNode);

    void getNextReservedNode();

    void removeNode( const xmlNodePtr pNode) const;

    static xmlNodePtr checkElement(
        const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& xXMLElement);

    void buildIDAttr( xmlNodePtr pNode ) const;
    void rebuildIDLink( xmlNodePtr pNode ) const;

public:
    XMLDocumentWrapper_XmlSecImpl();
    virtual ~XMLDocumentWrapper_XmlSecImpl();

    /* css::xml::wrapper::XXMLDocumentWrapper */
    virtual css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > SAL_CALL getCurrentElement(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setCurrentElement( const css::uno::Reference<
        css::xml::wrapper::XXMLElementWrapper >& element )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeCurrentElement(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isCurrent( const css::uno::Reference<
        css::xml::wrapper::XXMLElementWrapper >& node )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isCurrentElementEmpty(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getNodeName( const css::uno::Reference<
        css::xml::wrapper::XXMLElementWrapper >& node )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL clearUselessData(
        const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& node,
        const css::uno::Sequence< css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > >& reservedDescendants,
        const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& stopAtNode )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL collapse( const css::uno::Reference<
        css::xml::wrapper::XXMLElementWrapper >& node )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL generateSAXEvents(
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& handler,
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xEventKeeperHandler,
        const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& startNode,
        const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& endNode )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL getTree(
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& handler )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL rebuildIDLink(
        const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& node )
        throw (css::uno::RuntimeException, std::exception) override;

    /* css::xml::sax::XDocumentHandler */
    virtual void SAL_CALL startDocument(  )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL endDocument(  )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL startElement(
        const OUString& aName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL endElement( const OUString& aName )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL characters( const OUString& aChars )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    /* css::xml::csax::XCompressedDocumentHandler */
    virtual void SAL_CALL compressedStartDocument(  )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedEndDocument(  )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedStartElement(
        const OUString& aName,
        const css::uno::Sequence< css::xml::csax::XMLAttribute >& aAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedEndElement( const OUString& aName )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedCharacters( const OUString& aChars )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedIgnorableWhitespace( const OUString& aWhitespaces )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedProcessingInstruction( const OUString& aTarget, const OUString& aData )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL compressedSetDocumentLocator(
        sal_Int32 columnNumber,
        sal_Int32 lineNumber,
        const OUString& publicId,
        const OUString& systemId )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    /* css::lang::XServiceInfo */
    virtual OUString SAL_CALL getImplementationName(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (css::uno::RuntimeException, std::exception) override;
};

OUString XMLDocumentWrapper_XmlSecImpl_getImplementationName()
    throw ( css::uno::RuntimeException );

css::uno::Sequence< OUString > SAL_CALL
    XMLDocumentWrapper_XmlSecImpl_getSupportedServiceNames(  )
    throw ( css::uno::RuntimeException );

css::uno::Reference< css::uno::XInterface >
SAL_CALL XMLDocumentWrapper_XmlSecImpl_createInstance(
    const css::uno::Reference< css::uno::XComponentContext > &)
    throw ( css::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
