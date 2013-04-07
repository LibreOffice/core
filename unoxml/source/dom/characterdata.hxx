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

#ifndef DOM_CHARACTERDATA_HXX
#define DOM_CHARACTERDATA_HXX

#include <libxml/tree.h>

#include <sal/types.h>

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XCharacterData.hpp>

#include <node.hxx>


using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;

namespace DOM
{
    typedef ::cppu::ImplInheritanceHelper1< CNode, XCharacterData >
        CCharacterData_Base;

    class CCharacterData
        : public CCharacterData_Base
    {

    protected:
        CCharacterData(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                NodeType const& reNodeType, xmlNodePtr const& rpNode);

        void dispatchEvent_Impl(
                OUString const& prevValue, OUString const& newValue);

    public:
        /**
        Append the string to the end of the character data of the node.
        */
        virtual void SAL_CALL appendData(const OUString& arg)
            throw (RuntimeException, DOMException);

        /**
        Remove a range of 16-bit units from the node.
        */
        virtual void SAL_CALL deleteData(sal_Int32 offset, sal_Int32 count)
            throw (RuntimeException, DOMException);

        /**
        Return the character data of the node that implements this interface.
        */
        virtual OUString SAL_CALL getData() throw (RuntimeException);

        /**
        The number of 16-bit units that are available through data and the
        substringData method below.
        */
        virtual sal_Int32 SAL_CALL getLength() throw (RuntimeException);

        /**
        Insert a string at the specified 16-bit unit offset.
        */
        virtual void SAL_CALL insertData(sal_Int32 offset, const OUString& arg)
            throw (RuntimeException, DOMException);

        /**
        Replace the characters starting at the specified 16-bit unit offset
        with the specified string.
        */
        virtual void SAL_CALL replaceData(sal_Int32 offset, sal_Int32 count, const OUString& arg)
            throw (RuntimeException, DOMException);

        /**
        Set the character data of the node that implements this interface.
        */
        virtual void SAL_CALL setData(const OUString& data)
            throw (RuntimeException, DOMException);

        /**
        Extracts a range of data from the node.
        */
        virtual OUString SAL_CALL subStringData(sal_Int32 offset, sal_Int32 count)
            throw (RuntimeException, DOMException);

        // --- delegation for XNode base.
        virtual Reference< XNode > SAL_CALL appendChild(const Reference< XNode >& newChild)
            throw (RuntimeException, DOMException)
        {
            return CNode::appendChild(newChild);
        }
        virtual Reference< XNode > SAL_CALL cloneNode(sal_Bool deep)
            throw (RuntimeException)
        {
            return CNode::cloneNode(deep);
        }
        virtual Reference< XNamedNodeMap > SAL_CALL getAttributes()
            throw (RuntimeException)
        {
            return CNode::getAttributes();
        }
        virtual Reference< XNodeList > SAL_CALL getChildNodes()
            throw (RuntimeException)
        {
            return CNode::getChildNodes();
        }
        virtual Reference< XNode > SAL_CALL getFirstChild()
            throw (RuntimeException)
        {
            return CNode::getFirstChild();
        }
        virtual Reference< XNode > SAL_CALL getLastChild()
            throw (RuntimeException)
        {
            return CNode::getLastChild();
        }
        virtual OUString SAL_CALL getLocalName()
            throw (RuntimeException)
        {
            return CNode::getLocalName();
        }
        virtual OUString SAL_CALL getNamespaceURI()
            throw (RuntimeException)
        {
            return CNode::getNamespaceURI();
        }
        virtual Reference< XNode > SAL_CALL getNextSibling()
            throw (RuntimeException)
        {
            return CNode::getNextSibling();
        }
        virtual OUString SAL_CALL getNodeName()
            throw (RuntimeException)
        {
            return CNode::getNodeName();
        }
        virtual NodeType SAL_CALL getNodeType()
            throw (RuntimeException)
        {
            return CNode::getNodeType();
        }
        virtual OUString SAL_CALL getNodeValue()
            throw (RuntimeException)
        {
            return getData();
        }
        virtual Reference< XDocument > SAL_CALL getOwnerDocument()
            throw (RuntimeException)
        {
            return CNode::getOwnerDocument();
        }
        virtual Reference< XNode > SAL_CALL getParentNode()
            throw (RuntimeException)
        {
            return CNode::getParentNode();
        }
        virtual OUString SAL_CALL getPrefix()
            throw (RuntimeException)
        {
            return CNode::getPrefix();
        }
        virtual Reference< XNode > SAL_CALL getPreviousSibling()
            throw (RuntimeException)
        {
            return CNode::getPreviousSibling();
        }
        virtual sal_Bool SAL_CALL hasAttributes()
            throw (RuntimeException)
        {
            return CNode::hasAttributes();
        }
        virtual sal_Bool SAL_CALL hasChildNodes()
            throw (RuntimeException)
        {
            return CNode::hasChildNodes();
        }
        virtual Reference< XNode > SAL_CALL insertBefore(
                const Reference< XNode >& newChild, const Reference< XNode >& refChild)
            throw (RuntimeException, DOMException)
        {
            return CNode::insertBefore(newChild, refChild);
        }
        virtual sal_Bool SAL_CALL isSupported(const OUString& feature, const OUString& ver)
            throw (RuntimeException)
        {
            return CNode::isSupported(feature, ver);
        }
        virtual void SAL_CALL normalize()
            throw (RuntimeException)
        {
            CNode::normalize();
        }
        virtual Reference< XNode > SAL_CALL removeChild(const Reference< XNode >& oldChild)
            throw (RuntimeException, DOMException)
        {
            return CNode::removeChild(oldChild);
        }
        virtual Reference< XNode > SAL_CALL replaceChild(
                const Reference< XNode >& newChild, const Reference< XNode >& oldChild)
            throw (RuntimeException, DOMException)
        {
            return CNode::replaceChild(newChild, oldChild);
        }
        virtual void SAL_CALL setNodeValue(const OUString& nodeValue)
            throw (RuntimeException, DOMException)
        {
            return setData(nodeValue);
        }
        virtual void SAL_CALL setPrefix(const OUString& prefix)
            throw (RuntimeException, DOMException)
        {
            return CNode::setPrefix(prefix);
        }


    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
