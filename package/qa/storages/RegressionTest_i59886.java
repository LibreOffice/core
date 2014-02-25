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
import com.sun.star.io.XStream;
import com.sun.star.io.XInputStream;

import com.sun.star.embed.*;

import share.LogWriter;
import complex.storages.TestHelper;
import complex.storages.StorageTest;

public class RegressionTest_i59886 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public RegressionTest_i59886( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "RegressionTest_i59886: " );
    }

    public boolean test()
    {
        try
        {
            XStream xTempFileStream = m_aTestHelper.CreateTempFileStream( m_xMSF );
            if ( xTempFileStream == null )
                return false;

            // create storage based on the temporary stream
            Object pArgs[] = new Object[2];
            pArgs[0] = (Object) xTempFileStream;
            pArgs[1] = new Integer( ElementModes.WRITE );

            Object oTempStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            XStorage xTempStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
            if ( xTempStorage == null )
            {
                m_aTestHelper.Error( "Can't create temporary storage representation!" );
                return false;
            }

            byte pBytes[] = new byte[36000];
            for ( int nInd = 0; nInd < 36000; nInd++ )
                pBytes[nInd] = (byte)( nInd % 128 );

            String sPass = "12345";

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempStorage, "SubStream1", "MediaType1", true, pBytes, sPass ) )
                return false;

            // open a new substorage
            XStorage xTempSubStorage = m_aTestHelper.openSubStorage( xTempStorage,
                                                                        "SubStorage1",
                                                                        ElementModes.WRITE );
            if ( xTempSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempSubStorage, "SubStream2", "MediaType2", false, pBytes, sPass ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempStorage,
                                                            "MediaType3",
                                                            true,
                                                            ElementModes.WRITE ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempSubStorage,
                                                            "MediaType4",
                                                            false,
                                                            ElementModes.WRITE ) )
                return false;

            // commit substorage first
            if ( !m_aTestHelper.commitStorage( xTempSubStorage ) )
                return false;

            // commit the root storage so the contents must be stored now
            if ( !m_aTestHelper.commitStorage( xTempStorage ) )
                return false;

            // dispose used storage to free resources
            if ( !m_aTestHelper.disposeStorage( xTempStorage ) )
                return false;


            // now reopen the storage, set the common storage key
            // and copy the storage


            Object oStep2TempStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            XStorage xStep2TempStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oStep2TempStorage );
            if ( xStep2TempStorage == null )
            {
                m_aTestHelper.Error( "Can't create temporary storage representation!" );
                return false;
            }


            XStorage xStep2TempSubStorage = m_aTestHelper.openSubStorage( xStep2TempStorage,
                                                                        "SubStorage1",
                                                                        ElementModes.WRITE );
            if ( xStep2TempSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // set the common storage password
            XEncryptionProtectedSource xEncr = (XEncryptionProtectedSource) UnoRuntime.queryInterface( XEncryptionProtectedSource.class, xStep2TempStorage );
            if ( xEncr == null )
            {
                m_aTestHelper.Error( "The storage does not support encryption access!" );
                return false;
            }
            try
            {
                xEncr.setEncryptionPassword( sPass );
            }
            catch( Exception e )
            {
                m_aTestHelper.Error( "Can not set the common storage password!" );
                return false;
            }

            // open the stream for writing and read them so that the cache is created, but do not change
            byte pDummyData[][] = new byte[1][3];
            XStream xTempStream1 = m_aTestHelper.OpenStream( xStep2TempStorage, "SubStream1", ElementModes.WRITE );
            XStream xTempStream2 = m_aTestHelper.OpenStream( xStep2TempSubStorage, "SubStream2", ElementModes.WRITE );
            if ( xTempStream1 == null || xTempStream2 == null )
                return false;

            XInputStream xTempInStream1 = xTempStream1.getInputStream();
            XInputStream xTempInStream2 = xTempStream2.getInputStream();
            if ( xTempInStream1 == null || xTempInStream2 == null )
            {
                m_aTestHelper.Error( "No input stream is available!" );
                return false;
            }

            xTempInStream1.readBytes( pDummyData, 3 );
            xTempInStream2.readBytes( pDummyData, 3 );


            // create temporary storage, it will be checked later
            Object oTargetStorage = m_xStorageFactory.createInstance();
            XStorage xTargetStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTargetStorage );
            if ( xTargetStorage == null )
            {
                m_aTestHelper.Error( "Can't create temporary storage representation!" );
                return false;
            }

            // copy the current storage to the target
            try
            {
                xStep2TempStorage.copyToStorage( xTargetStorage );
            }
            catch( Exception e )
            {
                m_aTestHelper.Error( "Can not copy the storage with common storage password!" );
                return false;
            }

            // dispose used storage to free resources
            if ( !m_aTestHelper.disposeStorage( xStep2TempStorage ) )
                return false;


            // now check all the information in the copy


            if ( !m_aTestHelper.checkStorageProperties( xTargetStorage, "MediaType3", true, ElementModes.WRITE ) )
                return false;

            // open existing substorage
            XStorage xTargetSubStorage = m_aTestHelper.openSubStorage( xTargetStorage,
                                                                        "SubStorage1",
                                                                        ElementModes.WRITE );
            if ( xTargetSubStorage == null )
            {
                m_aTestHelper.Error( "Can't open existing substorage!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xTargetSubStorage, "MediaType4", false, ElementModes.WRITE ) )
                return false;

            // set the common storage password
            XEncryptionProtectedSource xTargetEncr = (XEncryptionProtectedSource) UnoRuntime.queryInterface( XEncryptionProtectedSource.class, xTargetStorage );
            if ( xTargetEncr == null )
            {
                m_aTestHelper.Error( "The storage does not support encryption access!" );
                return false;
            }
            try
            {
                xTargetEncr.setEncryptionPassword( sPass );
            }
            catch( Exception e )
            {
                m_aTestHelper.Error( "Can not set the common storage password!" );
                return false;
            }

            // check the streams
            if ( !m_aTestHelper.checkStream( xTargetStorage, "SubStream1", "MediaType1", true, pBytes ) )
                return false;
            if ( !m_aTestHelper.checkStream( xTargetSubStorage, "SubStream2", "MediaType2", true, pBytes ) )
                return false;


            // dispose used storages to free resources
            if ( !m_aTestHelper.disposeStorage( xTargetStorage ) )
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

