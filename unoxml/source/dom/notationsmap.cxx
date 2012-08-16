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

#include <notationsmap.hxx>

#include <documenttype.hxx>


namespace DOM
{
    CNotationsMap::CNotationsMap(
            ::rtl::Reference<CDocumentType> const& pDocType)
        : m_pDocType(pDocType)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
