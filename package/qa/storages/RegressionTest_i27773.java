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
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.AnyConverter;

import com.sun.star.embed.*;

import share.LogWriter;
import complex.storages.TestHelper;
import complex.storages.StorageTest;


// Tests also fix for i51352


public class RegressionTest_i27773 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public RegressionTest_i27773( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "RegressionTest_i27773: " );
    }

    public boolean test()
    {
        try
        {
            XStream xTempFileStream = m_aTestHelper.CreateTempFileStream( m_xMSF );
            if ( xTempFileStream == null )
                return false;

            if ( true )
            {
                // for debugging proposes

                XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, xTempFileStream );
                if ( xPropSet != null )
                {
                    try
                    {
                        String sTempURL = AnyConverter.toString( xPropSet.getPropertyValue( "Uri" ) );
                        // m_aTestHelper.Message( "URL: " + sTempURL );
                        xPropSet.setPropertyValue( "RemoveFile", new Boolean( false ) );
                    }
                    catch ( Exception e )
                    {
                    }
                }
            }

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

            // open an empty substorage
            XStorage xEmptySubStorage = m_aTestHelper.openSubStorage( xTempStorage,
                                                                    "EmptySubStorage1",
                                                                    ElementModes.WRITE );
            if ( xEmptySubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // open an empty substorage
            XStorage xEmptySubSubStorage = m_aTestHelper.openSubStorage( xTempSubStorage,
                                                                        "EmptySubSubStorage1",
                                                                        ElementModes.WRITE );
            if ( xEmptySubSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }


            byte pBytes1[] = { 1, 1, 1, 1, 1 };

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToSubstream( xTempSubStorage, "SubStream1", "MediaType1", true, pBytes1 ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempStorage,
                                                            "MediaType2",
                                                            true,
                                                            ElementModes.WRITE ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xEmptySubStorage,
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
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xEmptySubSubStorage,
                                                            "MediaType5",
                                                            false,
                                                            ElementModes.WRITE ) )
                return false;


            // make a copy of substorage

            if ( !m_aTestHelper.copyElementTo( xTempStorage, "SubStorage1", xTempStorage, "SubStorage1_copy" ) )
                return false;

            if ( !m_aTestHelper.copyElementTo( xTempStorage, "EmptySubStorage1", xTempStorage, "EmptySubStorage1_copy" ) )
                return false;


            // copy all the changed and noncommited substorages
            // and dispose them


            if ( !m_aTestHelper.commitStorage( xEmptySubSubStorage ) )
                return false;

            if ( !m_aTestHelper.commitStorage( xTempSubStorage ) )
                return false;

            if ( !m_aTestHelper.commitStorage( xEmptySubStorage ) )
                return false;

            if ( !m_aTestHelper.commitStorage( xTempStorage ) )
                return false;

            // dispose substorages

            if ( !m_aTestHelper.disposeStorage( xEmptySubSubStorage ) )
                return false;

            if ( !m_aTestHelper.disposeStorage( xTempSubStorage ) )
                return false;

            if ( !m_aTestHelper.disposeStorage( xEmptySubStorage ) )
                return false;

            if ( !m_aTestHelper.disposeStorage( xTempStorage ) )
                return false;


            // reopen the storage in readonly mode an check contents


            pArgs[1] = new Integer( ElementModes.READ );

            oTempStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            xTempStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
            if ( xTempStorage == null )
            {
                m_aTestHelper.Error( "Can't create temporary storage representation!" );
                return false;
            }

            // open original substorage
            xTempSubStorage = m_aTestHelper.openSubStorage( xTempStorage,
                                                                    "SubStorage1",
                                                                    ElementModes.READ );
            if ( xTempSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // open copy of the original substorage
            XStorage xTempSubStorage_copy = m_aTestHelper.openSubStorage( xTempStorage,
                                                                    "SubStorage1_copy",
                                                                    ElementModes.READ );
            if ( xTempSubStorage_copy == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // open empty substorage
            xEmptySubStorage = m_aTestHelper.openSubStorage( xTempStorage,
                                                            "EmptySubStorage1",
                                                            ElementModes.READ );
            if ( xEmptySubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // open copy of empty substorage
            XStorage xEmptySubStorage_copy = m_aTestHelper.openSubStorage( xTempStorage,
                                                                        "EmptySubStorage1_copy",
                                                                        ElementModes.READ );
            if ( xEmptySubStorage_copy == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // open an empty substorage of the substorage
            xEmptySubSubStorage = m_aTestHelper.openSubStorage( xTempSubStorage,
                                                                "EmptySubSubStorage1",
                                                                ElementModes.READ );
            if ( xEmptySubSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // open an empty substorage of the substorage copy
            XStorage xEmptySubSubStorage_inCopy = m_aTestHelper.openSubStorage( xTempSubStorage_copy,
                                                                                "EmptySubSubStorage1",
                                                                                ElementModes.READ );
            if ( xEmptySubSubStorage_inCopy == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }


            // check contents

            if ( !m_aTestHelper.checkStorageProperties( xEmptySubSubStorage, "MediaType5", false, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.checkStorageProperties( xEmptySubSubStorage_inCopy, "MediaType5", false, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.checkStorageProperties( xTempSubStorage, "MediaType4", false, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.checkStorageProperties( xTempSubStorage_copy, "MediaType4", false, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.checkStorageProperties( xEmptySubStorage, "MediaType3", false, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.checkStorageProperties( xEmptySubStorage_copy, "MediaType3", false, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.checkStorageProperties( xTempStorage, "MediaType2", true, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.checkStream( xTempSubStorage, "SubStream1", "MediaType1", true, pBytes1 ) )
                return false;

            if ( !m_aTestHelper.checkStream( xTempSubStorage_copy, "SubStream1", "MediaType1", true, pBytes1 ) )
                return false;

            // the root storage is based on the temporary stream so it can be left undisposed, since it does not lock
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

