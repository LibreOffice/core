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
 *    Version: MPL 1.1 / GPLv3+ / LGPLv2.1+
 *
 *    The contents of this file are subject to the Mozilla Public License Version
 *    1.1 (the "License"); you may not use this file except in compliance with
 *    the License or as specified alternatively below. You may obtain a copy of
 *    the License at http://www.mozilla.org/MPL/
 *
 *    Software distributed under the License is distributed on an "AS IS" basis,
 *    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *    for the specific language governing rights and limitations under the
 *    License.
 *
 *    Major Contributor(s):
 *    [ Copyright (C) 2011 Lionel Elie Mamane <lionel@mamane.lu> ]
 *
 *    All Rights Reserved.
 *
 *    For minor contributions see the git repository.
 *
 *    Alternatively, the contents of this file may be used under the terms of
 *    either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 *    the GNU Lesser General Public License Version 2.1 or later (the "LGPLv2.1+"),
 *    in which case the provisions of the GPLv3+ or the LGPLv2.1+ are applicable
 *    instead of those above.
 *
 ************************************************************************/

#ifndef _PQ_RESULTSETMETADATA_HXX_
#define _PQ_RESULTSETMETADATA_HXX_
#include <vector>

#include "pq_connection.hxx"

#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <cppuhelper/implbase1.hxx>

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

typedef std::vector< ColDesc, Allocator< ColDesc > > ColDescVector;


class ResultSetMetaData :
        public ::cppu::WeakImplHelper1 < com::sun::star::sdbc::XResultSetMetaData >
{
    ::rtl::Reference< RefCountedMutex > m_refMutex;
    ConnectionSettings **m_ppSettings;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > m_origin;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_table;
    OUString m_tableName;
    OUString m_schemaName;
    ColDescVector m_colDesc;
    ResultSet *m_pResultSet;

    bool m_checkedForTable;
    bool m_checkedForTypes;

    sal_Int32 m_colCount;

    void checkClosed()
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    void checkColumnIndex( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    void checkTable();
    void checkForTypes();
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > getColumnByIndex( int index );

    sal_Int32 getIntColumnProperty( const OUString & name, int index, int def );
    sal_Bool getBoolColumnProperty( const OUString & name, int index, sal_Bool def );

public:
    ResultSetMetaData(
        const ::rtl::Reference< RefCountedMutex > & reMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >  & origin,
        ResultSet *pResultSet,
        ConnectionSettings **pSettings,
        PGresult *pResult,
        const OUString &schemaName,
        const OUString &tableName );

public:
    // Methods
    virtual sal_Int32 SAL_CALL getColumnCount(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAutoIncrement( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isCaseSensitive( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isSearchable( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isCurrency( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL isNullable( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isSigned( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getColumnDisplaySize( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getColumnLabel( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getColumnName( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getSchemaName( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getPrecision( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getScale( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getTableName( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getCatalogName( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getColumnType( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getColumnTypeName( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isReadOnly( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isWritable( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isDefinitelyWritable( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getColumnServiceName( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
};

}

#endif
