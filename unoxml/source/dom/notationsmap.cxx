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

#include <notationsmap.hxx>

#include <documenttype.hxx>


namespace DOM
{
    CNotationsMap::CNotationsMap(
            ::rtl::Reference<CDocumentType> const& pDocType,
            ::osl::Mutex & rMutex)
        : m_pDocType(pDocType)
        , m_rMutex(rMutex)
    {
    }

    /**
    The number of nodes in this map.
    */
    sal_Int32 SAL_CALL CNotationsMap::getLength() throw (RuntimeException)
    {
        OSL_ENSURE(false,
            "CNotationsMap::getLength: not implemented (#i113683#)");
        return 0;
    }

    /**
    Retrieves a node specified by local name
    */
    Reference< XNode > SAL_CALL
    CNotationsMap::getNamedItem(OUString const& /*name*/)
    throw (RuntimeException)
    {
        OSL_ENSURE(false,
            "CNotationsMap::getNamedItem: not implemented (#i113683#)");
        return Reference< XNode >();
    }

    /**
    Retrieves a node specified by local name and namespace URI.
    */
    Reference< XNode > SAL_CALL
    CNotationsMap::getNamedItemNS(
        OUString const& /*namespaceURI*/, OUString const& /*localName*/)
    throw (RuntimeException)
    {
        OSL_ENSURE(false,
            "CNotationsMap::getNamedItemNS: not implemented (#i113683#)");
        return Reference< XNode >();
    }

    /**
    Returns the indexth item in the map.
    */
    Reference< XNode > SAL_CALL
    CNotationsMap::item(sal_Int32 /*index*/) throw (RuntimeException)
    {
        OSL_ENSURE(false, "CNotationsMap::item: not implemented (#i113683#)");
        return Reference< XNode >();
    }

    /**
    Removes a node specified by name.
    */
    Reference< XNode > SAL_CALL
    CNotationsMap::removeNamedItem(OUString const& /*name*/)
    throw (RuntimeException)
    {
        OSL_ENSURE(false,
            "CNotationsMap::removeNamedItem: not implemented (#i113683#)");
        return Reference< XNode >();
    }

    /**
    // Removes a node specified by local name and namespace URI.
    */
    Reference< XNode > SAL_CALL
    CNotationsMap::removeNamedItemNS(
        OUString const& /*namespaceURI*/, OUString const& /*localName*/)
    throw (RuntimeException)
    {
        OSL_ENSURE(false,
            "CNotationsMap::removeNamedItemNS: not implemented (#i113683#)");
        return Reference< XNode >();
    }

    /**
    // Adds a node using its nodeName attribute.
    */
    Reference< XNode > SAL_CALL
    CNotationsMap::setNamedItem(Reference< XNode > const& /*arg*/)
    throw (RuntimeException)
    {
        OSL_ENSURE(false,
            "CNotationsMap::setNamedItem: not implemented (#i113683#)");
        return Reference< XNode >();
    }

    /**
    Adds a node using its namespaceURI and localName.
    */
    Reference< XNode > SAL_CALL
    CNotationsMap::setNamedItemNS(Reference< XNode > const& /*arg*/)
    throw (RuntimeException)
    {
        OSL_ENSURE(false,
            "CNotationsMap::setNamedItemNS: not implemented (#i113683#)");
        return Reference< XNode >();
    }
}
