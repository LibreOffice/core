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

package complex.storages;

import java.lang.Integer;

import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.io.XStream;
import com.sun.star.io.XInputStream;

import com.sun.star.embed.*;

import share.LogWriter;
import complex.storages.TestHelper;
import complex.storages.StorageTest;
import complex.storages.BorderedStream;

public class RegressionTest_125919 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    int nMinTestLen = 0;
    int nMaxTestLen = 60000;

    public RegressionTest_125919( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "RegressionTest_125919: " );
    }

    public boolean test()
    {
        try
        {
            byte[] pBytes0 = new byte[0];
            byte[] pBytes18 = new byte[18000];
            byte[] pBytes36 = new byte[36000];

            for ( int nInitInd = 0; nInitInd < 36000; nInitInd++ )
            {
                pBytes36[nInitInd] = ( Integer.valueOf( nInitInd >> ( ( nInitInd % 2 ) * 8 ) ) ).byteValue();
                if ( nInitInd < 18000 )
                    pBytes18[nInitInd] = ( Integer.valueOf( 256  - pBytes36[nInitInd] ) ).byteValue();
            }

            System.out.println( "This test can take up to some hours. The file size currently is about 50000." );
            System.out.println( "Progress: " );
            for ( int nAvailableBytes = nMinTestLen; nAvailableBytes < nMaxTestLen; nAvailableBytes++ )
            {
                Object oBStream = new BorderedStream( nAvailableBytes );
                XStream xBorderedStream = (XStream)UnoRuntime.queryInterface( XStream.class, oBStream );
                if ( xBorderedStream == null )
                {
                    m_aTestHelper.Error( "Can't create bordered stream!" );
                    return false;
                }

                // create storage based on the temporary stream
                Object pArgs[] = new Object[2];
                pArgs[0] = (Object) xBorderedStream;
                pArgs[1] = Integer.valueOf( ElementModes.WRITE );

                Object oTempStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
                XStorage xTempStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
                if ( xTempStorage == null )
                {
                    m_aTestHelper.Error( "Can't create temporary storage representation!" );
                    return false;
                }

                XTransactedObject xTransact = (XTransactedObject) UnoRuntime.queryInterface( XTransactedObject.class, xTempStorage );
                if ( xTransact == null )
                {
                    m_aTestHelper.Error( "This test is designed for storages in transacted mode!" );
                    return false;
                }


                if ( !m_aTestHelper.WriteBytesToSubstream( xTempStorage, "SubStream" + 0, "MediaType1", true, pBytes0 ) )
                    return false;
                if ( !m_aTestHelper.WriteBytesToSubstream( xTempStorage, "SubStream" + 18, "MediaType2", true, pBytes18 ) )
                    return false;
                if ( !m_aTestHelper.WriteBytesToSubstream( xTempStorage, "SubStream" + 36, "MediaType3", true, pBytes36 ) )
                    return false;

                if ( nAvailableBytes > 0 && nAvailableBytes % 100 == 0 )
                    System.out.println( " " + nAvailableBytes );

                if ( nAvailableBytes > 0 && nAvailableBytes % 2 == 1 )
                    System.out.print( "#" );

                try
                {
                    xTransact.commit();

                    System.out.println( "" );
                    if ( !m_aTestHelper.disposeStorage( xTempStorage ) )
                        return false;

                    // SUCCESS
                    return true;
                }
                catch( UseBackupException aExc )
                {
                    // when there is not enough place in the target location and the target file is empty
                    // the direct writing will fail and must throw this exception with empty URL
                    if ( aExc.TemporaryFileURL.length() != 0 )
                        return false;
                }
                catch( Exception e )
                {
                    System.out.println( "" );
                    m_aTestHelper.Error( "Unexpected exception: " + e + "\nnAvailableBytes = " + nAvailableBytes );
                    return false;
                }
            }

            return false;
        }
        catch( Exception e )
        {
            m_aTestHelper.Error( "Exception: " + e );
            return false;
        }
    }
}

