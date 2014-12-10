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
package complex.tempfile;


import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.io.*;

import com.sun.star.uno.UnoRuntime;
import java.util.Random;

public class Test02 {

    private XMultiServiceFactory m_xMSF;
    private XSimpleFileAccess m_xSFA;
    private TestHelper m_aTestHelper;

    public Test02(XMultiServiceFactory xMSF, XSimpleFileAccess xSFA) {
        m_xMSF = xMSF;
        m_xSFA = xSFA;
        m_aTestHelper = new TestHelper( "Test02: ");
    }

    public boolean test() {
        Object oTempFile = null;
        XTempFile xTempFile = null;
        String sFileURL = null;
        //create a temporary file.
        try {
            oTempFile = m_xMSF.createInstance( "com.sun.star.io.TempFile" );
            xTempFile = UnoRuntime.queryInterface(XTempFile.class, oTempFile);
            m_aTestHelper.Message( "Tempfile created." );
            UnoRuntime.queryInterface(XTruncate.class, oTempFile);
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
