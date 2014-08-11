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
        final XRowSet xRowRes = UnoRuntime.queryInterface(XRowSet.class,
                m_xORB.createInstance("com.sun.star.sdb.RowSet"));

        getLog().println("starting SQL test");
        // set the properties needed to connect to a database
        final XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class, xRowRes);
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
            final XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class, xRowRes);
            xProp.setPropertyValue("Command", "SELECT " + sql);
            xRowRes.execute();
        }
        catch(SQLException e)
        {
            getLog().println(sql + " Error: " + e.getMessage());
        }
    }


}
