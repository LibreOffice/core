/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XCloseable;
import com.sun.star.sdbc.XConnection;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.uno.XComponentContext;
import java.io.File;

import com.sun.star.util.CloseVetoException;

public class HsqlDatabase
{
    private XComponentContext       m_context;
    // the URL of the temporary file used for the database document
    private String                  m_databaseDocumentFile;
    // the database document
    private XOfficeDatabaseDocument m_databaseDocument;
    // the data source belonging to the database document
    // the default connection
    private XConnection             m_connection;


    public HsqlDatabase( XComponentContext _context ) throws Exception
    {
        m_context = _context;
        createDBDocument();
    }


    public HsqlDatabase( XComponentContext _context, String _existingDocumentURL ) throws Exception
    {
        m_context = _context;
        createDBDocument( _existingDocumentURL );
    }


    private void createDBDocument( String _docURL ) throws Exception
    {
        m_databaseDocumentFile = _docURL;

        XNameAccess dbContext = UnoRuntime.queryInterface( XNameAccess.class,
            m_context.getServiceManager().createInstanceWithContext( "com.sun.star.sdb.DatabaseContext", m_context ) );
        XDocumentDataSource dataSource = UnoRuntime.queryInterface( XDocumentDataSource.class,
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

        m_databaseDocument = UnoRuntime.queryInterface(
            XOfficeDatabaseDocument.class, m_context.getServiceManager().createInstanceWithContext(
                "com.sun.star.sdb.OfficeDatabaseDocument", m_context ) );

        XPropertySet dsProperties = UnoRuntime.queryInterface( XPropertySet.class, m_databaseDocument.getDataSource() );
        dsProperties.setPropertyValue("URL", "sdbc:embedded:hsqldb");

        XStorable storable = UnoRuntime.queryInterface( XStorable.class, m_databaseDocument );
        storable.storeAsURL( m_databaseDocumentFile, new PropertyValue[]{} );
    }

    /** closes the database document
     *
     *  Any CloseVetoExceptions fired by third parties are ignored, and any reference to the
     *  database document is released.
     */
    private void close()
    {
        // close connection
        XCloseable closeConn = UnoRuntime.queryInterface( XCloseable.class,
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
        com.sun.star.util.XCloseable closeDoc = UnoRuntime.queryInterface(
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
    private void closeAndDelete()
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



    /** returns the URL of the ODB document represented by this instance
     */
    public String getDocumentURL()
    {
        return m_databaseDocumentFile;
    }



    @Override
    protected void finalize() throws Throwable
    {
        closeAndDelete();
        super.finalize();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
