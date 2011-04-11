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


import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.io.*;

import com.sun.star.uno.UnoRuntime;
import java.util.Random;

public class Test02 implements TempFileTest {

    XMultiServiceFactory m_xMSF;
    XSimpleFileAccess m_xSFA;
    TestHelper m_aTestHelper;

    public Test02(XMultiServiceFactory xMSF, XSimpleFileAccess xSFA) {
        m_xMSF = xMSF;
        m_xSFA = xSFA;
        m_aTestHelper = new TestHelper( "Test02: ");
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
            xTempFile = UnoRuntime.queryInterface(XTempFile.class, oTempFile);
            m_aTestHelper.Message( "Tempfile created." );
            xTruncate = UnoRuntime.queryInterface(XTruncate.class, oTempFile);
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
