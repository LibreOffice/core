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



#include <nodelist.hxx>

#include "../dom/document.hxx"

namespace XPath
{
    CNodeList::CNodeList(
                ::rtl::Reference<DOM::CDocument> const& pDocument,
                ::osl::Mutex & rMutex,
                boost::shared_ptr<xmlXPathObject> const& rxpathObj)
        : m_pDocument(pDocument)
        , m_rMutex(rMutex)
        , m_pNodeSet(0)
    {
        if( bool(rxpathObj) && rxpathObj->type == XPATH_NODESET)
        {
            m_pNodeSet = rxpathObj->nodesetval;
            m_pXPathObj = rxpathObj;
        }
    }

    /**
    The number of nodes in the list.
    */
    sal_Int32 SAL_CALL CNodeList::getLength() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        sal_Int32 value = 0;
        if (m_pNodeSet != NULL)
            value = xmlXPathNodeSetGetLength(m_pNodeSet);
        return value;
    }

    /**
    Returns the indexth item in the collection.
    */
    Reference< XNode > SAL_CALL CNodeList::item(sal_Int32 index)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_pNodeSet) {
            return 0;
        }
        xmlNodePtr const pNode = xmlXPathNodeSetItem(m_pNodeSet, index);
        Reference< XNode > const xNode(m_pDocument->GetCNode(pNode).get());
        return xNode;
    }
}

