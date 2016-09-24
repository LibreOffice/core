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

package complex.ofopxmlstorages;

import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import com.sun.star.embed.*;
import com.sun.star.beans.StringPair;

import share.LogWriter;
import complex.ofopxmlstorages.TestHelper;
import complex.ofopxmlstorages.StorageTest;

public class Test01 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public Test01( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "Test01: " );
    }

    public boolean test()
    {
        StringPair[][] aRelations1 =
            { { new StringPair( "Id", "Num1" ) },
              { new StringPair( "Target", "TargetURLValue1" ), new StringPair( "Id", "Num6" ) },
              { new StringPair( "Target", "" ), new StringPair( "Id", "Num7" ) },
              { new StringPair( "Id", "Num2" ), new StringPair( "TargetMode", "Internal1" ), new StringPair( "Type", "unknown1" ), new StringPair( "Target", "URL value 1" ) },
              { new StringPair( "Id", "Num3" ), new StringPair( "TargetMode", "Internal1" ), new StringPair( "Type", "unknown1" ), new StringPair( "Target", "URL value 1" ) },
              { new StringPair( "Id", "Num4" ), new StringPair( "TargetMode", "Internal1" ), new StringPair( "Type", "unknown1" ), new StringPair( "Target", "URL value 1" ) },
              { new StringPair( "Id", "Num5" ), new StringPair( "TargetMode", "" ), new StringPair( "Type", "unknown1" ), new StringPair( "Target", "URL value1" ) }
            };

        StringPair[][] aRelations2 =
            { { new StringPair( "Id", "Num1" ) },
              { new StringPair( "Target", "TargetURLValue2" ), new StringPair( "Id", "Num6" ) },
              { new StringPair( "Target", "" ), new StringPair( "Id", "Num7" ) },
              { new StringPair( "Id", "Num2" ), new StringPair( "TargetMode", "Internal2" ), new StringPair( "Type", "unknown2" ), new StringPair( "Target", "URL value 2" ) },
              { new StringPair( "Id", "Num3" ), new StringPair( "TargetMode", "Internal2" ), new StringPair( "Type", "unknown2" ), new StringPair( "Target", "URL value 2" ) },
              { new StringPair( "Id", "Num4" ), new StringPair( "TargetMode", "Internal2" ), new StringPair( "Type", "unknown" ), new StringPair( "Target", "URL value" ) },
              { new StringPair( "Id", "Num5" ), new StringPair( "TargetMode", "" ), new StringPair( "Type", "unknown" ), new StringPair( "Target", "URL value" ) }
            };

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
            XStorage xTempStorage = m_aTestHelper.createTempStorage( m_xMSF, m_xStorageFactory );
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
            if ( !m_aTestHelper.WriteBytesToSubstream( xTempSubStorage,
                                                        "SubStream1",
                                                        "MediaType1",
                                                        true,
                                                        pBytes1,
                                                        aRelations1 ) )
                return false;

            byte pBytes2[] = { 2, 2, 2, 2, 2 };

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToSubstream( xTempSubStorage,
                                                        "SubStream2",
                                                        "MediaType2",
                                                        false,
                                                        pBytes2,
                                                        aRelations2 ) )
                return false;

            // set Relations for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempStorage,
                                                            true,
                                                            ElementModes.WRITE,
                                                            aRelations1 ) )
                return false;

            // set Relations for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempSubStorage,
                                                            false,
                                                            ElementModes.WRITE,
                                                            aRelations1 ) )
                return false;

            // create temporary storage based on a previously created temporary file
            XStorage xTempFileStorage = m_aTestHelper.createStorageFromURL( m_xStorageFactory,
                                                                            sTempFileURL,
                                                                            ElementModes.WRITE );
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
            XStorage xResultStorage = m_aTestHelper.createStorageFromURL( m_xStorageFactory,
                                                                            sTempFileURL,
                                                                            ElementModes.WRITE );
            if ( xResultStorage == null )
            {
                m_aTestHelper.Error( "Can't reopen storage based on temporary file!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xResultStorage,
                                                        true,
                                                        ElementModes.WRITE,
                                                        aRelations1 ) )
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

            if ( !m_aTestHelper.checkStorageProperties( xResultSubStorage,
                                                        false,
                                                        ElementModes.READ,
                                                        aRelations1 ) )
                return false;

            if ( !m_aTestHelper.checkStream( xResultSubStorage,
                                            "SubStream1",
                                            "MediaType1",
                                            pBytes1,
                                            aRelations1 ) )
                return false;

            if ( !m_aTestHelper.checkStream( xResultSubStorage,
                                            "SubStream2",
                                            "MediaType2",
                                            pBytes2,
                                            aRelations2 ) )
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

