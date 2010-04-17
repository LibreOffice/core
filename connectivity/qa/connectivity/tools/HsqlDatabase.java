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
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;
import java.io.File;

/**
 *
 * @author fs93730
 */
public class HsqlDatabase extends AbstractDatabase
{

    // --------------------------------------------------------------------------------------------------------
    public HsqlDatabase(final XMultiServiceFactory orb) throws Exception
    {
        super(orb);
        createDBDocument();
    }

    // --------------------------------------------------------------------------------------------------------
    public HsqlDatabase(final XMultiServiceFactory orb, final String _existingDocumentURL) throws Exception
    {
        super(orb, _existingDocumentURL);
    }

    /** creates an empty database document in a temporary location
     */
    private void createDBDocument() throws Exception
    {
        final File documentFile = File.createTempFile("testdb", ".odb");
        if ( documentFile.exists() )
            documentFile.delete();
        m_databaseDocumentFile = URLHelper.getFileURLFromSystemPath(documentFile);

        m_databaseDocument = (XOfficeDatabaseDocument) UnoRuntime.queryInterface(
                XOfficeDatabaseDocument.class, m_orb.createInstance("com.sun.star.sdb.OfficeDatabaseDocument"));
        m_dataSource = new DataSource(m_orb, m_databaseDocument.getDataSource());

        final XPropertySet dsProperties = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, m_databaseDocument.getDataSource());
        dsProperties.setPropertyValue("URL", "sdbc:embedded:hsqldb");

        final XStorable storable = (XStorable) UnoRuntime.queryInterface(XStorable.class, m_databaseDocument);
        storable.storeAsURL( m_databaseDocumentFile, new PropertyValue[]
            {   new PropertyValue( "PickListEntry", 0, false, PropertyState.DIRECT_VALUE )
            } );
    }

    /** drops the table with a given name

    @param _name
    the name of the table to drop
    @param _ifExists
    TRUE if it should be dropped only when it exists.
     */
    public void dropTable(final String _name, final boolean _ifExists) throws SQLException
    {
        final StringBuffer dropStatement = new StringBuffer("DROP TABLE \"");
        dropStatement.append(_name);
        if (_ifExists)
        {
            dropStatement.append("\" IF EXISTS");
        }
        executeSQL(dropStatement.toString());
    }

    public void createTable(final HsqlTableDescriptor _tableDesc, final boolean _dropIfExists) throws SQLException
    {
        if (_dropIfExists)
        {
            dropTable(_tableDesc.getName(), true);
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

        final HashMap foreignKeys = new HashMap();
        final HashMap foreignKeyRefs = new HashMap();

        final HsqlColumnDescriptor[] columns = _tableDesc.getColumns();
        for (int i = 0; i < columns.length; ++i)
        {
            if (i > 0)
            {
                createStatement.append(", ");
            }

            createStatement.append("\"" + columns[i].getName());
            createStatement.append("\" " + columns[i].getTypeName());

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

                String foreignKeysForTable = foreignKeys.containsKey(foreignTable) ? (String) foreignKeys.get(foreignTable) : "";
                if (foreignKeysForTable.length() > 0)
                {
                    foreignKeysForTable += ", ";
                }
                foreignKeysForTable += "\"" + columns[i].getName() + "\"";
                foreignKeys.put(foreignTable, foreignKeysForTable);

                final StringBuffer foreignKeyRefsForTable = new StringBuffer(foreignKeyRefs.containsKey(foreignTable) ? (String) foreignKeyRefs.get(foreignTable) : "");
                if (foreignKeyRefsForTable.length() > 0)
                {
                    foreignKeyRefsForTable.append(", ");
                }
                foreignKeyRefsForTable.append("\"" + columns[i].getForeignColumn() + "\"");
                foreignKeyRefs.put(foreignTable, foreignKeyRefsForTable.toString());
            }
        }

        if (primaryKeyList.length() > 0)
        {
            createStatement.append(", PRIMARY KEY (");
            createStatement.append(primaryKeyList);
            createStatement.append(')');
        }

        final Set foreignKeyTables = foreignKeys.keySet();
        for (final Iterator foreignKey = foreignKeyTables.iterator();
                foreignKey.hasNext();)
        {
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

        //System.err.println( createStatement );
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
}
