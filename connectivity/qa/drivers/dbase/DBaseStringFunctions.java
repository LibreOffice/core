/*************************************************************************
 *
 *  $RCSfile: DBaseStringFunctions.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:22:49 $
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


public class DBaseStringFunctions {

        private String where = "FROM \"biblio\" \"biblio\" where \"Identifier\" = 'BOR02b'";
        private XMultiServiceFactory m_xORB;
        private DBaseDriverTest testcase;
        public DBaseStringFunctions(XMultiServiceFactory _xORB,DBaseDriverTest _testcase){
            m_xORB = _xORB;
            testcase = _testcase;
        }

        private void assure(String s,boolean b){
            testcase.assure2(s,b);
        }

    public void testFunctions() throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRowSet xRowRes = (XRowSet)UnoRuntime.queryInterface(XRowSet.class,
                            m_xORB.createInstance("com.sun.star.sdb.RowSet"));

        System.out.println("starting String function test");
        // set the properties needed to connect to a database
        XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class,xRowRes);
        xProp.setPropertyValue("DataSourceName","Bibliography");

        xProp.setPropertyValue("CommandType",new Integer(com.sun.star.sdb.CommandType.COMMAND));

        try {
        upper(xRowRes);
        } catch( SQLException ex){
            assure("upper " + ex.getMessage(),false);
            throw ex;
        }
        try{
        lower(xRowRes);
        } catch( SQLException ex){
            assure("lower " + ex.getMessage(),false);
            throw ex;
        }
        try{
        acsii(xRowRes);
        } catch( SQLException ex){
            assure("ascii " + ex.getMessage(),false);
            throw ex;
        }
        try{
        char_length(xRowRes);
        } catch( SQLException ex){
            assure("char_len " + ex.getMessage(),false);
            throw ex;
        }
        try{
        concat(xRowRes);
        } catch( SQLException ex){
            assure("concat " + ex.getMessage(),false);
            throw ex;
        }
        try{
            chartest(xRowRes);
        } catch( SQLException ex){
            assure("char " + ex.getMessage(),false);
            throw ex;
        }
        try{
        locate(xRowRes);
        } catch( SQLException ex){
            assure("locate " + ex.getMessage(),false);
            throw ex;
        }
        try{
        substring(xRowRes);
        } catch( SQLException ex){
            assure("substr " + ex.getMessage(),false);
            throw ex;
        }
        try{
        ltrim(xRowRes);
        } catch( SQLException ex){
            assure("ltrim " + ex.getMessage(),false);
            throw ex;
        }
        try{
        rtrim(xRowRes);
        } catch( SQLException ex){
            assure("rtrim " + ex.getMessage(),false);
            throw ex;
        }
        try{
        space(xRowRes);
        } catch( SQLException ex){
            assure("space " + ex.getMessage(),false);
            throw ex;
        }
        try{
        replace(xRowRes);
        } catch( SQLException ex){
            assure("replace " + ex.getMessage(),false);
            throw ex;
        }
        try{
        repeat(xRowRes);
        } catch( SQLException ex){
            assure("repeat " + ex.getMessage(),false);
            throw ex;
        }
        try{
        insert(xRowRes);
        } catch( SQLException ex){
            assure("insert " + ex.getMessage(),false);
            throw ex;
        }
        try{
        left(xRowRes);
        } catch( SQLException ex){
            assure("left " + ex.getMessage(),false);
            throw ex;
        }
        try{
        right(xRowRes);
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

    private void upper(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"upper('test'),UCASE('test') ");
        assure("upper('test') failed!",row.getString(1).equals("TEST"));
        assure("ucase('test') failed!",row.getString(2).equals("TEST"));
    }

    private void lower(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"lower('TEST'),LCASE('TEST') ");
        assure("lower('TEST') failed!",row.getString(1).equals("test"));
        assure("lcase('TEST') failed!",row.getString(2).equals("test"));
    }

    private void acsii(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"ASCII('2') ");
        assure("acsii('2') failed!",row.getInt(1) == 50);
    }

    private void char_length(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"char_length('test'),character_length('test'),OCTET_LENGTH('test') ");
        assure("char_length('test') failed!",row.getInt(1) == 4);
        assure("character_length('test') failed!",row.getInt(2) == 4);
        assure("OCTET_LENGTH('test') failed!",row.getInt(3) == 4);
    }

    private void concat(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"CONCAT('Hello',' ','World') ");
        assure("CONCAT('Hello',' ',,'World') failed!",row.getString(1).equals("Hello World"));
    }

    private void locate(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"LOCATE('bar', 'foobarbar') ");
        assure("LOCATE('bar', 'foobarbar') failed!",row.getInt(1) == 4);
    }

    private void substring(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"SUBSTRING('Quadratically',5) ");
        assure("SUBSTRING('Quadratically',5) failed!",row.getString(1).equals("ratically"));
    }

    private void ltrim(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"LTRIM('  barbar') ");
        assure("LTRIM('  barbar') failed!",row.getString(1).equals("barbar"));
    }

    private void rtrim(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"RTRIM('barbar  ') ");
        assure("RTRIM('barbar  ') failed!",row.getString(1).equals( "barbar"));
    }

    private void space(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"space(6) ");
        assure("space(6) failed!",row.getString(1).equals("      "));
    }

    private void replace(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"REPLACE('www.OOo.com', 'w', 'Ww') ");
        assure("REPLACE('www.OOo.com', 'w', 'Ww') failed!",row.getString(1).equals("WwWwWw.OOo.com"));
    }

    private void repeat(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"REPEAT('OOo', 3) ");
        assure("REPEAT('OOo', 3) failed!",row.getString(1).equals("OOoOOoOOo"));
    }

    private void insert(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"INSERT('Quadratic', 3, 4, 'What') ");
        assure("INSERT('Quadratic', 3, 4, 'What') failed!",row.getString(1).equals("QuWhattic"));
    }

    private void left(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"LEFT('foobarbar', 5) ");
        assure("LEFT('foobarbar', 5) failed!",row.getString(1).equals("fooba"));
    }

    private void right(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"RIGHT('foobarbar', 4) ");
        assure("RIGHT('foobarbar', 4) failed!",row.getString(1).equals("rbar"));
    }
    private void chartest(XRowSet xRowRes) throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        XRow row = execute(xRowRes,"CHAR(ascii('t'),ascii('e'),ascii('s'),ascii('t')) ");
        assure("CHAR(ascii('t'),ascii('e'),ascii('s'),ascii('t')) failed!",row.getString(1).equals("test"));
    }
}
