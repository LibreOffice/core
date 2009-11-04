/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Query.java,v $
 * $Revision: 1.7 $
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
package complex.dbaccess;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XQueriesSupplier;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.uno.UnoRuntime;
import connectivity.tools.CRMDatabase;

public class Query extends complexlib.ComplexTestCase {

    connectivity.tools.HsqlDatabase m_database;

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
                final CRMDatabase database = new CRMDatabase( getFactory(), false );
                m_database = database.getDatabase();
            }
        }
        catch( Exception e )
        {
            log.println( "could not create the test case, error message:\n" + e.getMessage() );
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
            final XQueriesSupplier suppQueries = (XQueriesSupplier)UnoRuntime.queryInterface(
                XQueriesSupplier.class, m_database.defaultConnection());
            final XNameAccess queries = suppQueries.getQueries();

            final String[] queryNames = new String[] { "parseable", "parseable native", "unparseable" };
            final String[][] expectedColumnNames = new String[][] {
                new String[] { "ID", "Name", "Address", "City", "Postal","Comment" },
                new String[] { "TABLE_CATALOG", "TABLE_SCHEMA", "TABLE_NAME", "VIEW_DEFINITION", "CHECK_OPTION", "IS_UPDATABLE", "VALID" },
                new String[] { "ID_VARCHAR" }
            };

            for ( int i = 0; i < queryNames.length; ++i )
            {
                final XPropertySet query = (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, queries.getByName( queryNames[i] ) );

                final XColumnsSupplier suppCols = (XColumnsSupplier)UnoRuntime.queryInterface(
                    XColumnsSupplier.class, query);
                final XIndexAccess columns = (XIndexAccess)UnoRuntime.queryInterface(
                    XIndexAccess.class, suppCols.getColumns());

                // check whether the columns supplied by the query match what we expected
                assure( "invalid column count (found " + columns.getCount() + ", expected: " + expectedColumnNames[i].length + ") for query \"" + queryNames[i] + "\"",
                    columns.getCount() == expectedColumnNames[i].length );
                for ( int col = 0; col < columns.getCount(); ++col )
                {
                    final XNamed columnName = (XNamed)UnoRuntime.queryInterface(
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
