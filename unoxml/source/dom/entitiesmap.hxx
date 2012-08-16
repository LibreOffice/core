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

#ifndef DOM_ENTITIESMAP_HXX
#define DOM_ENTITIESMAP_HXX

#include <sal/types.h>
#include <rtl/ref.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>

#include <cppuhelper/implbase1.hxx>


using ::rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;

namespace DOM
{
    class CDocumentType;

    class CEntitiesMap
        : public cppu::WeakImplHelper1< XNamedNodeMap >
    {
    private:
        ::rtl::Reference<CDocumentType> const m_pDocType;

    public:
        CEntitiesMap(::rtl::Reference<CDocumentType> const& pDocType);

        /**
        The number of nodes in this map.
        */
        virtual sal_Int32 SAL_CALL getLength() throw (RuntimeException);

        /**
        Retrieves a node specified by local name
        */
        virtual Reference< XNode > SAL_CALL
            getNamedItem(const OUString& name) throw (RuntimeException);

        /**
        Retrieves a node specified by local name and namespace URI.
        */
        virtual Reference< XNode > SAL_CALL getNamedItemNS(
                OUString const& namespaceURI, OUString const& localName)
            throw (RuntimeException);

        /**
        Returns the indexth item in the map.
        */
        virtual Reference< XNode > SAL_CALL
            item(sal_Int32 index) throw (RuntimeException);

        /**
        Removes a node specified by name.
        */
        virtual Reference< XNode > SAL_CALL
            removeNamedItem(OUString const& name) throw (RuntimeException);

        /**
        // Removes a node specified by local name and namespace URI.
        */
        virtual Reference< XNode > SAL_CALL removeNamedItemNS(
                OUString const& namespaceURI, OUString const& localName)
            throw (RuntimeException);

        /**
        // Adds a node using its nodeName attribute.
        */
        virtual Reference< XNode > SAL_CALL
            setNamedItem(Reference< XNode > const& arg)
            throw (RuntimeException);

        /**
        Adds a node using its namespaceURI and localName.
        */
        virtual Reference< XNode > SAL_CALL
            setNamedItemNS(Reference< XNode > const& arg)
            throw (RuntimeException);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
