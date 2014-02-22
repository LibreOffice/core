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

#include <notation.hxx>

#include <string.h>

namespace DOM
{
    CNotation::CNotation(CDocument const& rDocument, ::osl::Mutex const& rMutex,
            xmlNotationPtr const pNotation)
        : CNotation_Base(rDocument, rMutex,
            NodeType_NOTATION_NODE, reinterpret_cast<xmlNodePtr>(pNotation))
        , m_aNotationPtr(pNotation)
    {
    }

    OUString SAL_CALL CNotation::getPublicId() throw (RuntimeException)
    {
        OSL_ENSURE(false,
            "CNotation::getPublicId: not implemented (#i113683#)");
        return OUString();
    }

    /**
    The system identifier of this notation.
    */
    OUString SAL_CALL CNotation::getSystemId() throw (RuntimeException)
    {
        OSL_ENSURE(false,
            "CNotation::getSystemId: not implemented (#i113683#)");
        return OUString();
    }


    OUString SAL_CALL CNotation::getNodeName()throw (RuntimeException)
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

    OUString SAL_CALL CNotation::getNodeValue() throw (RuntimeException)
    {
        return OUString();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
