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
                                        byte[] pBytes,
                                        StringPair[][] aRelations )
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

        // get access to the relationship information
        XRelationshipAccess xRelAccess = (XRelationshipAccess) UnoRuntime.queryInterface( XRelationshipAccess.class, xStream );
        if ( xRelAccess == null )
        {
            Error( "Can't get XRelationshipAccess implementation from substream '" + sStreamName + "'!" );
            return false;
        }

        // set the relationship information
        try
        {
            xRelAccess.insertRelationships( aRelations, false );
        }
        catch( Exception e )
        {
            Error( "Can't set relationships to substream '" + sStreamName + "', exception: " + e );
            return false;
        }

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xStream, sStreamName ) )
            return false;

        return true;
    }

    public boolean WriteBytesToSubstream( XStorage xStorage,
                                          String sStreamName,
                                          String sMediaType,
                                          boolean bCompressed,
                                          byte[] pBytes,
                                          StringPair[][] aRelations )
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

        return WriteBytesToStream( xSubStream, sStreamName, sMediaType, bCompressed, pBytes, aRelations );
    }

    public boolean setStorageTypeAndCheckProps( XStorage xStorage,
                                                boolean bIsRoot,
                                                int nMode,
                                                StringPair[][] aRelations )
    {
        boolean bOk = false;

        // get access to the XPropertySet interface
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, xStorage );
        if ( xPropSet != null )
        {
            try
            {
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

        // get access to the relationship information
        XRelationshipAccess xRelAccess = (XRelationshipAccess) UnoRuntime.queryInterface( XRelationshipAccess.class, xStorage );

        if ( xRelAccess == null )
        {
            Error( "Can't get XRelationshipAccess implementation from the storage!" );
            return false;
        }

        // set the relationship information
        try
        {
            xRelAccess.insertRelationships( aRelations, false );
        }
        catch( Exception e )
        {
            Error( "Can't set relationships to the storage, exception: " + e );
            return false;
        }


        return bOk;
    }

    public boolean checkRelations( StringPair[][] aStorRels, StringPair[][] aTestRels )
    {
        // Message( "StorageRels:" );
        // PrintRelations( aStorRels );
        // Message( "TestRels:" );
        // PrintRelations( aTestRels );

        if ( aStorRels.length != aTestRels.length )
        {
            Error( "The provided relations sequence has different size than the storage's one!" );
            return false;
        }

        for ( int nStorInd = 0; nStorInd < aStorRels.length; nStorInd++ )
        {
            int nStorIDInd = -1;
            for ( int nStorTagInd = 0; nStorTagInd < aStorRels[nStorInd].length; nStorTagInd++ )
            {
                if ( aStorRels[nStorInd][nStorTagInd].First.equals( "Id" ) )
                {
                    nStorIDInd = nStorTagInd;
                    break;
                }
            }

            if ( nStorIDInd == -1 )
            {
                Error( "One of the storage relations entries has no ID!" );
                return false;
            }

            for ( int nInd = 0; nInd < aTestRels.length; nInd++ )
            {
                int nIDInd = -1;
                for ( int nTagInd = 0; nTagInd < aTestRels[nInd].length; nTagInd++ )
                {
                    if ( aTestRels[nInd][nTagInd].First.equals( "Id" ) )
                    {
                        nIDInd = nTagInd;
                        break;
                    }
                }

                if ( nIDInd == -1 )
                {
                    Error( "One of the test hardcoded entries has no ID, num = " + nInd + ", length = " + aTestRels[nInd].length + ", global length = " + aTestRels.length + "!" );
                    return false;
                }

                if ( aStorRels[nStorInd][nStorIDInd].Second.equals( aTestRels[nInd][nIDInd].Second ) )
                {
                    boolean[] pRelCheckMark = new boolean[ aTestRels[nInd].length ];
                    for ( int nCheckInd = 0; nCheckInd < pRelCheckMark.length; nCheckInd++ )
                    {
                        pRelCheckMark[nCheckInd] = false;
                    }

                    for ( int nStorTagInd = 0; nStorTagInd < aStorRels[nStorInd].length; nStorTagInd++ )
                    {
                        boolean bFound = false;
                        for ( int nTagInd = 0; nTagInd < aTestRels[nInd].length; nTagInd++ )
                        {
                            if ( aTestRels[nInd][nTagInd].First.equals( aStorRels[nStorInd][nStorTagInd].First ) )
                            {
                                if ( !aTestRels[nInd][nTagInd].Second.equals( aStorRels[nStorInd][nStorTagInd].Second ) )
                                {
                                    Error( "Test rel. num. " + nInd + " has different tag \"" + aTestRels[nInd][nTagInd].First + "\" value!" );
                                    return false;
                                }

                                bFound = true;
                                pRelCheckMark[nTagInd] = true;
                                break;
                            }
                        }

                        if ( !bFound )
                        {
                            Error( "Stor rel. num. " + nStorInd + " has unexpected tag \"" + aStorRels[nStorInd][nStorTagInd].First + "\", ID = \"" + aStorRels[nStorInd][nStorIDInd].Second + "\"!" );
                            return false;
                        }
                    }

                    for ( int nCheckInd = 0; nCheckInd < pRelCheckMark.length; nCheckInd++ )
                    {
                        if ( !pRelCheckMark[nCheckInd] && !aTestRels[nInd][nCheckInd].Second.equals( "" ) )
                        {
                            Error( "Test rel. num. " + nInd + " has unexpected tag \"" + aTestRels[nInd][nCheckInd].First + "\" with nonempty value!" );
                            return false;
                        }
                    }

                    break;
                }
            }
        }

        return true;
    }

    public boolean checkStorageProperties( XStorage xStorage,
                                            boolean bIsRoot,
                                            int nMode,
                                            StringPair[][] aRelations )
    {
        boolean bOk = false;

        // get access to the XPropertySet interface
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, xStorage );
        if ( xPropSet != null )
        {
            try
            {
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
                Error( "Can't get properties of substorage, exception: " + e );
            }
        }
        else
        {
            Error( "Can't get XPropertySet implementation from storage!" );
        }

        // get access to the relationship information
        XRelationshipAccess xRelAccess = (XRelationshipAccess) UnoRuntime.queryInterface( XRelationshipAccess.class, xStorage );

        if ( xRelAccess == null )
        {
            Error( "Can't get XRelationshipAccess implementation from the checked storage!" );
            return false;
        }

        // get the relationship information
        StringPair[][] aStorRels;
        try
        {
            aStorRels = xRelAccess.getAllRelationships();
        }
        catch( Exception e )
        {
            Error( "Can't get relationships of the checked storage, exception: " + e );
            return false;
        }

        if ( !checkRelations( aStorRels, aRelations ) )
        {
            Error( "StorageRelationsChecking has failed!" );
            return false;
        }

        return bOk;
    }

    public boolean InternalCheckStream( XStream xStream,
                                        String sName,
                                        String sMediaType,
                                        byte[] pBytes,
                                        StringPair[][] aRelations )
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
                        + ind + " should be" + pBytes[ind] + " but it is " + pContents[0][ind] + ")" );
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


        // get access to the relationship information
        XRelationshipAccess xRelAccess = (XRelationshipAccess) UnoRuntime.queryInterface( XRelationshipAccess.class, xStream );

        if ( xRelAccess == null )
        {
            Error( "Can't get XRelationshipAccess implementation from the stream\"" + sName + "\"!" );
            return false;
        }

        // get the relationship information
        StringPair[][] aStorRels;
        try
        {
            aStorRels = xRelAccess.getAllRelationships();
        }
        catch( Exception e )
        {
            Error( "Can't get relationships of the substream '" + sName + "', exception: " + e );
            return false;
        }

        if ( !checkRelations( aStorRels, aRelations ) )
        {
            Error( "Stream '" + sName + "' RelationsChecking has failed!" );
            return false;
        }

        return bOk;
    }

    public boolean checkStream( XStorage xParentStorage,
                                String sName,
                                String sMediaType,
                                byte[] pBytes,
                                StringPair[][] aRelations )
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

        boolean bResult = InternalCheckStream( xSubStream, sName, sMediaType, pBytes, aRelations );

        // free the stream resources, garbage collector may remove the object too late
        if ( !disposeStream( xSubStream, sName ) )
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

    public XStorage createStorageFromURL(
                                XSingleServiceFactory xFactory,
                                String aURL,
                                int nMode )
    {
        XStorage xResult = null;

        try
        {
            PropertyValue[] aAddArgs = new PropertyValue[1];
            aAddArgs[0] = new PropertyValue();
            aAddArgs[0].Name = "StorageFormat";
            aAddArgs[0].Value = "OFOPXMLFormat";

            Object pArgs[] = new Object[3];
            pArgs[0] = (Object) aURL;
            pArgs[1] = new Integer( nMode );
            pArgs[2] = (Object) aAddArgs;

            Object oTempStorage = xFactory.createInstanceWithArguments( pArgs );
            xResult = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
        }
        catch( Exception e )
        {
            Error( "Can't create storage from URL, exception: " + e );
            return null;
        }

        if ( xResult == null )
            Error( "Can't create storage from URL!" );

        return xResult;
    }

    public XStorage createStorageFromStream(
                                XSingleServiceFactory xFactory,
                                XStream xStream,
                                int nMode )
    {
        XStorage xResult = null;

        try
        {
            PropertyValue[] aAddArgs = new PropertyValue[1];
            aAddArgs[0] = new PropertyValue();
            aAddArgs[0].Name = "StorageFormat";
            aAddArgs[0].Value = "OFOPXMLFormat";

            Object pArgs[] = new Object[3];
            pArgs[0] = (Object) xStream;
            pArgs[1] = new Integer( nMode );
            pArgs[2] = (Object) aAddArgs;

            Object oTempStorage = xFactory.createInstanceWithArguments( pArgs );
            xResult = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
        }
        catch( Exception e )
        {
            Error( "Can't create storage from stream, exception: " + e );
            return null;
        }

        if ( xResult == null )
            Error( "Can't create storage from stream!" );

        return xResult;
    }

    public XStorage createStorageFromInputStream(
                                XSingleServiceFactory xFactory,
                                XInputStream xInStream )
    {
        XStorage xResult = null;

        try
        {
            PropertyValue[] aAddArgs = new PropertyValue[1];
            aAddArgs[0] = new PropertyValue();
            aAddArgs[0].Name = "StorageFormat";
            aAddArgs[0].Value = "OFOPXMLFormat";

            Object pArgs[] = new Object[3];
            pArgs[0] = (Object) xInStream;
            pArgs[1] = new Integer( ElementModes.READ );
            pArgs[2] = (Object) aAddArgs;

            Object oTempStorage = xFactory.createInstanceWithArguments( pArgs );
            xResult = (XStorage) UnoRuntime.queryInterface( XStorage.class, oTempStorage );
        }
        catch( Exception e )
        {
            Error( "Can't create storage from input stream, exception: " + e );
            return null;
        }

        if ( xResult == null )
            Error( "Can't create storage from input stream!" );

        return xResult;
    }

    public XStorage createTempStorage( XMultiServiceFactory xMSF, XSingleServiceFactory xFactory )
    {
        // create a temporary storage
        XStorage xResult = null;
        XStream xStream = CreateTempFileStream( xMSF );
        if ( xStream == null )
        {
            Error( "Can't create temp file stream!" );
            return null;
        }

        try
        {
            xResult = createStorageFromStream( xFactory, xStream, ElementModes.WRITE );
        }
        catch( Exception e )
        {
            Error( "Can't create temp storage, exception: " + e );
        }

        return xResult;
    }

    public XStorage cloneStorage( XMultiServiceFactory xMSF, XSingleServiceFactory xFactory, XStorage xStorage )
    {
        // create a copy of a last commited version of specified storage
        XStorage xResult = null;
        try
        {
            xResult = createTempStorage( xMSF, xFactory );
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

    public XStorage cloneSubStorage( XMultiServiceFactory xMSF, XSingleServiceFactory xFactory, XStorage xStorage, String sName )
    {
        // create a copy of a last commited version of specified substorage
        XStorage xResult = null;
        try
        {
            xResult = createTempStorage( xMSF, xFactory );
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

    public void Error( String sError )
    {
        m_aLogWriter.println( m_sTestPrefix + "Error: " + sError );
    }

    public void Message( String sMessage )
    {
        m_aLogWriter.println( m_sTestPrefix + sMessage );
    }

    public void PrintRelations( StringPair[][] aRels )
    {
        m_aLogWriter.println( "========" );
        for ( int nInd1 = 0; nInd1 < aRels.length; nInd1++ )
        {
            for ( int nInd2 = 0; nInd2 < aRels[nInd1].length; nInd2++ )
            {
                m_aLogWriter.println( "\"" + aRels[nInd1][nInd2].First + "\" = \"" + aRels[nInd1][nInd2].Second + "\", " );
            }
            m_aLogWriter.println( "========" );
        }
    }
}

