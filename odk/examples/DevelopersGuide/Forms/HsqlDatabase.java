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
