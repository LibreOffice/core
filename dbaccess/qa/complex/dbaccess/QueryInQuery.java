/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QueryInQuery.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:26:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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
 ************************************************************************/
package complex.dbaccess;

import com.sun.star.container.ElementExistException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.ErrorCondition;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.UnoRuntime;
import connectivity.tools.HsqlColumnDescriptor;
import connectivity.tools.HsqlTableDescriptor;
import connectivity.tools.RowSet;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbc.XResultSet;

public class QueryInQuery extends complexlib.ComplexTestCase
{
    private CRMDatabase                 m_database;

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
    private final XMultiServiceFactory getORB()
    {
        return (XMultiServiceFactory)param.getMSF();
    }

    // --------------------------------------------------------------------------------------------------------
    public String getTestObjectName()
    {
        return "QueryInQuery";
    }

    // --------------------------------------------------------------------------------------------------------
    private void createTestCase()
    {
        try
        {
            m_database = new CRMDatabase( getORB() );

            m_database.getDatabase().getDataSource().createQuery( "query products", "SELECT * FROM \"products\"" );
        }
        catch ( Exception e )
        {
            e.printStackTrace( System.err );
            assure( "caught an exception (" + e.getMessage() + ") while creating the test case", false );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    public void before()
    {
        createTestCase();
    }

    // --------------------------------------------------------------------------------------------------------
    public void after()
    {
        if ( m_database != null )
            m_database.close();
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
