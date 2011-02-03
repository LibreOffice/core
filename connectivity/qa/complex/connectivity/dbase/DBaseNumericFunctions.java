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
import complex.connectivity.SubTestCase;
import complex.connectivity.TestCase;


public class DBaseNumericFunctions extends SubTestCase
{
    private final String where = "FROM \"biblio\" \"biblio\" where \"Identifier\" = 'BOR00'";
    private final XMultiServiceFactory m_xORB;

    public DBaseNumericFunctions(final XMultiServiceFactory _xORB, final TestCase i_testCase)
    {
        super( i_testCase );
        m_xORB = _xORB;
    }

    public void testFunctions() throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRowSet xRowRes = (XRowSet) UnoRuntime.queryInterface(XRowSet.class,
                m_xORB.createInstance("com.sun.star.sdb.RowSet"));

        getLog().println("starting Numeric function test");
        // set the properties needed to connect to a database
        final XPropertySet xProp = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xRowRes);
        xProp.setPropertyValue("DataSourceName", "Bibliography");

        xProp.setPropertyValue("CommandType", Integer.valueOf(com.sun.star.sdb.CommandType.COMMAND));

        try
        {
            abs(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("abs " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            acos(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("acos " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            asin(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("asin " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            atan(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("atan " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            atan2(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("atan2 " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            ceiling(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("ceiling " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            cos(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("cos " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            degrees(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("degrees " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            exp(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("exp " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            floor(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("floor " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            log(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("log " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            log10(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("log10 " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            mod(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("mod " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            pi(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("pi " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            pow(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("pow " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            radians(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("radians " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            round(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("round " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            sign(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("sign " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            sin(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("sin " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            sqrt(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("sqrt " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            tan(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("tan " + ex.getMessage(), false);
            throw ex;
        }

    }

    private XRow execute(final XRowSet xRowRes,final  String sql) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XPropertySet xProp = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xRowRes);
        xProp.setPropertyValue("Command", "SELECT " + sql + where);
        xRowRes.execute();
        final XResultSet xRes = (XResultSet) UnoRuntime.queryInterface(XResultSet.class, xRowRes);
        assure("No valid row! ", xRes.next());

        return (XRow) UnoRuntime.queryInterface(XRow.class, xRes);
    }

    private void abs(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "ABS(2),ABS(-32) ");
        assure("ABS(2) failed!", row.getInt(1) == 2);
        assure("ABS(-32) failed!", row.getInt(2) == 32);
    }

    private void sign(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "SIGN(-32),SIGN(0),SIGN(234) ");
        assure("SIGN(-32)failed!", row.getInt(1) == -1);
        assure("SIGN(0) failed!", row.getInt(2) == 0);
        assure("SIGN(234) failed!", row.getInt(3) == 1);
    }

    private void mod(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "MOD(234, 10) ");
        assure("MOD(234, 10) failed!", row.getInt(1) == 4);
    }

    private void floor(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "FLOOR(1.23),FLOOR(-1.23) ");
        assure("FLOOR(1.23) failed!", row.getInt(1) == 1);
        assure("FLOOR(-1.23) failed!", row.getInt(2) == -2);
    }

    private void ceiling(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "CEILING(1.23),CEILING(-1.23) ");
        assure("CEILING(1.23) failed!", row.getInt(1) == 2);
        assure("CEILING(-1.23) failed!", row.getInt(2) == -1);
    }

    private void round(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "ROUND(-1.23),ROUND(1.298, 1) ");
        assure("ROUND(-1.23) failed!", row.getInt(1) == -1);
        assure("ROUND(1.298, 1) failed!", row.getDouble(2) == 1.3);
    }

    private void exp(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "EXP(2),EXP(-2) ");
        assure("EXP(2) failed!", (float) row.getDouble(1) == (float) java.lang.Math.exp(2));
        assure("EXP(-2) failed!", (float) row.getDouble(2) == (float) java.lang.Math.exp(-2));
    }

    private void log(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "LOG(2),LOG(-2) ");
        assure("LOG(2) failed!", (float) row.getDouble(1) == (float) java.lang.Math.log(2));
        row.getDouble(2);
        assure("LOG(-2) failed!", row.wasNull());
    }

    private void log10(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "LOG10(100) ");
        assure("LOG10(100) failed!", row.getDouble(1) == 2.0);
    }

    private void pow(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "POWER(2,2) ");
        assure("POWER(2,2) failed!", row.getDouble(1) == 4.0);
    }

    private void sqrt(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "SQRT(4) ");
        assure("SQRT(4) failed!", row.getDouble(1) == 2.0);
    }

    private void pi(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "PI() ");
        assure("PI() failed!", (float) row.getDouble(1) == (float) java.lang.Math.PI);
    }

    private void cos(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "COS(PI()) ");
        assure("COS(PI()) failed!", row.getDouble(1) == -1.0);
    }

    private void sin(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "SIN(2) ");
        assure("SIN(PI()) failed!", (float) row.getDouble(1) == (float) java.lang.Math.sin(2));
    }

    private void tan(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "TAN(PI()+1) ");
        assure("TAN(PI()+1) failed!", (float) row.getDouble(1) == (float) java.lang.Math.tan(java.lang.Math.PI + 1.0));
    }

    private void acos(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "ACOS(1) ");
        assure("ACOS(1) failed!", (float) row.getDouble(1) == 0.0);
    }

    private void asin(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "ASIN(0) ");
        assure("ASIN(0) failed!", (float) row.getDouble(1) == (float) java.lang.Math.asin(0.0));
    }

    private void atan(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "ATAN(0) ");
        assure("ATAN(0) failed!", row.getDouble(1) == 0.0);
    }

    private void atan2(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "ATAN2(0,2) ");
        assure("ATAN2(0,2) failed!", (float) row.getDouble(1) == 0.0);
    }

    private void degrees(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "DEGREES(PI()) ");
        assure("DEGREES(PI()) failed!", row.getDouble(1) == 180.0);
    }

    private void radians(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "RADIANS(90) ");
        assure("RADIANS(90) failed!", (float) row.getDouble(1) == (float) (java.lang.Math.PI / 2.0));
    }
}
