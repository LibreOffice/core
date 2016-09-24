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
import com.sun.star.io.XStream;
import com.sun.star.io.XInputStream;

import com.sun.star.embed.*;
import com.sun.star.beans.StringPair;

import share.LogWriter;
import complex.ofopxmlstorages.TestHelper;
import complex.ofopxmlstorages.StorageTest;

public class Test02 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public Test02( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "Test02: " );
    }

    public boolean test()
    {
        try
        {
            StringPair[][] aRelations =
                { { new StringPair( "Id", "Num1" ) },
                  { new StringPair( "Target", "TargetURLValue" ), new StringPair( "Id", "Num6" ) },
                  { new StringPair( "Target", "" ), new StringPair( "Id", "Num7" ) },
                  { new StringPair( "Id", "Num2" ), new StringPair( "TargetMode", "Internal" ), new StringPair( "Type", "unknown" ), new StringPair( "Target", "URL value" ) },
                  { new StringPair( "Id", "Num3" ), new StringPair( "TargetMode", "Internal" ), new StringPair( "Type", "unknown" ), new StringPair( "Target", "URL value" ) },
                  { new StringPair( "Id", "Num4" ), new StringPair( "TargetMode", "Internal" ), new StringPair( "Type", "unknown" ), new StringPair( "Target", "URL value" ) },
                  { new StringPair( "Id", "Num5" ), new StringPair( "TargetMode", "" ), new StringPair( "Type", "unknown" ), new StringPair( "Target", "URL value" ) }
                };


            XStream xTempFileStream = m_aTestHelper.CreateTempFileStream( m_xMSF );
            if ( xTempFileStream == null )
                return false;

            // create storage based on the temporary stream
            XStorage xTempStorage = m_aTestHelper.createStorageFromStream( m_xStorageFactory,
                                                                            xTempFileStream,
                                                                            ElementModes.WRITE );
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
                                                        aRelations ) )
                return false;

            // set Relations for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempStorage,
                                                            true,
                                                            ElementModes.WRITE,
                                                            aRelations ) )
                return false;

            // set Relations for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempSubStorage,
                                                            false,
                                                            ElementModes.WRITE,
                                                            aRelations ) )
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


            // close the output part of the temporary stream
            // the output part must present since we already wrote to the stream
            if ( !m_aTestHelper.closeOutput( xTempFileStream ) )
                return false;

            XInputStream xTempInStream = m_aTestHelper.getInputStream( xTempFileStream );
            if ( xTempInStream == null )
                return false;


            // open input stream
            XStorage xResultStorage = m_aTestHelper.createStorageFromInputStream( m_xStorageFactory, xTempInStream );
            if ( xResultStorage == null )
            {
                m_aTestHelper.Error( "Can't open storage based on input stream!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xResultStorage, true, ElementModes.READ, aRelations ) )
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
                                                        aRelations ) )
                return false;

            if ( !m_aTestHelper.checkStream( xResultSubStorage, "SubStream1", "MediaType1", pBytes1, aRelations ) )
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

