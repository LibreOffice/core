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
