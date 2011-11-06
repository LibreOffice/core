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



#ifndef _PROPMAP_HXX_
#define _PROPMAP_HXX_

#include <hash_map>
#include <rtl/ustring.hxx>
#include <sal/types.h>

typedef ::std::hash_map< ::rtl::OUString                    ,
                         void*                              ,
                         ::rtl::OUStringHash                ,
                         ::std::equal_to< ::rtl::OUString > > TPropMapBase;

class TPropMap
{
    private:

        TPropMapBase m_aMap;

    public:

        template< class TValueType >
        void put(const ::rtl::OUString& sKey  ,
                 const TValueType&      rValue)
        {
            void* pValue = (void*)&rValue;
            m_aMap[sKey] = pValue;
        }

        template< class TValueType >
        void put_copy(const ::rtl::OUString& sKey  ,
                      const TValueType&      rValue)
        {
            TValueType* pCopy = new TValueType(rValue);
            m_aMap[sKey] = (void*)pCopy;
        }

        template< class TValueType >
        sal_Bool get(const ::rtl::OUString& sKey  ,
                           TValueType**     pValue)
        {
            TPropMapBase::iterator pIt = m_aMap.find(sKey);
            if (pIt == m_aMap.end())
                return sal_False;

            void*  pItem  = pIt->second;
                  *pValue = (TValueType*)pItem;
            return (pItem != 0);
        }

        template< class TValueType >
        sal_Bool get_copy(const ::rtl::OUString& sKey  ,
                                TValueType&      rValue)
        {
            TPropMapBase::iterator pIt = m_aMap.find(sKey);
            if (pIt == m_aMap.end())
                return sal_False;

            void* pValue = pIt->second;
            if ( ! pValue)
                return sal_False;

            rValue = *((TValueType*)pValue);
            //delete pValue;
            m_aMap.erase(pIt);
            return sal_True;
        }

        void clear()
        {
            m_aMap.clear();
        }
};

#endif
