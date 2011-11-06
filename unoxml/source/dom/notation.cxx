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
