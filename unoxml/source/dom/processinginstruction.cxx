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

#include <processinginstruction.hxx>

#include <string.h>

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>


namespace DOM
{
    CProcessingInstruction::CProcessingInstruction(
            CDocument const& rDocument, ::osl::Mutex const& rMutex,
            xmlNodePtr const pNode)
        : CProcessingInstruction_Base(rDocument, rMutex,
            NodeType_PROCESSING_INSTRUCTION_NODE, pNode)
    {
    }

    void CProcessingInstruction::saxify(
            const Reference< XDocumentHandler >& i_xHandler) {
        if (!i_xHandler.is()) throw RuntimeException();
        Reference< XExtendedDocumentHandler > xExtended(i_xHandler, UNO_QUERY);
        if (xExtended.is()) {
            xExtended->processingInstruction(getTarget(), getData());
        }
    }

    /**
    The content of this processing instruction.
    */
    OUString SAL_CALL
    CProcessingInstruction::getData() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_aNodePtr) {
            return OUString();
        }

        char const*const pContent(
                reinterpret_cast<char const*>(m_aNodePtr->content));
        if (0 == pContent) {
            return OUString();
        }
        OUString const ret(pContent, strlen(pContent), RTL_TEXTENCODING_UTF8);
        return ret;
    }

    /**
    The target of this processing instruction.
    */
    OUString SAL_CALL
    CProcessingInstruction::getTarget() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_aNodePtr) {
            return OUString();
        }

        char const*const pName(
                reinterpret_cast<char const*>(m_aNodePtr->name));
        if (0 == pName) {
            return OUString();
        }
        OUString const ret(pName, strlen(pName), RTL_TEXTENCODING_UTF8);
        return ret;
    }

    /**
    The content of this processing instruction.
    */
    void SAL_CALL CProcessingInstruction::setData(OUString const& rData)
        throw (RuntimeException, DOMException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_aNodePtr) {
            throw RuntimeException();
        }

        OString const data(
                OUStringToOString(rData, RTL_TEXTENCODING_UTF8));
        xmlChar const*const pData(
                reinterpret_cast<xmlChar const*>(data.getStr()) );
        xmlFree(m_aNodePtr->content);
        m_aNodePtr->content = xmlStrdup(pData);
    }

    OUString SAL_CALL
    CProcessingInstruction::getNodeName() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_aNodePtr) {
            return OUString();
        }

        sal_Char const*const pName =
            reinterpret_cast<sal_Char const*>(m_aNodePtr->name);
        OUString const ret(pName, strlen(pName), RTL_TEXTENCODING_UTF8);
        return ret;
    }

    OUString SAL_CALL CProcessingInstruction::getNodeValue()
        throw (RuntimeException)
    {
        return getData();
    }

    void SAL_CALL
    CProcessingInstruction::setNodeValue(OUString const& rNodeValue)
        throw (RuntimeException, DOMException)
    {
        return setData(rNodeValue);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
