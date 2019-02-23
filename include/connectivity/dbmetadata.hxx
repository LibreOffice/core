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

#ifndef INCLUDED_CONNECTIVITY_DBMETADATA_HXX
#define INCLUDED_CONNECTIVITY_DBMETADATA_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <memory>
#include <connectivity/dbtoolsdllapi.hxx>

namespace com::sun::star::sdbc { class XConnection; }
namespace com::sun::star::uno { class XComponentContext; }

namespace dbtools
{


    //= DatabaseMetaData

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
        ::std::unique_ptr< DatabaseMetaData_Impl >    m_pImpl;

    public:
        DatabaseMetaData();
        /** constructs a DatabaseMetaData instance
            @param  _rxConnection
                is the connection whose meta data you're interested in.
                Note that some of the information provided by this class can only be obtained
                if this connection denotes an application-level connection, i.e. supports
                the com.sun.star.sdb.Connection service.

            @throws css::lang::IllegalArgumentException
                if the given connection is not <NULL/>, but the XDatabaseMetaData provided by it
                are <NULL/>
            @throws css::sdbc::SQLException
                if obtaining the meta data from the connection throws an SQLException
            @throws css::uno::RuntimeException
                if obtaining the meta data from the connection throws an RuntimeException
        */
        DatabaseMetaData(
            const css::uno::Reference< css::sdbc::XConnection >& _connection );
        DatabaseMetaData( const DatabaseMetaData& _copyFrom );
        DatabaseMetaData& operator=( const DatabaseMetaData& _copyFrom );
        DatabaseMetaData( DatabaseMetaData&& _copyFrom );
        DatabaseMetaData& operator=( DatabaseMetaData&& _copyFrom );

        ~DatabaseMetaData();

    public:
        /** determines whether or not the instances is based on a valid connection

            As long as this method returns true<TRUE/>, you should expect all other
            methods throwing an SQLException when called.
        */
        bool    isConnected() const;

        /** resets the instance so that it's based on a new connection
        */
        void    reset( const css::uno::Reference< css::sdbc::XConnection >& _connection )
        {
            *this = DatabaseMetaData( _connection );
        }

        /// wraps XDatabaseMetaData::getIdentifierQuoteString
        const OUString&  getIdentifierQuoteString() const;

        /// wraps XDatabaseMetaData::getCatalogSeparator
        const OUString&  getCatalogSeparator() const;

        /** determines whether the database supports sub queries in the FROM part
            of a SELECT clause are supported.
            @throws css::sdbc::SQLException
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

        /** should date time be escaped like '2001-01-01' => {D '2001-01-01' }
        */
        bool shouldEscapeDateTime() const;

        /** should named parameters (:foo, [foo]) be replaced by unnamed parameters (?)
        */
        bool shouldSubstituteParameterNames() const;

        /** auto increment columns should be automatically used as primary key.
        */
        bool isAutoIncrementPrimaryKey() const;

        /** determines the syntax to use for boolean comparison predicates

            @see css::sdb::BooleanComparisonMode
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
        bool    supportsUserAdministration( const css::uno::Reference<css::uno::XComponentContext>& _rContext ) const;

        /** determines whether in the application UI, empty table folders (aka catalogs/schemas) should be displayed
        */
        bool displayEmptyTableFolders() const;

        /** determines that threads are supported.
        *
        * \return <TRUE/> when threads are supported, otherwise <FALSE/>
        */
        bool supportsThreads() const;
    };


} // namespace dbtools


#endif // INCLUDED_CONNECTIVITY_DBMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
