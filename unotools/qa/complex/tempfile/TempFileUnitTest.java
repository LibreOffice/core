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
package complex.tempfile;

// import complexlib.ComplexTestCase;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/* Document.
 */

public class TempFileUnitTest /* extends ComplexTestCase */ {
    private XMultiServiceFactory m_xMSF = null;
    private XSimpleFileAccess m_xSFA = null;

//    public String[] getTestMethodNames() {
//        return new String[] {
//            "ExecuteTest01",
//            "ExecuteTest02"};
//    }
//
//    public String getTestObjectName() {
//        return "TempFileUnitTest";
//    }

    @Before public void before() {
        m_xMSF = getMSF();
        if ( m_xMSF == null ) {
            fail ( "Cannot create service factory!" );
        }
        try
        {
            Object oSFA = m_xMSF.createInstance( "com.sun.star.ucb.SimpleFileAccess" );
            m_xSFA = UnoRuntime.queryInterface( XSimpleFileAccess.class, oSFA );
        }
        catch ( Exception e )
        {
            fail ( "Cannot get simple file access! Exception: " + e);
        }
        if ( m_xSFA == null ) {
            fail ( "Cannot get simple file access!" );
        }
    }

    @After public void after() {
        m_xMSF = null;
        m_xSFA = null;
    }

    @Test public void ExecuteTest01() {
        TempFileTest aTest = new Test01( m_xMSF, m_xSFA );
        assertTrue( "Test01 failed!", aTest.test() );
    }

    @Test public void ExecuteTest02() {
        TempFileTest aTest = new Test02( m_xMSF, m_xSFA );
        assertTrue( "Test02 failed!", aTest.test() );
    }

    private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();
};

