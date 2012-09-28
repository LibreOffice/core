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

import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.io.IOException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XCloseable;
import com.sun.star.sdbc.XStatement;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.CloseVetoException;
import connectivity.tools.sdb.Connection;
import java.io.File;

/**
 *
 * @author oj93728
 */
public abstract class AbstractDatabase implements DatabaseAccess
{
    public AbstractDatabase(final XMultiServiceFactory orb) throws Exception
    {
        m_orb = orb;
    }

    // --------------------------------------------------------------------------------------------------------
    public AbstractDatabase(final XMultiServiceFactory orb, final String _existingDocumentURL ) throws Exception
    {
        m_orb = orb;
        createDBDocument( _existingDocumentURL );
    }

    /** returns a connection to the database
     *
     * Multiple calls to this method return the same connection. The DbaseDatabase object keeps
     * the ownership of the connection, so you don't need to (and should not) dispose/close it.
     */
    public Connection defaultConnection() throws SQLException
    {
        if ( m_connection == null )
            m_connection = new Connection( m_databaseDocument.getDataSource().getConnection("", "") );

        return m_connection;
    }

    /** executes the given SQL statement via the defaultConnection
     */
    public void executeSQL(final String statementString) throws SQLException
    {
        final XStatement statement = defaultConnection().createStatement();
        statement.execute(statementString);
    }

    /** stores the database document
     */
    public void store() throws IOException
    {
        if (m_databaseDocument != null)
        {
            final XStorable storeDoc = UnoRuntime.queryInterface(XStorable.class, m_databaseDocument);
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
        final XCloseable closeConn = UnoRuntime.queryInterface( XCloseable.class,
            m_connection != null ? m_connection.getXConnection() : null );
        if (closeConn != null)
        {
            try
            {
                closeConn.close();
            }
            catch (SQLException e)
            {
            }
        }
        m_connection = null;

        // close document
        final com.sun.star.util.XCloseable closeDoc = UnoRuntime.queryInterface( com.sun.star.util.XCloseable.class, m_databaseDocument );
        if (closeDoc != null)
        {
            try
            {
                closeDoc.close(true);
            }
            catch (CloseVetoException e)
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

        if (m_databaseDocumentFile != null)
        {
            try
            {
                final File file = new File(m_databaseDocumentFile);
                file.delete();
            }
            catch (Exception e)
            {
            }
        }
    }

    /** returns the underlying database document
     */
    public XOfficeDatabaseDocument getDatabaseDocument()
    {
        return m_databaseDocument;
    }

    /** returns the model interface of the underlying database document
     */
    public XModel getModel()
    {
        return UnoRuntime.queryInterface( XModel.class, m_databaseDocument );
    }

    public XMultiServiceFactory getORB()
    {
        return m_orb;
    }

    // --------------------------------------------------------------------------------------------------------
    final protected void createDBDocument(final String _docURL) throws Exception
    {
        m_databaseDocumentFile = _docURL;

        final XNameAccess dbContext = UnoRuntime.queryInterface( XNameAccess.class,
            m_orb.createInstance( "com.sun.star.sdb.DatabaseContext" ) );
        final XDocumentDataSource dataSource = UnoRuntime.queryInterface( XDocumentDataSource.class, dbContext.getByName( _docURL ) );

        m_databaseDocument = dataSource.getDatabaseDocument();
        m_dataSource = new DataSource(m_databaseDocument.getDataSource());
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
    public RowSet createRowSet(final int _commandType, final String _command)
    {
        return new RowSet(m_orb, getDocumentURL(), _commandType, _command);
    }

    @Override
    protected void finalize() throws Throwable
    {
        closeAndDelete();
        super.finalize();
    }

    // the service factory
    protected final XMultiServiceFactory m_orb;
    // the URL of the temporary file used for the database document
    protected String m_databaseDocumentFile;
    // the database document
    protected XOfficeDatabaseDocument m_databaseDocument;
    // the data source belonging to the database document
    protected DataSource m_dataSource;
    // the default connection
    protected Connection    m_connection;
}
