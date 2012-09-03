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

package storagetesting;

import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.io.XStream;

import com.sun.star.embed.*;

import storagetesting.TestHelper;
import storagetesting.StorageTest;

public class Test05 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public Test05( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( "Test05: " );
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

            // open a new substorage
            XStorage xTempSubStorage = m_aTestHelper.openSubStorage( xTempFileStorage,
                                                                        "SubStorage1",
                                                                        ElementModes.WRITE );
            if ( xTempSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // open a new substorage
            XStorage xSubSubStorage = m_aTestHelper.openSubStorage( xTempSubStorage,
                                                                        "SubSubStorage1",
                                                                        ElementModes.WRITE );
            if ( xSubSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }


            byte pBytes1[] = { 1, 1, 1, 1, 1 };

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToSubstream( xSubSubStorage, "SubStream1", "MediaType1", true, pBytes1 ) )
                return false;

            byte pBytes2[] = { 2, 2, 2, 2, 2 };

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToSubstream( xSubSubStorage, "SubStream2", "MediaType2", false, pBytes2 ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempFileStorage,
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

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xSubSubStorage,
                                                            "MediaType5",
                                                            false,
                                                            ElementModes.WRITE ) )
                return false;


            // commit all the storages
            if ( !m_aTestHelper.commitStorage( xSubSubStorage ) )
                return false;

            if ( !m_aTestHelper.commitStorage( xTempSubStorage ) )
                return false;

            if ( !m_aTestHelper.commitStorage( xTempFileStorage ) )
                return false;

            // try to open an opened substorage, open call must fail
            if ( !m_aTestHelper.cantOpenStorage( xTempFileStorage, "SubStorage1" ) )
                return false;


            // reopen created streams
            XStream xSubStream1 = m_aTestHelper.OpenStream( xSubSubStorage,
                                                            "SubStream1",
                                                            ElementModes.READWRITE | ElementModes.NOCREATE );
            XStream xSubStream2 = m_aTestHelper.OpenStream( xSubSubStorage,
                                                            "SubStream2",
                                                            ElementModes.READ | ElementModes.NOCREATE );
            if ( xSubStream1 == null || xSubStream2 == null )
                return false;

            // it should be possible to have more then one copy of stream for reading
            XStream xSubStream2clone = m_aTestHelper.OpenStream( xSubSubStorage,
                                                                "SubStream2",
                                                                ElementModes.READ | ElementModes.NOCREATE );
            if ( xSubStream2 == null )
                return false;


            // so now the first stream can not be open neither for reading nor for writing
            if ( !m_aTestHelper.cantOpenStream( xSubSubStorage, "SubStream1", ElementModes.WRITE )
              || !m_aTestHelper.cantOpenStream( xSubSubStorage, "SubStream1", ElementModes.READ ) )
                return false;

            // the second stream can not be open for writing
            if ( !m_aTestHelper.cantOpenStream( xSubSubStorage, "SubStream2", ElementModes.WRITE ) )
                return false;


            // dispose xTestSubStorage, all the subtree must be disposed
            if ( !m_aTestHelper.disposeStorage( xTempSubStorage ) )
                return false;

            // check that subtree was disposed correctly
            try
            {
                xSubSubStorage.isStreamElement( "SubStream1" );
                m_aTestHelper.Error( "Substorage was not disposed!" );
                return false;
            }
            catch ( com.sun.star.lang.DisposedException de )
            {}
            catch ( Exception e )
            {
                m_aTestHelper.Error( "Wrong exception is thrown by disposed storage: " + e );
                return false;
            }

            try
            {
                xSubStream1.getInputStream();
                m_aTestHelper.Error( "Writeable substream was not disposed!" );
                return false;
            }
            catch ( com.sun.star.lang.DisposedException de )
            {}
            catch ( Exception e )
            {
                m_aTestHelper.Error( "Wrong exception is thrown by disposed stream: " + e );
                return false;
            }

            try
            {
                xSubStream2.getInputStream();
                m_aTestHelper.Error( "Readonly substream was not disposed!" );
                return false;
            }
            catch ( com.sun.star.lang.DisposedException de )
            {}
            catch ( Exception e )
            {
                m_aTestHelper.Error( "Wrong exception is thrown by disposed stream: " + e );
                return false;
            }


            // dispose root storage
            if ( !m_aTestHelper.disposeStorage( xTempFileStorage ) )
                return false;


            // ================================================
            // now check all the written and copied information
            // ================================================

            pArgs[1] = new Integer( ElementModes.READ );
            Object oResultStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            XStorage xResultStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oResultStorage );
            if ( xResultStorage == null )
            {
                m_aTestHelper.Error( "Can't reopen storage based on temporary file!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xResultStorage, "MediaType3", true, ElementModes.READ ) )
                return false;

            // open existing substorage
            XStorage xResSubStorage = m_aTestHelper.openSubStorage( xResultStorage,
                                                                    "SubStorage1",
                                                                    ElementModes.READ );
            if ( xResSubStorage == null )
            {
                m_aTestHelper.Error( "Can't open existing substorage 'SubSubStorage'!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xResSubStorage, "MediaType4", false, ElementModes.READ ) )
                return false;

            // open existing substorage
            XStorage xResSubSubStorage = m_aTestHelper.openSubStorage( xResSubStorage,
                                                                        "SubSubStorage1",
                                                                        ElementModes.READ );
            if ( xResSubSubStorage == null )
            {
                m_aTestHelper.Error( "Can't open existing substorage 'SubSubStorage'!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xResSubSubStorage, "MediaType5", false, ElementModes.READ ) )
                return false;

            // check substreams
            if ( !m_aTestHelper.checkStream( xResSubSubStorage, "SubStream1", "MediaType1", pBytes1 ) )
                return false;

            if ( !m_aTestHelper.checkStream( xResSubSubStorage, "SubStream2", "MediaType2", pBytes2 ) )
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

