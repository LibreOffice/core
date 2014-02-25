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

public class RegressionTest_i29169 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public RegressionTest_i29169( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "RegressionTest_i29169: " );
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

            // open a new substorage
            XStorage xTempSubStorage = m_aTestHelper.openSubStorage( xTempStorage,
                                                                    "SubStorage1",
                                                                    ElementModes.WRITE );
            if ( xTempSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            byte pBytes1[] = { 1, 1, 1, 1, 1 };

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToSubstream( xTempSubStorage, "SubStream1", "MediaType1", true, pBytes1 ) )
                return false;

            // open a new substorage in the existing substorage
            XStorage xTempSubSubStorage = m_aTestHelper.openSubStorage( xTempSubStorage,
                                                                        "SubSubStorage1",
                                                                        ElementModes.WRITE );
            if ( xTempSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToSubstream( xTempSubSubStorage, "SubSubStream1", "MediaType2", true, pBytes1 ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempSubSubStorage,
                                                            "MediaType3",
                                                            false,
                                                            ElementModes.WRITE ) )
                return false;


            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempSubStorage,
                                                            "MediaType4",
                                                            false,
                                                            ElementModes.WRITE ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempStorage,
                                                            "MediaType5",
                                                            true,
                                                            ElementModes.WRITE ) )
                return false;


            // commit the storages, and check the renaming in all stages


            // rename the storage before it is commited
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubSubStorage1", "SubSubStorage2" ) )
                return false;

            // rename the stream
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubStream1", "SubStream2" ) )
                return false;

            // commit lowlevel substorage first
            if ( !m_aTestHelper.commitStorage( xTempSubSubStorage ) )
                return false;

            // rename the storage after it is commited
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubSubStorage2", "SubSubStorage3" ) )
                return false;

            // rename the stream one more time
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubStream2", "SubStream3" ) )
                return false;

            // commit substorage
            if ( !m_aTestHelper.commitStorage( xTempSubStorage ) )
                return false;

            // rename the storage after it`s parent is commited
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubSubStorage3", "SubSubStorage4" ) )
                return false;

            // rename the stream after it`s parent is commited
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubStream3", "SubStream4" ) )
                return false;

            // commit substorage to let the renaming take place
            if ( !m_aTestHelper.commitStorage( xTempSubStorage ) )
                return false;

            // commit the root storage so the contents must be stored now
            if ( !m_aTestHelper.commitStorage( xTempStorage ) )
                return false;

            // rename the storage after the package is commited
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubSubStorage4", "SubSubStorage5" ) )
                return false;

            // rename the stream after it`s parent is commited
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubStream4", "SubStream5" ) )
                return false;

            // commit substorage to let the renaming take place
            if ( !m_aTestHelper.commitStorage( xTempSubStorage ) )
                return false;

            // commit the root storage so the contents must be stored now
            if ( !m_aTestHelper.commitStorage( xTempStorage ) )
                return false;


            // dispose the storages


            // dispose lowerest substorage
            if ( !m_aTestHelper.disposeStorage( xTempSubSubStorage ) )
                return false;

            // dispose substorage
            if ( !m_aTestHelper.disposeStorage( xTempSubStorage ) )
                return false;

            // dispose the temporary storage
            if ( !m_aTestHelper.disposeStorage( xTempStorage ) )
                return false;


            // create a new storage based on the stream and check the substreams and substorages


            oTempStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            xTempStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
            if ( xTempStorage == null )
            {
                m_aTestHelper.Error( "Can't create temporary storage representation!" );
                return false;
            }

            // open the substorage
            xTempSubStorage = m_aTestHelper.openSubStorage( xTempStorage,
                                                            "SubStorage1",
                                                            ElementModes.WRITE );
            if ( xTempSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // open the lowlevel substorage
            xTempSubSubStorage = m_aTestHelper.openSubStorage( xTempSubStorage,
                                                                        "SubSubStorage5",
                                                                        ElementModes.WRITE );
            if ( xTempSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // check the storages and streams

            if ( !m_aTestHelper.checkStorageProperties( xTempSubSubStorage, "MediaType3", false, ElementModes.WRITE ) )
                return false;

            if ( !m_aTestHelper.checkStorageProperties( xTempSubStorage, "MediaType4", false, ElementModes.WRITE ) )
                return false;

            if ( !m_aTestHelper.checkStorageProperties( xTempStorage, "MediaType5", true, ElementModes.WRITE ) )
                return false;

            if ( !m_aTestHelper.checkStream( xTempSubStorage, "SubStream5", "MediaType1", true, pBytes1 ) )
                return false;

            if ( !m_aTestHelper.checkStream( xTempSubSubStorage, "SubSubStream1", "MediaType2", true, pBytes1 ) )
                return false;


            // rename the reopened storages and streams


            // rename the storage before it is commited
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubSubStorage5", "SubSubStorage6" ) )
                return false;

            // rename the stream
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubStream5", "SubStream6" ) )
                return false;

            // commit lowlevel substorage first
            if ( !m_aTestHelper.commitStorage( xTempSubSubStorage ) )
                return false;

            // rename the storage after it is commited
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubSubStorage6", "SubSubStorage7" ) )
                return false;

            // rename the stream one more time
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubStream6", "SubStream7" ) )
                return false;

            // commit substorage
            if ( !m_aTestHelper.commitStorage( xTempSubStorage ) )
                return false;

            // rename the storage after it`s parent is commited
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubSubStorage7", "SubSubStorage8" ) )
                return false;

            // rename the stream after it`s parent is commited
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubStream7", "SubStream8" ) )
                return false;

            // commit substorage to let the renaming take place
            if ( !m_aTestHelper.commitStorage( xTempSubStorage ) )
                return false;

            // commit the root storage so the contents must be stored now
            if ( !m_aTestHelper.commitStorage( xTempStorage ) )
                return false;

            // rename the storage after the package is commited
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubSubStorage8", "SubSubStorage9" ) )
                return false;

            // rename the stream after it`s parent is commited
            if ( !m_aTestHelper.renameElement( xTempSubStorage, "SubStream8", "SubStream9" ) )
                return false;

            // commit substorage to let the renaming take place
            if ( !m_aTestHelper.commitStorage( xTempSubStorage ) )
                return false;

            // commit the root storage so the contents must be stored now
            if ( !m_aTestHelper.commitStorage( xTempStorage ) )
                return false;


            // dispose the storages


            // dispose lowerest substorage
            if ( !m_aTestHelper.disposeStorage( xTempSubSubStorage ) )
                return false;

            // dispose substorage
            if ( !m_aTestHelper.disposeStorage( xTempSubStorage ) )
                return false;

            // dispose the temporary storage
            if ( !m_aTestHelper.disposeStorage( xTempStorage ) )
                return false;



            // create a new readonly storage based on the stream and check the contents


            pArgs[1] = new Integer( ElementModes.READ );
            oTempStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            xTempStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
            if ( xTempStorage == null )
            {
                m_aTestHelper.Error( "Can't create temporary storage representation!" );
                return false;
            }

            // open the substorage
            xTempSubStorage = m_aTestHelper.openSubStorage( xTempStorage,
                                                            "SubStorage1",
                                                            ElementModes.READ );
            if ( xTempSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // open the lowlevel substorage
            xTempSubSubStorage = m_aTestHelper.openSubStorage( xTempSubStorage,
                                                                        "SubSubStorage9",
                                                                        ElementModes.READ );
            if ( xTempSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // check the storages and streams

            if ( !m_aTestHelper.checkStorageProperties( xTempSubSubStorage, "MediaType3", false, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.checkStorageProperties( xTempSubStorage, "MediaType4", false, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.checkStorageProperties( xTempStorage, "MediaType5", true, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.checkStream( xTempSubStorage, "SubStream9", "MediaType1", true, pBytes1 ) )
                return false;

            if ( !m_aTestHelper.checkStream( xTempSubSubStorage, "SubSubStream1", "MediaType2", true, pBytes1 ) )
                return false;

            // the storage is based on the temporary stream so it can be left undisposed, since it does not lock
            // any resource, later the garbage collector will release the object and it must die by refcount

            return true;
        }
        catch( Exception e )
        {
            m_aTestHelper.Error( "Exception: " + e );
            return false;
        }
    }
}

