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
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XOfficeDatabaseDocument;
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
        m_dataSource = new DataSource(m_databaseDocument.getDataSource());

        final XPropertySet dsProperties = UnoRuntime.queryInterface(XPropertySet.class, m_databaseDocument.getDataSource());
        dsProperties.setPropertyValue("URL", "sdbc:" + m_urlSubScheme + ":" + path);

        final XStorable storable = UnoRuntime.queryInterface( XStorable.class, m_databaseDocument );
        storable.storeAsURL( m_databaseDocumentFile, new PropertyValue[] { } );
    }

    final String    m_urlSubScheme;
    File            m_tableFileLocation = null;
}
