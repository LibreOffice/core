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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_STATICS_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_STATICS_HXX

#include <unordered_map>
#include <vector>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/sdbc/DataType.hpp>

#include <cppuhelper/propshlp.hxx>

namespace pq_sdbc_driver
{

struct ColumnMetaData
{
    ColumnMetaData(
        const OUString &_columnName,
        const OUString &_tableName,
        const OUString &_schemaTableName,
        const OUString &_typeName,
        sal_Int32 _type,
        sal_Int32 _precision,
        sal_Int32 _scale,
        bool _isCurrency,
        bool _isNullable,
        bool _isAutoIncrement,
        bool _isReadOnly,
        bool _isSigned ) :
        columnName( _columnName ),
        tableName( _tableName ),
        schemaTableName( _schemaTableName ),
        typeName( _typeName ),
        type( _type ),
        precision( _precision ),
        scale( _scale ),
        isCurrency( _isCurrency ),
        isNullable( _isNullable ),
        isAutoIncrement( _isAutoIncrement ),
        isReadOnly( _isReadOnly ),
        isSigned( _isSigned )
    {}

    OUString columnName;
    OUString tableName;
    OUString schemaTableName;
    OUString typeName;
    sal_Int32 type;
    sal_Int32 precision;
    sal_Int32 scale;
    bool isCurrency;
    bool isNullable;
    bool isAutoIncrement;
    bool isReadOnly;
    bool isSigned;
};

typedef std::vector< ColumnMetaData > ColumnMetaDataVector;

struct TypeDetails
{
    sal_Int32 dataType;
    sal_Int32 minScale;
    sal_Int32 maxScale;  // in case nothing is given in getTypeInfo
    bool isAutoIncrement;
    bool isSearchable;
};

typedef std::unordered_map
<
   OUString,
   sal_Int32,
   OUStringHash,
   ::std::equal_to< OUString >
> BaseTypeMap;

struct ImplementationStatics
{
    ImplementationStatics() :
        pProps(nullptr)
    {}

    OUString implName;
    com::sun::star::uno::Sequence< OUString > serviceNames;
    cppu::IPropertyArrayHelper *pProps;
    com::sun::star::uno::Sequence< com::sun::star::uno::Type > types;
};

struct ReflectionImplementations
{
    struct ImplementationStatics table;
    struct ImplementationStatics tableDescriptor;
    struct ImplementationStatics column;
    struct ImplementationStatics columnDescriptor;
    struct ImplementationStatics key;
    struct ImplementationStatics keyDescriptor;
    struct ImplementationStatics keycolumn;
    struct ImplementationStatics keycolumnDescriptor;
    struct ImplementationStatics user;
    struct ImplementationStatics userDescriptor;
    struct ImplementationStatics view;
    struct ImplementationStatics viewDescriptor;
    struct ImplementationStatics index;
    struct ImplementationStatics indexDescriptor;
    struct ImplementationStatics indexColumn;
    struct ImplementationStatics indexColumnDescriptor;

    struct ImplementationStatics updateableResultSet;
    struct ImplementationStatics resultSet;
};

static const sal_Int32 TABLE_INDEX_CATALOG = 0;
static const sal_Int32 TABLE_INDEX_SCHEMA = 1;
static const sal_Int32 TABLE_INDEX_NAME = 2;
static const sal_Int32 TABLE_INDEX_TYPE = 3;
static const sal_Int32 TABLE_INDEX_REMARKS = 4;

struct Statics
{
    OUString SYSTEM_TABLE;
    OUString TABLE;
    OUString VIEW;
    OUString UNKNOWN;
    OUString YES;
    OUString NO;
    OUString NO_NULLS;
    OUString NULABLE;
    OUString NULLABLE_UNKNOWN;
    OUString SELECT;
    OUString UPDATE;
    OUString INSERT;
    OUString DELETE;
    OUString RULE;
    OUString REFERENCES;
    OUString TRIGGER;
    OUString EXECUTE;
    OUString USAGE;
    OUString CREATE;
    OUString TEMPORARY;
    OUString INDEX;
    OUString INDEX_COLUMN;

    OUString NAME;
    OUString SCHEMA_NAME;
    OUString CATALOG_NAME;
    OUString DESCRIPTION;
    OUString TYPE;
    OUString TYPE_NAME;
    OUString PRIVILEGES;

    OUString DEFAULT_VALUE;
    OUString IS_AUTO_INCREMENT;
    OUString IS_CURRENCY;
    OUString IS_NULLABLE;
    OUString IS_ROW_VERSISON;
    OUString PRECISION;
    OUString SCALE;

    OUString cPERCENT;

    OUString BEGIN;
    OUString ROLLBACK;
    OUString COMMIT;

    OUString KEY;
    OUString REFERENCED_TABLE;
    OUString UPDATE_RULE;
    OUString DELETE_RULE;
    OUString PRIVATE_COLUMNS;
    OUString PRIVATE_FOREIGN_COLUMNS;

    OUString KEY_COLUMN;
    OUString RELATED_COLUMN;

    OUString PASSWORD;
    OUString USER;

    OUString CURSOR_NAME;
    OUString ESCAPE_PROCESSING;
    OUString FETCH_DIRECTION;
    OUString FETCH_SIZE;
    OUString IS_BOOKMARKABLE;
    OUString RESULT_SET_CONCURRENCY;
    OUString RESULT_SET_TYPE;

    OUString COMMAND;
    OUString CHECK_OPTION;

    OUString TRUE;
    OUString FALSE;

    OUString IS_PRIMARY_KEY_INDEX;
    OUString IS_CLUSTERED;
    OUString IS_UNIQUE;
    OUString PRIVATE_COLUMN_INDEXES;
    OUString HELP_TEXT;

    OUString CATALOG;
    OUString IS_ASCENDING;
    ReflectionImplementations refl;

    com::sun::star::uno::Sequence< OUString > tablesRowNames;
    com::sun::star::uno::Sequence< OUString > columnRowNames;
    com::sun::star::uno::Sequence< OUString > primaryKeyNames;
    com::sun::star::uno::Sequence< OUString > tablePrivilegesNames;
    com::sun::star::uno::Sequence< OUString > schemaNames;
    com::sun::star::uno::Sequence< OUString > tableTypeNames;
    com::sun::star::uno::Sequence< OUString > typeinfoColumnNames;
    com::sun::star::uno::Sequence< OUString > indexinfoColumnNames;
    com::sun::star::uno::Sequence< OUString > importedKeysColumnNames;
    com::sun::star::uno::Sequence< OUString > resultSetArrayColumnNames;
    com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< com::sun::star::uno::Any > > tableTypeData;

    ColumnMetaDataVector typeInfoMetaData;
    BaseTypeMap baseTypeMap;
    Statics(){}
private:
    Statics( const Statics & ) = delete;
    Statics & operator = ( const Statics & ) = delete;
};

Statics & getStatics();

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
