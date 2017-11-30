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
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ElementExistException;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbcx.XAppend;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.UnoRuntime;

import helper.URLHelper;
import java.io.File;
import java.util.HashMap;
import java.util.Map;
import org.junit.Assert;

public class HsqlDatabase extends AbstractDatabase
{


    public HsqlDatabase(final XMultiServiceFactory orb) throws Exception
    {
        super(orb);
        createDBDocument();
    }


    public HsqlDatabase(final XMultiServiceFactory orb, final String _existingDocumentURL) throws Exception
    {
        super(orb, _existingDocumentURL);
    }

    /** creates an empty database document in a temporary location
     */
    private void createDBDocument() throws Exception
    {
        Assert.assertNull(m_documentFile);
        m_documentFile = File.createTempFile("testdb", ".odb");
        if ( m_documentFile.exists() )
            m_documentFile.delete();
        m_databaseDocumentFile = URLHelper.getFileURLFromSystemPath(m_documentFile);

        m_databaseDocument = UnoRuntime.queryInterface(
                XOfficeDatabaseDocument.class, m_orb.createInstance("com.sun.star.sdb.OfficeDatabaseDocument"));
        m_dataSource = new DataSource(m_databaseDocument.getDataSource());

        final XPropertySet dsProperties = UnoRuntime.queryInterface(XPropertySet.class, m_databaseDocument.getDataSource());
        dsProperties.setPropertyValue("URL", "sdbc:embedded:hsqldb");

        final XStorable storable = UnoRuntime.queryInterface(XStorable.class, m_databaseDocument);
        storable.storeAsURL( m_databaseDocumentFile, new PropertyValue[]
            {   new PropertyValue( "PickListEntry", 0, false, PropertyState.DIRECT_VALUE )
            } );
    }

    @Override protected final void delete() {
        if (m_documentFile != null) {
            boolean ok = m_documentFile.delete();
            //TODO: fails on Windows: Assert.assertTrue("delete " + m_documentFile.getPath(), ok);
        }
    }

    /** drops the table with a given name

    @param _name
    the name of the table to drop
     */
    private void dropTable(final String _name) throws SQLException
    {
        final StringBuffer dropStatement = new StringBuffer("DROP TABLE \"");
        dropStatement.append(_name);
        dropStatement.append("\" IF EXISTS");
        executeSQL(dropStatement.toString());
    }

    public void createTable(final HsqlTableDescriptor _tableDesc, final boolean _dropIfExists) throws SQLException
    {
        if (_dropIfExists)
        {
            dropTable(_tableDesc.getName());
        }
        createTable(_tableDesc);
    }

    /** creates a table
     */
    public void createTable(final HsqlTableDescriptor _tableDesc) throws SQLException
    {
        StringBuffer createStatement = new StringBuffer("CREATE CACHED TABLE \"");
        createStatement.append(_tableDesc.getName());
        createStatement.append("\" ( ");

        String primaryKeyList = "";

        final HashMap<String, String> foreignKeys = new HashMap<String, String>();
        final HashMap<String, String> foreignKeyRefs = new HashMap<String, String>();

        final HsqlColumnDescriptor[] columns = _tableDesc.getColumns();
        for (int i = 0; i < columns.length; ++i)
        {
            if (i > 0)
            {
                createStatement.append(", ");
            }

            createStatement.append("\"").append(columns[i].getName());
            createStatement.append("\" ").append(columns[i].getTypeName());

            if (columns[i].isRequired())
            {
                createStatement.append(" NOT NULL");
            }

            if (columns[i].isPrimaryKey())
            {
                if (primaryKeyList.length() > 0)
                {
                    primaryKeyList += ", ";
                }
                primaryKeyList += "\"" + columns[i].getName() + "\"";
            }

            if (columns[i].isForeignKey())
            {
                final String foreignTable = columns[i].getForeignTable();

                String foreignKeysForTable = foreignKeys.containsKey(foreignTable) ? foreignKeys.get(foreignTable) : "";
                if (foreignKeysForTable.length() > 0)
                {
                    foreignKeysForTable += ", ";
                }
                foreignKeysForTable += "\"" + columns[i].getName() + "\"";
                foreignKeys.put(foreignTable, foreignKeysForTable);

                final StringBuffer foreignKeyRefsForTable = new StringBuffer(foreignKeyRefs.containsKey(foreignTable) ? foreignKeyRefs.get(foreignTable) : "");
                if (foreignKeyRefsForTable.length() > 0)
                {
                    foreignKeyRefsForTable.append(", ");
                }
                foreignKeyRefsForTable.append("\"").append(columns[i].getForeignColumn()).append("\"");
                foreignKeyRefs.put(foreignTable, foreignKeyRefsForTable.toString());
            }
        }

        if (primaryKeyList.length() > 0)
        {
            createStatement.append(", PRIMARY KEY (");
            createStatement.append(primaryKeyList);
            createStatement.append(')');
        }

        for (Map.Entry<String, String> foreignKey : foreignKeys.entrySet())
        {
            final String foreignTable = foreignKey.getKey();

            createStatement.append(", FOREIGN KEY (");
            createStatement.append(foreignKey.getValue());
            createStatement.append(") REFERENCES \"");
            createStatement.append(foreignTable);
            createStatement.append("\"(");
            createStatement.append(foreignKeyRefs.get(foreignTable));
            createStatement.append(')');
        }

        createStatement.append(')');

        executeSQL(createStatement.toString());
    }

    /** creates a table in the database. using the SDBCX-API
     */
    public void createTableInSDBCX(final HsqlTableDescriptor _tableDesc) throws SQLException, ElementExistException
    {
        final XPropertySet sdbcxDescriptor = _tableDesc.createSdbcxDescriptor(defaultConnection());
        final XTablesSupplier suppTables = UnoRuntime.queryInterface( XTablesSupplier.class, defaultConnection().getXConnection() );
        final XAppend appendTable = UnoRuntime.queryInterface( XAppend.class, suppTables.getTables() );
        appendTable.appendByDescriptor(sdbcxDescriptor);
    }

    private File m_documentFile;
}
