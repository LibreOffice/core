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

#ifndef DOM_NOTATIONSMAP_HXX
#define DOM_NOTATIONSMAP_HXX

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

    class CNotationsMap
        : public cppu::WeakImplHelper1< XNamedNodeMap >
    {
    private:
        ::rtl::Reference<CDocumentType> const m_pDocType;

    public:
        CNotationsMap(::rtl::Reference<CDocumentType> const& pDocType);

        /**
        The number of nodes in this map.
        */
        virtual sal_Int32 SAL_CALL getLength() throw (RuntimeException);

        /**
        Retrieves a node specified by local name
        */
        virtual Reference< XNode > SAL_CALL
            getNamedItem(OUString const& name) throw (RuntimeException);

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
