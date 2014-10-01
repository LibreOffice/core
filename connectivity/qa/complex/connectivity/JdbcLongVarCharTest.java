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

package complex.connectivity;

import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XClob;
import com.sun.star.sdbc.XDriverAccess;
import com.sun.star.sdbc.XParameters;
import com.sun.star.sdbc.XPreparedStatement;
import com.sun.star.sdbc.XResultSetMetaData;
import com.sun.star.sdbc.XResultSetMetaDataSupplier;
import com.sun.star.sdbc.XRow;
import com.sun.star.uno.UnoRuntime;
import complexlib.ComplexTestCase;

public class JdbcLongVarCharTest extends ComplexTestCase
{

    @Override
    public String[] getTestMethodNames()
    {
        return new String[]
                {
                    "testLongVarChar"
                };
    }

    @Override
    public String getTestObjectName()
    {
        return "LongVarCharTest";
    }

    public void testLongVarChar()
    {

        try
        {
            System.out.println("== Start testing ==");

            String url = "jdbc:mysql://localhost:3306/mysql?user=root";
            com.sun.star.beans.PropertyValue prop[] = new PropertyValue[1];
            prop[0] = new PropertyValue("JavaDriverClass", 0, "com.mysql.jdbc.Driver", PropertyState.DIRECT_VALUE);

            // get the remote office component context
            XMultiServiceFactory xServiceManager = param.getMSF();
            Object x = xServiceManager.createInstance("com.sun.star.sdbc.DriverManager");
            com.sun.star.sdbc.XDriverAccess xDriverAccess = UnoRuntime.queryInterface(XDriverAccess.class, x);
            com.sun.star.sdbc.XDriver xDriver = xDriverAccess.getDriverByURL(url);
            com.sun.star.sdbc.XConnection xConnection = xDriver.connect(url, prop);

            Object prepStmnt = xConnection.prepareStatement("SELECT * FROM i90114 WHERE i90114.c1 = ?");
            UnoRuntime.queryInterface(XParameters.class, prepStmnt).clearParameters();
            UnoRuntime.queryInterface(XParameters.class, prepStmnt).setInt(1, 1);
            XResultSet xResultSet = ((XPreparedStatement) prepStmnt).executeQuery();
            XRow xRow = UnoRuntime.queryInterface(XRow.class, xResultSet);

            XResultSetMetaDataSupplier xRsMetaSup = UnoRuntime.queryInterface(XResultSetMetaDataSupplier.class, xResultSet);
            XResultSetMetaData xRsMetaData = xRsMetaSup.getMetaData();
            int nColumnCount = xRsMetaData.getColumnCount();

            System.out.println("== MetaData ==");
            for (int i = 1; i <= nColumnCount; ++i)
            {
                System.out.println("Name: " + xRsMetaData.getColumnName(i) + " Type: " +
                        xRsMetaData.getColumnType(i));
            }

            System.out.println("== Result ==");
            while (xResultSet.next())
            {
                String str = "not set";

                XClob xClob = xRow.getClob(2);
                if (xClob != null)
                {
                    System.out.println("xClob != null");
                    int len = (int) xClob.length();
                    str = xClob.getSubString(1, len);
                }
                else
                {
                    System.out.println("xClob == null");
                }

                System.out.println("c1 (Int): " + xRow.getInt(1) + " c2 (String): " + xRow.getString(2) + " c3 (Clob): " + str);
            }

            xConnection.close();
        }
        catch (java.lang.Exception e)
        {
            System.out.println("== Exception occurred while testing ==");
            e.printStackTrace();
        } finally
        {
            System.out.println("== End testing ==");
            System.exit(0);
        }
    }
}