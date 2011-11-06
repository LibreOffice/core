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



#ifndef _CONNECTIVITY_MOZAB_TABLE_HXX_
#define _CONNECTIVITY_MOZAB_TABLE_HXX_

#include "connectivity/TTableHelper.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#ifndef _CONNECTIVITY_MOZAB_BCONNECTION_HXX_
#include "MConnection.hxx"
#endif

namespace connectivity
{
    namespace mozab
    {
        typedef ::connectivity::OTableHelper OTable_Base;

        class OTable :  public OTable_Base
        {
            OConnection*    m_pConnection;

        public:
            OTable( sdbcx::OCollection* _pTables, OConnection* _pConnection);
            OTable( sdbcx::OCollection* _pTables,
                    OConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description );

            OConnection* getConnection() { return m_pConnection;}

            sal_Bool isReadOnly() const { return sal_False; }

            ::rtl::OUString getTableName() const { return m_Name; }
            ::rtl::OUString getSchema() const { return m_SchemaName; }

            // OTableHelper overridables
            virtual sdbcx::OCollection* createColumns( const TStringVector& _rNames );
            virtual sdbcx::OCollection* createKeys(const TStringVector& _rNames);
            virtual sdbcx::OCollection* createIndexes(const TStringVector& _rNames);
        private:
            using OTable_Base::getConnection;
        };
    }
}
#endif // _CONNECTIVITY_MOZAB_TABLE_HXX_

