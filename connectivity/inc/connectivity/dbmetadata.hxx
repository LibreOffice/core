/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CONNECTIVITY_INC_CONNECTIVITY_DBMETADATA_HXX
#define CONNECTIVITY_INC_CONNECTIVITY_DBMETADATA_HXX

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/XConnection.hpp>
/** === end UNO includes === **/

#include <memory>
#include "connectivity/dbtoolsdllapi.hxx"

namespace comphelper
{
    class ComponentContext;
}

//........................................................................
namespace dbtools
{
//........................................................................

    //====================================================================
    //= DatabaseMetaData
    //====================================================================
    struct DatabaseMetaData_Impl;
    /** encapsulates meta data about a database/connection which cannot be obtained
        from the usual XDatabaseMetaData result set.

        Meta data perhaps isn't really the right term ... Some of the methods
        in this class involved heuristics, some are just a convenient wrapper
        around more complex ways to obtain the same information.

        @todo
            Once CWS dba30 is integrated, we could easily add all the meta data
            which is part of the "Info" property of a data source.
    */
    class OOO_DLLPUBLIC_DBTOOLS DatabaseMetaData
    {
    private:
        ::std::auto_ptr< DatabaseMetaData_Impl >    m_pImpl;

    public:
        DatabaseMetaData();
        /** constructs a DatabaseMetaData instance
            @param  _rxConnection
                is the connection whose meta data you're interested in.
                Note that some of the information provided by this class can only be obtained
                if this connection denotes an application-level connection, i.e. supports
                the com.sun.star.sdb.Connection service.

            @throws ::com::sun::star::lang::IllegalArgumentException
                if the given connection is not <NULL/>, but the XDatabaseMetaData provided by it
                are <NULL/>
            @throws ::com::sun::star::sdbc::SQLException
                if obtaining the meta data from the connection throws an SQLException
            @throws ::com::sun::star::uno::RuntimeException
                if obtaining the meta data from the connection throws an RuntimeException
        */
        DatabaseMetaData(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _connection );
        DatabaseMetaData( const DatabaseMetaData& _copyFrom );
        DatabaseMetaData& operator=( const DatabaseMetaData& _copyFrom );

        ~DatabaseMetaData();

    public:
        /** determines whether or not the instances is based on a valid connection

            As long as this method returns true<TRUE/>, you should expect all other
            methods throwing an SQLException when called.
        */
        bool    isConnected() const;

        /** resets the instance so that it's based on a new connection
        */
        inline  void    reset( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _connection )
        {
            *this = DatabaseMetaData( _connection );
        }

        /// wraps XDatabaseMetaData::getIdentifierQuoteString
        const ::rtl::OUString&  getIdentifierQuoteString() const;

        /// wraps XDatabaseMetaData::getCatalogSeparator
        const ::rtl::OUString&  getCatalogSeparator() const;

        /** determines whether the database supports sub queries in the FROM part
            of a SELECT clause are supported.
            @throws ::com::sun::star::sdbc::SQLException
                with SQLState 08003 (connection does not exist) if the instances was
                default-constructed and does not have a connection, yet.
        */
        bool supportsSubqueriesInFrom() const;

        /** checks whether the database supports primary keys

            Since there's no dedicated API to ask a database for this, a heuristics needs to be applied.
            First, the <code>PrimaryKeySupport<code> settings of the data source is examined. If it is <TRUE/>
            or <FALSE/>, then value is returned. If it is <NULL/>, then the database meta data are examined
            for support of core SQL grammar, and the result is returned. The assumption is that a database/driver
            which supports core SQL grammar usually also supports primary keys, and vice versa. At least, experience
            shows this is true most of the time.
        */
        bool supportsPrimaryKeys() const;

        /** determines whether names in the database should be restricted to SQL-92 identifiers

            Effectively, this method checks the EnableSQL92Check property of the data source settings,
            if present.
        */
        bool restrictIdentifiersToSQL92() const;

        /** determines whether when generating SQL statements, an AS keyword should be generated
            before a correlation name.

            E.g., it determines whether <code>SELECT * FROM table AS correlation_name</code> or
            <code>SELECT * FROM table correlation_name</code> is generated.
        */
        bool generateASBeforeCorrelationName() const;

        /** should date time be escaped like '2001-01-01' => #2001-01-01#
        */
        bool shouldEscapeDateTime() const;

        /** auto increment columns should be automaticly used as primary key.
        */
        bool isAutoIncrementPrimaryKey() const;

        /** determines the syntax to use for boolean comparison predicates

            @see ::com::sun::star::sdb::BooleanComparisonMode
        */
        sal_Int32
            getBooleanComparisonMode() const;

        /** determines in relations are supported.
        *
        * \return <TRUE/> when relations are supported, otherwise <FALSE/>
        */
        bool supportsRelations() const;

        /** determines if column alias names can be used in the order by clause.
        *
        * \return <TRUE/> when relations are supported, otherwise <FALSE/>
        */
        bool supportsColumnAliasInOrderBy() const;

        /** determines whether user administration is supported for the database

            User administration support is controlled by the availability of the XUsersSupplier
            interface, and it returning a non-NULL users container.

            @param _rContext
                the component context we operate in. Might be needed to create the
                css.sdbc.DriverManager instance.
        */
        bool    supportsUserAdministration( const ::comphelper::ComponentContext& _rContext ) const;

        /** determines whether in the application UI, empty table folders (aka catalogs/schemas) should be displayed
        */
        bool displayEmptyTableFolders() const;

        /** determines that threads are supported.
        *
        * \return <TRUE/> when threads are supported, otherwise <FALSE/>
        */
        bool supportsThreads() const;
    };

//........................................................................
} // namespace dbtools
//........................................................................

#endif // CONNECTIVITY_INC_CONNECTIVITY_DBMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
