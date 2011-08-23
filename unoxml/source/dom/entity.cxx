/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "entity.hxx"

#include <string.h>

namespace DOM
{

    CEntity::CEntity(const xmlEntityPtr aEntityPtr)
    {
        m_aNodeType = NodeType_ENTITY_NODE;
        m_aEntityPtr = aEntityPtr;
        init_node((xmlNodePtr)aEntityPtr);
    }

    /**
    For unparsed entities, the name of the notation for the entity.
    */
    OUString SAL_CALL CEntity::getNotationName() throw (RuntimeException)
    {
        // XXX
        return OUString();
    }

    /**
    The public identifier associated with the entity, if specified.
    */
    OUString SAL_CALL CEntity::getPublicId() throw (RuntimeException)
    {
        OUString aID;
        if(m_aEntityPtr != NULL)
        {
            aID = OUString((sal_Char*)m_aEntityPtr->ExternalID, strlen((char*)m_aEntityPtr->ExternalID), RTL_TEXTENCODING_UTF8);
        }
        return aID;
    }

    /**
    The system identifier associated with the entity, if specified.
    */
    OUString SAL_CALL CEntity::getSystemId() throw (RuntimeException)
    {
        OUString aID;
        if(m_aEntityPtr != NULL)
        {
            aID = OUString((sal_Char*)m_aEntityPtr->SystemID, strlen((char*)m_aEntityPtr->SystemID), RTL_TEXTENCODING_UTF8);
        }
        return aID;
    }
    OUString SAL_CALL CEntity::getNodeName()throw (RuntimeException)
    {
       OUString aName;
        if (m_aNodePtr != NULL)
        {
            const xmlChar* xName = m_aNodePtr->name;
            aName = OUString((sal_Char*)xName, strlen((char*)xName), RTL_TEXTENCODING_UTF8);
        }
        return aName;
    }
    OUString SAL_CALL CEntity::getNodeValue() throw (RuntimeException)
    {
        return OUString();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
