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

#include "pq_statics.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <utility>

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::Type;

using com::sun::star::beans::PropertyAttribute::READONLY;
using com::sun::star::beans::Property;

namespace pq_sdbc_driver
{

namespace {

struct DefColumnMetaData
{
    const char * columnName;
    const char * tableName;
    const char * schemaTableName;
    const char * typeName;
    sal_Int32 type;
    sal_Int32 precision;
    sal_Int32 scale;
    bool isCurrency;
    bool isNullable;
    bool isAutoIncrement;
};

struct BaseTypeDef { const char * typeName; sal_Int32 value; };

struct PropertyDef
{
    PropertyDef( OUString str, const Type &t )
        : name(std::move( str )) , type( t ) {}
    OUString name;
    css::uno::Type type;
};

struct PropertyDefEx : public PropertyDef
{
    PropertyDefEx( const OUString & str, const Type &t , sal_Int32 a )
        : PropertyDef( str, t ) , attribute( a )
    {}
    sal_Int32 attribute;
};

}

static cppu::IPropertyArrayHelper * createPropertyArrayHelper(
    PropertyDef const *props, int count , sal_Int16 attr )
{
    Sequence< Property > seq( count );
    auto seqRange = asNonConstRange(seq);
    for( int i = 0 ; i < count ; i ++ )
    {
        seqRange[i] = Property( props[i].name, i, props[i].type, attr );
    }
    return new cppu::OPropertyArrayHelper( seq, true );
}

static cppu::IPropertyArrayHelper * createPropertyArrayHelper(
    PropertyDefEx const *props, int count )
{
    Sequence< Property > seq( count );
    auto seqRange = asNonConstRange(seq);
    for( int i = 0 ; i < count ; i ++ )
    {
        seqRange[i] = Property( props[i].name, i, props[i].type, props[i].attribute );
    }
    return new cppu::OPropertyArrayHelper( seq, true );
}

Statics & getStatics()
{
    static Statics* p = []() {
            static Statics statics ;
            statics.SYSTEM_TABLE = u"SYSTEM TABLE"_ustr;
            statics.TABLE = u"TABLE"_ustr;
            statics.VIEW = u"VIEW"_ustr;
            statics.UNKNOWN = u"UNKNOWN"_ustr;
            statics.YES = u"YES"_ustr;
            statics.NO = u"NO"_ustr;
            statics.NO_NULLS = u"NO_NULLS"_ustr;
            statics.NULABLE = u"NULABLE"_ustr;
            statics.NULLABLE_UNKNOWN = u"NULLABLE_UNKNOWN"_ustr;

            statics.TYPE = u"Type"_ustr;
            statics.TYPE_NAME = u"TypeName"_ustr;
            statics.NAME = u"Name"_ustr;
            statics.SCHEMA_NAME = u"SchemaName"_ustr;
            statics.CATALOG_NAME = u"CatalogName"_ustr;
            statics.DESCRIPTION = u"Description"_ustr;
            statics.PRIVILEGES = u"Privileges"_ustr;

            statics.DEFAULT_VALUE = u"DefaultValue"_ustr;
            statics.IS_AUTO_INCREMENT = u"IsAutoIncrement"_ustr;
            statics.IS_CURRENCY = u"IsCurrency"_ustr;
            statics.IS_NULLABLE = u"IsNullable"_ustr;
            statics.IS_ROW_VERSISON = u"IsRowVersion"_ustr;
            statics.PRECISION = u"Precision"_ustr;
            statics.SCALE = u"Scale"_ustr;

            statics.cPERCENT = u"%"_ustr;
            statics.BEGIN = u"BEGIN"_ustr;
            statics.COMMIT = u"COMMIT"_ustr;
            statics.ROLLBACK = u"ROLLBACK"_ustr;

            statics.KEY = u"Key"_ustr;
            statics.REFERENCED_TABLE = u"ReferencedTable"_ustr;
            statics.UPDATE_RULE = u"UpdateRule"_ustr;
            statics.DELETE_RULE = u"DeleteRule"_ustr;
            statics.PRIVATE_COLUMNS = u"PrivateColumns"_ustr;
            statics.PRIVATE_FOREIGN_COLUMNS = u"PrivateForeignColumns"_ustr;

            statics.KEY_COLUMN = u"KeyColumn"_ustr;
            statics.RELATED_COLUMN = u"RelatedColumn"_ustr;
            statics.PASSWORD = u"Password"_ustr;
            statics.USER = u"User"_ustr;

            statics.CURSOR_NAME = u"CursorName"_ustr;
            statics.ESCAPE_PROCESSING = u"EscapeProcessing"_ustr;
            statics.FETCH_DIRECTION = u"FetchDirection"_ustr;
            statics.FETCH_SIZE = u"FetchSize"_ustr;
            statics.IS_BOOKMARKABLE = u"IsBookmarkable"_ustr;
            statics.RESULT_SET_CONCURRENCY = u"ResultSetConcurrency"_ustr;
            statics.RESULT_SET_TYPE = u"ResultSetType"_ustr;

            statics.COMMAND = u"Command"_ustr;
            statics.CHECK_OPTION = u"CheckOption"_ustr;

            statics.TRUE = u"t"_ustr;
            statics.FALSE = u"f"_ustr;
            statics.IS_PRIMARY_KEY_INDEX = u"IsPrimaryKeyIndex"_ustr;
            statics.IS_CLUSTERED = u"IsClustered"_ustr;
            statics.IS_UNIQUE = u"IsUnique"_ustr;
            statics.IS_ASCENDING = u"IsAscending"_ustr;
            statics.PRIVATE_COLUMN_INDEXES = u"PrivateColumnIndexes"_ustr;
            statics.HELP_TEXT = u"HelpText"_ustr;

            statics.CATALOG = u"Catalog"_ustr;

            Type tString = cppu::UnoType<OUString>::get();
            Type tInt = cppu::UnoType<sal_Int32>::get();
            Type tBool = cppu::UnoType<bool>::get();
            Type tStringSequence = cppu::UnoType<css::uno::Sequence< OUString >>::get();

            // Table props set
            ImplementationStatics &ist = statics.refl.table;
            ist.implName = u"org.openoffice.comp.pq.sdbcx.Table"_ustr;
            ist.serviceNames = { u"com.sun.star.sdbcx.Table"_ustr };
            PropertyDef tableDef[] =
                {
                    PropertyDef( statics.CATALOG_NAME , tString ),
                    PropertyDef( statics.DESCRIPTION , tString ),
                    PropertyDef( statics.NAME , tString ),
                    PropertyDef( statics.PRIVILEGES , tInt ),
                    PropertyDef( statics.SCHEMA_NAME , tString ),
                    PropertyDef( statics.TYPE , tString )
                };
            ist.pProps = createPropertyArrayHelper(
                tableDef, std::size(tableDef), READONLY );

            statics.refl.tableDescriptor.implName =
                u"org.openoffice.comp.pq.sdbcx.TableDescriptor"_ustr;
            statics.refl.tableDescriptor.serviceNames = { u"com.sun.star.sdbcx.TableDescriptor"_ustr };
            PropertyDef tableDescDef[] =
                {
                    PropertyDef( statics.CATALOG_NAME , tString ),
                    PropertyDef( statics.DESCRIPTION , tString ),
                    PropertyDef( statics.NAME , tString ),
                    PropertyDef( statics.PRIVILEGES , tInt ),
                    PropertyDef( statics.SCHEMA_NAME , tString )
                };
            statics.refl.tableDescriptor.pProps = createPropertyArrayHelper(
                tableDescDef, std::size(tableDescDef), 0 );

            // Column props set
            statics.refl.column.implName = u"org.openoffice.comp.pq.sdbcx.Column"_ustr;
            statics.refl.column.serviceNames = { u"com.sun.star.sdbcx.Column"_ustr };
            PropertyDefEx columnDef[] =
                {
                    PropertyDefEx( statics.CATALOG_NAME , tString, READONLY ),
                    PropertyDefEx( statics.DEFAULT_VALUE, tString, READONLY ),
                    PropertyDefEx( statics.DESCRIPTION , tString, READONLY ),
//                     PropertyDefEx( statics.HELP_TEXT , tString, BOUND ),
                    PropertyDefEx( statics.IS_AUTO_INCREMENT, tBool, READONLY ),
                    PropertyDefEx( statics.IS_CURRENCY, tBool, READONLY ),
                    PropertyDefEx( statics.IS_NULLABLE, tInt, READONLY ),
                    PropertyDefEx( statics.IS_ROW_VERSISON, tBool,READONLY ),
                    PropertyDefEx( statics.NAME , tString,READONLY ),
                    PropertyDefEx( statics.PRECISION , tInt, READONLY ),
                    PropertyDefEx( statics.SCALE , tInt ,READONLY),
                    PropertyDefEx( statics.TYPE , tInt ,READONLY),
                    PropertyDefEx( statics.TYPE_NAME , tString ,READONLY)
                };
            statics.refl.column.pProps = createPropertyArrayHelper(
                columnDef, std::size(columnDef) );

            statics.refl.columnDescriptor.implName =
                u"org.openoffice.comp.pq.sdbcx.ColumnDescriptor"_ustr;
            statics.refl.columnDescriptor.serviceNames = { u"com.sun.star.sdbcx.ColumnDescriptor"_ustr };
            PropertyDef columnDescDef[] =
                {
                    PropertyDef( statics.CATALOG_NAME , tString ),
                    PropertyDef( statics.DEFAULT_VALUE, tString ),
                    PropertyDef( statics.DESCRIPTION , tString ),
//                     PropertyDef( statics.HELP_TEXT , tString ),
                    PropertyDef( statics.IS_AUTO_INCREMENT, tBool ),
                    PropertyDef( statics.IS_CURRENCY, tBool ),
                    PropertyDef( statics.IS_NULLABLE, tInt ),
                    PropertyDef( statics.IS_ROW_VERSISON, tBool ),
                    PropertyDef( statics.NAME , tString ),
                    PropertyDef( statics.PRECISION , tInt ),
                    PropertyDef( statics.SCALE , tInt ),
                    PropertyDef( statics.TYPE , tInt ),
                    PropertyDef( statics.TYPE_NAME , tString )
                };

            statics.refl.columnDescriptor.pProps = createPropertyArrayHelper(
                columnDescDef, std::size(columnDescDef), 0 );

            // Key properties
            statics.refl.key.implName = u"org.openoffice.comp.pq.sdbcx.Key"_ustr;
            statics.refl.key.serviceNames = { u"com.sun.star.sdbcx.Key"_ustr };
            PropertyDef keyDef[] =
                {
                    PropertyDef( statics.DELETE_RULE, tInt ),
                    PropertyDef( statics.NAME, tString ),
                    PropertyDef( statics.PRIVATE_COLUMNS, tStringSequence ),
                    PropertyDef( statics.PRIVATE_FOREIGN_COLUMNS, tStringSequence ),
                    PropertyDef( statics.REFERENCED_TABLE, tString ),
                    PropertyDef( statics.TYPE, tInt ),
                    PropertyDef( statics.UPDATE_RULE, tInt )
                };
            statics.refl.key.pProps = createPropertyArrayHelper(
                keyDef, std::size(keyDef), READONLY );


            // Key properties
            statics.refl.keyDescriptor.implName =
                u"org.openoffice.comp.pq.sdbcx.KeyDescriptor"_ustr;
            statics.refl.keyDescriptor.serviceNames = { u"com.sun.star.sdbcx.KeyDescriptor"_ustr };
            PropertyDef keyDescDef[] =
                {
                    PropertyDef( statics.DELETE_RULE, tInt ),
                    PropertyDef( statics.NAME, tString ),
                    PropertyDef( statics.REFERENCED_TABLE, tString ),
                    PropertyDef( statics.TYPE, tInt ),
                    PropertyDef( statics.UPDATE_RULE, tInt )
                };
            statics.refl.keyDescriptor.pProps = createPropertyArrayHelper(
                keyDescDef, std::size(keyDescDef), 0 );


            // KeyColumn props set
            statics.refl.keycolumn.implName = u"org.openoffice.comp.pq.sdbcx.KeyColumn"_ustr;
            statics.refl.keycolumn.serviceNames = { u"com.sun.star.sdbcx.KeyColumn"_ustr };
            PropertyDef keycolumnDef[] =
                {
                    PropertyDef( statics.CATALOG_NAME , tString ),
                    PropertyDef( statics.DEFAULT_VALUE, tString ),
                    PropertyDef( statics.DESCRIPTION , tString ),
                    PropertyDef( statics.IS_AUTO_INCREMENT, tBool ),
                    PropertyDef( statics.IS_CURRENCY, tBool ),
                    PropertyDef( statics.IS_NULLABLE, tInt ),
                    PropertyDef( statics.IS_ROW_VERSISON, tBool ),
                    PropertyDef( statics.NAME , tString ),
                    PropertyDef( statics.PRECISION , tInt ),
                    PropertyDef( statics.RELATED_COLUMN, tString ),
                    PropertyDef( statics.SCALE , tInt ),
                    PropertyDef( statics.TYPE , tInt ),
                    PropertyDef( statics.TYPE_NAME , tString )
                };
            statics.refl.keycolumn.pProps = createPropertyArrayHelper(
                keycolumnDef, std::size(keycolumnDef), READONLY );

            // KeyColumn props set
            statics.refl.keycolumnDescriptor.implName =
                u"org.openoffice.comp.pq.sdbcx.KeyColumnDescriptor"_ustr;
            statics.refl.keycolumnDescriptor.serviceNames =
                { u"com.sun.star.sdbcx.KeyColumnDescriptor"_ustr };
            PropertyDef keycolumnDescDef[] =
                {
                    PropertyDef( statics.NAME , tString ),
                    PropertyDef( statics.RELATED_COLUMN, tString )
                };
            statics.refl.keycolumnDescriptor.pProps = createPropertyArrayHelper(
                keycolumnDescDef, std::size(keycolumnDescDef), 0 );

            // view props set
            statics.refl.view.implName = u"org.openoffice.comp.pq.sdbcx.View"_ustr;
            statics.refl.view.serviceNames = { u"com.sun.star.sdbcx.View"_ustr };
            PropertyDef viewDef[] =
                {
                    PropertyDef( statics.CATALOG_NAME , tString ),
                    PropertyDef( statics.CHECK_OPTION , tInt ),
                    PropertyDef( statics.COMMAND , tString ),
                    PropertyDef( statics.NAME , tString ),
                    PropertyDef( statics.SCHEMA_NAME , tString )
                };
            statics.refl.view.pProps = createPropertyArrayHelper(
                viewDef, std::size(viewDef), READONLY );

            // view props set
            statics.refl.viewDescriptor.implName = u"org.openoffice.comp.pq.sdbcx.ViewDescriptor"_ustr;
            statics.refl.viewDescriptor.serviceNames = { u"com.sun.star.sdbcx.ViewDescriptor"_ustr };
            statics.refl.viewDescriptor.pProps = createPropertyArrayHelper(
                viewDef, std::size(viewDef), 0 ); // reuse view, as it is identical
            // user props set
            statics.refl.user.implName = u"org.openoffice.comp.pq.sdbcx.User"_ustr;
            statics.refl.user.serviceNames = { u"com.sun.star.sdbcx.User"_ustr };
            PropertyDef userDefRO[] =
                {
                    PropertyDef( statics.NAME , tString )
                };
            statics.refl.user.pProps = createPropertyArrayHelper(
                userDefRO, std::size(userDefRO), READONLY );

            // user props set
            statics.refl.userDescriptor.implName =
                u"org.openoffice.comp.pq.sdbcx.UserDescriptor"_ustr;
            statics.refl.userDescriptor.serviceNames = { u"com.sun.star.sdbcx.UserDescriptor"_ustr };
            PropertyDef userDefWR[] =
                {
                    PropertyDef( statics.NAME , tString ),
                    PropertyDef( statics.PASSWORD , tString )
                };
            statics.refl.userDescriptor.pProps = createPropertyArrayHelper(
                userDefWR, std::size(userDefWR), 0 );

            // index props set
            statics.refl.index.implName = u"org.openoffice.comp.pq.sdbcx.Index"_ustr;
            statics.refl.index.serviceNames = { u"com.sun.star.sdbcx.Index"_ustr };
            PropertyDef indexDef[] =
                {
                    PropertyDef( statics.CATALOG , tString ),
                    PropertyDef( statics.IS_CLUSTERED, tBool ),
                    PropertyDef( statics.IS_PRIMARY_KEY_INDEX, tBool ),
                    PropertyDef( statics.IS_UNIQUE, tBool ),
                    PropertyDef( statics.NAME , tString ),
                    PropertyDef( statics.PRIVATE_COLUMN_INDEXES, tStringSequence )
                };
            statics.refl.index.pProps = createPropertyArrayHelper(
                indexDef, std::size(indexDef), READONLY );

            // index props set
            statics.refl.indexDescriptor.implName =
                u"org.openoffice.comp.pq.sdbcx.IndexDescriptor"_ustr;
            statics.refl.indexDescriptor.serviceNames = { u"com.sun.star.sdbcx.IndexDescriptor"_ustr };
            statics.refl.indexDescriptor.pProps = createPropertyArrayHelper(
                indexDef, std::size(indexDef), 0 );

            // indexColumn props set
            statics.refl.indexColumn.implName = u"org.openoffice.comp.pq.sdbcx.IndexColumn"_ustr;
            statics.refl.indexColumn.serviceNames = { u"com.sun.star.sdbcx.IndexColumn"_ustr };
            PropertyDef indexColumnDef[] =
                {
                    PropertyDef( statics.CATALOG_NAME , tString ),
                    PropertyDef( statics.DEFAULT_VALUE, tString ),
                    PropertyDef( statics.DESCRIPTION , tString ),
                    PropertyDef( statics.IS_ASCENDING, tBool ),
                    PropertyDef( statics.IS_AUTO_INCREMENT, tBool ),
                    PropertyDef( statics.IS_CURRENCY, tBool ),
                    PropertyDef( statics.IS_NULLABLE, tInt ),
                    PropertyDef( statics.IS_ROW_VERSISON, tBool ),
                    PropertyDef( statics.NAME , tString ),
                    PropertyDef( statics.PRECISION , tInt ),
                    PropertyDef( statics.SCALE , tInt ),
                    PropertyDef( statics.TYPE , tInt ),
                    PropertyDef( statics.TYPE_NAME , tString )
                };
            statics.refl.indexColumn.pProps = createPropertyArrayHelper(
                indexColumnDef, std::size(indexColumnDef), READONLY );

            // indexColumn props set
            statics.refl.indexColumnDescriptor.implName =
                u"org.openoffice.comp.pq.sdbcx.IndexColumnDescriptor"_ustr;
            statics.refl.indexColumnDescriptor.serviceNames =
                { u"com.sun.star.sdbcx.IndexColumnDescriptor"_ustr };
            PropertyDef indexColumnDescDef[] =
                {
                    PropertyDef( statics.IS_ASCENDING, tBool ),
                    PropertyDef( statics.NAME , tString )
                };
            statics.refl.indexColumnDescriptor.pProps = createPropertyArrayHelper(
                indexColumnDescDef, std::size(indexColumnDescDef), 0 );

            // databasemetadata
            statics.tablesRowNames = std::vector< OUString > ( 5 );
            statics.tablesRowNames[TABLE_INDEX_CATALOG] = u"TABLE_CAT"_ustr;
            statics.tablesRowNames[TABLE_INDEX_SCHEMA] = u"TABLE_SCHEM"_ustr;
            statics.tablesRowNames[TABLE_INDEX_NAME] = u"TABLE_NAME"_ustr;
            statics.tablesRowNames[TABLE_INDEX_TYPE] = u"TABLE_TYPE"_ustr;
            statics.tablesRowNames[TABLE_INDEX_REMARKS] = u"REMARKS"_ustr;

            statics.primaryKeyNames = std::vector< OUString > ( 6 );
            statics.primaryKeyNames[0] = u"TABLE_CAT"_ustr;
            statics.primaryKeyNames[1] = u"TABLE_SCHEM"_ustr;
            statics.primaryKeyNames[2] = u"TABLE_NAME"_ustr;
            statics.primaryKeyNames[3] = u"COLUMN_NAME"_ustr;
            statics.primaryKeyNames[4] = u"KEY_SEQ"_ustr;
            statics.primaryKeyNames[5] = u"PK_NAME"_ustr;

            statics.SELECT = u"SELECT"_ustr;
            statics.UPDATE = u"UPDATE"_ustr;
            statics.INSERT = u"INSERT"_ustr;
            statics.DELETE = u"DELETE"_ustr;
            statics.RULE = u"RULE"_ustr;
            statics.REFERENCES = u"REFERENCES"_ustr;
            statics.TRIGGER = u"TRIGGER"_ustr;
            statics.EXECUTE = u"EXECUTE"_ustr;
            statics.USAGE = u"USAGE"_ustr;
            statics.CREATE = u"CREATE"_ustr;
            statics.TEMPORARY = u"TEMPORARY"_ustr;
            statics.INDEX = u"Index"_ustr;
            statics.INDEX_COLUMN = u"IndexColumn"_ustr;

            statics.schemaNames = std::vector< OUString > ( 1 );
            statics.schemaNames[0] = u"TABLE_SCHEM"_ustr;

            statics.tableTypeData = std::vector< std::vector< Any > >( 2 );

            statics.tableTypeData[0] = std::vector< Any > ( 1 );
            statics.tableTypeData[0][0] <<= statics.TABLE;

//             statics.tableTypeData[2] = Sequence< Any > ( 1 );
//             statics.tableTypeData[2][0] <<= statics.VIEW;

            statics.tableTypeData[1] = std::vector< Any > ( 1 );
            statics.tableTypeData[1][0] <<= statics.SYSTEM_TABLE;

            statics.tableTypeNames = std::vector< OUString > ( 1 );
            statics.tableTypeNames[0] = u"TABLE_TYPE"_ustr;

            statics.columnRowNames =
            {
                u"TABLE_CAT"_ustr, u"TABLE_SCHEM"_ustr, u"TABLE_NAME"_ustr, u"COLUMN_NAME"_ustr,
                u"DATA_TYPE"_ustr, u"TYPE_NAME"_ustr, u"COLUMN_SIZE"_ustr, u"BUFFER_LENGTH"_ustr,
                u"DECIMAL_DIGITS"_ustr, u"NUM_PREC_RADIX"_ustr, u"NULLABLE"_ustr, u"REMARKS"_ustr,
                u"COLUMN_DEF"_ustr, u"SQL_DATA_TYPE"_ustr, u"SQL_DATETIME_SUB"_ustr, u"CHAR_OCTET_LENGTH"_ustr,
                u"ORDINAL_POSITION"_ustr, u"IS_NULLABLE"_ustr
            };

            statics.typeinfoColumnNames =
                {
                    u"TYPE_NAME"_ustr, u"DATA_TYPE"_ustr, u"PRECISION"_ustr, u"LITERAL_PREFIX"_ustr,
                    u"LITERAL_SUFFIX"_ustr,  u"CREATE_PARAMS"_ustr, u"NULLABLE"_ustr, u"CASE_SENSITIVE"_ustr,
                    u"SEARCHABLE"_ustr, u"UNSIGNED_ATTRIBUTE"_ustr, u"FIXED_PREC_SCALE"_ustr,
                    u"AUTO_INCREMENT"_ustr, u"LOCAL_TYPE_NAME"_ustr, u"MINIMUM_SCALE"_ustr,
                    u"MAXIMUM_SCALE"_ustr, u"SQL_DATA_TYPE"_ustr, u"SQL_DATETIME_SUB"_ustr,
                    u"NUM_PREC_RADIX"_ustr
                };

            statics.indexinfoColumnNames =
                {
                    u"TABLE_CAT"_ustr, u"TABLE_SCHEM"_ustr, u"TABLE_NAME"_ustr,
                    u"NON_UNIQUE"_ustr, u"INDEX_QUALIFIER"_ustr, u"INDEX_NAME"_ustr,
                    u"TYPE"_ustr, u"ORDINAL_POSITION"_ustr, u"COLUMN_NAME"_ustr,
                    u"ASC_OR_DESC"_ustr, u"CARDINALITY"_ustr, u"PAGES"_ustr, u"FILTER_CONDITION"_ustr
                };

            statics.resultSetArrayColumnNames = { u"INDEX"_ustr , u"VALUE"_ustr  };

            // LEM TODO see if a refresh is needed; obtain automatically from pg_catalog.pg_type?
            BaseTypeDef baseTypeDefs[] =
            {
                { "bool" , css::sdbc::DataType::BOOLEAN },
                { "bytea", css::sdbc::DataType::VARBINARY },
                { "char" , css::sdbc::DataType::CHAR },

                { "int8" , css::sdbc::DataType::BIGINT },
                { "serial8" , css::sdbc::DataType::BIGINT },


                { "int2" , css::sdbc::DataType::SMALLINT },

                { "int4" , css::sdbc::DataType::INTEGER },
//                 { "regproc" , css::sdbc::DataType::INTEGER },
//                 { "oid" , css::sdbc::DataType::INTEGER },
//                 { "xid" , css::sdbc::DataType::INTEGER },
//                 { "cid" , css::sdbc::DataType::INTEGER },
//                 { "serial", css::sdbc::DataType::INTEGER },
//                 { "serial4", css::sdbc::DataType::INTEGER },

                { "text", css::sdbc::DataType::LONGVARCHAR },
                { "bpchar", css::sdbc::DataType::CHAR },
                { "varchar", css::sdbc::DataType::VARCHAR },

                { "float4", css::sdbc::DataType::REAL },
                { "float8", css::sdbc::DataType::DOUBLE },

                { "numeric", css::sdbc::DataType::NUMERIC },
                { "decimal", css::sdbc::DataType::DECIMAL },

                { "date",  css::sdbc::DataType::DATE },
                { "time",  css::sdbc::DataType::TIME },
                { "timestamp",  css::sdbc::DataType::TIMESTAMP },

//                 { "_bool" , css::sdbc::DataType::ARRAY },
//                 { "_bytea", css::sdbc::DataType::ARRAY },
//                 { "_char" , css::sdbc::DataType::ARRAY },

//                 { "_int8" , css::sdbc::DataType::ARRAY },
//                 { "_serial8" , css::sdbc::DataType::ARRAY },


//                 { "_int2" , css::sdbc::DataType::ARRAY },

//                 { "_int4" , css::sdbc::DataType::ARRAY },
//                 { "_regproc" , css::sdbc::DataType::ARRAY },
//                 { "_oid" , css::sdbc::DataType::ARRAY },
//                 { "_xid" , css::sdbc::DataType::ARRAY },
//                 { "_cid" , css::sdbc::DataType::ARRAY },

//                 { "_text", css::sdbc::DataType::ARRAY },
//                 { "_bpchar", css::sdbc::DataType::ARRAY },
//                 { "_varchar", css::sdbc::DataType::ARRAY },

//                 { "_float4", css::sdbc::DataType::ARRAY },
//                 { "_float8", css::sdbc::DataType::ARRAY },

//                 { "_numeric", css::sdbc::DataType::ARRAY },
//                 { "_decimal", css::sdbc::DataType::ARRAY },

//                 { "_date",  css::sdbc::DataType::ARRAY }, // switch to date later
//                 { "_time",  css::sdbc::DataType::ARRAY }, // switch to time later

                { nullptr, 0 }
            };
            int i;
            for( i = 0 ; baseTypeDefs[i].typeName ; i ++ )
            {
                statics.baseTypeMap[
                    OUString::createFromAscii( baseTypeDefs[i].typeName) ] =
                           baseTypeDefs[i].value;
            }

            // This is the metadata for the columns of the recordset returned
            // by css::sdbc::XDatabaseMetaData::getTypeInfo(),
            // that is what is returned by getTypeInfo().getMetaData()
            DefColumnMetaData defTypeInfoMetaData[] =
                {
                    { "TYPE_NAME", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::VARCHAR, 0,50,false,false,false },  // 0
                    { "DATA_TYPE", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::SMALLINT, 0,50,false,false,false },  // 1
                    { "PRECISION", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::INTEGER, 0,50,false,false,false },  // 2
                    { "LITERAL_PREFIX", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::VARCHAR, 0,50,false,false,false },  // 3
                    { "LITERAL_SUFFIX", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::VARCHAR, 0,50,false,false,false },  // 4
                    { "CREATE_PARAMS", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::VARCHAR, 0,50,false,false,false },  // 5
                    { "NULLABLE", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::INTEGER, 0,50,false,false,false },  // 6
                    { "CASE_SENSITIVE", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::BOOLEAN, 0,50,false,false,false },  // 7
                    { "SEARCHABLE", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::SMALLINT, 0,50,false,false,false },  // 8
                    { "UNSIGNED_ATTRIBUTE", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::BOOLEAN, 0,50,false,false,false },  // 9
                    { "FIXED_PREC_SCALE", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::BOOLEAN, 0,50,false,false,false },  // 10
                    { "AUTO_INCREMENT", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::BOOLEAN, 0,50,false,false,false },  // 11
                    { "LOCAL_TYPE_NAME", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::VARCHAR, 0,50,false,false,false },  // 12
                    { "MINIMUM_SCALE", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::SMALLINT, 0,50,false,false,false},  // 13
                    { "MAXIMUM_SCALE", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::SMALLINT, 0,50,false,false,false },  // 14
                    { "SQL_DATA_TYPE", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::INTEGER, 0,50,false,false,false },  // 15
                    { "SQL_DATETIME_SUB", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::INTEGER, 0,50,false,false,false},  // 16
                    { "NUM_PREC_RADIX", "TYPEINFO", "pg_catalog", "", css::sdbc::DataType::INTEGER, 0,50,false,false,false },  // 17
                    {nullptr,nullptr,nullptr,nullptr,0,0,0,false,false,false}
                };

            for( i = 0 ; defTypeInfoMetaData[i].columnName ; i++ )
            {
                statics.typeInfoMetaData.push_back(
                    ColumnMetaData(
                        OUString::createFromAscii( defTypeInfoMetaData[i].columnName ),
                        OUString::createFromAscii( defTypeInfoMetaData[i].tableName ),
                        OUString::createFromAscii( defTypeInfoMetaData[i].schemaTableName ),
                        OUString::createFromAscii( defTypeInfoMetaData[i].typeName ),
                        defTypeInfoMetaData[i].type,
                        defTypeInfoMetaData[i].precision,
                        defTypeInfoMetaData[i].scale,
                        defTypeInfoMetaData[i].isCurrency,
                        defTypeInfoMetaData[i].isNullable,
                        defTypeInfoMetaData[i].isAutoIncrement ) );
            }

            return &statics;
    }();
    return *p;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
