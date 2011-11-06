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


#ifndef CONNECTIVITY_ADABAS_STATEMENT_HXX
#define CONNECTIVITY_ADABAS_STATEMENT_HXX

#include "odbc/OStatement.hxx"
#include "adabas/BConnection.hxx"
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/commontools.hxx"
#endif
#include <vos/ref.hxx>

namespace connectivity
{
    namespace adabas
    {
        class OAdabasStatement :    public  ::connectivity::odbc::OStatement
        {
            OAdabasConnection*          m_pOwnConnection;
            ::vos::ORef<OSQLColumns>    m_aSelectColumns;
        protected:
            virtual odbc::OResultSet* createResulSet();
            virtual void setResultSetConcurrency(sal_Int32 _par0);
            virtual void setResultSetType(sal_Int32 _par0)      ;
            virtual void setUsingBookmarks(sal_Bool _bUseBookmark);
        public:
            OAdabasStatement( OAdabasConnection* _pConnection)
                : ::connectivity::odbc::OStatement( _pConnection )
                ,m_pOwnConnection(_pConnection)
            {}

            virtual sal_Bool SAL_CALL execute( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // CONNECTIVITY_ADABAS_STATEMENT_HXX

