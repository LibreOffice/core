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

package complex.olesimplestorage;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.io.XTempFile;
import com.sun.star.embed.XOLESimpleStorage;
import com.sun.star.uno.UnoRuntime;

import java.util.Random;


public class Test01 implements OLESimpleStorageTest
{
    XMultiServiceFactory m_xMSF = null;
    TestHelper m_aTestHelper = null;
    final int pStreamCnt = 5;
    final int pBytesCnt = 10;

    public Test01 ( XMultiServiceFactory xMSF )
    {
        m_xMSF = xMSF;
        m_aTestHelper = new TestHelper ("Test01: ");
    }

    public boolean test ()
    {
        try
        {
            //create a new temporary stream
            Object oTempFile = m_xMSF.createInstance ( "com.sun.star.io.TempFile" );
            XTempFile xTempFile = UnoRuntime.queryInterface(XTempFile.class, oTempFile);
            m_aTestHelper.Message ( "A new temporary stream created." );

            //create OLESimpleStorage based on it
            Object pArgs[] = new Object[2];
            pArgs[0] = (Object) xTempFile;
            pArgs[1] = new Boolean( true );
            Object oOLESimpleStorage = m_xMSF.createInstanceWithArguments ( "com.sun.star.embed.OLESimpleStorage", pArgs );
            XOLESimpleStorage xOLESimpleStorage = UnoRuntime.queryInterface(XOLESimpleStorage.class, oOLESimpleStorage);
            m_aTestHelper.Message ( "OLESimpleStorage based on XStream created." );

            //fill it with some streams
            Object oStream[] = new Object[pStreamCnt];
            byte pBytesIn[][][] = new byte [pStreamCnt][1][pBytesCnt];
            byte pBytesOut[][] = new byte [pStreamCnt][pBytesCnt];
            XTempFile xTempStream[] = new XTempFile[pStreamCnt];
            Random oRandom = new Random ();
            final String sSubStreamPrefix = "SubStream";
            for ( int i = 0; i < pStreamCnt; i++ )
            {
                oRandom.nextBytes (pBytesOut[i]);
                oStream[i] = m_xMSF.createInstance ( "com.sun.star.io.TempFile" );
                xTempStream[i] = UnoRuntime.queryInterface(XTempFile.class, oStream[i]);
                xTempStream[i].getOutputStream ().writeBytes (pBytesOut[i]);
                xTempStream[i].seek (0);
                m_aTestHelper.Message ( "Substream " + i + " initialized." );
                if (xOLESimpleStorage.hasByName (sSubStreamPrefix + i))
                {
                    xOLESimpleStorage.replaceByName ( sSubStreamPrefix + i, xTempStream[i] );
                }
                else
                {
                    xOLESimpleStorage.insertByName ( sSubStreamPrefix + i, xTempStream[i] );
                    m_aTestHelper.Message ( "Substream " + i + " inserted." );
                }
            }

            //commit the storage and close it
            xOLESimpleStorage.commit ();
            m_aTestHelper.Message ( "Storage commited." );
            xOLESimpleStorage.dispose ();
            for ( int i = 0; i < pStreamCnt; ++i )
            {
                xTempStream[i].setRemoveFile ( true );
                xTempStream[i].getInputStream ().closeInput ();
                xTempStream[i].getOutputStream ().closeOutput ();
            }
            m_aTestHelper.Message ( "Storage closed." );

            //open the same stream with the constructor for inputstream
            pArgs[0] = (Object)xTempFile.getInputStream ();
            oOLESimpleStorage = m_xMSF.createInstanceWithArguments ( "com.sun.star.embed.OLESimpleStorage", pArgs );
            xOLESimpleStorage = UnoRuntime.queryInterface(XOLESimpleStorage.class, oOLESimpleStorage);
            m_aTestHelper.Message ( "Storage reopened, based on XInputStream." );

            //check that all the streams contain correct information
            m_aTestHelper.Message ( "Checking data contained in all the substreams..." );
            for ( int i = 0; i < pStreamCnt; ++i )
            {
                if ( xOLESimpleStorage.hasByName (sSubStreamPrefix + i) )
                {
                    xTempStream[i] = UnoRuntime.queryInterface(XTempFile.class, xOLESimpleStorage.getByName(sSubStreamPrefix + i));
                    xTempStream[i].seek (0);
                    xTempStream[i].getInputStream ().readBytes (pBytesIn[i], pBytesIn[i][0].length + 1 );
                    for ( int j = 0; j < pBytesCnt; ++j )
                    {
                        if ( pBytesIn[i][0][j] != pBytesOut[i][j] )
                        {
                            m_aTestHelper.Error ( "Stream " + i + " byte " + j + ": INCORRECT DATA!");
                            return false;
                        }
                        else
                        {
                            m_aTestHelper.Message ( "Stream " + i + " byte " + j + ":  CORRECT." );
                        }
                    }
                }
                else
                {
                    m_aTestHelper.Error( "Stream " + i + " is lost!");
                    return false;
                }
            }
            m_aTestHelper.Message ( "All substreams contain correct data. SUCCESS." );
        }
        catch ( Exception e )
        {
            m_aTestHelper.Error ( "Exception: " + e );
            return false;
        }
        return true;
    }
}
