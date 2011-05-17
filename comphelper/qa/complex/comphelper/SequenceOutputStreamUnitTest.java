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
package complex.comphelper;

// import complexlib.ComplexTestCase;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.io.XSequenceOutputStream;
import com.sun.star.io.XSeekableInputStream;

import java.util.Random;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/* Document.
 */

class TestHelper
{
    // LogWriter m_aLogWriter;
    String m_sTestPrefix;

    /** Creates a new instance of TestHelper
     * @param sTestPrefix
     */
    public TestHelper ( String sTestPrefix ) {
        m_sTestPrefix = sTestPrefix;
    }

    public void Error ( String sError ) {
        System.out.println ( m_sTestPrefix + "Error: " + sError );
    }

    public void Message ( String sMessage ) {
        System.out.println ( m_sTestPrefix + sMessage );
    }
}

public class SequenceOutputStreamUnitTest /* extends ComplexTestCase*/ {
    private XMultiServiceFactory m_xMSF = null;

    TestHelper m_aTestHelper = null;

//    public String[] getTestMethodNames() {
//        return new String[] {
//            "ExecuteTest01"};
//    }

//    public String getTestObjectName () {
//        return "SequenceOutputStreamUnitTest";
//    }

//    public static String getShortTestDescription() {
//        return "tests the SequenceOutput/InputStream implementations";
//    }

    @Before public void before() {
        try {
            m_xMSF = getMSF();
            m_aTestHelper = new TestHelper ( "Test01: ");
        } catch (Exception e) {
            fail ("Cannot create service factory!");
        }
        if (m_xMSF==null) {
            fail ("Cannot create service factory!");
        }
    }

    @After public void after() {
        m_xMSF = null;
    }

//    @Test public void ExecuteTest01() {
//        Test01 aTest = new Test01 (m_xMSF);
//        assertTrue( "Test01 failed!", aTest.test() );
//    }

    @Test public void test () {
        try {
            final int nBytesCnt = 20;

            //create SequenceOutputStream
            Object oSequenceOutputStream = m_xMSF.createInstance (
                    "com.sun.star.io.SequenceOutputStream" );
            XSequenceOutputStream xSeqOutStream =
                    UnoRuntime.queryInterface (
                    XSequenceOutputStream.class, oSequenceOutputStream );
            m_aTestHelper.Message ( "SequenceOutputStream created." );

            //write something to the stream
            byte pBytesOriginal[] = new byte [nBytesCnt];
            Random oRandom = new Random();
            oRandom.nextBytes (pBytesOriginal);
            xSeqOutStream.writeBytes (pBytesOriginal);
            byte pBytesWritten[] = xSeqOutStream.getWrittenBytes ();
            m_aTestHelper.Message ( "SeuenceOutputStream filled." );

            //create SequenceInputstream
            Object pArgs[] = new Object[1];
            pArgs[0] = pBytesWritten;
            Object oSequenceInputStream = m_xMSF.createInstanceWithArguments (
                    "com.sun.star.io.SequenceInputStream", pArgs );
            XSeekableInputStream xSeekableInStream =
                    UnoRuntime.queryInterface (
                    XSeekableInputStream.class, oSequenceInputStream );
            m_aTestHelper.Message ( "SequenceInputStream created." );

            //read from the stream
            byte pBytesRead[][] = new byte [1][nBytesCnt];
            xSeekableInStream.readBytes ( pBytesRead, pBytesRead[0].length + 1 );
            m_aTestHelper.Message ( "Read from SequenceInputStream." );

            //close the streams
            xSeqOutStream.closeOutput ();
            xSeekableInStream.closeInput ();
            m_aTestHelper.Message ( "Both streams closed." );

            //compare the original, written and read arrys
            for ( int i = 0; i < nBytesCnt; ++i ) {
                if ( pBytesOriginal[i] != pBytesWritten[i] ) {
                    m_aTestHelper.Error ( "Written array not identical to " +
                            "original array. Position: " + i );
                    return /* false */;
                } else if ( pBytesOriginal[i] != pBytesRead[0][i] ) {
                    m_aTestHelper.Error ( "Read array not identical to original " +
                            "array. Position: " + i );
                    return /* false */;
                }
            }
            m_aTestHelper.Message ( "All data correct." );
        } catch ( Exception e ) {
            m_aTestHelper.Error ( "Exception: " + e );
            return /* false */;
        }
        return /* true */;
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
}