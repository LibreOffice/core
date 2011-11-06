/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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


