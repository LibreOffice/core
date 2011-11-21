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

#ifndef _PQ_STATICS_HXX_
#define _PQ_STATICS_HXX_

#include <boost/unordered_map.hpp>
#include <vector>

#include <rtl/uuid.h>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/sdbc/DataType.hpp>

#include <cppuhelper/propshlp.hxx>

#include "pq_allocator.hxx"

namespace pq_sdbc_driver
{

struct ColumnMetaData
{
    ColumnMetaData(
        const rtl::OUString &_columnName,
        const rtl::OUString &_tableName,
        const rtl::OUString &_schemaTableName,
        const rtl::OUString &_typeName,
        sal_Int32 _type,
        sal_Int32 _precision,
        sal_Int32 _scale,
        sal_Bool _isCurrency,
        sal_Bool _isNullable,
        sal_Bool _isAutoIncrement,
        sal_Bool _isReadOnly,
        sal_Bool _isSigned ) :
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

    rtl::OUString columnName;
    rtl::OUString tableName;
    rtl::OUString schemaTableName;
    rtl::OUString typeName;
    sal_Int32 type;
    sal_Int32 precision;
    sal_Int32 scale;
    sal_Bool isCurrency;
    sal_Bool isNullable;
    sal_Bool isAutoIncrement;
    sal_Bool isReadOnly;
    sal_Bool isSigned;
};

typedef std::vector< ColumnMetaData, Allocator< ColumnMetaData > > ColumnMetaDataVector;

struct TypeDetails
{
    sal_Int32 dataType;
    sal_Int32 minScale;
    sal_Int32 maxScale;  // in case nothing is given in getTypeInfo
    sal_Bool isAutoIncrement;
    sal_Bool isSearchable;
};

typedef ::boost::unordered_map
<
   rtl::OUString,
   sal_Int32,
   rtl::OUStringHash,
   ::std::equal_to< rtl::OUString >,
   Allocator< ::std::pair< const ::rtl::OUString , sal_Int32 > >
> BaseTypeMap;



struct ImplementationStatics
{
    ImplementationStatics() :
        implementationId( 16 )
    {
        rtl_createUuid( (sal_uInt8*)implementationId.getArray(), 0 , sal_False );
    }

    rtl::OUString implName;
    com::sun::star::uno::Sequence< ::rtl::OUString > serviceNames;
    com::sun::star::uno::Sequence< sal_Int8 > implementationId;
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
    ::rtl::OUString SYSTEM_TABLE;
    ::rtl::OUString TABLE;
    ::rtl::OUString VIEW;
    ::rtl::OUString UNKNOWN;
    ::rtl::OUString YES;
    ::rtl::OUString NO;
    ::rtl::OUString NO_NULLS;
    ::rtl::OUString NULABLE;
    ::rtl::OUString NULLABLE_UNKNOWN;
    ::rtl::OUString SELECT;
    ::rtl::OUString UPDATE;
    ::rtl::OUString INSERT;
    ::rtl::OUString DELETE;
    ::rtl::OUString RULE;
    ::rtl::OUString REFERENCES;
    ::rtl::OUString TRIGGER;
    ::rtl::OUString EXECUTE;
    ::rtl::OUString USAGE;
    ::rtl::OUString CREATE;
    ::rtl::OUString TEMPORARY;
    ::rtl::OUString INDEX;
    ::rtl::OUString INDEX_COLUMN;

    ::rtl::OUString NAME;
    ::rtl::OUString SCHEMA_NAME;
    ::rtl::OUString CATALOG_NAME;
    ::rtl::OUString DESCRIPTION;
    ::rtl::OUString TYPE;
    ::rtl::OUString TYPE_NAME;
    ::rtl::OUString PRIVILEGES;

    ::rtl::OUString DEFAULT_VALUE;
    ::rtl::OUString IS_AUTO_INCREMENT;
    ::rtl::OUString IS_CURRENCY;
    ::rtl::OUString IS_NULLABLE;
    ::rtl::OUString IS_ROW_VERSISON;
    ::rtl::OUString PRECISION;
    ::rtl::OUString SCALE;

    ::rtl::OUString cPERCENT;

    ::rtl::OUString BEGIN;
    ::rtl::OUString ROLLBACK;
    ::rtl::OUString COMMIT;

    ::rtl::OUString KEY;
    ::rtl::OUString REFERENCED_TABLE;
    ::rtl::OUString UPDATE_RULE;
    ::rtl::OUString DELETE_RULE;
    ::rtl::OUString PRIVATE_COLUMNS;
    ::rtl::OUString PRIVATE_FOREIGN_COLUMNS;

    ::rtl::OUString KEY_COLUMN;
    ::rtl::OUString RELATED_COLUMN;

    ::rtl::OUString PASSWORD;
    ::rtl::OUString USER;

    ::rtl::OUString CURSOR_NAME;
    ::rtl::OUString ESCAPE_PROCESSING;
    ::rtl::OUString FETCH_DIRECTION;
    ::rtl::OUString FETCH_SIZE;
    ::rtl::OUString IS_BOOKMARKABLE;
    ::rtl::OUString RESULT_SET_CONCURRENCY;
    ::rtl::OUString RESULT_SET_TYPE;

    ::rtl::OUString COMMAND;
    ::rtl::OUString CHECK_OPTION;

    ::rtl::OUString TRUE;
    ::rtl::OUString FALSE;

    ::rtl::OUString IS_PRIMARY_KEY_INDEX;
    ::rtl::OUString IS_CLUSTERED;
    ::rtl::OUString IS_UNIQUE;
    ::rtl::OUString PRIVATE_COLUMN_INDEXES;
    ::rtl::OUString HELP_TEXT;

    ::rtl::OUString CATALOG;
    ::rtl::OUString IS_ASCENDING;
    ReflectionImplementations refl;

    com::sun::star::uno::Sequence< ::rtl::OUString > tablesRowNames;
    com::sun::star::uno::Sequence< ::rtl::OUString > columnRowNames;
    com::sun::star::uno::Sequence< ::rtl::OUString > primaryKeyNames;
    com::sun::star::uno::Sequence< ::rtl::OUString > tablePrivilegesNames;
    com::sun::star::uno::Sequence< ::rtl::OUString > schemaNames;
    com::sun::star::uno::Sequence< ::rtl::OUString > tableTypeNames;
    com::sun::star::uno::Sequence< ::rtl::OUString > typeinfoColumnNames;
    com::sun::star::uno::Sequence< ::rtl::OUString > indexinfoColumnNames;
    com::sun::star::uno::Sequence< ::rtl::OUString > importedKeysColumnNames;
    com::sun::star::uno::Sequence< ::rtl::OUString > resultSetArrayColumnNames;
    com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< com::sun::star::uno::Any > > tableTypeData;

    ColumnMetaDataVector typeInfoMetaData;
    BaseTypeMap baseTypeMap;
    Statics(){}
private:
    Statics( const Statics & );
    Statics & operator = ( const Statics & );
};

Statics & getStatics();

}
#endif
