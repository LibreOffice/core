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

#pragma once

#include <com/sun/star/xml/wrapper/XXMLDocumentWrapper.hpp>
#include <com/sun/star/xml/csax/XCompressedDocumentHandler.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

#include <xmlsec/saxhelper.hxx>
#include <xsecxmlsecdllapi.h>

#define NODEPOSITION_NORMAL        1
#define NODEPOSITION_STARTELEMENT  2
#define NODEPOSITION_ENDELEMENT    3

class XMLDocumentWrapper_XmlSecImpl final : public cppu::WeakImplHelper
<
    css::xml::wrapper::XXMLDocumentWrapper,
    css::xml::sax::XDocumentHandler,
    css::xml::csax::XCompressedDocumentHandler,
    css::lang::XServiceInfo
>
/**
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

    /// @throws css::xml::sax::SAXException
    static void sendStartElement(
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xHandler,
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xHandler2,
        const xmlNodePtr pNode);

    /// @throws css::xml::sax::SAXException
    static void sendEndElement(
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xHandler,
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xHandler2,
        const xmlNodePtr pNode);

    /// @throws css::xml::sax::SAXException
    static void sendNode(
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xHandler,
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xHandler2,
        const xmlNodePtr pNode);

    static OString getNodeQName(const xmlNodePtr pNode);

    sal_Int32 recursiveDelete( const xmlNodePtr pNode);

    void getNextReservedNode();

    void removeNode( const xmlNodePtr pNode) const;

    static xmlNodePtr checkElement(
        const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& xXMLElement);

    void buildIDAttr( xmlNodePtr pNode ) const;
    void rebuildIDLink( xmlNodePtr pNode ) const;

public:
    XSECXMLSEC_DLLPUBLIC XMLDocumentWrapper_XmlSecImpl();
    virtual ~XMLDocumentWrapper_XmlSecImpl() override;

    /* css::xml::wrapper::XXMLDocumentWrapper */
    virtual css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > SAL_CALL getCurrentElement(  ) override;

    virtual void SAL_CALL setCurrentElement( const css::uno::Reference<
        css::xml::wrapper::XXMLElementWrapper >& element ) override;

    virtual void SAL_CALL removeCurrentElement(  ) override;

    virtual sal_Bool SAL_CALL isCurrent( const css::uno::Reference<
        css::xml::wrapper::XXMLElementWrapper >& node ) override;

    virtual sal_Bool SAL_CALL isCurrentElementEmpty(  ) override;

    virtual OUString SAL_CALL getNodeName( const css::uno::Reference<
        css::xml::wrapper::XXMLElementWrapper >& node ) override;

    virtual void SAL_CALL clearUselessData(
        const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& node,
        const css::uno::Sequence< css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > >& reservedDescendants,
        const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& stopAtNode ) override;

    virtual void SAL_CALL collapse( const css::uno::Reference<
        css::xml::wrapper::XXMLElementWrapper >& node ) override;

    virtual void SAL_CALL generateSAXEvents(
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& handler,
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& xEventKeeperHandler,
        const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& startNode,
        const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& endNode ) override;

    virtual void SAL_CALL getTree(
        const css::uno::Reference< css::xml::sax::XDocumentHandler >& handler ) override;

    virtual void SAL_CALL rebuildIDLink(
        const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& node ) override;

    /* css::xml::sax::XDocumentHandler */
    virtual void SAL_CALL startDocument(  ) override;

    virtual void SAL_CALL endDocument(  ) override;

    virtual void SAL_CALL startElement(
        const OUString& aName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs ) override;

    virtual void SAL_CALL endElement( const OUString& aName ) override;

    virtual void SAL_CALL characters( const OUString& aChars ) override;

    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) override;

    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) override;

    virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator ) override;

    /* css::xml::csax::XCompressedDocumentHandler */
    virtual void SAL_CALL compressedStartDocument(  ) override;

    virtual void SAL_CALL compressedEndDocument(  ) override;

    virtual void SAL_CALL compressedStartElement(
        const OUString& aName,
        const css::uno::Sequence< css::xml::csax::XMLAttribute >& aAttributes ) override;

    virtual void SAL_CALL compressedEndElement( const OUString& aName ) override;

    virtual void SAL_CALL compressedCharacters( const OUString& aChars ) override;

    virtual void SAL_CALL compressedIgnorableWhitespace( const OUString& aWhitespaces ) override;

    virtual void SAL_CALL compressedProcessingInstruction( const OUString& aTarget, const OUString& aData ) override;

    virtual void SAL_CALL compressedSetDocumentLocator(
        sal_Int32 columnNumber,
        sal_Int32 lineNumber,
        const OUString& publicId,
        const OUString& systemId ) override;

    /* css::lang::XServiceInfo */
    virtual OUString SAL_CALL getImplementationName(  ) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
