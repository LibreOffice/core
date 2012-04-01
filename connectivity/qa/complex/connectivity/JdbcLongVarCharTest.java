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

    public void testLongVarChar() throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {

        try
        {
            System.out.println("== Start testing ==");

            String url = "jdbc:mysql://localhost:3306/mysql?user=root";
            //String url = "jdbc:ingres://localhost:II7/demodb;AUTO=multi";
            com.sun.star.sdbc.XConnection xConnection = null;
            com.sun.star.beans.PropertyValue prop[] = new PropertyValue[1];
            prop[0] = new PropertyValue("JavaDriverClass", 0, "com.mysql.jdbc.Driver", PropertyState.DIRECT_VALUE);
            //prop[0] = new PropertyValue("JavaDriverClass", 0, "com.ingres.jdbc.IngresDriver", PropertyState.DIRECT_VALUE);

            // get the remote office component context
            XMultiServiceFactory xServiceManager = (XMultiServiceFactory) param.getMSF();
            Object x = xServiceManager.createInstance("com.sun.star.sdbc.DriverManager");
            com.sun.star.sdbc.XDriverAccess xDriverAccess = (XDriverAccess) UnoRuntime.queryInterface(XDriverAccess.class, x);
            com.sun.star.sdbc.XDriver xDriver = xDriverAccess.getDriverByURL(url);
            xConnection = xDriver.connect(url, prop);

            //Object prepStmnt = xConnection.prepareStatement("SELECT * FROM t1 WHERE t1.c1 = ?");
            Object prepStmnt = xConnection.prepareStatement("SELECT * FROM i90114 WHERE i90114.c1 = ?");
            ((XParameters) UnoRuntime.queryInterface(XParameters.class, prepStmnt)).clearParameters();
            ((XParameters) UnoRuntime.queryInterface(XParameters.class, prepStmnt)).setInt(1, 1);
            XResultSet xResultSet = ((XPreparedStatement) prepStmnt).executeQuery();
            XRow xRow = (XRow) UnoRuntime.queryInterface(XRow.class, xResultSet);

            XResultSetMetaDataSupplier xRsMetaSup = (XResultSetMetaDataSupplier) UnoRuntime.queryInterface(XResultSetMetaDataSupplier.class, xResultSet);
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

                XClob xClob = null;
                xClob = xRow.getClob(2);
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