/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Query.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:26:33 $
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
package complex.dbaccess;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XQueriesSupplier;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.uno.UnoRuntime;
import connectivity.tools.*;

public class Query extends complexlib.ComplexTestCase {

    HsqlDatabase    m_database;
    DataSource      m_dataSource;

    // --------------------------------------------------------------------------------------------------------
    public String[] getTestMethodNames() {
        return new String[]
        {
            "testQueryColumns"
        };
    }

    // --------------------------------------------------------------------------------------------------------
    public String getTestObjectName() {
        return "Query";
    }

    // --------------------------------------------------------------------------------------------------------
    private void createTestCase()
    {
        try
        {
            if ( m_database == null )
            {
                CRMDatabase database = new CRMDatabase( getFactory() );
                m_database = database.getDatabase();
                m_dataSource = m_database.getDataSource();
            }
        }
        catch( Exception e )
        {
            System.err.println( "could not create the test case, error message:\n" + e.getMessage() );
            e.printStackTrace( System.err );
            assure( "failed to created the test case", false );
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private XMultiServiceFactory getFactory()
    {
        return (XMultiServiceFactory)param.getMSF();
    }

    // --------------------------------------------------------------------------------------------------------
    public void testQueryColumns()
    {
        createTestCase();

        try
        {
            XQueriesSupplier suppQueries = (XQueriesSupplier)UnoRuntime.queryInterface(
                XQueriesSupplier.class, m_database.defaultConnection());
            XNameAccess queries = suppQueries.getQueries();

            String[] queryNames = new String[] { "parseable", "parseable native", "unparseable" };
            String[][] expectedColumnNames = new String[][] {
                new String[] { "ID", "Name", "Address", "City", "Postal" },
                new String[] { "TABLE_CATALOG", "TABLE_SCHEMA", "TABLE_NAME", "VIEW_DEFINITION", "CHECK_OPTION", "IS_UPDATABLE", "VALID" },
                new String[] { "ID_VARCHAR" }
            };

            for ( int i = 0; i < queryNames.length; ++i )
            {
                XPropertySet query = (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, queries.getByName( queryNames[i] ) );

                XColumnsSupplier suppCols = (XColumnsSupplier)UnoRuntime.queryInterface(
                    XColumnsSupplier.class, query);
                XIndexAccess columns = (XIndexAccess)UnoRuntime.queryInterface(
                    XIndexAccess.class, suppCols.getColumns());

                // check whether the columns supplied by the query match what we expected
                assure( "invalid column count (found " + columns.getCount() + ", expected: " + expectedColumnNames[i].length + ") for query \"" + queryNames[i] + "\"",
                    columns.getCount() == expectedColumnNames[i].length );
                for ( int col = 0; col < columns.getCount(); ++col )
                {
                    XNamed columnName = (XNamed)UnoRuntime.queryInterface(
                        XNamed.class, columns.getByIndex(col) );
                    assure( "column no. " + col + " of query \"" + queryNames[i] + "\" not matching",
                        columnName.getName().equals( expectedColumnNames[i][col] ) );
                }
            }
        }
        catch ( Exception e )
        {
            assure( "caught an unexpected exception: " + e.getMessage(), false );
        }
    }
}
