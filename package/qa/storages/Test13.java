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

public class Test13 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public Test13( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "Test13: " );
    }

    public boolean test()
    {
        String aStreamPrefix = "";
        for ( int nInd = 0; nInd < 4; ++nInd, aStreamPrefix += "SubStorage" + nInd )
            if ( !testForPath( aStreamPrefix ) )
                return false;

        return true;
    }

    public boolean testForPath( String aStreamPrefix )
    {
        try
        {
            String aSubStream1Path = aStreamPrefix + "SubStream1";
            String aSubStream2Path = aStreamPrefix + "SubStream2";
            String aSubStream3Path = aStreamPrefix + "SubStream3";
            String aBigSubStream1Path = aStreamPrefix + "BigSubStream1";
            String aBigSubStream2Path = aStreamPrefix + "BigSubStream2";
            String aBigSubStream3Path = aStreamPrefix + "BigSubStream3";

            String sTempFileURL = m_aTestHelper.CreateTempFile( m_xMSF );
            if ( sTempFileURL == null || sTempFileURL == "" )
            {
                m_aTestHelper.Error( "No valid temporary file was created!" );
                return false;
            }

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

            byte pBigBytes[] = new byte[33000];
            for ( int nInd = 0; nInd < 33000; nInd++ )
                pBigBytes[nInd] = (byte)( nInd % 128 );

            byte pBytes1[] = { 1, 1, 1, 1, 1 };

            // open a new substream hierarchically, set "MediaType" and "Compressed" properties to it, write some bytes
            // and commit
            if ( !m_aTestHelper.WriteBytesToStreamH( xTempFileStorage, aSubStream1Path, "MediaType1", true, pBytes1, true ) )
                return false;
            if ( !m_aTestHelper.WriteBytesToStreamH( xTempFileStorage, aBigSubStream1Path, "MediaType1", true, pBigBytes, true ) )
                return false;

            byte pBytes2[] = { 2, 2, 2, 2, 2 };

            // open a new substream hierarchically, set "MediaType" and "Compressed" properties to it, write some bytes
            // and commit
            if ( !m_aTestHelper.WriteBytesToStreamH( xTempFileStorage, aSubStream2Path, "MediaType2", false, pBytes2, true ) )
                return false;
            if ( !m_aTestHelper.WriteBytesToStreamH( xTempFileStorage, aBigSubStream2Path, "MediaType2", false, pBigBytes, true ) )
                return false;

            // open a new substream hierarchically, set "MediaType" and "Compressed" properties to it, write some bytes
            // and don't commit
            if ( !m_aTestHelper.WriteBytesToStreamH( xTempFileStorage, aSubStream3Path, "MediaType2", false, pBytes2, false ) )
                return false;
            if ( !m_aTestHelper.WriteBytesToStreamH( xTempFileStorage, aBigSubStream3Path, "MediaType2", false, pBigBytes, false ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempFileStorage,
                                                            "MediaType3",
                                                            true,
                                                            ElementModes.WRITE ) )
                return false;

            // commit the root storage so the contents must be stored now
            if ( !m_aTestHelper.commitStorage( xTempFileStorage ) )
                return false;

            // dispose used storages to free resources
            if ( !m_aTestHelper.disposeStorage( xTempFileStorage ) )
                return false;


            // now reopen the storage,
            // check all the written and copied information
            // and change it


            // the temporary file must not be locked any more after storage disposing
            oTempFileStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            xTempFileStorage = (XStorage)UnoRuntime.queryInterface( XStorage.class, oTempFileStorage );
            if ( xTempFileStorage == null )
            {
                m_aTestHelper.Error( "Can't create storage based on temporary file!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xTempFileStorage, "MediaType3", true, ElementModes.WRITE ) )
                return false;

            if ( !m_aTestHelper.checkStreamH( xTempFileStorage, aSubStream1Path, "MediaType1", true, pBytes1 ) )
                return false;

            if ( !m_aTestHelper.checkStreamH( xTempFileStorage, aBigSubStream1Path, "MediaType1", true, pBigBytes ) )
                return false;

            if ( !m_aTestHelper.checkStreamH( xTempFileStorage, aSubStream2Path, "MediaType2", false, pBytes2 ) )
                return false;

            if ( !m_aTestHelper.checkStreamH( xTempFileStorage, aBigSubStream2Path, "MediaType2", false, pBigBytes ) )
                return false;

            if ( !m_aTestHelper.cantOpenStreamH( xTempFileStorage, aSubStream3Path, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.cantOpenStreamH( xTempFileStorage, aBigSubStream3Path, ElementModes.READ ) )
                return false;

            // open existing substream hierarchically, set "MediaType" and "Compressed" properties to it, write some bytes
            // and commit
            if ( !m_aTestHelper.WriteBytesToStreamH( xTempFileStorage, aSubStream1Path, "MediaType3", true, pBytes2, true ) )
                return false;
            if ( !m_aTestHelper.WriteBytesToStreamH( xTempFileStorage, aBigSubStream1Path, "MediaType3", true, pBigBytes, true ) )
                return false;


            // open existing substream hierarchically, set "MediaType" and "Compressed" properties to it, write some bytes
            // and don't commit
            if ( !m_aTestHelper.WriteBytesToStreamH( xTempFileStorage, aSubStream2Path, "MediaType3", true, pBytes1, false ) )
                return false;
            if ( !m_aTestHelper.WriteBytesToStreamH( xTempFileStorage, aBigSubStream2Path, "MediaType3", true, pBigBytes, false ) )
                return false;

            // commit the root storage so the contents must be stored now
            if ( !m_aTestHelper.commitStorage( xTempFileStorage ) )
                return false;

            // dispose used storages to free resources
            if ( !m_aTestHelper.disposeStorage( xTempFileStorage ) )
                return false;


            // now reopen the storage,
            // check all the written information


            // the temporary file must not be locked any more after storage disposing
            pArgs[1] = Integer.valueOf( ElementModes.READ );
            Object oResultStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            XStorage xResultStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oResultStorage );
            if ( xResultStorage == null )
            {
                m_aTestHelper.Error( "Can't reopen storage based on temporary file!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xResultStorage, "MediaType3", true, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.checkStreamH( xResultStorage, aSubStream1Path, "MediaType3", true, pBytes2 ) )
                return false;
            if ( !m_aTestHelper.checkStreamH( xResultStorage, aBigSubStream1Path, "MediaType3", true, pBigBytes ) )
                return false;

            // the following stream was not committed last time, so the last change must be lost
            if ( !m_aTestHelper.checkStreamH( xResultStorage, aBigSubStream2Path, "MediaType2", false, pBigBytes ) )
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

