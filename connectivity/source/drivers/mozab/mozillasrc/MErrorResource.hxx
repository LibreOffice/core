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



#ifndef CONNECITIVITY_MOZAB_ERROR_RESOURCE_HXX
#define CONNECITIVITY_MOZAB_ERROR_RESOURCE_HXX

#include <rtl/ustring.hxx>

namespace connectivity
{
    namespace mozab
    {
        class ErrorDescriptor
        {
        private:
            sal_uInt16      m_nErrorResourceId;
            sal_Int32       m_nErrorCondition;
            ::rtl::OUString m_sParameter;

        public:
            ErrorDescriptor()
                :m_nErrorResourceId(0)
                ,m_nErrorCondition(0)
                ,m_sParameter()
            {
            }

            inline void set( const sal_uInt16 _nErrorResourceId, const sal_Int32 _nErrorCondition, const ::rtl::OUString& _rParam )
            {
                m_nErrorResourceId = _nErrorResourceId;
                m_nErrorCondition = _nErrorCondition;
                m_sParameter = _rParam;
            }
            inline void setResId( const sal_uInt16 _nErrorResourceId )
            {
                m_nErrorResourceId = _nErrorResourceId;
            }
            inline void reset()
            {
                m_nErrorResourceId = 0;
                m_nErrorCondition = 0;
            }

            inline sal_uInt16 getResId() const                  { return m_nErrorResourceId; }
            inline sal_Int32  getErrorCondition() const         { return m_nErrorCondition; }
            inline const ::rtl::OUString& getParameter() const  { return m_sParameter; }

            inline bool is() const { return ( m_nErrorResourceId != 0 ) || ( m_nErrorCondition != 0 ); }
        };
    }
}

#endif // CONNECITIVITY_MOZAB_ERROR_RESOURCE_HXX
