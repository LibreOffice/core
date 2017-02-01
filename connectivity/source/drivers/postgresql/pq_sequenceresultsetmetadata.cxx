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
 *    Copyright: 200? by Sun Microsystems, Inc.
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

#include "pq_sequenceresultsetmetadata.hxx"

#include <rtl/ustrbuf.hxx>

using com::sun::star::uno::Any;

using com::sun::star::uno::RuntimeException;

using com::sun::star::sdbc::SQLException;

namespace pq_sdbc_driver
{

SequenceResultSetMetaData::SequenceResultSetMetaData(
    const ColumnMetaDataVector &metaDataVector,
    int colCount ) :
    m_columnData( metaDataVector ),
    m_colCount( colCount )
{
}


// Methods
sal_Int32 SequenceResultSetMetaData::getColumnCount(  )
{
    return m_colCount;
}

sal_Bool SequenceResultSetMetaData::isAutoIncrement( sal_Int32 column )
{
    checkColumnIndex( column );
    return m_columnData[column-1].isAutoIncrement;
}

sal_Bool SequenceResultSetMetaData::isCaseSensitive( sal_Int32 /* column */ )
{

    return true; // ??? hmm, numeric types or
}

sal_Bool SequenceResultSetMetaData::isSearchable( sal_Int32 /* column */ )
{
    return true; // mmm, what types are not searchable ?
}

sal_Bool SequenceResultSetMetaData::isCurrency( sal_Int32 column )
{
    checkColumnIndex( column );
    return m_columnData[column-1].isCurrency;
}

sal_Int32 SequenceResultSetMetaData::isNullable( sal_Int32 column )
{
    checkColumnIndex( column );
    return m_columnData[column-1].isNullable ? 1 : 0;
}

sal_Bool SequenceResultSetMetaData::isSigned( sal_Int32 /* column */ )
{
    return true;
}

sal_Int32 SequenceResultSetMetaData::getColumnDisplaySize( sal_Int32 /* column */ )
{
    return 50;
}

OUString SequenceResultSetMetaData::getColumnLabel( sal_Int32 column )
{
    checkColumnIndex( column );
    return m_columnData[column-1].columnName;
}

OUString SequenceResultSetMetaData::getColumnName( sal_Int32 column )
{
    checkColumnIndex( column );
    return m_columnData[column-1].columnName;
}

OUString SequenceResultSetMetaData::getSchemaName( sal_Int32 column )
{
    checkColumnIndex( column );
    return m_columnData[column-1].schemaTableName;
}


sal_Int32 SequenceResultSetMetaData::getPrecision( sal_Int32 column )
{
    checkColumnIndex( column );
    return m_columnData[column-1].precision;
}

sal_Int32 SequenceResultSetMetaData::getScale( sal_Int32 column )
{
    checkColumnIndex( column );
    return m_columnData[column-1].scale;
}

OUString SequenceResultSetMetaData::getTableName( sal_Int32 column )
{
    checkColumnIndex( column );
    return m_columnData[column-1].tableName;
}

OUString SequenceResultSetMetaData::getCatalogName( sal_Int32 /* column */ )
{
    // can do this through XConnection.getCatalog() !
    return OUString();
}
sal_Int32 SequenceResultSetMetaData::getColumnType( sal_Int32 column )
{
    checkColumnIndex( column );
    return m_columnData[column-1].type;
}

OUString SequenceResultSetMetaData::getColumnTypeName( sal_Int32 column )
{
    checkColumnIndex( column );
    return m_columnData[column-1].typeName;
}


sal_Bool SequenceResultSetMetaData::isReadOnly( sal_Int32 /* column */ )
{
    return false;
}

sal_Bool SequenceResultSetMetaData::isWritable( sal_Int32 column )
{
    return ! isReadOnly( column ); // what's the sense if this method ?
}

sal_Bool SequenceResultSetMetaData::isDefinitelyWritable( sal_Int32 column )
{
    return isWritable(column); // uhh, now it becomes really esoteric ....
}
OUString SequenceResultSetMetaData::getColumnServiceName( sal_Int32 /* column */ )
{
    return OUString();
}

void SequenceResultSetMetaData::checkColumnIndex(sal_Int32 columnIndex)
{
    if( columnIndex < 1 || columnIndex > m_colCount )
    {
        throw SQLException(
            "pq_sequenceresultsetmetadata: index out of range (expected 1 to "
            + OUString::number( m_colCount )
            + ", got " + OUString::number( columnIndex ),
            *this, OUString(), 1, Any() );
    }
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
