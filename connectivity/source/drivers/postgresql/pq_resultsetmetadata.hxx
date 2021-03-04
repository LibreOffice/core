/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/

#pragma once
#include <vector>

#include "pq_connection.hxx"

#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <cppuhelper/implbase.hxx>

namespace pq_sdbc_driver
{

struct ColDesc
{
    OUString name;
    sal_Int32 precision;
    sal_Int32 scale;
    sal_Int32 displaySize;
    Oid typeOid;
    OUString typeName;
    sal_Int32 type;
};

class ResultSet;

class ResultSetMetaData :
        public ::cppu::WeakImplHelper< css::sdbc::XResultSetMetaData >
{
    ::rtl::Reference< comphelper::RefCountedMutex > m_xMutex;
    ConnectionSettings **m_ppSettings;
    css::uno::Reference< css::sdbc::XResultSet > m_origin;
    css::uno::Reference< css::beans::XPropertySet > m_table;
    OUString m_tableName;
    OUString m_schemaName;
    std::vector< ColDesc > m_colDesc;
    ResultSet *m_pResultSet;

    bool m_checkedForTable;
    bool m_checkedForTypes;

    sal_Int32 m_colCount;

    /// @throws css::sdbc::SQLException
    /// @throws css::uno::RuntimeException
    void checkColumnIndex( sal_Int32 columnIndex );
    void checkTable();
    void checkForTypes();
    css::uno::Reference< css::beans::XPropertySet > getColumnByIndex( int index );

    sal_Int32 getIntColumnProperty( const OUString & name, int index, int def );
    bool getBoolColumnProperty( const OUString & name, int index, bool def );

public:
    ResultSetMetaData(
        const ::rtl::Reference< comphelper::RefCountedMutex > & reMutex,
        const css::uno::Reference< css::sdbc::XResultSet >  & origin,
        ResultSet *pResultSet,
        ConnectionSettings **pSettings,
        PGresult const *pResult,
        const OUString &schemaName,
        const OUString &tableName );

public:
    // Methods
    virtual sal_Int32 SAL_CALL getColumnCount(  ) override;
    virtual sal_Bool SAL_CALL isAutoIncrement( sal_Int32 column ) override;
    virtual sal_Bool SAL_CALL isCaseSensitive( sal_Int32 column ) override;
    virtual sal_Bool SAL_CALL isSearchable( sal_Int32 column ) override;
    virtual sal_Bool SAL_CALL isCurrency( sal_Int32 column ) override;
    virtual sal_Int32 SAL_CALL isNullable( sal_Int32 column ) override;
    virtual sal_Bool SAL_CALL isSigned( sal_Int32 column ) override;
    virtual sal_Int32 SAL_CALL getColumnDisplaySize( sal_Int32 column ) override;
    virtual OUString SAL_CALL getColumnLabel( sal_Int32 column ) override;
    virtual OUString SAL_CALL getColumnName( sal_Int32 column ) override;
    virtual OUString SAL_CALL getSchemaName( sal_Int32 column ) override;
    virtual sal_Int32 SAL_CALL getPrecision( sal_Int32 column ) override;
    virtual sal_Int32 SAL_CALL getScale( sal_Int32 column ) override;
    virtual OUString SAL_CALL getTableName( sal_Int32 column ) override;
    virtual OUString SAL_CALL getCatalogName( sal_Int32 column ) override;
    virtual sal_Int32 SAL_CALL getColumnType( sal_Int32 column ) override;
    virtual OUString SAL_CALL getColumnTypeName( sal_Int32 column ) override;
    virtual sal_Bool SAL_CALL isReadOnly( sal_Int32 column ) override;
    virtual sal_Bool SAL_CALL isWritable( sal_Int32 column ) override;
    virtual sal_Bool SAL_CALL isDefinitelyWritable( sal_Int32 column ) override;
    virtual OUString SAL_CALL getColumnServiceName( sal_Int32 column ) override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
