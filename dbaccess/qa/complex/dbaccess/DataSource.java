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
// ------------------------------------------


public class DataSource extends TestCase
{

    HsqlDatabase m_database;
    connectivity.tools.DataSource m_dataSource;

    // --------------------------------------------------------------------------------------------------------
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

    // --------------------------------------------------------------------------------------------------------
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
