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

import com.sun.star.container.XNameAccess;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XNamingService;
import connectivity.tools.CRMDatabase;
import connectivity.tools.HsqlDatabase;

// ---------- junit imports -----------------
import org.junit.Test;
import static org.junit.Assert.*;



public class DataSource extends TestCase
{

    HsqlDatabase m_database;
    connectivity.tools.DataSource m_dataSource;


    private void createTestCase()
    {
        try
        {
            if (m_database == null)
            {
                final CRMDatabase database = new CRMDatabase( getMSF(), false );
                m_database = database.getDatabase();
                m_dataSource = m_database.getDataSource();
            }
        }
        catch (Exception e)
        {
            fail("could not create the test case, error message:\n" + e.getMessage());
        }
        catch (java.lang.Exception e)
        {
            fail("could not create the test case, error message:\n" + e.getMessage());
        }
    }


    @Test
    public void testRegistrationName()
    {
        try
        {
            createTestCase();
            // 1. check the existing "Bibliography" data source whether it has the proper name
            String dataSourceName = "Bibliography";
            final connectivity.tools.DataSource bibliography = new connectivity.tools.DataSource(getMSF(), dataSourceName);
            assertEquals("pre-registered database has a wrong name!", dataSourceName, bibliography.getName());
            // 2. register a newly created data source, and verify it has the proper name
            dataSourceName = "someDataSource";
            final XNamingService dataSourceRegistrations = UnoRuntime.queryInterface(
                XNamingService.class, getMSF().createInstance( "com.sun.star.sdb.DatabaseContext" ) );
            final XNameAccess existenceCheck = UnoRuntime.queryInterface( XNameAccess.class, dataSourceRegistrations );
            if ( existenceCheck.hasByName( "someDataSource" ) )
                dataSourceRegistrations.revokeObject( "someDataSource" );
            dataSourceRegistrations.registerObject("someDataSource", m_dataSource.getXDataSource());
            assertEquals("registration name of a newly registered data source is wrong", dataSourceName, m_dataSource.getName());
        }
        catch (Exception ex)
        {
            // Logger.getLogger(DataSource.class.getName()).log(Level.SEVERE, null, ex);
            fail();
        }
    }
}
