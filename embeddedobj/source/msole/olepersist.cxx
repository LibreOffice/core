/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <oleembobj.hxx>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XOptimizedStorage.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/classids.hxx>
#include <osl/thread.hxx>

#include <olecomponent.hxx>
#include <closepreventer.hxx>

using namespace ::com::sun::star;
using namespace ::comphelper;

//-------------------------------------------------------------------------
sal_Bool KillFile_Impl( const OUString& aURL, const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    if ( !xFactory.is() )
        return sal_False;

    sal_Bool bRet = sal_False;

    try
    {
        uno::Reference < ucb::XSimpleFileAccess3 > xAccess(
                ucb::SimpleFileAccess::create( comphelper::getComponentContext(xFactory) ) );

        xAccess->kill( aURL );
        bRet = sal_True;
    }
    catch( const uno::Exception& )
    {
    }

    return bRet;
}

//----------------------------------------------
OUString GetNewTempFileURL_Impl( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    SAL_WARN_IF( !xFactory.is(), "embeddedobj.ole", "No factory is provided!" );

    OUString aResult;

    uno::Reference < beans::XPropertySet > xTempFile(
            io::TempFile::create(comphelper::getComponentContext(xFactory)),
            uno::UNO_QUERY_THROW );

    try {
        xTempFile->setPropertyValue("RemoveFile", uno::makeAny( sal_False ) );
        uno::Any aUrl = xTempFile->getPropertyValue("Uri");
        aUrl >>= aResult;
    }
    catch ( const uno::Exception& )
    {
    }

    if ( aResult.isEmpty() )
        throw uno::RuntimeException(); // TODO: can not create tempfile

    return aResult;
}

//-----------------------------------------------
OUString GetNewFilledTempFile_Impl( const uno::Reference< io::XInputStream >& xInStream,
                                      const uno::Reference< lang::XMultiServiceFactory >& xFactory )
        throw ( io::IOException,
                uno::RuntimeException )
{
    OSL_ENSURE( xInStream.is() && xFactory.is(), "Wrong parameters are provided!\n" );

    OUString aResult = GetNewTempFileURL_Impl( xFactory );

    if ( !aResult.isEmpty() )
    {
        try {
            uno::Reference < ucb::XSimpleFileAccess3 > xTempAccess(
                    ucb::SimpleFileAccess::create( comphelper::getComponentContext(xFactory) ) );

            uno::Reference< io::XOutputStream > xTempOutStream = xTempAccess->openFileWrite( aResult );
            if ( xTempOutStream.is() )
            {
                // copy stream contents to the file
                ::comphelper::OStorageHelper::CopyInputToOutput( xInStream, xTempOutStream );
                xTempOutStream->closeOutput();
                xTempOutStream = uno::Reference< io::XOutputStream >();
            }
            else
                throw io::IOException(); // TODO:
        }
        catch( const packages::WrongPasswordException& )
        {
               KillFile_Impl( aResult, xFactory );
            throw io::IOException(); //TODO:
        }
        catch( const io::IOException& )
        {
               KillFile_Impl( aResult, xFactory );
            throw;
        }
        catch( const uno::RuntimeException& )
        {
               KillFile_Impl( aResult, xFactory );
            throw;
        }
        catch( const uno::Exception& )
        {
               KillFile_Impl( aResult, xFactory );
            aResult = OUString();
        }
    }

    return aResult;
}
#ifdef WNT
OUString GetNewFilledTempFile_Impl( const uno::Reference< embed::XOptimizedStorage >& xParentStorage, const OUString& aEntryName, const uno::Reference< lang::XMultiServiceFactory >& xFactory )
    throw( io::IOException, uno::RuntimeException )
{
    OUString aResult;

    try
    {
        uno::Reference < beans::XPropertySet > xTempFile(
                io::TempFile::create(comphelper::getComponentContext(xFactory)),
                uno::UNO_QUERY );
        uno::Reference < io::XStream > xTempStream( xTempFile, uno::UNO_QUERY_THROW );

        xParentStorage->copyStreamElementData( aEntryName, xTempStream );

        xTempFile->setPropertyValue("RemoveFile", uno::makeAny( sal_False ) );
        uno::Any aUrl = xTempFile->getPropertyValue("Uri");
        aUrl >>= aResult;
    }
    catch( const uno::RuntimeException& )
    {
        throw;
    }
    catch( const uno::Exception& )
    {
    }

    if ( aResult.isEmpty() )
        throw io::IOException();

    return aResult;
}

//------------------------------------------------------
void SetStreamMediaType_Impl( const uno::Reference< io::XStream >& xStream, const OUString& aMediaType )
{
    uno::Reference< beans::XPropertySet > xPropSet( xStream, uno::UNO_QUERY );
    if ( !xPropSet.is() )
        throw uno::RuntimeException(); // TODO: all the storage streams must support XPropertySet

    xPropSet->setPropertyValue("MediaType", uno::makeAny( aMediaType ) );
}
#endif
//------------------------------------------------------
void LetCommonStoragePassBeUsed_Impl( const uno::Reference< io::XStream >& xStream )
{
    uno::Reference< beans::XPropertySet > xPropSet( xStream, uno::UNO_QUERY );
    if ( !xPropSet.is() )
        throw uno::RuntimeException(); // Only StorageStreams must be provided here, they must implement the interface

    xPropSet->setPropertyValue("UseCommonStoragePasswordEncryption",
                                uno::makeAny( (sal_Bool)sal_True ) );
}
#ifdef WNT
//------------------------------------------------------
void VerbExecutionController::StartControlExecution()
{
    osl::MutexGuard aGuard( m_aVerbExecutionMutex );

    // the class is used to detect STAMPIT object, that can never be active
    if ( !m_bVerbExecutionInProgress && !m_bWasEverActive )
    {
        m_bVerbExecutionInProgress = sal_True;
        m_nVerbExecutionThreadIdentifier = osl::Thread::getCurrentIdentifier();
        m_bChangedOnVerbExecution = sal_False;
    }
}

//------------------------------------------------------
sal_Bool VerbExecutionController::EndControlExecution_WasModified()
{
    osl::MutexGuard aGuard( m_aVerbExecutionMutex );

    sal_Bool bResult = sal_False;
    if ( m_bVerbExecutionInProgress && m_nVerbExecutionThreadIdentifier == osl::Thread::getCurrentIdentifier() )
    {
        bResult = m_bChangedOnVerbExecution;
        m_bVerbExecutionInProgress = sal_False;
    }

    return bResult;
}

//------------------------------------------------------
void VerbExecutionController::ModificationNotificationIsDone()
{
    osl::MutexGuard aGuard( m_aVerbExecutionMutex );

    if ( m_bVerbExecutionInProgress && osl::Thread::getCurrentIdentifier() == m_nVerbExecutionThreadIdentifier )
        m_bChangedOnVerbExecution = sal_True;
}
#endif
//-----------------------------------------------
void VerbExecutionController::LockNotification()
{
    osl::MutexGuard aGuard( m_aVerbExecutionMutex );
    if ( m_nNotificationLock < SAL_MAX_INT32 )
        m_nNotificationLock++;
}

//-----------------------------------------------
void VerbExecutionController::UnlockNotification()
{
    osl::MutexGuard aGuard( m_aVerbExecutionMutex );
    if ( m_nNotificationLock > 0 )
        m_nNotificationLock--;
}

//-----------------------------------------------
uno::Reference< io::XStream > OleEmbeddedObject::GetNewFilledTempStream_Impl( const uno::Reference< io::XInputStream >& xInStream )
        throw( io::IOException )
{
    SAL_WARN_IF( !xInStream.is(), "embeddedobj.ole", "Wrong parameter is provided!" );

    uno::Reference < io::XStream > xTempFile(
            io::TempFile::create(comphelper::getComponentContext(m_xFactory)),
            uno::UNO_QUERY_THROW );

    uno::Reference< io::XOutputStream > xTempOutStream = xTempFile->getOutputStream();
    if ( xTempOutStream.is() )
    {
        ::comphelper::OStorageHelper::CopyInputToOutput( xInStream, xTempOutStream );
        xTempOutStream->flush();
    }
    else
        throw io::IOException(); // TODO:

    return xTempFile;
}

//------------------------------------------------------
uno::Reference< io::XStream > OleEmbeddedObject::TryToGetAcceptableFormat_Impl( const uno::Reference< io::XStream >& xStream )
        throw ( uno::Exception )
{
    // TODO/LATER: Actually this should be done by a centralized component ( may be a graphical filter )
    if ( !m_xFactory.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XInputStream > xInStream = xStream->getInputStream();
    if ( !xInStream.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XSeekable > xSeek( xStream, uno::UNO_QUERY_THROW );
    xSeek->seek( 0 );

    uno::Sequence< sal_Int8 > aData( 8 );
    sal_Int32 nRead = xInStream->readBytes( aData, 8 );
    xSeek->seek( 0 );

    if ( ( nRead >= 2 && aData[0] == 'B' && aData[1] == 'M' )
      || ( nRead >= 4 && aData[0] == 1 && aData[1] == 0 && aData[2] == 9 && aData[3] == 0 ) )
    {
        // it should be a bitmap or a Metafile
        return xStream;
    }


    sal_uInt32 nHeaderOffset = 0;
    if ( ( nRead >= 8 && aData[0] == -1 && aData[1] == -1 && aData[2] == -1 && aData[3] == -1 )
      && ( aData[4] == 2 || aData[4] == 3 || aData[4] == 14 ) && aData[5] == 0 && aData[6] == 0 && aData[7] == 0 )
    {
        nHeaderOffset = 40;
        xSeek->seek( 8 );

        // TargetDevice might be used in future, currently the cache has specified NULL
        uno::Sequence< sal_Int8 > aHeadData( 4 );
        nRead = xInStream->readBytes( aHeadData, 4 );
        sal_uInt32 nLen = 0;
        if ( nRead == 4 && aHeadData.getLength() == 4 )
            nLen = ( ( ( (sal_uInt32)aHeadData[3] * 0x100 + (sal_uInt32)aHeadData[2] ) * 0x100 ) + (sal_uInt32)aHeadData[1] ) * 0x100 + (sal_uInt32)aHeadData[0];
        if ( nLen > 4 )
        {
            xInStream->skipBytes( nLen - 4 );
            nHeaderOffset += nLen - 4;
        }

    }
    else if ( nRead > 4 )
    {
        // check whether the first bytes represent the size
        sal_uInt32 nSize = 0;
        for ( sal_Int32 nInd = 3; nInd >= 0; nInd-- )
            nSize = ( nSize << 8 ) + (sal_uInt8)aData[nInd];

        if ( nSize == xSeek->getLength() - 4 )
            nHeaderOffset = 4;
    }

    if ( nHeaderOffset )
    {
        // this is either a bitmap or a metafile clipboard format, retrieve the pure stream
        uno::Reference < io::XStream > xResult(
            io::TempFile::create(comphelper::getComponentContext(m_xFactory)),
            uno::UNO_QUERY_THROW );
        uno::Reference < io::XSeekable > xResultSeek( xResult, uno::UNO_QUERY_THROW );
        uno::Reference < io::XOutputStream > xResultOut = xResult->getOutputStream();
        uno::Reference < io::XInputStream > xResultIn = xResult->getInputStream();
        if ( !xResultOut.is() || !xResultIn.is() )
            throw uno::RuntimeException();

        xSeek->seek( nHeaderOffset ); // header size for these formats
        ::comphelper::OStorageHelper::CopyInputToOutput( xInStream, xResultOut );
        xResultOut->closeOutput();
        xResultSeek->seek( 0 );
        xSeek->seek( 0 );

        return xResult;
    }

    return uno::Reference< io::XStream >();
}

//------------------------------------------------------
void OleEmbeddedObject::InsertVisualCache_Impl( const uno::Reference< io::XStream >& xTargetStream,
                                                const uno::Reference< io::XStream >& xCachedVisualRepresentation )
        throw ( uno::Exception )
{
    OSL_ENSURE( xTargetStream.is() && xCachedVisualRepresentation.is(), "Invalid argumants!\n" );

    if ( !xTargetStream.is() || !xCachedVisualRepresentation.is() )
        throw uno::RuntimeException();

    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[0] <<= xTargetStream;
    aArgs[1] <<= (sal_Bool)sal_True; // do not create copy

    uno::Reference< container::XNameContainer > xNameContainer(
            m_xFactory->createInstanceWithArguments(
                    OUString( "com.sun.star.embed.OLESimpleStorage" ),
                    aArgs ),
            uno::UNO_QUERY );

    if ( !xNameContainer.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XSeekable > xCachedSeek( xCachedVisualRepresentation, uno::UNO_QUERY_THROW );
    if ( xCachedSeek.is() )
        xCachedSeek->seek( 0 );

    uno::Reference < io::XStream > xTempFile(
            io::TempFile::create(comphelper::getComponentContext(m_xFactory)),
            uno::UNO_QUERY_THROW );

    uno::Reference< io::XSeekable > xTempSeek( xTempFile, uno::UNO_QUERY_THROW );
    uno::Reference< io::XOutputStream > xTempOutStream = xTempFile->getOutputStream();
    if ( xTempOutStream.is() )
    {
        // the OlePres stream must have additional header
        // TODO/LATER: might need to be extended in future ( actually makes sence only for SO7 format )
        uno::Reference< io::XInputStream > xInCacheStream = xCachedVisualRepresentation->getInputStream();
        if ( !xInCacheStream.is() )
            throw uno::RuntimeException();

        // write 0xFFFFFFFF at the beginning
        uno::Sequence< sal_Int8 > aData( 4 );
        *( (sal_uInt32*)aData.getArray() ) = 0xFFFFFFFF;

        xTempOutStream->writeBytes( aData );

        // write clipboard format
        uno::Sequence< sal_Int8 > aSigData( 2 );
        xInCacheStream->readBytes( aSigData, 2 );
        if ( aSigData.getLength() < 2 )
            throw io::IOException();

        if ( aSigData[0] == 'B' && aSigData[1] == 'M' )
        {
            // it's a bitmap
            aData[0] = 0x02; aData[1] = 0; aData[2] = 0; aData[3] = 0;
        }
        else
        {
            // treat it as a metafile
            aData[0] = 0x03; aData[1] = 0; aData[2] = 0; aData[3] = 0;
        }
        xTempOutStream->writeBytes( aData );

        // write job related information
        aData[0] = 0x04; aData[1] = 0; aData[2] = 0; aData[3] = 0;
        xTempOutStream->writeBytes( aData );

        // write aspect
        aData[0] = 0x01; aData[1] = 0; aData[2] = 0; aData[3] = 0;
        xTempOutStream->writeBytes( aData );

        // write l-index
        *( (sal_uInt32*)aData.getArray() ) = 0xFFFFFFFF;
        xTempOutStream->writeBytes( aData );

        // write adv. flags
        aData[0] = 0x02; aData[1] = 0; aData[2] = 0; aData[3] = 0;
        xTempOutStream->writeBytes( aData );

        // write compression
        *( (sal_uInt32*)aData.getArray() ) = 0x0;
        xTempOutStream->writeBytes( aData );

        // get the size
        awt::Size aSize = getVisualAreaSize( embed::Aspects::MSOLE_CONTENT );
        sal_Int32 nIndex = 0;

        // write width
        for ( nIndex = 0; nIndex < 4; nIndex++ )
        {
            aData[nIndex] = (sal_Int8)( aSize.Width % 0x100 );
            aSize.Width /= 0x100;
        }
        xTempOutStream->writeBytes( aData );

        // write height
        for ( nIndex = 0; nIndex < 4; nIndex++ )
        {
            aData[nIndex] = (sal_Int8)( aSize.Height % 0x100 );
            aSize.Height /= 0x100;
        }
        xTempOutStream->writeBytes( aData );

        // write garbage, it will be overwritten by the size
        xTempOutStream->writeBytes( aData );

        // write first bytes that was used to detect the type
        xTempOutStream->writeBytes( aSigData );

        // write the rest of the stream
        ::comphelper::OStorageHelper::CopyInputToOutput( xInCacheStream, xTempOutStream );

        // write the size of the stream
        sal_Int64 nLength = xTempSeek->getLength() - 40;
        if ( nLength < 0 || nLength >= 0xFFFFFFFF )
        {
            SAL_WARN( "embeddedobj.ole", "Length is not acceptable!" );
            return;
        }
        for ( sal_Int32 nInd = 0; nInd < 4; nInd++ )
        {
            aData[nInd] = (sal_Int8)( ( (sal_uInt64) nLength ) % 0x100 );
            nLength /= 0x100;
        }
        xTempSeek->seek( 36 );
        xTempOutStream->writeBytes( aData );

        xTempOutStream->flush();

        xTempSeek->seek( 0 );
        if ( xCachedSeek.is() )
            xCachedSeek->seek( 0 );
    }
    else
        throw io::IOException(); // TODO:

    // insert the result file as replacement image
    OUString aCacheName = OUString( "\002OlePres000" );
    if ( xNameContainer->hasByName( aCacheName ) )
        xNameContainer->replaceByName( aCacheName, uno::makeAny( xTempFile ) );
    else
        xNameContainer->insertByName( aCacheName, uno::makeAny( xTempFile ) );

    uno::Reference< embed::XTransactedObject > xTransacted( xNameContainer, uno::UNO_QUERY );
    if ( !xTransacted.is() )
        throw uno::RuntimeException();

    xTransacted->commit();
}

//------------------------------------------------------
void OleEmbeddedObject::RemoveVisualCache_Impl( const uno::Reference< io::XStream >& xTargetStream )
        throw ( uno::Exception )
{
    OSL_ENSURE( xTargetStream.is(), "Invalid argumant!\n" );
    if ( !xTargetStream.is() )
        throw uno::RuntimeException();

    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[0] <<= xTargetStream;
    aArgs[1] <<= (sal_Bool)sal_True; // do not create copy
    uno::Reference< container::XNameContainer > xNameContainer(
            m_xFactory->createInstanceWithArguments(
                    OUString( "com.sun.star.embed.OLESimpleStorage" ),
                    aArgs ),
            uno::UNO_QUERY );

    if ( !xNameContainer.is() )
        throw uno::RuntimeException();

    for ( sal_uInt8 nInd = 0; nInd < 10; nInd++ )
    {
        OUString aStreamName( "\002OlePres00" );
        aStreamName += OUString::valueOf( (sal_Int32)nInd );
        if ( xNameContainer->hasByName( aStreamName ) )
            xNameContainer->removeByName( aStreamName );
    }

    uno::Reference< embed::XTransactedObject > xTransacted( xNameContainer, uno::UNO_QUERY );
    if ( !xTransacted.is() )
        throw uno::RuntimeException();

    xTransacted->commit();
}

//------------------------------------------------------
void OleEmbeddedObject::SetVisReplInStream( sal_Bool bExists )
{
    m_bVisReplInitialized = sal_True;
    m_bVisReplInStream = bExists;
}

//------------------------------------------------------
sal_Bool OleEmbeddedObject::HasVisReplInStream()
{
    if ( !m_bVisReplInitialized )
    {
        if ( m_xCachedVisualRepresentation.is() )
            SetVisReplInStream( sal_True );
        else
        {
            SAL_INFO( "embeddedobj.ole", "embeddedobj (mv76033) OleEmbeddedObject::HasVisualReplInStream, analizing" );

            uno::Reference< io::XInputStream > xStream;

            OSL_ENSURE( !m_pOleComponent || !m_aTempURL.isEmpty(), "The temporary file must exist if there is a component!\n" );
            if ( !m_aTempURL.isEmpty() )
            {
                try
                {
                    // open temporary file for reading
                    uno::Reference < ucb::XSimpleFileAccess3 > xTempAccess(
                            ucb::SimpleFileAccess::create( comphelper::getComponentContext(m_xFactory) ) );

                    xStream = xTempAccess->openFileRead( m_aTempURL );
                }
                catch( const uno::Exception& )
                {}
            }

            if ( !xStream.is() )
                xStream = m_xObjectStream->getInputStream();

            if ( xStream.is() )
            {
                sal_Bool bExists = sal_False;

                uno::Sequence< uno::Any > aArgs( 2 );
                aArgs[0] <<= xStream;
                aArgs[1] <<= (sal_Bool)sal_True; // do not create copy
                uno::Reference< container::XNameContainer > xNameContainer(
                        m_xFactory->createInstanceWithArguments(
                                OUString( "com.sun.star.embed.OLESimpleStorage" ),
                                aArgs ),
                        uno::UNO_QUERY );

                if ( xNameContainer.is() )
                {
                    for ( sal_uInt8 nInd = 0; nInd < 10 && !bExists; nInd++ )
                    {
                        OUString aStreamName( "\002OlePres00" );
                        aStreamName += OUString::valueOf( (sal_Int32)nInd );
                        try
                        {
                            bExists = xNameContainer->hasByName( aStreamName );
                        }
                        catch( const uno::Exception& )
                        {}
                    }
                }

                SetVisReplInStream( bExists );
            }
        }
    }

    return m_bVisReplInStream;
}

//------------------------------------------------------
uno::Reference< io::XStream > OleEmbeddedObject::TryToRetrieveCachedVisualRepresentation_Impl(
        const uno::Reference< io::XStream >& xStream,
        sal_Bool bAllowToRepair50 )
    throw ()
{
    uno::Reference< io::XStream > xResult;

    if ( xStream.is() )
    {
        SAL_INFO( "embeddedobj.ole", "embeddedobj (mv76033) OleEmbeddedObject::TryToRetrieveCachedVisualRepresentation, retrieving" );

        uno::Reference< container::XNameContainer > xNameContainer;
        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[0] <<= xStream;
        aArgs[1] <<= (sal_Bool)sal_True; // do not create copy
        try
        {
            xNameContainer = uno::Reference< container::XNameContainer >(
                m_xFactory->createInstanceWithArguments(
                        OUString( "com.sun.star.embed.OLESimpleStorage" ),
                        aArgs ),
                uno::UNO_QUERY );
        }
        catch( const uno::Exception& )
        {}

        if ( xNameContainer.is() )
        {
            for ( sal_uInt8 nInd = 0; nInd < 10; nInd++ )
            {
                OUString aStreamName( "\002OlePres00" );
                aStreamName += OUString::valueOf( (sal_Int32)nInd );
                uno::Reference< io::XStream > xCachedCopyStream;
                try
                {
                    if ( ( xNameContainer->getByName( aStreamName ) >>= xCachedCopyStream ) && xCachedCopyStream.is() )
                    {
                        xResult = TryToGetAcceptableFormat_Impl( xCachedCopyStream );
                        if ( xResult.is() )
                            break;
                    }
                }
                catch( const uno::Exception& )
                {}

                if ( nInd == 0 )
                {
                    // to be compatible with the old versions Ole10Native is checked after OlePress000
                    aStreamName = OUString( "\001Ole10Native" );
                    try
                    {
                        if ( ( xNameContainer->getByName( aStreamName ) >>= xCachedCopyStream ) && xCachedCopyStream.is() )
                        {
                            xResult = TryToGetAcceptableFormat_Impl( xCachedCopyStream );
                            if ( xResult.is() )
                                break;
                        }
                    }
                    catch( const uno::Exception& )
                    {}
                }
            }

            try
            {
                if ( bAllowToRepair50 && !xResult.is() )
                {
                    OUString aOrigContName( "Ole-Object" );
                    if ( xNameContainer->hasByName( aOrigContName ) )
                    {
                        uno::Reference< embed::XClassifiedObject > xClassified( xNameContainer, uno::UNO_QUERY_THROW );
                        uno::Sequence< sal_Int8 > aClassID;
                        if ( MimeConfigurationHelper::ClassIDsEqual( xClassified->getClassID(), MimeConfigurationHelper::GetSequenceClassID( SO3_OUT_CLASSID ) ) )
                        {
                            // this is an OLE object wrongly stored in 5.0 format
                            // this object must be repaired since SO7 has done it

                            uno::Reference< io::XOutputStream > xOutputStream = xStream->getOutputStream();
                            uno::Reference< io::XTruncate > xTruncate( xOutputStream, uno::UNO_QUERY_THROW );

                            uno::Reference< io::XInputStream > xOrigInputStream;
                            if ( ( xNameContainer->getByName( aOrigContName ) >>= xOrigInputStream )
                              && xOrigInputStream.is() )
                            {
                                // the provided input stream must be based on temporary medium and must be independent
                                // from the stream the storage is based on
                                uno::Reference< io::XSeekable > xOrigSeekable( xOrigInputStream, uno::UNO_QUERY );
                                if ( xOrigSeekable.is() )
                                    xOrigSeekable->seek( 0 );

                                uno::Reference< lang::XComponent > xNameContDisp( xNameContainer, uno::UNO_QUERY_THROW );
                                xNameContDisp->dispose(); // free the original stream

                                xTruncate->truncate();
                                ::comphelper::OStorageHelper::CopyInputToOutput( xOrigInputStream, xOutputStream );
                                xOutputStream->flush();

                                if ( xStream == m_xObjectStream )
                                {
                                    if ( !m_aTempURL.isEmpty() )
                                    {
                                        // this is the own stream, so the temporary URL must be cleaned if it exists
                                        KillFile_Impl( m_aTempURL, m_xFactory );
                                        m_aTempURL = OUString();
                                    }

#ifdef WNT
                                    // retry to create the component after recovering
                                    GetRidOfComponent();

                                    try
                                    {
                                        CreateOleComponentAndLoad_Impl( NULL );
                                        m_aClassID = m_pOleComponent->GetCLSID(); // was not set during consruction
                                    }
                                    catch( const uno::Exception& )
                                    {
                                        GetRidOfComponent();
                                    }
#endif
                                }

                                xResult = TryToRetrieveCachedVisualRepresentation_Impl( xStream, sal_False );
                            }
                        }
                    }
                }
            }
            catch( const uno::Exception& )
            {}
        }
    }

    return xResult;
}

//------------------------------------------------------
void OleEmbeddedObject::SwitchOwnPersistence( const uno::Reference< embed::XStorage >& xNewParentStorage,
                                              const uno::Reference< io::XStream >& xNewObjectStream,
                                              const OUString& aNewName )
{
    if ( xNewParentStorage == m_xParentStorage && aNewName.equals( m_aEntryName ) )
    {
        SAL_WARN_IF( xNewObjectStream != m_xObjectStream, "embeddedobj.ole", "The streams must be the same!" );
        return;
    }

    try {
        uno::Reference< lang::XComponent > xComponent( m_xObjectStream, uno::UNO_QUERY );
        OSL_ENSURE( !m_xObjectStream.is() || xComponent.is(), "Wrong stream implementation!" );
        if ( xComponent.is() )
            xComponent->dispose();
    }
    catch ( const uno::Exception& )
    {
    }

    m_xObjectStream = xNewObjectStream;
    m_xParentStorage = xNewParentStorage;
    m_aEntryName = aNewName;
}

//------------------------------------------------------
void OleEmbeddedObject::SwitchOwnPersistence( const uno::Reference< embed::XStorage >& xNewParentStorage,
                                              const OUString& aNewName )
{
    if ( xNewParentStorage == m_xParentStorage && aNewName.equals( m_aEntryName ) )
        return;

    sal_Int32 nStreamMode = m_bReadOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE;

    uno::Reference< io::XStream > xNewOwnStream = xNewParentStorage->openStreamElement( aNewName, nStreamMode );
    SAL_WARN_IF( !xNewOwnStream.is(), "embeddedobj.ole", "The method can not return empty reference!" );

    SwitchOwnPersistence( xNewParentStorage, xNewOwnStream, aNewName );
}

#ifdef WNT
//----------------------------------------------
sal_Bool OleEmbeddedObject::SaveObject_Impl()
{
    sal_Bool bResult = sal_False;

    if ( m_xClientSite.is() )
    {
        try
        {
            m_xClientSite->saveObject();
            bResult = sal_True;
        }
        catch( const uno::Exception& )
        {
        }
    }

    return bResult;
}

//----------------------------------------------
sal_Bool OleEmbeddedObject::OnShowWindow_Impl( sal_Bool bShow )
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    sal_Bool bResult = sal_False;

    SAL_WARN_IF( m_nObjectState == -1, "embeddedobj.ole", "The object has no persistence!" );
    SAL_WARN_IF( m_nObjectState == embed::EmbedStates::LOADED, "embeddedobj.ole", "The object get OnShowWindow in loaded state!" );
    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::LOADED )
        return sal_False;

    // the object is either activated or deactivated
    sal_Int32 nOldState = m_nObjectState;
    if ( bShow && m_nObjectState == embed::EmbedStates::RUNNING )
    {
        m_nObjectState = embed::EmbedStates::ACTIVE;
        m_aVerbExecutionController.ObjectIsActive();

        aGuard.clear();
        StateChangeNotification_Impl( sal_False, nOldState, m_nObjectState );
    }
    else if ( !bShow && m_nObjectState == embed::EmbedStates::ACTIVE )
    {
        m_nObjectState = embed::EmbedStates::RUNNING;
        aGuard.clear();
        StateChangeNotification_Impl( sal_False, nOldState, m_nObjectState );
    }

    if ( m_xClientSite.is() )
    {
        try
        {
            m_xClientSite->visibilityChanged( bShow );
            bResult = sal_True;
        }
        catch( const uno::Exception& )
        {
        }
    }

    return bResult;
}

//------------------------------------------------------
void OleEmbeddedObject::OnIconChanged_Impl()
{
    // TODO/LATER: currently this notification seems to be impossible
    // MakeEventListenerNotification_Impl( OUString( "OnIconChanged" ) );
}

//------------------------------------------------------
void OleEmbeddedObject::OnViewChanged_Impl()
{
    if ( m_bDisposed )
        throw lang::DisposedException();

    // For performance reasons the notification currently is ignored, STAMPIT object is the exception,
    // it can never be active and never call SaveObject, so it is the only way to detect that it is changed

    // ==== the STAMPIT related solution =============================
    // the following variable is used to detect whether the object was modified during verb execution
    m_aVerbExecutionController.ModificationNotificationIsDone();

    // The following things are controlled by VerbExecutionController:
    // - if the verb execution is in progress and the view is changed the object will be stored
    // after the execution, so there is no need to send the notification.
    // - the STAMPIT object can never be active.
    if ( m_aVerbExecutionController.CanDoNotification()
      && m_pOleComponent && m_nUpdateMode == embed::EmbedUpdateModes::ALWAYS_UPDATE )
    {
        OSL_ENSURE( MimeConfigurationHelper::ClassIDsEqual( m_aClassID, MimeConfigurationHelper::GetSequenceClassID( 0x852ee1c9, 0x9058, 0x44ba, 0x8c,0x6c,0x0c,0x5f,0xc6,0x6b,0xdb,0x8d ) )
                    || MimeConfigurationHelper::ClassIDsEqual( m_aClassID, MimeConfigurationHelper::GetSequenceClassID( 0xcf1b4491, 0xbea3, 0x4c9f, 0xa7,0x0f,0x22,0x1b,0x1e,0xca,0xef,0x3e ) ),
                    "Expected to be triggered for STAMPIT only! Please contact developers!\n" );

        // The view is changed while the object is in running state, save the new object
        m_xCachedVisualRepresentation = uno::Reference< io::XStream >();
        SaveObject_Impl();
        MakeEventListenerNotification_Impl( OUString( "OnVisAreaChanged" ) );
    }
    // ===============================================================
}

//------------------------------------------------------
void OleEmbeddedObject::OnClosed_Impl()
{
    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( m_nObjectState != embed::EmbedStates::LOADED )
    {
        sal_Int32 nOldState = m_nObjectState;
        m_nObjectState = embed::EmbedStates::LOADED;
        StateChangeNotification_Impl( sal_False, nOldState, m_nObjectState );
    }
}

//------------------------------------------------------
OUString OleEmbeddedObject::CreateTempURLEmpty_Impl()
{
    SAL_WARN_IF( !m_aTempURL.isEmpty(), "embeddedobj.ole", "The object has already the temporary file!" );
    m_aTempURL = GetNewTempFileURL_Impl( m_xFactory );

    return m_aTempURL;
}

//------------------------------------------------------
OUString OleEmbeddedObject::GetTempURL_Impl()
{
    if ( m_aTempURL.isEmpty() )
    {
        SAL_INFO( "embeddedobj.ole", "embeddedobj (mv76033) OleEmbeddedObject::GetTempURL_Impl, tempfile creation" );

        // if there is no temporary file, it will be created from the own entry
        uno::Reference< embed::XOptimizedStorage > xOptParStorage( m_xParentStorage, uno::UNO_QUERY );
        if ( xOptParStorage.is() )
        {
            m_aTempURL = GetNewFilledTempFile_Impl( xOptParStorage, m_aEntryName, m_xFactory );
        }
        else if ( m_xObjectStream.is() )
        {
            // load object from the stream
            uno::Reference< io::XInputStream > xInStream = m_xObjectStream->getInputStream();
            if ( !xInStream.is() )
                throw io::IOException(); // TODO: access denied

            m_aTempURL = GetNewFilledTempFile_Impl( xInStream, m_xFactory );
        }
    }

    return m_aTempURL;
}

//------------------------------------------------------
void OleEmbeddedObject::CreateOleComponent_Impl( OleComponent* pOleComponent )
{
    if ( !m_pOleComponent )
    {
        m_pOleComponent = pOleComponent ? pOleComponent : new OleComponent( m_xFactory, this );
        m_pOleComponent->acquire(); // TODO: needs holder?

        if ( !m_xClosePreventer.is() )
            m_xClosePreventer = uno::Reference< util::XCloseListener >(
                                    static_cast< ::cppu::OWeakObject* >( new OClosePreventer ),
                                    uno::UNO_QUERY );

        m_pOleComponent->addCloseListener( m_xClosePreventer );
    }
}

//------------------------------------------------------
void OleEmbeddedObject::CreateOleComponentAndLoad_Impl( OleComponent* pOleComponent )
{
    if ( !m_pOleComponent )
    {
        if ( !m_xObjectStream.is() )
            throw uno::RuntimeException();

        CreateOleComponent_Impl( pOleComponent );

        // after the loading the object can appear as a link
        // will be detected later by olecomponent

        GetTempURL_Impl();
        if ( m_aTempURL.isEmpty() )
            throw uno::RuntimeException(); // TODO

        m_pOleComponent->LoadEmbeddedObject( m_aTempURL );
    }
}

//------------------------------------------------------
void OleEmbeddedObject::CreateOleComponentFromClipboard_Impl( OleComponent* pOleComponent )
{
    if ( !m_pOleComponent )
    {
        if ( !m_xObjectStream.is() )
            throw uno::RuntimeException();

        CreateOleComponent_Impl( pOleComponent );

        // after the loading the object can appear as a link
        // will be detected later by olecomponent
        m_pOleComponent->CreateObjectFromClipboard();
    }
}

//------------------------------------------------------
uno::Reference< io::XOutputStream > OleEmbeddedObject::GetStreamForSaving()
{
    if ( !m_xObjectStream.is() )
        throw uno::RuntimeException(); //TODO:

    uno::Reference< io::XOutputStream > xOutStream = m_xObjectStream->getOutputStream();
    if ( !xOutStream.is() )
        throw io::IOException(); //TODO: access denied

    uno::Reference< io::XTruncate > xTruncate( xOutStream, uno::UNO_QUERY );
    if ( !xTruncate.is() )
        throw uno::RuntimeException(); //TODO:

    xTruncate->truncate();

    return xOutStream;
}

//----------------------------------------------
void OleEmbeddedObject::StoreObjectToStream( uno::Reference< io::XOutputStream > xOutStream )
    throw ( uno::Exception )
{
    // this method should be used only on windows
    if ( m_pOleComponent )
        m_pOleComponent->StoreOwnTmpIfNecessary();

    // now all the changes should be in temporary location
    if ( m_aTempURL.isEmpty() )
        throw uno::RuntimeException();

    // open temporary file for reading
    uno::Reference < ucb::XSimpleFileAccess3 > xTempAccess(
            ucb::SimpleFileAccess::create( comphelper::getComponentContext(m_xFactory) ) );

    uno::Reference< io::XInputStream > xTempInStream = xTempAccess->openFileRead( m_aTempURL );
    SAL_WARN_IF( !xTempInStream.is(), "embeddedobj.ole", "The object's temporary file can not be reopened for reading!" );

    // TODO: use bStoreVisReplace

    if ( xTempInStream.is() )
    {
        // write all the contents to XOutStream
        uno::Reference< io::XTruncate > xTrunc( xOutStream, uno::UNO_QUERY );
        if ( !xTrunc.is() )
            throw uno::RuntimeException(); //TODO:

        xTrunc->truncate();

        ::comphelper::OStorageHelper::CopyInputToOutput( xTempInStream, xOutStream );
    }
    else
        throw io::IOException(); // TODO:

    // TODO: should the view replacement be in the stream ???
    //       probably it must be specified on storing
}
#endif
//------------------------------------------------------
void OleEmbeddedObject::StoreToLocation_Impl(
                            const uno::Reference< embed::XStorage >& xStorage,
                            const OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lObjArgs,
                            sal_Bool bSaveAs )
        throw ( uno::Exception )
{
    // TODO: use lObjArgs
    // TODO: exchange StoreVisualReplacement by SO file format version?

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( OUString( "Can't store object without persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    OUString( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    OSL_ENSURE( m_xParentStorage.is() && m_xObjectStream.is(), "The object has no valid persistence!\n" );

    sal_Bool bVisReplIsStored = sal_False;

    sal_Bool bTryOptimization = sal_False;
    sal_Bool bStoreVis = m_bStoreVisRepl;
    uno::Reference< io::XStream > xCachedVisualRepresentation;
    for ( sal_Int32 nInd = 0; nInd < lObjArgs.getLength(); nInd++ )
    {
        if ( lObjArgs[nInd].Name == "StoreVisualReplacement" )
            lObjArgs[nInd].Value >>= bStoreVis;
        else if ( lObjArgs[nInd].Name == "VisualReplacement" )
            lObjArgs[nInd].Value >>= xCachedVisualRepresentation;
        else if ( lObjArgs[nInd].Name == "CanTryOptimization" )
            lObjArgs[nInd].Value >>= bTryOptimization;
    }

    // ignore visual representation provided from outside if it should not be stored
    if ( !bStoreVis )
        xCachedVisualRepresentation = uno::Reference< io::XStream >();

    if ( bStoreVis && !HasVisReplInStream() && !xCachedVisualRepresentation.is() )
        throw io::IOException(); // TODO: there is no cached visual representation and nothing is provided from outside

    // if the representation is provided from outside it should be copied to a local stream
    sal_Bool bNeedLocalCache = xCachedVisualRepresentation.is();

    uno::Reference< io::XStream > xTargetStream;

    sal_Bool bStoreLoaded = sal_False;
    if ( m_nObjectState == embed::EmbedStates::LOADED
#ifdef WNT
        // if the object was NOT modified after storing it can be just copied
        // as if it was in loaded state
      || ( m_pOleComponent && !m_pOleComponent->IsDirty() )
#endif
    )
    {
        sal_Bool bOptimizedCopyingDone = sal_False;

        if ( bTryOptimization && bStoreVis == HasVisReplInStream() )
        {
            try
            {
                uno::Reference< embed::XOptimizedStorage > xSourceOptStor( m_xParentStorage, uno::UNO_QUERY_THROW );
                uno::Reference< embed::XOptimizedStorage > xTargetOptStor( xStorage, uno::UNO_QUERY_THROW );
                xSourceOptStor->copyElementDirectlyTo( m_aEntryName, xTargetOptStor, sEntName );
                bOptimizedCopyingDone = sal_True;
            }
            catch( const uno::Exception& )
            {
            }
        }

        if ( !bOptimizedCopyingDone )
        {
            // if optimized copying fails a normal one should be tried
            m_xParentStorage->copyElementTo( m_aEntryName, xStorage, sEntName );
        }

        // the locally retrieved representation is always preferable
        // since the object is in loaded state the representation is unchanged
        if ( m_xCachedVisualRepresentation.is() )
        {
            xCachedVisualRepresentation = m_xCachedVisualRepresentation;
            bNeedLocalCache = sal_False;
        }

        bVisReplIsStored = HasVisReplInStream();
        bStoreLoaded = sal_True;
    }
#ifdef WNT
    else if ( m_pOleComponent )
    {
        xTargetStream =
                xStorage->openStreamElement( sEntName, embed::ElementModes::READWRITE );
        if ( !xTargetStream.is() )
            throw io::IOException(); //TODO: access denied

        SetStreamMediaType_Impl( xTargetStream, OUString( "application/vnd.sun.star.oleobject" ));
        uno::Reference< io::XOutputStream > xOutStream = xTargetStream->getOutputStream();
        if ( !xOutStream.is() )
            throw io::IOException(); //TODO: access denied

        StoreObjectToStream( xOutStream );
        bVisReplIsStored = sal_True;

        if ( bSaveAs )
        {
            // no need to do it on StoreTo since in this case the replacement is in the stream
            // and there is no need to cache it even if it is thrown away because the object
            // is not changed by StoreTo action

            uno::Reference< io::XStream > xTmpCVRepresentation =
                        TryToRetrieveCachedVisualRepresentation_Impl( xTargetStream );

            // the locally retrieved representation is always preferable
            if ( xTmpCVRepresentation.is() )
            {
                xCachedVisualRepresentation = xTmpCVRepresentation;
                bNeedLocalCache = sal_False;
            }
        }
    }
#endif
    else
    {
        throw io::IOException(); // TODO
    }

    if ( !xTargetStream.is() )
    {
        xTargetStream =
            xStorage->openStreamElement( sEntName, embed::ElementModes::READWRITE );
        if ( !xTargetStream.is() )
            throw io::IOException(); //TODO: access denied
    }

    LetCommonStoragePassBeUsed_Impl( xTargetStream );

    if ( bStoreVis != bVisReplIsStored )
    {
        if ( bStoreVis )
        {
            if ( !xCachedVisualRepresentation.is() )
                xCachedVisualRepresentation = TryToRetrieveCachedVisualRepresentation_Impl( xTargetStream );

            SAL_WARN_IF( !xCachedVisualRepresentation.is(), "embeddedobj.ole", "No representation is available!" );

            // the following copying will be done in case it is SaveAs anyway
            // if it is not SaveAs the seekable access is not required currently
            // TODO/LATER: may be required in future
            if ( bSaveAs )
            {
                uno::Reference< io::XSeekable > xCachedSeek( xCachedVisualRepresentation, uno::UNO_QUERY );
                if ( !xCachedSeek.is() )
                {
                    xCachedVisualRepresentation
                        = GetNewFilledTempStream_Impl( xCachedVisualRepresentation->getInputStream() );
                    bNeedLocalCache = sal_False;
                }
            }

            InsertVisualCache_Impl( xTargetStream, xCachedVisualRepresentation );
        }
        else
        {
            // the removed representation could be cached by this method
            if ( !xCachedVisualRepresentation.is() )
                xCachedVisualRepresentation = TryToRetrieveCachedVisualRepresentation_Impl( xTargetStream );

            RemoveVisualCache_Impl( xTargetStream );
        }
    }

    if ( bSaveAs )
    {
        m_bWaitSaveCompleted = sal_True;
        m_xNewObjectStream = xTargetStream;
        m_xNewParentStorage = xStorage;
        m_aNewEntryName = sEntName;
        m_bNewVisReplInStream = bStoreVis;
        m_bStoreLoaded = bStoreLoaded;

        if ( xCachedVisualRepresentation.is() )
        {
            if ( bNeedLocalCache )
                m_xNewCachedVisRepl = GetNewFilledTempStream_Impl( xCachedVisualRepresentation->getInputStream() );
            else
                m_xNewCachedVisRepl = xCachedVisualRepresentation;
        }

        // TODO: register listeners for storages above, in case they are disposed
        //       an exception will be thrown on saveCompleted( true )
    }
    else
    {
        uno::Reference< lang::XComponent > xComp( xTargetStream, uno::UNO_QUERY );
        if ( xComp.is() )
        {
            try {
                xComp->dispose();
            } catch( const uno::Exception& )
            {
            }
        }
    }
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::setPersistentEntry(
                    const uno::Reference< embed::XStorage >& xStorage,
                    const OUString& sEntName,
                    sal_Int32 nEntryConnectionMode,
                    const uno::Sequence< beans::PropertyValue >& lArguments,
                    const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    SAL_INFO( "embeddedobj.ole", "embeddedobj (mv76033) OleEmbeddedObject::setPersistentEntry" );

    // begin wrapping related part ====================
    uno::Reference< embed::XEmbedPersist > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->setPersistentEntry( xStorage, sEntName, nEntryConnectionMode, lArguments, lObjArgs );
        return;
    }
    // end wrapping related part ====================

    // TODO: use lObjArgs

    // the type of the object must be already set
    // a kind of typedetection should be done in the factory;
    // the only exception is object initialized from a stream,
    // the class ID will be detected from the stream

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( OUString( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( OUString( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    // May be LOADED should be forbidden here ???
    if ( ( m_nObjectState != -1 || nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
      && ( m_nObjectState == -1 || nEntryConnectionMode != embed::EntryInitModes::NO_INIT ) )
    {
        // if the object is not loaded
        // it can not get persistent representation without initialization

        // if the object is loaded
        // it can switch persistent representation only without initialization

        throw embed::WrongStateException(
                    OUString( "Can't change persistent representation of activated object!\n" ),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
    {
        if ( nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
            saveCompleted( ( m_xParentStorage != xStorage || !m_aEntryName.equals( sEntName ) ) );
        else
            throw embed::WrongStateException(
                        OUString( "The object waits for saveCompleted() call!\n" ),
                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    uno::Reference< container::XNameAccess > xNameAccess( xStorage, uno::UNO_QUERY );
    if ( !xNameAccess.is() )
        throw uno::RuntimeException(); //TODO

    // detect entry existence
    sal_Bool bElExists = xNameAccess->hasByName( sEntName );

    m_bReadOnly = sal_False;
    for ( sal_Int32 nInd = 0; nInd < lArguments.getLength(); nInd++ )
        if ( lArguments[nInd].Name == "ReadOnly" )
            lArguments[nInd].Value >>= m_bReadOnly;

#ifdef WNT
    sal_Int32 nStorageMode = m_bReadOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE;
#endif

    SwitchOwnPersistence( xStorage, sEntName );

    for ( sal_Int32 nInd = 0; nInd < lObjArgs.getLength(); nInd++ )
        if ( lObjArgs[nInd].Name == "StoreVisualReplacement" )
            lObjArgs[nInd].Value >>= m_bStoreVisRepl;

#ifdef WNT
    if ( nEntryConnectionMode == embed::EntryInitModes::DEFAULT_INIT )
    {
        if ( m_bFromClipboard )
        {
            // the object should be initialized from clipboard
            // inpossibility to initialize the object means error here
            CreateOleComponentFromClipboard_Impl( NULL );
            m_aClassID = m_pOleComponent->GetCLSID(); // was not set during consruction
            m_pOleComponent->RunObject();
            m_nObjectState = embed::EmbedStates::RUNNING;
        }
        else if ( bElExists )
        {
            // load object from the stream
            // after the loading the object can appear as a link
            // will be detected by olecomponent
            try
            {
                CreateOleComponentAndLoad_Impl( NULL );
                m_aClassID = m_pOleComponent->GetCLSID(); // was not set during consruction
            }
            catch( const uno::Exception& )
            {
                // TODO/LATER: detect classID of the object if possible
                // means that the object inprocess server could not be successfully instantiated
                GetRidOfComponent();
            }

            m_nObjectState = embed::EmbedStates::LOADED;
        }
        else
        {
            // create a new object
            CreateOleComponent_Impl();
            m_pOleComponent->CreateNewEmbeddedObject( m_aClassID );
            m_pOleComponent->RunObject();
            m_nObjectState = embed::EmbedStates::RUNNING;
        }
    }
    else
    {
        if ( ( nStorageMode & embed::ElementModes::READWRITE ) != embed::ElementModes::READWRITE )
            throw io::IOException();

        if ( nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
        {
            // the document just already changed its stream to store to;
            // the links to OLE documents switch their persistence in the same way
            // as normal embedded objects
        }
        else if ( nEntryConnectionMode == embed::EntryInitModes::TRUNCATE_INIT )
        {
            // create a new object, that will be stored in specified stream
            CreateOleComponent_Impl();

            m_pOleComponent->CreateNewEmbeddedObject( m_aClassID );
            m_pOleComponent->RunObject();
            m_nObjectState = embed::EmbedStates::RUNNING;
        }
        else if ( nEntryConnectionMode == embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT )
        {
            // use URL ( may be content or stream later ) from MediaDescriptor to initialize object
            OUString aURL;
            for ( sal_Int32 nInd = 0; nInd < lArguments.getLength(); nInd++ )
                if ( lArguments[nInd].Name == "URL" )
                    lArguments[nInd].Value >>= aURL;

            if ( aURL.isEmpty() )
                throw lang::IllegalArgumentException(
                                    OUString( "Empty URL is provided in the media descriptor!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                    4 );

            CreateOleComponent_Impl();

            // TODO: the m_bIsLink value must be set already
            if ( !m_bIsLink )
                m_pOleComponent->CreateObjectFromFile( aURL );
            else
                m_pOleComponent->CreateLinkFromFile( aURL );

            m_pOleComponent->RunObject();
            m_aClassID = m_pOleComponent->GetCLSID(); // was not set during consruction

            m_nObjectState = embed::EmbedStates::RUNNING;
        }
        //else if ( nEntryConnectionMode == embed::EntryInitModes::TRANSFERABLE_INIT )
        //{
            //TODO:
        //}
        else
            throw lang::IllegalArgumentException( OUString( "Wrong connection mode is provided!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                        3 );
    }
#else
    // On unix the ole object can not do anything except storing itself somewere
    if ( nEntryConnectionMode == embed::EntryInitModes::DEFAULT_INIT && bElExists )
    {
        // TODO/LATER: detect classID of the object
        // can be a real problem for the links

        m_nObjectState = embed::EmbedStates::LOADED;
    }
    else if ( nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
    {
        // do nothing, the object has already switched it's persistence
    }
    else
        throw lang::IllegalArgumentException( OUString( "Wrong connection mode is provided!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                    3 );

#endif
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::storeToEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lArguments,
                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    SAL_INFO( "embeddedobj.ole", "embeddedobj (mv76033) OleEmbeddedObject::storeToEntry" );

    // begin wrapping related part ====================
    uno::Reference< embed::XEmbedPersist > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->storeToEntry( xStorage, sEntName, lArguments, lObjArgs );
        return;
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    VerbExecutionControllerGuard aVerbGuard( m_aVerbExecutionController );

    StoreToLocation_Impl( xStorage, sEntName, lObjArgs, sal_False );

    // TODO: should the listener notification be done?
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::storeAsEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lArguments,
                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    SAL_INFO( "embeddedobj.ole", "embeddedobj (mv76033) OleEmbeddedObject::storeAsEntry" );

    // begin wrapping related part ====================
    uno::Reference< embed::XEmbedPersist > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->storeAsEntry( xStorage, sEntName, lArguments, lObjArgs );
        return;
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    VerbExecutionControllerGuard aVerbGuard( m_aVerbExecutionController );

    StoreToLocation_Impl( xStorage, sEntName, lObjArgs, sal_True );

    // TODO: should the listener notification be done here or in saveCompleted?
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::saveCompleted( sal_Bool bUseNew )
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    SAL_INFO( "embeddedobj.ole", "embeddedobj (mv76033) OleEmbeddedObject::saveCompleted" );

    // begin wrapping related part ====================
    uno::Reference< embed::XEmbedPersist > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->saveCompleted( bUseNew );
        return;
    }
    // end wrapping related part ====================

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( OUString( "Can't store object without persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    // it is allowed to call saveCompleted( false ) for nonstored objects
    if ( !m_bWaitSaveCompleted && !bUseNew )
        return;

    SAL_WARN_IF( !m_bWaitSaveCompleted, "embeddedobj.ole", "Unexpected saveCompleted() call!\n" );
    if ( !m_bWaitSaveCompleted )
        throw io::IOException(); // TODO: illegal call

    OSL_ENSURE( m_xNewObjectStream.is() && m_xNewParentStorage.is() , "Internal object information is broken!\n" );
    if ( !m_xNewObjectStream.is() || !m_xNewParentStorage.is() )
        throw uno::RuntimeException(); // TODO: broken internal information

    if ( bUseNew )
    {
        SwitchOwnPersistence( m_xNewParentStorage, m_xNewObjectStream, m_aNewEntryName );
        m_bStoreVisRepl = m_bNewVisReplInStream;
        SetVisReplInStream( m_bNewVisReplInStream );
        m_xCachedVisualRepresentation = m_xNewCachedVisRepl;
    }
    else
    {
        // close remembered stream
        try {
            uno::Reference< lang::XComponent > xComponent( m_xNewObjectStream, uno::UNO_QUERY );
            SAL_WARN_IF( !xComponent.is(), "embeddedobj.ole", "Wrong storage implementation!" );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( const uno::Exception& )
        {
        }
    }

    sal_Bool bStoreLoaded = m_bStoreLoaded;

    m_xNewObjectStream = uno::Reference< io::XStream >();
    m_xNewParentStorage = uno::Reference< embed::XStorage >();
    m_aNewEntryName = OUString();
    m_bWaitSaveCompleted = sal_False;
    m_bNewVisReplInStream = sal_False;
    m_xNewCachedVisRepl = uno::Reference< io::XStream >();
    m_bStoreLoaded = sal_False;

    if ( bUseNew && m_pOleComponent && m_nUpdateMode == embed::EmbedUpdateModes::ALWAYS_UPDATE && !bStoreLoaded
      && m_nObjectState != embed::EmbedStates::LOADED )
    {
        // the object replacement image should be updated, so the cached size as well
        m_bHasCachedSize = sal_False;
        try
        {
            // the call will cache the size in case of success
            // probably it might need to be done earlier, while the object is in active state
            getVisualAreaSize( embed::Aspects::MSOLE_CONTENT );
        }
        catch( const uno::Exception& )
        {}
    }

    aGuard.clear();
    if ( bUseNew )
    {
        MakeEventListenerNotification_Impl( OUString( "OnSaveAsDone" ));

        // the object can be changed only on windows
        // the notification should be done only if the object is not in loaded state
        if ( m_pOleComponent && m_nUpdateMode == embed::EmbedUpdateModes::ALWAYS_UPDATE && !bStoreLoaded )
        {
            MakeEventListenerNotification_Impl( OUString( "OnVisAreaChanged" ));
        }
    }
}

//------------------------------------------------------
sal_Bool SAL_CALL OleEmbeddedObject::hasEntry()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbedPersist > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        return xWrappedObject->hasEntry();
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    OUString( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_xObjectStream.is() )
        return sal_True;

    return sal_False;
}

//------------------------------------------------------
OUString SAL_CALL OleEmbeddedObject::getEntryName()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbedPersist > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        return xWrappedObject->getEntryName();
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( OUString( "The object persistence is not initialized!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    OUString( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_aEntryName;
}


//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::storeOwn()
        throw ( embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    SAL_INFO( "embeddedobj.ole", "embeddedobj (mv76033) OleEmbeddedObject::storeOwn" );

    // begin wrapping related part ====================
    uno::Reference< embed::XEmbedPersist > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->storeOwn();
        return;
    }
    // end wrapping related part ====================

    // during switching from Activated to Running and from Running to Loaded states the object will
    // ask container to store the object, the container has to make decision
    // to do so or not

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    VerbExecutionControllerGuard aVerbGuard( m_aVerbExecutionController );

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( OUString( "Can't store object without persistence!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    OUString( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_bReadOnly )
        throw io::IOException(); // TODO: access denied

    LetCommonStoragePassBeUsed_Impl( m_xObjectStream );

    sal_Bool bStoreLoaded = sal_True;

#ifdef WNT
    if ( m_nObjectState != embed::EmbedStates::LOADED && m_pOleComponent && m_pOleComponent->IsDirty() )
    {
        bStoreLoaded = sal_False;

        OSL_ENSURE( m_xParentStorage.is() && m_xObjectStream.is(), "The object has no valid persistence!\n" );

        if ( !m_xObjectStream.is() )
            throw io::IOException(); //TODO: access denied

        SetStreamMediaType_Impl( m_xObjectStream, OUString( "application/vnd.sun.star.oleobject" ));
        uno::Reference< io::XOutputStream > xOutStream = m_xObjectStream->getOutputStream();
        if ( !xOutStream.is() )
            throw io::IOException(); //TODO: access denied

        // TODO: does this work for links too?
        StoreObjectToStream( GetStreamForSaving() );

        // the replacement is changed probably, and it must be in the object stream
        if ( !m_pOleComponent->IsWorkaroundActive() )
            m_xCachedVisualRepresentation = uno::Reference< io::XStream >();
        SetVisReplInStream( sal_True );
    }
#endif

    if ( m_bStoreVisRepl != HasVisReplInStream() )
    {
        if ( m_bStoreVisRepl )
        {
            // the m_xCachedVisualRepresentation must be set or it should be already stored
            if ( m_xCachedVisualRepresentation.is() )
                InsertVisualCache_Impl( m_xObjectStream, m_xCachedVisualRepresentation );
            else
            {
                m_xCachedVisualRepresentation = TryToRetrieveCachedVisualRepresentation_Impl( m_xObjectStream );
                SAL_WARN_IF( !m_xCachedVisualRepresentation.is(), "embeddedobj.ole", "No representation is available!" );
            }
        }
        else
        {
            if ( !m_xCachedVisualRepresentation.is() )
                m_xCachedVisualRepresentation = TryToRetrieveCachedVisualRepresentation_Impl( m_xObjectStream );
            RemoveVisualCache_Impl( m_xObjectStream );
        }

        SetVisReplInStream( m_bStoreVisRepl );
    }

    if ( m_pOleComponent && m_nUpdateMode == embed::EmbedUpdateModes::ALWAYS_UPDATE && !bStoreLoaded )
    {
        // the object replacement image should be updated, so the cached size as well
        m_bHasCachedSize = sal_False;
        try
        {
            // the call will cache the size in case of success
            // probably it might need to be done earlier, while the object is in active state
            getVisualAreaSize( embed::Aspects::MSOLE_CONTENT );
        }
        catch( const uno::Exception& )
        {}
    }

    aGuard.clear();

    MakeEventListenerNotification_Impl( OUString( "OnSaveDone" ));

    // the object can be changed only on Windows
    // the notification should be done only if the object is not in loaded state
    if ( m_pOleComponent && m_nUpdateMode == embed::EmbedUpdateModes::ALWAYS_UPDATE && !bStoreLoaded )
        MakeEventListenerNotification_Impl( OUString( "OnVisAreaChanged" ));
}

//------------------------------------------------------
sal_Bool SAL_CALL OleEmbeddedObject::isReadonly()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbedPersist > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        return xWrappedObject->isReadonly();
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( OUString( "The object persistence is not initialized!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    OUString( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_bReadOnly;
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::reload(
                const uno::Sequence< beans::PropertyValue >& lArguments,
                const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbedPersist > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->reload( lArguments, lObjArgs );
        return;
    }
    // end wrapping related part ====================

    // TODO: use lObjArgs

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( OUString( "The object persistence is not initialized!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    OUString( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    // TODO:
    // throw away current document
    // load new document from current storage
    // use meaningfull part of lArguments
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::breakLink( const uno::Reference< embed::XStorage >& xStorage,
                                                const OUString& sEntName )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XLinkageSupport > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->breakLink( xStorage, sEntName );
        return;
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( OUString( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( OUString( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    // TODO: The object must be at least in Running state;
    if ( !m_bIsLink || m_nObjectState == -1 || !m_pOleComponent )
    {
        // it must be a linked initialized object
        throw embed::WrongStateException(
                    OUString( "The object is not a valid linked object!\n" ),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bReadOnly )
        throw io::IOException(); // TODO: Access denied

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    OUString( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );


#ifdef WNT
    if ( m_pOleComponent )
    {
        // TODO: create an object based on the link

        // disconnect the old temporary URL
        OUString aOldTempURL = m_aTempURL;
        m_aTempURL = OUString();

        OleComponent* pNewOleComponent = new OleComponent( m_xFactory, this );
        try {
            pNewOleComponent->InitEmbeddedCopyOfLink( m_pOleComponent );
        }
        catch ( const uno::Exception& )
        {
            delete pNewOleComponent;
            if ( !m_aTempURL.isEmpty() )
                   KillFile_Impl( m_aTempURL, m_xFactory );
            m_aTempURL = aOldTempURL;
            throw;
        }

        try {
            GetRidOfComponent();
        }
        catch( const uno::Exception& )
        {
            delete pNewOleComponent;
            if ( !m_aTempURL.isEmpty() )
                   KillFile_Impl( m_aTempURL, m_xFactory );
            m_aTempURL = aOldTempURL;
            throw;
        }

           KillFile_Impl( aOldTempURL, m_xFactory );

        CreateOleComponent_Impl( pNewOleComponent );

        if ( m_xParentStorage != xStorage || !m_aEntryName.equals( sEntName ) )
            SwitchOwnPersistence( xStorage, sEntName );

        if ( m_nObjectState != embed::EmbedStates::LOADED )
        {
            // TODO: should we activate the new object if the link was activated?

            sal_Int32 nTargetState = m_nObjectState;
            m_nObjectState = embed::EmbedStates::LOADED;

            if ( m_nObjectState == embed::EmbedStates::RUNNING )
                m_pOleComponent->RunObject(); // the object already was in running state, the server must be installed
            else // m_nObjectState == embed::EmbedStates::ACTIVE
            {
                m_pOleComponent->RunObject(); // the object already was in running state, the server must be installed
                m_pOleComponent->ExecuteVerb( embed::EmbedVerbs::MS_OLEVERB_OPEN );
            }

            m_nObjectState = nTargetState;
        }

        m_bIsLink = sal_False;
        m_aLinkURL = OUString();
    }
    else
#endif
    {
        throw io::IOException(); //TODO:
    }
}

//------------------------------------------------------
sal_Bool SAL_CALL  OleEmbeddedObject::isLink()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XLinkageSupport > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        return xWrappedObject->isLink();
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_bIsLink;
}

//------------------------------------------------------
OUString SAL_CALL OleEmbeddedObject::getLinkURL()
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XLinkageSupport > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        return xWrappedObject->getLinkURL();
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    OUString( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    if ( !m_bIsLink )
        throw embed::WrongStateException(
                    OUString( "The object is not a link object!\n" ),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    // TODO: probably the link URL can be retrieved from OLE

    return m_aLinkURL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
