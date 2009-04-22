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
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.form.XFormController;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import helper.URLHelper;
import java.io.File;
import java.net.URI;

public class UISettings extends TestCase
{
    // --------------------------------------------------------------------------------------------------------
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkTableFormattingPersistence"
        };
    }

    // --------------------------------------------------------------------------------------------------------
    public String getTestObjectName()
    {
        return "UISettings";
    }

    // --------------------------------------------------------------------------------------------------------
    /** verifies that aliases for inner queries work as expected
     */
    public void checkTableFormattingPersistence() throws java.lang.Exception
    {
        CRMDatabase database = new CRMDatabase( getORB() );

        // load the document
        String docURL = database.getDatabase().getDocumentURL();
        XComponentLoader loader = (XComponentLoader)UnoRuntime.queryInterface( XComponentLoader.class,
            getORB().createInstance( "com.sun.star.frame.Desktop" ) );
        XModel doc = (XModel)UnoRuntime.queryInterface( XModel.class,
            loader.loadComponentFromURL( docURL, "_blank", 0, new PropertyValue[] {} ) );

        // establish the connection
        XDatabaseDocumentUI docUI = (XDatabaseDocumentUI)UnoRuntime.queryInterface( XDatabaseDocumentUI.class,
            doc.getCurrentController() );
        docUI.connect();

        // display the table
        XComponent tableViewComp = docUI.loadComponent( com.sun.star.sdb.application.DatabaseObject.TABLE, "customers", false );
        XFormController tableViewController = (XFormController)UnoRuntime.queryInterface( XFormController.class,
            tableViewComp );
        XPropertySet tableControlModel = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class,
            tableViewController.getCurrentControl().getModel() );

        // change the table's formatting
        tableControlModel.setPropertyValue( "FontName", "Andale Sans UI" );
        tableControlModel.setPropertyValue( "FontHeight", new Float( 20 ) );
        tableControlModel.setPropertyValue( "FontSlant", FontSlant.ITALIC );

        // close the table
        docUI.closeSubComponents();

        // save and close the database document
        database.getDatabase().store();
        database.close();

        // load a copy of the document
        // normally, it should be sufficient to load the same doc. However, there might be objects in the Java VM
        // which are not yet freed, and which effectively hold the document alive. More precise: The document (|doc|)
        // is certainly disposed, but other objects might hold a reference to one of the many other components
        // around the database document, the data source, the connection, etc. As long as those objects are
        // not cleaned up, the "database model impl" - the structure holding all document data - will
        // stay alive, and subsequent requests to load the doc will just reuse it, without really loading it.
        docURL = copyToTempFile( docURL );
        doc = (XModel)UnoRuntime.queryInterface( XModel.class,
            loader.loadComponentFromURL( docURL, "_blank", 0, new PropertyValue[] {} ) );

        docUI = (XDatabaseDocumentUI)UnoRuntime.queryInterface( XDatabaseDocumentUI.class,
            doc.getCurrentController() );
        docUI.connect();

        // display the table, again
        tableViewComp = docUI.loadComponent( com.sun.star.sdb.application.DatabaseObject.TABLE, "customers", false );
        tableViewController = (XFormController)UnoRuntime.queryInterface( XFormController.class,
            tableViewComp );
        tableControlModel = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class,
            tableViewController.getCurrentControl().getModel() );

        // verify the properties
        assureEquals( "wrong font name", "Andale Sans UI", (String)tableControlModel.getPropertyValue( "FontName" ) );
        assureEquals( "wrong font height", (float)20, ((Float)tableControlModel.getPropertyValue( "FontHeight" )).floatValue() );
        assureEquals( "wrong font slant", FontSlant.ITALIC, (FontSlant)tableControlModel.getPropertyValue( "FontSlant" ) );

        // close the doc
        docUI.closeSubComponents();
        XCloseable closeDoc = (XCloseable)UnoRuntime.queryInterface( XCloseable.class,
            doc );
        closeDoc.close( true );
    }
}
