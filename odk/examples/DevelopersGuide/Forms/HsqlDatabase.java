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

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XStorable;
import com.sun.star.frame.XModel;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XCloseable;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XStatement;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.io.IOException;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.uno.XComponentContext;
import java.io.File;

import com.sun.star.util.CloseVetoException;
import java.io.File;

/**
 *
 * @author fs93730
 */
public class HsqlDatabase
{
    XComponentContext       m_context;
    // the URL of the temporary file used for the database document
    String                  m_databaseDocumentFile;
    // the database document
    XOfficeDatabaseDocument m_databaseDocument;
    // the data source belonging to the database document
    // the default connection
    XConnection             m_connection;

    // --------------------------------------------------------------------------------------------------------
    public HsqlDatabase( XComponentContext _context ) throws Exception
    {
        m_context = _context;
        createDBDocument();
    }

    // --------------------------------------------------------------------------------------------------------
    public HsqlDatabase( XComponentContext _context, String _existingDocumentURL ) throws Exception
    {
        m_context = _context;
        createDBDocument( _existingDocumentURL );
    }

    // --------------------------------------------------------------------------------------------------------
    private void createDBDocument( String _docURL ) throws Exception
    {
        m_databaseDocumentFile = _docURL;

        XNameAccess dbContext = (XNameAccess)UnoRuntime.queryInterface( XNameAccess.class,
            m_context.getServiceManager().createInstanceWithContext( "com.sun.star.sdb.DatabaseContext", m_context ) );
        XDocumentDataSource dataSource = (XDocumentDataSource)UnoRuntime.queryInterface( XDocumentDataSource.class,
            dbContext.getByName( _docURL ) );

        m_databaseDocument = dataSource.getDatabaseDocument();
    }

    /** creates an empty database document in a temporary location
     */
    private void createDBDocument() throws Exception
    {
        File documentFile = File.createTempFile("testdb",".odb");
        documentFile.deleteOnExit();
        m_databaseDocumentFile = URLHelper.getFileURLFromSystemPath( documentFile );

        m_databaseDocument = (XOfficeDatabaseDocument)UnoRuntime.queryInterface(
            XOfficeDatabaseDocument.class, m_context.getServiceManager().createInstanceWithContext(
                "com.sun.star.sdb.OfficeDatabaseDocument", m_context ) );

        XPropertySet dsProperties = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, m_databaseDocument.getDataSource() );
        dsProperties.setPropertyValue("URL", "sdbc:embedded:hsqldb");

        XStorable storable = (XStorable)UnoRuntime.queryInterface( XStorable.class, m_databaseDocument );
        storable.storeAsURL( m_databaseDocumentFile, new PropertyValue[]{} );
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

    /** stores the database document
    */
    public void store() throws IOException
    {
        if ( m_databaseDocument != null )
        {
            XStorable storeDoc = (XStorable)UnoRuntime.queryInterface( XStorable.class,
                m_databaseDocument );
            storeDoc.store();
        }
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
        com.sun.star.util.XCloseable closeDoc = (com.sun.star.util.XCloseable)UnoRuntime.queryInterface(
            com.sun.star.util.XCloseable.class, m_databaseDocument );
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

    /** returns the underlying database document
    */
    public XOfficeDatabaseDocument getDatabaseDocument()
    {
        return m_databaseDocument;
    }

    /** returns the associated data source
     */
    public XDataSource getDataSource()
    {
        return m_databaseDocument.getDataSource();
    }

    /** returns the model interface of the underlying database document
    */
    XModel getModel()
    {
        return (XModel)UnoRuntime.queryInterface( XModel.class, m_databaseDocument );
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

    /** returns the URL of the ODB document represented by this instance
     */
    public String getDocumentURL()
    {
        return m_databaseDocumentFile;
    }

    /** creates a row set operating the database, with a given command/type
     */
    public RowSet createRowSet( int _commandType, String _command )
    {
        return new RowSet( m_context, getDocumentURL(), _commandType, _command );
    }

    protected void finalize() throws Throwable
    {
        closeAndDelete();
        super.finalize();
    }
}
