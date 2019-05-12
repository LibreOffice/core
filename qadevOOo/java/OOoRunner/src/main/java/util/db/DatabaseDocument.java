/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
