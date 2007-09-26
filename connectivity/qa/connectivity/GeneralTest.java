/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GeneralTest.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-26 14:26:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package complex.connectivity;

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
//import complex.connectivity.DBaseStringFunctions;

public class GeneralTest  extends ComplexTestCase {

    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public String getTestObjectName() {
        return "GeneralTest";
    }
    public void assure2(String s,boolean b){
        assure(s,b);
    }

    public void test() throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        try
        {
            XDriverManager driverManager = (XDriverManager)UnoRuntime.queryInterface(XDriverManager.class,((XMultiServiceFactory)param.getMSF()).createInstance("com.sun.star.sdbc.DriverManager"));
            String databaseURL = "sdbc:calc:singin' in the rain" ;
            XConnection catalogConnection = driverManager.getConnection(databaseURL);
            failed();
        }
        catch(SQLException e){}
    }
}
