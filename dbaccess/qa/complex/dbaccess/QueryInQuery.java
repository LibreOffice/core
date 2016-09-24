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
package complex.dbaccess;

import com.sun.star.container.ElementExistException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdbc.SQLException;
import connectivity.tools.HsqlColumnDescriptor;
import connectivity.tools.HsqlTableDescriptor;
import connectivity.tools.RowSet;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbc.XResultSet;

// ---------- junit imports -----------------
import org.junit.Test;
import static org.junit.Assert.*;


public class QueryInQuery extends CRMBasedTestCase
{
    private static final String QUERY_PRODUCTS = "query products";


    @Override
    protected void createTestCase()
    {
        try
        {
            super.createTestCase();
            m_database.getDatabase().getDataSource().createQuery( QUERY_PRODUCTS,"SELECT * FROM \"products\"");
        }
        catch ( Exception e )
        {
            e.printStackTrace( System.err );
            fail( "caught an exception (" + e.getMessage() + ") while creating the test case" );
        }
    }


    private void verifyEqualRowSetContent( int _outerCommandType, String _outerCommand, int _innerCommandType, String _innerCommand ) throws SQLException
    {
        final RowSet outerRowSet = m_database.getDatabase().createRowSet( _outerCommandType, _outerCommand );
        outerRowSet.execute();

        final RowSet innerRowSet = m_database.getDatabase().createRowSet( _innerCommandType, _innerCommand );
        innerRowSet.execute();

        outerRowSet.last();
        innerRowSet.last();
        assertTrue( "wrong record counts", outerRowSet.getRow() == innerRowSet.getRow() );

        outerRowSet.beforeFirst();
        innerRowSet.beforeFirst();
        assertTrue( "wrong column counts", outerRowSet.getColumnCount() == innerRowSet.getColumnCount() );

        while ( outerRowSet.next() && innerRowSet.next() )
        {
            for ( int i=1; i <= outerRowSet.getColumnCount(); ++i )
            {
                assertTrue( "content of column " + i + " of row " + outerRowSet.getRow() + " not identical",
                    innerRowSet.getString(i).equals( outerRowSet.getString(i) ) );
            }
        }
    }


    /** executes a SQL statement simply selecting all columns from a query
     */
    @Test
    public void executeSimpleSelect() throws SQLException
    {
        verifyEqualRowSetContent(
            CommandType.COMMAND, "SELECT * FROM \"query products\"",
            CommandType.QUERY,QUERY_PRODUCTS);
    }


    /** verifies that aliases for inner queries work as expected
     */
    @Test
    public void executeAliasedSelect() throws SQLException
    {
        verifyEqualRowSetContent(
            CommandType.COMMAND, "SELECT \"PROD\".\"ID\" FROM \"query products\" AS \"PROD\"",
            CommandType.COMMAND, "SELECT \"ID\" FROM \"products\"" );
        verifyEqualRowSetContent(
            CommandType.COMMAND, "SELECT \"PROD\".* FROM \"query products\" AS \"PROD\"",
            CommandType.QUERY,QUERY_PRODUCTS);
    }


    /** verifies that aliases for inner queries work as expected
     */
    @Test
    public void checkNameCollisions()
    {
        // create a query with a name which is used by a table
        boolean caughtExpected = false;
        try
        {
            m_database.getDatabase().getDataSource().createQuery( "products", "SELECT * FROM \"products\"" );
        }
        catch ( WrappedTargetException e ) { caughtExpected = true; }
        catch ( IllegalArgumentException e ) {}
        catch ( ElementExistException e ) { caughtExpected = true; }
        assertTrue( "creating queries with the name of an existing table should not be possible",
            caughtExpected );

        // create a table with a name which is used by a query
        final HsqlTableDescriptor table = new HsqlTableDescriptor( QUERY_PRODUCTS,
            new HsqlColumnDescriptor[] {
                new HsqlColumnDescriptor( "ID", "INTEGER" ),
                new HsqlColumnDescriptor( "Name", "VARCHAR(50)" ) } );

        caughtExpected = false;
        try
        {
            m_database.getDatabase().createTableInSDBCX( table );
        }
        catch ( SQLException e ) { caughtExpected = true; }
        catch ( ElementExistException ex ) { }
        assertTrue( "creating tables with the name of an existing query should not be possible",
            caughtExpected );
    }


    @Test
    public void checkCyclicReferences() throws ElementExistException, WrappedTargetException, IllegalArgumentException
    {
        // some queries which create a cycle in the sub query tree
        m_database.getDatabase().getDataSource().createQuery( "orders level 1", "SELECT * FROM \"orders level 0\"" );
        m_database.getDatabase().getDataSource().createQuery( "orders level 2", "SELECT * FROM \"orders level 1\"" );
        m_database.getDatabase().getDataSource().createQuery( "orders level 3", "SELECT * FROM \"orders level 2\"" );
        m_database.getDatabase().getDataSource().createQuery( "orders level 0", "SELECT * FROM \"orders level 3\"" );

        final RowSet rowSet = m_database.getDatabase().createRowSet( CommandType.QUERY, "orders level 0" );

        boolean caughtExpected = false;
        try { rowSet.execute(); }
        catch ( SQLException e ) { caughtExpected = ( e.ErrorCode == -com.sun.star.sdb.ErrorCondition.PARSER_CYCLIC_SUB_QUERIES ); }

        assertTrue( "executing a query with cyclic nested sub queries should fail!", caughtExpected );
    }


    @Test
    public void checkStatementQiQSupport()
    {
        try
        {
            final XStatement statement = m_database.getConnection().createStatement();
            final XResultSet resultSet = statement.executeQuery( "SELECT * FROM \"query products\"" );
            assertTrue( "Result Set is null", resultSet != null );
        }
        catch( SQLException e )
        {
            fail( "SDB level statements do not allow for queries in queries" );
        }
    }
}
