/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Test02.java,v $
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
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;
import java.util.Random;
import share.LogWriter;

public class Test02 implements TempFileTest {

    XMultiServiceFactory m_xMSF;
    XSimpleFileAccess m_xSFA;
    TestHelper m_aTestHelper;

    public Test02(XMultiServiceFactory xMSF, XSimpleFileAccess xSFA, LogWriter aLogWriter) {
        m_xMSF = xMSF;
        m_xSFA = xSFA;
        m_aTestHelper = new TestHelper(aLogWriter, "Test02: ");
    }

    public boolean test() {
        Object oTempFile = null;
        XTempFile xTempFile = null;
        XTruncate xTruncate = null;
        String sFileURL = null;
        String sFileName = null;
        //create a temporary file.
        try {
            oTempFile = m_xMSF.createInstance( "com.sun.star.io.TempFile" );
            xTempFile = (XTempFile) UnoRuntime.queryInterface( XTempFile.class,
                    oTempFile );
            m_aTestHelper.Message( "Tempfile created." );
            xTruncate = (XTruncate)UnoRuntime.queryInterface( XTruncate.class,
                    oTempFile );
        } catch(Exception e) {
            m_aTestHelper.Error( "Cannot create TempFile. exception: " + e );
            return false;
        }
        try {
            //write something.
            byte pBytesIn[] = new byte[9];
            byte pBytesOut[][] = new byte[1][9];
            Random oRandom = new Random();
            oRandom.nextBytes( pBytesIn );
            m_aTestHelper.WriteBytesWithStream( pBytesIn, xTempFile );

            //get the URL.
            sFileURL = m_aTestHelper.GetTempFileURL( xTempFile );

            //let the service not to remove the URL.
            m_aTestHelper.SetTempFileRemove( xTempFile, false );

            //close the tempfile by closing input and output.
            m_aTestHelper.CloseTempFile( xTempFile );

            //check that the file is still available.
            //xTempFile.seek(0);
            m_aTestHelper.ReadDirectlyFromTempFile( pBytesOut, pBytesIn.length + 1, m_xSFA, sFileURL );
            for ( int i = 0; i < pBytesIn.length; i++ ) {
                if ( pBytesOut[0][i] != pBytesIn[i] ) {
                    m_aTestHelper.Error( "Tempfile contains false data!" );
                }
            }
        } catch ( Exception e) {
            m_aTestHelper.Error("Exception: " + e);
            return false;
        }
        return true;
    }
}
