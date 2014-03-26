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

#ifndef INCLUDED_UNOXML_SOURCE_DOM_CDATASECTION_HXX
#define INCLUDED_UNOXML_SOURCE_DOM_CDATASECTION_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XCDATASection.hpp>

#include <text.hxx>


using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;

namespace DOM
{
    typedef ::cppu::ImplInheritanceHelper1< CText, XCDATASection >
        CCDATASection_Base;

    class CCDATASection
        : public CCDATASection_Base
    {
        friend class CDocument;

    protected:
        CCDATASection(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                xmlNodePtr const pNode);

    public:

        virtual void saxify(const Reference< XDocumentHandler >& i_xHandler) SAL_OVERRIDE;

        virtual Reference< XText > SAL_CALL splitText(sal_Int32 offset)
             throw (RuntimeException, std::exception) SAL_OVERRIDE
        {
            return CText::splitText(offset);
        }

         // --- delegations for XCharacterData
        virtual void SAL_CALL appendData(const OUString& arg)
            throw (RuntimeException, DOMException, std::exception) SAL_OVERRIDE
        {
            CCharacterData::appendData(arg);
        }
        virtual void SAL_CALL deleteData(sal_Int32 offset, sal_Int32 count)
            throw (RuntimeException, DOMException, std::exception) SAL_OVERRIDE
        {
            CCharacterData::deleteData(offset, count);
        }
        virtual OUString SAL_CALL getData() throw (RuntimeException, std::exception) SAL_OVERRIDE
        {
            return CCharacterData::getData();
        }
        virtual sal_Int32 SAL_CALL getLength() throw (RuntimeException, std::exception) SAL_OVERRIDE
        {
            return CCharacterData::getLength();
        }
        virtual void SAL_CALL insertData(sal_Int32 offset, const OUString& arg)
            throw (RuntimeException, DOMException, std::exception) SAL_OVERRIDE
        {
            CCharacterData::insertData(offset, arg);
        }
        virtual void SAL_CALL replaceData(sal_Int32 offset, sal_Int32 count, const OUString& arg)
            throw (RuntimeException, DOMException, std::exception) SAL_OVERRIDE
        {
            CCharacterData::replaceData(offset, count, arg);
        }
        virtual void SAL_CALL setData(const OUString& data)
            throw (RuntimeException, DOMException, std::exception) SAL_OVERRIDE
        {
            CCharacterData::setData(data);
        }
        virtual OUString SAL_CALL subStringData(sal_Int32 offset, sal_Int32 count)
            throw (RuntimeException, DOMException, std::exception) SAL_OVERRIDE
        {
            return CCharacterData::subStringData(offset, count);
        }


         // --- overrides for XNode base
        virtual OUString SAL_CALL getNodeName()
            throw (RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getNodeValue()
            throw (RuntimeException, std::exception) SAL_OVERRIDE;

    // --- delegation for XNde base.
    virtual Reference< XNode > SAL_CALL appendChild(const Reference< XNode >& newChild)
        throw (RuntimeException, DOMException, std::exception) SAL_OVERRIDE
    {
        return CNode::appendChild(newChild);
    }
    virtual Reference< XNode > SAL_CALL cloneNode(sal_Bool deep)
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::cloneNode(deep);
    }
    virtual Reference< XNamedNodeMap > SAL_CALL getAttributes()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::getAttributes();
    }
    virtual Reference< XNodeList > SAL_CALL getChildNodes()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::getChildNodes();
    }
    virtual Reference< XNode > SAL_CALL getFirstChild()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::getFirstChild();
    }
    virtual Reference< XNode > SAL_CALL getLastChild()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::getLastChild();
    }
    virtual OUString SAL_CALL getLocalName()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::getLocalName();
    }
    virtual OUString SAL_CALL getNamespaceURI()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::getNamespaceURI();
    }
    virtual Reference< XNode > SAL_CALL getNextSibling()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::getNextSibling();
    }
    virtual NodeType SAL_CALL getNodeType()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::getNodeType();
    }
    virtual Reference< XDocument > SAL_CALL getOwnerDocument()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::getOwnerDocument();
    }
    virtual Reference< XNode > SAL_CALL getParentNode()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::getParentNode();
    }
    virtual OUString SAL_CALL getPrefix()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::getPrefix();
    }
    virtual Reference< XNode > SAL_CALL getPreviousSibling()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::getPreviousSibling();
    }
    virtual sal_Bool SAL_CALL hasAttributes()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::hasAttributes();
    }
    virtual sal_Bool SAL_CALL hasChildNodes()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::hasChildNodes();
    }
    virtual Reference< XNode > SAL_CALL insertBefore(
            const Reference< XNode >& newChild, const Reference< XNode >& refChild)
        throw (RuntimeException, DOMException, std::exception) SAL_OVERRIDE
    {
        return CNode::insertBefore(newChild, refChild);
    }
    virtual sal_Bool SAL_CALL isSupported(const OUString& feature, const OUString& ver)
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        return CNode::isSupported(feature, ver);
    }
    virtual void SAL_CALL normalize()
        throw (RuntimeException, std::exception) SAL_OVERRIDE
    {
        CNode::normalize();
    }
    virtual Reference< XNode > SAL_CALL removeChild(const Reference< XNode >& oldChild)
        throw (RuntimeException, DOMException, std::exception) SAL_OVERRIDE
    {
        return CNode::removeChild(oldChild);
    }
    virtual Reference< XNode > SAL_CALL replaceChild(
            const Reference< XNode >& newChild, const Reference< XNode >& oldChild)
        throw (RuntimeException, DOMException, std::exception) SAL_OVERRIDE
    {
        return CNode::replaceChild(newChild, oldChild);
    }
    virtual void SAL_CALL setNodeValue(const OUString& nodeValue)
        throw (RuntimeException, DOMException, std::exception) SAL_OVERRIDE
    {
        return CText::setNodeValue(nodeValue);
    }
    virtual void SAL_CALL setPrefix(const OUString& prefix)
        throw (RuntimeException, DOMException, std::exception) SAL_OVERRIDE
    {
        return CNode::setPrefix(prefix);
    }

    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
