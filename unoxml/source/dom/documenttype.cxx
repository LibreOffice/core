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



#include <documenttype.hxx>

#include <string.h>

#include <entitiesmap.hxx>
#include <notationsmap.hxx>


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
    Reference< XNamedNodeMap > SAL_CALL CDocumentType::getEntities() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNamedNodeMap > aMap;
        if (m_aDtdPtr != NULL)
        {
            aMap.set(new CEntitiesMap(this, m_rMutex));
        }
        return aMap;
    }

    /**
    The internal subset as a string, or null if there is none.
    */
    OUString SAL_CALL CDocumentType::getInternalSubset() throw (RuntimeException)
    {
        OSL_ENSURE(false,
            "CDocumentType::getInternalSubset: not implemented (#i113683#)");
        return OUString();
    }

    /**
    The name of DTD; i.e., the name immediately following the DOCTYPE
    keyword.
    */
    OUString SAL_CALL CDocumentType::getName() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        OUString aName;
        if (m_aDtdPtr != NULL)
        {
            aName = OUString((sal_Char*)m_aDtdPtr->name, strlen((char*)m_aDtdPtr->name), RTL_TEXTENCODING_UTF8);
        }
        return aName;
    }

    /**
    A NamedNodeMap containing the notations declared in the DTD.
    */
    Reference< XNamedNodeMap > SAL_CALL CDocumentType::getNotations() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNamedNodeMap > aMap;
        if (m_aDtdPtr != NULL)
        {
            aMap.set(new CNotationsMap(this, m_rMutex));
        }
        return aMap;
    }

    /**
    The public identifier of the external subset.
    */
    OUString SAL_CALL CDocumentType::getPublicId() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        OUString aId;
        if (m_aDtdPtr != NULL)
        {
            aId = OUString((sal_Char*)m_aDtdPtr->name, strlen((char*)m_aDtdPtr->ExternalID), RTL_TEXTENCODING_UTF8);
        }
        return aId;
    }

    /**
    The system identifier of the external subset.
    */
    OUString SAL_CALL CDocumentType::getSystemId() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        OUString aId;
        if (m_aDtdPtr != NULL)
        {
            aId = OUString((sal_Char*)m_aDtdPtr->name, strlen((char*)m_aDtdPtr->SystemID), RTL_TEXTENCODING_UTF8);
        }
        return aId;
    }

    OUString SAL_CALL CDocumentType::getNodeName()throw (RuntimeException)
    {
        return getName();
    }

    OUString SAL_CALL CDocumentType::getNodeValue() throw (RuntimeException)
    {
        return OUString();
    }
}
