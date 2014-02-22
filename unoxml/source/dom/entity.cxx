/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <entity.hxx>

#include <string.h>


namespace DOM
{

    CEntity::CEntity(CDocument const& rDocument, ::osl::Mutex const& rMutex,
            xmlEntityPtr const pEntity)
        : CEntity_Base(rDocument, rMutex,
            NodeType_ENTITY_NODE, reinterpret_cast<xmlNodePtr>(pEntity))
        , m_aEntityPtr(pEntity)
    {
    }

    bool CEntity::IsChildTypeAllowed(NodeType const nodeType)
    {
        switch (nodeType) {
            case NodeType_ELEMENT_NODE:
            case NodeType_PROCESSING_INSTRUCTION_NODE:
            case NodeType_COMMENT_NODE:
            case NodeType_TEXT_NODE:
            case NodeType_CDATA_SECTION_NODE:
            case NodeType_ENTITY_REFERENCE_NODE:
                return true;
            default:
                return false;
        }
    }

    /**
    For unparsed entities, the name of the notation for the entity.
    */
    OUString SAL_CALL CEntity::getNotationName() throw (RuntimeException)
    {
        OSL_ENSURE(false,
                "CEntity::getNotationName: not implemented (#i113683#)");
        return OUString();
    }

    /**
    The public identifier associated with the entity, if specified.
    */
    OUString SAL_CALL CEntity::getPublicId() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

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
        ::osl::MutexGuard const g(m_rMutex);

        OUString aID;
        if(m_aEntityPtr != NULL)
        {
            aID = OUString((sal_Char*)m_aEntityPtr->SystemID, strlen((char*)m_aEntityPtr->SystemID), RTL_TEXTENCODING_UTF8);
        }
        return aID;
    }
    OUString SAL_CALL CEntity::getNodeName()throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

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
