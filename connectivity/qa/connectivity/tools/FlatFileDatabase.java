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
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.UnoRuntime;

import helper.URLHelper;
import java.io.File;

class FlatFileDatabase extends AbstractDatabase
{
    // --------------------------------------------------------------------------------------------------------
    protected FlatFileDatabase( final XMultiServiceFactory i_orb, final String i_urlSubScheme ) throws Exception
    {
        super(i_orb);
        m_urlSubScheme = i_urlSubScheme;
        createDBDocument();
    }

    // --------------------------------------------------------------------------------------------------------
    protected FlatFileDatabase(final XMultiServiceFactory i_orb, final String i_existingDocumentURL,
        final String i_urlSubScheme ) throws Exception
    {
        super( i_orb, i_existingDocumentURL );
        m_urlSubScheme = i_urlSubScheme;

        final XPropertySet dsProperties = UnoRuntime.queryInterface(XPropertySet.class, m_databaseDocument.getDataSource());
        final String url = (String)dsProperties.getPropertyValue( "URL" );
        final String expectedURLPrefix = "sdbc:" + m_urlSubScheme + ":";
        if ( !url.startsWith( expectedURLPrefix ) )
            throw new IllegalArgumentException( i_existingDocumentURL + " is of wrong type" );

        final String location = url.substring( expectedURLPrefix.length() );
        m_tableFileLocation = new File( location );
        if ( m_tableFileLocation.isDirectory() )
            throw new IllegalArgumentException( "unsupported table file location (must be a folder)" );
    }

    /**
     * returns a {@link File} which represents the folder where the database's table files reside.
     */
    public File getTableFileLocation()
    {
        return m_tableFileLocation;
    }

    /** creates an empty database document in a temporary location
     */
    private void createDBDocument() throws Exception
    {
        final File documentFile = File.createTempFile( m_urlSubScheme, ".odb" );
        if ( documentFile.exists() )
            documentFile.delete();
        m_tableFileLocation = new File(documentFile.getParent() + File.separator + documentFile.getName().replace(".odb", "") + File.separator );
        m_tableFileLocation.mkdir();
        //subPath.deleteOnExit();
        m_databaseDocumentFile = URLHelper.getFileURLFromSystemPath(documentFile);
        final String path = URLHelper.getFileURLFromSystemPath( m_tableFileLocation.getPath() );

        m_databaseDocument = UnoRuntime.queryInterface( XOfficeDatabaseDocument.class,
            m_orb.createInstance("com.sun.star.sdb.OfficeDatabaseDocument"));
        m_dataSource = new DataSource(m_orb, m_databaseDocument.getDataSource());

        final XPropertySet dsProperties = UnoRuntime.queryInterface(XPropertySet.class, m_databaseDocument.getDataSource());
        dsProperties.setPropertyValue("URL", "sdbc:" + m_urlSubScheme + ":" + path);

        final XStorable storable = UnoRuntime.queryInterface( XStorable.class, m_databaseDocument );
        storable.storeAsURL( m_databaseDocumentFile, new PropertyValue[] { } );
    }

    /** drops the table with a given name

    @param _name
    the name of the table to drop
    @param _ifExists
    TRUE if it should be dropped only when it exists.
     */
    public void dropTable(final String _name,final boolean _ifExists) throws SQLException
    {
        String dropStatement = "DROP TABLE \"" + _name;
        executeSQL(dropStatement);
    }

    final String    m_urlSubScheme;
    File            m_tableFileLocation = null;
}
