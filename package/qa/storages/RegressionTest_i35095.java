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

public class RegressionTest_i35095 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public RegressionTest_i35095( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "RegressionTest_i35095: " );
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


            // now check all the written information
            // and the raw stream contents


            // close the output part of the temporary stream
            // the output part must present since we already wrote to the stream
            if ( !m_aTestHelper.closeOutput( xTempFileStream ) )
                return false;

            XInputStream xTempInStream = m_aTestHelper.getInputStream( xTempFileStream );
            if ( xTempInStream == null )
                return false;

            // open input stream
            // since no mode is provided the result storage must be opened readonly
            Object pOneArg[] = new Object[1];
            pOneArg[0] = (Object) xTempInStream;

            Object oResultStorage = m_xStorageFactory.createInstanceWithArguments( pOneArg );
            XStorage xResultStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oResultStorage );
            if ( xResultStorage == null )
            {
                m_aTestHelper.Error( "Can't open storage based on input stream!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xResultStorage, "MediaType3", true, ElementModes.READ ) )
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

            if ( !m_aTestHelper.checkStorageProperties( xResultSubStorage, "MediaType4", false, ElementModes.READ ) )
                return false;

            if ( !m_aTestHelper.compareRawMethodsOnEncrStream( xResultStorage, "SubStream1" ) )
                return false;

            if ( !m_aTestHelper.compareRawMethodsOnEncrStream( xResultSubStorage, "SubStream2" ) )
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

