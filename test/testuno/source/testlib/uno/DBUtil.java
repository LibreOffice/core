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

package testlib.uno;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyState;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XCloseable;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbcx.XAppend;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.CloseVetoException;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;
import java.io.File;

import org.openoffice.test.common.FileUtil;


public class DBUtil {
    // the service factory
    protected static XMultiServiceFactory m_orb;
    // the URL of the temporary file used for the database document
    protected static String m_databaseDocumentFile;
    // the database document
    protected static XOfficeDatabaseDocument m_databaseDocument;
    // the data source belonging to the database document
    protected static XDataSource m_dataSource;
    // the default connection
    protected static XConnection m_connection;


    static public void createNewDocument(final XMultiServiceFactory orb)
            throws Exception {
        m_orb = orb;
        createDBDocument();
    }

    static public void loadNewDocument(final XMultiServiceFactory orb,
            final String _existingDocumentURL) throws Exception {
        m_orb = orb;
        getDocument(_existingDocumentURL);
    }

    /**
     * creates an empty database document in a temporary location
     */
    public static void createDBDocument() throws Exception {
        final File documentFile = File.createTempFile("testdb", ".odb");
        if (documentFile.exists())
            documentFile.delete();
        m_databaseDocumentFile = FileUtil.getUrl(documentFile);
        m_databaseDocument = (XOfficeDatabaseDocument) UnoRuntime
                .queryInterface(
                        XOfficeDatabaseDocument.class,
                        m_orb.createInstance("com.sun.star.sdb.OfficeDatabaseDocument"));
        m_dataSource = m_databaseDocument.getDataSource();

        final XPropertySet dsProperties = (XPropertySet) UnoRuntime
                .queryInterface(XPropertySet.class,
                        m_databaseDocument.getDataSource());
        dsProperties.setPropertyValue("URL", "sdbc:embedded:hsqldb");

        final XStorable storable = (XStorable) UnoRuntime.queryInterface(
                XStorable.class, m_databaseDocument);
        storable.storeAsURL(m_databaseDocumentFile,
                new PropertyValue[] { new PropertyValue("PickListEntry", 0,
                        false, PropertyState.DIRECT_VALUE) });
    }


    public static void getDocument(final String _docURL) throws Exception {
        m_databaseDocumentFile = _docURL;

        final XNameAccess dbContext = UnoRuntime.queryInterface(
                XNameAccess.class,
                m_orb.createInstance("com.sun.star.sdb.DatabaseContext"));
        final XDocumentDataSource dataSource = UnoRuntime.queryInterface(
                XDocumentDataSource.class, dbContext.getByName(_docURL));

        m_databaseDocument = dataSource.getDatabaseDocument();
        m_dataSource = m_databaseDocument.getDataSource();
    }

    /**
     * drops the table with a given name
     *
     * @param _name
     *            the name of the table to drop
     * @param _ifExists
     *            TRUE if it should be dropped only when it exists.
     */
    static public void dropTable(final String _name, final boolean _ifExists)
            throws SQLException {
        final StringBuffer dropStatement = new StringBuffer("DROP TABLE \"");
        dropStatement.append(_name);
        if (_ifExists) {
            dropStatement.append("\" IF EXISTS");
        }
        executeSQL(dropStatement.toString());
    }

    static public void createTable(String _name,
            HsqlColumnDescriptor[] _columns, final boolean _dropIfExists)
            throws SQLException {
        if (_dropIfExists) {
            dropTable(_name, true);
        }
        createTable(_name, _columns);
    }

    /**
     * creates a table
     */
    static public void createTable(String _name, HsqlColumnDescriptor[] _columns)
            throws SQLException {
        StringBuffer createStatement = new StringBuffer(
                "CREATE CACHED TABLE \"");
        createStatement.append(_name);
        createStatement.append("\" ( ");

        String primaryKeyList = "";

        final HashMap foreignKeys = new HashMap();
        final HashMap foreignKeyRefs = new HashMap();

        final HsqlColumnDescriptor[] columns = _columns;
        for (int i = 0; i < columns.length; ++i) {
            if (i > 0) {
                createStatement.append(", ");
            }

            createStatement.append("\"" + columns[i].getName());
            createStatement.append("\" " + columns[i].getTypeName());

            if (columns[i].isRequired()) {
                createStatement.append(" NOT NULL");
            }

            if (columns[i].isPrimaryKey()) {
                if (primaryKeyList.length() > 0) {
                    primaryKeyList += ", ";
                }
                primaryKeyList += "\"" + columns[i].getName() + "\"";
            }

            if (columns[i].isForeignKey()) {
                final String foreignTable = columns[i].getForeignTable();

                String foreignKeysForTable = foreignKeys
                        .containsKey(foreignTable) ? (String) foreignKeys
                        .get(foreignTable) : "";
                if (foreignKeysForTable.length() > 0) {
                    foreignKeysForTable += ", ";
                }
                foreignKeysForTable += "\"" + columns[i].getName() + "\"";
                foreignKeys.put(foreignTable, foreignKeysForTable);

                final StringBuffer foreignKeyRefsForTable = new StringBuffer(
                        foreignKeyRefs.containsKey(foreignTable) ? (String) foreignKeyRefs
                                .get(foreignTable) : "");
                if (foreignKeyRefsForTable.length() > 0) {
                    foreignKeyRefsForTable.append(", ");
                }
                foreignKeyRefsForTable.append("\""
                        + columns[i].getForeignColumn() + "\"");
                foreignKeyRefs.put(foreignTable,
                        foreignKeyRefsForTable.toString());
            }
        }

        if (primaryKeyList.length() > 0) {
            createStatement.append(", PRIMARY KEY (");
            createStatement.append(primaryKeyList);
            createStatement.append(')');
        }

        final Set foreignKeyTables = foreignKeys.keySet();
        for (final Iterator foreignKey = foreignKeyTables.iterator(); foreignKey
                .hasNext();) {
            final String foreignTable = (String) foreignKey.next();

            createStatement.append(", FOREIGN KEY (");
            createStatement.append((String) foreignKeys.get(foreignTable));
            createStatement.append(") REFERENCES \"");
            createStatement.append(foreignTable);
            createStatement.append("\"(");
            createStatement.append((String) foreignKeyRefs.get(foreignTable));
            createStatement.append(')');
        }

        createStatement.append(')');

        // System.err.println( createStatement );
        executeSQL(createStatement.toString());
    }


    /**
     * executes the given SQL statement via the defaultConnection
     */
    static public void executeSQL(final String statementString)
            throws SQLException {
        final XStatement statement = defaultConnection().createStatement();
        statement.execute(statementString);
    }

    /**
     * returns a connection to the database
     *
     * Multiple calls to this method return the same connection. The
     * DbaseDatabase object keeps the ownership of the connection, so you don't
     * need to (and should not) dispose/close it.
     */
    static public XConnection defaultConnection() throws SQLException {
        if (m_connection == null)
            m_connection = m_databaseDocument.getDataSource().getConnection("",
                    "");

        return m_connection;
    }

    /**
     * closes the database document
     *
     * Any CloseVetoExceptions fired by third parties are ignored, and any
     * reference to the database document is released.
     */
    static public void close() {
        // close connection
        final XCloseable closeConn = UnoRuntime.queryInterface(
                XCloseable.class, m_connection != null ? m_connection : null);
        if (closeConn != null) {
            try {
                closeConn.close();
            } catch (SQLException e) {
            }
        }
        m_connection = null;

        // close document
        final com.sun.star.util.XCloseable closeDoc = UnoRuntime
                .queryInterface(com.sun.star.util.XCloseable.class,
                        m_databaseDocument);
        if (closeDoc != null) {
            try {
                closeDoc.close(true);
            } catch (CloseVetoException e) {
            }
        }
        m_databaseDocument = null;
    }

    /**
     * returns the underlying database document
     */
    static public XOfficeDatabaseDocument getDatabaseDocument() {
        return m_databaseDocument;
    }

    static public String getDocumentURL() {
        return m_databaseDocumentFile;
    }
}
