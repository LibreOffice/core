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
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.io.IOException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.uno.UnoRuntime;

/**
 *  encapsulates a css.sdb.DatabaseDocument
 */
public class DatabaseDocument
{
    protected DatabaseDocument( final XMultiServiceFactory _orb, final DataSource _dataSource )
    {
        m_orb = _orb;
        m_dataSource = _dataSource;

        XDocumentDataSource docDataSource = (XDocumentDataSource)UnoRuntime.queryInterface(
            XDocumentDataSource.class, m_dataSource.getDataSource() );
        m_databaseDocument = (XOfficeDatabaseDocument)UnoRuntime.queryInterface(XOfficeDatabaseDocument.class,
            docDataSource.getDatabaseDocument() );

        m_model = (XModel)UnoRuntime.queryInterface( XModel.class, m_databaseDocument );
        m_storeDoc = (XStorable)UnoRuntime.queryInterface( XStorable.class, m_databaseDocument );
    }

    public DataSource getDataSource()
    {
        return m_dataSource;
    }

    public XOfficeDatabaseDocument getDatabaseDocument()
    {
        return m_databaseDocument;
    }

    /**
     * passes through to XModel.getURL.
     */
    public String getURL()
    {
        return m_model.getURL();
    }

    /**
     * simplified version (taking no arguments except the target URL) of XStorage.storeAsURL
     * @param _url
     *      specifies the location to where to store the document
     */
    public void storeAsURL( final String _url ) throws IOException
    {
        m_storeDoc.storeAsURL( _url, new PropertyValue[] { } );
    }

    private XMultiServiceFactory    m_orb;
    private DataSource              m_dataSource;
    private XOfficeDatabaseDocument m_databaseDocument;
    private XModel                  m_model;
    private XStorable               m_storeDoc;
}
