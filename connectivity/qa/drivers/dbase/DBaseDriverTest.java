/*************************************************************************
 *
 *  $RCSfile: DBaseDriverTest.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:22:32 $
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

public class DBaseDriverTest  extends ComplexTestCase {

    private static Properties props = new Properties();
    private XDriver m_xDiver;
        private String where = "FROM \"biblio\" \"biblio\" where \"Identifier\" = 'BOR02b'";

    static {
        try {
            String propsFile = "test.properties";
            props.load( new FileInputStream(propsFile) );
        } catch(Exception ex) {
            throw new RuntimeException(ex);
        }
    }

    public String[] getTestMethodNames() {
        return new String[] { "Functions" };
    }

    public String getTestObjectName() {
        return "DBaseDriverTest";
    }
    public void assure2(String s,boolean b){
        assure(s,b);
    }

    public void Functions() throws com.sun.star.uno.Exception,com.sun.star.beans.UnknownPropertyException {
        DBaseStringFunctions aStringTest = new DBaseStringFunctions(((XMultiServiceFactory)param.getMSF()),this);
        aStringTest.testFunctions();

        DBaseNumericFunctions aNumericTest = new DBaseNumericFunctions(((XMultiServiceFactory)param.getMSF()),this);
        aNumericTest.testFunctions();

        DBaseDateFunctions aDateTest = new DBaseDateFunctions(((XMultiServiceFactory)param.getMSF()),this);
        aDateTest.testFunctions();
    }
}
