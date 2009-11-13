/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ApplicationController.java,v $
 * $Revision: 1.1.2.1 $
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

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import connectivity.tools.HsqlColumnDescriptor;
import connectivity.tools.HsqlDatabase;
import connectivity.tools.HsqlTableDescriptor;
import helper.URLHelper;
import java.io.File;
import java.io.IOException;

/** complex test case for Base's application UI
 */
public class ApplicationController extends complexlib.ComplexTestCase
{

    private HsqlDatabase m_database;
    private XOfficeDatabaseDocument m_databaseDocument;
    private XDatabaseDocumentUI m_documentUI;

    public ApplicationController()
    {
        super();
    }

    // --------------------------------------------------------------------------------------------------------
    protected final XComponentContext getComponentContext()
    {
        XComponentContext context = null;
        try
        {
            final XPropertySet orbProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, getORB());
            context = (XComponentContext) UnoRuntime.queryInterface(XComponentContext.class,
                    orbProps.getPropertyValue("DefaultContext"));
        }
        catch (Exception ex)
        {
            failed("could not retrieve the ComponentContext");
        }
        return context;
    }
    // --------------------------------------------------------------------------------------------------------

    public String[] getTestMethodNames()
    {
        return new String[]
                {
                    "checkSaveAs"
                };
    }

    // --------------------------------------------------------------------------------------------------------
    public String getTestObjectName()
    {
        return getClass().getName();
    }

    // --------------------------------------------------------------------------------------------------------
    protected final XMultiServiceFactory getORB()
    {
        return (XMultiServiceFactory) param.getMSF();
    }

    // --------------------------------------------------------------------------------------------------------
    private void impl_closeDocument()
    {
        if (m_database != null)
        {
            m_database.close();
            m_database = null;
            m_databaseDocument = null;
            m_documentUI = null;
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private void impl_switchToDocument(String _documentURL) throws java.lang.Exception
    {
        // close previous database document
        impl_closeDocument();

        // create/load the new database document
        m_database = (_documentURL == null)
                ? new HsqlDatabase(getORB())
                : new HsqlDatabase(getORB(), _documentURL);
        m_databaseDocument = m_database.getDatabaseDocument();

        // load it into a frame
        final Object object = getORB().createInstance("com.sun.star.frame.Desktop");
        final XComponentLoader xComponentLoader = UnoRuntime.queryInterface(XComponentLoader.class, object);
        final XComponent loadedComponent = xComponentLoader.loadComponentFromURL(m_database.getDocumentURL(), "_blank", FrameSearchFlag.ALL, new PropertyValue[0]);

        assure("too many document instances!",
                UnoRuntime.areSame(loadedComponent, m_databaseDocument));

        // get the controller, which provides access to various UI operations
        final XModel docModel = UnoRuntime.queryInterface(XModel.class,
                loadedComponent);
        m_documentUI = UnoRuntime.queryInterface(XDatabaseDocumentUI.class,
                docModel.getCurrentController());
    }

    // --------------------------------------------------------------------------------------------------------
    public void before() throws Exception, java.lang.Exception
    {
        impl_switchToDocument(null);
    }

    // --------------------------------------------------------------------------------------------------------
    public void after()
    {
        impl_closeDocument();
    }
    // --------------------------------------------------------------------------------------------------------

    public void checkSaveAs() throws Exception, IOException, java.lang.Exception
    {
        // issue 93737 describes the problem that when you save-as a database document, and do changes to it,
        // then those changes are saved in the old document, actually
        final String oldDocumentURL = m_database.getDocumentURL();

        final File documentFile = java.io.File.createTempFile(getTestObjectName(), ".odb");
        documentFile.deleteOnExit();
        final String newDocumentURL = URLHelper.getFileURLFromSystemPath(documentFile.getAbsoluteFile());

        // store the doc in a new location
        final XStorable storeDoc = UnoRuntime.queryInterface( XStorable.class, m_databaseDocument );
        storeDoc.storeAsURL( newDocumentURL, new PropertyValue[] { } );

        // connect
        m_documentUI.connect();
        assure("could not connect to " + m_database.getDocumentURL(), m_documentUI.isConnected());

        // create a table in the database
        m_database.createTable(new HsqlTableDescriptor("abc", new HsqlColumnDescriptor[]
                {
                    new HsqlColumnDescriptor("a", "VARCHAR(50)"),
                    new HsqlColumnDescriptor("b", "VARCHAR(50)"),
                    new HsqlColumnDescriptor("c", "VARCHAR(50)")
                }));

        // load the old document, and verify there is *no* table therein
        impl_switchToDocument(oldDocumentURL);
        m_documentUI.connect();
        assure("could not connect to " + m_database.getDocumentURL(), m_documentUI.isConnected());
        XTablesSupplier suppTables = UnoRuntime.queryInterface( XTablesSupplier.class, m_documentUI.getActiveConnection() );
        XNameAccess tables = suppTables.getTables();
        assure("the table was created in the wrong database", !tables.hasByName("abc"));

        // load the new document, and verify there *is* a table therein
        impl_switchToDocument(newDocumentURL);
        m_documentUI.connect();
        assure("could not connect to " + m_database.getDocumentURL(), m_documentUI.isConnected());

        suppTables = UnoRuntime.queryInterface( XTablesSupplier.class, m_documentUI.getActiveConnection() );
        tables = suppTables.getTables();
        assure("the newly created table has not been written", tables.hasByName("abc"));
    }
}
