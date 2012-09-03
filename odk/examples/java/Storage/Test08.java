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

import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import com.sun.star.embed.*;

public class Test08 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public Test08( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( "Test08: " );
    }

    public boolean test()
    {
        try
        {

            // create temporary storage based on arbitrary medium
            // after such a storage is closed it is lost
            Object oTempStorage = m_xStorageFactory.createInstance();
            XStorage xTempStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
            if ( xTempStorage == null )
            {
                m_aTestHelper.Error( "Can't create temporary storage representation!" );
                return false;
            }

            // set the global password for the root storage
            XEncryptionProtectedSource xTempStorageEncryption =
                (XEncryptionProtectedSource) UnoRuntime.queryInterface( XEncryptionProtectedSource.class, xTempStorage );

            if ( xTempStorageEncryption == null )
            {
                m_aTestHelper.Message( "Optional interface XEncryptionProtectedSource is not implemented, feature can not be tested!" );
                return true;
            }

            byte pPass1[] = { 1, 2, 3 };
            byte pPass2[] = { 3, 2, 1 };

            try {
                xTempStorageEncryption.setEncryptionPassword( new String(pPass1) );
            }
            catch( Exception e )
            {
                m_aTestHelper.Error( "Can't set a common encryption key for the storage, exception:" + e );
                return false;
            }

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
            // the stream will be encrypted with common password
            byte pBytes1[] = { 1, 1, 1, 1, 1 };
            if ( !m_aTestHelper.WBToSubstrOfEncr( xTempSubStorage, "SubStream1", "MediaType1", true, pBytes1, true ) )
                return false;

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            // the stream will not be encrypted
            byte pBytes2[] = { 2, 2, 2, 2, 2 };
            if ( !m_aTestHelper.WBToSubstrOfEncr( xTempSubStorage, "SubStream2", "MediaType2", false, pBytes2, false ) )
                return false;

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            // the stream will be compressed with own password
            byte pBytes3[] = { 3, 3, 3, 3, 3 };

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            // the stream will not be encrypted
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempSubStorage, "SubStream3", "MediaType3", false, pBytes3, pPass2 ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempStorage,
                                                            "MediaType4",
                                                            true,
                                                            ElementModes.READWRITE ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempSubStorage,
                                                            "MediaType5",
                                                            false,
                                                            ElementModes.WRITE ) )
                return false;

            // create temporary file
            String sTempFileURL = m_aTestHelper.CreateTempFile( m_xMSF );
            if ( sTempFileURL == null || sTempFileURL == "" )
            {
                m_aTestHelper.Error( "No valid temporary file was created!" );
                return false;
            }

            // create temporary storage based on a previously created temporary file
            Object pArgs[] = new Object[2];
            pArgs[0] = (Object) sTempFileURL;
            pArgs[1] = new Integer( ElementModes.WRITE );

            Object oTempFileStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            XStorage xTempFileStorage = (XStorage)UnoRuntime.queryInterface( XStorage.class, oTempFileStorage );
            if ( xTempFileStorage == null )
            {
                m_aTestHelper.Error( "Can't create storage based on temporary file!" );
                return false;
            }

            // copy xTempStorage to xTempFileStorage
            // xTempFileStorage will be automatically commited
            if ( !m_aTestHelper.copyStorage( xTempStorage, xTempFileStorage ) )
                return false;

            // dispose used storages to free resources
            if ( !m_aTestHelper.disposeStorage( xTempStorage ) || !m_aTestHelper.disposeStorage( xTempFileStorage ) )
                return false;

            // ================================================
            // now check all the written and copied information
            // ================================================

            // the temporary file must not be locked any more after storage disposing
            pArgs[1] = new Integer( ElementModes.READ );
            Object oResultStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            XStorage xResultStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oResultStorage );
            if ( xResultStorage == null )
            {
                m_aTestHelper.Error( "Can't reopen storage based on temporary file!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xResultStorage, "MediaType4", true, ElementModes.READ ) )
                return false;

            // open existing substorage
            XStorage xResultSubStorage = m_aTestHelper.openSubStorage( xResultStorage,
                                                                        "SubStorage1",
                                                                        ElementModes.READ );
            if ( xResultSubStorage == null )
            {
                m_aTestHelper.Error( "Can't open existing substorage!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xResultSubStorage, "MediaType5", false, ElementModes.READ ) )
                return false;

            // set the global password for the root storage
            XEncryptionProtectedSource xResultStorageEncryption =
                (XEncryptionProtectedSource) UnoRuntime.queryInterface( XEncryptionProtectedSource.class, xResultStorage );

            if ( xResultStorageEncryption == null )
            {
                m_aTestHelper.Error( "XEncryptionProtectedSource was successfully used already, so it must be supported!" );
                return false;
            }

            try {
                xResultStorageEncryption.setEncryptionPassword( new String(pPass2) );
            }
            catch( Exception e )
            {
                m_aTestHelper.Error( "Can't set a common encryption key for the storage, exception:" + e );
                return false;
            }

            if ( !m_aTestHelper.checkEncrStream( xResultSubStorage, "SubStream1", "MediaType1", pBytes1, pPass1 ) )
                return false;

            if ( !m_aTestHelper.checkStream( xResultSubStorage, "SubStream2", "MediaType2", pBytes2 ) )
                return false;

            // the common root storage password should allow to open this stream
            if ( !m_aTestHelper.checkStream( xResultSubStorage, "SubStream3", "MediaType3", pBytes3 ) )
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

