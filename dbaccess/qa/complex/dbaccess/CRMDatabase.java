/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CRMDatabase.java,v $
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
import com.sun.star.io.IOException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XRefreshable;
import connectivity.tools.DataSource;
import connectivity.tools.HsqlColumnDescriptor;
import connectivity.tools.HsqlDatabase;
import connectivity.tools.HsqlTableDescriptor;
import connectivity.tools.QueryDefinition;

/** implements a small Customer Relationship Management database
 *
 *  Not finished, by far. Feel free to add features as you need them.
 */
public class CRMDatabase
{
    private XMultiServiceFactory        m_orb;
    private HsqlDatabase                m_database;
    private DataSource                  m_dataSource;
    private XConnection                 m_connection;

    /** constructs the CRM database
     */
    public CRMDatabase( XMultiServiceFactory _orb ) throws Exception
    {
        m_orb = _orb;

        m_database = new HsqlDatabase( m_orb );
        m_dataSource = m_database.getDataSource();
        m_connection = m_database.defaultConnection();
        createTables();
        createQueries();
    }

    // --------------------------------------------------------------------------------------------------------
    /** returns the database document underlying the CRM database
     */
    public final HsqlDatabase getDatabase()
    {
        return m_database;
    }

    // --------------------------------------------------------------------------------------------------------
    /** returns the default connection to the database
     */
    public final XConnection getConnection()
    {
        return m_connection;
    }

    // --------------------------------------------------------------------------------------------------------
    public void close() throws SQLException, IOException
    {
        m_database.store();
        m_database.close();
    }

    // --------------------------------------------------------------------------------------------------------
    private void createTables() throws SQLException
    {
        HsqlTableDescriptor table = new HsqlTableDescriptor( "categories",
            new HsqlColumnDescriptor[] {
                new HsqlColumnDescriptor( "ID", "INTEGER", HsqlColumnDescriptor.PRIMARY ),
                new HsqlColumnDescriptor( "Name", "VARCHAR(50)" ),
                new HsqlColumnDescriptor( "Description", "VARCHAR(1024)" ),
                new HsqlColumnDescriptor( "Image", "LONGVARBINARY" ) } );
        m_database.createTable( table, true );

        m_database.executeSQL( "INSERT INTO \"categories\" ( \"ID\", \"Name\" ) VALUES ( 1, 'Food' )" );
        m_database.executeSQL( "INSERT INTO \"categories\" ( \"ID\", \"Name\" ) VALUES ( 2, 'Furniture' )" );

        table = new HsqlTableDescriptor( "products",
            new HsqlColumnDescriptor[] {
                new HsqlColumnDescriptor( "ID", "INTEGER", HsqlColumnDescriptor.PRIMARY ),
                new HsqlColumnDescriptor( "Name", "VARCHAR(50)" ),
                new HsqlColumnDescriptor( "CategoryID", "INTEGER", HsqlColumnDescriptor.REQUIRED, "categories", "ID" ) } );
        m_database.createTable( table, true );

        m_database.executeSQL( "INSERT INTO \"products\" VALUES ( 1, 'Oranges', 1 )" );
        m_database.executeSQL( "INSERT INTO \"products\" VALUES ( 2, 'Apples', 1 )" );
        m_database.executeSQL( "INSERT INTO \"products\" VALUES ( 3, 'Pears', 1 )" );
        m_database.executeSQL( "INSERT INTO \"products\" VALUES ( 4, 'Strawberries', 1 )" );

        table = new HsqlTableDescriptor( "customers",
            new HsqlColumnDescriptor[] {
                new HsqlColumnDescriptor( "ID", "INTEGER", HsqlColumnDescriptor.PRIMARY ),
                new HsqlColumnDescriptor( "Name", "VARCHAR(50)" ),
                new HsqlColumnDescriptor( "Address", "VARCHAR(50)" ),
                new HsqlColumnDescriptor( "City", "VARCHAR(50)" ),
                new HsqlColumnDescriptor( "Postal", "VARCHAR(50)" ) } );
        m_database.createTable( table, true );

        m_database.executeSQL( "INSERT INTO \"customers\" VALUES(1,'Food, Inc.','Down Under','Melbourne','509') " );
        m_database.executeSQL( "INSERT INTO \"customers\" VALUES(2,'Simply Delicious','Down Under','Melbourne','518') " );
        m_database.executeSQL( "INSERT INTO \"customers\" VALUES(3,'Pure Health','10 Fish St.','San Francisco','94107') " );
        m_database.executeSQL( "INSERT INTO \"customers\" VALUES(4,'Milk And More','Arlington Road 21','Dublin','31021') " );

        table = new HsqlTableDescriptor( "orders",
            new HsqlColumnDescriptor[] {
                new HsqlColumnDescriptor( "ID", "INTEGER", HsqlColumnDescriptor.PRIMARY ),
                new HsqlColumnDescriptor( "CustomerID", "INTEGER", HsqlColumnDescriptor.REQUIRED, "customers", "ID"  ),
                new HsqlColumnDescriptor( "OrderDate", "DATE" ),
                new HsqlColumnDescriptor( "ShipDate", "DATE" ) } );
        m_database.createTable( table, true );

        m_database.executeSQL( "INSERT INTO \"orders\" (\"ID\", \"CustomerID\", \"OrderDate\") VALUES(1, 1, {D '2009-01-01'})" );
        m_database.executeSQL( "INSERT INTO \"orders\" VALUES(2, 2, {D '2009-01-01'}, {D '2009-01-23'})" );

        table = new HsqlTableDescriptor( "orders_details",
            new HsqlColumnDescriptor[] {
                new HsqlColumnDescriptor( "OrderID", "INTEGER", HsqlColumnDescriptor.PRIMARY, "orders", "ID" ),
                new HsqlColumnDescriptor( "ProductID", "INTEGER", HsqlColumnDescriptor.PRIMARY, "products", "ID"  ),
                new HsqlColumnDescriptor( "Quantity", "INTEGER" ) } );
        m_database.createTable( table, true );

        m_database.executeSQL( "INSERT INTO \"orders_details\" VALUES(1, 1, 100)" );
        m_database.executeSQL( "INSERT INTO \"orders_details\" VALUES(1, 2, 100)" );
        m_database.executeSQL( "INSERT INTO \"orders_details\" VALUES(2, 2, 2000)" );
        m_database.executeSQL( "INSERT INTO \"orders_details\" VALUES(2, 3, 2000)" );
        m_database.executeSQL( "INSERT INTO \"orders_details\" VALUES(2, 4, 2000)" );

        // since we created the tables by directly executing the SQL statements, we need to refresh
        // the tables container
        XTablesSupplier suppTables = (XTablesSupplier)UnoRuntime.queryInterface(
            XTablesSupplier.class, m_connection );
        XRefreshable refreshTables = (XRefreshable)UnoRuntime.queryInterface(
            XRefreshable.class, suppTables.getTables() );
        refreshTables.refresh();
    }

    // --------------------------------------------------------------------------------------------------------
    private void validateUnparseable()
    {
        // The "unparseable" query should be indeed be unparseable by OOo (though a valid HSQL query)
        XSingleSelectQueryComposer composer = null;
        QueryDefinition unparseableQuery = null;
        try
        {
            XMultiServiceFactory factory = (XMultiServiceFactory)UnoRuntime.queryInterface(
                    XMultiServiceFactory.class, m_database.defaultConnection() );
            composer = (XSingleSelectQueryComposer)UnoRuntime.queryInterface(
                    XSingleSelectQueryComposer.class, factory.createInstance( "com.sun.star.sdb.SingleSelectQueryComposer" ) );
            unparseableQuery = m_dataSource.getQueryDefinition( "unparseable" );
        }
        catch( Exception e )
        {
            throw new RuntimeException( "caught an unexpected exception: " + e.getMessage() );
        }

        boolean caughtExpected = false;
        try
        {
            composer.setQuery( unparseableQuery.getCommand() );
        }
        catch (WrappedTargetException e) { }
        catch( SQLException e )
        {
            caughtExpected = true;
        }

        if ( !caughtExpected )
            throw new RuntimeException( "Somebody improved the parser! This is bad :), since we need an unparsable query here!" );
    }

    // --------------------------------------------------------------------------------------------------------
    private void createQueries() throws ElementExistException, WrappedTargetException, com.sun.star.lang.IllegalArgumentException
    {
        m_database.getDataSource().createQuery(
            "all orders",
            "SELECT \"orders\".\"ID\" AS \"Order No.\", " +
                    "\"customers\".\"Name\" AS \"Customer Name\", " +
                    "\"orders\".\"OrderDate\" AS \"Order Date\", " +
                    "\"orders\".\"ShipDate\" AS \"Ship Date\", " +
                    "\"orders_details\".\"Quantity\", " +
                    "\"products\".\"Name\" AS \"Product Name\" " +
            "FROM \"orders_details\" AS \"orders_details\", " +
                  "\"orders\" AS \"orders\", " +
                  "\"products\" AS \"products\", " +
                  "\"customers\" AS \"customers\" " +
            "WHERE  ( \"orders_details\".\"OrderID\" = \"orders\".\"ID\" " +
                 "AND \"orders_details\".\"ProductID\" = \"products\".\"ID\" " +
                 "AND \"orders\".\"CustomerID\" = \"customers\".\"ID\" )"
        );

        m_database.getDataSource().createQuery(
            "unshipped orders",
            "SELECT * " +
            "FROM \"all orders\"" +
            "WHERE ( \"ShipDate\" IS NULL )"
        );

        m_database.getDataSource().createQuery( "parseable", "SELECT * FROM \"customers\"" );
        m_database.getDataSource().createQuery( "parseable native", "SELECT * FROM INFORMATION_SCHEMA.SYSTEM_VIEWS", false );
        m_database.getDataSource().createQuery( "unparseable",
            "SELECT CAST( \"ID\" AS VARCHAR(3) ) AS \"ID_VARCHAR\" FROM \"products\"", false );

        validateUnparseable();
    }
}
