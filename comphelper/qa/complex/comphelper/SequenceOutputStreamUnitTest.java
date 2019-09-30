/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package complex.comphelper;

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

public class SequenceOutputStreamUnitTest
{
    private XMultiServiceFactory m_xMSF = null;

    @Before public void before() {
        try {
            m_xMSF = getMSF();
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

    @Test public void test () {
        try {
            final int nBytesCnt = 20;

            //create SequenceOutputStream
            Object oSequenceOutputStream = m_xMSF.createInstance (
                    "com.sun.star.io.SequenceOutputStream" );
            XSequenceOutputStream xSeqOutStream =
                    UnoRuntime.queryInterface (
                    XSequenceOutputStream.class, oSequenceOutputStream );

            //write something to the stream
            byte pBytesOriginal[] = new byte [nBytesCnt];
            Random oRandom = new Random();
            oRandom.nextBytes (pBytesOriginal);
            xSeqOutStream.writeBytes (pBytesOriginal);

            // Append the same content once again
            xSeqOutStream.writeBytes (pBytesOriginal);

            byte pBytesWritten[] = xSeqOutStream.getWrittenBytes ();
            assertTrue( "SequenceOutputStream::getWrittenBytes() - wrong amount of bytes returned",
                    pBytesWritten.length == nBytesCnt * 2 );

            //create SequenceInputstream
            Object pArgs[] = new Object[1];
            pArgs[0] = pBytesWritten;
            Object oSequenceInputStream = m_xMSF.createInstanceWithArguments (
                    "com.sun.star.io.SequenceInputStream", pArgs );
            XSeekableInputStream xSeekableInStream =
                    UnoRuntime.queryInterface (
                    XSeekableInputStream.class, oSequenceInputStream );

            //read from the stream
            byte pBytesRead[][] = new byte [1][nBytesCnt*2];
            int nBytesCountRead = xSeekableInStream.readBytes ( pBytesRead, pBytesRead[0].length + 1 );

            assertTrue( "SequenceInputStream::readBytes() - wrong amount of bytes returned " + pBytesRead[0].length + " vs " + (nBytesCountRead),
                    pBytesRead[0].length == nBytesCountRead);

            //close the streams
            xSeqOutStream.closeOutput ();
            xSeekableInStream.closeInput ();

            //compare the original, written and read arrays
            for ( int i = 0; i < nBytesCnt * 2; ++i ) {
                assertTrue( "Written array not identical to original array. Position: " + i,
                    pBytesOriginal[i % nBytesCnt] == pBytesWritten[i] );
                assertTrue( "Read array not identical to original array. Position: " + i,
                    pBytesOriginal[i % nBytesCnt] == pBytesRead[0][i] );
            }
        } catch ( Exception e ) {
            fail ( "Exception: " + e );
        }
    }

    private static XMultiServiceFactory getMSF()
    {
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
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
