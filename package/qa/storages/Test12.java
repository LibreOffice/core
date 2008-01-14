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

public class Test12 implements StorageTest {

    XMultiServiceFactory m_xMSF;
    XSingleServiceFactory m_xStorageFactory;
    TestHelper m_aTestHelper;

    public Test12( XMultiServiceFactory xMSF, XSingleServiceFactory xStorageFactory, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_xStorageFactory = xStorageFactory;
        m_aTestHelper = new TestHelper( aLogWriter, "Test12: " );
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

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempStorage,
                                                            "MediaType2",
                                                            true,
                                                            ElementModes.WRITE ) )
                return false;

            // set "MediaType" property for storages and check that "IsRoot" and "OpenMode" properties are set correctly
            if ( !m_aTestHelper.setStorageTypeAndCheckProps( xTempSubStorage,
                                                            "MediaType3",
                                                            false,
                                                            ElementModes.WRITE ) )
                return false;

            // commit substorage first
            if ( !m_aTestHelper.commitStorage( xTempSubStorage ) )
                return false;

            // commit the root storage so the contents must be stored now
            if ( !m_aTestHelper.commitStorage( xTempStorage ) )
                return false;

            // dispose substorage
            if ( !m_aTestHelper.disposeStorage( xTempSubStorage ) )
                return false;

            // ================================================
            // check substorage
            // ================================================

            if ( !checkSubStorages( xTempStorage, pBytes1 ) )
                return false;

            // dispose used storage to free resources
            if ( !m_aTestHelper.disposeStorage( xTempStorage ) )
                return false;

            // ================================================
            // now check all the written information with readwrite access
            // ================================================

            Object oResWriteStorage = m_xStorageFactory.createInstanceWithArguments( pArgs );
            XStorage xResWriteStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oResWriteStorage );
            if ( xResWriteStorage == null )
            {
                m_aTestHelper.Error( "Can't open storage based on input stream!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xResWriteStorage, "MediaType2", true, ElementModes.WRITE ) )
                return false;

            if( !checkSubStorages( xResWriteStorage, pBytes1 ) )
                return false;

            // try to open for writing after opening for reading
            XStorage xResWSubStorage = m_aTestHelper.openSubStorage( xResWriteStorage,
                                                                    "SubStorage1",
                                                                    ElementModes.WRITE );
            if ( xResWSubStorage == null )
            {
                m_aTestHelper.Error( "Can't open substorage for writing after it was opened for reading!" );
                return false;
            }

            if ( !m_aTestHelper.checkStorageProperties( xResWSubStorage, "MediaType3", false, ElementModes.WRITE ) )
                return false;

            if ( !m_aTestHelper.checkStream( xResWSubStorage, "SubStream1", "MediaType1", true, pBytes1 ) )
                return false;

            // dispose used storage to free resources
            if ( !m_aTestHelper.disposeStorage( xResWriteStorage ) )
                return false;


            // ================================================
            // now check all the written information with readonly access
            // ================================================

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

            if ( !m_aTestHelper.checkStorageProperties( xResultStorage, "MediaType2", true, ElementModes.READ ) )
                return false;

            if( !checkSubStorages( xResultStorage, pBytes1 ) )
                return false;

            return true;
        }
        catch( Exception e )
        {
            m_aTestHelper.Error( "Exception: " + e );
            return false;
        }
    }

    private boolean checkSubStorages( XStorage xStorage, byte[] pBytes1 )
    {
        XStorage xReadSubStorage1 = m_aTestHelper.openSubStorage( xStorage,
                                                                "SubStorage1",
                                                                ElementModes.READ );

        XStorage xReadSubStorage2 = m_aTestHelper.openSubStorage( xStorage,
                                                                "SubStorage1",
                                                                ElementModes.READ );

        if ( xReadSubStorage1 == null || xReadSubStorage2 == null )
        {
            m_aTestHelper.Error( "Can't open substorage for reading!" );
            return false;
        }

        if ( !m_aTestHelper.checkStorageProperties( xReadSubStorage1, "MediaType3", false, ElementModes.READ ) )
            return false;

        if ( !m_aTestHelper.checkStorageProperties( xReadSubStorage2, "MediaType3", false, ElementModes.READ ) )
            return false;

        if ( !m_aTestHelper.checkStream( xReadSubStorage1, "SubStream1", "MediaType1", true, pBytes1 ) )
            return false;

        if ( !m_aTestHelper.checkStream( xReadSubStorage2, "SubStream1", "MediaType1", true, pBytes1 ) )
            return false;

        if ( !m_aTestHelper.disposeStorage( xReadSubStorage1 ) )
            return false;

        if ( !m_aTestHelper.disposeStorage( xReadSubStorage2 ) )
            return false;

        return true;
    }
}

