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

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.ElementExistException;

import com.sun.star.embed.*;

public class Test06 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public Test06( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( "Test06: " );
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

            try
            {
                xTempStorage.copyToStorage( null );
                m_aTestHelper.Error( "The method must throw an exception because of illegal parameter!" );
                return false;
            }
            catch( com.sun.star.lang.IllegalArgumentException iae )
            {}
            catch( com.sun.star.uno.Exception ue )
            {}
            catch( Exception e )
            {
                m_aTestHelper.Error( "Unexpected excepion because of illegal parameter : " + e );
                return false;
            }

            // open new substorages
            XStorage xTempSubStorage1 = m_aTestHelper.openSubStorage( xTempStorage,
                                                                    "SubStorage1",
                                                                    ElementModes.WRITE );
            XStorage xTempSubStorage2 = m_aTestHelper.openSubStorage( xTempStorage,
                                                                    "SubStorage2",
                                                                    ElementModes.WRITE );
            if ( xTempSubStorage1 == null || xTempSubStorage2 == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // in case stream is open for reading it must exist
            try
            {
                xTempSubStorage1.openStreamElement( "NonExistingStream", ElementModes.READ );
                m_aTestHelper.Error( "The method must throw an exception in case of try to open nonexistent stream for reading!" );
                return false;
            }
            catch( com.sun.star.uno.Exception ue )
            {}
            catch( Exception e )
            {
                m_aTestHelper.Error( "Unexpected excepion in case of try to open nonexistent stream for reading : " + e );
                return false;
            }

            // in case a storage is open for reading it must exist
            try
            {
                xTempSubStorage1.openStreamElement( "NonExistingStorage", ElementModes.READ );
                m_aTestHelper.Error( "The method must throw an exception in case of try to open nonexistent storage for reading!" );
                return false;
            }
            catch( com.sun.star.uno.Exception ue )
            {}
            catch( Exception e )
            {
                m_aTestHelper.Error( "Unexpected excepion in case of try to open nonexistent storage for reading : " + e );
                return false;
            }

            // in case of removing nonexistent element an exception must be thrown
            try
            {
                xTempSubStorage1.removeElement( "NonExistingElement" );
                m_aTestHelper.Error( "An exception must be thrown in case of removing nonexistent element!" );
                return false;
            }
            catch( com.sun.star.container.NoSuchElementException ne )
            {}
            catch( Exception e )
            {
                m_aTestHelper.Error( "Unexpected excepion in case of try to remove nonexistent element : " + e );
                return false;
            }

            // in case of renaming of nonexistent element an exception must be thrown
            try
            {
                xTempSubStorage1.renameElement( "NonExistingElement", "NewName" );
                m_aTestHelper.Error( "An exception must be thrown in case of renaming nonexistent element!" );
                return false;
            }
            catch( com.sun.star.container.NoSuchElementException ne )
            {}
            catch( Exception e )
            {
                m_aTestHelper.Error( "Unexpected excepion in case of try to rename nonexistent element : " + e );
                return false;
            }

            // in case of renaming to a name of existent element an exception must be thrown
            try
            {
                xTempStorage.renameElement( "SubStorage1", "SubStorage2" );
                m_aTestHelper.Error( "An exception must be thrown in case of renaming to the name of existent element!" );
                return false;
            }
            catch( com.sun.star.container.ElementExistException ee )
            {}
            catch( Exception e )
            {
                m_aTestHelper.Error( "Unexpected excepion in case of try to rename to the name of existent element : " + e );
                return false;
            }

            // in case of copying target storage must be provided
            try
            {
                xTempStorage.copyElementTo( "SubStorage1", null, "SubStorage1" );
                m_aTestHelper.Error( "An exception must be thrown in case empty reference is provided as target for copying!" );
                return false;
            }
            catch( com.sun.star.lang.IllegalArgumentException iae )
            {}
            catch( com.sun.star.uno.Exception ue )
            {}
            catch( Exception e )
            {
                m_aTestHelper.Error( "Unexpected excepion in case empty reference is provieded as target for copying : " + e );
                return false;
            }

            // in case of moving target storage must be provided
            try
            {
                xTempStorage.moveElementTo( "SubStorage1", null, "SubStorage1" );
                m_aTestHelper.Error( "An exception must be thrown in case empty reference is provided as target for moving!" );
                return false;
            }
            catch( com.sun.star.lang.IllegalArgumentException iae )
            {}
            catch( com.sun.star.uno.Exception ue )
            {}
            catch( Exception e )
            {
                m_aTestHelper.Error( "Unexpected excepion in case empty reference is provieded as target for moving : " + e );
                return false;
            }


            // prepare target for further testings

            // create new temporary storage based on arbitrary medium
            Object oTargetStorage = m_xStorageFactory.createInstance();
            XStorage xTargetStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTargetStorage );
            if ( xTargetStorage == null )
            {
                m_aTestHelper.Error( "Can't create temporary storage representation!" );
                return false;
            }

            // open a new substorage
            XStorage xTargetSubStorage = m_aTestHelper.openSubStorage( xTargetStorage,
                                                                    "SubStorage1",
                                                                    ElementModes.WRITE );
            if ( xTargetSubStorage == null )
            {
                m_aTestHelper.Error( "Can't create substorage!" );
                return false;
            }

            // in case of copying of nonexistent element an exception must be thrown
            try
            {
                xTempStorage.copyElementTo( "Nonexistent element", xTargetStorage, "Target" );
                m_aTestHelper.Error( "An exception must be thrown in case of copying of nonexisting element!" );
                return false;
            }
            catch( com.sun.star.container.NoSuchElementException ne )
            {}
            catch( Exception e )
            {
                m_aTestHelper.Error( "Unexpected excepion in case of copying of nonexistent element: " + e );
                return false;
            }

            // in case of moving of nonexistent element an exception must be thrown
            try
            {
                xTempStorage.moveElementTo( "Nonexistent element", xTargetStorage, "Target" );
                m_aTestHelper.Error( "An exception must be thrown in case of moving of nonexisting element!" );
                return false;
            }
            catch( com.sun.star.container.NoSuchElementException ne )
            {}
            catch( Exception e )
            {
                m_aTestHelper.Error( "Unexpected excepion in case of moving of nonexistent element: " + e );
                return false;
            }

            // in case target for copying already exists an exception must be thrown
            try
            {
                xTempStorage.copyElementTo( "SubStorage1", xTargetStorage, "SubStorage1" );
                m_aTestHelper.Error( "An exception must be thrown in case target for copying already exists!" );
                return false;
            }
            catch( com.sun.star.container.ElementExistException ee )
            {}
            catch( Exception e )
            {
                m_aTestHelper.Error( "Unexpected excepion in case target for copying already exists: " + e );
                return false;
            }

            // in case target for moving already exists an exception must be thrown
            try
            {
                xTempStorage.moveElementTo( "SubStorage1", xTargetStorage, "SubStorage1" );
                m_aTestHelper.Error( "An exception must be thrown in case target for moving already exists!" );
                return false;
            }
            catch( com.sun.star.container.ElementExistException ee )
            {}
            catch( Exception e )
            {
                m_aTestHelper.Error( "Unexpected excepion in case target for moving already exists: " + e );
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

}

