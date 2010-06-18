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
package complex.olesimplestorage;

import com.sun.star.lang.XMultiServiceFactory;
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

public class OLESimpleStorageUnitTest /* extends ComplexTestCase */
{
    private XMultiServiceFactory m_xMSF = null;

//    public String[] getTestMethodNames() {
//        return new String[] {
//            "ExecuteTest01"};
//    }
//
//    public String getTestObjectName() {
//        return "OLESimpleStorageUnitTest";
//    }

    @Before public void before () {
        System.out.println("before()");
        try {
            m_xMSF = getMSF();
        } catch ( Exception e ){
            fail( "Cannot create service factory!" );
        }
        if ( m_xMSF == null ) {
            fail( "Cannot create service factory!" );
        }
    }

    @After public void after () {
        System.out.println("after()");
        m_xMSF = null;
    }

    @Test public void ExecuteTest01() {
        System.out.println("ExecuteTest01()");
        OLESimpleStorageTest aTest = new Test01( m_xMSF );
        assertTrue( "Test01 failed!", aTest.test() );
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
//        try
//        {
//            Thread.sleep(5000);
//        }
//        catch (java.lang.InterruptedException e)
//        {
//        }
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}