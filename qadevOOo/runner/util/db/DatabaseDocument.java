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

package util.db;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.io.IOException;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.uno.UnoRuntime;

/**
 *  encapsulates a css.sdb.DatabaseDocument
 */
public class DatabaseDocument
{
    protected DatabaseDocument( final DataSource _dataSource )
    {
        XDocumentDataSource docDataSource = UnoRuntime.queryInterface(
            XDocumentDataSource.class, _dataSource.getDataSource() );
        m_databaseDocument = UnoRuntime.queryInterface(XOfficeDatabaseDocument.class,
            docDataSource.getDatabaseDocument() );

        m_model = UnoRuntime.queryInterface( XModel.class, m_databaseDocument );
        m_storeDoc = UnoRuntime.queryInterface( XStorable.class, m_databaseDocument );
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

    private final XOfficeDatabaseDocument m_databaseDocument;
    private final XModel                  m_model;
    private final XStorable               m_storeDoc;
}
