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
#ifdef _MSC_VER
#pragma warning(disable : 4701)
#endif

#include "saxbuilder.hxx"

#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

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

    const char* CSAXDocumentBuilder::aImplementationName = "com.sun.star.comp.xml.dom.SAXDocumentBuilder";
    const char* CSAXDocumentBuilder::aSupportedServiceNames[] = {
        "com.sun.star.xml.dom.SAXDocumentBuilder",
        nullptr
    };

    CSAXDocumentBuilder::CSAXDocumentBuilder(const Reference< XMultiServiceFactory >& mgr)
        : m_aServiceManager(mgr)
        , m_aState( SAXDocumentBuilderState_READY)
    {}

    OUString CSAXDocumentBuilder::_getImplementationName()
    {
        return OUString::createFromAscii(aImplementationName);
    }
    Sequence<OUString> CSAXDocumentBuilder::_getSupportedServiceNames()
    {
        Sequence<OUString> aSequence;
        for (int i=0; aSupportedServiceNames[i]!=nullptr; i++) {
            aSequence.realloc(i+1);
            aSequence[i]=(OUString::createFromAscii(aSupportedServiceNames[i]));
        }
        return aSequence;
    }

    Sequence< OUString > SAL_CALL CSAXDocumentBuilder::getSupportedServiceNames()
        throw (RuntimeException, std::exception)
    {
        return CSAXDocumentBuilder::_getSupportedServiceNames();
    }

    OUString SAL_CALL CSAXDocumentBuilder::getImplementationName()
        throw (RuntimeException, std::exception)
    {
        return CSAXDocumentBuilder::_getImplementationName();
    }

    sal_Bool SAL_CALL CSAXDocumentBuilder::supportsService(const OUString& aServiceName)
        throw (RuntimeException, std::exception)
    {
        return cppu::supportsService(this, aServiceName);
    }

    SAXDocumentBuilderState SAL_CALL CSAXDocumentBuilder::getState()
        throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard g(m_Mutex);

        return m_aState;
    }

    void SAL_CALL CSAXDocumentBuilder::reset()
        throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard g(m_Mutex);

        m_aDocument.clear();
        m_aFragment.clear();
        while (!m_aNodeStack.empty()) m_aNodeStack.pop();
        while (!m_aNSStack.empty()) m_aNSStack.pop();
        m_aState = SAXDocumentBuilderState_READY;
    }

    Reference< XDocument > SAL_CALL CSAXDocumentBuilder::getDocument()
        throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard g(m_Mutex);

        if (m_aState != SAXDocumentBuilderState_DOCUMENT_FINISHED)
            throw RuntimeException();

        return m_aDocument;
    }

    Reference< XDocumentFragment > SAL_CALL CSAXDocumentBuilder::getDocumentFragment()
         throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard g(m_Mutex);

        if (m_aState != SAXDocumentBuilderState_FRAGMENT_FINISHED)
            throw RuntimeException();
        return m_aFragment;
    }

    void SAL_CALL CSAXDocumentBuilder::startDocumentFragment(const Reference< XDocument >& ownerDoc)
        throw (RuntimeException, std::exception)
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
        throw (RuntimeException, std::exception)
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

    // document handler

    void SAL_CALL  CSAXDocumentBuilder::startDocument() throw (RuntimeException, SAXException, std::exception)
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

    void SAL_CALL CSAXDocumentBuilder::endDocument() throw (RuntimeException, SAXException, std::exception)
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

    void SAL_CALL CSAXDocumentBuilder::startElement(const OUString& aName, const Reference< XAttributeList>& attribs)
        throw (RuntimeException, SAXException, std::exception)
    {
        ::osl::MutexGuard g(m_Mutex);

        if ( m_aState != SAXDocumentBuilderState_BUILDING_DOCUMENT &&
             m_aState != SAXDocumentBuilderState_BUILDING_FRAGMENT)
        {
            throw SAXException();
        }

        // start with mappings in effect for last level
        NSMap aNSMap;
        if (!m_aNSStack.empty())
            aNSMap = NSMap(m_aNSStack.top());

        // handle xmlns: attributes and add to mappings
        OUString attr_qname;
        OUString attr_value;
        OUString newprefix;
        AttrMap aAttrMap;
        sal_Int32 idx=-1;
        sal_Int16 nAttributes = attribs->getLength();
        for (sal_Int16 i=0; i<nAttributes; i++)
        {
            attr_qname = attribs->getNameByIndex(i);
            attr_value = attribs->getValueByIndex(i);
            // new prefix mapping
            if (attr_qname.startsWith("xmlns:"))
            {
                newprefix = attr_qname.copy(attr_qname.indexOf(':')+1);
                aNSMap.insert(NSMap::value_type(newprefix, attr_value));
            }
            else if ( attr_qname == "xmlns" )
            {
                // new default prefix
                aNSMap.insert(NSMap::value_type(OUString(), attr_value));
            }
            else
            {
                aAttrMap.insert(AttrMap::value_type(attr_qname, attr_value));
            }
        }

        // does the element have a prefix?
        OUString aPrefix;
        OUString aURI;
        Reference< XElement > aElement;
        idx = aName.indexOf(':');
        if (idx != -1)
        {
            aPrefix = aName.copy(0, idx);
        }
        else
            aPrefix.clear();

        NSMap::const_iterator result = aNSMap.find(aPrefix);
        if ( result != aNSMap.end())
        {
            // found a URI for prefix
            // qualified name
            aElement = m_aDocument->createElementNS( result->second, aName);
        }
        else
        {
            // no URI for prefix
            aElement = m_aDocument->createElement(aName);
        }
        aElement.set( m_aNodeStack.top()->appendChild(aElement), UNO_QUERY);
        m_aNodeStack.push(aElement);

        // set non xmlns attributes
        aPrefix.clear();
        aURI.clear();
        AttrMap::const_iterator a = aAttrMap.begin();
        while (a != aAttrMap.end())
        {
            attr_qname = a->first;
            attr_value = a->second;
            idx = attr_qname.indexOf(':');
            if (idx != -1)
                aPrefix = attr_qname.copy(0, idx);
            else
                aPrefix.clear();

            result = aNSMap.find(aPrefix);
            if (result != aNSMap.end())
            {
                // set attribute with namespace
                aElement->setAttributeNS(result->second, attr_qname, attr_value);
            }
            else
            {
                // set attribute without namespace
                aElement->setAttribute(attr_qname, attr_value);
            }
            ++a;
        }
        m_aNSStack.push(aNSMap);
    }

    void SAL_CALL CSAXDocumentBuilder::endElement(const OUString& aName)
        throw (RuntimeException, SAXException, std::exception)
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
        OUString aPrefix = aElement->getPrefix();
        if (!aPrefix.isEmpty())
            aRefName = aPrefix + ":" + aElement->getTagName();
        else
            aRefName = aElement->getTagName();
        if (aRefName != aName) // consistency check
            throw SAXException();

        // pop it
        m_aNodeStack.pop();
        m_aNSStack.pop();
    }

    void SAL_CALL CSAXDocumentBuilder::characters(const OUString& aChars)
        throw (RuntimeException, SAXException, std::exception)
    {
        ::osl::MutexGuard g(m_Mutex);

        //  append text node to the current top element
         if (m_aState != SAXDocumentBuilderState_BUILDING_DOCUMENT &&
             m_aState != SAXDocumentBuilderState_BUILDING_FRAGMENT)
            throw SAXException();

         Reference< XText > aText = m_aDocument->createTextNode(aChars);
         m_aNodeStack.top()->appendChild(aText);
    }

    void SAL_CALL CSAXDocumentBuilder::ignorableWhitespace(const OUString& )
        throw (RuntimeException, SAXException, std::exception)
    {
        ::osl::MutexGuard g(m_Mutex);

        //  ignore ignorable whitespace
        if ( m_aState != SAXDocumentBuilderState_BUILDING_DOCUMENT &&
             m_aState != SAXDocumentBuilderState_BUILDING_FRAGMENT)
            throw SAXException();
    }

    void SAL_CALL CSAXDocumentBuilder::processingInstruction(const OUString& aTarget, const OUString& aData)
        throw (RuntimeException, SAXException, std::exception)
    {
        ::osl::MutexGuard g(m_Mutex);

        //  append PI node to the current top
        if ( m_aState != SAXDocumentBuilderState_BUILDING_DOCUMENT &&
             m_aState != SAXDocumentBuilderState_BUILDING_FRAGMENT)
            throw SAXException();

        Reference< XProcessingInstruction > aInstruction = m_aDocument->createProcessingInstruction(
                aTarget, aData);
        m_aNodeStack.top()->appendChild(aInstruction);
    }

    void SAL_CALL CSAXDocumentBuilder::setDocumentLocator(const Reference< XLocator >& aLocator)
        throw (RuntimeException, SAXException, std::exception)
    {
        ::osl::MutexGuard g(m_Mutex);

        // set the document locator...
        m_aLocator = aLocator;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
