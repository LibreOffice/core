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

#include <documenttype.hxx>

#include <string.h>

#include <osl/diagnose.h>

#include <entitiesmap.hxx>
#include <notationsmap.hxx>

using namespace css::uno;
using namespace css::xml::dom;

namespace DOM
{

    CDocumentType::CDocumentType(
            CDocument const& rDocument, ::osl::Mutex const& rMutex,
            xmlDtdPtr const pDtd)
        : CDocumentType_Base(rDocument, rMutex,
            NodeType_DOCUMENT_TYPE_NODE, reinterpret_cast<xmlNodePtr>(pDtd))
        , m_aDtdPtr(pDtd)
    {
    }

    /**
    A NamedNodeMap containing the general entities, both external and
    internal, declared in the DTD.
    */
    css::uno::Reference< XNamedNodeMap > SAL_CALL CDocumentType::getEntities() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_rMutex);

        css::uno::Reference< XNamedNodeMap > aMap;
        if (m_aDtdPtr != nullptr)
        {
            aMap.set(new CEntitiesMap(this));
        }
        return aMap;
    }

    /**
    The internal subset as a string, or null if there is none.
    */
    OUString SAL_CALL CDocumentType::getInternalSubset() throw (RuntimeException, std::exception)
    {
        OSL_ENSURE(false,
            "CDocumentType::getInternalSubset: not implemented (#i113683#)");
        return OUString();
    }

    /**
    The name of DTD; i.e., the name immediately following the DOCTYPE
    keyword.
    */
    OUString SAL_CALL CDocumentType::getName() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_rMutex);

        OUString aName;
        if (m_aDtdPtr != nullptr)
        {
            aName = OUString(reinterpret_cast<char const *>(m_aDtdPtr->name), strlen(reinterpret_cast<char const *>(m_aDtdPtr->name)), RTL_TEXTENCODING_UTF8);
        }
        return aName;
    }

    /**
    A NamedNodeMap containing the notations declared in the DTD.
    */
    css::uno::Reference< XNamedNodeMap > SAL_CALL CDocumentType::getNotations() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_rMutex);

        css::uno::Reference< XNamedNodeMap > aMap;
        if (m_aDtdPtr != nullptr)
        {
            aMap.set(new CNotationsMap(this));
        }
        return aMap;
    }

    /**
    The public identifier of the external subset.
    */
    OUString SAL_CALL CDocumentType::getPublicId() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_rMutex);

        OUString aId;
        if (m_aDtdPtr != nullptr)
        {
            aId = OUString(reinterpret_cast<char const *>(m_aDtdPtr->name), strlen(reinterpret_cast<char const *>(m_aDtdPtr->ExternalID)), RTL_TEXTENCODING_UTF8);
        }
        return aId;
    }

    /**
    The system identifier of the external subset.
    */
    OUString SAL_CALL CDocumentType::getSystemId() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_rMutex);

        OUString aId;
        if (m_aDtdPtr != nullptr)
        {
            aId = OUString(reinterpret_cast<char const *>(m_aDtdPtr->name), strlen(reinterpret_cast<char const *>(m_aDtdPtr->SystemID)), RTL_TEXTENCODING_UTF8);
        }
        return aId;
    }

    OUString SAL_CALL CDocumentType::getNodeName()throw (RuntimeException, std::exception)
    {
        return getName();
    }

    OUString SAL_CALL CDocumentType::getNodeValue() throw (RuntimeException, std::exception)
    {
        return OUString();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
