/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: HsqlDatabase.java,v $
 * $Revision: 1.4 $
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
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ElementExistException;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XCloseable;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbcx.XAppend;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.UnoRuntime;
import java.io.File;

import com.sun.star.util.CloseVetoException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;

/**
 *
 * @author fs93730
 */
public class HsqlDatabase
{
    // the service factory
    XMultiServiceFactory    m_orb;
    // the URL of the temporary file used for the database document
    String                  m_databaseDocumentFile;
    // the database document
    XOfficeDatabaseDocument m_databaseDocument;
    // the data source belonging to the database document
    DataSource              m_dataSource;
    // the default connection
    XConnection             m_connection;

    /** Creates a new instance of HsqlDatabase */
    public HsqlDatabase( XMultiServiceFactory orb ) throws Exception
    {
        m_orb = orb;
        createDBDocument();
    }

    private void createDBDocument() throws Exception
    {
        m_databaseDocumentFile = new String();
        String str = File.createTempFile("testdb",".odb").getCanonicalPath();
        str = str.replaceAll(" ","%20");
        str = "file:///" +str;
        m_databaseDocumentFile = str.replace('\\','/');

        m_databaseDocument = (XOfficeDatabaseDocument)UnoRuntime.queryInterface(
            XOfficeDatabaseDocument.class, m_orb.createInstance( "com.sun.star.sdb.OfficeDatabaseDocument" ) );
        m_dataSource = new DataSource( m_orb, m_databaseDocument.getDataSource() );
        XPropertySet dsProperties = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, m_databaseDocument.getDataSource() );
        dsProperties.setPropertyValue("URL", "sdbc:embedded:hsqldb");

        XStorable storable = (XStorable)UnoRuntime.queryInterface( XStorable.class, m_databaseDocument );
        storable.storeAsURL(m_databaseDocumentFile,new PropertyValue[]{});
    }

    /** returns a connection to the database
     *
     * Multiple calls to this method return the same connection. The HsqlDatabase object keeps
     * the ownership of the connection, so you don't need to (and should not) dispose/close it.
     *
     */
    public XConnection defaultConnection() throws SQLException
    {
        if ( m_connection != null )
            return m_connection;
        m_connection  = m_databaseDocument.getDataSource().getConnection(new String(),new String());
        return m_connection;
    }

    /** executes the given SQL statement via the defaultConnection
     */
    public void executeSQL( String statementString ) throws SQLException
    {
        XStatement statement = defaultConnection().createStatement();
        statement.execute( statementString );
    }

    /** closes the database document
     *
     *  Any CloseVetoExceptions fired by third parties are ignored, and any reference to the
     *  database document is released.
     */
    public void close()
    {
        // close connection
        XCloseable closeConn = (XCloseable)UnoRuntime.queryInterface( XCloseable.class,
            m_connection );
        if ( closeConn != null )
        {
            try
            {
                closeConn.close();
            }
            catch( SQLException e )
            {
            }
        }
        m_connection = null;

        // close document
        com.sun.star.util.XCloseable closeDoc = (com.sun.star.util.XCloseable)UnoRuntime.queryInterface( XCloseable.class,
            m_databaseDocument );
        if ( closeDoc != null )
        {
            try
            {
                closeDoc.close( true );
            }
            catch( CloseVetoException e )
            {
            }
        }
        m_databaseDocument = null;
    }

    /** closes the document, and deletes the underlying file
     */
    public void closeAndDelete()
    {
        close();

        if ( m_databaseDocumentFile != null )
        {
            try
            {
                File file = new File(m_databaseDocumentFile);
                file.delete();
            }
            catch(Exception e)
            {
            }
            m_databaseDocumentFile = null;
        }
    }

    /** drops the table with a given name

        @param _name
            the name of the table to drop
        @param _ifExists
            TRUE if it should be dropped only when it exists.
    */
    public void dropTable( String _name, boolean _ifExists ) throws SQLException
    {
        String dropStatement = "DROP TABLE \"" + _name;
        if ( _ifExists )
            dropStatement += "\" IF EXISTS";
        executeSQL( dropStatement );
    }

    public void createTable( HsqlTableDescriptor _tableDesc, boolean _dropIfExists ) throws SQLException
    {
        if ( _dropIfExists )
            dropTable( _tableDesc.getName(), true );
        createTable( _tableDesc );
    }

    /** creates a table
     */
    public void createTable( HsqlTableDescriptor _tableDesc ) throws SQLException
    {
        String createStatement = "CREATE CACHED TABLE \"";
        createStatement += _tableDesc.getName();
        createStatement += "\" ( ";

        String primaryKeyList = "";

        HashMap foreignKeys = new HashMap();
        HashMap foreignKeyRefs = new HashMap();

        HsqlColumnDescriptor[] columns = _tableDesc.getColumns();
        for ( int i=0; i<columns.length; ++i )
        {
            if ( i > 0 )
                createStatement += ", ";

            createStatement += "\"" + columns[i].getName();
            createStatement += "\" " + columns[i].getTypeName();

            if ( columns[i].isRequired() )
                createStatement += " NOT NULL";

            if ( columns[i].isPrimaryKey() )
            {
                if ( primaryKeyList.length() > 0 )
                    primaryKeyList += ", ";
                primaryKeyList += "\"" + columns[i].getName() + "\"";
            }

            if ( columns[i].isForeignKey() )
            {
                String foreignTable = columns[i].getForeignTable();

                String foreignKeysForTable = foreignKeys.containsKey( foreignTable ) ? (String)foreignKeys.get( foreignTable ) : "";
                if ( foreignKeysForTable.length() > 0 )
                    foreignKeysForTable += ", ";
                foreignKeysForTable += "\"" + columns[i].getName() + "\"";
                foreignKeys.put( foreignTable, foreignKeysForTable );

                String foreignKeyRefsForTable = foreignKeyRefs.containsKey( foreignTable ) ? (String)foreignKeyRefs.get( foreignTable ) : "";
                if ( foreignKeyRefsForTable.length() > 0 )
                    foreignKeyRefsForTable += ", ";
                foreignKeyRefsForTable += "\"" + columns[i].getForeignColumn() + "\"";
                foreignKeyRefs.put( foreignTable, foreignKeyRefsForTable );
            }
        }

        if ( primaryKeyList.length() > 0 )
        {
            createStatement += ", PRIMARY KEY (";
            createStatement += primaryKeyList;
            createStatement += ")";
        }

        Set foreignKeyTables = foreignKeys.keySet();
        for (   Iterator foreignKey = foreignKeyTables.iterator();
                foreignKey.hasNext();
            )
        {
            String foreignTable = (String)foreignKey.next();

            createStatement += ", FOREIGN KEY (";
            createStatement += (String)foreignKeys.get(foreignTable);
            createStatement += ") REFERENCES \"";
            createStatement += foreignTable;
            createStatement += "\"(";
            createStatement += (String)foreignKeyRefs.get(foreignTable);
            createStatement += ")";
        }

        createStatement += ")";

        //System.err.println( createStatement );
        executeSQL( createStatement );
    }

    /** creates a table in the database. using the SDBCX-API
     */
    public void createTableInSDBCX( HsqlTableDescriptor _tableDesc ) throws SQLException, ElementExistException
    {
        XPropertySet sdbcxDescriptor = _tableDesc.createSdbcxDescriptor( defaultConnection() );
        XTablesSupplier suppTables = (XTablesSupplier)UnoRuntime.queryInterface(
            XTablesSupplier.class, defaultConnection() );
        XAppend appendTable = (XAppend)UnoRuntime.queryInterface(
            XAppend.class, suppTables.getTables() );
        appendTable.appendByDescriptor( sdbcxDescriptor );
    }

    /** returns the URL of the ODB document represented by this instance
     */
    public String getDocumentURL()
    {
        return m_databaseDocumentFile;
    }

    /** returns the data source belonging to this database
    */
    public DataSource getDataSource()
    {
        return m_dataSource;
    }

    /** creates a row set operating the database, with a given command/type
     */
    public RowSet createRowSet( int _commandType, String _command )
    {
        return new RowSet(m_orb, getDocumentURL(), _commandType, _command);
    }

    protected void finalize() throws Throwable
    {
        closeAndDelete();
        super.finalize();
    }
}
