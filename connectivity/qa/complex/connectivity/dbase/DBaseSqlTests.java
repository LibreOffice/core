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
package complex.connectivity.dbase;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.sdbc.*;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import complex.connectivity.TestCase;
import complex.connectivity.SubTestCase;

public class DBaseSqlTests extends SubTestCase
{
    private final XMultiServiceFactory m_xORB;

    public DBaseSqlTests(final XMultiServiceFactory _xORB,final TestCase i_testCase)
    {
        super( i_testCase );
        m_xORB = _xORB;
    }

    public void testFunctions() throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRowSet xRowRes = (XRowSet) UnoRuntime.queryInterface(XRowSet.class,
                m_xORB.createInstance("com.sun.star.sdb.RowSet"));

        getLog().println("starting SQL test");
        // set the properties needed to connect to a database
        final XPropertySet xProp = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xRowRes);
        xProp.setPropertyValue("DataSourceName", "Bibliography");
        xProp.setPropertyValue("CommandType", Integer.valueOf(com.sun.star.sdb.CommandType.COMMAND));

        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where \"Identifier\" like 'B%'");
        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where not \"Identifier\" like 'B%'");
        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where not \"Identifier\" not like 'B%'");
        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where not(0 = 1)");
        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where 0 = 0");
        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where (0 = 0)");
        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where 0 <> 1");
        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where 0 < 1");
        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where 2 > 1");
        execute(xRowRes,"1,1+1,'a' + 'b' FROM \"biblio\" \"biblio\" where 2 > 1");
        // execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where (0 = 0) is true");
        // execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where not (0 = 0) is not true");
        // execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where 1 between 0 and 2");
        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where not \"Identifier\" is NULL");
        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where \"Identifier\" is not NULL");
        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where \"Identifier\" = \"Identifier\"");
        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where not(not(\"Identifier\" = \"Identifier\"))");
        execute(xRowRes,"1 FROM \"biblio\" \"biblio\" where (1 = 1 and 2 = 1) or 3 = 33 or 4 = 44 or ('a' = 'a' and 'b' = 'b')");
    }

    private void execute(final XRowSet xRowRes, String sql) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        try
        {
            final XPropertySet xProp = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xRowRes);
            xProp.setPropertyValue("Command", "SELECT " + sql);
            xRowRes.execute();
        }
        catch(SQLException e)
        {
            getLog().println(sql + " Error: " + e.getMessage());
        }
    }


}
