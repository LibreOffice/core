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

package testcase.uno.db;

import com.sun.star.beans.PropertyValue;
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

import testlib.uno.HsqlColumnDescriptor;
import testlib.uno.DBUtil;

import java.io.IOException;

// ---------- junit imports -----------------
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import static org.junit.Assert.*;

/**
 * test case for Base's application UI
 */
public class DBAccess {
    UnoApp app = new UnoApp();
    private XOfficeDatabaseDocument m_databaseDocument;
    private XDatabaseDocumentUI m_documentUI;

//  public DBAccess() {
//      super();
//  }


    @Before
    public void before() throws java.lang.Exception {
        app.start();
        String a = null;
        switchToDocument(a);
    }

    @After
    public void after() throws java.lang.Exception {
        closeDocument();
        app.close();
    }

    private void closeDocument() {
        DBUtil.close();
        m_databaseDocument = null;
        m_documentUI = null;

    }

    private void switchToDocument(String _documentURL)
            throws java.lang.Exception {
        // close previous database document
        closeDocument();

        if (_documentURL == null) {
            DBUtil.createNewDocument(getMSF());
        } else {
            DBUtil.loadNewDocument(getMSF(), _documentURL);
        }
        m_databaseDocument = DBUtil.getDatabaseDocument();

    }


    @Test
    public void testSaveAs() throws Exception, IOException, java.lang.Exception {

        m_databaseDocument = saveAndReloadDoc(m_databaseDocument, "", "odb");
        XModel docModel = UnoRuntime.queryInterface(XModel.class,
                m_databaseDocument);
        m_documentUI = UnoRuntime.queryInterface(XDatabaseDocumentUI.class,
                docModel.getCurrentController());
        m_documentUI.connect();
        assertTrue("could not connect to " + DBUtil.getDocumentURL(),
                m_documentUI.isConnected());

    }

    @Test
    public void testCreateTable() throws java.lang.Exception {
        // create a table in the database
        DBUtil.createTable("test", new HsqlColumnDescriptor[] {
                new HsqlColumnDescriptor("a", "VARCHAR(50)"),
                new HsqlColumnDescriptor("b", "VARCHAR(50)"),
                new HsqlColumnDescriptor("c", "VARCHAR(50)") });
        switchToDocument(DBUtil.getDocumentURL());
        // ---save and reload database document
        m_databaseDocument = saveAndReloadDoc(m_databaseDocument, "", "odb");

        XModel docModel = UnoRuntime.queryInterface(XModel.class,
                m_databaseDocument);
        m_documentUI = UnoRuntime.queryInterface(XDatabaseDocumentUI.class,
                docModel.getCurrentController());
        m_documentUI.connect();
        XTablesSupplier suppTables = UnoRuntime.queryInterface(
                XTablesSupplier.class, m_documentUI.getActiveConnection());
        XNameAccess tables = suppTables.getTables();
        assertTrue("the newly created table has not been written",
                tables.hasByName("test"));
    }

    protected XMultiServiceFactory getMSF() {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(
                XMultiServiceFactory.class, app.getComponentContext()
                        .getServiceManager());
        return xMSF1;
    }

    private XOfficeDatabaseDocument saveAndReloadDoc(
            XOfficeDatabaseDocument m_databaseDocument2, String sFilter,
            String sExtension) throws java.lang.Exception {
        String filePath = Testspace.getPath("tmp/basetest." + sExtension);
        PropertyValue[] aStoreProperties = new PropertyValue[2];
        aStoreProperties[0] = new PropertyValue();
        aStoreProperties[1] = new PropertyValue();
        aStoreProperties[0].Name = "Override";
        aStoreProperties[0].Value = true;
        aStoreProperties[1].Name = "FilterName";
        XStorable xStorable = (XStorable) UnoRuntime.queryInterface(
                XStorable.class, m_databaseDocument2);
        xStorable.storeToURL(FileUtil.getUrl(filePath), aStoreProperties);

        return UnoRuntime.queryInterface(XOfficeDatabaseDocument.class,
                app.loadDocument(filePath));
    }
}
