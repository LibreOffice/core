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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.AnyConverter;

import com.sun.star.lang.*;
import com.sun.star.embed.*;
import com.sun.star.packages.*;
import com.sun.star.io.*;
import com.sun.star.beans.*;

public class TestHelper  {

    String m_sTestPrefix;

    public TestHelper( String sTestPrefix )
    {
        m_sTestPrefix = sTestPrefix;
    }

    public boolean WriteBytesToStream( XStream xStream,
                                        String sStreamName,
                                        String sMediaType,
                                        boolean bCompressed,
                                        byte[] pBytes )
    {
        // get output stream of substream
        XOutputStream xOutput = xStream.getOutputStream();
        if ( xOutput == null )
        {
            Error( "Can't get XOutputStream implementation from substream '" + sStreamName + "'!" );
            return false;
        }

        // get XTrucate implementation from output stream
        XTruncate xTruncate = (XTruncate) UnoRuntime.queryInterface( XTruncate.class, xOutput );
        if ( xTruncate == null )
        {
            Error( "Can't get XTruncate implementation from substream '" + sStreamName + "'!" );
            return false;
        }

        // write requested byte sequence
        try
        {
            xTruncate.truncate();
            xOutput.writeBytes( pBytes );
        }
        catch( Exception e )
        {
            Error( "Can't write to stream '" + sStreamName + "', exception: " + e );
            return false;
        }

        // get access to the XPropertySet interface
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, xStream );
        if ( xPropSet == null )
        {
            Error( "Can't get XPropertySet implementation from substream '" + sStreamName + "'!" );
            return false;
        }

        // set properties to the stream
        try
        {
            xPropSet.setPropertyValue( "MediaType", sMediaType );
            xPropSet.setPropertyValue( "Compressed", new Boolean( bCompressed ) );
        }
        catch( Exception e )
        {
            Error( "Can't set properties to substream '" + sStreamName + "', exception: " + e );
            return false;
        }

        // check size property of the stream
        try
        {
            int nSize = AnyConverter.toInt( xPropSet.getPropertyValue( "Size" ) );
            if ( nSize != pBytes.length )
            {
                Error( "The 'Size' property of substream '" + sStreamName + "' contains wrong value!" );
                return false;
            }
        }
        catch( Exception e )
        {
            Error( "Can't get 'Size' property from substream '" + sStreamName + "', exception: " + e );
            return false;
        }

        // free the stream resources, garbage collector may remove the object too late
        XComponent xComponent = (XComponent) UnoRuntime.queryInterface( XComponent.class, xStream );
        if ( xComponent == null )
        {
            Error( "Can't get XComponent implementation from substream '" + sStreamName + "'!" );
            return false;
        }
        xComponent.dispose();

        return true;
    }


    public boolean WriteBytesToSubstream( XStorage xStorage,
                                          String sStreamName,
                                          String sMediaType,
                                          boolean bCompressed,
                                          byte[] pBytes )
    {
        // open substream element
        XStream xSubStream = null;
        try
        {
            Object oSubStream = xStorage.openStreamElement( sStreamName, ElementModes.WRITE );
            xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            if ( xSubStream == null )
            {
                Error( "Can't create substream '" + sStreamName + "'!" );
                return false;
            }
        }
        catch( Exception e )
        {
            Error( "Can't create substream '" + sStreamName + "', exception : " + e + "!" );
            return false;
        }

        return WriteBytesToStream( xSubStream, sStreamName, sMediaType, bCompressed, pBytes );
    }

    public boolean WriteBytesToEncrSubstream( XStorage xStorage,
                                              String sStreamName,
                                              String sMediaType,
                                              boolean bCompressed,
                                              byte[] pBytes,
                                              byte[] pPass )
    {
        // open substream element
        XStream xSubStream = null;
        try
        {
            Object oSubStream = xStorage.openEncryptedStreamElement( sStreamName, ElementModes.WRITE, new String(pPass) );
            xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            if ( xSubStream == null )
            {
                Error( "Can't create substream '" + sStreamName + "'!" );
                return false;
            }
        }
        catch( Exception e )
        {
            Error( "Can't create substream '" + sStreamName + "', exception : " + e + "!" );
            return false;
        }

        return WriteBytesToStream( xSubStream, sStreamName, sMediaType, bCompressed, pBytes );
    }

    public boolean WBToSubstrOfEncr( XStorage xStorage,
                                              String sStreamName,
                                              String sMediaType,
                                              boolean bCompressed,
                                              byte[] pBytes,
                                              boolean bEncrypted )
    {
        // open substream element
        XStream xSubStream = null;
        try
        {
            Object oSubStream = xStorage.openStreamElement( sStreamName, ElementModes.WRITE );
            xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            if ( xSubStream == null )
            {
                Error( "Can't create substream '" + sStreamName + "'!" );
                return false;
            }
        }
        catch( Exception e )
        {
            Error( "Can't create substream '" + sStreamName + "', exception : " + e + "!" );
            return false;
        }

        // get access to the XPropertySet interface
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, xSubStream );
        if ( xPropSet == null )
        {
            Error( "Can't get XPropertySet implementation from substream '" + sStreamName + "'!" );
            return false;
        }

        // set properties to the stream
        try
        {
            xPropSet.setPropertyValue( "Encrypted", new Boolean( bEncrypted ) );
        }
        catch( Exception e )
        {
            Error( "Can't set 'Encrypted' property to substream '" + sStreamName + "', exception: " + e );
            return false;
        }

        return WriteBytesToStream( xSubStream, sStreamName, sMediaType, bCompressed, pBytes );
    }

    public int ChangeStreamPass( XStorage xStorage,
                                 String sStreamName,
                                 byte[] pOldPass,
                                 byte[] pNewPass )
    {
        // open substream element
        XStream xSubStream = null;
        try
        {
            Object oSubStream = xStorage.openEncryptedStreamElement( sStreamName, ElementModes.WRITE, new String(pOldPass) );
            xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            if ( xSubStream == null )
            {
                Error( "Can't open substream '" + sStreamName + "'!" );
                return 0;
            }
        }
        catch( Exception e )
        {
            Error( "Can't open substream '" + sStreamName + "', exception : " + e + "!" );
            return 0;
        }


        // change the password for the stream
        XEncryptionProtectedSource xStreamEncryption =
                (XEncryptionProtectedSource) UnoRuntime.queryInterface( XEncryptionProtectedSource.class, xSubStream );

        if ( xStreamEncryption == null )
        {
            Message( "Optional interface XEncryptionProtectedSource is not implemented, feature can not be tested!" );
            return -1;
        }

        try {
            xStreamEncryption.setEncryptionPassword( new String(pNewPass) );
        }
        catch( Exception e )
        {
            Error( "Can't change encryption key of the substream '" + sStreamName + "', exception:" + e );
            return 0;
        }

        // free the stream resources, garbage collector may remove the object too late
        XComponent xComponent = (XComponent) UnoRuntime.queryInterface( XComponent.class, xSubStream );
        if ( xComponent == null )
        {
            Error( "Can't get XComponent implementation from substream '" + sStreamName + "'!" );
            return 0;
        }
        xComponent.dispose();

        return 1;
    }

    public boolean setStorageTypeAndCheckProps( XStorage xStorage, String sMediaType, boolean bIsRoot, int nMode )
    {
        boolean bOk = false;

        // get access to the XPropertySet interface
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, xStorage );
        if ( xPropSet != null )
        {
            try
            {
                // set "MediaType" property to the stream
                xPropSet.setPropertyValue( "MediaType", sMediaType );

                // get "IsRoot" and "OpenMode" properties and control there values
                boolean bPropIsRoot = AnyConverter.toBoolean( xPropSet.getPropertyValue( "IsRoot" ) );
                int nPropMode = AnyConverter.toInt( xPropSet.getPropertyValue( "OpenMode" ) );

                bOk = true;
                if ( bPropIsRoot != bIsRoot )
                {
                    Error( "'IsRoot' property contains wrong value!" );
                    bOk = false;
                }

                if ( ( bIsRoot && ( nPropMode | ElementModes.READ ) != ( nMode | ElementModes.READ ) )
                  || ( !bIsRoot && ( nPropMode & nMode ) != nMode ) )
                {
                    Error( "'OpenMode' property contains wrong value!" );
                    bOk = false;
                }
            }
            catch( Exception e )
            {
                Error( "Can't control properties of substorage, exception: " + e );
            }
        }
        else
        {
            Error( "Can't get XPropertySet implementation from storage!" );
        }

        return bOk;
    }

    public boolean checkStorageProperties( XStorage xStorage, String sMediaType, boolean bIsRoot, int nMode )
    {
        boolean bOk = false;

        // get access to the XPropertySet interface
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, xStorage );
        if ( xPropSet != null )
        {
            try
            {
                // get "MediaType", "IsRoot" and "OpenMode" properties and control there values
                String sPropMediaType = AnyConverter.toString( xPropSet.getPropertyValue( "MediaType" ) );
                boolean bPropIsRoot = AnyConverter.toBoolean( xPropSet.getPropertyValue( "IsRoot" ) );
                int nPropMode = AnyConverter.toInt( xPropSet.getPropertyValue( "OpenMode" ) );

                bOk = true;
                if ( !sPropMediaType.equals( sMediaType ) )
                {
                    Error( "'MediaType' property contains wrong value, expected '"
                            + sMediaType + "', set '" + sPropMediaType + "' !" );
                    bOk = false;
                }

                if ( bPropIsRoot != bIsRoot )
                {
                    Error( "'IsRoot' property contains wrong value!" );
                    bOk = false;
                }

                if ( ( bIsRoot && ( nPropMode | ElementModes.READ ) != ( nMode | ElementModes.READ ) )
                  || ( !bIsRoot && ( nPropMode & nMode ) != nMode ) )
                {
                    Error( "'OpenMode' property contains wrong value!" );
                    bOk = false;
                }
            }
            catch( Exception e )
            {
                Error( "Can't get properties of substorage, exception: " + e );
            }
        }
        else
        {
            Error( "Can't get XPropertySet implementation from storage!" );
        }

        return bOk;
    }

    public boolean InternalCheckStream( XStream xStream,
                                        String sName,
                                        String sMediaType,
                                        byte[] pBytes )
    {
        // get input stream of substream
        XInputStream xInput = xStream.getInputStream();
        if ( xInput == null )
        {
            Error( "Can't get XInputStream implementation from substream '" + sName + "'!" );
            return false;
        }

        byte pContents[][] = new byte[1][]; // ???

        // read contents
        try
        {
            xInput.readBytes( pContents, pBytes.length + 1 );
        }
        catch( Exception e )
        {
            Error( "Can't read from stream '" + sName + "', exception: " + e );
            return false;
        }

        // check size of stream data
        if ( pContents.length == 0 )
        {
            Error( "SubStream '" + sName + "' reading produced disaster!"  );
            return false;
        }

        if ( pBytes.length != pContents[0].length )
        {
            Error( "SubStream '" + sName + "' contains wrong amount of data! (" + pContents[0].length + "/" + pBytes.length + ")" );
            return false;
        }

        // check stream data
        for ( int ind = 0; ind < pBytes.length; ind++ )
        {
            if ( pBytes[ind] != pContents[0][ind] )
            {
                Error( "SubStream '" + sName + "' contains wrong data!" );
                return false;
            }
        }


        // check properties
        boolean bOk = false;

        // get access to the XPropertySet interface
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, xStream );
        if ( xPropSet != null )
        {
            try
            {
                // get "MediaType" and "Size" properties and control there values
                String sPropMediaType = AnyConverter.toString( xPropSet.getPropertyValue( "MediaType" ) );
                int nPropSize = AnyConverter.toInt( xPropSet.getPropertyValue( "Size" ) );

                bOk = true;
                if ( !sPropMediaType.equals( sMediaType ) )
                {
                    Error( "'MediaType' property contains wrong value for stream '" + sName + "',\nexpected: '"
                            + sMediaType + "', set: '" + sPropMediaType + "'!" );
                    bOk = false;
                }

                if ( nPropSize != pBytes.length )
                {
                    Error( "'Size' property contains wrong value for stream'" + sName + "'!" );
                    bOk = false;
                }
            }
            catch( Exception e )
            {
                Error( "Can't get properties of substream '" + sName + "', exception: " + e );
            }
        }
        else
        {
            Error( "Can't get XPropertySet implementation from stream '" + sName + "'!" );
        }

        return bOk;
    }

    public boolean checkStream( XStorage xParentStorage,
                                String sName,
                                String sMediaType,
                                byte[] pBytes )
    {
        // open substream element first
        XStream xSubStream = null;
        try
        {
            Object oSubStream = xParentStorage.openStreamElement( sName, ElementModes.READ );
            xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            if ( xSubStream == null )
            {
                Error( "Can't open substream '" + sName + "'!" );
                return false;
            }
        }
        catch( Exception e )
        {
            Error( "Can't open substream '" + sName + "', exception : " + e + "!" );
            return false;
        }

        return InternalCheckStream( xSubStream, sName, sMediaType, pBytes );
    }

    public boolean checkEncrStream( XStorage xParentStorage,
                                    String sName,
                                    String sMediaType,
                                    byte[] pBytes,
                                    byte[] pPass )
    {
        // Important: a common password for any of parent storage should not be set or
        //            should be different from pPass

        if ( pPass.length == 0 )
        {
            Error( "Wrong password is used in the test!" );
            return false;
        }

        try
        {
            Object oSubStream = xParentStorage.openStreamElement( sName, ElementModes.READ );
            Error( "Encrypted stream '" + sName + "' was opened without password!" );
            return false;
        }
        catch( WrongPasswordException wpe )
        {}
        catch( Exception e )
        {
            Error( "Unexpected exception in case of opening of encrypted stream '" + sName + "' without password: " + e + "!" );
            return false;
        }

        byte pWrongPass[] = { 1, 1 };
        pWrongPass[0] += pPass[0];
        try
        {
            Object oSubStream = xParentStorage.openEncryptedStreamElement( sName, ElementModes.READ, new String(pWrongPass) );
            Error( "Encrypted stream '" + sName + "' was opened with wrong password!" );
            return false;
        }
        catch( WrongPasswordException wpe )
        {}
        catch( Exception e )
        {
            Error( "Unexpected exception in case of opening of encrypted stream '" + sName + "' with wrong password: " + e + "!" );
            return false;
        }

        XStream xSubStream = null;
        try
        {
            Object oSubStream = xParentStorage.openEncryptedStreamElement( sName, ElementModes.READ, new String(pPass) );
            xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            if ( xSubStream == null )
            {
                Error( "Can't open encrypted substream '" + sName + "'!" );
                return false;
            }
        }
        catch( Exception e )
        {
            Error( "Can't open encrypted substream '" + sName + "', exception : " + e + "!" );
            return false;
        }

        return InternalCheckStream( xSubStream, sName, sMediaType, pBytes );
    }

    public boolean copyStorage( XStorage xSourceStorage, XStorage xDestStorage )
    {
        // copy xSourceStorage to xDestStorage
        try
        {
            xSourceStorage.copyToStorage( xDestStorage );
        }
        catch( Exception e )
        {
            Error( "Storage copying failed, exception: " + e );
            return false;
        }

        return true;
    }

    public boolean commitStorage( XStorage xStorage )
    {
        // XTransactedObject must be supported by storages
        XTransactedObject xTransact = (XTransactedObject) UnoRuntime.queryInterface( XTransactedObject.class, xStorage );
        if ( xTransact == null )
        {
            Error( "Storage doesn't implement transacted access!" );
            return false;
        }

        try
        {
            xTransact.commit();
        }
        catch( Exception e )
        {
            Error( "Storage commit failed, exception:" + e );
            return false;
        }

        return true;
    }

    public boolean disposeStorage( XStorage xStorage )
    {
        // dispose the storage
        XComponent xComponent = (XComponent) UnoRuntime.queryInterface( XComponent.class, xStorage );
        if ( xComponent == null )
        {
            Error( "Can't retrieve XComponent implementation from storage!" );
            return false;
        }

        try
        {
            xComponent.dispose();
        }
        catch( Exception e )
        {
            Error( "Storage disposing failed!" );
            return false;
        }

        return true;
    }

    public XInputStream getInputStream( XStream xStream )
    {
        XInputStream xInTemp = null;
        try
        {
            xInTemp = xStream.getInputStream();
            if ( xInTemp == null )
                Error( "Can't get the input part of a stream!" );
        }
        catch ( Exception e )
        {
            Error( "Can't get the input part of a stream, exception :" + e );
        }

        return xInTemp;
    }

    public boolean closeOutput( XStream xStream )
    {
        XOutputStream xOutTemp = null;
        try
        {
            xOutTemp = xStream.getOutputStream();
            if ( xOutTemp == null )
            {
                Error( "Can't get the output part of a stream!" );
                return false;
            }
        }
        catch ( Exception e )
        {
            Error( "Can't get the output part of a stream, exception :" + e );
            return false;
        }

        try
        {
            xOutTemp.closeOutput();
        }
        catch ( Exception e )
        {
            Error( "Can't close output part of a stream, exception :" + e );
            return false;
        }

        return true;
    }

    public XStorage openSubStorage( XStorage xStorage, String sName, int nMode )
    {
        // open existing substorage
        try
        {
            Object oSubStorage = xStorage.openStorageElement( sName, nMode );
            XStorage xSubStorage = (XStorage) UnoRuntime.queryInterface( XStorage.class, oSubStorage );
            return xSubStorage;
        }
        catch( Exception e )
        {
            Error( "Can't open substorage '" + sName + "', exception: " + e );
        }

        return null;
    }

    public XStream CreateTempFileStream( XMultiServiceFactory xMSF )
    {
        // try to get temporary file representation
        XStream xTempFileStream = null;
        try
        {
            Object oTempFile = xMSF.createInstance( "com.sun.star.io.TempFile" );
            xTempFileStream = (XStream)UnoRuntime.queryInterface( XStream.class, oTempFile );
        }
        catch( Exception e )
        {}

        if ( xTempFileStream == null )
            Error( "Can't create temporary file!" );

        return xTempFileStream;
    }

    public String CreateTempFile( XMultiServiceFactory xMSF )
    {
        String sResult = null;

        // try to get temporary file representation
        XPropertySet xTempFileProps = null;
        try
        {
            Object oTempFile = xMSF.createInstance( "com.sun.star.io.TempFile" );
            xTempFileProps = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, oTempFile );
        }
        catch( Exception e )
        {}

        if ( xTempFileProps != null )
        {
            try
            {
                xTempFileProps.setPropertyValue( "RemoveFile", new Boolean( false ) );
                sResult = AnyConverter.toString( xTempFileProps.getPropertyValue( "Uri" ) );
            }
            catch( Exception e )
            {
                Error( "Can't control TempFile properties, exception: " + e );
            }
        }
        else
        {
            Error( "Can't create temporary file representation!" );
        }

        // close temporary file explicitly
        try
        {
            XStream xStream = (XStream)UnoRuntime.queryInterface( XStream.class, xTempFileProps );
            if ( xStream != null )
            {
                XOutputStream xOut = xStream.getOutputStream();
                if ( xOut != null )
                    xOut.closeOutput();

                XInputStream xIn = xStream.getInputStream();
                if ( xIn != null )
                    xIn.closeInput();
            }
            else
                Error( "Can't close TempFile!" );
        }
        catch( Exception e )
        {
            Error( "Can't close TempFile, exception: " + e );
        }

        return sResult;
    }

    public boolean copyElementTo( XStorage xSource, String sName, XStorage xDest )
    {
        // copy element with name sName from xSource to xDest
        try
        {
            xSource.copyElementTo( sName, xDest, sName );
        }
        catch( Exception e )
        {
            Error( "Element copying failed, exception: " + e );
            return false;
        }

        return true;
    }

    public boolean moveElementTo( XStorage xSource, String sName, XStorage xDest )
    {
        // move element with name sName from xSource to xDest
        try
        {
            xSource.moveElementTo( sName, xDest, sName );
        }
        catch( Exception e )
        {
            Error( "Element moving failed, exception: " + e );
            return false;
        }

        return true;
    }

    public boolean renameElement( XStorage xStorage, String sOldName, String sNewName )
    {
        // rename element with name sOldName to sNewName
        try
        {
            xStorage.renameElement( sOldName, sNewName );
        }
        catch( Exception e )
        {
            Error( "Element renaming failed, exception: " + e );
            return false;
        }

        return true;
    }

    public boolean removeElement( XStorage xStorage, String sName )
    {
        // remove element with name sName
        try
        {
            xStorage.removeElement( sName );
        }
        catch( Exception e )
        {
            Error( "Element removing failed, exception: " + e );
            return false;
        }

        return true;
    }

    public XStream OpenStream( XStorage xStorage,
                                String sStreamName,
                                int nMode )
    {
        // open substream element
        XStream xSubStream = null;
        try
        {
            Object oSubStream = xStorage.openStreamElement( sStreamName, nMode );
            xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            if ( xSubStream == null )
                Error( "Can't create substream '" + sStreamName + "'!" );
        }
        catch( Exception e )
        {
            Error( "Can't create substream '" + sStreamName + "', exception : " + e + "!" );
        }

        return xSubStream;
    }

    public boolean cantOpenStorage( XStorage xStorage, String sName )
    {
        // try to open an opened substorage, open call must fail
        try
        {
            Object oDummyStorage = xStorage.openStorageElement( sName, ElementModes.READ );
            Error( "The trying to reopen opened substorage '" + sName + "' must fail!" );
        }
        catch( Exception e )
        {
            return true;
        }

        return false;
    }

    public boolean cantOpenStream( XStorage xStorage, String sName, int nMode )
    {
        // try to open the substream with specified mode must fail
        try
        {
            Object oDummyStream = xStorage.openStreamElement( sName, nMode );
            Error( "The trying to open substoream '" + sName + "' must fail!" );
        }
        catch( Exception e )
        {
            return true;
        }

        return false;
    }

    public void Error( String sError )
    {
        System.out.println( m_sTestPrefix + "Error: " + sError );
    }

    public void Message( String sError )
    {
        System.out.println( m_sTestPrefix + sError );
    }
}

