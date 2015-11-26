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
#include "pq_updateableresultset.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <string.h>

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::Type;

using com::sun::star::beans::PropertyAttribute::READONLY;
using com::sun::star::beans::PropertyAttribute::BOUND;
using com::sun::star::beans::Property;

namespace pq_sdbc_driver
{

struct DefColumnMetaData
{
    const sal_Char * columnName;
    const sal_Char * tableName;
    const sal_Char * schemaTableName;
    const sal_Char * typeName;
    sal_Int32 type;
    sal_Int32 precision;
    sal_Int32 scale;
    bool isCurrency;
    bool isNullable;
    bool isAutoIncrement;
    bool isReadOnly;
    bool isSigned;
};

struct BaseTypeDef { const char * typeName; sal_Int32 value; };

static Sequence< OUString > createStringSequence( const char * names[] )
{
    int length = 0;
    while (names[length])
        ++length;

    Sequence< OUString > seq( length );
    for( int i = 0; i < length; i ++ )
    {
        seq[i] = OUString( names[i] , strlen( names[i] ), RTL_TEXTENCODING_ASCII_US );
    }
    return seq;
}

struct PropertyDef
{
    PropertyDef( const OUString &str, const Type &t )
        : name( str ) , type( t ) {}
    OUString name;
    com::sun::star::uno::Type type;
};

struct PropertyDefEx : public PropertyDef
{
    PropertyDefEx( const OUString & str, const Type &t , sal_Int32 a )
        : PropertyDef( str, t ) , attribute( a )
    {}
    sal_Int32 attribute;
};

static cppu::IPropertyArrayHelper * createPropertyArrayHelper(
    PropertyDef *props, int count , sal_Int16 attr )
{
    Sequence< Property > seq( count );
    for( int i = 0 ; i < count ; i ++ )
    {
        seq[i] = Property( props[i].name, i, props[i].type, attr );
    }
    return new cppu::OPropertyArrayHelper( seq, sal_True );
}

static cppu::IPropertyArrayHelper * createPropertyArrayHelper(
    PropertyDefEx *props, int count )
{
    Sequence< Property > seq( count );
    for( int i = 0 ; i < count ; i ++ )
    {
        seq[i] = Property( props[i].name, i, props[i].type, props[i].attribute );
    }
    return new cppu::OPropertyArrayHelper( seq, sal_True );
}

Statics & getStatics()
{
    static Statics * p;
    if( ! p )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( ! p )
        {
            static Statics statics ;
            statics.SYSTEM_TABLE = "SYSTEM TABLE";
            statics.TABLE = "TABLE";
            statics.VIEW = "VIEW";
            statics.UNKNOWN = "UNKNOWN";
            statics.YES = "YES";
            statics.NO = "NO";
            statics.NO_NULLS = "NO_NULLS";
            statics.NULABLE = "NULABLE";
            statics.NULLABLE_UNKNOWN = "NULLABLE_UNKNOWN";
            statics.cPERCENT = "%";

            statics.TYPE = "Type";
            statics.TYPE_NAME = "TypeName";
            statics.NAME = "Name";
            statics.SCHEMA_NAME = "SchemaName";
            statics.CATALOG_NAME = "CatalogName";
            statics.DESCRIPTION = "Description";
            statics.PRIVILEGES = "Privileges";

            statics.DEFAULT_VALUE = "DefaultValue";
            statics.IS_AUTO_INCREMENT = "IsAutoIncrement";
            statics.IS_CURRENCY = "IsCurrency";
            statics.IS_NULLABLE = "IsNullable";
            statics.IS_ROW_VERSISON = "IsRowVersion";
            statics.PRECISION = "Precision";
            statics.SCALE = "Scale";

            statics.cPERCENT = "%";
            statics.BEGIN = "BEGIN";
            statics.COMMIT = "COMMIT";
            statics.ROLLBACK = "ROLLBACK";

            statics.KEY = "Key";
            statics.REFERENCED_TABLE = "ReferencedTable";
            statics.UPDATE_RULE = "UpdateRule";
            statics.DELETE_RULE = "DeleteRule";
            statics.PRIVATE_COLUMNS = "PrivateColumns";
            statics.PRIVATE_FOREIGN_COLUMNS = "PrivateForeignColumns";

            statics.KEY_COLUMN = "KeyColumn";
            statics.RELATED_COLUMN = "RelatedColumn";
            statics.PASSWORD = "Password";
            statics.USER = "User";

            statics.CURSOR_NAME = "CursorName";
            statics.ESCAPE_PROCESSING = "EscapeProcessing";
            statics.FETCH_DIRECTION = "FetchDirection";
            statics.FETCH_SIZE = "FetchSize";
            statics.IS_BOOKMARKABLE = "IsBookmarkable";
            statics.RESULT_SET_CONCURRENCY = "ResultSetConcurrency";
            statics.RESULT_SET_TYPE = "ResultSetType";

            statics.COMMAND = "Command";
            statics.CHECK_OPTION = "CheckOption";

            statics.TRUE = "t";
            statics.FALSE = "f";
            statics.IS_PRIMARY_KEY_INDEX = "IsPrimaryKeyIndex";
            statics.IS_CLUSTERED = "IsClustered";
            statics.IS_UNIQUE = "IsUnique";
            statics.IS_ASCENDING = "IsAscending";
            statics.PRIVATE_COLUMN_INDEXES = "PrivateColumnIndexes";
            statics.HELP_TEXT = "HelpText";

            statics.CATALOG = "Catalog";

            Type tString = cppu::UnoType<OUString>::get();
            Type tInt = cppu::UnoType<sal_Int32>::get();
            Type tBool = cppu::UnoType<bool>::get();
            Type tStringSequence = cppu::UnoType<com::sun::star::uno::Sequence< OUString >>::get();

            // Table props set
            ImplementationStatics &ist = statics.refl.table;
            ist.implName = "org.openoffice.comp.pq.sdbcx.Table";
            ist.serviceNames = Sequence< OUString > ( 1 );
            ist.serviceNames[0] = "com.sun.star.sdbcx.Table";
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
                tableDef, sizeof(tableDef)/sizeof(PropertyDef), READONLY );

            statics.refl.tableDescriptor.implName =
                "org.openoffice.comp.pq.sdbcx.TableDescriptor";
            statics.refl.tableDescriptor.serviceNames = Sequence< OUString > (1);
            statics.refl.tableDescriptor.serviceNames[0] =
                "com.sun.star.sdbcx.TableDescriptor";
            PropertyDef tableDescDef[] =
                {
                    PropertyDef( statics.CATALOG_NAME , tString ),
                    PropertyDef( statics.DESCRIPTION , tString ),
                    PropertyDef( statics.NAME , tString ),
                    PropertyDef( statics.PRIVILEGES , tInt ),
                    PropertyDef( statics.SCHEMA_NAME , tString )
                };
            statics.refl.tableDescriptor.pProps = createPropertyArrayHelper(
                tableDescDef, sizeof(tableDescDef)/sizeof(PropertyDef), 0 );

            // Column props set
            statics.refl.column.implName = "org.openoffice.comp.pq.sdbcx.Column";
            statics.refl.column.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.column.serviceNames[0] = "com.sun.star.sdbcx.Column";
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
                columnDef, sizeof(columnDef)/sizeof(PropertyDefEx) );

            statics.refl.columnDescriptor.implName =
                "org.openoffice.comp.pq.sdbcx.ColumnDescriptor";
            statics.refl.columnDescriptor.serviceNames = Sequence< OUString > ( 1 );
             statics.refl.columnDescriptor.serviceNames[0] =
                "com.sun.star.sdbcx.ColumnDescriptor";
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
                columnDescDef, sizeof(columnDescDef)/sizeof(PropertyDef), 0 );

            // Key properties
            statics.refl.key.implName = "org.openoffice.comp.pq.sdbcx.Key";
            statics.refl.key.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.key.serviceNames[0] = "com.sun.star.sdbcx.Key";
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
                keyDef, sizeof(keyDef)/sizeof(PropertyDef), READONLY );


            // Key properties
            statics.refl.keyDescriptor.implName =
                "org.openoffice.comp.pq.sdbcx.KeyDescriptor";
            statics.refl.keyDescriptor.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.keyDescriptor.serviceNames[0] =
                "com.sun.star.sdbcx.KeyDescriptor";
            PropertyDef keyDescDef[] =
                {
                    PropertyDef( statics.DELETE_RULE, tInt ),
                    PropertyDef( statics.NAME, tString ),
                    PropertyDef( statics.REFERENCED_TABLE, tString ),
                    PropertyDef( statics.TYPE, tInt ),
                    PropertyDef( statics.UPDATE_RULE, tInt )
                };
            statics.refl.keyDescriptor.pProps = createPropertyArrayHelper(
                keyDescDef, sizeof(keyDescDef)/sizeof(PropertyDef), 0 );



            // KeyColumn props set
            statics.refl.keycolumn.implName = "org.openoffice.comp.pq.sdbcx.KeyColumn";
            statics.refl.keycolumn.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.keycolumn.serviceNames[0] = "com.sun.star.sdbcx.KeyColumn";
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
                keycolumnDef, sizeof(keycolumnDef)/sizeof(PropertyDef), READONLY );

            // KeyColumn props set
            statics.refl.keycolumnDescriptor.implName =
                "org.openoffice.comp.pq.sdbcx.KeyColumnDescriptor";
            statics.refl.keycolumnDescriptor.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.keycolumnDescriptor.serviceNames[0] =
                "com.sun.star.sdbcx.KeyColumnDescriptor";
            PropertyDef keycolumnDescDef[] =
                {
                    PropertyDef( statics.NAME , tString ),
                    PropertyDef( statics.RELATED_COLUMN, tString )
                };
            statics.refl.keycolumnDescriptor.pProps = createPropertyArrayHelper(
                keycolumnDescDef, sizeof(keycolumnDescDef)/sizeof(PropertyDef), 0 );

            // view props set
            statics.refl.view.implName = "org.openoffice.comp.pq.sdbcx.View";
            statics.refl.view.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.view.serviceNames[0] = "com.sun.star.sdbcx.View";
            PropertyDef viewDef[] =
                {
                    PropertyDef( statics.CATALOG_NAME , tString ),
                    PropertyDef( statics.CHECK_OPTION , tInt ),
                    PropertyDef( statics.COMMAND , tString ),
                    PropertyDef( statics.NAME , tString ),
                    PropertyDef( statics.SCHEMA_NAME , tString )
                };
            statics.refl.view.pProps = createPropertyArrayHelper(
                viewDef, sizeof(viewDef)/sizeof(PropertyDef), READONLY );

            // view props set
            statics.refl.viewDescriptor.implName = "org.openoffice.comp.pq.sdbcx.ViewDescriptor";
            statics.refl.viewDescriptor.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.viewDescriptor.serviceNames[0] = "com.sun.star.sdbcx.ViewDescriptor";
            statics.refl.viewDescriptor.pProps = createPropertyArrayHelper(
                viewDef, sizeof(viewDef)/sizeof(PropertyDef), 0 ); // reuse view, as it is identical
            // user props set
            statics.refl.user.implName = "org.openoffice.comp.pq.sdbcx.User";
            statics.refl.user.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.user.serviceNames[0] = "com.sun.star.sdbcx.User";
            PropertyDef userDefRO[] =
                {
                    PropertyDef( statics.NAME , tString )
                };
            statics.refl.user.pProps = createPropertyArrayHelper(
                userDefRO, sizeof(userDefRO)/sizeof(PropertyDef), READONLY );

            // user props set
            statics.refl.userDescriptor.implName =
                "org.openoffice.comp.pq.sdbcx.UserDescriptor";
            statics.refl.userDescriptor.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.userDescriptor.serviceNames[0] =
                "com.sun.star.sdbcx.UserDescriptor";
            PropertyDef userDefWR[] =
                {
                    PropertyDef( statics.NAME , tString ),
                    PropertyDef( statics.PASSWORD , tString )
                };
            statics.refl.userDescriptor.pProps = createPropertyArrayHelper(
                userDefWR, sizeof(userDefWR)/sizeof(PropertyDef), 0 );

            // index props set
            statics.refl.index.implName = "org.openoffice.comp.pq.sdbcx.Index";
            statics.refl.index.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.index.serviceNames[0] = "com.sun.star.sdbcx.Index";
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
                indexDef, sizeof(indexDef)/sizeof(PropertyDef), READONLY );

            // index props set
            statics.refl.indexDescriptor.implName =
                "org.openoffice.comp.pq.sdbcx.IndexDescriptor";
            statics.refl.indexDescriptor.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.indexDescriptor.serviceNames[0] =
                "com.sun.star.sdbcx.IndexDescriptor";
            statics.refl.indexDescriptor.pProps = createPropertyArrayHelper(
                indexDef, sizeof(indexDef)/sizeof(PropertyDef), 0 );

            // indexColumn props set
            statics.refl.indexColumn.implName = "org.openoffice.comp.pq.sdbcx.IndexColumn";
            statics.refl.indexColumn.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.indexColumn.serviceNames[0] = "com.sun.star.sdbcx.IndexColumn";
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
                indexColumnDef, sizeof(indexColumnDef)/sizeof(PropertyDef), READONLY );

            // indexColumn props set
            statics.refl.indexColumnDescriptor.implName =
                "org.openoffice.comp.pq.sdbcx.IndexColumnDescriptor";
            statics.refl.indexColumnDescriptor.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.indexColumnDescriptor.serviceNames[0] =
                "com.sun.star.sdbcx.IndexColumnDescriptor";
            PropertyDef indexColumnDescDef[] =
                {
                    PropertyDef( statics.IS_ASCENDING, tBool ),
                    PropertyDef( statics.NAME , tString )
                };
            statics.refl.indexColumnDescriptor.pProps = createPropertyArrayHelper(
                indexColumnDescDef, sizeof(indexColumnDescDef)/sizeof(PropertyDef), 0 );

            // resultset
            statics.refl.resultSet.implName = "org.openoffice.comp.pq.ResultSet";
            statics.refl.resultSet.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.resultSet.serviceNames[0] = "com.sun.star.sdbc.ResultSet";
            statics.refl.resultSet.types = UpdateableResultSet::getStaticTypes( false /* updateable */ );
            PropertyDef resultSet[] =
                {
                    PropertyDef( statics.CURSOR_NAME , tString ),
                    PropertyDef( statics.ESCAPE_PROCESSING , tBool ),
                    PropertyDef( statics.FETCH_DIRECTION , tInt ),
                    PropertyDef( statics.FETCH_SIZE , tInt ),
                    PropertyDef( statics.IS_BOOKMARKABLE , tBool ),
                    PropertyDef( statics.RESULT_SET_CONCURRENCY , tInt ),
                    PropertyDef( statics.RESULT_SET_TYPE , tInt )
                };
            statics.refl.resultSet.pProps = createPropertyArrayHelper(
                resultSet, sizeof(resultSet)/sizeof(PropertyDef), 0 );

            // updateableResultset
            statics.refl.updateableResultSet.implName = "org.openoffice.comp.pq.UpdateableResultSet";
            statics.refl.updateableResultSet.serviceNames = Sequence< OUString > ( 1 );
            statics.refl.updateableResultSet.serviceNames[0] = "com.sun.star.sdbc.ResultSet";
            statics.refl.updateableResultSet.types = UpdateableResultSet::getStaticTypes( true /* updateable */ );
            statics.refl.updateableResultSet.pProps = createPropertyArrayHelper(
                resultSet, sizeof(resultSet)/sizeof(PropertyDef), 0 );

            // databasemetadata
            statics.tablesRowNames = Sequence< OUString > ( 5 );
            statics.tablesRowNames[TABLE_INDEX_CATALOG] = "TABLE_CAT";
            statics.tablesRowNames[TABLE_INDEX_SCHEMA] = "TABLE_SCHEM";
            statics.tablesRowNames[TABLE_INDEX_NAME] = "TABLE_NAME";
            statics.tablesRowNames[TABLE_INDEX_TYPE] = "TABLE_TYPE";
            statics.tablesRowNames[TABLE_INDEX_REMARKS] = "REMARKS";

            statics.primaryKeyNames = Sequence< OUString > ( 6 );
            statics.primaryKeyNames[0] = "TABLE_CAT";
            statics.primaryKeyNames[1] = "TABLE_SCHEM";
            statics.primaryKeyNames[2] = "TABLE_NAME";
            statics.primaryKeyNames[3] = "COLUMN_NAME";
            statics.primaryKeyNames[4] = "KEY_SEQ";
            statics.primaryKeyNames[5] = "PK_NAME";

            statics.SELECT = "SELECT";
            statics.UPDATE = "UPDATE";
            statics.INSERT = "INSERT";
            statics.DELETE = "DELETE";
            statics.RULE = "RULE";
            statics.REFERENCES = "REFERENCES";
            statics.TRIGGER = "TRIGGER";
            statics.EXECUTE = "EXECUTE";
            statics.USAGE = "USAGE";
            statics.CREATE = "CREATE";
            statics.TEMPORARY = "TEMPORARY";
            statics.INDEX = "Index";
            statics.INDEX_COLUMN = "IndexColumn";

            statics.schemaNames = Sequence< OUString > ( 1 );
            statics.schemaNames[0] = "TABLE_SCHEM";

            statics.tableTypeData = Sequence< Sequence< Any > >( 2 );

            statics.tableTypeData[0] = Sequence< Any > ( 1 );
            statics.tableTypeData[0][0] <<= statics.TABLE;

//             statics.tableTypeData[2] = Sequence< Any > ( 1 );
//             statics.tableTypeData[2][0] <<= statics.VIEW;

            statics.tableTypeData[1] = Sequence< Any > ( 1 );
            statics.tableTypeData[1][0] <<= statics.SYSTEM_TABLE;

            statics.tableTypeNames = Sequence< OUString > ( 1 );
            statics.tableTypeNames[0] = "TABLE_TYPE";

            static const char *tablePrivilegesNames[] =
                {
                    "TABLE_CAT", "TABLE_SCHEM", "TABLE_NAME", "GRANTOR", "GRANTEE", "PRIVILEGE",
                    "IS_GRANTABLE" , nullptr
                };
            statics.tablePrivilegesNames =
                createStringSequence( tablePrivilegesNames );

            static const char * columnNames[] =
            {
                "TABLE_CAT", "TABLE_SCHEM", "TABLE_NAME", "COLUMN_NAME",
                "DATA_TYPE", "TYPE_NAME", "COLUMN_SIZE", "BUFFER_LENGTH",
                "DECIMAL_DIGITS", "NUM_PREC_RADIX", "NULLABLE", "REMARKS",
                "COLUMN_DEF", "SQL_DATA_TYPE", "SQL_DATETIME_SUB", "CHAR_OCTET_LENGTH",
                "ORDINAL_POSITION", "IS_NULLABLE", nullptr
            };
            statics.columnRowNames =
                createStringSequence( columnNames );

            static const char * typeinfoColumnNames[] =
                {
                    "TYPE_NAME", "DATA_TYPE", "PRECISION", "LITERAL_PREFIX",
                    "LITERAL_SUFFIX",  "CREATE_PARAMS", "NULLABLE", "CASE_SENSITIVE",
                    "SEARCHABLE", "UNSIGNED_ATTRIBUTE", "FIXED_PREC_SCALE",
                    "AUTO_INCREMENT", "LOCAL_TYPE_NAME", "MINIMUM_SCALE",
                    "MAXIMUM_SCALE", "SQL_DATA_TYPE", "SQL_DATETIME_SUB",
                    "NUM_PREC_RADIX", nullptr
                };
            statics.typeinfoColumnNames = createStringSequence( typeinfoColumnNames );

            static const char * indexinfoColumnNames[] =
                {
                    "TABLE_CAT", "TABLE_SCHEM", "TABLE_NAME",
                    "NON_UNIQUE", "INDEX_QUALIFIER", "INDEX_NAME",
                    "TYPE", "ORDINAL_POSITION", "COLUMN_NAME",
                    "ASC_OR_DESC", "CARDINALITY", "PAGES", "FILTER_CONDITION",nullptr
                };
            statics.indexinfoColumnNames = createStringSequence( indexinfoColumnNames );

            static const char * importedKeysColumnNames[] =
                {
                    "PKTABLE_CAT" ,
                    "PKTABLE_SCHEM",
                    "PKTABLE_NAME" ,
                    "PKCOLUMN_NAME",
                    "FKTABLE_CAT" ,
                    "FKTABLE_SCHEM",
                    "FKTABLE_NAME" ,
                    "FKCOLUMN_NAME",
                    "KEY_SEQ" ,
                    "UPDATE_RULE",
                    "DELETE_RULE",
                    "FK_NAME" ,
                    "PK_NAME" ,
                    "DEFERRABILITY" ,
                    nullptr
                };
            statics.importedKeysColumnNames =
                createStringSequence( importedKeysColumnNames );

            static const char * resultSetArrayColumnNames[] = { "INDEX" , "VALUE", nullptr  };
            statics.resultSetArrayColumnNames =
                createStringSequence( resultSetArrayColumnNames );

            // LEM TODO see if a refresh is needed; obtain automatically from pg_catalog.pg_type?
            BaseTypeDef baseTypeDefs[] =
            {
                { "bool" , com::sun::star::sdbc::DataType::BOOLEAN },
                { "bytea", com::sun::star::sdbc::DataType::VARBINARY },
                { "char" , com::sun::star::sdbc::DataType::CHAR },

                { "int8" , com::sun::star::sdbc::DataType::BIGINT },
                { "serial8" , com::sun::star::sdbc::DataType::BIGINT },


                { "int2" , com::sun::star::sdbc::DataType::SMALLINT },

                { "int4" , com::sun::star::sdbc::DataType::INTEGER },
//                 { "regproc" , com::sun::star::sdbc::DataType::INTEGER },
//                 { "oid" , com::sun::star::sdbc::DataType::INTEGER },
//                 { "xid" , com::sun::star::sdbc::DataType::INTEGER },
//                 { "cid" , com::sun::star::sdbc::DataType::INTEGER },
//                 { "serial", com::sun::star::sdbc::DataType::INTEGER },
//                 { "serial4", com::sun::star::sdbc::DataType::INTEGER },

                { "text", com::sun::star::sdbc::DataType::VARCHAR },
                { "bpchar", com::sun::star::sdbc::DataType::CHAR },
                { "varchar", com::sun::star::sdbc::DataType::VARCHAR },

                { "float4", com::sun::star::sdbc::DataType::REAL },
                { "float8", com::sun::star::sdbc::DataType::DOUBLE },

                { "numeric", com::sun::star::sdbc::DataType::NUMERIC },
                { "decimal", com::sun::star::sdbc::DataType::DECIMAL },

                { "date",  com::sun::star::sdbc::DataType::DATE },
                { "time",  com::sun::star::sdbc::DataType::TIME },
                { "timestamp",  com::sun::star::sdbc::DataType::TIMESTAMP },

//                 { "_bool" , com::sun::star::sdbc::DataType::ARRAY },
//                 { "_bytea", com::sun::star::sdbc::DataType::ARRAY },
//                 { "_char" , com::sun::star::sdbc::DataType::ARRAY },

//                 { "_int8" , com::sun::star::sdbc::DataType::ARRAY },
//                 { "_serial8" , com::sun::star::sdbc::DataType::ARRAY },


//                 { "_int2" , com::sun::star::sdbc::DataType::ARRAY },

//                 { "_int4" , com::sun::star::sdbc::DataType::ARRAY },
//                 { "_regproc" , com::sun::star::sdbc::DataType::ARRAY },
//                 { "_oid" , com::sun::star::sdbc::DataType::ARRAY },
//                 { "_xid" , com::sun::star::sdbc::DataType::ARRAY },
//                 { "_cid" , com::sun::star::sdbc::DataType::ARRAY },

//                 { "_text", com::sun::star::sdbc::DataType::ARRAY },
//                 { "_bpchar", com::sun::star::sdbc::DataType::ARRAY },
//                 { "_varchar", com::sun::star::sdbc::DataType::ARRAY },

//                 { "_float4", com::sun::star::sdbc::DataType::ARRAY },
//                 { "_float8", com::sun::star::sdbc::DataType::ARRAY },

//                 { "_numeric", com::sun::star::sdbc::DataType::ARRAY },
//                 { "_decimal", com::sun::star::sdbc::DataType::ARRAY },

//                 { "_date",  com::sun::star::sdbc::DataType::ARRAY }, // switch to date later
//                 { "_time",  com::sun::star::sdbc::DataType::ARRAY }, // switch to time later

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
            // by ::com::sun::star::sdbc::XDatabaseMetaData::getTypeInfo(),
            // that is what is returned by getTypeInfo().getMetaData()
            DefColumnMetaData defTypeInfoMetaData[] =
                {
                    { "TYPE_NAME", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::VARCHAR, 0,50,false,false,false,false, false },  // 0
                    { "DATA_TYPE", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::SMALLINT, 0,50,false,false,false,false, true },  // 1
                    { "PRECISION", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::INTEGER, 0,50,false,false,false,false, true },  // 2
                    { "LITERAL_PREFIX", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::VARCHAR, 0,50,false,false,false,false, false },  // 3
                    { "LITERAL_SUFFIX", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::VARCHAR, 0,50,false,false,false,false, false },  // 4
                    { "CREATE_PARAMS", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::VARCHAR, 0,50,false,false,false,false, false },  // 5
                    { "NULLABLE", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::INTEGER, 0,50,false,false,false,false, true },  // 6
                    { "CASE_SENSITIVE", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::BOOLEAN, 0,50,false,false,false,false, false },  // 7
                    { "SEARCHABLE", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::SMALLINT, 0,50,false,false,false,false, true },  // 8
                    { "UNSIGNED_ATTRIBUTE", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::BOOLEAN, 0,50,false,false,false,false, false },  // 9
                    { "FIXED_PREC_SCALE", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::BOOLEAN, 0,50,false,false,false,false, false },  // 10
                    { "AUTO_INCREMENT", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::BOOLEAN, 0,50,false,false,false,false, false },  // 11
                    { "LOCAL_TYPE_NAME", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::VARCHAR, 0,50,false,false,false,false, false },  // 12
                    { "MINIMUM_SCALE", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::SMALLINT, 0,50,false,false,false,false,  true},  // 13
                    { "MAXIMUM_SCALE", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::SMALLINT, 0,50,false,false,false,false, true },  // 14
                    { "SQL_DATA_TYPE", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::INTEGER, 0,50,false,false,false,false, true },  // 15
                    { "SQL_DATETIME_SUB", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::INTEGER, 0,50,false,false,false,false , true},  // 16
                    { "NUM_PREC_RADIX", "TYPEINFO", "pg_catalog", "", com::sun::star::sdbc::DataType::INTEGER, 0,50,false,false,false,false, true },  // 17
                    {nullptr,nullptr,nullptr,nullptr,0,0,0,false,false,false,false, false}
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

            p = &statics;
        }
    }
    return *p;
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
