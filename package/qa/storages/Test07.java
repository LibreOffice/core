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

import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import com.sun.star.embed.*;

import share.LogWriter;
import complex.storages.TestHelper;
import complex.storages.StorageTest;

public class Test07 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public Test07( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "Test07: " );
    }

    public boolean test()
    {
        try
        {
            String sTempFileURL = m_aTestHelper.CreateTempFile( m_xMSF );
            if ( sTempFileURL == null || sTempFileURL == "" )
            {
                m_aTestHelper.Error( "No valid temporary file was created!" );
                return false;
            }

            // create temporary storage based on arbitrary medium
            // after such a storage is closed it is lost
            Object oTempStorage = m_xStorageFactory.createInstance();
            XStorage xTempStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
            if ( xTempStorage == null )
            {
                m_aTestHelper.Error( "Can't create temporary storage representation!" );
                return false;
            }

            byte pBigBytes[] = new byte[33000];
            for ( int nInd = 0; nInd < 33000; nInd++ )
                pBigBytes[nInd] = (byte)( nInd % 128 );

            byte pBytes1[] = { 1, 1, 1, 1, 1 };
            String sPass1 = "12345";

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempStorage, "BigSubStream1", "MediaType1", true, pBigBytes, sPass1 ) )
                return false;

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempStorage, "SubStream1", "MediaType1", true, pBytes1, sPass1 ) )
                return false;

            byte pBytes2[] = { 2, 2, 2, 2, 2 };
            String sPass2 = "54321";

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempStorage, "BigSubStream2", "MediaType2", false, pBigBytes, sPass2 ) )
                return false;

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempStorage, "SubStream2", "MediaType2", false, pBytes2, sPass2 ) )
                return false;

            // create temporary storage based on a previously created temporary file
            Object pArgs[] = new Object[2];
            pArgs[0] = (Object) sTempFileURL;
            pArgs[1] = Integer.valueOf( ElementModes.WRITE );

            Object oTempFileStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            XStorage xTempFileStorage = (XStorage)UnoRuntime.queryInterface( XStorage.class, oTempFileStorage );
            if ( xTempFileStorage == null )
            {
                m_aTestHelper.Error( "Can't create storage based on temporary file!" );
                return false;
            }

            // copy xTempStorage to xTempFileStorage
            // xTempFileStorage will be automatically committed
            if ( !m_aTestHelper.copyStorage( xTempStorage, xTempFileStorage ) )
                return false;

            // dispose used storages to free resources
            if ( !m_aTestHelper.disposeStorage( xTempStorage ) || !m_aTestHelper.disposeStorage( xTempFileStorage ) )
                return false;


            // now check all the written and copied information


            // the temporary file must not be locked any more after storage disposing
            pArgs[1] = Integer.valueOf( ElementModes.WRITE );
            Object oResultStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            XStorage xResultStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oResultStorage );
            if ( xResultStorage == null )
            {
                m_aTestHelper.Error( "Can't reopen storage based on temporary file!" );
                return false;
            }

            Object o2CopyStorage = m_xStorageFactory.createInstance();
            XStorage x2CopyStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, o2CopyStorage );
            if ( x2CopyStorage == null )
            {
                m_aTestHelper.Error( "Can't create temporary storage representation!" );
                return false;
            }

            if ( !m_aTestHelper.copyStorage( xResultStorage, x2CopyStorage ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( xResultStorage, "SubStream1", "MediaType1", pBytes1, sPass1 ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( xResultStorage, "BigSubStream1", "MediaType1", pBigBytes, sPass1 ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( xResultStorage, "SubStream2", "MediaType2", pBytes2, sPass2 ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( xResultStorage, "BigSubStream2", "MediaType2", pBigBytes, sPass2 ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( x2CopyStorage, "SubStream1", "MediaType1", pBytes1, sPass1 ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( x2CopyStorage, "BigSubStream1", "MediaType1", pBigBytes, sPass1 ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( x2CopyStorage, "SubStream2", "MediaType2", pBytes2, sPass2 ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( x2CopyStorage, "BigSubStream2", "MediaType2", pBigBytes, sPass2 ) )
                return false;

            // dispose used storages to free resources
            if ( !m_aTestHelper.disposeStorage( xResultStorage ) )
                return false;

            return true;
        }
        catch( Exception e )
        {
            m_aTestHelper.Error( "Exception: " + e );
            return false;
        }
    }

}

