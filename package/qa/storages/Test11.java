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

import com.sun.star.container.XNameAccess;
import com.sun.star.io.XStream;

import com.sun.star.embed.*;

import share.LogWriter;
import complex.storages.TestHelper;
import complex.storages.StorageTest;

public class Test11 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public Test11( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "Test11: " );
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

            byte pBigBytes[] = new byte[33000];
            for ( int nInd = 0; nInd < 33000; nInd++ )
                pBigBytes[nInd] = (byte)( nInd % 128 );

            String sPass1 = "111111111";
            byte pBytes1[] = { 1, 1, 1, 1, 1 };

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempStorage, "SubStream1", "MediaType1", true, pBytes1, sPass1 ) )
                return false;

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempStorage, "BigSubStream1", "MediaType1", true, pBigBytes, sPass1 ) )
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

            String sPass2 = "2222222222";
            byte pBytes2[] = { 2, 2, 2, 2, 2 };

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempSubStorage, "SubStream2", "MediaType2", true, pBytes2, sPass2 ) )
                return false;

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempSubStorage, "BigSubStream2", "MediaType2", true, pBigBytes, sPass2 ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempStorage,
                                                            "MediaType3",
                                                            true,
                                                            ElementModes.WRITE ) )
                return false;

            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempSubStorage,
                                                            "MediaType4",
                                                            false,
                                                            ElementModes.WRITE ) )
                return false;


            // check cloning at current state


            // the new storage still was not commited so the clone must be empty
            XStorage xClonedSubStorage = m_aTestHelper.cloneSubStorage( m_xStorageFactory, xTempStorage, "SubStorage1" );

            if ( xClonedSubStorage == null )
            {
                m_aTestHelper.Error( "The result of clone is empty!" );
                return false;
            }

            XNameAccess xClonedNameAccess = (XNameAccess) UnoRuntime.queryInterface( XNameAccess.class, xClonedSubStorage );
            if ( xClonedNameAccess == null )
            {
                m_aTestHelper.Error( "XNameAccess is not implemented by the clone!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xClonedSubStorage, "", true, ElementModes.WRITE ) )
                return false;

            if ( xClonedNameAccess.hasElements() )
            {
                m_aTestHelper.Error( "The new substorage still was not commited so it must be empty!" );
                return false;
            }

            if ( !m_aTestHelper.disposeStorage( xClonedSubStorage ) )
                return false;

            xClonedSubStorage = null;
            xClonedNameAccess = null;

            // the new stream was opened, written and closed, that means flashed
            // so the clone must contain all the information
            XStream xClonedSubStream = m_aTestHelper.cloneEncrSubStream( xTempStorage, "SubStream1", sPass1 );
            if ( !m_aTestHelper.InternalCheckStream( xClonedSubStream, "SubStream1", "MediaType1", true, pBytes1, true ) )
                return false;

            XStream xClonedBigSubStream = m_aTestHelper.cloneEncrSubStream( xTempStorage, "BigSubStream1", sPass1 );
            if ( !m_aTestHelper.InternalCheckStream( xClonedBigSubStream, "BigSubStream1", "MediaType1", true, pBigBytes, true ) )
                return false;

            if ( !m_aTestHelper.disposeStream( xClonedSubStream, "SubStream1" ) )
                return false;

            if ( !m_aTestHelper.disposeStream( xClonedBigSubStream, "BigSubStream1" ) )
                return false;


            // commit substorage and check cloning


            if ( !m_aTestHelper.commitStorage( xTempSubStorage ) )
                return false;

            xClonedSubStorage = m_aTestHelper.cloneSubStorage( m_xStorageFactory, xTempStorage, "SubStorage1" );
            if ( xClonedSubStorage == null )
            {
                m_aTestHelper.Error( "The result of clone is empty!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xClonedSubStorage, "MediaType4", true, ElementModes.WRITE ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( xClonedSubStorage, "SubStream2", "MediaType2", pBytes2, sPass2 ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( xClonedSubStorage, "BigSubStream2", "MediaType2", pBigBytes, sPass2 ) )
                return false;


            // commit the root storage and check cloning


            if ( !m_aTestHelper.commitStorage( xTempStorage ) )
                return false;

            XStorage xCloneOfRoot = m_aTestHelper.cloneStorage( m_xStorageFactory, xTempStorage );
            if ( xCloneOfRoot == null )
            {
                m_aTestHelper.Error( "The result of root clone is empty!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xCloneOfRoot, "MediaType3", true, ElementModes.WRITE ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( xCloneOfRoot, "SubStream1", "MediaType1", pBytes1, sPass1 ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( xCloneOfRoot, "BigSubStream1", "MediaType1", pBigBytes, sPass1 ) )
                return false;

            XStorage xSubStorageOfClone = xCloneOfRoot.openStorageElement( "SubStorage1", ElementModes.READ );
            if ( xSubStorageOfClone == null )
            {
                m_aTestHelper.Error( "The result of root clone is wrong!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xSubStorageOfClone, "MediaType4", false, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( xSubStorageOfClone, "SubStream2", "MediaType2", pBytes2, sPass2 ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( xSubStorageOfClone, "BigSubStream2", "MediaType2", pBigBytes, sPass2 ) )
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

