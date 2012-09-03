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

import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import com.sun.star.embed.*;
import com.sun.star.container.XNameAccess;

public class Test03 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public Test03( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( "Test03: " );
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
            if ( !m_aTestHelper.WriteBytesToSubstream( xTempStorage, "SubStream1", "MediaType1", true, pBytes1 ) )
                return false;

            byte pBytes2[] = { 2, 2, 2, 2, 2 };

            // open a new substream, set "MediaType" and "Compressed" properties to it and write some bytes
            if ( !m_aTestHelper.WriteBytesToSubstream( xTempSubStorage, "SubStream2", "MediaType2", false, pBytes2 ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempSubStorage,
                                                            "MediaType3",
                                                            false,
                                                            ElementModes.WRITE ) )
                return false;

            if ( !m_aTestHelper.commitStorage( xTempSubStorage ) )
                return false;

            if ( !m_aTestHelper.disposeStorage( xTempSubStorage ) )
                return false;

            // ================================================
            // check storage hyerarchy tree
            // ================================================

            // check that isStorageElement() and isStreamElement reacts to nonexisting object correctly
            try {
                xTempStorage.isStorageElement( "does not exist" );
                m_aTestHelper.Error( "Nonexisting element doesn't detected by isStorageElement() call!" );
                return false;
            }
            catch( com.sun.star.container.NoSuchElementException ne )
            {
            }
            catch( Exception e )
            {
                m_aTestHelper.Error( "Wrong exception is thrown by isStorageElement() call: " + e );
                return false;
            }

            try {
                xTempStorage.isStreamElement( "does not exist" );
                m_aTestHelper.Error( "Nonexisting element doesn't detected by isStreamElement() call!" );
                return false;
            }
            catch( com.sun.star.container.NoSuchElementException ne )
            {
            }
            catch( Exception e )
            {
                m_aTestHelper.Error( "Wrong exception is thrown by isStreamElement() call: " + e );
                return false;
            }

            XNameAccess xRootNameAccess = (XNameAccess) UnoRuntime.queryInterface( XNameAccess.class, xTempStorage );
            if ( xRootNameAccess == null )
            {
                m_aTestHelper.Error( "Root storage doesn't support XNameAccess!" );
                return false;
            }

            try {
                if ( !xTempStorage.isStorageElement( "SubStorage1" ) || xTempStorage.isStreamElement( "SubStorage1" ) )
                {
                    m_aTestHelper.Error( "Child 'SubStorage1' can not be detected as storage!" );
                    return false;
                }

                if ( xTempStorage.isStorageElement( "SubStream1" ) || !xTempStorage.isStreamElement( "SubStream1" ) )
                {
                    m_aTestHelper.Error( "Child 'SubStream1' can not be detected as stream!" );
                    return false;
                }
            }
            catch( Exception e )
            {
                m_aTestHelper.Error( "Child's type can not be detected, exception: " + e );
                return false;
            }


            // check that root storage contents are represented correctly
            String sRootCont[] = xRootNameAccess.getElementNames();

            if ( sRootCont.length != 2 )
            {
                m_aTestHelper.Error( "Root storage contains wrong amount of children!" );
                return false;
            }

            if ( !( sRootCont[0].equals( "SubStorage1" ) && sRootCont[1].equals( "SubStream1" )
                 || sRootCont[0].equals( "SubStream1" ) && sRootCont[1].equals( "SubStorage1" ) )
              || !( xRootNameAccess.hasByName( "SubStream1" ) && xRootNameAccess.hasByName( "SubStorage1" ) ) )
            {
                m_aTestHelper.Error( "Root storage contains wrong list of children!" );
                return false;
            }

            // get storage through XNameAccess
            XStorage xResultSubStorage = getStorageFromNameAccess( xRootNameAccess, "SubStorage1" );
            if ( xResultSubStorage == null )
                return false;

            if ( !m_aTestHelper.checkStorageProperties( xResultSubStorage, "MediaType3", false, ElementModes.READ ) )
                return false;

            XNameAccess xChildAccess = (XNameAccess) UnoRuntime.queryInterface( XNameAccess.class, xResultSubStorage );
            if ( xChildAccess == null )
            {
                m_aTestHelper.Error( "Child storage doesn't support XNameAccess!" );
                return false;
            }

            if ( !xChildAccess.hasByName( "SubStream2" )
              || !xResultSubStorage.isStreamElement( "SubStream2" )
              || xResultSubStorage.isStorageElement( "SubStream2" ) )
            {
                m_aTestHelper.Error( "'SubStream2' can not be detected as child stream element of 'SubStorage1'!" );
                return false;
            }

            return true;
        }
        catch( Exception e )
        {
            m_aTestHelper.Error( "Exception: " + e );
            return false;
        }
    }

    public XStorage getStorageFromNameAccess( XNameAccess xAccess, String sName )
    {
        try
        {
            Object oStorage = xAccess.getByName( sName );
            XStorage xResult = (XStorage) UnoRuntime.queryInterface( XStorage.class, oStorage );

            if ( xResult != null )
                return xResult;
            else
                m_aTestHelper.Error( "Can't retrieve substorage '" + sName + "' through XNameAccess!" );
        }
        catch( Exception e )
        {
            m_aTestHelper.Error( "Can't retrieve substorage '" + sName + "' through XNameAccess, exception: " + e );
        }

        return null;
    }

}

