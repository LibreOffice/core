/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: QueryInQuery.java,v $
 * $Revision: 1.6.2.1 $
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
package complex.dbaccess;

import com.sun.star.container.ElementExistException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdbc.SQLException;
import connectivity.tools.HsqlColumnDescriptor;
import connectivity.tools.HsqlTableDescriptor;
import connectivity.tools.RowSet;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbc.XResultSet;

public class QueryInQuery extends CRMBasedTestCase
{
    // --------------------------------------------------------------------------------------------------------
    public String[] getTestMethodNames()
    {
        return new String[] {
            "executeSimpleSelect",
            "executeAliasedSelect",
            "checkNameCollisions",
            "checkCyclicReferences",
            "checkStatementQiQSupport"
        };
    }

    // --------------------------------------------------------------------------------------------------------
    public String getTestObjectName()
    {
        return "QueryInQuery";
    }

    // --------------------------------------------------------------------------------------------------------
    protected void createTestCase()
    {
        try
        {
            super.createTestCase();
            m_database.getDatabase().getDataSource().createQuery( "query products", "SELECT * FROM \"products\"" );
        }
        catch ( Exception e )
        {
            e.printStackTrace( System.err );
            assure( "caught an exception (" + e.getMessage() + ") while creating the test case", false );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private void verifyEqualRowSetContent( int _outerCommandType, String _outerCommand, int _innerCommandType, String _innerCommand ) throws SQLException
    {
        RowSet outerRowSet = m_database.getDatabase().createRowSet( _outerCommandType, _outerCommand );
        outerRowSet.execute();

        RowSet innerRowSet = m_database.getDatabase().createRowSet( _innerCommandType, _innerCommand );
        innerRowSet.execute();

        outerRowSet.last();
        innerRowSet.last();
        assure( "wrong record counts", outerRowSet.getRow() == innerRowSet.getRow() );

        outerRowSet.beforeFirst();
        innerRowSet.beforeFirst();
        assure( "wrong column counts", outerRowSet.getColumnCount() == innerRowSet.getColumnCount() );

        while ( outerRowSet.next() && innerRowSet.next() )
        {
            for ( int i=1; i <= outerRowSet.getColumnCount(); ++i )
            {
                assure( "content of column " + i + " of row " + outerRowSet.getRow() + " not identical",
                    innerRowSet.getString(i).equals( outerRowSet.getString(i) ) );
            }
        }
    }

    // --------------------------------------------------------------------------------------------------------
    /** executes a SQL statement simply selecting all columns from a query
     */
    public void executeSimpleSelect() throws SQLException
    {
        verifyEqualRowSetContent(
            CommandType.COMMAND, "SELECT * FROM \"query products\"",
            CommandType.QUERY, "query products" );
    }

    // --------------------------------------------------------------------------------------------------------
    /** verifies that aliases for inner queries work as expected
     */
    public void executeAliasedSelect() throws SQLException
    {
        verifyEqualRowSetContent(
            CommandType.COMMAND, "SELECT \"PROD\".\"ID\" FROM \"query products\" AS \"PROD\"",
            CommandType.COMMAND, "SELECT \"ID\" FROM \"products\"" );
        verifyEqualRowSetContent(
            CommandType.COMMAND, "SELECT \"PROD\".* FROM \"query products\" AS \"PROD\"",
            CommandType.QUERY, "query products" );
    }

    // --------------------------------------------------------------------------------------------------------
    /** verifies that aliases for inner queries work as expected
     */
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
        assure( "creating queries with the name of an existing table should not be possible",
            caughtExpected );

        // create a table with a name which is used by a query
        HsqlTableDescriptor table = new HsqlTableDescriptor( "query products",
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
        assure( "creating tables with the name of an existing query should not be possible",
            caughtExpected );
    }

    // --------------------------------------------------------------------------------------------------------
    public void checkCyclicReferences() throws ElementExistException, WrappedTargetException, IllegalArgumentException
    {
        // some queries which create a cycle in the sub query tree
        m_database.getDatabase().getDataSource().createQuery( "orders level 1", "SELECT * FROM \"orders level 0\"" );
        m_database.getDatabase().getDataSource().createQuery( "orders level 2", "SELECT * FROM \"orders level 1\"" );
        m_database.getDatabase().getDataSource().createQuery( "orders level 3", "SELECT * FROM \"orders level 2\"" );
        m_database.getDatabase().getDataSource().createQuery( "orders level 0", "SELECT * FROM \"orders level 3\"" );

        RowSet rowSet = m_database.getDatabase().createRowSet( CommandType.QUERY, "orders level 0" );

        boolean caughtExpected = false;
        try { rowSet.execute(); }
        catch ( SQLException e ) { caughtExpected = ( e.ErrorCode == -com.sun.star.sdb.ErrorCondition.PARSER_CYCLIC_SUB_QUERIES ); }

        assure( "executing a query with cyclic nested sub queries should fail!", caughtExpected );
    }

    // --------------------------------------------------------------------------------------------------------
    public void checkStatementQiQSupport()
    {
        try
        {
            XStatement statement = m_database.getConnection().createStatement();
            XResultSet resultSet = statement.executeQuery( "SELECT * FROM \"query products\"" );
        }
        catch( SQLException e )
        {
            assure( "SDB level statements do not allow for queries in queries", false );
        }
    }
}
