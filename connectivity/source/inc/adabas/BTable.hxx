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



#ifndef _CONNECTIVITY_ADABAS_TABLE_HXX_
#define _CONNECTIVITY_ADABAS_TABLE_HXX_

#include "connectivity/sdbcx/VTable.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "adabas/BConnection.hxx"
#include "connectivity/TTableHelper.hxx"

namespace connectivity
{
    namespace adabas
    {
        typedef connectivity::OTableHelper OTable_TYPEDEF;

        ::rtl::OUString getTypeString(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xColProp);

        class OAdabasTable : public OTableHelper
        {
            OAdabasConnection* m_pConnection;
        protected:
            /** creates the column collection for the table
                @param  _rNames
                    The column names.
            */
            virtual sdbcx::OCollection* createColumns(const TStringVector& _rNames);

            /** creates the key collection for the table
                @param  _rNames
                    The key names.
            */
            virtual sdbcx::OCollection* createKeys(const TStringVector& _rNames);

            /** creates the index collection for the table
                @param  _rNames
                    The index names.
            */
            virtual sdbcx::OCollection* createIndexes(const TStringVector& _rNames);

        public:
            OAdabasTable(   sdbcx::OCollection* _pTables,
                            OAdabasConnection* _pConnection);
            OAdabasTable(   sdbcx::OCollection* _pTables,
                            OAdabasConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description = ::rtl::OUString(),
                    const ::rtl::OUString& _SchemaName = ::rtl::OUString(),
                    const ::rtl::OUString& _CatalogName = ::rtl::OUString()
                );

            ::rtl::OUString getTableName() const { return m_Name; }
            ::rtl::OUString getSchema() const { return m_SchemaName; }

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            // XAlterTable
            virtual void SAL_CALL alterColumnByName( const ::rtl::OUString& colName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
            // XNamed
            virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
            /**
                returns the ALTER TABLE XXX COLUMN statement
            */
            ::rtl::OUString getAlterTableColumnPart(const ::rtl::OUString& _rsColumnName );

            // starts a sql transaaction
            void beginTransAction();
            // rolls back a sql transaaction
            void rollbackTransAction();
            // ends a sql transaaction
            void endTransAction();
            // some methods to alter table structures
            void alterColumnType(const ::rtl::OUString& _rColName,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xDescriptor);
            void alterNotNullValue(sal_Int32 _nNewNullable,const ::rtl::OUString& _rColName);
            void alterDefaultValue(const ::rtl::OUString& _sNewDefault,const ::rtl::OUString& _rColName);
            void dropDefaultValue(const ::rtl::OUString& _sNewDefault);
            void addDefaultValue(const ::rtl::OUString& _sNewDefault,const ::rtl::OUString& _rColName);
        };
    }
}
#endif // _CONNECTIVITY_ADABAS_TABLE_HXX_

