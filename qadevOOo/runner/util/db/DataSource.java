/*
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
 */

package util.db;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XNamingService;
import java.util.logging.Level;
import java.util.logging.Logger;
import lib.StatusException;

/** wraps a com.sun.star.sdb.DataSource
 *
 * @author fs93730
 */
public class DataSource
{
    protected DataSource( XMultiServiceFactory _orb, DataSourceDescriptor _descriptor )
    {
        m_orb = _orb;
        try
        {
            m_dataSource = (XDataSource)UnoRuntime.queryInterface( XDataSource.class,
                m_orb.createInstance( "com.sun.star.sdb.DataSource" ) );
            m_properties = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class,
                m_dataSource );

            Object[] descriptorProperties = new Object[] {
                _descriptor.Name, _descriptor.URL, _descriptor.Info, _descriptor.User, _descriptor.Password,
                _descriptor.IsPasswordRequired };
            String[] propertyNames = new String[] {
                "Name", "URL", "Info", "User", "Password", "IsPasswordRequired" };
            for ( int i=0; i < descriptorProperties.length; ++i )
                if ( descriptorProperties[i] != null )
                    m_properties.setPropertyValue( propertyNames[i], descriptorProperties[i] );
        }
        catch ( Exception e )
        {
            throw new StatusException( "could not create/fill a css.sdb.DataSource object", e );
        }
    }

    public XDataSource getDataSource()
    {
        return m_dataSource;
    }

    /**
     * retrieves the css.sdb.OfficeDatabaseDocument associated with the data source
     * @return
     */
    public DatabaseDocument getDatabaseDocument()
    {
        synchronized ( this )
        {
            if ( m_document == null )
                m_document = new DatabaseDocument( m_orb, this );
        }
        return m_document;
    }

    public void revokeRegistration()
    {
        String dataSourceName = "";
        try
        {
            dataSourceName = (String)m_properties.getPropertyValue( "Name" );
            XNamingService dbContext = (XNamingService)UnoRuntime.queryInterface( XNamingService.class,
            m_orb.createInstance( "com.sun.star.sdb.DatabaseContext" ) );
            dbContext.revokeObject( dataSourceName );
        }
        catch ( Exception e )
        {
            throw new StatusException( "DataSource.revokeRegistration: could not revoke the object (" + dataSourceName + ")", e );
        }
    }

    public void registerAs( final String _registrationName, final boolean _revokeIfRegistered )
    {
        String doing = null;
        try
        {
            doing = "creating database context";
            XNamingService dbContext = UnoRuntime.queryInterface( XNamingService.class,
                m_orb.createInstance( "com.sun.star.sdb.DatabaseContext" ) );

            if ( _revokeIfRegistered )
            {
                doing = "revoking previously registered data source";
                try
                {
                    dbContext.revokeObject( _registrationName );
                }
                catch( NoSuchElementException e )
                {  /* allowed here */ }
            }

            // if the document associated with the database document has not yet been saved, then we need to do so
            DatabaseDocument doc = getDatabaseDocument();
            String docURL = doc.getURL();
            if ( docURL.length() == 0 )
            {
                final java.io.File tempFile = java.io.File.createTempFile( _registrationName + "_", ".odb" );
                if ( tempFile.exists() )
                    // we did not really want to create that file, we just wanted its local name, but
                    // createTempFile actually creates it => throw it away
                    // (This is necessary since some JVM/platform combinations seem to actually lock the file)
                    tempFile.delete();
                String localPart = tempFile.toURI().toURL().toString();
                localPart = localPart.substring( localPart.lastIndexOf( '/' ) + 1 );
                docURL = util.utils.getOfficeTemp( m_orb ) + localPart;
                doing = "storing database document to temporary location (" + docURL + ")";
                doc.storeAsURL( docURL );
            }

            // register the data soource
            doing = "registering the data source at the database context";
            dbContext.registerObject( _registrationName, m_dataSource );
        }
        catch( final java.lang.Exception e )
        {
            throw new StatusException( "DataSource.registerAs: error during " + doing, e );
        }
    }

    private XMultiServiceFactory    m_orb = null;
    private XDataSource             m_dataSource = null;
    private XPropertySet            m_properties = null;
    private DatabaseDocument        m_document = null;
}
