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


#ifndef CONNECTIVITY_TKEYVALUE_HXX
#define CONNECTIVITY_TKEYVALUE_HXX

#include "connectivity/FValue.hxx"
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS OKeyValue
    {
    private:
        ::std::vector<ORowSetValueDecoratorRef> m_aKeys;
        sal_Int32 m_nValue;

    protected:
        OKeyValue();
        OKeyValue(sal_Int32 nVal);
    public:

        ~OKeyValue();

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t,void* _pHint ) SAL_THROW( () )
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void *,void* ) SAL_THROW( () )
            {  }

        static OKeyValue* createKeyValue(sal_Int32 nVal);
        //  static OKeyValue* createEmptyKeyValue();

        inline void pushKey(const ORowSetValueDecoratorRef& _aValueRef)
        {
            m_aKeys.push_back(_aValueRef);
        }
        inline void setValue(sal_Int32 nVal) { m_nValue = nVal; }

        ::rtl::OUString getKeyString(::std::vector<ORowSetValueDecoratorRef>::size_type i) const
        {
            OSL_ENSURE(m_aKeys.size() > i,"Wrong index for KEyValue");
            return m_aKeys[i]->getValue();
        }
        double          getKeyDouble(::std::vector<ORowSetValueDecoratorRef>::size_type i) const
        {
            OSL_ENSURE(m_aKeys.size() > i,"Wrong index for KEyValue");
            return m_aKeys[i]->getValue();
        }

        inline sal_Int32 getValue() const { return m_nValue; }
    };
}

#endif // CONNECTIVITY_TKEYVALUE_HXX

