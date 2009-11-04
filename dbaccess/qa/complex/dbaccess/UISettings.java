/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Parser.java,v $
 * $Revision: 1.1.6.2 $
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

import com.sun.star.awt.FontSlant;
import com.sun.star.awt.TextAlign;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.form.runtime.XFormController;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.sdb.application.DatabaseObject;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import connectivity.tools.CRMDatabase;

public class UISettings extends TestCase
{
    // --------------------------------------------------------------------------------------------------------
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkTableFormattingPersistence",
            "checkTransparentQueryColumnSettings"
        };
    }

    // --------------------------------------------------------------------------------------------------------
    public String getTestObjectName()
    {
        return "UISettings";
    }

    // --------------------------------------------------------------------------------------------------------
    private CRMDatabase impl_createConnectedDatabaseDocument() throws Exception
    {
        final CRMDatabase database = new CRMDatabase( getORB() );

        // load the document
        String docURL = database.getDatabase().getDocumentURL();
        XModel doc = loadDocument( docURL );

        // establish the connection
        XDatabaseDocumentUI docUI = UnoRuntime.queryInterface( XDatabaseDocumentUI.class,
            doc.getCurrentController() );
        docUI.connect();

        return database;
    }

    // --------------------------------------------------------------------------------------------------------
    /** verifies that aliases for inner queries work as expected
     */
    public void checkTableFormattingPersistence() throws java.lang.Exception
    {
        // create, load, and connect a DB doc
        CRMDatabase database = impl_createConnectedDatabaseDocument();

        // display a table
        XFormController tableViewController = UnoRuntime.queryInterface( XFormController.class,
            database.loadSubComponent( DatabaseObject.TABLE, "customers" ) );
        XPropertySet tableControlModel = UnoRuntime.queryInterface( XPropertySet.class,
            tableViewController.getCurrentControl().getModel() );

        // change the table's formatting
        tableControlModel.setPropertyValue( "FontName", "Andale Sans UI" );
        tableControlModel.setPropertyValue( "FontHeight", Float.valueOf( 20 ) );
        tableControlModel.setPropertyValue( "FontSlant", FontSlant.ITALIC );

        String docURL = database.getDatabase().getModel().getURL();

        // save close the database document
        database.saveAndClose();

        // load a copy of the document
        // normally, it should be sufficient to load the same doc. However, there might be objects in the Java VM
        // which are not yet freed, and which effectively hold the document alive. More precise: The document (|doc|)
        // is certainly disposed, but other objects might hold a reference to one of the many other components
        // around the database document, the data source, the connection, etc. As long as those objects are
        // not cleaned up, the "database model impl" - the structure holding all document data - will
        // stay alive, and subsequent requests to load the doc will just reuse it, without really loading it.
        docURL = copyToTempFile( docURL );
        loadDocument( docURL );
        database = new CRMDatabase( getORB(), docURL );

        // display the table, again
        tableViewController = UnoRuntime.queryInterface( XFormController.class,
            database.loadSubComponent( DatabaseObject.TABLE, "customers" ) );
        tableControlModel = UnoRuntime.queryInterface( XPropertySet.class,
            tableViewController.getCurrentControl().getModel() );

        // verify the properties
        assureEquals( "wrong font name", "Andale Sans UI", (String)tableControlModel.getPropertyValue( "FontName" ) );
        assureEquals( "wrong font height", (float)20, ((Float)tableControlModel.getPropertyValue( "FontHeight" )).floatValue() );
        assureEquals( "wrong font slant", FontSlant.ITALIC, (FontSlant)tableControlModel.getPropertyValue( "FontSlant" ) );

        // close the doc
        database.saveAndClose();
    }

    /**
     * checks whether query columns use the settings of the underlying table column, if they do not (yet) have own
     * settings
     * @throws java.lang.Exception
     */
    public void checkTransparentQueryColumnSettings() throws java.lang.Exception
    {
        // create, load, and connect a DB doc
        CRMDatabase database = impl_createConnectedDatabaseDocument();

        // display a table
        XController tableView = database.loadSubComponent( DatabaseObject.TABLE, "customers" );
        XFormController tableViewController = UnoRuntime.queryInterface( XFormController.class,
             tableView );
        XNameAccess tableControlModel = UnoRuntime.queryInterface( XNameAccess.class,
            tableViewController.getCurrentControl().getModel() );

        // change the formatting of a table column
        XPropertySet idColumn = UnoRuntime.queryInterface( XPropertySet.class, tableControlModel.getByName( "ID" ) );
        assure( "precondition not met: column already centered",
            ((Short)idColumn.getPropertyValue( "Align" )).shortValue() != TextAlign.CENTER );
        idColumn.setPropertyValue( "Align", TextAlign.CENTER );

        // close the table data view
        XCloseable closeSubComponent = UnoRuntime.queryInterface( XCloseable.class, tableView.getFrame() );
        closeSubComponent.close( true );

        // create a query based on that column
        database.getDatabase().getDataSource().createQuery( "q_customers", "SELECT * FROM \"customers\"" );

        // load this query, and verify the table column settings was propagated to the query column
        XFormController queryViewController = UnoRuntime.queryInterface( XFormController.class,
            database.loadSubComponent( DatabaseObject.QUERY, "q_customers" ) );
        tableControlModel = UnoRuntime.queryInterface( XNameAccess.class,
            queryViewController.getCurrentControl().getModel() );
        idColumn = UnoRuntime.queryInterface( XPropertySet.class, tableControlModel.getByName( "ID" ) );

        assure( "table column alignment was not propagated to the query column",
            ((Short)idColumn.getPropertyValue( "Align" )).shortValue() == TextAlign.CENTER );

        // save close the database document
        database.saveAndClose();
    }
}
