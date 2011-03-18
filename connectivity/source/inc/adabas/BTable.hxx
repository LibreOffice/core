/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
