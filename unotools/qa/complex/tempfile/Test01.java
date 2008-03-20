/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Test01.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-20 15:27:27 $
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
package complex.tempfile;

import complexlib.ComplexTestCase;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.io.*;
import com.sun.star.uno.UnoRuntime;
import java.util.Random;

import share.LogWriter;

public class Test01 implements TempFileTest {
    LogWriter m_aLogWriter;
    XMultiServiceFactory m_xMSF = null;
    XSimpleFileAccess m_xSFA = null;
    TestHelper m_aTestHelper = null;

    public Test01(XMultiServiceFactory xMSF, XSimpleFileAccess xSFA, LogWriter aLogWriter) {
        m_xMSF = xMSF;
        m_xSFA = xSFA;
        m_aTestHelper = new TestHelper(aLogWriter, "Test01: ");
    }

    public boolean test() {
        XTempFile xTempFile = null;
        XTruncate xTruncate = null;
        String sFileURL = null;
        String sFileName = null;
        //create a temporary file.
        try {
            Object oTempFile = m_xMSF.createInstance( "com.sun.star.io.TempFile" );
            xTempFile = (XTempFile) UnoRuntime.queryInterface( XTempFile.class,
                    oTempFile );
            m_aTestHelper.Message( "Tempfile created." );
            xTruncate = (XTruncate)UnoRuntime.queryInterface( XTruncate.class,
                    oTempFile );
        } catch( Exception e ) {
            m_aTestHelper.Error( "Cannot create TempFile. exception: " + e );
            return false;
        }

        //retrieve the tempfile URL
        if ( xTempFile == null ) {
            m_aTestHelper.Error( "Cannot get XTempFile interface." );
            return false;
        }

        try {
            //compare the file name with the name in the URL.
            sFileURL = m_aTestHelper.GetTempFileURL( xTempFile );
            sFileName = m_aTestHelper.GetTempFileName( xTempFile );
            m_aTestHelper.CompareFileNameAndURL( sFileName, sFileURL );

            //write to the stream using the service.
            byte pBytesIn[] = new byte[9];
            byte pBytesOut1[][] = new byte [1][9];
            byte pBytesOut2[][] = new byte [1][9];
            Random oRandom = new Random();
            oRandom.nextBytes( pBytesIn );
            m_aTestHelper.WriteBytesWithStream( pBytesIn, xTempFile );

            //check the result by reading from the service.
            xTempFile.seek(0);
            m_aTestHelper.ReadBytesWithStream( pBytesOut1, pBytesIn.length + 1, xTempFile );
            for ( int i = 0; i < pBytesIn.length ; i++ ) {
                if ( pBytesOut1[0][i] != pBytesIn[i] ) {
                    m_aTestHelper.Error( "Tempfile outputs false data!" );
                }
            }

            //check the result by reading from the file directly.
            m_aTestHelper.ReadDirectlyFromTempFile( pBytesOut2, pBytesIn.length + 1, m_xSFA, sFileURL );
            for ( int i = 0; i < pBytesIn.length; i++ ) {
                if ( pBytesOut2[0][i] != pBytesIn[i] ) {
                    m_aTestHelper.Error( "Tempfile contains false data!" );
                }
            }

            //close the object(by closing input and output), check that the file was removed.
            xTempFile.setRemoveFile( false );
            m_aTestHelper.CloseTempFile( xTempFile );
            if( !m_aTestHelper.IfTempFileExists( m_xSFA, sFileURL ) ) {
                m_aTestHelper.Error( "TempFile mistakenly removed. " );
            } else {
                m_aTestHelper.KillTempFile( sFileURL, m_xSFA );
            }
        } catch ( Exception e ) {
            m_aTestHelper.Error( "Exception: " + e );
            return false;
        }
        return true;
    }
}
