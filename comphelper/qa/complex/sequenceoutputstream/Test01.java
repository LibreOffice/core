/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Test01.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-04 16:35:59 $
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
package complex.sequenceoutputstream;

import complexlib.ComplexTestCase;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.io.XSequenceOutputStream;
import com.sun.star.io.XSeekableInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XInputStream;
import com.sun.star.uno.UnoRuntime;

import java.util.Random;
import share.LogWriter;

public class Test01 implements SequenceOutputStreamTest {
    XMultiServiceFactory m_xMSF = null;
    TestHelper m_aTestHelper = null;

    public Test01 ( XMultiServiceFactory xMSF, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_aTestHelper = new TestHelper (aLogWriter, "Test01: ");
    }


    public boolean test () {
        try {
            final int nBytesCnt = 20;

            //create SequenceOutputStream
            Object oSequenceOutputStream = m_xMSF.createInstance (
                    "com.sun.star.io.SequenceOutputStream" );
            XSequenceOutputStream xSeqOutStream =
                    (XSequenceOutputStream) UnoRuntime.queryInterface (
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
                    (XSeekableInputStream)UnoRuntime.queryInterface (
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
                    return false;
                } else if ( pBytesOriginal[i] != pBytesRead[0][i] ) {
                    m_aTestHelper.Error ( "Read array not identical to original " +
                            "array. Position: " + i );
                    return false;
                }
            }
            m_aTestHelper.Message ( "All data correct." );
        } catch ( Exception e ) {
            m_aTestHelper.Error ( "Exception: " + e );
            return false;
        }
        return true;
    }
}