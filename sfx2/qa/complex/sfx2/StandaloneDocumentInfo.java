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
package complex.sfx2;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import complex.sfx2.standalonedocinfo.StandaloneDocumentInfoTest;
import complex.sfx2.standalonedocinfo.Test01;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/* Document here
*/

public class StandaloneDocumentInfo {
    private XMultiServiceFactory m_xMSF = null;

    @Before public void before() {
        try {
            m_xMSF = getMSF();
        } catch(Exception e) {
            fail( "Failed to create service factory!" );
        }
        if( m_xMSF ==null ) {
            fail( "Failed to create service factory!" );
        }
    }

    @After public void after() {
        m_xMSF = null;
    }

    @Test public void ExecuteTest01() {
        StandaloneDocumentInfoTest aTest = new Test01 (m_xMSF);
        assertTrue( "Test01 failed!", aTest.test() );
    }




      private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception
    {
        System.out.println( "------------------------------------------------------------" );
        System.out.println( "starting class: " + StandaloneDocumentInfo.class.getName() );
        System.out.println( "------------------------------------------------------------" );
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println( "------------------------------------------------------------" );
        System.out.println( "finishing class: " + StandaloneDocumentInfo.class.getName() );
        System.out.println( "------------------------------------------------------------" );
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}


