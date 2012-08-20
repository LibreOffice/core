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
package connectivity.tools;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyState;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.io.IOException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XRefreshable;
import connectivity.tools.sdb.Connection;

/** implements a small Customer Relationship Management database
 *
 *  Not finished, by far. Feel free to add features as you need them.
 */
public class CRMDatabase
{
    private static final String INTEGER = "INTEGER";
    private static final String VARCHAR50 = "VARCHAR(50)";
    private final XMultiServiceFactory        m_orb;
    private final HsqlDatabase                m_database;
    private final DataSource                  m_dataSource;
    private final Connection                  m_connection;

    /** constructs the CRM database
     */
    public CRMDatabase( XMultiServiceFactory _orb, boolean _withUI ) throws Exception
    {
        m_orb = _orb;

        m_database = new HsqlDatabase( m_orb );
        m_dataSource = m_database.getDataSource();

        if ( _withUI )
        {
            final XComponentLoader loader = UnoRuntime.queryInterface( XComponentLoader.class,
                m_orb.createInstance( "com.sun.star.frame.Desktop" ) );
            PropertyValue[] loadArgs = new PropertyValue[] {
                new PropertyValue( "PickListEntry", 0, false, PropertyState.DIRECT_VALUE )
            };
            loader.loadComponentFromURL( m_database.getDocumentURL(), "_blank", 0, loadArgs );
            getDocumentUI().connect();
            m_connection = new Connection( getDocumentUI().getActiveConnection() );
        }
        else
        {
            m_connection = m_database.defaultConnection();
        }

        createTables();
        createQueries();
    }

    /**
     * creates a CRMDatabase from an existing document, given by URL
     * @param _orb
     * @param _existingDocumentURL
     * @throws Exception
     */
    public CRMDatabase( XMultiServiceFactory _orb, final String _existingDocumentURL ) throws Exception
    {
        m_orb = _orb;

        m_database = new HsqlDatabase( m_orb, _existingDocumentURL );
        m_dataSource = m_database.getDataSource();
        m_connection = m_database.defaultConnection();
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
    public final Connection getConnection()
    {
        return m_connection;
    }

    // --------------------------------------------------------------------------------------------------------
    public void saveAndClose() throws SQLException, IOException
    {
        XDatabaseDocumentUI ui = getDocumentUI();
        if ( ui != null )
            ui.closeSubComponents();
        m_database.store();
        m_database.closeAndDelete();
    }

    // --------------------------------------------------------------------------------------------------------
    public XDatabaseDocumentUI getDocumentUI()
    {
        XModel docModel = UnoRuntime.queryInterface( XModel.class, m_database.getDatabaseDocument() );
        return UnoRuntime.queryInterface( XDatabaseDocumentUI.class, docModel.getCurrentController() );
    }

    // --------------------------------------------------------------------------------------------------------
    public XController loadSubComponent( final int _objectType, final String _name ) throws IllegalArgumentException, SQLException, NoSuchElementException
    {
        XDatabaseDocumentUI docUI = getDocumentUI();
        if ( !docUI.isConnected() )
            docUI.connect();

        XComponent subComponent = docUI.loadComponent( _objectType, _name, false );
        XController controller = UnoRuntime.queryInterface( XController.class, subComponent );
        if ( controller != null )
            return controller;
        XModel document = UnoRuntime.queryInterface( XModel.class, subComponent );
        return document.getCurrentController();
    }

    // --------------------------------------------------------------------------------------------------------
    private void createTables() throws SQLException
    {
        HsqlTableDescriptor table = new HsqlTableDescriptor( "categories",
            new HsqlColumnDescriptor[] {
                new HsqlColumnDescriptor( "ID",INTEGER, HsqlColumnDescriptor.PRIMARY ),
                new HsqlColumnDescriptor( "Name",VARCHAR50),
                new HsqlColumnDescriptor( "Description", "VARCHAR(1024)" ),
                new HsqlColumnDescriptor( "Image", "LONGVARBINARY" ) } );
        m_database.createTable( table, true );

        m_database.executeSQL( "INSERT INTO \"categories\" ( \"ID\", \"Name\" ) VALUES ( 1, 'Food' )" );
        m_database.executeSQL( "INSERT INTO \"categories\" ( \"ID\", \"Name\" ) VALUES ( 2, 'Furniture' )" );

        table = new HsqlTableDescriptor( "products",
            new HsqlColumnDescriptor[] {
                new HsqlColumnDescriptor( "ID",INTEGER, HsqlColumnDescriptor.PRIMARY ),
                new HsqlColumnDescriptor( "Name",VARCHAR50),
                new HsqlColumnDescriptor( "CategoryID",INTEGER, HsqlColumnDescriptor.REQUIRED, "categories", "ID" ) } );
        m_database.createTable( table, true );

        m_database.executeSQL( "INSERT INTO \"products\" VALUES ( 1, 'Oranges', 1 )" );
        m_database.executeSQL( "INSERT INTO \"products\" VALUES ( 2, 'Apples', 1 )" );
        m_database.executeSQL( "INSERT INTO \"products\" VALUES ( 3, 'Pears', 1 )" );
        m_database.executeSQL( "INSERT INTO \"products\" VALUES ( 4, 'Strawberries', 1 )" );

        table = new HsqlTableDescriptor( "customers",
            new HsqlColumnDescriptor[] {
                new HsqlColumnDescriptor( "ID",INTEGER, HsqlColumnDescriptor.PRIMARY ),
                new HsqlColumnDescriptor( "Name",VARCHAR50),
                new HsqlColumnDescriptor( "Address",VARCHAR50),
                new HsqlColumnDescriptor( "City",VARCHAR50),
                new HsqlColumnDescriptor( "Postal",VARCHAR50),
                new HsqlColumnDescriptor( "Comment","LONGVARCHAR")} );
        m_database.createTable( table, true );

        m_database.executeSQL( "INSERT INTO \"customers\" VALUES(1,'Food, Inc.','Down Under','Melbourne','509','Prefered') " );
        m_database.executeSQL( "INSERT INTO \"customers\" VALUES(2,'Simply Delicious','Down Under','Melbourne','518',null) " );
        m_database.executeSQL( "INSERT INTO \"customers\" VALUES(3,'Pure Health','10 Fish St.','San Francisco','94107',null) " );
        m_database.executeSQL( "INSERT INTO \"customers\" VALUES(4,'Milk And More','Arlington Road 21','Dublin','31021','Good one.') " );

        table = new HsqlTableDescriptor( "orders",
            new HsqlColumnDescriptor[] {
                new HsqlColumnDescriptor( "ID",INTEGER, HsqlColumnDescriptor.PRIMARY ),
                new HsqlColumnDescriptor( "CustomerID",INTEGER, HsqlColumnDescriptor.REQUIRED, "customers", "ID"  ),
                new HsqlColumnDescriptor( "OrderDate", "DATE" ),
                new HsqlColumnDescriptor( "ShipDate", "DATE" ) } );
        m_database.createTable( table, true );

        m_database.executeSQL( "INSERT INTO \"orders\" (\"ID\", \"CustomerID\", \"OrderDate\") VALUES(1, 1, {D '2009-01-01'})" );
        m_database.executeSQL( "INSERT INTO \"orders\" VALUES(2, 2, {D '2009-01-01'}, {D '2009-01-23'})" );

        table = new HsqlTableDescriptor( "orders_details",
            new HsqlColumnDescriptor[] {
                new HsqlColumnDescriptor( "OrderID",INTEGER, HsqlColumnDescriptor.PRIMARY, "orders", "ID" ),
                new HsqlColumnDescriptor( "ProductID",INTEGER, HsqlColumnDescriptor.PRIMARY, "products", "ID"  ),
                new HsqlColumnDescriptor( "Quantity",INTEGER) } );
        m_database.createTable( table, true );

        m_database.executeSQL( "INSERT INTO \"orders_details\" VALUES(1, 1, 100)" );
        m_database.executeSQL( "INSERT INTO \"orders_details\" VALUES(1, 2, 100)" );
        m_database.executeSQL( "INSERT INTO \"orders_details\" VALUES(2, 2, 2000)" );
        m_database.executeSQL( "INSERT INTO \"orders_details\" VALUES(2, 3, 2000)" );
        m_database.executeSQL( "INSERT INTO \"orders_details\" VALUES(2, 4, 2000)" );

        // since we created the tables by directly executing the SQL statements, we need to refresh
        // the tables container
        m_connection.refreshTables();
    }

    // --------------------------------------------------------------------------------------------------------
    private void validateUnparseable()
    {
    /*
        // The "unparseable" query should be indeed be unparseable by OOo (though a valid HSQL query)
        XSingleSelectQueryComposer composer;
        QueryDefinition unparseableQuery;
        try
        {
            final XMultiServiceFactory factory = UnoRuntime.queryInterface(
                    XMultiServiceFactory.class, m_database.defaultConnection().getXConnection() );
            composer = UnoRuntime.queryInterface(
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
            */
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
/*
        m_database.getDataSource().createQuery( "unparseable",
            "SELECT {fn DAYOFMONTH ('2001-01-01')} AS \"ID_VARCHAR\" FROM \"products\"", false );
*/
        validateUnparseable();
    }
}
