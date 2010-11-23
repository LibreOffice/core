/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import connectivity.tools.HsqlColumnDescriptor;
import connectivity.tools.HsqlDatabase;
import connectivity.tools.HsqlTableDescriptor;
import java.io.IOException;


// ---------- junit imports -----------------
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;
// ------------------------------------------

/** complex test case for Base's application UI
 */
public class ApplicationController extends TestCase
{

    private HsqlDatabase m_database;
    private XOfficeDatabaseDocument m_databaseDocument;
    private XDatabaseDocumentUI m_documentUI;

    public ApplicationController()
    {
        super();
    }

    // --------------------------------------------------------------------------------------------------------
    public String getTestObjectName()
    {
        return getClass().getName();
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
                ? new HsqlDatabase(getMSF())
                : new HsqlDatabase(getMSF(), _documentURL);
        m_databaseDocument = m_database.getDatabaseDocument();

        // load it into a frame
        final Object object = getMSF().createInstance("com.sun.star.frame.Desktop");
        final XComponentLoader xComponentLoader = UnoRuntime.queryInterface(XComponentLoader.class, object);
        final XComponent loadedComponent = xComponentLoader.loadComponentFromURL(m_database.getDocumentURL(), "_blank", FrameSearchFlag.ALL, new PropertyValue[0]);

        assertTrue("too many document instances!",
                UnoRuntime.areSame(loadedComponent, m_databaseDocument));

        // get the controller, which provides access to various UI operations
        final XModel docModel = UnoRuntime.queryInterface(XModel.class,
                loadedComponent);
        m_documentUI = UnoRuntime.queryInterface(XDatabaseDocumentUI.class,
                docModel.getCurrentController());
    }

    // --------------------------------------------------------------------------------------------------------
    @Before
    @Override
    public void before() throws java.lang.Exception
    {
        super.before();
        impl_switchToDocument(null);
    }

    // --------------------------------------------------------------------------------------------------------
    @After
    @Override
    public void after() throws java.lang.Exception
    {
        impl_closeDocument();
        super.after();
    }
    // --------------------------------------------------------------------------------------------------------

    @Test
    public void checkSaveAs() throws Exception, IOException, java.lang.Exception
    {
        // issue 93737 describes the problem that when you save-as a database document, and do changes to it,
        // then those changes are saved in the old document, actually
        final String oldDocumentURL = m_database.getDocumentURL();

        final String newDocumentURL = createTempFileURL();

        // store the doc in a new location
        final XStorable storeDoc = UnoRuntime.queryInterface( XStorable.class, m_databaseDocument );
        storeDoc.storeAsURL( newDocumentURL, new PropertyValue[] { } );

        // connect
        m_documentUI.connect();
        assertTrue("could not connect to " + m_database.getDocumentURL(), m_documentUI.isConnected());

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
        assertTrue("could not connect to " + m_database.getDocumentURL(), m_documentUI.isConnected());
        XTablesSupplier suppTables = UnoRuntime.queryInterface( XTablesSupplier.class, m_documentUI.getActiveConnection() );
        XNameAccess tables = suppTables.getTables();
        assertTrue("the table was created in the wrong database", !tables.hasByName("abc"));

        // load the new document, and verify there *is* a table therein
        impl_switchToDocument(newDocumentURL);
        m_documentUI.connect();
        assertTrue("could not connect to " + m_database.getDocumentURL(), m_documentUI.isConnected());

        suppTables = UnoRuntime.queryInterface( XTablesSupplier.class, m_documentUI.getActiveConnection() );
        tables = suppTables.getTables();
        assertTrue("the newly created table has not been written", tables.hasByName("abc"));
    }
}
