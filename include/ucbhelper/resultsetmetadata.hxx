/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_UCBHELPER_RESULTSETMETADATA_HXX
#define INCLUDED_UCBHELPER_RESULTSETMETADATA_HXX

#include <vector>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/ucbhelperdllapi.h>

namespace com { namespace sun { namespace star {
    namespace lang  { class XMultiServiceFactory; }
    namespace beans { struct Property; }
} } }

namespace ucbhelper_impl {
    struct ResultSetMetaData_Impl;
}

namespace ucbhelper
{



/**
 * This is a structure that holds additional meta data for one column
 * of a resultset. The default values set in the constructor should be a
 * good guess for many UCB use cases.
 */
struct ResultSetColumnData
{
    /** @see ResultSetMetaData::isAutoIncrement */
    bool        isAutoIncrement;

    /** @see ResultSetMetaData::isCaseSensitive */
    bool        isCaseSensitive;

    /** @see ResultSetMetaData::isSearchable */
    bool        isSearchable;

    /** @see ResultSetMetaData::isCurrency */
    bool        isCurrency;

    /** @see ResultSetMetaData::isNullable */
    sal_Int32       isNullable;

    /** @see ResultSetMetaData::isSigned */
    bool        isSigned;

    /** @see ResultSetMetaData::getColumnDisplaySize */
    sal_Int32       columnDisplaySize;

    /** @see ResultSetMetaData::getColumnLabel */
    OUString columnLabel;

    /** @see ResultSetMetaData::getSchemaName */
    OUString schemaName;

    /** @see ResultSetMetaData::getPrecision */
    sal_Int32       precision;

    /** @see ResultSetMetaData::getScale */
    sal_Int32       scale;

    /** @see ResultSetMetaData::getTableName */
    OUString tableName;

    /** @see ResultSetMetaData::getCatalogName */
    OUString catalogName;

    /** @see ResultSetMetaData::getColumnTypeName */
    OUString columnTypeName;

    /** @see ResultSetMetaData::isReadOnly */
    bool        isReadOnly;

    /** @see ResultSetMetaData::isWritable */
    bool        isWritable;

    /** @see ResultSetMetaData::isDefinitelyWritable */
    bool        isDefinitelyWritable;

    /** @see ResultSetMetaData::getColumnServiceName */
    OUString columnServiceName;

    inline ResultSetColumnData();
};

// Note: Never change the initial values! Implementations using this struct
//       may heavily depend on the behaviour of the default constructor.

ResultSetColumnData::ResultSetColumnData()
: isAutoIncrement( false ),
  isCaseSensitive( true ),
  isSearchable( false ),
  isCurrency( false ),
  isNullable( css::sdbc::ColumnValue::NULLABLE ),
  isSigned( false ),
  columnDisplaySize( 16 ),
  precision( -1 ),
  scale( 0 ),
  isReadOnly( true ),
  isWritable( false ),
  isDefinitelyWritable( false )
{
}



/**
 * This is an implementation of the interface XResultSetMetaData. It can be
 * used to implement the interface
 * css::sdbc::XResultSetMetaDataSupplier, which is required for
 * implementations of service com.sun.star.ucb.ContentResultSet.
 */
class UCBHELPER_DLLPUBLIC ResultSetMetaData :
                public ::cppu::OWeakObject,
                public css::lang::XTypeProvider,
                public css::sdbc::XResultSetMetaData
{
private:
    ucbhelper_impl::ResultSetMetaData_Impl* m_pImpl;

protected:
    css::uno::Reference< css::uno::XComponentContext >    m_xContext;
    css::uno::Sequence< css::beans::Property >            m_aProps;
    bool m_bReadOnly;

public:

    /**
      * Constructor.
      *
      * @param rxSMgr is a Servive Manager.
      * @param rProps is a sequence of properties (partially) describing the
      *        columns of a resultset.
      * @param bReadOnly is used to specify whether the whole(!) resultset
      *        is read-only.
      */
    ResultSetMetaData(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            const css::uno::Sequence< css::beans::Property >& rProps,
            bool bReadOnly = true );

    /**
      * Constructor.
      *
      * @param rxSMgr is a Servive Manager.
      * @param rProps is a sequence of properties (partially) describing the
      *        columns of a resultset.
      * @param rColumnData contains additional meta data for the columns of
      *        a resultset, which override the default values returned by the
      *        appropriate methods of this class. The length of rColumnData
      *        must be the same as length of rProps.
      *        rColumnData[ 0 ] corresponds to data in rProps[ 0 ],
      *        rColumnData[ 1 ] corresponds to data in rProps[ 1 ], ...
      */
    ResultSetMetaData(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            const css::uno::Sequence< css::beans::Property >& rProps,
            const std::vector< ResultSetColumnData >& rColumnData );

    /**
      * Destructor.
      */
    virtual ~ResultSetMetaData();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XResultSetMetaData

    /**
      * Returns the number of columns of the resultset.
      *
      * @return the length of the property sequence.
      */
    virtual sal_Int32 SAL_CALL
    getColumnCount()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Checks whether column is automatically numbered, which makes it
      * read-only.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if column is automatically numbered.
      */
    virtual sal_Bool SAL_CALL
    isAutoIncrement( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Checks whether column is case sensitive.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if column is case sensitive.
      */
    virtual sal_Bool SAL_CALL
    isCaseSensitive( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Checks whether the value stored in column can be used in a
      * WHERE clause.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if the column is searchable.
      */
    virtual sal_Bool SAL_CALL
    isSearchable( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Checks whether column is a cash value.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if the column is a cash value.
      */
    virtual sal_Bool SAL_CALL
    isCurrency( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Checks whether a NULL can be stored in column.
      *
      * @see css::sdbc::ColumnValue
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return css::sdbc::ColumnValue::NULLABLE, if a NULL
      *         can be stored in the column.
      */
    virtual sal_Int32 SAL_CALL
    isNullable( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Checks whether the value stored in column is a signed number.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if the value stored in column is a signed number.
      */
    virtual sal_Bool SAL_CALL
    isSigned( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Gets the normal maximum width in characters for column.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the normal maximum width in characters for column.
      */
    virtual sal_Int32 SAL_CALL
    getColumnDisplaySize( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Gets the suggested column title for column, to be used in print-
      * outs and displays.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the column label.
      */
    virtual OUString SAL_CALL
    getColumnLabel( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Gets the name of column.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the name of the property that corresponds to column.
      */
    virtual OUString SAL_CALL
    getColumnName( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Gets the schema name for the table from which column of this
      * result set was derived.
      * Because this feature is not widely supported, the return value
      * for many DBMSs will be an empty string.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the schema name of column or an empty string.
      */
    virtual OUString SAL_CALL
    getSchemaName( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * For number types, getprecision gets the number of decimal digits
      * in column.
      * For character types, it gets the maximum length in characters for
      * column.
      * For binary types, it gets the maximum length in bytes for column.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the precision for the column.
      */
    virtual sal_Int32 SAL_CALL
    getPrecision( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Gets the number of digits to the right of the decimal point for
      * values in column.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the scale of the column.
      */
    virtual sal_Int32 SAL_CALL
    getScale( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Gets the name of the table from which column of this result set
      * was derived or "" if there is none (for example, for a join).
      * Because this feature is not widely supported, the return value
      * for many DBMSs will be an empty string.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the table name for column or an empty string.
      */
    virtual OUString SAL_CALL
    getTableName( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL
    /**
      * Gets the catalog name for the table from which column of this
      * result set was derived.
      * Because this feature is not widely supported, the return value
      * for many DBMSs will be an empty string.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the catalog name for column or an empty string.
      */
    getCatalogName( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Gets the JDBC type for the value stored in column. ... The STRUCT
      * and DISTINCT type codes are always returned for structured and
      * distinct types, regardless of whether the value will be mapped
      * according to the standard mapping or be a custom mapping.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the type of the property that corresponds to column - mapped
      *         from UNO-Type to SQL-Type.
      */
    virtual sal_Int32 SAL_CALL
    getColumnType( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Gets the type name used by this particular data source for the
      * values stored in column. If the type code for the type of value
      * stored in column is STRUCT, DISTINCT or JAVA_OBJECT, this method
      * returns a fully-qualified SQL type name.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the column type name.
      */
    virtual OUString SAL_CALL
    getColumnTypeName( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Indicates whether a column is definitely not writable.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if the column is definetely not writable.
      */
    virtual sal_Bool SAL_CALL
    isReadOnly( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Indicates whether it is possible for a write on the column to succeed.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if it is possible for a write to succeed.
      */
    virtual sal_Bool SAL_CALL
    isWritable( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Indicates whether a write on the column will definitely succeed.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if a write on the column will definetely succeed.
      */
    virtual sal_Bool SAL_CALL
    isDefinitelyWritable( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    /**
      * Returns the fully-qualified name of the service whose instances
      * are manufactured if the method
      * css::sdbc::ResultSet::getObject is called to retrieve a
      * value from the column.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the service name for column or an empty string, if no service
      *         is applicable.
      */
    virtual OUString SAL_CALL
    getColumnServiceName( sal_Int32 column )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
};

} // namespace ucbhelper

#endif /* ! INCLUDED_UCBHELPER_RESULTSETMETADATA_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
