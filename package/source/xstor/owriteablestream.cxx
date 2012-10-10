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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <osl/diagnose.h>

#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/ofopxmlhelper.hxx>

#include <rtl/digest.h>
#include <rtl/logfile.hxx>
#include <rtl/instance.hxx>

#include <PackageConstants.hxx>
#include <mutexholder.hxx>

#include "selfterminatefilestream.hxx"
#include "owriteablestream.hxx"
#include "oseekinstream.hxx"
#include "xstorage.hxx"

// since the copying uses 32000 blocks usually, it makes sense to have a smaller size
#define MAX_STORCACHE_SIZE 30000


using namespace ::com::sun::star;

namespace package
{
//-----------------------------------------------
bool PackageEncryptionDatasEqual( const ::comphelper::SequenceAsHashMap& aHash1, const ::comphelper::SequenceAsHashMap& aHash2 )
{
    bool bResult = ( aHash1.size() && aHash1.size() == aHash2.size() );
    for ( ::comphelper::SequenceAsHashMap::const_iterator aIter = aHash1.begin();
          bResult && aIter != aHash1.end();
          ++aIter )
    {
        uno::Sequence< sal_Int8 > aKey1;
        bResult = ( ( aIter->second >>= aKey1 ) && aKey1.getLength() );
        if ( bResult )
        {
            uno::Sequence< sal_Int8 > aKey2 = aHash2.getUnpackedValueOrDefault( aIter->first, uno::Sequence< sal_Int8 >() );
            bResult = ( aKey1.getLength() == aKey2.getLength() );
            for ( sal_Int32 nInd = 0; bResult && nInd < aKey1.getLength(); nInd++ )
                bResult = ( aKey1[nInd] == aKey2[nInd] );
        }
    }

    return bResult;
}

//-----------------------------------------------
void StaticAddLog( const ::rtl::OUString& aMessage )
{
    try
    {
        ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
        if ( aContext.is() )
        {
            uno::Reference< logging::XSimpleLogRing > xLogRing( aContext.getSingleton( "com.sun.star.logging.DocumentIOLogRing" ), uno::UNO_QUERY_THROW );
            xLogRing->logString( aMessage );
        }
    }
    catch( const uno::Exception& )
    {
        // No log
    }
}
} // namespace package

// ================================================================
namespace
{
//-----------------------------------------------
void SetEncryptionKeyProperty_Impl( const uno::Reference< beans::XPropertySet >& xPropertySet,
                                    const uno::Sequence< beans::NamedValue >& aKey )
{
    OSL_ENSURE( xPropertySet.is(), "No property set is provided!\n" );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    try {
        xPropertySet->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( STORAGE_ENCRYPTION_KEYS_PROPERTY ) ), uno::makeAny( aKey ) );
    }
    catch ( const uno::Exception& rException )
    {
        ::package::StaticAddLog( rException.Message );
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Can't set encryption") ) );
        OSL_FAIL( "Can't write encryption related properties!\n" );
        throw io::IOException(); // TODO
    }
}

//-----------------------------------------------
uno::Any GetEncryptionKeyProperty_Impl( const uno::Reference< beans::XPropertySet >& xPropertySet )
{
    OSL_ENSURE( xPropertySet.is(), "No property set is provided!\n" );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    try {
        return xPropertySet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( STORAGE_ENCRYPTION_KEYS_PROPERTY ) ) );
    }
    catch ( const uno::Exception& rException )
    {
        ::package::StaticAddLog( rException.Message );
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Can't get encryption property" ) ) );

        OSL_FAIL( "Can't get encryption related properties!\n" );
        throw io::IOException(); // TODO
    }
}

//-----------------------------------------------
bool SequencesEqual( const uno::Sequence< sal_Int8 >& aSequence1, const uno::Sequence< sal_Int8 >& aSequence2 )
{
    if ( aSequence1.getLength() != aSequence2.getLength() )
        return false;

    for ( sal_Int32 nInd = 0; nInd < aSequence1.getLength(); nInd++ )
        if ( aSequence1[nInd] != aSequence2[nInd] )
            return false;

    return true;
}

//-----------------------------------------------
bool SequencesEqual( const uno::Sequence< beans::NamedValue >& aSequence1, const uno::Sequence< beans::NamedValue >& aSequence2 )
{
    if ( aSequence1.getLength() != aSequence2.getLength() )
        return false;

    for ( sal_Int32 nInd = 0; nInd < aSequence1.getLength(); nInd++ )
    {
        bool bHasMember = false;
        uno::Sequence< sal_Int8 > aMember1;
        sal_Int32 nMember1 = 0;
        if ( ( aSequence1[nInd].Value >>= aMember1 ) )
        {
            for ( sal_Int32 nInd2 = 0; nInd2 < aSequence2.getLength(); nInd2++ )
            {
                if ( aSequence1[nInd].Name.equals( aSequence2[nInd2].Name ) )
                {
                    bHasMember = true;

                    uno::Sequence< sal_Int8 > aMember2;
                    if ( !( aSequence2[nInd2].Value >>= aMember2 ) || !SequencesEqual( aMember1, aMember2 ) )
                        return false;
                }
            }
        }
        else if ( ( aSequence1[nInd].Value >>= nMember1 ) )
        {
            for ( sal_Int32 nInd2 = 0; nInd2 < aSequence2.getLength(); nInd2++ )
            {
                if ( aSequence1[nInd].Name.equals( aSequence2[nInd2].Name ) )
                {
                    bHasMember = true;

                    sal_Int32 nMember2 = 0;
                    if ( !( aSequence2[nInd2].Value >>= nMember2 ) || nMember1 != nMember2 )
                        return false;
                }
            }
        }
        else
            return false;

        if ( !bHasMember )
            return false;
    }

    return true;
}

//-----------------------------------------------
sal_Bool KillFile( const ::rtl::OUString& aURL, const uno::Reference< uno::XComponentContext >& xContext )
{
    if ( !xContext.is() )
        return sal_False;

    sal_Bool bRet = sal_False;

    try
    {
        uno::Reference < ucb::XSimpleFileAccess2 > xAccess( ucb::SimpleFileAccess::create( xContext ) );

        xAccess->kill( aURL );
        bRet = sal_True;
    }
    catch( const uno::Exception& rException )
    {
        ::package::StaticAddLog( rException.Message );
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Quiet exception") ) );
    }

    return bRet;
}

const sal_Int32 n_ConstBufferSize = 32000;

//-----------------------------------------------
::rtl::OUString GetNewTempFileURL( const uno::Reference< lang::XMultiServiceFactory > xFactory )
{
    ::rtl::OUString aTempURL;

    uno::Reference < beans::XPropertySet > xTempFile(
            io::TempFile::create(comphelper::getComponentContext(xFactory)),
            uno::UNO_QUERY_THROW );

    try {
        xTempFile->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RemoveFile") ), uno::makeAny( sal_False ) );
        uno::Any aUrl = xTempFile->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Uri") ) );
        aUrl >>= aTempURL;
    }
    catch ( const uno::Exception& rException )
    {
        ::package::StaticAddLog( rException.Message );
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Quiet exception") ) );
    }

    if ( aTempURL.isEmpty() )
        throw uno::RuntimeException(); // TODO: can not create tempfile

    return aTempURL;
}

//-----------------------------------------------
uno::Reference< io::XStream > CreateMemoryStream( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    if ( !xFactory.is() )
        throw uno::RuntimeException();

    return uno::Reference< io::XStream >( xFactory->createInstance (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.MemoryStream") ) ), uno::UNO_QUERY_THROW);
}

} // anonymous namespace
// ================================================================

//-----------------------------------------------
OWriteStream_Impl::OWriteStream_Impl( OStorage_Impl* pParent,
                                      const uno::Reference< packages::XDataSinkEncrSupport >& xPackageStream,
                                      const uno::Reference< lang::XSingleServiceFactory >& xPackage,
                                      const uno::Reference< lang::XMultiServiceFactory >& xFactory,
                                      sal_Bool bForceEncrypted,
                                      sal_Int32 nStorageType,
                                      sal_Bool bDefaultCompress,
                                      const uno::Reference< io::XInputStream >& xRelInfoStream )
: m_pAntiImpl( NULL )
, m_bHasDataToFlush( sal_False )
, m_bFlushed( sal_False )
, m_xPackageStream( xPackageStream )
, m_xFactory( xFactory )
, m_pParent( pParent )
, m_bForceEncrypted( bForceEncrypted )
, m_bUseCommonEncryption( !bForceEncrypted && nStorageType == embed::StorageFormats::PACKAGE )
, m_bHasCachedEncryptionData( sal_False )
, m_bCompressedSetExplicit( !bDefaultCompress )
, m_xPackage( xPackage )
, m_bHasInsertedStreamOptimization( sal_False )
, m_nStorageType( nStorageType )
, m_xOrigRelInfoStream( xRelInfoStream )
, m_bOrigRelInfoBroken( sal_False )
, m_nRelInfoStatus( RELINFO_NO_INIT )
, m_nRelId( 1 )
{
    OSL_ENSURE( xPackageStream.is(), "No package stream is provided!\n" );
    OSL_ENSURE( xPackage.is(), "No package component is provided!\n" );
    OSL_ENSURE( m_xFactory.is(), "No package stream is provided!\n" );
    OSL_ENSURE( pParent, "No parent storage is provided!\n" );
    OSL_ENSURE( m_nStorageType == embed::StorageFormats::OFOPXML || !m_xOrigRelInfoStream.is(), "The Relations info makes sence only for OFOPXML format!\n" );
}

//-----------------------------------------------
OWriteStream_Impl::~OWriteStream_Impl()
{
    DisposeWrappers();

    if ( !m_aTempURL.isEmpty() )
    {
        KillFile( m_aTempURL, comphelper::getProcessComponentContext() );
        m_aTempURL = ::rtl::OUString();
    }

    CleanCacheStream();
}

//-----------------------------------------------
void OWriteStream_Impl::CleanCacheStream()
{
    if ( m_xCacheStream.is() )
    {
        try
        {
            uno::Reference< io::XInputStream > xInputCache = m_xCacheStream->getInputStream();
            if ( xInputCache.is() )
                xInputCache->closeInput();
        }
        catch( const uno::Exception& )
        {}

        try
        {
            uno::Reference< io::XOutputStream > xOutputCache = m_xCacheStream->getOutputStream();
            if ( xOutputCache.is() )
                xOutputCache->closeOutput();
        }
        catch( const uno::Exception& )
        {}

        m_xCacheStream = uno::Reference< io::XStream >();
        m_xCacheSeek = uno::Reference< io::XSeekable >();
    }
}

//-----------------------------------------------
void OWriteStream_Impl::AddLog( const ::rtl::OUString& aMessage )
{
    if ( !m_xLogRing.is() )
    {
        try
        {
            ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            if ( aContext.is() )
                m_xLogRing.set( aContext.getSingleton( "com.sun.star.logging.DocumentIOLogRing" ), uno::UNO_QUERY_THROW );
        }
        catch( const uno::Exception& )
        {
            // No log
        }
    }

    if ( m_xLogRing.is() )
        m_xLogRing->logString( aMessage );
}


//-----------------------------------------------
void OWriteStream_Impl::InsertIntoPackageFolder( const ::rtl::OUString& aName,
                                                  const uno::Reference< container::XNameContainer >& xParentPackageFolder )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    OSL_ENSURE( m_bFlushed, "This method must not be called for nonflushed streams!\n" );
    if ( m_bFlushed )
    {
        OSL_ENSURE( m_xPackageStream.is(), "An inserted stream is incomplete!\n" );
        uno::Reference< lang::XUnoTunnel > xTunnel( m_xPackageStream, uno::UNO_QUERY );
        if ( !xTunnel.is() )
            throw uno::RuntimeException(); // TODO

        xParentPackageFolder->insertByName( aName, uno::makeAny( xTunnel ) );

        m_bFlushed = sal_False;
        m_bHasInsertedStreamOptimization = sal_False;
    }
}
//-----------------------------------------------
sal_Bool OWriteStream_Impl::IsEncrypted()
{
    if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        return sal_False;

    if ( m_bForceEncrypted || m_bHasCachedEncryptionData )
        return sal_True;

    if ( !m_aTempURL.isEmpty() || m_xCacheStream.is() )
        return sal_False;

    GetStreamProperties();

    // the following value can not be cached since it can change after root commit
    sal_Bool bWasEncr = sal_False;
    uno::Reference< beans::XPropertySet > xPropSet( m_xPackageStream, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        uno::Any aValue = xPropSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WasEncrypted") ) );
        if ( !( aValue >>= bWasEncr ) )
        {
            OSL_FAIL( "The property WasEncrypted has wrong type!\n" );
        }
    }

    sal_Bool bToBeEncr = sal_False;
    for ( sal_Int32 nInd = 0; nInd < m_aProps.getLength(); nInd++ )
    {
        if ( m_aProps[nInd].Name == "Encrypted" )
        {
            if ( !( m_aProps[nInd].Value >>= bToBeEncr ) )
            {
                OSL_FAIL( "The property has wrong type!\n" );
            }
        }
    }

    // since a new key set to the package stream it should not be removed except the case when
    // the stream becomes nonencrypted
    uno::Sequence< beans::NamedValue > aKey;
    if ( bToBeEncr )
        GetEncryptionKeyProperty_Impl( xPropSet ) >>= aKey;

    // If the properties must be investigated the stream is either
    // was never changed or was changed, the parent was commited
    // and the stream was closed.
    // That means that if it is intended to use common storage key
    // it is already has no encryption but is marked to be stored
    // encrypted and the key is empty.
    if ( !bWasEncr && bToBeEncr && !aKey.getLength() )
    {
        // the stream is intended to use common storage password
        m_bUseCommonEncryption = sal_True;
        return sal_False;
    }
    else
        return bToBeEncr;
}

//-----------------------------------------------
void OWriteStream_Impl::SetDecrypted()
{
    OSL_ENSURE( m_nStorageType == embed::StorageFormats::PACKAGE, "The encryption is supported only for package storages!\n" );
    if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        throw uno::RuntimeException();

    GetStreamProperties();

    // let the stream be modified
    FillTempGetFileName();
    m_bHasDataToFlush = sal_True;

    // remove encryption
    m_bForceEncrypted = sal_False;
    m_bHasCachedEncryptionData = sal_False;
    m_aEncryptionData.clear();

    for ( sal_Int32 nInd = 0; nInd < m_aProps.getLength(); nInd++ )
    {
        if ( m_aProps[nInd].Name == "Encrypted" )
            m_aProps[nInd].Value <<= sal_False;
    }
}

//-----------------------------------------------
void OWriteStream_Impl::SetEncrypted( const ::comphelper::SequenceAsHashMap& aEncryptionData )
{
    OSL_ENSURE( m_nStorageType == embed::StorageFormats::PACKAGE, "The encryption is supported only for package storages!\n" );
    if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        throw uno::RuntimeException();

    if ( !aEncryptionData.size() )
        throw uno::RuntimeException();

    GetStreamProperties();

    // let the stream be modified
    FillTempGetFileName();
    m_bHasDataToFlush = sal_True;

    // introduce encryption info
    for ( sal_Int32 nInd = 0; nInd < m_aProps.getLength(); nInd++ )
    {
        if ( m_aProps[nInd].Name == "Encrypted" )
            m_aProps[nInd].Value <<= sal_True;
    }

    m_bUseCommonEncryption = sal_False; // very important to set it to false

    m_bHasCachedEncryptionData = sal_True;
    m_aEncryptionData = aEncryptionData;
}

//-----------------------------------------------
void OWriteStream_Impl::DisposeWrappers()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_pAntiImpl )
    {
        try {
            m_pAntiImpl->dispose();
        }
        catch ( const uno::RuntimeException& rRuntimeException )
        {
            AddLog( rRuntimeException.Message );
            AddLog( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(OSL_LOG_PREFIX "Quiet exception") ) );
        }

        m_pAntiImpl = NULL;
    }
    m_pParent = NULL;

    if ( !m_aInputStreamsList.empty() )
    {
        for ( InputStreamsList_Impl::iterator pStreamIter = m_aInputStreamsList.begin();
              pStreamIter != m_aInputStreamsList.end(); ++pStreamIter )
        {
            if ( (*pStreamIter) )
            {
                (*pStreamIter)->InternalDispose();
                (*pStreamIter) = NULL;
            }
        }

        m_aInputStreamsList.clear();
    }
}

//-----------------------------------------------
uno::Reference< lang::XMultiServiceFactory > OWriteStream_Impl::GetServiceFactory()
{
    if ( m_xFactory.is() )
        return m_xFactory;

    return ::comphelper::getProcessServiceFactory();
}

//-----------------------------------------------
::rtl::OUString OWriteStream_Impl::GetFilledTempFileIfNo( const uno::Reference< io::XInputStream >& xStream )
{
    if ( !m_aTempURL.getLength() )
    {
        ::rtl::OUString aTempURL = GetNewTempFileURL( GetServiceFactory() );

        try {
            if ( !aTempURL.isEmpty() && xStream.is() )
            {
                uno::Reference < ucb::XSimpleFileAccess2 > xTempAccess( ucb::SimpleFileAccess::create( ::comphelper::getProcessComponentContext() ) );

                uno::Reference< io::XOutputStream > xTempOutStream = xTempAccess->openFileWrite( aTempURL );
                if ( xTempOutStream.is() )
                {
                    // the current position of the original stream should be still OK, copy further
                    ::comphelper::OStorageHelper::CopyInputToOutput( xStream, xTempOutStream );
                    xTempOutStream->closeOutput();
                    xTempOutStream = uno::Reference< io::XOutputStream >();
                }
                else
                    throw io::IOException(); // TODO:
            }
        }
        catch( const packages::WrongPasswordException& rWrongPasswordException )
        {
            AddLog( rWrongPasswordException.Message );
            AddLog( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow") ) );

            KillFile( aTempURL, comphelper::getProcessComponentContext() );
            throw;
        }
        catch( const uno::Exception& rException )
        {
            AddLog( rException.Message );
            AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow") ) );

            KillFile( aTempURL, comphelper::getProcessComponentContext() );
        throw;
        }

        if ( !aTempURL.isEmpty() )
            CleanCacheStream();

        m_aTempURL = aTempURL;
    }

    return m_aTempURL;
}

//-----------------------------------------------
::rtl::OUString OWriteStream_Impl::FillTempGetFileName()
{
    // should try to create cache first, if the amount of contents is too big, the temp file should be taken
    if ( !m_xCacheStream.is() && m_aTempURL.isEmpty() )
    {
        uno::Reference< io::XInputStream > xOrigStream = m_xPackageStream->getDataStream();
        if ( !xOrigStream.is() )
        {
            // in case of new inserted package stream it is possible that input stream still was not set
            uno::Reference< io::XStream > xCacheStream = CreateMemoryStream( GetServiceFactory() );
            OSL_ENSURE( xCacheStream.is(), "If the stream can not be created an exception must be thrown!\n" );
            m_xCacheSeek.set( xCacheStream, uno::UNO_QUERY_THROW );
            m_xCacheStream = xCacheStream;
        }
        else
        {
            sal_Int32 nRead = 0;
            uno::Sequence< sal_Int8 > aData( MAX_STORCACHE_SIZE + 1 );
            nRead = xOrigStream->readBytes( aData, MAX_STORCACHE_SIZE + 1 );
            if ( aData.getLength() > nRead )
                aData.realloc( nRead );

            if ( nRead <= MAX_STORCACHE_SIZE )
            {
                uno::Reference< io::XStream > xCacheStream = CreateMemoryStream( GetServiceFactory() );
                OSL_ENSURE( xCacheStream.is(), "If the stream can not be created an exception must be thrown!\n" );

                if ( nRead )
                {
                    uno::Reference< io::XOutputStream > xOutStream( xCacheStream->getOutputStream(), uno::UNO_SET_THROW );
                    xOutStream->writeBytes( aData );
                }
                m_xCacheSeek.set( xCacheStream, uno::UNO_QUERY_THROW );
                m_xCacheStream = xCacheStream;
                m_xCacheSeek->seek( 0 );
            }
            else if ( m_aTempURL.isEmpty() )
            {
                m_aTempURL = GetNewTempFileURL( GetServiceFactory() );

                try {
                    if ( !m_aTempURL.isEmpty() )
                    {
                        uno::Reference < ucb::XSimpleFileAccess2 > xTempAccess( ucb::SimpleFileAccess::create( ::comphelper::getProcessComponentContext() ) );

                        uno::Reference< io::XOutputStream > xTempOutStream = xTempAccess->openFileWrite( m_aTempURL );
                        if ( xTempOutStream.is() )
                        {
                            // copy stream contents to the file
                            xTempOutStream->writeBytes( aData );

                            // the current position of the original stream should be still OK, copy further
                            ::comphelper::OStorageHelper::CopyInputToOutput( xOrigStream, xTempOutStream );
                            xTempOutStream->closeOutput();
                            xTempOutStream = uno::Reference< io::XOutputStream >();
                        }
                        else
                            throw io::IOException(); // TODO:
                    }
                }
                catch( const packages::WrongPasswordException& )
                {
                    KillFile( m_aTempURL, comphelper::getProcessComponentContext() );
                    m_aTempURL = ::rtl::OUString();

                    throw;
                }
                catch( const uno::Exception& )
                {
                    KillFile( m_aTempURL, comphelper::getProcessComponentContext() );
                    m_aTempURL = ::rtl::OUString();
                }
            }
        }
    }

    return m_aTempURL;
}

//-----------------------------------------------
uno::Reference< io::XStream > OWriteStream_Impl::GetTempFileAsStream()
{
    uno::Reference< io::XStream > xTempStream;

    if ( !m_xCacheStream.is() )
    {
        if ( m_aTempURL.isEmpty() )
            m_aTempURL = FillTempGetFileName();

        if ( !m_aTempURL.isEmpty() )
        {
            // the temporary file is not used if the cache is used
            uno::Reference < ucb::XSimpleFileAccess2 > xTempAccess( ucb::SimpleFileAccess::create( ::comphelper::getProcessComponentContext() ) );

            try
            {
                xTempStream = xTempAccess->openFileReadWrite( m_aTempURL );
            }
            catch( const uno::Exception& rException )
            {
                AddLog( rException.Message );
                AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Quiet exception" ) ) );
            }
        }
    }

    if ( m_xCacheStream.is() )
        xTempStream = m_xCacheStream;

    // the method must always return a stream
    // in case the stream can not be open
    // an exception should be thrown
    if ( !xTempStream.is() )
        throw io::IOException(); //TODO:

    return xTempStream;
}

//-----------------------------------------------
uno::Reference< io::XInputStream > OWriteStream_Impl::GetTempFileAsInputStream()
{
    uno::Reference< io::XInputStream > xInputStream;

    if ( !m_xCacheStream.is() )
    {
        if ( m_aTempURL.isEmpty() )
            m_aTempURL = FillTempGetFileName();

        if ( !m_aTempURL.isEmpty() )
        {
            // the temporary file is not used if the cache is used
            uno::Reference < ucb::XSimpleFileAccess2 > xTempAccess( ucb::SimpleFileAccess::create( ::comphelper::getProcessComponentContext() ) );

            try
            {
                xInputStream = xTempAccess->openFileRead( m_aTempURL );
            }
            catch( const uno::Exception& rException )
            {
                AddLog( rException.Message );
                AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Quiet exception" ) ) );
            }
        }
    }

    if ( m_xCacheStream.is() )
        xInputStream = m_xCacheStream->getInputStream();

    // the method must always return a stream
    // in case the stream can not be open
    // an exception should be thrown
    if ( !xInputStream.is() )
        throw io::IOException(); // TODO:

    return xInputStream;
}

// =================================================================================================

//-----------------------------------------------
void OWriteStream_Impl::InsertStreamDirectly( const uno::Reference< io::XInputStream >& xInStream,
                                              const uno::Sequence< beans::PropertyValue >& aProps )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    // this call can be made only during parent storage commit
    // the  parent storage is responsible for the correct handling
    // of deleted and renamed contents

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    if ( m_bHasDataToFlush )
        throw io::IOException();

    OSL_ENSURE( m_aTempURL.isEmpty() && !m_xCacheStream.is(), "The temporary must not exist!\n" );

    // use new file as current persistent representation
    // the new file will be removed after it's stream is closed
    m_xPackageStream->setDataStream( xInStream );

    // copy properties to the package stream
    uno::Reference< beans::XPropertySet > xPropertySet( m_xPackageStream, uno::UNO_QUERY );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    // The storage-package communication has a problem
    // the storage caches properties, thus if the package changes one of them itself
    // the storage does not know about it

    // Depending from MediaType value the package can change the compressed property itself
    // Thus if Compressed property is provided it must be set as the latest one
    sal_Bool bCompressedIsSet = sal_False;
    sal_Bool bCompressed = sal_False;
    ::rtl::OUString aComprPropName( RTL_CONSTASCII_USTRINGPARAM( "Compressed" ) );
    ::rtl::OUString aMedTypePropName( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) );
    for ( sal_Int32 nInd = 0; nInd < aProps.getLength(); nInd++ )
    {
        if ( aProps[nInd].Name.equals( aComprPropName ) )
        {
            bCompressedIsSet = sal_True;
            aProps[nInd].Value >>= bCompressed;
        }
        else if ( ( m_nStorageType == embed::StorageFormats::OFOPXML || m_nStorageType == embed::StorageFormats::PACKAGE )
               && aProps[nInd].Name.equals( aMedTypePropName ) )
        {
            xPropertySet->setPropertyValue( aProps[nInd].Name, aProps[nInd].Value );
        }
        else if ( m_nStorageType == embed::StorageFormats::PACKAGE && aProps[nInd].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "UseCommonStoragePasswordEncryption" ) ) )
            aProps[nInd].Value >>= m_bUseCommonEncryption;
        else
            throw lang::IllegalArgumentException();

        // if there are cached properties update them
        if ( aProps[nInd].Name.equals( aMedTypePropName ) || aProps[nInd].Name.equals( aComprPropName ) )
            for ( sal_Int32 nMemInd = 0; nMemInd < m_aProps.getLength(); nMemInd++ )
            {
                if ( aProps[nInd].Name.equals( m_aProps[nMemInd].Name ) )
                    m_aProps[nMemInd].Value = aProps[nInd].Value;
            }
    }

    if ( bCompressedIsSet )
    {
        xPropertySet->setPropertyValue( aComprPropName, uno::makeAny( (sal_Bool)bCompressed ) );
        m_bCompressedSetExplicit = sal_True;
    }

    if ( m_bUseCommonEncryption )
    {
        if ( m_nStorageType != embed::StorageFormats::PACKAGE )
            throw uno::RuntimeException();

        // set to be encrypted but do not use encryption key
        xPropertySet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( STORAGE_ENCRYPTION_KEYS_PROPERTY ) ),
                                        uno::makeAny( uno::Sequence< beans::NamedValue >() ) );
        xPropertySet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Encrypted") ),
                                        uno::makeAny( sal_True ) );
    }

    // the stream should be free soon, after package is stored
    m_bHasDataToFlush = sal_False;
    m_bFlushed = sal_True; // will allow to use transaction on stream level if will need it
    m_bHasInsertedStreamOptimization = sal_True;
}

//-----------------------------------------------
void OWriteStream_Impl::Commit()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    if ( !m_bHasDataToFlush )
        return;

    uno::Reference< packages::XDataSinkEncrSupport > xNewPackageStream;
    uno::Sequence< uno::Any > aSeq( 1 );
    aSeq[0] <<= sal_False;

    if ( m_xCacheStream.is() )
    {
        if ( m_pAntiImpl )
            m_pAntiImpl->DeInit();

        uno::Reference< io::XInputStream > xInStream( m_xCacheStream->getInputStream(), uno::UNO_SET_THROW );

        xNewPackageStream = uno::Reference< packages::XDataSinkEncrSupport >(
                                                        m_xPackage->createInstanceWithArguments( aSeq ),
                                                        uno::UNO_QUERY_THROW );

        xNewPackageStream->setDataStream( xInStream );

        m_xCacheStream = uno::Reference< io::XStream >();
        m_xCacheSeek = uno::Reference< io::XSeekable >();

    }
    else if ( !m_aTempURL.isEmpty() )
    {
        if ( m_pAntiImpl )
            m_pAntiImpl->DeInit();

        uno::Reference< io::XInputStream > xInStream;
        try
        {
            xInStream.set( static_cast< io::XInputStream* >( new OSelfTerminateFileStream( GetServiceFactory(), m_aTempURL ) ), uno::UNO_QUERY );
        }
        catch( const uno::Exception& )
        {
        }

        if ( !xInStream.is() )
            throw io::IOException();

        xNewPackageStream = uno::Reference< packages::XDataSinkEncrSupport >(
                                                        m_xPackage->createInstanceWithArguments( aSeq ),
                                                        uno::UNO_QUERY_THROW );

        // TODO/NEW: Let the temporary file be removed after commit
        xNewPackageStream->setDataStream( xInStream );
        m_aTempURL = ::rtl::OUString();
    }
    else // if ( m_bHasInsertedStreamOptimization )
    {
        // if the optimization is used the stream can be accessed directly
        xNewPackageStream = m_xPackageStream;
    }

    // copy properties to the package stream
    uno::Reference< beans::XPropertySet > xPropertySet( xNewPackageStream, uno::UNO_QUERY );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    for ( sal_Int32 nInd = 0; nInd < m_aProps.getLength(); nInd++ )
    {
        if ( m_aProps[nInd].Name == "Size" )
        {
            if ( m_pAntiImpl && !m_bHasInsertedStreamOptimization && m_pAntiImpl->m_xSeekable.is() )
            {
                m_aProps[nInd].Value <<= m_pAntiImpl->m_xSeekable->getLength();
                xPropertySet->setPropertyValue( m_aProps[nInd].Name, m_aProps[nInd].Value );
            }
        }
        else
            xPropertySet->setPropertyValue( m_aProps[nInd].Name, m_aProps[nInd].Value );
    }

    if ( m_bUseCommonEncryption )
    {
        if ( m_nStorageType != embed::StorageFormats::PACKAGE )
            throw uno::RuntimeException();

        // set to be encrypted but do not use encryption key
        xPropertySet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( STORAGE_ENCRYPTION_KEYS_PROPERTY ) ),
                                        uno::makeAny( uno::Sequence< beans::NamedValue >() ) );
        xPropertySet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Encrypted") ),
                                        uno::makeAny( sal_True ) );
    }
    else if ( m_bHasCachedEncryptionData )
    {
        if ( m_nStorageType != embed::StorageFormats::PACKAGE )
            throw uno::RuntimeException();

        xPropertySet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( STORAGE_ENCRYPTION_KEYS_PROPERTY ) ),
                                        uno::makeAny( m_aEncryptionData.getAsConstNamedValueList() ) );
    }

    // the stream should be free soon, after package is stored
    m_xPackageStream = xNewPackageStream;
    m_bHasDataToFlush = sal_False;
    m_bFlushed = sal_True; // will allow to use transaction on stream level if will need it
}

//-----------------------------------------------
void OWriteStream_Impl::Revert()
{
    // can be called only from parent storage
    // means complete reload of the stream

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    if ( !m_bHasDataToFlush )
        return; // nothing to do

    OSL_ENSURE( !m_aTempURL.isEmpty() || m_xCacheStream.is(), "The temporary must exist!\n" );

    if ( m_xCacheStream.is() )
    {
        m_xCacheStream = uno::Reference< io::XStream >();
        m_xCacheSeek = uno::Reference< io::XSeekable >();
    }

    if ( !m_aTempURL.isEmpty() )
    {
        KillFile( m_aTempURL, comphelper::getProcessComponentContext() );
        m_aTempURL = ::rtl::OUString();
    }

    m_aProps.realloc( 0 );

    m_bHasDataToFlush = sal_False;

    m_bUseCommonEncryption = sal_True;
    m_bHasCachedEncryptionData = sal_False;
    m_aEncryptionData.clear();

    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        // currently the relations storage is changed only on commit
        m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
        m_aNewRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
        if ( m_xOrigRelInfoStream.is() )
        {
            // the original stream is still here, that means that it was not parsed
            m_aOrigRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
            m_nRelInfoStatus = RELINFO_NO_INIT;
        }
        else
        {
            // the original stream was aready parsed
            if ( !m_bOrigRelInfoBroken )
                m_nRelInfoStatus = RELINFO_READ;
            else
                m_nRelInfoStatus = RELINFO_BROKEN;
        }
    }
}

//-----------------------------------------------
uno::Sequence< beans::PropertyValue > OWriteStream_Impl::GetStreamProperties()
{
    if ( !m_aProps.getLength() )
        m_aProps = ReadPackageStreamProperties();

    return m_aProps;
}

//-----------------------------------------------
uno::Sequence< beans::PropertyValue > OWriteStream_Impl::InsertOwnProps(
                                                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                                                    sal_Bool bUseCommonEncryption )
{
    uno::Sequence< beans::PropertyValue > aResult( aProps );
    sal_Int32 nLen = aResult.getLength();

    if ( m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        for ( sal_Int32 nInd = 0; nInd < nLen; nInd++ )
            if ( aResult[nInd].Name == "UseCommonStoragePasswordEncryption" )
            {
                aResult[nInd].Value <<= bUseCommonEncryption;
                return aResult;
            }

        aResult.realloc( ++nLen );
        aResult[nLen - 1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseCommonStoragePasswordEncryption") );
        aResult[nLen - 1].Value <<= bUseCommonEncryption;
    }
    else if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        ReadRelInfoIfNecessary();

        uno::Any aValue;
        if ( m_nRelInfoStatus == RELINFO_READ )
            aValue <<= m_aOrigRelInfo;
        else if ( m_nRelInfoStatus == RELINFO_CHANGED_STREAM_READ || m_nRelInfoStatus == RELINFO_CHANGED )
            aValue <<= m_aNewRelInfo;
        else // m_nRelInfoStatus == RELINFO_CHANGED_BROKEN || m_nRelInfoStatus == RELINFO_BROKEN
            throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Wrong relinfo stream!" ) ),
                                    uno::Reference< uno::XInterface >() );

        for ( sal_Int32 nInd = 0; nInd < nLen; nInd++ )
            if ( aResult[nInd].Name == "RelationsInfo" )
            {
                aResult[nInd].Value = aValue;
                return aResult;
            }

        aResult.realloc( ++nLen );
        aResult[nLen - 1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RelationsInfo") );
        aResult[nLen - 1].Value = aValue;
    }

    return aResult;
}

//-----------------------------------------------
sal_Bool OWriteStream_Impl::IsTransacted()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;
    return ( m_pAntiImpl && m_pAntiImpl->m_bTransacted );
}

void OWriteStream_Impl::ReadRelInfoIfNecessary()
{
    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        return;

    if ( m_nRelInfoStatus == RELINFO_NO_INIT )
    {
        try
        {
            // Init from original stream
            if ( m_xOrigRelInfoStream.is() )
                m_aOrigRelInfo = ::comphelper::OFOPXMLHelper::ReadRelationsInfoSequence(
                                        m_xOrigRelInfoStream,
                                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_rels/*.rels" ) ),
                                        comphelper::getComponentContext(m_xFactory) );

            // in case of success the stream must be thrown away, that means that the OrigRelInfo is initialized
            // the reason for this is that the original stream might not be seekable ( at the same time the new
            // provided stream must be seekable ), so it must be read only once
            m_xOrigRelInfoStream = uno::Reference< io::XInputStream >();
            m_nRelInfoStatus = RELINFO_READ;
        }
        catch( const uno::Exception& rException )
        {
            AddLog( rException.Message );
            AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Quiet exception" ) ) );

            m_nRelInfoStatus = RELINFO_BROKEN;
            m_bOrigRelInfoBroken = sal_True;
        }
    }
    else if ( m_nRelInfoStatus == RELINFO_CHANGED_STREAM )
    {
        // Init from the new stream
        try
        {
            if ( m_xNewRelInfoStream.is() )
                m_aNewRelInfo = ::comphelper::OFOPXMLHelper::ReadRelationsInfoSequence(
                                        m_xNewRelInfoStream,
                                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_rels/*.rels" ) ),
                                        comphelper::getComponentContext(m_xFactory) );

            m_nRelInfoStatus = RELINFO_CHANGED_STREAM_READ;
        }
        catch( const uno::Exception& )
        {
            m_nRelInfoStatus = RELINFO_CHANGED_BROKEN;
        }
    }
}

//-----------------------------------------------
uno::Sequence< beans::PropertyValue > OWriteStream_Impl::ReadPackageStreamProperties()
{
    sal_Int32 nPropNum = 0;
    if ( m_nStorageType == embed::StorageFormats::ZIP )
        nPropNum = 2;
    else if ( m_nStorageType == embed::StorageFormats::OFOPXML )
        nPropNum = 3;
    else if ( m_nStorageType == embed::StorageFormats::PACKAGE )
        nPropNum = 4;
    uno::Sequence< beans::PropertyValue > aResult( nPropNum );

    // The "Compressed" property must be set after "MediaType" property,
    // since the setting of the last one can change the value of the first one

    if ( m_nStorageType == embed::StorageFormats::OFOPXML || m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        aResult[0].Name = "MediaType";
        aResult[1].Name = "Compressed";
        aResult[2].Name = "Size";

        if ( m_nStorageType == embed::StorageFormats::PACKAGE )
            aResult[3].Name = "Encrypted";
    }
    else
    {
        aResult[0].Name = "Compressed";
        aResult[1].Name = "Size";
    }

    // TODO: may be also raw stream should be marked

    uno::Reference< beans::XPropertySet > xPropSet( m_xPackageStream, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        for ( sal_Int32 nInd = 0; nInd < aResult.getLength(); nInd++ )
        {
            try {
                aResult[nInd].Value = xPropSet->getPropertyValue( aResult[nInd].Name );
            }
            catch( const uno::Exception& rException )
            {
                AddLog( rException.Message );
                AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Quiet exception" ) ) );

                OSL_FAIL( "A property can't be retrieved!\n" );
            }
        }
    }
    else
    {
        OSL_FAIL( "Can not get properties from a package stream!\n" );
        throw uno::RuntimeException();
    }

    return aResult;
}

//-----------------------------------------------
void OWriteStream_Impl::CopyInternallyTo_Impl( const uno::Reference< io::XStream >& xDestStream,
                                                const ::comphelper::SequenceAsHashMap& aEncryptionData )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    OSL_ENSURE( !m_bUseCommonEncryption, "The stream can not be encrypted!" );

    if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        throw packages::NoEncryptionException();

    if ( m_pAntiImpl )
    {
        m_pAntiImpl->CopyToStreamInternally_Impl( xDestStream );
    }
    else
    {
        uno::Reference< io::XStream > xOwnStream = GetStream( embed::ElementModes::READ, aEncryptionData, sal_False );
        if ( !xOwnStream.is() )
            throw io::IOException(); // TODO

        OStorage_Impl::completeStorageStreamCopy_Impl( xOwnStream, xDestStream, m_nStorageType, GetAllRelationshipsIfAny() );
    }

    uno::Reference< embed::XEncryptionProtectedSource2 > xEncr( xDestStream, uno::UNO_QUERY );
    if ( xEncr.is() )
        xEncr->setEncryptionData( aEncryptionData.getAsConstNamedValueList() );
}

//-----------------------------------------------
uno::Sequence< uno::Sequence< beans::StringPair > > OWriteStream_Impl::GetAllRelationshipsIfAny()
{
    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        return uno::Sequence< uno::Sequence< beans::StringPair > >();

    ReadRelInfoIfNecessary();

    if ( m_nRelInfoStatus == RELINFO_READ )
        return m_aOrigRelInfo;
    else if ( m_nRelInfoStatus == RELINFO_CHANGED_STREAM_READ || m_nRelInfoStatus == RELINFO_CHANGED )
        return m_aNewRelInfo;
    else // m_nRelInfoStatus == RELINFO_CHANGED_BROKEN || m_nRelInfoStatus == RELINFO_BROKEN
            throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Wrong relinfo stream!" ) ),
                                    uno::Reference< uno::XInterface >() );
}

//-----------------------------------------------
void OWriteStream_Impl::CopyInternallyTo_Impl( const uno::Reference< io::XStream >& xDestStream )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    if ( m_pAntiImpl )
    {
        m_pAntiImpl->CopyToStreamInternally_Impl( xDestStream );
    }
    else
    {
        uno::Reference< io::XStream > xOwnStream = GetStream( embed::ElementModes::READ, sal_False );
        if ( !xOwnStream.is() )
            throw io::IOException(); // TODO

        OStorage_Impl::completeStorageStreamCopy_Impl( xOwnStream, xDestStream, m_nStorageType, GetAllRelationshipsIfAny() );
    }
}

//-----------------------------------------------
uno::Reference< io::XStream > OWriteStream_Impl::GetStream( sal_Int32 nStreamMode, const ::comphelper::SequenceAsHashMap& aEncryptionData, sal_Bool bHierarchyAccess )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    if ( m_pAntiImpl )
        throw io::IOException(); // TODO:

    if ( !IsEncrypted() )
        throw packages::NoEncryptionException();

    uno::Reference< io::XStream > xResultStream;

    uno::Reference< beans::XPropertySet > xPropertySet( m_xPackageStream, uno::UNO_QUERY );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    if ( m_bHasCachedEncryptionData )
    {
        if ( !::package::PackageEncryptionDatasEqual( m_aEncryptionData, aEncryptionData ) )
            throw packages::WrongPasswordException();

        // the correct key must be set already
        xResultStream = GetStream_Impl( nStreamMode, bHierarchyAccess );
    }
    else
    {
        SetEncryptionKeyProperty_Impl( xPropertySet, aEncryptionData.getAsConstNamedValueList() );

        try {
            xResultStream = GetStream_Impl( nStreamMode, bHierarchyAccess );

            m_bUseCommonEncryption = sal_False; // very important to set it to false
            m_bHasCachedEncryptionData = sal_True;
            m_aEncryptionData = aEncryptionData;
        }
        catch( const packages::WrongPasswordException& rWrongPasswordException )
        {
            SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
            AddLog( rWrongPasswordException.Message );
            AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow" ) ) );
            throw;
        }
        catch ( const uno::Exception& rException )
        {
            AddLog( rException.Message );
            AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Quiet exception" ) ) );

            OSL_FAIL( "Can't write encryption related properties!\n" );
            SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
            throw io::IOException(); // TODO:
        }
    }

    OSL_ENSURE( xResultStream.is(), "In case stream can not be retrieved an exception must be thrown!\n" );

    return xResultStream;
}

//-----------------------------------------------
uno::Reference< io::XStream > OWriteStream_Impl::GetStream( sal_Int32 nStreamMode, sal_Bool bHierarchyAccess )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    if ( m_pAntiImpl )
        throw io::IOException(); // TODO:

    uno::Reference< io::XStream > xResultStream;

    if ( IsEncrypted() )
    {
        ::comphelper::SequenceAsHashMap aGlobalEncryptionData;
        try
        {
            aGlobalEncryptionData = GetCommonRootEncryptionData();
        }
        catch( const packages::NoEncryptionException& rNoEncryptionException )
        {
            AddLog( rNoEncryptionException.Message );
            AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow" ) ) );

            throw packages::WrongPasswordException();
        }

        xResultStream = GetStream( nStreamMode, aGlobalEncryptionData, bHierarchyAccess );
    }
    else
        xResultStream = GetStream_Impl( nStreamMode, bHierarchyAccess );

    return xResultStream;
}

//-----------------------------------------------
uno::Reference< io::XStream > OWriteStream_Impl::GetStream_Impl( sal_Int32 nStreamMode, sal_Bool bHierarchyAccess )
{
    // private method, no mutex is used
    GetStreamProperties();

    // TODO/LATER: this info might be read later, on demand in future
    ReadRelInfoIfNecessary();

    if ( ( nStreamMode & embed::ElementModes::READWRITE ) == embed::ElementModes::READ )
    {
        uno::Reference< io::XInputStream > xInStream;
        if ( m_xCacheStream.is() || !m_aTempURL.isEmpty() )
            xInStream = GetTempFileAsInputStream(); //TODO:
        else
            xInStream = m_xPackageStream->getDataStream();

        // The stream does not exist in the storage
        if ( !xInStream.is() )
            throw io::IOException();

        OInputCompStream* pStream = new OInputCompStream( *this, xInStream, InsertOwnProps( m_aProps, m_bUseCommonEncryption ), m_nStorageType );
        uno::Reference< io::XStream > xCompStream(
                        static_cast< ::cppu::OWeakObject* >( pStream ),
                        uno::UNO_QUERY );
        OSL_ENSURE( xCompStream.is(),
                    "OInputCompStream MUST provide XStream interfaces!\n" );

        m_aInputStreamsList.push_back( pStream );
        return xCompStream;
    }
    else if ( ( nStreamMode & embed::ElementModes::READWRITE ) == embed::ElementModes::SEEKABLEREAD )
    {
        if ( !m_xCacheStream.is() && m_aTempURL.isEmpty() && !( m_xPackageStream->getDataStream().is() ) )
        {
            // The stream does not exist in the storage
            throw io::IOException();
        }

        uno::Reference< io::XInputStream > xInStream;

        xInStream = GetTempFileAsInputStream(); //TODO:

        if ( !xInStream.is() )
            throw io::IOException();

        OInputSeekStream* pStream = new OInputSeekStream( *this, xInStream, InsertOwnProps( m_aProps, m_bUseCommonEncryption ), m_nStorageType );
        uno::Reference< io::XStream > xSeekStream(
                        static_cast< ::cppu::OWeakObject* >( pStream ),
                        uno::UNO_QUERY );
        OSL_ENSURE( xSeekStream.is(),
                    "OInputSeekStream MUST provide XStream interfaces!\n" );

        m_aInputStreamsList.push_back( pStream );
        return xSeekStream;
    }
    else if ( ( nStreamMode & embed::ElementModes::WRITE ) == embed::ElementModes::WRITE )
    {
        if ( !m_aInputStreamsList.empty() )
            throw io::IOException(); // TODO:

        uno::Reference< io::XStream > xStream;
        if ( ( nStreamMode & embed::ElementModes::TRUNCATE ) == embed::ElementModes::TRUNCATE )
        {
            if ( !m_aTempURL.isEmpty() )
            {
                KillFile( m_aTempURL, comphelper::getProcessComponentContext() );
                m_aTempURL = ::rtl::OUString();
            }
            if ( m_xCacheStream.is() )
                CleanCacheStream();

            m_bHasDataToFlush = sal_True;

            // this call is triggered by the parent and it will recognize the change of the state
            if ( m_pParent )
                m_pParent->m_bIsModified = sal_True;

            xStream = CreateMemoryStream( GetServiceFactory() );
            m_xCacheSeek.set( xStream, uno::UNO_QUERY_THROW );
            m_xCacheStream = xStream;
        }
        else if ( !m_bHasInsertedStreamOptimization )
        {
            if ( m_aTempURL.isEmpty() && !m_xCacheStream.is() && !( m_xPackageStream->getDataStream().is() ) )
            {
                // The stream does not exist in the storage
                m_bHasDataToFlush = sal_True;

                // this call is triggered by the parent and it will recognize the change of the state
                if ( m_pParent )
                    m_pParent->m_bIsModified = sal_True;
                xStream = GetTempFileAsStream();
            }

            // if the stream exists the temporary file is created on demand
            // xStream = GetTempFileAsStream();
        }

        if ( !xStream.is() )
            m_pAntiImpl = new OWriteStream( this, bHierarchyAccess );
        else
            m_pAntiImpl = new OWriteStream( this, xStream, bHierarchyAccess );

        uno::Reference< io::XStream > xWriteStream =
                                uno::Reference< io::XStream >( static_cast< ::cppu::OWeakObject* >( m_pAntiImpl ),
                                                                uno::UNO_QUERY );

        OSL_ENSURE( xWriteStream.is(), "OWriteStream MUST implement XStream && XComponent interfaces!\n" );

        return xWriteStream;
    }

    throw lang::IllegalArgumentException(); // TODO
}

//-----------------------------------------------
uno::Reference< io::XInputStream > OWriteStream_Impl::GetPlainRawInStream()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    // this method is used only internally, this stream object should not go outside of this implementation
    // if ( m_pAntiImpl )
    //  throw io::IOException(); // TODO:

    return m_xPackageStream->getPlainRawStream();
}

//-----------------------------------------------
uno::Reference< io::XInputStream > OWriteStream_Impl::GetRawInStream()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    if ( m_pAntiImpl )
        throw io::IOException(); // TODO:

    OSL_ENSURE( IsEncrypted(), "Impossible to get raw representation for nonencrypted stream!\n" );
    if ( !IsEncrypted() )
        throw packages::NoEncryptionException();

    return m_xPackageStream->getRawStream();
}

//-----------------------------------------------
::comphelper::SequenceAsHashMap OWriteStream_Impl::GetCommonRootEncryptionData()
    throw ( packages::NoEncryptionException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    if ( m_nStorageType != embed::StorageFormats::PACKAGE || !m_pParent )
        throw packages::NoEncryptionException();

    return m_pParent->GetCommonRootEncryptionData();
}

//-----------------------------------------------
void OWriteStream_Impl::InputStreamDisposed( OInputCompStream* pStream )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    m_aInputStreamsList.remove( pStream );
}

//-----------------------------------------------
void OWriteStream_Impl::CreateReadonlyCopyBasedOnData( const uno::Reference< io::XInputStream >& xDataToCopy, const uno::Sequence< beans::PropertyValue >& aProps, sal_Bool, uno::Reference< io::XStream >& xTargetStream )
{
    uno::Reference < io::XStream > xTempFile;
    if ( !xTargetStream.is() )
        xTempFile = uno::Reference < io::XStream >(
            io::TempFile::create(comphelper::getComponentContext(m_xFactory)),
            uno::UNO_QUERY );
    else
        xTempFile = xTargetStream;

    uno::Reference < io::XSeekable > xTempSeek( xTempFile, uno::UNO_QUERY );
    if ( !xTempSeek.is() )
        throw uno::RuntimeException(); // TODO

    uno::Reference < io::XOutputStream > xTempOut = xTempFile->getOutputStream();
    if ( !xTempOut.is() )
        throw uno::RuntimeException();

    if ( xDataToCopy.is() )
        ::comphelper::OStorageHelper::CopyInputToOutput( xDataToCopy, xTempOut );

    xTempOut->closeOutput();
    xTempSeek->seek( 0 );

    uno::Reference< io::XInputStream > xInStream = xTempFile->getInputStream();
    if ( !xInStream.is() )
        throw io::IOException();

    // TODO: remember last state of m_bUseCommonEncryption
    if ( !xTargetStream.is() )
        xTargetStream = uno::Reference< io::XStream > (
            static_cast< ::cppu::OWeakObject* >(
                new OInputSeekStream( xInStream, InsertOwnProps( aProps, m_bUseCommonEncryption ), m_nStorageType ) ),
            uno::UNO_QUERY_THROW );
}

//-----------------------------------------------
void OWriteStream_Impl::GetCopyOfLastCommit( uno::Reference< io::XStream >& xTargetStream )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    OSL_ENSURE( m_xPackageStream.is(), "The source stream for copying is incomplete!\n" );
    if ( !m_xPackageStream.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XInputStream > xDataToCopy;
    if ( IsEncrypted() )
    {
        // an encrypted stream must contain input stream
        ::comphelper::SequenceAsHashMap aGlobalEncryptionData;
        try
        {
            aGlobalEncryptionData = GetCommonRootEncryptionData();
        }
        catch( const packages::NoEncryptionException& rNoEncryptionException )
        {
            AddLog( rNoEncryptionException.Message );
            AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "No Element" ) ) );

            throw packages::WrongPasswordException();
        }

        GetCopyOfLastCommit( xTargetStream, aGlobalEncryptionData );
    }
    else
    {
        xDataToCopy = m_xPackageStream->getDataStream();

        // in case of new inserted package stream it is possible that input stream still was not set
        GetStreamProperties();

        CreateReadonlyCopyBasedOnData( xDataToCopy, m_aProps, m_bUseCommonEncryption, xTargetStream );
    }
}

//-----------------------------------------------
void OWriteStream_Impl::GetCopyOfLastCommit( uno::Reference< io::XStream >& xTargetStream, const ::comphelper::SequenceAsHashMap& aEncryptionData )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    OSL_ENSURE( m_xPackageStream.is(), "The source stream for copying is incomplete!\n" );
    if ( !m_xPackageStream.is() )
        throw uno::RuntimeException();

    if ( !IsEncrypted() )
        throw packages::NoEncryptionException();

    uno::Reference< io::XInputStream > xDataToCopy;

    if ( m_bHasCachedEncryptionData )
    {
        // TODO: introduce last commited cashed password information and use it here
        // that means "use common pass" also should be remembered on flash
        uno::Sequence< beans::NamedValue > aKey = aEncryptionData.getAsConstNamedValueList();

        uno::Reference< beans::XPropertySet > xProps( m_xPackageStream, uno::UNO_QUERY );
        if ( !xProps.is() )
            throw uno::RuntimeException();

        sal_Bool bEncr = sal_False;
        xProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Encrypted") ) ) >>= bEncr;
        if ( !bEncr )
            throw packages::NoEncryptionException();

        uno::Sequence< beans::NamedValue > aPackKey;
        xProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( STORAGE_ENCRYPTION_KEYS_PROPERTY ) ) ) >>= aPackKey;
        if ( !SequencesEqual( aKey, aPackKey ) )
            throw packages::WrongPasswordException();

        // the correct key must be set already
        xDataToCopy = m_xPackageStream->getDataStream();
    }
    else
    {
        uno::Reference< beans::XPropertySet > xPropertySet( m_xPackageStream, uno::UNO_QUERY );
        SetEncryptionKeyProperty_Impl( xPropertySet, aEncryptionData.getAsConstNamedValueList() );

        try {
            xDataToCopy = m_xPackageStream->getDataStream();

            if ( !xDataToCopy.is() )
            {
                OSL_FAIL( "Encrypted ZipStream must already have input stream inside!\n" );
                SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
            }
        }
        catch( const uno::Exception& rException )
        {
            OSL_FAIL( "Can't open encrypted stream!\n" );
            SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
            AddLog( rException.Message );
            AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow" ) ) );
            throw;
        }

        SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
    }

    // in case of new inserted package stream it is possible that input stream still was not set
    GetStreamProperties();

    CreateReadonlyCopyBasedOnData( xDataToCopy, m_aProps, m_bUseCommonEncryption, xTargetStream );
}

//-----------------------------------------------
void OWriteStream_Impl::CommitStreamRelInfo( const uno::Reference< embed::XStorage >& xRelStorage, const ::rtl::OUString& aOrigStreamName, const ::rtl::OUString& aNewStreamName )
{
    // at this point of time the old stream must be already cleaned
    OSL_ENSURE( m_nStorageType == embed::StorageFormats::OFOPXML, "The method should be used only with OFOPXML format!\n" );

    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        OSL_ENSURE( !aOrigStreamName.isEmpty() && !aNewStreamName.isEmpty() && xRelStorage.is(),
                    "Wrong relation persistence information is provided!\n" );

        if ( !xRelStorage.is() || aOrigStreamName.isEmpty() || aNewStreamName.isEmpty() )
            throw uno::RuntimeException();

        if ( m_nRelInfoStatus == RELINFO_BROKEN || m_nRelInfoStatus == RELINFO_CHANGED_BROKEN )
            throw io::IOException(); // TODO:

        ::rtl::OUString aOrigRelStreamName = aOrigStreamName;
        aOrigRelStreamName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".rels" ) );

        ::rtl::OUString aNewRelStreamName = aNewStreamName;
        aNewRelStreamName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".rels" ) );

        sal_Bool bRenamed = !aOrigRelStreamName.equals( aNewRelStreamName );
        if ( m_nRelInfoStatus == RELINFO_CHANGED
          || m_nRelInfoStatus == RELINFO_CHANGED_STREAM_READ
          || m_nRelInfoStatus == RELINFO_CHANGED_STREAM )
        {
            if ( bRenamed && xRelStorage->hasByName( aOrigRelStreamName ) )
                xRelStorage->removeElement( aOrigRelStreamName );

            if ( m_nRelInfoStatus == RELINFO_CHANGED )
            {
                if ( m_aNewRelInfo.getLength() )
                {
                    uno::Reference< io::XStream > xRelsStream =
                        xRelStorage->openStreamElement( aNewRelStreamName,
                                                          embed::ElementModes::TRUNCATE | embed::ElementModes::READWRITE );

                    uno::Reference< io::XOutputStream > xOutStream = xRelsStream->getOutputStream();
                    if ( !xOutStream.is() )
                        throw uno::RuntimeException();

                    ::comphelper::OFOPXMLHelper::WriteRelationsInfoSequence( xOutStream, m_aNewRelInfo, comphelper::getComponentContext(m_xFactory) );

                    // set the mediatype
                    uno::Reference< beans::XPropertySet > xPropSet( xRelsStream, uno::UNO_QUERY_THROW );
                    xPropSet->setPropertyValue(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
                        uno::makeAny( ::rtl::OUString(
                             RTL_CONSTASCII_USTRINGPARAM( "application/vnd.openxmlformats-package.relationships+xml" ) ) ) );

                    m_nRelInfoStatus = RELINFO_READ;
                }
            }
            else if ( m_nRelInfoStatus == RELINFO_CHANGED_STREAM_READ
                      || m_nRelInfoStatus == RELINFO_CHANGED_STREAM )
            {
                uno::Reference< io::XStream > xRelsStream =
                    xRelStorage->openStreamElement( aNewRelStreamName,
                                                        embed::ElementModes::TRUNCATE | embed::ElementModes::READWRITE );

                uno::Reference< io::XOutputStream > xOutputStream = xRelsStream->getOutputStream();
                if ( !xOutputStream.is() )
                    throw uno::RuntimeException();

                uno::Reference< io::XSeekable > xSeek( m_xNewRelInfoStream, uno::UNO_QUERY_THROW );
                xSeek->seek( 0 );
                ::comphelper::OStorageHelper::CopyInputToOutput( m_xNewRelInfoStream, xOutputStream );
                xSeek->seek( 0 );

                // set the mediatype
                uno::Reference< beans::XPropertySet > xPropSet( xRelsStream, uno::UNO_QUERY_THROW );
                xPropSet->setPropertyValue(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
                    uno::makeAny( ::rtl::OUString(
                         RTL_CONSTASCII_USTRINGPARAM( "application/vnd.openxmlformats-package.relationships+xml" ) ) ) );

                  if ( m_nRelInfoStatus == RELINFO_CHANGED_STREAM )
                    m_nRelInfoStatus = RELINFO_NO_INIT;
                else
                {
                    // the information is already parsed and the stream is stored, no need in temporary stream any more
                    m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
                    m_nRelInfoStatus = RELINFO_READ;
                }
            }

            // the original stream makes no sence after this step
            m_xOrigRelInfoStream = m_xNewRelInfoStream;
            m_aOrigRelInfo = m_aNewRelInfo;
            m_bOrigRelInfoBroken = sal_False;
            m_aNewRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
            m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
        }
        else
        {
            // the stream is not changed but it might be renamed
            if ( bRenamed && xRelStorage->hasByName( aOrigRelStreamName ) )
                xRelStorage->renameElement( aOrigRelStreamName, aNewRelStreamName );
        }
    }
}

//===============================================
// OWriteStream implementation
//===============================================

//-----------------------------------------------
OWriteStream::OWriteStream( OWriteStream_Impl* pImpl, sal_Bool bTransacted )
: m_pImpl( pImpl )
, m_bInStreamDisconnected( sal_False )
, m_bInitOnDemand( sal_True )
, m_nInitPosition( 0 )
, m_bTransacted( bTransacted )
{
    OSL_ENSURE( pImpl, "No base implementation!\n" );
    OSL_ENSURE( m_pImpl->m_rMutexRef.Is(), "No mutex!\n" );

    if ( !m_pImpl || !m_pImpl->m_rMutexRef.Is() )
        throw uno::RuntimeException(); // just a disaster

    m_pData = new WSInternalData_Impl( pImpl->m_rMutexRef, m_pImpl->m_nStorageType );
}

//-----------------------------------------------
OWriteStream::OWriteStream( OWriteStream_Impl* pImpl, uno::Reference< io::XStream > xStream, sal_Bool bTransacted )
: m_pImpl( pImpl )
, m_bInStreamDisconnected( sal_False )
, m_bInitOnDemand( sal_False )
, m_nInitPosition( 0 )
, m_bTransacted( bTransacted )
{
    OSL_ENSURE( pImpl && xStream.is(), "No base implementation!\n" );
    OSL_ENSURE( m_pImpl->m_rMutexRef.Is(), "No mutex!\n" );

    if ( !m_pImpl || !m_pImpl->m_rMutexRef.Is() )
        throw uno::RuntimeException(); // just a disaster

    m_pData = new WSInternalData_Impl( pImpl->m_rMutexRef, m_pImpl->m_nStorageType );

    if ( xStream.is() )
    {
        m_xInStream = xStream->getInputStream();
        m_xOutStream = xStream->getOutputStream();
        m_xSeekable = uno::Reference< io::XSeekable >( xStream, uno::UNO_QUERY );
        OSL_ENSURE( m_xInStream.is() && m_xOutStream.is() && m_xSeekable.is(), "Stream implementation is incomplete!\n" );
    }
}

//-----------------------------------------------
OWriteStream::~OWriteStream()
{
    {
        ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );
        if ( m_pImpl )
        {
            m_refCount++;
            try {
                dispose();
            }
            catch( const uno::RuntimeException& rRuntimeException )
            {
                m_pImpl->AddLog( rRuntimeException.Message );
                m_pImpl->AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Quiet exception" ) ) );
            }
        }
    }

    if ( m_pData && m_pData->m_pTypeCollection )
        delete m_pData->m_pTypeCollection;

    if ( m_pData )
        delete m_pData;
}

//-----------------------------------------------
void OWriteStream::DeInit()
{
    if ( !m_pImpl )
        return; // do nothing

    if ( m_xSeekable.is() )
        m_nInitPosition = m_xSeekable->getPosition();

    m_xInStream = uno::Reference< io::XInputStream >();
    m_xOutStream = uno::Reference< io::XOutputStream >();
    m_xSeekable = uno::Reference< io::XSeekable >();
    m_bInitOnDemand = sal_True;
}

//-----------------------------------------------
void OWriteStream::CheckInitOnDemand()
{
    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( m_bInitOnDemand )
    {
        RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OWriteStream::CheckInitOnDemand, initializing" );
        uno::Reference< io::XStream > xStream = m_pImpl->GetTempFileAsStream();
        if ( xStream.is() )
        {
            m_xInStream.set( xStream->getInputStream(), uno::UNO_SET_THROW );
            m_xOutStream.set( xStream->getOutputStream(), uno::UNO_SET_THROW );
            m_xSeekable.set( xStream, uno::UNO_QUERY_THROW );
            m_xSeekable->seek( m_nInitPosition );

            m_nInitPosition = 0;
            m_bInitOnDemand = sal_False;
        }
    }
}

//-----------------------------------------------
void OWriteStream::CopyToStreamInternally_Impl( const uno::Reference< io::XStream >& xDest )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_xInStream.is() )
        throw uno::RuntimeException();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    uno::Reference< beans::XPropertySet > xDestProps( xDest, uno::UNO_QUERY );
    if ( !xDestProps.is() )
        throw uno::RuntimeException(); //TODO

    uno::Reference< io::XOutputStream > xDestOutStream = xDest->getOutputStream();
    if ( !xDestOutStream.is() )
        throw io::IOException(); // TODO

    sal_Int64 nCurPos = m_xSeekable->getPosition();
    m_xSeekable->seek( 0 );

    uno::Exception eThrown;
    sal_Bool bThrown = sal_False;
    try {
        ::comphelper::OStorageHelper::CopyInputToOutput( m_xInStream, xDestOutStream );
    }
    catch ( const uno::Exception& e )
    {
        eThrown = e;
        bThrown = sal_True;
    }

    // position-related section below is critical
    // if it fails the stream will become invalid
    try {
        m_xSeekable->seek( nCurPos );
    }
    catch ( const uno::Exception& rException )
    {
        m_pImpl->AddLog( rException.Message );
        m_pImpl->AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Quiet exception" ) ) );

        // TODO: set the stoream in invalid state or dispose
        OSL_FAIL( "The stream become invalid during copiing!\n" );
        throw uno::RuntimeException();
    }

    if ( bThrown )
        throw eThrown;

    // now the properties can be copied
    // the order of the properties setting is not important for StorageStream API
    ::rtl::OUString aPropName (RTL_CONSTASCII_USTRINGPARAM("Compressed") );
    xDestProps->setPropertyValue( aPropName, getPropertyValue( aPropName ) );
    if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE || m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        aPropName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType" ) );
        xDestProps->setPropertyValue( aPropName, getPropertyValue( aPropName ) );

        if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
        {
            aPropName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseCommonStoragePasswordEncryption") );
            xDestProps->setPropertyValue( aPropName, getPropertyValue( aPropName ) );
        }
    }
}

//-----------------------------------------------
void OWriteStream::ModifyParentUnlockMutex_Impl( ::osl::ResettableMutexGuard& aGuard )
{
    if ( m_pImpl->m_pParent )
    {
        if ( m_pImpl->m_pParent->m_pAntiImpl )
        {
            uno::Reference< util::XModifiable > xParentModif( (util::XModifiable*)(m_pImpl->m_pParent->m_pAntiImpl) );
            aGuard.clear();
            xParentModif->setModified( sal_True );
        }
        else
            m_pImpl->m_pParent->m_bIsModified = sal_True;
    }
}

//-----------------------------------------------
uno::Any SAL_CALL OWriteStream::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
{
    uno::Any aReturn;

    // common interfaces
    aReturn <<= ::cppu::queryInterface
                (   rType
                    ,   static_cast<lang::XTypeProvider*> ( this )
                    ,   static_cast<io::XInputStream*> ( this )
                    ,   static_cast<io::XOutputStream*> ( this )
                    ,   static_cast<io::XStream*> ( this )
                    ,   static_cast<embed::XExtendedStorageStream*> ( this )
                    ,   static_cast<io::XSeekable*> ( this )
                    ,   static_cast<io::XTruncate*> ( this )
                    ,   static_cast<lang::XComponent*> ( this )
                    ,   static_cast<beans::XPropertySet*> ( this ) );

    if ( aReturn.hasValue() == sal_True )
        return aReturn ;

    if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        aReturn <<= ::cppu::queryInterface
                    (   rType
                        ,   static_cast<embed::XEncryptionProtectedSource2*> ( this )
                        ,   static_cast<embed::XEncryptionProtectedSource*> ( this ) );
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        aReturn <<= ::cppu::queryInterface
                    (   rType
                        ,   static_cast<embed::XRelationshipAccess*> ( this ) );
    }

    if ( aReturn.hasValue() == sal_True )
        return aReturn ;

    if ( m_bTransacted )
    {
        aReturn <<= ::cppu::queryInterface
                    (   rType
                        ,   static_cast<embed::XTransactedObject*> ( this )
                        ,   static_cast<embed::XTransactionBroadcaster*> ( this ) );

        if ( aReturn.hasValue() == sal_True )
            return aReturn ;
    }

    return OWeakObject::queryInterface( rType );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::acquire() throw()
{
    OWeakObject::acquire();
}

//-----------------------------------------------
void SAL_CALL OWriteStream::release() throw()
{
    OWeakObject::release();
}

//-----------------------------------------------
uno::Sequence< uno::Type > SAL_CALL OWriteStream::getTypes()
        throw( uno::RuntimeException )
{
    if ( m_pData->m_pTypeCollection == NULL )
    {
        ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

        if ( m_pData->m_pTypeCollection == NULL )
        {
            if ( m_bTransacted )
            {
                if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
                {
                    ::cppu::OTypeCollection aTmpCollection
                                    (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XInputStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XOutputStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XSeekable >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XTruncate >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< lang::XComponent >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XEncryptionProtectedSource2 >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XEncryptionProtectedSource >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XExtendedStorageStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XTransactedObject >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XTransactionBroadcaster >* )NULL ) );

                    m_pData->m_pTypeCollection = new ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< beans::XPropertySet >* )NULL )
                                    ,   aTmpCollection.getTypes() );
                }
                else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
                {
                    m_pData->m_pTypeCollection = new ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XInputStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XOutputStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XSeekable >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XTruncate >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< lang::XComponent >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XRelationshipAccess >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XExtendedStorageStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XTransactedObject >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XTransactionBroadcaster >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< beans::XPropertySet >* )NULL ) );
                }
                else // if ( m_pData->m_nStorageType == embed::StorageFormats::ZIP )
                {
                    m_pData->m_pTypeCollection = new ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XInputStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XOutputStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XSeekable >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XTruncate >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< lang::XComponent >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XExtendedStorageStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XTransactedObject >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XTransactionBroadcaster >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< beans::XPropertySet >* )NULL ) );
                }
            }
            else
            {
                if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
                {
                    m_pData->m_pTypeCollection = new ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XInputStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XOutputStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XSeekable >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XTruncate >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< lang::XComponent >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XEncryptionProtectedSource2 >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XEncryptionProtectedSource >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< beans::XPropertySet >* )NULL ) );
                }
                else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
                {
                    m_pData->m_pTypeCollection = new ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XInputStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XOutputStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XSeekable >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XTruncate >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< lang::XComponent >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XRelationshipAccess >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< beans::XPropertySet >* )NULL ) );
                }
                else // if ( m_pData->m_nStorageType == embed::StorageFormats::ZIP )
                {
                    m_pData->m_pTypeCollection = new ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XInputStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XOutputStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XStream >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XSeekable >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< io::XTruncate >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< lang::XComponent >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< beans::XPropertySet >* )NULL ) );
                }
            }
        }
    }

    return m_pData->m_pTypeCollection->getTypes() ;
}

namespace { struct lcl_ImplId : public rtl::Static< ::cppu::OImplementationId, lcl_ImplId > {}; }

//-----------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL OWriteStream::getImplementationId()
        throw( uno::RuntimeException )
{
    ::cppu::OImplementationId &rId = lcl_ImplId::get();
    return rId.getImplementationId();
}

//-----------------------------------------------
sal_Int32 SAL_CALL OWriteStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    return m_xInStream->readBytes( aData, nBytesToRead );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OWriteStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    return m_xInStream->readSomeBytes( aData, nMaxBytesToRead );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::skipBytes( sal_Int32 nBytesToSkip )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    m_xInStream->skipBytes( nBytesToSkip );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OWriteStream::available(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    return m_xInStream->available();

}

//-----------------------------------------------
void SAL_CALL OWriteStream::closeInput(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_bInitOnDemand && ( m_bInStreamDisconnected || !m_xInStream.is() ) )
        throw io::NotConnectedException();

    // the input part of the stream stays open for internal purposes ( to allow reading during copiing )
    // since it can not be reopened until output part is closed, it will be closed with output part.
    m_bInStreamDisconnected = sal_True;
    // m_xInStream->closeInput();
    // m_xInStream = uno::Reference< io::XInputStream >();

    if ( !m_xOutStream.is() )
        dispose();
}

//-----------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OWriteStream::getInputStream()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_bInitOnDemand && ( m_bInStreamDisconnected || !m_xInStream.is() ) )
        return uno::Reference< io::XInputStream >();

    return uno::Reference< io::XInputStream >( static_cast< io::XInputStream* >( this ), uno::UNO_QUERY );
}

//-----------------------------------------------
uno::Reference< io::XOutputStream > SAL_CALL OWriteStream::getOutputStream()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_xOutStream.is() )
        return uno::Reference< io::XOutputStream >();

    return uno::Reference< io::XOutputStream >( static_cast< io::XOutputStream* >( this ), uno::UNO_QUERY );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::writeBytes( const uno::Sequence< sal_Int8 >& aData )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    // the write method makes initialization itself, since it depends from the aData length
    // NO CheckInitOnDemand()!

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_bInitOnDemand )
    {
        if ( !m_xOutStream.is() || !m_xSeekable.is())
            throw io::NotConnectedException();

        if ( m_pImpl->m_xCacheStream.is() )
        {
            // check whether the cache should be turned off
            sal_Int64 nPos = m_xSeekable->getPosition();
            if ( nPos + aData.getLength() > MAX_STORCACHE_SIZE )
            {
                // disconnect the cache and copy the data to the temporary file
                m_xSeekable->seek( 0 );

                // it is enough to copy the cached stream, the cache should already contain everything
                if ( !m_pImpl->GetFilledTempFileIfNo( m_xInStream ).isEmpty() )
                {
                    DeInit();
                    // the last position is known and it is differs from the current stream position
                    m_nInitPosition = nPos;
                }
            }
        }
    }

    if ( m_bInitOnDemand )
    {
        RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OWriteStream::CheckInitOnDemand, initializing" );
        uno::Reference< io::XStream > xStream = m_pImpl->GetTempFileAsStream();
        if ( xStream.is() )
        {
            m_xInStream.set( xStream->getInputStream(), uno::UNO_SET_THROW );
            m_xOutStream.set( xStream->getOutputStream(), uno::UNO_SET_THROW );
            m_xSeekable.set( xStream, uno::UNO_QUERY_THROW );
            m_xSeekable->seek( m_nInitPosition );

            m_nInitPosition = 0;
            m_bInitOnDemand = sal_False;
        }
    }


    if ( !m_xOutStream.is() )
        throw io::NotConnectedException();

    m_xOutStream->writeBytes( aData );
    m_pImpl->m_bHasDataToFlush = sal_True;

    ModifyParentUnlockMutex_Impl( aGuard );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::flush()
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    // In case stream is flushed it's current version becomes visible
    // to the parent storage. Usually parent storage flushes the stream
    // during own commit but a user can explicitly flush the stream
    // so the changes will be available through cloning functionality.

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_bInitOnDemand )
    {
        if ( !m_xOutStream.is() )
            throw io::NotConnectedException();

        m_xOutStream->flush();
        m_pImpl->Commit();
    }
}

//-----------------------------------------------
void OWriteStream::CloseOutput_Impl()
{
    // all the checks must be done in calling method

    m_xOutStream->closeOutput();
    m_xOutStream = uno::Reference< io::XOutputStream >();

    if ( !m_bInitOnDemand )
    {
        // after the stream is disposed it can be commited
        // so transport correct size property
        if ( !m_xSeekable.is() )
            throw uno::RuntimeException();

        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( m_pImpl->m_aProps[nInd].Name == "Size" )
                m_pImpl->m_aProps[nInd].Value <<= m_xSeekable->getLength();
        }
    }
}

//-----------------------------------------------
void SAL_CALL OWriteStream::closeOutput()
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_xOutStream.is() )
        throw io::NotConnectedException();

    CloseOutput_Impl();

    if ( m_bInStreamDisconnected || !m_xInStream.is() )
        dispose();
}

//-----------------------------------------------
void SAL_CALL OWriteStream::seek( sal_Int64 location )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    m_xSeekable->seek( location );
}

//-----------------------------------------------
sal_Int64 SAL_CALL OWriteStream::getPosition()
        throw ( io::IOException,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getPosition();
}

//-----------------------------------------------
sal_Int64 SAL_CALL OWriteStream::getLength()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getLength();
}

//-----------------------------------------------
void SAL_CALL OWriteStream::truncate()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_xOutStream.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XTruncate > xTruncate( m_xOutStream, uno::UNO_QUERY );

    if ( !xTruncate.is() )
    {
        OSL_FAIL( "The output stream must support XTruncate interface!\n" );
        throw uno::RuntimeException();
    }

    xTruncate->truncate();

    m_pImpl->m_bHasDataToFlush = sal_True;

    ModifyParentUnlockMutex_Impl( aGuard );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::dispose()
        throw ( uno::RuntimeException )
{
    // should be an internal method since it can be called only from parent storage
    {
        ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

        if ( !m_pImpl )
        {
            ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
            throw lang::DisposedException();
        }

        if ( m_xOutStream.is() )
            CloseOutput_Impl();

        if ( m_xInStream.is() )
        {
            m_xInStream->closeInput();
            m_xInStream = uno::Reference< io::XInputStream >();
        }

        m_xSeekable = uno::Reference< io::XSeekable >();

        m_pImpl->m_pAntiImpl = NULL;

        if ( !m_bInitOnDemand )
        {
            try
            {
                if ( !m_bTransacted )
                {
                    m_pImpl->Commit();
                }
                else
                {
                    // throw away all the changes
                    m_pImpl->Revert();
                }
            }
            catch( const uno::Exception& rException )
            {
                m_pImpl->AddLog( rException.Message );
                m_pImpl->AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow" ) ) );

                uno::Any aCaught( ::cppu::getCaughtException() );
                throw lang::WrappedTargetRuntimeException(
                                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Can not commit/revert the storage!\n") ),
                                                uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                    uno::UNO_QUERY ),
                                                aCaught );
            }
        }

        m_pImpl = NULL;
    }

    // the listener might try to get rid of parent storage, and the storage would delete this object;
    // for now the listener is just notified at the end of the method to workaround the problem
    // in future a more elegant way should be found

       lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
    m_pData->m_aListenersContainer.disposeAndClear( aSource );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::addEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    m_pData->m_aListenersContainer.addInterface( ::getCppuType((const uno::Reference< lang::XEventListener >*)0),
                                                 xListener );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::removeEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    m_pData->m_aListenersContainer.removeInterface( ::getCppuType((const uno::Reference< lang::XEventListener >*)0),
                                                    xListener );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::setEncryptionPassword( const ::rtl::OUString& aPass )
    throw ( uno::RuntimeException,
            io::IOException )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    OSL_ENSURE( m_pImpl->m_xPackageStream.is(), "No package stream is set!\n" );

    m_pImpl->SetEncrypted( ::comphelper::OStorageHelper::CreatePackageEncryptionData( aPass ) );

    ModifyParentUnlockMutex_Impl( aGuard );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::removeEncryption()
    throw ( uno::RuntimeException,
            io::IOException )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    OSL_ENSURE( m_pImpl->m_xPackageStream.is(), "No package stream is set!\n" );

    m_pImpl->SetDecrypted();

    ModifyParentUnlockMutex_Impl( aGuard );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::setEncryptionData( const uno::Sequence< beans::NamedValue >& aEncryptionData )
    throw (io::IOException, uno::RuntimeException)
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    OSL_ENSURE( m_pImpl->m_xPackageStream.is(), "No package stream is set!\n" );

    m_pImpl->SetEncrypted( aEncryptionData );

    ModifyParentUnlockMutex_Impl( aGuard );
}

//-----------------------------------------------
sal_Bool SAL_CALL OWriteStream::hasEncryptionData()
    throw (uno::RuntimeException)
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if (!m_pImpl)
        return sal_False;

    sal_Bool bRet = m_pImpl->IsEncrypted();

    if (!bRet && m_pImpl->m_bUseCommonEncryption && m_pImpl->m_pParent)
        bRet = m_pImpl->m_pParent->m_bHasCommonEncryptionData;

    return bRet;
}

//-----------------------------------------------
sal_Bool SAL_CALL OWriteStream::hasByID(  const ::rtl::OUString& sID )
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    try
    {
        getRelationshipByID( sID );
        return sal_True;
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        m_pImpl->AddLog( rNoSuchElementException.Message );
        m_pImpl->AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "No Element" ) ) );
    }

    return sal_False;
}

//-----------------------------------------------
::rtl::OUString SAL_CALL OWriteStream::getTargetByID(  const ::rtl::OUString& sID  )
        throw ( container::NoSuchElementException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( aSeq[nInd].First == "Target" )
            return aSeq[nInd].Second;

    return ::rtl::OUString();
}

//-----------------------------------------------
::rtl::OUString SAL_CALL OWriteStream::getTypeByID(  const ::rtl::OUString& sID  )
        throw ( container::NoSuchElementException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( aSeq[nInd].First == "Type" )
            return aSeq[nInd].Second;

    return ::rtl::OUString();
}

//-----------------------------------------------
uno::Sequence< beans::StringPair > SAL_CALL OWriteStream::getRelationshipByID(  const ::rtl::OUString& sID  )
        throw ( container::NoSuchElementException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    // TODO/LATER: in future the unification of the ID could be checked
    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    for ( sal_Int32 nInd1 = 0; nInd1 < aSeq.getLength(); nInd1++ )
        for ( sal_Int32 nInd2 = 0; nInd2 < aSeq[nInd1].getLength(); nInd2++ )
            if ( aSeq[nInd1][nInd2].First == "Id" )
            {
                if ( aSeq[nInd1][nInd2].Second.equals( sID ) )
                    return aSeq[nInd1];
                break;
            }

    throw container::NoSuchElementException();
}

//-----------------------------------------------
uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OWriteStream::getRelationshipsByType(  const ::rtl::OUString& sType  )
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    uno::Sequence< uno::Sequence< beans::StringPair > > aResult;
    sal_Int32 nEntriesNum = 0;

    // TODO/LATER: in future the unification of the ID could be checked
    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    for ( sal_Int32 nInd1 = 0; nInd1 < aSeq.getLength(); nInd1++ )
        for ( sal_Int32 nInd2 = 0; nInd2 < aSeq[nInd1].getLength(); nInd2++ )
            if ( aSeq[nInd1][nInd2].First == "Type" )
            {
                if ( aSeq[nInd1][nInd2].Second.equals( sType ) )
                {
                    aResult.realloc( nEntriesNum );
                    aResult[nEntriesNum-1] = aSeq[nInd1];
                }
                break;
            }

    return aResult;
}

//-----------------------------------------------
uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OWriteStream::getAllRelationships()
        throw (io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    return m_pImpl->GetAllRelationshipsIfAny();
}

//-----------------------------------------------
void SAL_CALL OWriteStream::insertRelationshipByID(  const ::rtl::OUString& sID, const uno::Sequence< beans::StringPair >& aEntry, ::sal_Bool bReplace  )
        throw ( container::ElementExistException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    ::rtl::OUString aIDTag( RTL_CONSTASCII_USTRINGPARAM( "Id" ) );

    sal_Int32 nIDInd = -1;

    // TODO/LATER: in future the unification of the ID could be checked
    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    for ( sal_Int32 nInd1 = 0; nInd1 < aSeq.getLength(); nInd1++ )
        for ( sal_Int32 nInd2 = 0; nInd2 < aSeq[nInd1].getLength(); nInd2++ )
            if ( aSeq[nInd1][nInd2].First.equals( aIDTag ) )
            {
                if ( aSeq[nInd1][nInd2].Second.equals( sID ) )
                    nIDInd = nInd1;

                break;
            }

    if ( nIDInd == -1 || bReplace )
    {
        if ( nIDInd == -1 )
        {
            nIDInd = aSeq.getLength();
            aSeq.realloc( nIDInd + 1 );
        }

        aSeq[nIDInd].realloc( aEntry.getLength() + 1 );

        aSeq[nIDInd][0].First = aIDTag;
        aSeq[nIDInd][0].Second = sID;
        sal_Int32 nIndTarget = 1;
        for ( sal_Int32 nIndOrig = 0;
              nIndOrig < aEntry.getLength();
              nIndOrig++ )
        {
            if ( !aEntry[nIndOrig].First.equals( aIDTag ) )
                aSeq[nIDInd][nIndTarget++] = aEntry[nIndOrig];
        }

        aSeq[nIDInd].realloc( nIndTarget );
    }
    else
        throw container::ElementExistException(); // TODO


    m_pImpl->m_aNewRelInfo = aSeq;
    m_pImpl->m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

//-----------------------------------------------
void SAL_CALL OWriteStream::removeRelationshipByID(  const ::rtl::OUString& sID  )
        throw ( container::NoSuchElementException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    for ( sal_Int32 nInd1 = 0; nInd1 < aSeq.getLength(); nInd1++ )
        for ( sal_Int32 nInd2 = 0; nInd2 < aSeq[nInd1].getLength(); nInd2++ )
            if ( aSeq[nInd1][nInd2].First == "Id" )
            {
                if ( aSeq[nInd1][nInd2].Second.equals( sID ) )
                {
                    sal_Int32 nLength = aSeq.getLength();
                    aSeq[nInd1] = aSeq[nLength-1];
                    aSeq.realloc( nLength - 1 );

                    m_pImpl->m_aNewRelInfo = aSeq;
                    m_pImpl->m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
                    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;

                    // TODO/LATER: in future the unification of the ID could be checked
                    return;
                }

                break;
            }

    throw container::NoSuchElementException();
}

//-----------------------------------------------
void SAL_CALL OWriteStream::insertRelationships(  const uno::Sequence< uno::Sequence< beans::StringPair > >& aEntries, ::sal_Bool bReplace  )
        throw ( container::ElementExistException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    ::rtl::OUString aIDTag( RTL_CONSTASCII_USTRINGPARAM( "Id" ) );
    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    uno::Sequence< uno::Sequence< beans::StringPair > > aResultSeq( aSeq.getLength() + aEntries.getLength() );
    sal_Int32 nResultInd = 0;

    for ( sal_Int32 nIndTarget1 = 0; nIndTarget1 < aSeq.getLength(); nIndTarget1++ )
        for ( sal_Int32 nIndTarget2 = 0; nIndTarget2 < aSeq[nIndTarget1].getLength(); nIndTarget2++ )
            if ( aSeq[nIndTarget1][nIndTarget2].First.equals( aIDTag ) )
            {
                sal_Int32 nIndSourceSame = -1;

                for ( sal_Int32 nIndSource1 = 0; nIndSource1 < aEntries.getLength(); nIndSource1++ )
                    for ( sal_Int32 nIndSource2 = 0; nIndSource2 < aEntries[nIndSource1].getLength(); nIndSource2++ )
                    {
                        if ( aEntries[nIndSource1][nIndSource2].First.equals( aIDTag ) )
                        {
                            if ( aEntries[nIndSource1][nIndSource2].Second.equals( aSeq[nIndTarget1][nIndTarget2].Second ) )
                            {
                                if ( !bReplace )
                                    throw container::ElementExistException();

                                nIndSourceSame = nIndSource1;
                            }

                            break;
                        }
                    }

                if ( nIndSourceSame == -1 )
                {
                    // no such element in the provided sequence
                    aResultSeq[nResultInd++] = aSeq[nIndTarget1];
                }

                break;
            }

    for ( sal_Int32 nIndSource1 = 0; nIndSource1 < aEntries.getLength(); nIndSource1++ )
    {
        aResultSeq[nResultInd].realloc( aEntries[nIndSource1].getLength() );
        sal_Bool bHasID = sal_False;
        sal_Int32 nResInd2 = 1;

        for ( sal_Int32 nIndSource2 = 0; nIndSource2 < aEntries[nIndSource1].getLength(); nIndSource2++ )
            if ( aEntries[nIndSource1][nIndSource2].First.equals( aIDTag ) )
            {
                aResultSeq[nResultInd][0] = aEntries[nIndSource1][nIndSource2];
                bHasID = sal_True;
            }
            else if ( nResInd2 < aResultSeq[nResultInd].getLength() )
                aResultSeq[nResultInd][nResInd2++] = aEntries[nIndSource1][nIndSource2];
            else
                throw io::IOException(); // TODO: illegal relation ( no ID )

        if ( !bHasID )
            throw io::IOException(); // TODO: illegal relations

        nResultInd++;
    }

    aResultSeq.realloc( nResultInd );
    m_pImpl->m_aNewRelInfo = aResultSeq;
    m_pImpl->m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

//-----------------------------------------------
void SAL_CALL OWriteStream::clearRelationships()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    m_pImpl->m_aNewRelInfo.realloc( 0 );
    m_pImpl->m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

//-----------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OWriteStream::getPropertySetInfo()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    //TODO:
    return uno::Reference< beans::XPropertySetInfo >();
}

//-----------------------------------------------
void SAL_CALL OWriteStream::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
        throw ( beans::UnknownPropertyException,
                beans::PropertyVetoException,
                lang::IllegalArgumentException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    m_pImpl->GetStreamProperties();
    ::rtl::OUString aCompressedString( RTL_CONSTASCII_USTRINGPARAM( "Compressed" ) );
    ::rtl::OUString aMediaTypeString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) );
    if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE && aPropertyName.equals( aMediaTypeString ) )
    {
        // if the "Compressed" property is not set explicitly, the MediaType can change the default value
        sal_Bool bCompressedValueFromType = sal_True;
        ::rtl::OUString aType;
        aValue >>= aType;

        if ( !m_pImpl->m_bCompressedSetExplicit )
        {
            if ( aType == "image/jpeg" || aType == "image/png" || aType == "image/gif" )
                bCompressedValueFromType = sal_False;
        }

        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( aPropertyName.equals( m_pImpl->m_aProps[nInd].Name ) )
                m_pImpl->m_aProps[nInd].Value = aValue;
            else if ( !m_pImpl->m_bCompressedSetExplicit && aCompressedString.equals( m_pImpl->m_aProps[nInd].Name ) )
                m_pImpl->m_aProps[nInd].Value <<= bCompressedValueFromType;
        }
    }
    else if ( aPropertyName.equals( aCompressedString ) )
    {
        // if the "Compressed" property is not set explicitly, the MediaType can change the default value
        m_pImpl->m_bCompressedSetExplicit = sal_True;
        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( aPropertyName.equals( m_pImpl->m_aProps[nInd].Name ) )
                m_pImpl->m_aProps[nInd].Value = aValue;
        }
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE
            && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "UseCommonStoragePasswordEncryption" ) ) )
    {
        sal_Bool bUseCommonEncryption = sal_False;
        if ( aValue >>= bUseCommonEncryption )
        {
            if ( m_bInitOnDemand && m_pImpl->m_bHasInsertedStreamOptimization )
            {
                // the data stream is provided to the packagestream directly
                m_pImpl->m_bUseCommonEncryption = bUseCommonEncryption;
            }
            else if ( bUseCommonEncryption )
            {
                if ( !m_pImpl->m_bUseCommonEncryption )
                {
                    m_pImpl->SetDecrypted();
                    m_pImpl->m_bUseCommonEncryption = sal_True;
                }
            }
            else
                m_pImpl->m_bUseCommonEncryption = sal_False;
        }
        else
            throw lang::IllegalArgumentException(); //TODO
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aPropertyName.equals( aMediaTypeString ) )
    {
        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( aPropertyName.equals( m_pImpl->m_aProps[nInd].Name ) )
                m_pImpl->m_aProps[nInd].Value = aValue;
        }
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "RelationsInfoStream" ) ) )
    {
        uno::Reference< io::XInputStream > xInRelStream;
        if ( ( aValue >>= xInRelStream ) && xInRelStream.is() )
        {
            uno::Reference< io::XSeekable > xSeek( xInRelStream, uno::UNO_QUERY );
            if ( !xSeek.is() )
            {
                // currently this is an internal property that is used for optimization
                // and the stream must support XSeekable interface
                // TODO/LATER: in future it can be changed if property is used from outside
                throw lang::IllegalArgumentException(); // TODO
            }

            m_pImpl->m_xNewRelInfoStream = xInRelStream;
            m_pImpl->m_aNewRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
            m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED_STREAM;
        }
        else
            throw lang::IllegalArgumentException(); // TODO
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "RelationsInfo" ) ) )
    {
        if ( aValue >>= m_pImpl->m_aNewRelInfo )
        {
        }
        else
            throw lang::IllegalArgumentException(); // TODO
    }
    else if ( aPropertyName == "Size" )
        throw beans::PropertyVetoException(); // TODO
    else if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE
           && ( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsEncrypted" ) ) || aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Encrypted" ) ) ) )
        throw beans::PropertyVetoException(); // TODO
    else
        throw beans::UnknownPropertyException(); // TODO

    m_pImpl->m_bHasDataToFlush = sal_True;
    ModifyParentUnlockMutex_Impl( aGuard );
}


//-----------------------------------------------
uno::Any SAL_CALL OWriteStream::getPropertyValue( const ::rtl::OUString& aProp )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( aProp == "RelId" )
    {
        return uno::makeAny( m_pImpl->GetNewRelId() );
    }

    ::rtl::OUString aPropertyName;
    if ( aProp == "IsEncrypted" )
        aPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Encrypted" ) );
    else
        aPropertyName = aProp;

    if ( ( ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE || m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
            && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
      || ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Encrypted" ) ) )
      || aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Compressed" ) ) )
    {
        m_pImpl->GetStreamProperties();

        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( aPropertyName.equals( m_pImpl->m_aProps[nInd].Name ) )
                return m_pImpl->m_aProps[nInd].Value;
        }
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE
            && aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "UseCommonStoragePasswordEncryption" ) ) )
        return uno::makeAny( m_pImpl->m_bUseCommonEncryption );
    else if ( aPropertyName == "Size" )
    {
        CheckInitOnDemand();

        if ( !m_xSeekable.is() )
            throw uno::RuntimeException();

        return uno::makeAny( m_xSeekable->getLength() );
    }

    throw beans::UnknownPropertyException(); // TODO
}


//-----------------------------------------------
void SAL_CALL OWriteStream::addPropertyChangeListener(
    const ::rtl::OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OWriteStream::removePropertyChangeListener(
    const ::rtl::OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OWriteStream::addVetoableChangeListener(
    const ::rtl::OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OWriteStream::removeVetoableChangeListener(
    const ::rtl::OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    //TODO:
}

//____________________________________________________________________________________________________
//  XTransactedObject
//____________________________________________________________________________________________________

//-----------------------------------------------
void OWriteStream::BroadcastTransaction( sal_Int8 nMessage )
/*
    1 - preCommit
    2 - commited
    3 - preRevert
    4 - reverted
*/
{
    // no need to lock mutex here for the checking of m_pImpl, and m_pData is alive until the object is destructed
    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

       lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );

       ::cppu::OInterfaceContainerHelper* pContainer =
            m_pData->m_aListenersContainer.getContainer(
                ::getCppuType( ( const uno::Reference< embed::XTransactionListener >*) NULL ) );
       if ( pContainer )
    {
           ::cppu::OInterfaceIteratorHelper pIterator( *pContainer );
           while ( pIterator.hasMoreElements( ) )
           {
            OSL_ENSURE( nMessage >= 1 && nMessage <= 4, "Wrong internal notification code is used!\n" );

            switch( nMessage )
            {
                case STOR_MESS_PRECOMMIT:
                       ( ( embed::XTransactionListener* )pIterator.next( ) )->preCommit( aSource );
                    break;
                case STOR_MESS_COMMITED:
                       ( ( embed::XTransactionListener* )pIterator.next( ) )->commited( aSource );
                    break;
                case STOR_MESS_PREREVERT:
                       ( ( embed::XTransactionListener* )pIterator.next( ) )->preRevert( aSource );
                    break;
                case STOR_MESS_REVERTED:
                       ( ( embed::XTransactionListener* )pIterator.next( ) )->reverted( aSource );
                    break;
            }
           }
    }
}
//-----------------------------------------------
void SAL_CALL OWriteStream::commit()
        throw ( io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OWriteStream::commit" );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_bTransacted )
        throw uno::RuntimeException();

    try {
        BroadcastTransaction( STOR_MESS_PRECOMMIT );

        ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

        if ( !m_pImpl )
        {
            ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
            throw lang::DisposedException();
        }

        m_pImpl->Commit();

        // when the storage is commited the parent is modified
        ModifyParentUnlockMutex_Impl( aGuard );
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow" ) ) );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow" ) ) );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow" ) ) );
        throw;
    }
    catch( const uno::Exception& rException )
    {
        m_pImpl->AddLog( rException.Message );
        m_pImpl->AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow" ) ) );

        uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Problems on commit!") ),
                                  uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ),
                                  aCaught );
    }

    BroadcastTransaction( STOR_MESS_COMMITED );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::revert()
        throw ( io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OWriteStream::revert" );

    // the method removes all the changes done after last commit

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_bTransacted )
        throw uno::RuntimeException();

    BroadcastTransaction( STOR_MESS_PREREVERT );

    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    try {
        m_pImpl->Revert();
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow" ) ) );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow" ) ) );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow" ) ) );
        throw;
    }
    catch( const uno::Exception& rException )
    {
        m_pImpl->AddLog( rException.Message );
        m_pImpl->AddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Rethrow" ) ) );

        uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Problems on revert!") ),
                                  uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ),
                                  aCaught );
    }

    aGuard.clear();

    BroadcastTransaction( STOR_MESS_REVERTED );
}

//____________________________________________________________________________________________________
//  XTransactionBroadcaster
//____________________________________________________________________________________________________

//-----------------------------------------------
void SAL_CALL OWriteStream::addTransactionListener( const uno::Reference< embed::XTransactionListener >& aListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_bTransacted )
        throw uno::RuntimeException();

    m_pData->m_aListenersContainer.addInterface( ::getCppuType((const uno::Reference< embed::XTransactionListener >*)0),
                                                aListener );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::removeTransactionListener( const uno::Reference< embed::XTransactionListener >& aListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Disposed!" ) ) );
        throw lang::DisposedException();
    }

    if ( !m_bTransacted )
        throw uno::RuntimeException();

    m_pData->m_aListenersContainer.removeInterface( ::getCppuType((const uno::Reference< embed::XTransactionListener >*)0),
                                                    aListener );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
