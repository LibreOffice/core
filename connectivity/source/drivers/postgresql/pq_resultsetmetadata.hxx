/* -*- Mode: C++; eval:(c-set-style "bsd"); tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *  The Initial Developer of the Original Code is: Joerg Budischewski
 *
 *   Copyright: 2000 by Sun Microsystems, Inc.
 *              2011 Lionel Elie Mamane <lionel@mamane.lu>
 *
 *   All Rights Reserved.
 *
 *   Contributor(s): Joerg Budischewski
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
    rtl::OUString name;
    sal_Int32 precision;
    sal_Int32 scale;
    sal_Int32 displaySize;
    Oid typeOid;
    rtl::OUString typeName;
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
    ::rtl::OUString m_tableName;
    ::rtl::OUString m_schemaName;
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

    sal_Int32 getIntColumnProperty( const rtl::OUString & name, int index, int def );
    sal_Bool getBoolColumnProperty( const rtl::OUString & name, int index, sal_Bool def );

public:
    ResultSetMetaData(
        const ::rtl::Reference< RefCountedMutex > & reMutex,
        const ::com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >  & origin,
        ResultSet *pResultSet,
        ConnectionSettings **pSettings,
        PGresult *pResult,
        const rtl::OUString &schemaName,
        const rtl::OUString &tableName );

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
    virtual ::rtl::OUString SAL_CALL getColumnLabel( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getColumnName( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSchemaName( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getPrecision( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getScale( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTableName( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getCatalogName( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getColumnType( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getColumnTypeName( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isReadOnly( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isWritable( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isDefinitelyWritable( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getColumnServiceName( sal_Int32 column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
};

}

#endif
