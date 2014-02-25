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

import com.sun.star.embed.*;

import share.LogWriter;
import complex.storages.TestHelper;
import complex.storages.StorageTest;

public class RegressionTest_i55821 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public RegressionTest_i55821( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "RegressionTest_i55821: " );
    }

    public boolean test()
    {
        try
        {

            // create a temporary stream and a storage based on it
            // fill the storage with the data that will be used for testing


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

            String sPass = "12345";
            byte pBytes[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            // the stream will not be encrypted
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempStorage, "SubStream1", "MediaType1", false, pBytes, sPass ) )
                return false;

            if ( !m_aTestHelper.commitStorage( xTempStorage ) )
                return false;

            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempStorage, "SubStream2", "MediaType2", false, pBytes, sPass ) )
                return false;

            if ( !m_aTestHelper.commitStorage( xTempStorage ) )
                return false;

            // dispose used storages to free resources
            if ( !m_aTestHelper.disposeStorage( xTempStorage ) )
                return false;


            // reopen the target storage readonly, and check contents


            // the temporary file must not be locked any more after storage disposing
            pArgs[1] = new Integer( ElementModes.READ );
            Object oResultStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            XStorage xResultStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oResultStorage );
            if ( xResultStorage == null )
            {
                m_aTestHelper.Error( "Can't reopen storage based on temporary file!" );
                return false;
            }

            if ( !m_aTestHelper.checkEncrStream( xResultStorage, "SubStream1", "MediaType1", pBytes, sPass ) )
                return false;

            if ( !m_aTestHelper.checkEncrStream( xResultStorage, "SubStream2", "MediaType2", pBytes, sPass ) )
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

