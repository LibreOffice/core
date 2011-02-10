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
            return ::rtl::OUString();
        }

        char const*const pContent(
                reinterpret_cast<char const*>(m_aNodePtr->content));
        if (0 == pContent) {
            return ::rtl::OUString();
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
            return ::rtl::OUString();
        }

        char const*const pName(
                reinterpret_cast<char const*>(m_aNodePtr->name));
        if (0 == pName) {
            return ::rtl::OUString();
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
                ::rtl::OUStringToOString(rData, RTL_TEXTENCODING_UTF8));
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
            return ::rtl::OUString();
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
