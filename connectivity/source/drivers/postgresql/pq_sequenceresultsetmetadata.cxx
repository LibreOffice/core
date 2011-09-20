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
 *  Copyright: 200? by Sun Microsystems, Inc.
 *             2011 Lionel Elie Mamane <lionel@mamane.lu>
 *
 ************************************************************************/

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>

#include "pq_connection.hxx"
#include "pq_statics.hxx"

namespace pq_sdbc_driver
{
    class SequenceResultSetMetaData :
        public ::cppu::WeakImplHelper1 < com::sun::star::sdbc::XResultSetMetaData >
    {
        ::rtl::Reference< RefCountedMutex > m_refMutex;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > m_origin;
        ::rtl::OUString m_tableName;
        ::rtl::OUString m_schemaName;
        ColumnMetaDataVector m_columnData;
        sal_Int32 m_colCount;

        void checkColumnIndex( sal_Int32 columnIndex )
            throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    public:
        SequenceResultSetMetaData(
            const ::rtl::Reference< RefCountedMutex > & reMutex,
            const ColumnMetaDataVector &vec,
            int colCount );

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
#include "pq_sequenceresultsetmetadata.hxx"

#include <rtl/ustrbuf.hxx>

using rtl::OUStringBuffer;
using rtl::OUString;
using com::sun::star::uno::Any;

using com::sun::star::uno::RuntimeException;

using com::sun::star::sdbc::SQLException;

namespace pq_sdbc_driver
{

SequenceResultSetMetaData::SequenceResultSetMetaData(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const ColumnMetaDataVector &metaDataVector,
    int colCount ) :
    m_refMutex( refMutex ),
    m_columnData( metaDataVector ),
    m_colCount( colCount )
{
}


// Methods
sal_Int32 SequenceResultSetMetaData::getColumnCount(  )
    throw (SQLException, RuntimeException)
{
    return m_colCount;
}

sal_Bool SequenceResultSetMetaData::isAutoIncrement( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    checkColumnIndex( column );
    return m_columnData[column-1].isAutoIncrement;
}

sal_Bool SequenceResultSetMetaData::isCaseSensitive( sal_Int32 column )
    throw (SQLException, RuntimeException)
{

    return sal_True; // ??? hmm, numeric types or
}

sal_Bool SequenceResultSetMetaData::isSearchable( sal_Int32 column ) throw (SQLException, RuntimeException)
{
    return sal_True; // mmm, what types are not searchable ?
}

sal_Bool SequenceResultSetMetaData::isCurrency( sal_Int32 column ) throw (SQLException, RuntimeException)
{
    checkColumnIndex( column );
    return m_columnData[column-1].isCurrency;
}

sal_Int32 SequenceResultSetMetaData::isNullable( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    checkColumnIndex( column );
    return m_columnData[column-1].isNullable;
}

sal_Bool SequenceResultSetMetaData::isSigned( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    return sal_True; //
}

sal_Int32 SequenceResultSetMetaData::getColumnDisplaySize( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    return 50;
}

::rtl::OUString SequenceResultSetMetaData::getColumnLabel( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    checkColumnIndex( column );
    return m_columnData[column-1].columnName;
}

::rtl::OUString SequenceResultSetMetaData::getColumnName( sal_Int32 column ) throw (SQLException, RuntimeException)
{
    checkColumnIndex( column );
    return m_columnData[column-1].columnName;
}

::rtl::OUString SequenceResultSetMetaData::getSchemaName( sal_Int32 column ) throw (SQLException, RuntimeException)
{
    checkColumnIndex( column );
    return m_columnData[column-1].schemaTableName;
}



sal_Int32 SequenceResultSetMetaData::getPrecision( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    checkColumnIndex( column );
    return m_columnData[column-1].precision;
}

sal_Int32 SequenceResultSetMetaData::getScale( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    checkColumnIndex( column );
    return m_columnData[column-1].scale;
}

::rtl::OUString SequenceResultSetMetaData::getTableName( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    checkColumnIndex( column );
    return m_columnData[column-1].tableName;
}

::rtl::OUString SequenceResultSetMetaData::getCatalogName( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    // can do this through XConnection.getCatalog() !
    return OUString();
}
sal_Int32 SequenceResultSetMetaData::getColumnType( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    checkColumnIndex( column );
    return m_columnData[column-1].type;
}

::rtl::OUString SequenceResultSetMetaData::getColumnTypeName( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    checkColumnIndex( column );
    return m_columnData[column-1].typeName;
}


sal_Bool SequenceResultSetMetaData::isReadOnly( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    return sal_False;
}

sal_Bool SequenceResultSetMetaData::isWritable( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    return ! isReadOnly( column ); // what's the sense if this method ?
}

sal_Bool SequenceResultSetMetaData::isDefinitelyWritable( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    return isWritable(column); // uhh, now it becomes really esoteric ....
}
::rtl::OUString SequenceResultSetMetaData::getColumnServiceName( sal_Int32 column )
    throw (SQLException, RuntimeException)
{
    return OUString();
}


void SequenceResultSetMetaData::checkColumnIndex(sal_Int32 columnIndex)
    throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    if( columnIndex < 1 || columnIndex > m_colCount )
    {
        OUStringBuffer buf(128);

        buf.appendAscii( "pq_sequenceresultsetmetadata: index out of range (expected 1 to " );
        buf.append( m_colCount );
        buf.appendAscii( ", got " );
        buf.append( columnIndex );
        throw SQLException(
            buf.makeStringAndClear(), *this, OUString(), 1, Any() );
    }
}


}
