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

import com.sun.star.lang.XComponent;
import com.sun.star.uno.*;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.sdbc.*;
import com.sun.star.sdbcx.*;
import com.sun.star.lang.XMultiServiceFactory;

public class sdbcx
{
    private final XMultiServiceFactory xORB;
    private static XConnection con;
    private XTablesSupplier xTabSup;

    public static XMultiServiceFactory rSmgr;

    public static void main(String argv[]) throws java.lang.Exception
    {
        try{
            rSmgr = connect("socket,host=localhost,port=8100");
                        sdbcx test = new sdbcx(rSmgr);
                        test.createConnection();
                        test.displayTableProperties();
                        // now we dispose the connection to close it
                        XComponent xComponent = UnoRuntime.queryInterface(XComponent.class,con);
                        if(xComponent != null)
                        {
                                xComponent.dispose();
                                System.out.println("Connection disposed!");
                        }
                }
        catch(com.sun.star.uno.Exception e)
        {
            System.out.println(e);
            e.printStackTrace();
        }
        System.exit(0);
        }
        public static XMultiServiceFactory connect( String connectStr )
        throws com.sun.star.uno.Exception,
        com.sun.star.uno.RuntimeException, java.lang.Exception
    {
        // initial serviceManager
        XMultiServiceFactory xLocalServiceManager =
            com.sun.star.comp.helper.Bootstrap.createSimpleServiceManager();

        // create a connector, so that it can contact the office
        Object  xUrlResolver  = xLocalServiceManager.createInstance( "com.sun.star.bridge.UnoUrlResolver" );
        XUnoUrlResolver urlResolver = UnoRuntime.queryInterface(
            XUnoUrlResolver.class, xUrlResolver );

        Object rInitialObject = urlResolver.resolve( "uno:" + connectStr + ";urp;StarOffice.NamingService" );

        XNamingService rName = UnoRuntime.queryInterface(
            XNamingService.class, rInitialObject );

        XMultiServiceFactory xMSF = null;
        if( rName != null ) {
            System.err.println( "got the remote naming service !" );
            Object rXsmgr = rName.getRegisteredObject("StarOffice.ServiceManager" );

            xMSF = UnoRuntime.queryInterface( XMultiServiceFactory.class, rXsmgr );
        }

        return xMSF;
    }


    public sdbcx(XMultiServiceFactory rSmgr )
    {
        xORB = rSmgr;
    }

    public void createConnection() throws com.sun.star.uno.Exception
    {
        // create the Driver with the implementation name
        Object aDriver = xORB.createInstance("org.openoffice.comp.connectivity.pq.Driver.noext");
        // query for the interface
        com.sun.star.sdbc.XDriver xDriver;
        xDriver = UnoRuntime.queryInterface(XDriver.class,aDriver);
        if(xDriver != null)
        {
            // first create the needed url
            String URL = "sdbc:postgresql:dbname=MYDB0";
            // second create the necessary properties
            com.sun.star.beans.PropertyValue [] Props = new com.sun.star.beans.PropertyValue[]
            {
                new com.sun.star.beans.PropertyValue("user",0,"test1",com.sun.star.beans.PropertyState.DIRECT_VALUE),
                new com.sun.star.beans.PropertyValue("password",0,"test1",com.sun.star.beans.PropertyState.DIRECT_VALUE)
            };

            // now create a connection to the database
            con = xDriver.connect(URL, Props);
            if(con != null)
            {
                System.out.println("Connection could be created!");
                // we the XDatabaseDefinitionSupplier interface from the driver to get the XTablesSupplier
                XDataDefinitionSupplier xDDSup = UnoRuntime.queryInterface(
                        XDataDefinitionSupplier.class,xDriver);
                if(xDDSup != null)
                {
                    xTabSup = xDDSup.getDataDefinitionByConnection(con);
                    if(xTabSup != null)
                    {
                        XNameAccess xTables = xTabSup.getTables();
                        // now print all table names
                        System.out.println("Tables available:");
                        String [] aTableNames = xTables.getElementNames();
                        for ( int i =0; i<= aTableNames.length-1; i++)
                            System.out.println(aTableNames[i]);
                    }
                }
                else
                    System.out.println("The driver is not a SDBCX capable!");
            }
            else
                System.out.println("Connection could not be created!");
        }
    }

    public void displayTableProperties() throws com.sun.star.uno.Exception
    {
        XNameAccess xTables = xTabSup.getTables();
        String [] aTableNames = xTables.getElementNames();
        if(0 != aTableNames.length)
        {
            Object table = xTables.getByName(aTableNames[0]);
            XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class,table);
            System.out.println("Name:          " + xProp.getPropertyValue("Name"));
            System.out.println("CatalogName:   " + xProp.getPropertyValue("CatalogName"));
            System.out.println("SchemaName:    " + xProp.getPropertyValue("SchemaName"));
            System.out.println("Description:   " + xProp.getPropertyValue("Description"));
            // the following property is optional so we first must check if it exists
            if(xProp.getPropertySetInfo().hasPropertyByName("Type"))
                System.out.println("Type:          " + xProp.getPropertyValue("Type"));
        }
    }


    // 15. example
    // print all columns of a XColumnsSupplier

    public static void printColumns(XColumnsSupplier xColumnsSup)
    {
        System.out.println("Example printColumns");
        // the table must be at least support a XColumnsSupplier interface
        System.out.println("--- Columns ---");
        XNameAccess xColumns = xColumnsSup.getColumns();
        String [] aColumnNames = xColumns.getElementNames();
        for ( int i =0; i<= aColumnNames.length-1; i++)
            System.out.println("    " + aColumnNames[i]);
    }

    // 16. example
    // print all keys inclusive the columns of a key

    public static void printKeys(XColumnsSupplier xColumnsSup) throws com.sun.star.uno.Exception
    {
        System.out.println("Example printKeys");
        XKeysSupplier xKeysSup = UnoRuntime.queryInterface(XKeysSupplier.class,xColumnsSup);
        if(xKeysSup != null)
        {
            System.out.println("--- Keys ---");
            XIndexAccess xKeys = xKeysSup.getKeys();
            for ( int i =0; i < xKeys.getCount(); i++)
            {
                Object key = xKeys.getByIndex(i);
                XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class,key);
                System.out.println("    " + xProp.getPropertyValue("Name"));
                XColumnsSupplier xKeyColumnsSup = UnoRuntime.queryInterface(XColumnsSupplier.class,xProp);
                printColumns(xKeyColumnsSup);
            }
        }
    }

    // 17. example
    // print all keys inclusive the columns of a key

    public static void printIndexes(XColumnsSupplier xColumnsSup) throws com.sun.star.uno.Exception
    {
        System.out.println("Example printIndexes");
        XIndexesSupplier xIndexesSup = UnoRuntime.queryInterface(XIndexesSupplier.class,xColumnsSup);
        if(xIndexesSup != null)
        {
            System.out.println("--- Indexes ---");
            XNameAccess xIndexs = xIndexesSup.getIndexes();
            String [] aIndexNames = xIndexs.getElementNames();
            for ( int i =0; i<= aIndexNames.length-1; i++)
            {
                System.out.println("    " + aIndexNames[i]);
                Object index = xIndexs.getByName(aIndexNames[i]);
                XColumnsSupplier xIndexColumnsSup = UnoRuntime.queryInterface(XColumnsSupplier.class,index);
                printColumns(xIndexColumnsSup);
            }
        }
    }


    // 18. example
    // column properties

    public static void printColumnProperties(Object column) throws com.sun.star.uno.Exception
    {
        System.out.println("Example printColumnProperties");
        XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class,column);
        System.out.println("Name:            " + xProp.getPropertyValue("Name"));
        System.out.println("Type:            " + xProp.getPropertyValue("Type"));
        System.out.println("TypeName:        " + xProp.getPropertyValue("TypeName"));
        System.out.println("Precision:       " + xProp.getPropertyValue("Precision"));
        System.out.println("Scale:           " + xProp.getPropertyValue("Scale"));
        System.out.println("IsNullable:      " + xProp.getPropertyValue("IsNullable"));
        System.out.println("IsAutoIncrement: " + xProp.getPropertyValue("IsAutoIncrement"));
        System.out.println("IsCurrency:      " + xProp.getPropertyValue("IsCurrency"));
        // the following property is optional so we first must check if it exists
        if(xProp.getPropertySetInfo().hasPropertyByName("IsRowVersion"))
            System.out.println("IsRowVersion:    " + xProp.getPropertyValue("IsRowVersion"));
        if(xProp.getPropertySetInfo().hasPropertyByName("Description"))
            System.out.println("Description:     " + xProp.getPropertyValue("Description"));
        if(xProp.getPropertySetInfo().hasPropertyByName("DefaultValue"))
            System.out.println("DefaultValue:    " + xProp.getPropertyValue("DefaultValue"));
    }


    // 19. example
    // index properties

    public static void printIndexProperties(Object index) throws com.sun.star.uno.Exception
    {
        System.out.println("Example printIndexProperties");
        XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class,index);
        System.out.println("Name:              " + xProp.getPropertyValue("Name"));
        System.out.println("Catalog:           " + xProp.getPropertyValue("Catalog"));
        System.out.println("IsUnique:          " + xProp.getPropertyValue("IsUnique"));
        System.out.println("IsPrimaryKeyIndex: " + xProp.getPropertyValue("IsPrimaryKeyIndex"));
        System.out.println("IsClustered:       " + xProp.getPropertyValue("IsClustered"));
    }


    // 20. example
    // key properties

    public static void printKeyProperties(Object key) throws com.sun.star.uno.Exception
    {
        System.out.println("Example printKeyProperties");
        XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class,key);
        System.out.println("Name:            " + xProp.getPropertyValue("Name"));
        System.out.println("Type:            " + xProp.getPropertyValue("Type"));
        System.out.println("ReferencedTable: " + xProp.getPropertyValue("ReferencedTable"));
        System.out.println("UpdateRule:      " + xProp.getPropertyValue("UpdateRule"));
        System.out.println("DeleteRule:      " + xProp.getPropertyValue("DeleteRule"));
    }


    // 21. example
    // print all groups and the users with their privileges who belong to this group

    public static void printGroups(XTablesSupplier xTabSup) throws com.sun.star.uno.Exception,SQLException
    {
        System.out.println("Example printGroups");
        XGroupsSupplier xGroupsSup = UnoRuntime.queryInterface(XGroupsSupplier.class,xTabSup);
        if(xGroupsSup != null)
        {
            // the table must be at least support a XColumnsSupplier interface
            System.out.println("--- Groups ---");
            XNameAccess xGroups = xGroupsSup.getGroups();
            String [] aGroupNames = xGroups.getElementNames();
            for ( int i =0; i < aGroupNames.length; i++)
            {
                System.out.println("    " + aGroupNames[i]);
                XUsersSupplier xUsersSup = UnoRuntime.queryInterface(XUsersSupplier.class,xGroups.getByName(aGroupNames[i]));
                if(xUsersSup != null)
                {
                    XAuthorizable xAuth = UnoRuntime.queryInterface(XAuthorizable.class,xUsersSup);
                    // the table must be at least support a XColumnsSupplier interface
                    System.out.println("\t--- Users ---");
                    XNameAccess xUsers = xUsersSup.getUsers();
                    String [] aUserNames = xUsers.getElementNames();
                    for ( int j =0; j < aUserNames.length; j++)
                    {
                        System.out.println("\t    " + aUserNames[j] + " Privileges: " + xAuth.getPrivileges(aUserNames[j],PrivilegeObject.TABLE));
                    }
                }
            }
        }
    }


    // 22. example
    // create the table salesmen

    public static void createTableSalesMen(XNameAccess xTables) throws com.sun.star.uno.Exception,SQLException
    {
        System.out.println("Example createTableSalesMen");
        XDataDescriptorFactory xTabFac = UnoRuntime.queryInterface(XDataDescriptorFactory.class,xTables);
        if(xTabFac != null)
        {
            // create the new table
            XPropertySet xTable = xTabFac.createDataDescriptor();
            // set the name of the new table
            xTable.setPropertyValue("Name","SALESMAN");
            // append the columns
            XColumnsSupplier xColumSup = UnoRuntime.queryInterface(XColumnsSupplier.class,xTable);
            XDataDescriptorFactory xColFac = UnoRuntime.queryInterface(XDataDescriptorFactory.class,xColumSup.getColumns());
            XAppend xAppend = UnoRuntime.queryInterface(XAppend.class,xColFac);
            // we only need one descriptor
            XPropertySet xCol = xColFac.createDataDescriptor();
            // create first column and append
            xCol.setPropertyValue("Name","SNR");
            xCol.setPropertyValue("Type",Integer.valueOf(DataType.INTEGER));
            xCol.setPropertyValue("IsNullable",Integer.valueOf(ColumnValue.NO_NULLS));
            xAppend.appendByDescriptor(xCol);
            // 2nd only set the properties which differs
            xCol.setPropertyValue("Name","FIRSTNAME");
            xCol.setPropertyValue("Type",Integer.valueOf(DataType.VARCHAR));
            xCol.setPropertyValue("IsNullable",Integer.valueOf(ColumnValue.NULLABLE));
            xCol.setPropertyValue("Precision",Integer.valueOf(50));
            xAppend.appendByDescriptor(xCol);
            // 3nd only set the properties which differs
            xCol.setPropertyValue("Name","LASTNAME");
            xCol.setPropertyValue("Precision",Integer.valueOf(100));
            xAppend.appendByDescriptor(xCol);
            // 4nd only set the properties which differs
            xCol.setPropertyValue("Name","STREET");
            xCol.setPropertyValue("Precision",Integer.valueOf(50));
            xAppend.appendByDescriptor(xCol);
            // 5nd only set the properties which differs
            xCol.setPropertyValue("Name","STATE");
            xAppend.appendByDescriptor(xCol);
            // 6nd only set the properties which differs
            xCol.setPropertyValue("Name","ZIP");
            xCol.setPropertyValue("Type",Integer.valueOf(DataType.INTEGER));
            xCol.setPropertyValue("Precision",Integer.valueOf(10)); // default value integer
            xAppend.appendByDescriptor(xCol);
            // 7nd only set the properties which differs
            xCol.setPropertyValue("Name","BIRTHDATE");
            xCol.setPropertyValue("Type",Integer.valueOf(DataType.DATE));
            xCol.setPropertyValue("Precision",Integer.valueOf(10)); // default value integer
            xAppend.appendByDescriptor(xCol);
            // now we create the primary key
            XKeysSupplier xKeySup = UnoRuntime.queryInterface(XKeysSupplier.class,xTable);
            XDataDescriptorFactory xKeyFac = UnoRuntime.queryInterface(XDataDescriptorFactory.class,xKeySup.getKeys());
            XAppend xKeyAppend = UnoRuntime.queryInterface(XAppend.class,xKeyFac);
            XPropertySet xKey = xKeyFac.createDataDescriptor();
            xKey.setPropertyValue("Type",Integer.valueOf(KeyType.PRIMARY));
            // now append the columns to key
            XColumnsSupplier xKeyColumSup = UnoRuntime.queryInterface(XColumnsSupplier.class,xKey);
            XDataDescriptorFactory xKeyColFac = UnoRuntime.queryInterface(XDataDescriptorFactory.class,xKeyColumSup.getColumns());
            XAppend xKeyColAppend = UnoRuntime.queryInterface(XAppend.class,xKeyColFac);
            // we only need one descriptor
            XPropertySet xKeyCol = xKeyColFac.createDataDescriptor();
            xKeyCol.setPropertyValue("Name","SNR");
            // append the key column
            xKeyColAppend.appendByDescriptor(xKeyCol);
            // append the key
            xKeyAppend.appendByDescriptor(xKey);
            // the last step is to append the new table to the tables collection
             XAppend xTableAppend = UnoRuntime.queryInterface(XAppend.class,xTabFac);
             xTableAppend.appendByDescriptor(xTable);
        }
    }


    // 23. example
    // create a user

    public static void createUser(XNameAccess xUsers) throws com.sun.star.uno.Exception,SQLException
    {
        System.out.println("Example createUser");
        XDataDescriptorFactory xUserFac = UnoRuntime.queryInterface(XDataDescriptorFactory.class,xUsers);
        if(xUserFac != null)
        {
            // create the new table
            XPropertySet xUser = xUserFac.createDataDescriptor();
            // set the name of the new table
            xUser.setPropertyValue("Name","BOSS");
            xUser.setPropertyValue("Password","BOSSWIFENAME");
            XAppend xAppend = UnoRuntime.queryInterface(XAppend.class,xUserFac);
            xAppend.appendByDescriptor(xUser);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
