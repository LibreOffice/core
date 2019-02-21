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

#include "saxbuilder.hxx"

#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sax/fastattribs.hxx>
#include <xmloff/xmlimp.hxx>

using namespace css::lang;
using namespace css::uno;
using namespace css::xml::dom;
using namespace css::xml::sax;

namespace DOM
{
    Reference< XInterface > CSAXDocumentBuilder::_getInstance(const Reference< XMultiServiceFactory >& rSMgr)
    {
        return static_cast< XSAXDocumentBuilder* >(new CSAXDocumentBuilder(rSMgr));
    }

    static const char aImplementationName[] = "com.sun.star.comp.xml.dom.SAXDocumentBuilder";
    static const char* aSupportedServiceNames[] = {
        "com.sun.star.xml.dom.SAXDocumentBuilder",
        nullptr
    };

    CSAXDocumentBuilder::CSAXDocumentBuilder(const Reference< XMultiServiceFactory >& mgr)
        : m_aServiceManager(mgr)
        , m_aState( SAXDocumentBuilderState_READY)
    {}

    OUString CSAXDocumentBuilder::_getImplementationName()
    {
        return OUString(aImplementationName);
    }
    Sequence<OUString> CSAXDocumentBuilder::_getSupportedServiceNames()
    {
        Sequence<OUString> aSequence;
        for (int i=0; aSupportedServiceNames[i]!=nullptr; i++) {
            aSequence.realloc(i+1);
            aSequence[i] = OUString::createFromAscii(aSupportedServiceNames[i]);
        }
        return aSequence;
    }

    Sequence< OUString > SAL_CALL CSAXDocumentBuilder::getSupportedServiceNames()
    {
        return CSAXDocumentBuilder::_getSupportedServiceNames();
    }

    OUString SAL_CALL CSAXDocumentBuilder::getImplementationName()
    {
        return CSAXDocumentBuilder::_getImplementationName();
    }

    sal_Bool SAL_CALL CSAXDocumentBuilder::supportsService(const OUString& aServiceName)
    {
        return cppu::supportsService(this, aServiceName);
    }

    SAXDocumentBuilderState SAL_CALL CSAXDocumentBuilder::getState()
    {
        ::osl::MutexGuard g(m_Mutex);

        return m_aState;
    }

    void SAL_CALL CSAXDocumentBuilder::reset()
    {
        ::osl::MutexGuard g(m_Mutex);

        m_aDocument.clear();
        m_aFragment.clear();
        while (!m_aNodeStack.empty()) m_aNodeStack.pop();
        m_aState = SAXDocumentBuilderState_READY;
    }

    Reference< XDocument > SAL_CALL CSAXDocumentBuilder::getDocument()
    {
        ::osl::MutexGuard g(m_Mutex);

        if (m_aState != SAXDocumentBuilderState_DOCUMENT_FINISHED)
            throw RuntimeException();

        return m_aDocument;
    }

    Reference< XDocumentFragment > SAL_CALL CSAXDocumentBuilder::getDocumentFragment()
    {
        ::osl::MutexGuard g(m_Mutex);

        if (m_aState != SAXDocumentBuilderState_FRAGMENT_FINISHED)
            throw RuntimeException();
        return m_aFragment;
    }

    void SAL_CALL CSAXDocumentBuilder::startDocumentFragment(const Reference< XDocument >& ownerDoc)
    {
        ::osl::MutexGuard g(m_Mutex);

        // start a new document fragment and push it onto the stack
        // we have to be in a clean state to do this
        if (m_aState != SAXDocumentBuilderState_READY)
            throw RuntimeException();

        m_aDocument = ownerDoc;
        Reference< XDocumentFragment > aFragment = m_aDocument->createDocumentFragment();
        m_aNodeStack.push(aFragment);
        m_aFragment = aFragment;
        m_aState = SAXDocumentBuilderState_BUILDING_FRAGMENT;
    }

    void SAL_CALL CSAXDocumentBuilder::endDocumentFragment()
    {
        ::osl::MutexGuard g(m_Mutex);

        // there should only be the document left on the node stack
        if (m_aState != SAXDocumentBuilderState_BUILDING_FRAGMENT)
            throw RuntimeException();

        Reference< XNode > aNode = m_aNodeStack.top();
        if ( aNode->getNodeType() != NodeType_DOCUMENT_FRAGMENT_NODE)
            throw RuntimeException();
        m_aNodeStack.pop();
        m_aState = SAXDocumentBuilderState_FRAGMENT_FINISHED;
    }

    //XFastDocumentHandler
    void SAL_CALL CSAXDocumentBuilder::startDocument()
    {
        ::osl::MutexGuard g(m_Mutex);

        // start a new document and push it onto the stack
        // we have to be in a clean state to do this
        if (m_aState != SAXDocumentBuilderState_READY)
            throw SAXException();

        Reference< XDocumentBuilder > aBuilder(DocumentBuilder::create(comphelper::getComponentContext(m_aServiceManager)));
        Reference< XDocument > aDocument = aBuilder->newDocument();
        m_aNodeStack.push(aDocument);
        m_aDocument = aDocument;
        m_aState = SAXDocumentBuilderState_BUILDING_DOCUMENT;
    }

    void SAL_CALL CSAXDocumentBuilder::endDocument()
    {
        ::osl::MutexGuard g(m_Mutex);

        // there should only be the document left on the node stack
        if (m_aState != SAXDocumentBuilderState_BUILDING_DOCUMENT)
            throw SAXException();

        Reference< XNode > aNode = m_aNodeStack.top();
        if ( aNode->getNodeType() != NodeType_DOCUMENT_NODE)
            throw SAXException();
        m_aNodeStack.pop();
        m_aState = SAXDocumentBuilderState_DOCUMENT_FINISHED;
    }

    void SAL_CALL CSAXDocumentBuilder::processingInstruction( const OUString& rTarget, const OUString& rData )
    {
        ::osl::MutexGuard g(m_Mutex);

        //  append PI node to the current top
        if ( m_aState != SAXDocumentBuilderState_BUILDING_DOCUMENT &&
             m_aState != SAXDocumentBuilderState_BUILDING_FRAGMENT)
            throw SAXException();

        Reference< XProcessingInstruction > aInstruction = m_aDocument->createProcessingInstruction(
                rTarget, rData);
        m_aNodeStack.top()->appendChild(aInstruction);
    }

    void SAL_CALL CSAXDocumentBuilder::setDocumentLocator( const Reference< XLocator >& )
    {
    }

    void SAL_CALL CSAXDocumentBuilder::startFastElement( sal_Int32 nElement , const Reference< XFastAttributeList >& xAttribs  )
    {
        ::osl::MutexGuard g(m_Mutex);

        if ( m_aState != SAXDocumentBuilderState_BUILDING_DOCUMENT &&
             m_aState != SAXDocumentBuilderState_BUILDING_FRAGMENT)
        {
            throw SAXException();
        }

        Reference< XElement > aElement;
        const OUString& aPrefix(SvXMLImport::getNamespacePrefixFromToken(nElement, nullptr));
        const OUString& aURI( SvXMLImport::getNamespaceURIFromToken( nElement ) );
        OUString aQualifiedName( SvXMLImport::getNameFromToken( nElement ) );
        if( !aPrefix.isEmpty() )
            aQualifiedName = aPrefix + SvXMLImport::aNamespaceSeparator + aQualifiedName;

        if ( !aURI.isEmpty() )
        {
            // found a URI for prefix
            // qualified name
            aElement = m_aDocument->createElementNS( aURI, aQualifiedName );
        }
        else
        {
            // no URI for prefix
            aElement = m_aDocument->createElement( aQualifiedName );
        }
        aElement.set( m_aNodeStack.top()->appendChild(aElement), UNO_QUERY);
        m_aNodeStack.push(aElement);

        if (xAttribs.is())
            setElementFastAttributes(aElement, xAttribs);
    }

    // For arbitrary meta elements
    void SAL_CALL CSAXDocumentBuilder::startUnknownElement( const OUString& rNamespace, const OUString& rName, const Reference< XFastAttributeList >& xAttribs )
    {
        ::osl::MutexGuard g(m_Mutex);

        if ( m_aState != SAXDocumentBuilderState_BUILDING_DOCUMENT &&
             m_aState != SAXDocumentBuilderState_BUILDING_FRAGMENT)
        {
            throw SAXException();
        }

        Reference< XElement > aElement;
        if ( !rNamespace.isEmpty() )
            aElement = m_aDocument->createElementNS( rNamespace, rName );
        else
            aElement = m_aDocument->createElement( rName );

        aElement.set( m_aNodeStack.top()->appendChild(aElement), UNO_QUERY);
        m_aNodeStack.push(aElement);

        if (xAttribs.is())
        {
            setElementFastAttributes(aElement, xAttribs);
            Sequence< css::xml::Attribute > unknownAttribs = xAttribs->getUnknownAttributes();
            sal_Int32 len = unknownAttribs.getLength();
            for ( sal_Int32 i = 0; i < len; i++ )
            {
                const OUString& rAttrValue = unknownAttribs[i].Value;
                const OUString& rAttrName = unknownAttribs[i].Name;
                const OUString& rAttrNamespace = unknownAttribs[i].NamespaceURL;
                if ( !rAttrNamespace.isEmpty() )
                    aElement->setAttributeNS( rAttrNamespace, rAttrName, rAttrValue );
                else
                    aElement->setAttribute( rAttrName, rAttrValue );
            }
        }
    }

    void CSAXDocumentBuilder::setElementFastAttributes(const Reference< XElement >& aElement, const Reference< XFastAttributeList >& xAttribs)
    {
        sax_fastparser::FastAttributeList *pAttribList =
            sax_fastparser::FastAttributeList::castToFastAttributeList( xAttribs );

        for (auto &it : *pAttribList)
        {
            sal_Int32 nAttrToken = it.getToken();
            const OUString& aAttrPrefix(SvXMLImport::getNamespacePrefixFromToken(nAttrToken, nullptr));
            const OUString& aAttrURI( SvXMLImport::getNamespaceURIFromToken( nAttrToken ) );
            OUString aAttrQualifiedName( SvXMLImport::getNameFromToken( nAttrToken ) );
            if( !aAttrPrefix.isEmpty() )
                aAttrQualifiedName = aAttrPrefix + SvXMLImport::aNamespaceSeparator + aAttrQualifiedName;

            if ( !aAttrURI.isEmpty() )
                aElement->setAttributeNS( aAttrURI, aAttrQualifiedName, it.toString() );
            else
                aElement->setAttribute( aAttrQualifiedName, it.toString() );
        }
    }

    void SAL_CALL CSAXDocumentBuilder::endFastElement( sal_Int32 nElement )
    {
        ::osl::MutexGuard g(m_Mutex);

        // pop the current element from the stack
        if ( m_aState != SAXDocumentBuilderState_BUILDING_DOCUMENT &&
             m_aState != SAXDocumentBuilderState_BUILDING_FRAGMENT)
            throw SAXException();

        Reference< XNode > aNode(m_aNodeStack.top());
        if (aNode->getNodeType() != NodeType_ELEMENT_NODE)
            throw SAXException();

        Reference< XElement > aElement(aNode, UNO_QUERY);
        if( aElement->getPrefix() != SvXMLImport::getNamespacePrefixFromToken(nElement, nullptr) ||
            aElement->getTagName() != SvXMLImport::getNameFromToken( nElement ) ) // consistency check
            throw SAXException();

        // pop it
        m_aNodeStack.pop();
    }


    void SAL_CALL CSAXDocumentBuilder::endUnknownElement( const OUString& /*rNamespace*/, const OUString& rName )
    {
        ::osl::MutexGuard g(m_Mutex);

        // pop the current element from the stack
        if ( m_aState != SAXDocumentBuilderState_BUILDING_DOCUMENT &&
             m_aState != SAXDocumentBuilderState_BUILDING_FRAGMENT)
            throw SAXException();

        Reference< XNode > aNode(m_aNodeStack.top());
        if (aNode->getNodeType() != NodeType_ELEMENT_NODE)
            throw SAXException();

        Reference< XElement > aElement(aNode, UNO_QUERY);
        OUString aRefName;
        const OUString& aPrefix = aElement->getPrefix();
        if (!aPrefix.isEmpty())
            aRefName = aPrefix + SvXMLImport::aNamespaceSeparator + aElement->getTagName();
        else
            aRefName = aElement->getTagName();
        if (aRefName != rName) // consistency check
            throw SAXException();

        // pop it
        m_aNodeStack.pop();
    }

    Reference< XFastContextHandler > SAL_CALL CSAXDocumentBuilder::createFastChildContext( sal_Int32/* nElement */, const Reference< XFastAttributeList >&/* xAttribs */ )
    {
        return nullptr;
    }


    Reference< XFastContextHandler > SAL_CALL CSAXDocumentBuilder::createUnknownChildContext( const OUString&/* rNamespace */, const OUString&/* rName */, const Reference< XFastAttributeList >&/* xAttribs */ )
    {
        return nullptr;
    }

    void SAL_CALL CSAXDocumentBuilder::characters( const OUString& rChars )
    {
        ::osl::MutexGuard g(m_Mutex);

        //  append text node to the current top element
        if (m_aState != SAXDocumentBuilderState_BUILDING_DOCUMENT &&
             m_aState != SAXDocumentBuilderState_BUILDING_FRAGMENT)
            throw SAXException();

        Reference< XText > aText = m_aDocument->createTextNode(rChars);
        m_aNodeStack.top()->appendChild(aText);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
