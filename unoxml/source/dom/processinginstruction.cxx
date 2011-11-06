/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
