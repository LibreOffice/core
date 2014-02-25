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

public class RegressionTest_i30400 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public RegressionTest_i30400( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "RegressionTest_i30400: " );
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
            byte pBytes1[] = { 1, 1, 1, 1, 1 };
            String pPass1 = "1, 2, 3, 4, 5";

            Object oTempStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            XStorage xTempStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
            if ( xTempStorage == null )
            {
                m_aTestHelper.Error( "Can't create temporary storage representation!" );
                return false;
            }

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToSubstream( xTempStorage, "Stream1", "MediaType1", true, pBytes1 ) )
                return false;

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempStorage, "EncrStream1", "MediaType2", true, pBytes1, pPass1 ) )
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
            if ( !m_aTestHelper.WriteBytesToSubstream( xTempSubStorage, "SubStream1", "MediaType3", true, pBytes1 ) )
                return false;

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToEncrSubstream( xTempSubStorage, "SubEncrStream1", "MediaType4", true, pBytes1, pPass1 ) )
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
            if ( !m_aTestHelper.WriteBytesToSubstream( xTempSubSubStorage, "SubSubStream1", "MediaType5", true, pBytes1 ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempSubSubStorage,
                                                            "MediaType6",
                                                            false,
                                                            ElementModes.WRITE ) )
                return false;


            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempSubStorage,
                                                            "MediaType7",
                                                            false,
                                                            ElementModes.WRITE ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempStorage,
                                                            "MediaType8",
                                                            true,
                                                            ElementModes.WRITE ) )
                return false;


            // check the copying with renaming


            if ( !TestCopyWithRenaming( xTempStorage, xTempSubStorage, xTempSubSubStorage ) )
                return false;


            // commit the storages


            // commit lowlevel substorage
            if ( !m_aTestHelper.commitStorage( xTempSubSubStorage ) )
                return false;

            // commit substorage
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


            // reopen the target storage readonly, and check the copying with renaming


            pArgs[1] = new Integer( ElementModes.READ );
            oTempStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            xTempStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
            if ( xTempStorage == null )
            {
                m_aTestHelper.Error( "Can't create temporary storage representation!" );
                return false;
            }

            // open the substorages

            xTempSubStorage = m_aTestHelper.openSubStorage( xTempStorage,
                                                            "SubStorage1",
                                                            ElementModes.READ );
            if ( xTempSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // open the lowlevel substorages

            xTempSubSubStorage = m_aTestHelper.openSubStorage( xTempSubStorage,
                                                                "SubSubStorage1",
                                                                ElementModes.READ );
            if ( xTempSubSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // test the copying with renaming
            if ( !TestCopyWithRenaming( xTempStorage, xTempSubStorage, xTempSubSubStorage ) )
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


    public boolean TestCopyWithRenaming( XStorage xTempStorage, XStorage xTempSubStorage, XStorage xTempSubSubStorage )
        throws com.sun.star.uno.Exception
    {

        // create a second temporary stream and copy all the staff there
        // with renaming, check the success


        XStream xTempFileStream2 = m_aTestHelper.CreateTempFileStream( m_xMSF );
        if ( xTempFileStream2 == null )
            return false;

        Object pArgs[] = new Object[2];
        pArgs[0] = (Object) xTempFileStream2;
        pArgs[1] = new Integer( ElementModes.WRITE );
        byte pBytes1[] = { 1, 1, 1, 1, 1 };
        String pPass1 = "1, 2, 3, 4, 5";

        Object oTempStorage2 = m_xStorageFactory.createInstanceWithArguments( pArgs );
        XStorage xTempStorage2 = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage2 );
        if ( xTempStorage2 == null )
        {
            m_aTestHelper.Error( "Can't create temporary storage representation!" );
            return false;
        }

        // open a new substorage
        XStorage xTempSubStorage2 = m_aTestHelper.openSubStorage( xTempStorage2,
                                                                "SubStorage1_target",
                                                                ElementModes.WRITE );
        if ( xTempSubStorage2 == null )
        {
            m_aTestHelper.Error( "Can't create substorage!" );
            return false;
        }

        // open a new substorage in the existing substorage
        XStorage xTempSubSubStorage2 = m_aTestHelper.openSubStorage( xTempSubStorage2,
                                                                    "SubSubStorage1_target",
                                                                    ElementModes.WRITE );
        if ( xTempSubSubStorage2 == null )
        {
            m_aTestHelper.Error( "Can't create substorage!" );
            return false;
        }

        // make a copy with renaming on lowerest level
        if ( !m_aTestHelper.copyElementTo( xTempSubSubStorage, "SubSubStream1", xTempSubSubStorage2, "SubSubStream1_renamed" ) )
            return false;

        // make a copy with renaming on the next level

        if ( !m_aTestHelper.copyElementTo( xTempSubStorage, "SubStream1", xTempSubStorage2, "SubStream1_renamed" ) )
            return false;

        if ( !m_aTestHelper.copyElementTo( xTempSubStorage, "SubEncrStream1", xTempSubStorage2, "SubEncrStream1_renamed" ) )
            return false;

        if ( !m_aTestHelper.copyElementTo( xTempSubStorage, "SubSubStorage1", xTempSubStorage2, "SubSubStorage1_renamed" ) )
            return false;

        // make a copy with renaming of subelements of the root storage

        if ( !m_aTestHelper.copyElementTo( xTempStorage, "Stream1", xTempStorage2, "Stream1_renamed" ) )
            return false;

        if ( !m_aTestHelper.copyElementTo( xTempStorage, "EncrStream1", xTempStorage2, "EncrStream1_renamed" ) )
            return false;

        if ( !m_aTestHelper.copyElementTo( xTempStorage, "SubStorage1", xTempStorage2, "SubStorage1_renamed" ) )
            return false;


        // commit the storages, and check the renaming in all stages


        // commit substorage to let the renaming take place
        if ( !m_aTestHelper.commitStorage( xTempSubSubStorage2 ) )
            return false;

        // commit substorage to let the renaming take place
        if ( !m_aTestHelper.commitStorage( xTempSubStorage2 ) )
            return false;

        // commit the root storage so the contents must be stored now
        if ( !m_aTestHelper.commitStorage( xTempStorage2 ) )
            return false;


        // dispose the storages


        // dispose lowerest substorage
        if ( !m_aTestHelper.disposeStorage( xTempSubSubStorage2 ) )
            return false;

        // dispose substorage
        if ( !m_aTestHelper.disposeStorage( xTempSubStorage2 ) )
            return false;

        // dispose the temporary storage
        if ( !m_aTestHelper.disposeStorage( xTempStorage2 ) )
            return false;


        // reopen the target storage readonly, and check the contents


        pArgs[1] = new Integer( ElementModes.READ );
        oTempStorage2 = m_xStorageFactory.createInstanceWithArguments( pArgs );
        xTempStorage2 = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage2 );
        if ( xTempStorage2 == null )
        {
            m_aTestHelper.Error( "Can't create temporary storage representation!" );
            return false;
        }

        // open the substorages

        XStorage xTempSubStorage2_target = m_aTestHelper.openSubStorage( xTempStorage2,
                                                                "SubStorage1_target",
                                                                ElementModes.READ );
        if ( xTempSubStorage2_target == null )
        {
            m_aTestHelper.Error( "Can't create substorage!" );
            return false;
        }

        XStorage xTempSubStorage2_renamed = m_aTestHelper.openSubStorage( xTempStorage2,
                                                                "SubStorage1_renamed",
                                                                ElementModes.READ );
        if ( xTempSubStorage2_renamed == null )
        {
            m_aTestHelper.Error( "Can't create substorage!" );
            return false;
        }

        // open the lowlevel substorages

        XStorage xTempSubSubStorage2_inRenamed = m_aTestHelper.openSubStorage( xTempSubStorage2_renamed,
                                                                                "SubSubStorage1",
                                                                                ElementModes.READ );
        if ( xTempSubSubStorage2_inRenamed == null )
        {
            m_aTestHelper.Error( "Can't create substorage!" );
            return false;
        }

        XStorage xTempSubSubStorage2_renamed = m_aTestHelper.openSubStorage( xTempSubStorage2_target,
                                                                            "SubSubStorage1_renamed",
                                                                            ElementModes.READ );
        if ( xTempSubSubStorage2_renamed == null )
        {
            m_aTestHelper.Error( "Can't create substorage!" );
            return false;
        }

        XStorage xTempSubSubStorage2_target = m_aTestHelper.openSubStorage( xTempSubStorage2_target,
                                                                            "SubSubStorage1_target",
                                                                            ElementModes.READ );
        if ( xTempSubSubStorage2_target == null )
        {
            m_aTestHelper.Error( "Can't create substorage!" );
            return false;
        }

        // check the storages

        if ( !m_aTestHelper.checkStorageProperties( xTempSubSubStorage2_inRenamed, "MediaType6", false, ElementModes.READ ) )
            return false;

        if ( !m_aTestHelper.checkStorageProperties( xTempSubSubStorage2_renamed, "MediaType6", false, ElementModes.READ ) )
            return false;

        if ( !m_aTestHelper.checkStorageProperties( xTempSubStorage2_renamed, "MediaType7", false, ElementModes.READ ) )
            return false;


        // check the streams


        // sub sub level

        if ( !m_aTestHelper.checkStream( xTempSubSubStorage2_inRenamed, "SubSubStream1", "MediaType5", true, pBytes1 ) )
            return false;

        if ( !m_aTestHelper.checkStream( xTempSubSubStorage2_renamed, "SubSubStream1", "MediaType5", true, pBytes1 ) )
            return false;

        if ( !m_aTestHelper.checkStream( xTempSubSubStorage2_target, "SubSubStream1_renamed", "MediaType5", true, pBytes1 ) )
            return false;

        // sub level

        if ( !m_aTestHelper.checkStream( xTempSubStorage2_renamed, "SubStream1", "MediaType3", true, pBytes1 ) )
            return false;

        if ( !m_aTestHelper.checkEncrStream( xTempSubStorage2_renamed, "SubEncrStream1", "MediaType4", pBytes1, pPass1 ) )
            return false;

        if ( !m_aTestHelper.checkStream( xTempSubStorage2_target, "SubStream1_renamed", "MediaType3", true, pBytes1 ) )
            return false;

        if ( !m_aTestHelper.checkEncrStream( xTempSubStorage2_target, "SubEncrStream1_renamed", "MediaType4", pBytes1, pPass1 ) )
            return false;

        // root storage level

        if ( !m_aTestHelper.checkStream( xTempStorage2, "Stream1_renamed", "MediaType1", true, pBytes1 ) )
            return false;

        if ( !m_aTestHelper.checkEncrStream( xTempStorage2, "EncrStream1_renamed", "MediaType2", pBytes1, pPass1 ) )
            return false;

        // the storage is based on the temporary stream so it can be left undisposed, since it does not lock
        // any resource, later the garbage collector will release the object and it must die by refcount

        return true;
    }
}

