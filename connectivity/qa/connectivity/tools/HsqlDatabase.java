/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HsqlDatabase.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-02-06 16:43:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package connectivity.tools;

import java.io.File;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.sdb.*;
import com.sun.star.sdbc.*;
import com.sun.star.lang.*;
import com.sun.star.beans.*;
import com.sun.star.frame.*;
import com.sun.star.util.CloseVetoException;

/**
 *
 * @author fs93730
 */
public class HsqlDatabase
{
    // the service factory
    XMultiServiceFactory    m_orb;
    // the URL of the temporary file used for the database document
    String                  m_databaseDocumentFile;
    // the data source belonging to the database document
    XOfficeDatabaseDocument m_databaseDocument;
    // the default connection
    XConnection             m_connection;

    /** Creates a new instance of HsqlDatabase */
    public HsqlDatabase( XMultiServiceFactory orb ) throws Exception
    {
        m_orb = orb;
        createDBDocument();
    }

    private void createDBDocument() throws Exception
    {
        m_databaseDocumentFile = new String();
        String str = File.createTempFile("testdb",".odb").getCanonicalPath();
        str = str.replaceAll(" ","%20");
        str = "file:///" +str;
        m_databaseDocumentFile = str.replace('\\','/');

        m_databaseDocument = (XOfficeDatabaseDocument)UnoRuntime.queryInterface(
            XOfficeDatabaseDocument.class, m_orb.createInstance( "com.sun.star.sdb.OfficeDatabaseDocument" ) );
        XDataSource dataSource = m_databaseDocument.getDataSource();
        XPropertySet dsProperties = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, dataSource );
        dsProperties.setPropertyValue("URL", "sdbc:embedded:hsqldb");

        XStorable storable = (XStorable)UnoRuntime.queryInterface( XStorable.class, m_databaseDocument );
        storable.storeAsURL(m_databaseDocumentFile,new PropertyValue[]{});
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

    /** closes the database document
     *
     *  Any CloseVetoExceptions fired by third parties are ignored, and any reference to the
     *  database document is released.
     */
    public void close()
    {
        // close connection
        com.sun.star.sdbc.XCloseable closeConn = (com.sun.star.sdbc.XCloseable)UnoRuntime.queryInterface( XCloseable.class,
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
        com.sun.star.util.XCloseable closeDoc = (com.sun.star.util.XCloseable)UnoRuntime.queryInterface( XCloseable.class,
            m_databaseDocument );
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

    /** creates a table
     */
    public void createTable( HsqlTableDescriptor _tableDesc ) throws SQLException
    {
        String createStatement = "CREATE TABLE \"";
        createStatement += _tableDesc.getName();
        createStatement += "\" ( ";

        String primaryKeyList = "";

        HsqlColumnDescriptor[] columns = _tableDesc.getColumns();
        for ( int i=0; i<columns.length; ++i )
        {
            if ( i > 0 )
                createStatement += ", ";

            createStatement += "\"" + columns[i].Name;
            createStatement += "\"" + columns[i].TypeName;

            if ( columns[i].NotNull )
                createStatement += " NOT NULL";

            if ( columns[i].PrimaryKey )
            {
                if ( primaryKeyList.length() > 0 )
                    primaryKeyList += ", ";
                primaryKeyList += "\"" + columns[i].Name + "\"";
            }
        }

        if ( primaryKeyList.length() > 0 )
        {
            createStatement += ", PRIMARY KEY (";
            createStatement += primaryKeyList;
            createStatement += ")";
        }

        createStatement += ")";

        executeSQL( createStatement );
    }

    /** returns the URL of the ODB document represented by this instance
     */
    public String getDocumentURL()
    {
        return m_databaseDocumentFile;
    }

    protected void finalize() throws Throwable
    {
        closeAndDelete();
        super.finalize();
    }
}
