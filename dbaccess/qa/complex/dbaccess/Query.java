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
package complex.dbaccess;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.sdb.XQueriesSupplier;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.uno.UnoRuntime;
import connectivity.tools.CRMDatabase;

// ---------- junit imports -----------------
import org.junit.Test;
import static org.junit.Assert.*;


public class Query extends TestCase
{

    connectivity.tools.HsqlDatabase m_database;


    private void createTestCase()
    {
        try
        {
            if (m_database == null)
            {
                final CRMDatabase database = new CRMDatabase(getMSF(), false);
                m_database = database.getDatabase();
            }
        }
        catch (Exception e)
        {
            System.out.println("could not create the test case, error message:\n" + e.getMessage());
            e.printStackTrace(System.err);
            fail("failed to created the test case");
        }
    }


//    private XMultiServiceFactory getFactory()
//    {
//        return (XMultiServiceFactory)param.getMSF();
//    }

    @Test
    public void testQueryColumns()
    {
        createTestCase();

        try
        {
            final XQueriesSupplier suppQueries = UnoRuntime.queryInterface(
                XQueriesSupplier.class, m_database.defaultConnection().getXConnection() );
            final XNameAccess queries = suppQueries.getQueries();

            final String[] queryNames = new String[] { "parseable", "parseable native", "unparseable" };
            final String[][] expectedColumnNames = new String[][] {
                new String[] { "ID", "Name", "Address", "City", "Postal","Comment" },
                new String[] { "TABLE_CATALOG", "TABLE_SCHEMA", "TABLE_NAME", "VIEW_DEFINITION", "CHECK_OPTION", "IS_UPDATABLE", "VALID" },
                new String[] { "ID_VARCHAR" }
            };

            for ( int i = 0; i < queryNames.length; ++i )
            {
                if (queries.hasByName(queryNames[i]))
                {
                    final XPropertySet query = UnoRuntime.queryInterface(
                        XPropertySet.class, queries.getByName( queryNames[i] ) );

                    final XColumnsSupplier suppCols = UnoRuntime.queryInterface(
                        XColumnsSupplier.class, query);
                    final XIndexAccess columns = UnoRuntime.queryInterface(
                                XIndexAccess.class, suppCols.getColumns());

                    // check whether the columns supplied by the query match what we expected
                    assertTrue( "invalid column count (found " + columns.getCount() + ", expected: " + expectedColumnNames[i].length + ") for query \"" + queryNames[i] + "\"",
                        columns.getCount() == expectedColumnNames[i].length );
                    for ( int col = 0; col < columns.getCount(); ++col )
                    {
                        final XNamed columnName = UnoRuntime.queryInterface(
                            XNamed.class, columns.getByIndex(col) );
                        assertTrue( "column no. " + col + " of query \"" + queryNames[i] + "\" not matching",
                            columnName.getName().equals( expectedColumnNames[i][col] ) );
                    }
                }
            }
        }
        catch ( Exception e )
        {
            fail( "caught an unexpected exception: " + e.getMessage() );
        }
    }
}
