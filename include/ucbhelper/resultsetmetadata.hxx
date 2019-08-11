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
#include <memory>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/ucbhelperdllapi.h>

namespace com { namespace sun { namespace star {
    namespace beans { struct Property; }
    namespace uno { class XComponentContext; }
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
    /** @see ResultSetMetaData::isCaseSensitive */
    bool        isCaseSensitive;

    inline ResultSetColumnData();
};

// Note: Never change the initial values! Implementations using this struct
//       may heavily depend on the behaviour of the default constructor.

ResultSetColumnData::ResultSetColumnData()
: isCaseSensitive( true )
{
}


/**
 * This is an implementation of the interface XResultSetMetaData. It can be
 * used to implement the interface
 * css::sdbc::XResultSetMetaDataSupplier, which is required for
 * implementations of service com.sun.star.ucb.ContentResultSet.
 */
class UCBHELPER_DLLPUBLIC ResultSetMetaData final :
                public ::cppu::OWeakObject,
                public css::lang::XTypeProvider,
                public css::sdbc::XResultSetMetaData
{
    std::unique_ptr<ucbhelper_impl::ResultSetMetaData_Impl> m_pImpl;
    css::uno::Reference< css::uno::XComponentContext >    m_xContext;
    css::uno::Sequence< css::beans::Property >            m_aProps;

public:

    /**
      * Constructor. ResultSet is readonly by default.
      *
      * @param rxSMgr is a Servive Manager.
      * @param rProps is a sequence of properties (partially) describing the
      *        columns of a resultset.
      */
    ResultSetMetaData(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            const css::uno::Sequence< css::beans::Property >& rProps );

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
    virtual ~ResultSetMetaData() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes() override;

    // XResultSetMetaData

    /**
      * Returns the number of columns of the resultset.
      *
      * @return the length of the property sequence.
      */
    virtual sal_Int32 SAL_CALL
    getColumnCount() override;
    /**
      * Checks whether column is automatically numbered, which makes it
      * read-only.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if column is automatically numbered.
      */
    virtual sal_Bool SAL_CALL
    isAutoIncrement( sal_Int32 column ) override;
    /**
      * Checks whether column is case sensitive.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if column is case sensitive.
      */
    virtual sal_Bool SAL_CALL
    isCaseSensitive( sal_Int32 column ) override;
    /**
      * Checks whether the value stored in column can be used in a
      * WHERE clause.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if the column is searchable.
      */
    virtual sal_Bool SAL_CALL
    isSearchable( sal_Int32 column ) override;
    /**
      * Checks whether column is a cash value.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if the column is a cash value.
      */
    virtual sal_Bool SAL_CALL
    isCurrency( sal_Int32 column ) override;
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
    isNullable( sal_Int32 column ) override;
    /**
      * Checks whether the value stored in column is a signed number.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if the value stored in column is a signed number.
      */
    virtual sal_Bool SAL_CALL
    isSigned( sal_Int32 column ) override;
    /**
      * Gets the normal maximum width in characters for column.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the normal maximum width in characters for column.
      */
    virtual sal_Int32 SAL_CALL
    getColumnDisplaySize( sal_Int32 column ) override;
    /**
      * Gets the suggested column title for column, to be used in print-
      * outs and displays.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the column label.
      */
    virtual OUString SAL_CALL
    getColumnLabel( sal_Int32 column ) override;
    /**
      * Gets the name of column.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the name of the property that corresponds to column.
      */
    virtual OUString SAL_CALL
    getColumnName( sal_Int32 column ) override;
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
    getSchemaName( sal_Int32 column ) override;
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
    getPrecision( sal_Int32 column ) override;
    /**
      * Gets the number of digits to the right of the decimal point for
      * values in column.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return the scale of the column.
      */
    virtual sal_Int32 SAL_CALL
    getScale( sal_Int32 column ) override;
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
    getTableName( sal_Int32 column ) override;
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
    getCatalogName( sal_Int32 column ) override;
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
    getColumnType( sal_Int32 column ) override;
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
    getColumnTypeName( sal_Int32 column ) override;
    /**
      * Indicates whether a column is definitely not writable.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if the column is definitely not writable.
      */
    virtual sal_Bool SAL_CALL
    isReadOnly( sal_Int32 column ) override;
    /**
      * Indicates whether it is possible for a write on the column to succeed.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if it is possible for a write to succeed.
      */
    virtual sal_Bool SAL_CALL
    isWritable( sal_Int32 column ) override;
    /**
      * Indicates whether a write on the column will definitely succeed.
      *
      * @param  column is the number of the column for that a value shall
      *         be returned. The first column is 1, the second is 2, ...
      * @return true, if a write on the column will definitely succeed.
      */
    virtual sal_Bool SAL_CALL
    isDefinitelyWritable( sal_Int32 column ) override;
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
    getColumnServiceName( sal_Int32 column ) override;
};

} // namespace ucbhelper

#endif /* ! INCLUDED_UCBHELPER_RESULTSETMETADATA_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
