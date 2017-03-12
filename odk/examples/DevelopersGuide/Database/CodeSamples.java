/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.sdbc.*;
import com.sun.star.sdb.*;
import com.sun.star.sdbcx.*;
import com.sun.star.frame.*;

public class CodeSamples
{
    public static XComponentContext xContext;
    public static XMultiComponentFactory xMCF;

    public static void main(String argv[]) throws java.lang.Exception
    {
        try {
            // get the remote office component context
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
            xMCF = xContext.getServiceManager();
        }
        catch(Exception e) {
            System.err.println("ERROR: can't get a component context from a running office ...");
            e.printStackTrace();
            System.exit(1);
        }

        try{
            createQuerydefinition( );
            printQueryColumnNames( );

            XConnection con = openConnectionWithDriverManager();
            if ( con != null ) {
                {
                    SalesMan sm = new SalesMan( con );

                    try {
                        sm.dropSalesManTable( ); // doesn't matter here
                    }
                    catch(com.sun.star.uno.Exception e)
                    {
                    }
                    sm.createSalesManTable( );
                    sm.insertDataIntoSalesMan( );
                    sm.updateSalesMan( );
                    sm.retrieveSalesManData( );
                }

                {
                    Sales sm = new Sales( con );

                    try {
                        sm.dropSalesTable( ); // doesn't matter here
                    }
                    catch(com.sun.star.uno.Exception e)
                    {
                    }
                    sm.createSalesTable( );
                    sm.insertDataIntoSales( );
                    sm.updateSales( );
                    sm.retrieveSalesData( );
                    sm.displayColumnNames( );
                }
                displayTableStructure( con );
            }
        }
        catch(Exception e)
        {
            System.err.println(e);
            e.printStackTrace();
        }
        System.exit(0);
    }

    // check if the connection is not null and dispose it later on.
    public static void checkConnection(XConnection con)
    {
        if(con != null)
        {
            System.out.println("Connection was created!");
            // now we dispose the connection to close it
            XComponent xComponent = UnoRuntime.queryInterface(XComponent.class,con);
            if(xComponent != null)
            {
                // connections must be disposed
                xComponent.dispose();
                System.out.println("Connection disposed!");
            }
        }
        else
            System.out.println("Connection could not be created!");
    }

    // uses the driver manager to create a new connection and dispose it.
    public static XConnection openConnectionWithDriverManager() throws com.sun.star.uno.Exception
    {
        XConnection con = null;
        // create the DriverManager
        Object driverManager =
            xMCF.createInstanceWithContext("com.sun.star.sdbc.DriverManager",
                                           xContext);
        // query for the interface
        com.sun.star.sdbc.XDriverManager xDriverManager;
        xDriverManager = UnoRuntime.queryInterface(XDriverManager.class,driverManager);
        if(xDriverManager != null)
        {
            // first create the needed url
            String url = "jdbc:mysql://localhost:3306/TestTables";
            // second create the necessary properties
            com.sun.star.beans.PropertyValue [] props = new com.sun.star.beans.PropertyValue[]
            {
                new com.sun.star.beans.PropertyValue("user",0,"test1",com.sun.star.beans.PropertyState.DIRECT_VALUE),
                new com.sun.star.beans.PropertyValue("password",0,"test1",com.sun.star.beans.PropertyState.DIRECT_VALUE),
                new com.sun.star.beans.PropertyValue("JavaDriverClass",0,"org.gjt.mm.mysql.Driver",com.sun.star.beans.PropertyState.DIRECT_VALUE)
            };
            // now create a connection to mysql
            con = xDriverManager.getConnectionWithInfo(url,props);
        }
        return con;
    }

    // uses the driver directly to create a new connection and dispose it.
    public static XConnection openConnectionWithDriver() throws com.sun.star.uno.Exception
    {
        XConnection con = null;
        // create the Driver with the implementation name
        Object aDriver =
            xMCF.createInstanceWithContext("org.openoffice.comp.drivers.MySQL.Driver",
                                           xContext);
        // query for the interface
        com.sun.star.sdbc.XDriver xDriver;
        xDriver = UnoRuntime.queryInterface(XDriver.class,aDriver);
        if(xDriver != null)
        {
            // first create the needed url
            String url = "jdbc:mysql://localhost:3306/TestTables";
            // second create the necessary properties
            com.sun.star.beans.PropertyValue [] props = new com.sun.star.beans.PropertyValue[]
            {
                new com.sun.star.beans.PropertyValue("user",0,"test1",com.sun.star.beans.PropertyState.DIRECT_VALUE),
                new com.sun.star.beans.PropertyValue("password",0,"test1",com.sun.star.beans.PropertyState.DIRECT_VALUE),
                                new com.sun.star.beans.PropertyValue("JavaDriverClass",0,"org.gjt.mm.mysql.Driver",com.sun.star.beans.PropertyState.DIRECT_VALUE)
            };
            // now create a connection to mysql
            con = xDriver.connect(url,props);
        }
        return con;
    }

    // print all available datasources
    public static void printDataSources() throws com.sun.star.uno.Exception
    {
        // create a DatabaseContext and print all DataSource names
        XNameAccess xNameAccess = UnoRuntime.queryInterface(
            XNameAccess.class,
            xMCF.createInstanceWithContext("com.sun.star.sdb.DatabaseContext",
                                           xContext));
        String aNames [] = xNameAccess.getElementNames();
        for(int i=0;i<aNames.length;++i)
            System.out.println(aNames[i]);
    }

    // displays the structure of the first table
    public static void displayTableStructure(XConnection con) throws com.sun.star.uno.Exception
    {
        XDatabaseMetaData dm = con.getMetaData();
        XResultSet rsTables = dm.getTables(null,"%","SALES",null);
        XRow       rowTB = UnoRuntime.queryInterface(XRow.class, rsTables);
        while ( rsTables.next() )
        {
            String catalog = rowTB.getString( 1 );
            if ( rowTB.wasNull() )
                catalog = null;

            String schema = rowTB.getString( 2 );
            if ( rowTB.wasNull() )
                schema = null;

            String table = rowTB.getString( 3 );
            String type = rowTB.getString( 4 );
            System.out.println("Catalog: " + catalog + " Schema: " + schema + " Table: " + table + " Type: " + type);
            System.out.println("------------------ Columns ------------------");
            XResultSet rsColumns = dm.getColumns(catalog,schema,table,"%");
            XRow       rowCL = UnoRuntime.queryInterface(XRow.class, rsColumns);
            while ( rsColumns.next() )
            {
                System.out.println("Column: " + rowCL.getString( 4 ) + " Type: " + rowCL.getInt( 5 ) + " TypeName: " + rowCL.getString( 6 ) );
            }

        }
    }

    // quote the given name
    public static String quoteTableName(XConnection con, String sCatalog, String sSchema, String sTable) throws com.sun.star.uno.Exception
    {
        XDatabaseMetaData dbmd = con.getMetaData();
        String sQuoteString = dbmd.getIdentifierQuoteString();
        String sSeparator = ".";
        String sComposedName = "";
        String sCatalogSep = dbmd.getCatalogSeparator();
        if (0 != sCatalog.length() && dbmd.isCatalogAtStart() && 0 != sCatalogSep.length())
        {
            sComposedName += sCatalog;
            sComposedName += dbmd.getCatalogSeparator();
        }
        if (0 != sSchema.length())
        {
            sComposedName += sSchema;
            sComposedName += sSeparator;
            sComposedName += sTable;
        }
        else
                {
            sComposedName += sTable;
        }
        if (0 != sCatalog.length() && !dbmd.isCatalogAtStart() && 0 != sCatalogSep.length())
        {
            sComposedName += dbmd.getCatalogSeparator();
            sComposedName += sCatalog;
        }
        return sComposedName;
    }

    // creates a new query definition
    public static void createQuerydefinition() throws com.sun.star.uno.Exception
    {
        XNameAccess xNameAccess = UnoRuntime.queryInterface(
            XNameAccess.class,
            xMCF.createInstanceWithContext("com.sun.star.sdb.DatabaseContext",
                                           xContext));
        // we use the first datasource
        XQueryDefinitionsSupplier xQuerySup = UnoRuntime.queryInterface(XQueryDefinitionsSupplier.class,
        xNameAccess.getByName( "Bibliography" ));
        XNameAccess xQDefs = xQuerySup.getQueryDefinitions();
        // create new query definition
        XSingleServiceFactory xSingleFac =  UnoRuntime.queryInterface(XSingleServiceFactory.class, xQDefs);

        XPropertySet xProp = UnoRuntime.queryInterface(
            XPropertySet.class,xSingleFac.createInstance());
        xProp.setPropertyValue("Command","SELECT * FROM biblio");
        xProp.setPropertyValue("EscapeProcessing",Boolean.TRUE);

        XNameContainer xCont = UnoRuntime.queryInterface(XNameContainer.class, xQDefs);
                try
                {
                    if ( xCont.hasByName("Query1") )
                        xCont.removeByName("Query1");
                }
                catch(com.sun.star.uno.Exception e)
                {}
        xCont.insertByName("Query1",xProp);
        XDocumentDataSource xDs = UnoRuntime.queryInterface(XDocumentDataSource.class, xQuerySup);

        XStorable xStore = UnoRuntime.queryInterface(XStorable.class,xDs.getDatabaseDocument());
        xStore.store();
    }

    // prints all column names from Query1
    public static void printQueryColumnNames() throws com.sun.star.uno.Exception
    {
        XNameAccess xNameAccess = UnoRuntime.queryInterface(
            XNameAccess.class,
            xMCF.createInstanceWithContext("com.sun.star.sdb.DatabaseContext",
                                           xContext));
        // we use the first datasource
        XDataSource xDS = UnoRuntime.queryInterface(
            XDataSource.class, xNameAccess.getByName( "Bibliography" ));
        XConnection con = xDS.getConnection("","");
        XQueriesSupplier xQuerySup = UnoRuntime.queryInterface(XQueriesSupplier.class, con);

        XNameAccess xQDefs = xQuerySup.getQueries();

        XColumnsSupplier xColsSup = UnoRuntime.queryInterface(
            XColumnsSupplier.class,xQDefs.getByName("Query1"));
        XNameAccess xCols = xColsSup.getColumns();
        String aNames [] = xCols.getElementNames();
        for(int i=0;i<aNames.length;++i)
            System.out.println(aNames[i]);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
