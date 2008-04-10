/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: GeneralTest.java,v $
 * $Revision: 1.3 $
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
