/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.beans.XPropertySet;



/*
 * OpenQuery.java
 *
 * Created on 6. Juli 2002, 10:25
 */

/**
 *
 * @author  dschulten
 */
public class OpenQuery {

    private XComponentContext xContext = null;
    private XMultiComponentFactory xMCF = null;

    /** Creates a new instance of OpenQuery */
    public OpenQuery() {
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        OpenQuery openQuery1 = new OpenQuery();
        try {
            openQuery1.openQuery();
        }
        catch (java.lang.Exception e){
            e.printStackTrace();
        }
        finally {
            System.exit(0);
        }
    }

    protected void openQuery() throws com.sun.star.uno.Exception, java.lang.Exception {
        try {
            // get the remote office component context
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
            xMCF = xContext.getServiceManager();
        }
        catch( Exception e) {
            System.err.println("ERROR: can't get a component context from a running office ...");
            e.printStackTrace();
            System.exit(1);
        }

        // first we create our RowSet object and get its XRowSet interface
        Object rowSet = xMCF.createInstanceWithContext(
            "com.sun.star.sdb.RowSet", xContext);

        com.sun.star.sdbc.XRowSet xRowSet = (com.sun.star.sdbc.XRowSet)
            UnoRuntime.queryInterface(com.sun.star.sdbc.XRowSet.class, rowSet);

        // set the properties needed to connect to a database
        XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xRowSet);

        // the DataSourceName can be a data source registered with [PRODUCTNAME], among other possibilities
        xProp.setPropertyValue("DataSourceName","Bibliography");

        // the CommandType must be TABLE, QUERY or COMMAND, here we use COMMAND
        xProp.setPropertyValue("CommandType",new Integer(com.sun.star.sdb.CommandType.COMMAND));

        // the Command could be a table or query name or a SQL command, depending on the CommandType
        xProp.setPropertyValue("Command","SELECT IDENTIFIER, AUTHOR FROM biblio ORDER BY IDENTIFIER");

        // if your database requires logon, you can use the properties User and Password
        // xProp.setPropertyValue("User", "JohnDoe");
        // xProp.setPropertyValue("Password", "mysecret");

        xRowSet.execute();

        // prepare the XRow and XColumnLocate interface for column access
        // XRow gets column values
        com.sun.star.sdbc.XRow xRow = (com.sun.star.sdbc.XRow)UnoRuntime.queryInterface(
            com.sun.star.sdbc.XRow.class, xRowSet);
        // XColumnLocate finds columns by name
        com.sun.star.sdbc.XColumnLocate xLoc = (com.sun.star.sdbc.XColumnLocate)
            UnoRuntime.queryInterface(
                com.sun.star.sdbc.XColumnLocate.class, xRowSet);

        // print output header
        System.out.println("Identifier\tAuthor");
        System.out.println("----------\t------");

        // output result rows
        while ( xRowSet != null && xRowSet.next() ) {
            String ident = xRow.getString(xLoc.findColumn("IDENTIFIER"));
            String author = xRow.getString(xLoc.findColumn("AUTHOR"));
            System.out.println(ident + "\t\t" + author);
        }

        // XResultSetUpdate for insertRow handling
        com.sun.star.sdbc.XResultSetUpdate xResultSetUpdate = (com.sun.star.sdbc.XResultSetUpdate)
            UnoRuntime.queryInterface(
                com.sun.star.sdbc.XResultSetUpdate.class, xRowSet);

        // XRowUpdate for row updates
        com.sun.star.sdbc.XRowUpdate xRowUpdate = (com.sun.star.sdbc.XRowUpdate)
            UnoRuntime.queryInterface(
                com.sun.star.sdbc.XRowUpdate.class, xRowSet);

        // move to insertRow buffer
        xResultSetUpdate.moveToInsertRow();

        // edit insertRow buffer
        xRowUpdate.updateString(xLoc.findColumn("IDENTIFIER"), "GOF95");
        xRowUpdate.updateString(xLoc.findColumn("AUTHOR"), "Gamma, Helm, Johnson, Vlissides");

        // write buffer to database
        xResultSetUpdate.insertRow();

        // throw away the row set
        com.sun.star.lang.XComponent xComp = (com.sun.star.lang.XComponent)UnoRuntime.queryInterface(
            com.sun.star.lang.XComponent.class, xRowSet);
        xComp.dispose();
    }

}
