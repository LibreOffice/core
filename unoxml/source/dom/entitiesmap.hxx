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

#ifndef INCLUDED_UNOXML_SOURCE_DOM_ENTITIESMAP_HXX
#define INCLUDED_UNOXML_SOURCE_DOM_ENTITIESMAP_HXX

#include <sal/types.h>
#include <rtl/ref.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>

#include <cppuhelper/implbase.hxx>

namespace DOM
{
    class CDocumentType;

    class CEntitiesMap
        : public cppu::WeakImplHelper< css::xml::dom::XNamedNodeMap >
    {
    private:
        ::rtl::Reference<CDocumentType> const m_pDocType;

    public:
        CEntitiesMap(::rtl::Reference<CDocumentType> const& pDocType);

        /**
        The number of nodes in this map.
        */
        virtual sal_Int32 SAL_CALL getLength() throw (css::uno::RuntimeException, std::exception) override;

        /**
        Retrieves a node specified by local name
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL
            getNamedItem(const OUString& name) throw (css::uno::RuntimeException, std::exception) override;

        /**
        Retrieves a node specified by local name and namespace URI.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getNamedItemNS(
                OUString const& namespaceURI, OUString const& localName)
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Returns the indexth item in the map.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL
            item(sal_Int32 index) throw (css::uno::RuntimeException, std::exception) override;

        /**
        Removes a node specified by name.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL
            removeNamedItem(OUString const& name) throw (css::uno::RuntimeException, std::exception) override;

        /**
        // Removes a node specified by local name and namespace URI.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL removeNamedItemNS(
                OUString const& namespaceURI, OUString const& localName)
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        // Adds a node using its nodeName attribute.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL
            setNamedItem(css::uno::Reference< css::xml::dom::XNode > const& arg)
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Adds a node using its namespaceURI and localName.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL
            setNamedItemNS(css::uno::Reference< css::xml::dom::XNode > const& arg)
            throw (css::uno::RuntimeException, std::exception) override;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
