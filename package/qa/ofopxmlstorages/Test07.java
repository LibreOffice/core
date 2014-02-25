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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import com.sun.star.container.XNameAccess;
import com.sun.star.io.XStream;

import com.sun.star.embed.*;
import com.sun.star.beans.StringPair;

import share.LogWriter;
import complex.ofopxmlstorages.TestHelper;
import complex.ofopxmlstorages.StorageTest;

public class Test07 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public Test07( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "Test07: " );
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
            // create temporary storage based on arbitrary medium
            // after such a storage is closed it is lost
            XStorage xTempStorage = m_aTestHelper.createTempStorage( m_xMSF, m_xStorageFactory );
            if ( xTempStorage == null )
            {
                m_aTestHelper.Error( "Can't create temporary storage representation!" );
                return false;
            }

            byte pBytes1[] = { 1, 1, 1, 1, 1 };

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToSubstream( xTempStorage,
                                                        "SubStream1",
                                                        "MediaType1",
                                                        true,
                                                        pBytes1,
                                                        aRelations1 ) )
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

            byte pBytes2[] = { 2, 2, 2, 2, 2 };

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToSubstream( xTempSubStorage,
                                                        "SubStream2",
                                                        "MediaType2",
                                                        true,
                                                        pBytes2,
                                                        aRelations2 ) )
                return false;

            // set Relations for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempStorage,
                                                            true,
                                                            ElementModes.WRITE,
                                                            aRelations2 ) )
                return false;

            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempSubStorage,
                                                            false,
                                                            ElementModes.WRITE,
                                                            aRelations2 ) )
                return false;


            // check cloning at current state


            // the new storage still was not commited so the clone must be empty
            XStorage xClonedSubStorage = m_aTestHelper.cloneSubStorage( m_xMSF, m_xStorageFactory, xTempStorage, "SubStorage1" );

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

            if ( !m_aTestHelper.checkStorageProperties( xClonedSubStorage,
                                                        true,
                                                        ElementModes.WRITE,
                                                        new StringPair[0][0] ) )
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
            XStream xClonedSubStream = m_aTestHelper.cloneSubStream( xTempStorage, "SubStream1" );
            if ( !m_aTestHelper.InternalCheckStream( xClonedSubStream,
                                                    "SubStream1",
                                                    "MediaType1",
                                                    pBytes1,
                                                    aRelations1 ) )
                return false;

            if ( !m_aTestHelper.disposeStream( xClonedSubStream, "SubStream1" ) )
                return false;


            // commit substorage and check cloning


            if ( !m_aTestHelper.commitStorage( xTempSubStorage ) )
                return false;

            xClonedSubStorage = m_aTestHelper.cloneSubStorage( m_xMSF, m_xStorageFactory, xTempStorage, "SubStorage1" );
            if ( xClonedSubStorage == null )
            {
                m_aTestHelper.Error( "The result of clone is empty!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xClonedSubStorage,
                                                        true,
                                                        ElementModes.WRITE,
                                                        aRelations2 ) )
                return false;

            if ( !m_aTestHelper.checkStream( xClonedSubStorage,
                                            "SubStream2",
                                            "MediaType2",
                                            pBytes2,
                                            aRelations2 ) )
                return false;

            XStorage xCloneOfRoot = m_aTestHelper.cloneStorage( m_xMSF, m_xStorageFactory, xTempStorage );
            if ( xCloneOfRoot == null )
            {
                m_aTestHelper.Error( "The result of root clone is empty!" );
                return false;
            }

            XNameAccess xCloneOfRootNA = (XNameAccess) UnoRuntime.queryInterface( XNameAccess.class, xCloneOfRoot );
            if ( xCloneOfRootNA == null )
            {
                m_aTestHelper.Error( "XNameAccess is not implemented by the root clone!" );
                return false;
            }

            if ( xCloneOfRootNA.hasElements() )
            {
                m_aTestHelper.Error( "The root storage still was not commited so it's clone must be empty!" );
                return false;
            }

            if ( !m_aTestHelper.disposeStorage( xCloneOfRoot ) )
                return false;

            xCloneOfRoot = null;


            // commit root storage and check cloning


            if ( !m_aTestHelper.commitStorage( xTempStorage ) )
                return false;

            xCloneOfRoot = m_aTestHelper.cloneStorage( m_xMSF, m_xStorageFactory, xTempStorage );
            if ( xCloneOfRoot == null )
            {
                m_aTestHelper.Error( "The result of root clone is empty!" );
                return false;
            }

            XStorage xSubStorageOfClone = xCloneOfRoot.openStorageElement( "SubStorage1", ElementModes.READ );
            if ( xSubStorageOfClone == null )
            {
                m_aTestHelper.Error( "The result of root clone is wrong!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xSubStorageOfClone,
                                                        false,
                                                        ElementModes.READ,
                                                        aRelations2 ) )
                return false;

            if ( !m_aTestHelper.checkStream( xSubStorageOfClone,
                                            "SubStream2",
                                            "MediaType2",
                                            pBytes2,
                                            aRelations2 ) )
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

