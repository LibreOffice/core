/*************************************************************************
 *
 *  $RCSfile: DBaseDateFunctions.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:22:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
package complex.connectivity;

import complex.connectivity.DBaseDriverTest;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import com.sun.star.sdbc.*;
import com.sun.star.sdb.*;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;

import com.sun.star.lang.XMultiServiceFactory;

import complexlib.ComplexTestCase;

import java.io.PrintWriter;

import util.utils;
import java.util.*;
import java.io.*;


public class DBaseDateFunctions {

        private String where = "FROM \"biblio\" \"biblio\" where \"Identifier\" = 'BOR02b'";
        private XMultiServiceFactory m_xORB;
        private DBaseDriverTest testcase;
        public DBaseDateFunctions(XMultiServiceFactory _xORB,DBaseDriverTest _testcase){
            m_xORB = _xORB;
            testcase = _testcase;
        }

        private void assure(String s,boolean b){
            testcase.assure2(s,b);
        }

    public void testFunctions() throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRowSet xRowRes = (XRowSet)UnoRuntime.queryInterface(XRowSet.class,
                            m_xORB.createInstance("com.sun.star.sdb.RowSet"));

        System.out.println("starting DateTime function test!");
        // set the properties needed to connect to a database
        XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class,xRowRes);
        xProp.setPropertyValue("DataSourceName","Bibliography");

        xProp.setPropertyValue("CommandType",new Integer(com.sun.star.sdb.CommandType.COMMAND));

        try {
            curdate(xRowRes);
        } catch( SQLException ex){
            assure("upper " + ex.getMessage(),false);
            throw ex;
        }
        try{
            curtime(xRowRes);
        } catch( SQLException ex){
            assure("lower " + ex.getMessage(),false);
            throw ex;
        }
        try{
            dayname(xRowRes);
        } catch( SQLException ex){
            assure("ascii " + ex.getMessage(),false);
            throw ex;
        }
        try{
            dayofmonth(xRowRes);
        } catch( SQLException ex){
            assure("char_len " + ex.getMessage(),false);
            throw ex;
        }
        try{
            dayofweek(xRowRes);
        } catch( SQLException ex){
            assure("concat " + ex.getMessage(),false);
            throw ex;
        }
        try{
            dayofyear(xRowRes);
        } catch( SQLException ex){
            assure("locate " + ex.getMessage(),false);
            throw ex;
        }
        try{
            hour(xRowRes);
        } catch( SQLException ex){
            assure("substr " + ex.getMessage(),false);
            throw ex;
        }
        try{
            minute(xRowRes);
        } catch( SQLException ex){
            assure("ltrim " + ex.getMessage(),false);
            throw ex;
        }
        try{
            month(xRowRes);
        } catch( SQLException ex){
            assure("rtrim " + ex.getMessage(),false);
            throw ex;
        }
        try{
            monthname(xRowRes);
        } catch( SQLException ex){
            assure("space " + ex.getMessage(),false);
            throw ex;
        }
        try{
            now(xRowRes);
        } catch( SQLException ex){
            assure("replace " + ex.getMessage(),false);
            throw ex;
        }
        try{
            quarter(xRowRes);
        } catch( SQLException ex){
            assure("repeat " + ex.getMessage(),false);
            throw ex;
        }
        try{
            second(xRowRes);
        } catch( SQLException ex){
            assure("insert " + ex.getMessage(),false);
            throw ex;
        }
        try{
            week(xRowRes);
        } catch( SQLException ex){
            assure("left " + ex.getMessage(),false);
            throw ex;
        }
        try{
            year(xRowRes);
        } catch( SQLException ex){
            assure("right " + ex.getMessage(),false);
            throw ex;
        }
    }

    private XRow execute(XRowSet xRowRes,String sql) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class,xRowRes);
        xProp.setPropertyValue("Command","SELECT " + sql + where);
        xRowRes.execute();
        XResultSet xRes = (XResultSet)UnoRuntime.queryInterface(XResultSet.class,xRowRes);
        assure("No valid row! ",xRes.next());

        return (XRow)UnoRuntime.queryInterface(XRow.class, xRes);
    }

    private void dayofweek(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"DAYOFWEEK('1998-02-03') ");
        assure("DAYOFWEEK('1998-02-03') failed!",row.getInt(1) == 3);
    }
    private void dayofmonth(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"DAYOFMONTH('1998-02-03') ");
        assure("DAYOFMONTH('1998-02-03') failed!",row.getInt(1) == 3);
    }
    private void dayofyear(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"DAYOFYEAR('1998-02-03') ");
        assure("DAYOFYEAR('1998-02-03') failed!",row.getInt(1) == 34);
    }
    private void month(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"month('1998-02-03') ");
        assure("month('1998-02-03') failed!",row.getInt(1) == 2);
    }
    private void dayname(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"DAYNAME('1998-02-05') ");
        assure("DAYNAME('1998-02-05') failed!",row.getString(1).equals("Thursday"));
    }
    private void monthname(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"MONTHNAME('1998-02-05') ");
        assure("MONTHNAME('1998-02-05') failed!",row.getString(1).equals("February"));
    }
    private void quarter(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"QUARTER('98-01-01'),QUARTER('98-04-01'),QUARTER('98-07-01'),QUARTER('98-10-01') ");
        assure("QUARTER('98-01-01') failed!",row.getInt(1) == 1);
        assure("QUARTER('98-04-01') failed!",row.getInt(2) == 2);
        assure("QUARTER('98-07-01') failed!",row.getInt(3) == 3);
        assure("QUARTER('98-10-01') failed!",row.getInt(4) == 4);
    }
    private void week(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"WEEK('1998-02-20') ");
        assure("WEEK('1998-02-20') failed!",row.getInt(1) == 7);
    }
    private void year(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"YEAR('98-02-03') ");
        assure("YEAR('98-02-03') failed!",row.getInt(1) == 98);
    }
    private void hour(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"HOUR('10:05:03') ");
        assure("HOUR('10:05:03') failed!",row.getInt(1) == 10);
    }
    private void minute(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"MINUTE('98-02-03 10:05:03') ");
        assure("MINUTE('98-02-03 10:05:03') failed!",row.getInt(1) == 5);
    }
    private void second(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"SECOND('10:05:03') ");
        assure("SECOND('10:05:03') failed!",row.getInt(1) == 3);
    }
    private void curdate(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"CURDATE() ");
        com.sun.star.util.Date aDate = row.getDate(1);
        System.out.println("CURDATE() is '" + aDate.Year + "-" + aDate.Month + "-" + aDate.Day +"'");
    }
    private void curtime(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"CURTIME() ");
        com.sun.star.util.Time aTime = row.getTime(1);
        System.out.println("CURTIME() is '" + aTime.Hours + ":" + aTime.Minutes + ":" + aTime.Seconds + "'");
    }
    private void now(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"NOW() ");
        com.sun.star.util.DateTime aTime = row.getTimestamp(1);
        System.out.println("NOW() is '" + aTime.Year + "-" + aTime.Month + "-" + aTime.Day + "'");
        System.out.println("'" + aTime.Hours + ":" + aTime.Minutes + ":" + aTime.Seconds + "'");
    }
}
