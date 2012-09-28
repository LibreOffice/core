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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.AnyConverter;

import com.sun.star.lang.*;
import com.sun.star.embed.*;
import com.sun.star.packages.*;
import com.sun.star.io.*;
import com.sun.star.beans.*;

import share.LogWriter;

public class TestHelper  {

    LogWriter m_aLogWriter;
    String m_sTestPrefix;

    public TestHelper( LogWriter aLogWriter, String sTestPrefix )
    {
        m_aLogWriter = aLogWriter;
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
            long nSize = AnyConverter.toLong( xPropSet.getPropertyValue( "Size" ) );
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

        return true;
    }

    public boolean WriteBytesToSubstreamDefaultCompressed( XStorage xStorage,
                                                            String sStreamName,
                                                            String sMediaType,
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

        // get output stream of substream
        XOutputStream xOutput = xSubStream.getOutputStream();
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
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, xSubStream );
        if ( xPropSet == null )
        {
            Error( "Can't get XPropertySet implementation from substream '" + sStreamName + "'!" );
            return false;
        }

        // set properties to the stream
        // do not set the compressed property
        try
        {
            xPropSet.setPropertyValue( "MediaType", sMediaType );
        }
        catch( Exception e )
        {
            Error( "Can't set properties to substream '" + sStreamName + "', exception: " + e );
            return false;
        }

        // check size property of the stream
        try
        {
            long nSize = AnyConverter.toLong( xPropSet.getPropertyValue( "Size" ) );
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
        if ( !disposeStream( xSubStream, sStreamName ) )
            return false;

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

        if ( !WriteBytesToStream( xSubStream, sStreamName, sMediaType, bCompressed, pBytes ) )
            return false;

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xSubStream, sStreamName ) )
            return false;

        return true;
    }

    public boolean WriteBytesToEncrSubstream( XStorage xStorage,
                                              String sStreamName,
                                              String sMediaType,
                                              boolean bCompressed,
                                              byte[] pBytes,
                                              String sPass )
    {
        // open substream element
        XStream xSubStream = null;
        try
        {
            Object oSubStream = xStorage.openEncryptedStreamElement( sStreamName, ElementModes.WRITE, sPass );
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

        if ( !WriteBytesToStream( xSubStream, sStreamName, sMediaType, bCompressed, pBytes ) )
            return false;

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xSubStream, sStreamName ) )
            return false;

        return true;
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
            xPropSet.setPropertyValue( "UseCommonStoragePasswordEncryption", new Boolean( bEncrypted ) );
        }
        catch( Exception e )
        {
            Error( "Can't set 'UseCommonStoragePasswordEncryption' property to substream '" + sStreamName + "', exception: " + e );
            return false;
        }

        if ( !WriteBytesToStream( xSubStream, sStreamName, sMediaType, bCompressed, pBytes ) )
            return false;

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xSubStream, sStreamName ) )
            return false;

        return true;
    }

    public boolean WriteBytesToStreamH( XStorage xStorage,
                                          String sStreamPath,
                                          String sMediaType,
                                          boolean bCompressed,
                                          byte[] pBytes,
                                          boolean bCommit )
    {
        // open substream element
        XStream xSubStream = null;
        try
        {
            XHierarchicalStorageAccess xHStorage =
                (XHierarchicalStorageAccess) UnoRuntime.queryInterface( XHierarchicalStorageAccess.class, xStorage );
            if ( xHStorage == null )
            {
                Error( "The storage does not support hierarchical access!" );
                return false;
            }

            Object oSubStream = xHStorage.openStreamElementByHierarchicalName( sStreamPath, ElementModes.WRITE );
            xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            if ( xSubStream == null )
            {
                Error( "Can't create substream '" + sStreamPath + "'!" );
                return false;
            }
        }
        catch( Exception e )
        {
            Error( "Can't create substream '" + sStreamPath + "', exception : " + e + "!" );
            return false;
        }

        if ( !WriteBytesToStream( xSubStream, sStreamPath, sMediaType, bCompressed, pBytes ) )
            return false;

        XTransactedObject xTransact =
            (XTransactedObject) UnoRuntime.queryInterface( XTransactedObject.class, xSubStream );
        if ( xTransact == null )
        {
            Error( "Substream '" + sStreamPath + "', stream opened for writing must be transacted!" );
            return false;
        }

        if ( bCommit )
        {
            try {
                xTransact.commit();
            } catch( Exception e )
            {
                Error( "Can't commit storage after substream '" + sStreamPath + "' change, exception : " + e + "!" );
                return false;
            }
        }

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xSubStream, sStreamPath ) )
            return false;

        return true;
    }

    public boolean WriteBytesToEncrStreamH( XStorage xStorage,
                                          String sStreamPath,
                                          String sMediaType,
                                          boolean bCompressed,
                                          byte[] pBytes,
                                          String sPass,
                                          boolean bCommit )
    {
        // open substream element
        XStream xSubStream = null;
        try
        {
            XHierarchicalStorageAccess xHStorage =
                (XHierarchicalStorageAccess) UnoRuntime.queryInterface( XHierarchicalStorageAccess.class, xStorage );
            if ( xHStorage == null )
            {
                Error( "The storage does not support hierarchical access!" );
                return false;
            }

            Object oSubStream = xHStorage.openEncryptedStreamElementByHierarchicalName( sStreamPath,
                                                                                        ElementModes.WRITE,
                                                                                        sPass );
            xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            if ( xSubStream == null )
            {
                Error( "Can't create substream '" + sStreamPath + "'!" );
                return false;
            }
        }
        catch( Exception e )
        {
            Error( "Can't create substream '" + sStreamPath + "', exception : " + e + "!" );
            return false;
        }

        if ( !WriteBytesToStream( xSubStream, sStreamPath, sMediaType, bCompressed, pBytes ) )
            return false;

        XTransactedObject xTransact =
            (XTransactedObject) UnoRuntime.queryInterface( XTransactedObject.class, xSubStream );
        if ( xTransact == null )
        {
            Error( "Substream '" + sStreamPath + "', stream opened for writing must be transacted!" );
            return false;
        }

        if ( bCommit )
        {
            try {
                xTransact.commit();
            } catch( Exception e )
            {
                Error( "Can't commit storage after substream '" + sStreamPath + "' change, exception : " + e + "!" );
                return false;
            }
        }

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xSubStream, sStreamPath ) )
            return false;

        return true;
    }

    public boolean WBToSubstrOfEncrH( XStorage xStorage,
                                      String sStreamPath,
                                      String sMediaType,
                                      boolean bCompressed,
                                      byte[] pBytes,
                                      boolean bEncrypted,
                                      boolean bCommit )
    {
        // open substream element
        XStream xSubStream = null;
        try
        {
            XHierarchicalStorageAccess xHStorage =
                (XHierarchicalStorageAccess) UnoRuntime.queryInterface( XHierarchicalStorageAccess.class, xStorage );
            if ( xHStorage == null )
            {
                Error( "The storage does not support hierarchical access!" );
                return false;
            }

            Object oSubStream = xHStorage.openStreamElementByHierarchicalName( sStreamPath, ElementModes.WRITE );
            xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            if ( xSubStream == null )
            {
                Error( "Can't create substream '" + sStreamPath + "'!" );
                return false;
            }
        }
        catch( Exception e )
        {
            Error( "Can't create substream '" + sStreamPath + "', exception : " + e + "!" );
            return false;
        }

        // get access to the XPropertySet interface
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, xSubStream );
        if ( xPropSet == null )
        {
            Error( "Can't get XPropertySet implementation from substream '" + sStreamPath + "'!" );
            return false;
        }

        // set properties to the stream
        try
        {
            xPropSet.setPropertyValue( "UseCommonStoragePasswordEncryption", new Boolean( bEncrypted ) );
        }
        catch( Exception e )
        {
            Error( "Can't set 'UseCommonStoragePasswordEncryption' property to substream '" + sStreamPath + "', exception: " + e );
            return false;
        }

        if ( !WriteBytesToStream( xSubStream, sStreamPath, sMediaType, bCompressed, pBytes ) )
            return false;

        XTransactedObject xTransact =
            (XTransactedObject) UnoRuntime.queryInterface( XTransactedObject.class, xSubStream );
        if ( xTransact == null )
        {
            Error( "Substream '" + sStreamPath + "', stream opened for writing must be transacted!" );
            return false;
        }

        if ( bCommit )
        {
            try {
                xTransact.commit();
            } catch( Exception e )
            {
                Error( "Can't commit storage after substream '" + sStreamPath + "' change, exception : " + e + "!" );
                return false;
            }
        }

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xSubStream, sStreamPath ) )
            return false;

        return true;
    }

    public int ChangeStreamPass( XStorage xStorage,
                                 String sStreamName,
                                 String sOldPass,
                                 String sNewPass )
    {
        // open substream element
        XStream xSubStream = null;
        try
        {
            Object oSubStream = xStorage.openEncryptedStreamElement( sStreamName, ElementModes.WRITE, sOldPass );
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
            xStreamEncryption.setEncryptionPassword( sNewPass );
        }
        catch( Exception e )
        {
            Error( "Can't change encryption key of the substream '" + sStreamName + "', exception:" + e );
            return 0;
        }

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xSubStream, sStreamName ) )
            return 0;

        return 1;
    }

    public int ChangeStreamPassH( XStorage xStorage,
                                 String sPath,
                                 String sOldPass,
                                 String sNewPass,
                                 boolean bCommit )
    {
        // open substream element
        XHierarchicalStorageAccess xHStorage =
            (XHierarchicalStorageAccess) UnoRuntime.queryInterface( XHierarchicalStorageAccess.class, xStorage );
        if ( xHStorage == null )
        {
            Error( "The storage does not support hierarchical access!" );
            return 0;
        }

        XStream xSubStream = null;
        try
        {
            Object oSubStream = xHStorage.openEncryptedStreamElementByHierarchicalName( sPath, ElementModes.WRITE, sOldPass );
            xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            if ( xSubStream == null )
            {
                Error( "Can't open encrypted substream '" + sPath + "'!" );
                return 0;
            }
        }
        catch( Exception e )
        {
            Error( "Can't open encrypted substream '" + sPath + "', exception : " + e + "!" );
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
            xStreamEncryption.setEncryptionPassword( sNewPass );
        }
        catch( Exception e )
        {
            Error( "Can't change encryption key of the substream '" + sPath + "', exception:" + e );
            return 0;
        }

        XTransactedObject xTransact =
            (XTransactedObject) UnoRuntime.queryInterface( XTransactedObject.class, xSubStream );
        if ( xTransact == null )
        {
            Error( "Substream '" + sPath + "', stream opened for writing must be transacted!" );
            return 0;
        }

        if ( bCommit )
        {
            try {
                xTransact.commit();
            } catch( Exception e )
            {
                Error( "Can't commit storage after substream '" + sPath + "' change, exception : " + e + "!" );
                return 0;
            }
        }

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xSubStream, sPath ) )
            return 0;

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

                if ( ( bIsRoot
                  && ( nPropMode | ElementModes.READ ) != ( nMode | ElementModes.READ ) )
                  || ( !bIsRoot && ( nPropMode & nMode ) != nMode ) )
                {
                    Error( "'OpenMode' property contains wrong value, expected " + nMode + ", in reality " + nPropMode + "!" );
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

                if ( ( bIsRoot
                  && ( nPropMode | ElementModes.READ ) != ( nMode | ElementModes.READ ) )
                  || ( !bIsRoot && ( nPropMode & nMode ) != nMode ) )
                {
                    Error( "'OpenMode' property contains wrong value, expected " + nMode + ", in reality " + nPropMode + "!" );
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
                                        boolean bCompressed,
                                        byte[] pBytes,
                                        boolean bCheckCompressed )
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
                Error( "SubStream '" + sName + "' contains wrong data! ( byte num. "
                        + ind + " should be " + pBytes[ind] + " but it is " + pContents[0][ind] + ")" );
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
                long nPropSize = AnyConverter.toLong( xPropSet.getPropertyValue( "Size" ) );
                boolean bPropCompress = AnyConverter.toBoolean( xPropSet.getPropertyValue( "Compressed" ) );

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

                if ( bCheckCompressed && bPropCompress != bCompressed )
                {
                    Error( "'Compressed' property contains wrong value for stream'" + sName + "'!" );
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
                                boolean bCompressed,
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

        boolean bResult = InternalCheckStream( xSubStream, sName, sMediaType, bCompressed, pBytes, true );

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xSubStream, sName ) )
            return false;

        return bResult;
    }

    public boolean checkEncrStream( XStorage xParentStorage,
                                    String sName,
                                    String sMediaType,
                                    byte[] pBytes,
                                    String sPass )
    {
        // Important: a common password for any of parent storage should not be set or
        //            should be different from sPass

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

        String sWrongPass = "11";
        sWrongPass += sPass;
        try
        {
            Object oSubStream = xParentStorage.openEncryptedStreamElement( sName, ElementModes.READ, sWrongPass );
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
            Object oSubStream = xParentStorage.openEncryptedStreamElement( sName, ElementModes.READ, sPass );
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

        // encrypted streams will be compressed always, so after the storing this property is always true,
        // although before the storing it can be set to false ( it is not always clear whether a stream is encrypted
        // before the storing )
        boolean bResult = InternalCheckStream( xSubStream, sName, sMediaType, true, pBytes, false );

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xSubStream, sName ) )
            return false;

        return bResult;
    }

    public boolean checkStreamH( XStorage xParentStorage,
                                String sPath,
                                String sMediaType,
                                boolean bCompressed,
                                byte[] pBytes )
    {
        // open substream element first
        XStream xSubStream = null;
        try
        {
            XHierarchicalStorageAccess xHStorage =
                (XHierarchicalStorageAccess) UnoRuntime.queryInterface( XHierarchicalStorageAccess.class, xParentStorage );
            if ( xHStorage == null )
            {
                Error( "The storage does not support hierarchical access!" );
                return false;
            }

            Object oSubStream = xHStorage.openStreamElementByHierarchicalName( sPath, ElementModes.READ );
            xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            if ( xSubStream == null )
            {
                Error( "Can't open substream '" + sPath + "'!" );
                return false;
            }
        }
        catch( Exception e )
        {
            Error( "Can't open substream '" + sPath + "', exception : " + e + "!" );
            return false;
        }

        boolean bResult = InternalCheckStream( xSubStream, sPath, sMediaType, bCompressed, pBytes, true );

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xSubStream, sPath ) )
            return false;

        return bResult;
    }

    public boolean checkEncrStreamH( XStorage xParentStorage,
                                    String sPath,
                                    String sMediaType,
                                    byte[] pBytes,
                                    String sPass )
    {
        // Important: a common password for any of parent storage should not be set or
        //            should be different from sPass
        XHierarchicalStorageAccess xHStorage =
            (XHierarchicalStorageAccess) UnoRuntime.queryInterface( XHierarchicalStorageAccess.class, xParentStorage );
        if ( xHStorage == null )
        {
            Error( "The storage does not support hierarchical access!" );
            return false;
        }

        try
        {
            Object oSubStream = xHStorage.openStreamElementByHierarchicalName( sPath, ElementModes.READ );
            XStream xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            Error( "Encrypted substream '" + sPath + "' was opened without password!" );
            return false;
        }
        catch( WrongPasswordException wpe )
        {}
        catch( Exception e )
        {
            Error( "Unexpected exception in case of opening of encrypted stream '" + sPath + "' without password: " + e + "!" );
            return false;
        }

        String sWrongPass = "11";
        sWrongPass += sPass;
        try
        {
            Object oSubStream = xHStorage.openEncryptedStreamElementByHierarchicalName( sPath, ElementModes.READ, sWrongPass );
            XStream xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            Error( "Encrypted substream '" + sPath + "' was opened with wrong password!" );
            return false;
        }
        catch( WrongPasswordException wpe )
        {}
        catch( Exception e )
        {
            Error( "Unexpected exception in case of opening of encrypted stream '" + sPath + "' with wrong password: " + e + "!" );
            return false;
        }

        XStream xSubStream = null;
        try
        {
            Object oSubStream = xHStorage.openEncryptedStreamElementByHierarchicalName( sPath, ElementModes.READ, sPass );
            xSubStream = (XStream) UnoRuntime.queryInterface( XStream.class, oSubStream );
            if ( xSubStream == null )
            {
                Error( "Can't open encrypted substream '" + sPath + "'!" );
                return false;
            }
        }
        catch( Exception e )
        {
            Error( "Can't open encrypted substream '" + sPath + "', exception : " + e + "!" );
            return false;
        }

        // encrypted streams will be compressed always, so after the storing this property is always true,
        // although before the storing it can be set to false ( it is not always clear whether a stream is encrypted
        // before the storing )
        boolean bResult = InternalCheckStream( xSubStream, sPath, sMediaType, true, pBytes, false );

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xSubStream, sPath ) )
            return false;

        return bResult;
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

    public boolean disposeStream( XStream xStream, String sStreamName )
    {
        XComponent xComponent = (XComponent) UnoRuntime.queryInterface( XComponent.class, xStream );
        if ( xComponent == null )
        {
            Error( "Can't get XComponent implementation from substream '" + sStreamName + "'!" );
            return false;
        }

        try
        {
            xComponent.dispose();
        }
        catch( Exception e )
        {
            Error( "Substream '" + sStreamName + "' disposing throws exception: " + e );
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

    public boolean copyElementTo( XStorage xSource, String sName, XStorage xDest, String sTargetName )
    {
        // copy element with name sName from xSource to xDest
        try
        {
            xSource.copyElementTo( sName, xDest, sTargetName );
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

    public boolean compareRawMethodsOnEncrStream( XStorage xStorage, String sStreamName )
    {

        XStorageRawAccess xRawStorage;
        try
        {
            xRawStorage = (XStorageRawAccess) UnoRuntime.queryInterface( XStorageRawAccess.class, xStorage );
        }
        catch( Exception e )
        {
            Error( "Can't get raw access to the storage, exception : " + e + "!" );
            return false;
        }

        if ( xRawStorage == null )
        {
            Error( "Can't get raw access to the storage!" );
            return false;
        }

        XInputStream xHeadRawStream = null;
        try
        {
            xHeadRawStream = xRawStorage.getRawEncrStreamElement( sStreamName );
        }
        catch( Exception e )
        {
            Error( "Can't open encrypted stream '" + sStreamName + "' in raw mode with header, exception : " + e + "!" );
        }

        XInputStream xPlainRawStream = null;
        try
        {
            xPlainRawStream = xRawStorage.getPlainRawStreamElement( sStreamName );
        }
        catch( Exception e )
        {
            Error( "Can't open encrypted stream '" + sStreamName + "' in raw mode with header, exception : " + e + "!" );
        }

        if ( xHeadRawStream == null || xPlainRawStream == null )
        {
            Error( "Can't open encrypted stream '" + sStreamName + "' in raw modes!" );
            return false;
        }

        try
        {
            byte pData[][] = new byte[1][38];
            if ( xHeadRawStream.readBytes( pData, 38 ) != 38 )
            {
                Error( "Can't read header of encrypted stream '" + sStreamName + "' raw representations!" );
                return false;
            }

            if ( pData[0][0] != 0x4d || pData[0][1] != 0x4d || pData[0][2] != 0x02 || pData[0][3] != 0x05 )
            {
                Error( "No signature in the header of encrypted stream '" + sStreamName + "' raw representations!" );
                return false;
            }

            int nVariableHeaderLength =
                        ( pData[0][30] + pData[0][31] * 0x100 ) // salt length
                        + ( pData[0][32] + pData[0][33] * 0x100 ) // iv length
                        + ( pData[0][34] + pData[0][35] * 0x100 ) // digest length
                        + ( pData[0][36] + pData[0][37] * 0x100 ); // mediatype length

            xHeadRawStream.skipBytes( nVariableHeaderLength );

            byte pRawData1[][] = new byte[1][32000];
            byte pRawData2[][] = new byte[1][32000];
            int nRead1 = 0;
            int nRead2 = 0;

            do
            {
                nRead1 = xHeadRawStream.readBytes( pRawData1, 32000 );
                nRead2 = xPlainRawStream.readBytes( pRawData2, 32000 );

                if ( nRead1 != nRead2 )
                {
                    Error( "The encrypted stream '" + sStreamName + "' raw representations have different size! nRead1 - nRead2 = " + ( new Integer( nRead1 - nRead2 ) ).toString() );
                    return false;
                }

                for ( int nInd = 0; nInd < nRead1; nInd++ )
                    if ( pRawData1[0][nInd] != pRawData2[0][nInd] )
                    {
                        Error( "The encrypted stream '" + sStreamName + "' raw representations have different data!" );
                        return false;
                    }
            }
            while( nRead1 == 32000 );
        }
        catch ( Exception e )
        {
            Error( "Can't compare stream '" + sStreamName + "' raw representations, exception : " + e + "!" );
            return false;
        }

        return true;
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
            Error( "The trying to open substream '" + sName + "' must fail!" );
        }
        catch( Exception e )
        {
            return true;
        }

        return false;
    }

    public boolean cantOpenStreamH( XStorage xStorage, String sPath, int nMode )
    {
        // try to open the substream with specified mode must fail

        XHierarchicalStorageAccess xHStorage =
            (XHierarchicalStorageAccess) UnoRuntime.queryInterface( XHierarchicalStorageAccess.class, xStorage );
        if ( xHStorage == null )
        {
            Error( "The storage does not support hierarchical access!" );
            return false;
        }

        try
        {
            Object oDummyStream = xHStorage.openStreamElementByHierarchicalName( sPath, nMode );
            Error( "The trying to open substream '" + sPath + "' must fail!" );
        }
        catch( Exception e )
        {
            return true;
        }

        return false;
    }

    public boolean cantOpenEncrStreamH( XStorage xStorage, String sPath, int nMode, String aPass )
    {
        // try to open the substream with specified mode must fail

        XHierarchicalStorageAccess xHStorage =
            (XHierarchicalStorageAccess) UnoRuntime.queryInterface( XHierarchicalStorageAccess.class, xStorage );
        if ( xHStorage == null )
        {
            Error( "The storage does not support hierarchical access!" );
            return false;
        }

        try
        {
            Object oDummyStream = xHStorage.openEncryptedStreamElementByHierarchicalName( sPath, nMode, aPass );
            Error( "The trying to open substream '" + sPath + "' must fail!" );
        }
        catch( WrongPasswordException wpe )
        {
            Error( "The substream '" + sPath + "' must not exist!" );
            return false;
        }
        catch( Exception e )
        {
            return true;
        }

        return false;
    }

    public XStorage cloneStorage( XSingleServiceFactory xFactory, XStorage xStorage )
    {
        // create a copy of a last commited version of specified storage
        XStorage xResult = null;
        try
        {
            Object oTempStorage = xFactory.createInstance();
            xResult = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
            if ( xResult != null )
                xStorage.copyLastCommitTo( xResult );
        }
        catch( Exception e )
        {
            Error( "Can't clone storage, exception: " + e );
            return null;
        }

        return xResult;
    }

    public XStorage cloneSubStorage( XSingleServiceFactory xFactory, XStorage xStorage, String sName )
    {
        // create a copy of a last commited version of specified substorage
        XStorage xResult = null;
        try
        {
            Object oTempStorage = xFactory.createInstance();
            xResult = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
            if ( xResult != null )
                xStorage.copyStorageElementLastCommitTo( sName, xResult );
        }
        catch( Exception e )
        {
            Error( "Can't clone substorage '" + sName + "', exception: " + e );
            return null;
        }

        return xResult;
    }

    public XStream cloneSubStream( XStorage xStorage, String sName )
    {
        // clone existing substream
        try
        {
            XStream xStream = xStorage.cloneStreamElement( sName );
            return xStream;
        }
        catch( Exception e )
        {
            Error( "Can't clone substream '" + sName + "', exception: " + e );
        }

        return null;
    }

    public XStream cloneEncrSubStream( XStorage xStorage, String sName, String sPass )
    {
        // clone existing substream
        try
        {
            XStream xStream = xStorage.cloneEncryptedStreamElement( sName, sPass );
            return xStream;
        }
        catch( Exception e )
        {
            Error( "Can't clone encrypted substream '" + sName + "', exception: " + e );
        }

        return null;
    }

    public void Error( String sError )
    {
        m_aLogWriter.println( m_sTestPrefix + "Error: " + sError );
    }

    public void Message( String sMessage )
    {
        m_aLogWriter.println( m_sTestPrefix + sMessage );
    }
}

