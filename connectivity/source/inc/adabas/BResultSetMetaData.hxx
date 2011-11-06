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


#ifndef CONNECTIVITY_ADABAS_RESULTSETMETADATA_HXX
#define CONNECTIVITY_ADABAS_RESULTSETMETADATA_HXX

#include "odbc/OResultSetMetaData.hxx"
#include <vos/ref.hxx>

namespace connectivity
{
    namespace adabas
    {
        //**************************************************************
        //************ Class: ResultSetMetaData
        //**************************************************************

        typedef odbc::OResultSetMetaData OAdabasResultSetMetaData_BASE;
        class OAdabasResultSetMetaData :    public  OAdabasResultSetMetaData_BASE
        {
            ::vos::ORef<OSQLColumns>    m_aSelectColumns;
        public:
            // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
            OAdabasResultSetMetaData(odbc::OConnection* _pConnection, SQLHANDLE _pStmt ,const ::vos::ORef<OSQLColumns>& _rSelectColumns);
            virtual ~OAdabasResultSetMetaData();

            virtual sal_Int32 SAL_CALL getColumnType( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL isNullable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isAutoIncrement( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // CONNECTIVITY_ADABAS_RESULTSETMETADATA_HXX
