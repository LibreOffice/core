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

public class DBaseDateFunctions extends SubTestCase
{

    private static final String where = "FROM \"biblio\" \"biblio\" where \"Identifier\" = 'BOR00'";
    private final XMultiServiceFactory m_xORB;

    public DBaseDateFunctions(final XMultiServiceFactory _xORB, final TestCase i_testCase)
    {
        super( i_testCase );
        m_xORB = _xORB;
    }

    public void testFunctions() throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRowSet xRowRes = UnoRuntime.queryInterface(XRowSet.class,
                m_xORB.createInstance("com.sun.star.sdb.RowSet"));

        getLog().println("starting DateTime function test!");
        // set the properties needed to connect to a database
        final XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class, xRowRes);
        xProp.setPropertyValue("DataSourceName", "Bibliography");

        xProp.setPropertyValue("CommandType", Integer.valueOf(com.sun.star.sdb.CommandType.COMMAND));

        try
        {
            curdate(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("upper " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            curtime(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("lower " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            dayname(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("ascii " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            dayofmonth(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("char_len " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            dayofweek(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("concat " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            dayofyear(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("locate " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            hour(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("substr " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            minute(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("ltrim " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            month(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("rtrim " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            monthname(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("space " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            now(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("replace " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            quarter(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("repeat " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            second(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("insert " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            week(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("left " + ex.getMessage(), false);
            throw ex;
        }
        try
        {
            year(xRowRes);
        }
        catch (SQLException ex)
        {
            assure("right " + ex.getMessage(), false);
            throw ex;
        }
    }

    private XRow execute(final XRowSet xRowRes, final String sql) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class, xRowRes);
        xProp.setPropertyValue("Command", "SELECT " + sql + where);
        xRowRes.execute();
        final XResultSet xRes = UnoRuntime.queryInterface(XResultSet.class, xRowRes);
        assure("No valid row! ", xRes.next());

        return UnoRuntime.queryInterface(XRow.class, xRes);
    }

    private void dayofweek(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "DAYOFWEEK('1998-02-03') ");
        assure("DAYOFWEEK('1998-02-03') failed!", row.getInt(1) == 3);
    }

    private void dayofmonth(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "DAYOFMONTH('1998-02-03') ");
        assure("DAYOFMONTH('1998-02-03') failed!", row.getInt(1) == 3);
    }

    private void dayofyear(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "DAYOFYEAR('1998-02-03') ");
        assure("DAYOFYEAR('1998-02-03') failed!", row.getInt(1) == 34);
    }

    private void month(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "month('1998-02-03') ");
        assure("month('1998-02-03') failed!", row.getInt(1) == 2);
    }

    private void dayname(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "DAYNAME('1998-02-05') ");
        assure("DAYNAME('1998-02-05') failed!", row.getString(1).equals("Thursday"));
    }

    private void monthname(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "MONTHNAME('1998-02-05') ");
        assure("MONTHNAME('1998-02-05') failed!", row.getString(1).equals("February"));
    }

    private void quarter(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "QUARTER('98-01-01'),QUARTER('98-04-01'),QUARTER('98-07-01'),QUARTER('98-10-01') ");
        assure("QUARTER('98-01-01') failed!", row.getInt(1) == 1);
        assure("QUARTER('98-04-01') failed!", row.getInt(2) == 2);
        assure("QUARTER('98-07-01') failed!", row.getInt(3) == 3);
        assure("QUARTER('98-10-01') failed!", row.getInt(4) == 4);
    }

    private void week(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "WEEK('1998-02-20') ");
        assure("WEEK('1998-02-20') failed!", row.getInt(1) == 7);
    }

    private void year(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "YEAR('98-02-03') ");
        assure("YEAR('98-02-03') failed!", row.getInt(1) == 98);
    }

    private void hour(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "HOUR('10:05:03') ");
        assure("HOUR('10:05:03') failed!", row.getInt(1) == 10);
    }

    private void minute(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "MINUTE('98-02-03 10:05:03') ");
        assure("MINUTE('98-02-03 10:05:03') failed!", row.getInt(1) == 5);
    }

    private void second(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "SECOND('10:05:03') ");
        assure("SECOND('10:05:03') failed!", row.getInt(1) == 3);
    }

    private void curdate(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "CURDATE() ");
        final com.sun.star.util.Date aDate = row.getDate(1);
        getLog().println("CURDATE() is '" + aDate.Year + "-" + aDate.Month + "-" + aDate.Day + "'");
    }

    private void curtime(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "CURTIME() ");
        final com.sun.star.util.Time aTime = row.getTime(1);
        getLog().println("CURTIME() is '" + aTime.Hours + ":" + aTime.Minutes + ":" + aTime.Seconds + "'");
    }

    private void now(final XRowSet xRowRes) throws com.sun.star.uno.Exception, com.sun.star.beans.UnknownPropertyException
    {
        final XRow row = execute(xRowRes, "NOW() ");
        final com.sun.star.util.DateTime aTime = row.getTimestamp(1);
        getLog().println("NOW() is '" + aTime.Year + "-" + aTime.Month + "-" + aTime.Day + "'");
        getLog().println("'" + aTime.Hours + ":" + aTime.Minutes + ":" + aTime.Seconds + "'");
    }
}
