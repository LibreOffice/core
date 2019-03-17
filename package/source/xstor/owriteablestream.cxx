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

#include <memory>
#include <sal/config.h>
#include <sal/log.hxx>

#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/embed/StorageWrappedTargetException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <osl/diagnose.h>

#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <comphelper/refcountedmutex.hxx>

#include <rtl/digest.h>
#include <rtl/instance.hxx>
#include <tools/diagnose_ex.h>

#include <PackageConstants.hxx>

#include "selfterminatefilestream.hxx"
#include "owriteablestream.hxx"
#include "oseekinstream.hxx"
#include "xstorage.hxx"

// since the copying uses 32000 blocks usually, it makes sense to have a smaller size
#define MAX_STORCACHE_SIZE 30000

using namespace ::com::sun::star;

struct WSInternalData_Impl
{
    rtl::Reference<comphelper::RefCountedMutex> m_xSharedMutex;
    ::std::unique_ptr< ::cppu::OTypeCollection> m_pTypeCollection;
    ::cppu::OMultiTypeInterfaceContainerHelper m_aListenersContainer; // list of listeners
    sal_Int32 const m_nStorageType;

    // the mutex reference MUST NOT be empty
    WSInternalData_Impl( const rtl::Reference<comphelper::RefCountedMutex>& rMutexRef, sal_Int32 nStorageType )
    : m_xSharedMutex( rMutexRef )
    , m_pTypeCollection()
    , m_aListenersContainer( rMutexRef->GetMutex() )
    , m_nStorageType( nStorageType )
    {}
};

namespace package
{

bool PackageEncryptionDatasEqual( const ::comphelper::SequenceAsHashMap& aHash1, const ::comphelper::SequenceAsHashMap& aHash2 )
{
    bool bResult = !aHash1.empty() && aHash1.size() == aHash2.size();
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

} // namespace package

namespace
{

void SetEncryptionKeyProperty_Impl( const uno::Reference< beans::XPropertySet >& xPropertySet,
                                    const uno::Sequence< beans::NamedValue >& aKey )
{
    SAL_WARN_IF( !xPropertySet.is(), "package.xstor", "No property set is provided!" );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    try {
        xPropertySet->setPropertyValue( STORAGE_ENCRYPTION_KEYS_PROPERTY, uno::makeAny( aKey ) );
    }
    catch ( const uno::Exception& rException )
    {
        SAL_INFO("package.xstor", rException);
        SAL_INFO("package.xstor", "Can't set encryption");
        SAL_WARN( "package.xstor", "Can't write encryption related properties!" );
        throw io::IOException(); // TODO
    }
}

uno::Any GetEncryptionKeyProperty_Impl( const uno::Reference< beans::XPropertySet >& xPropertySet )
{
    SAL_WARN_IF( !xPropertySet.is(), "package.xstor", "No property set is provided!" );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    try {
        return xPropertySet->getPropertyValue(STORAGE_ENCRYPTION_KEYS_PROPERTY);
    }
    catch ( const uno::Exception& rException )
    {
        SAL_INFO("package.xstor", rException);
        SAL_INFO("package.xstor", "Can't get encryption property");

        SAL_WARN( "package.xstor", "Can't get encryption related properties!" );
        throw io::IOException(); // TODO
    }
}

bool SequencesEqual( const uno::Sequence< sal_Int8 >& aSequence1, const uno::Sequence< sal_Int8 >& aSequence2 )
{
    if ( aSequence1.getLength() != aSequence2.getLength() )
        return false;

    for ( sal_Int32 nInd = 0; nInd < aSequence1.getLength(); nInd++ )
        if ( aSequence1[nInd] != aSequence2[nInd] )
            return false;

    return true;
}

bool SequencesEqual( const uno::Sequence< beans::NamedValue >& aSequence1, const uno::Sequence< beans::NamedValue >& aSequence2 )
{
    if ( aSequence1.getLength() != aSequence2.getLength() )
        return false;

    for ( sal_Int32 nInd = 0; nInd < aSequence1.getLength(); nInd++ )
    {
        bool bHasMember = false;
        uno::Sequence< sal_Int8 > aMember1;
        sal_Int32 nMember1 = 0;
        if ( aSequence1[nInd].Value >>= aMember1 )
        {
            for ( sal_Int32 nInd2 = 0; nInd2 < aSequence2.getLength(); nInd2++ )
            {
                if ( aSequence1[nInd].Name == aSequence2[nInd2].Name )
                {
                    bHasMember = true;

                    uno::Sequence< sal_Int8 > aMember2;
                    if ( !( aSequence2[nInd2].Value >>= aMember2 ) || !SequencesEqual( aMember1, aMember2 ) )
                        return false;
                }
            }
        }
        else if ( aSequence1[nInd].Value >>= nMember1 )
        {
            for ( sal_Int32 nInd2 = 0; nInd2 < aSequence2.getLength(); nInd2++ )
            {
                if ( aSequence1[nInd].Name == aSequence2[nInd2].Name )
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

bool KillFile( const OUString& aURL, const uno::Reference< uno::XComponentContext >& xContext )
{
    if ( !xContext.is() )
        return false;

    bool bRet = false;

    try
    {
        uno::Reference < ucb::XSimpleFileAccess3 > xAccess( ucb::SimpleFileAccess::create( xContext ) );

        xAccess->kill( aURL );
        bRet = true;
    }
    catch( const uno::Exception& rException )
    {
        SAL_INFO("package.xstor", rException);
        SAL_INFO("package.xstor", "Quiet exception");
    }

    return bRet;
}

OUString GetNewTempFileURL( const uno::Reference< uno::XComponentContext >& rContext )
{
    OUString aTempURL;

    uno::Reference < beans::XPropertySet > xTempFile(
            io::TempFile::create(rContext),
            uno::UNO_QUERY_THROW );

    try {
        xTempFile->setPropertyValue( "RemoveFile", uno::makeAny( false ) );
        uno::Any aUrl = xTempFile->getPropertyValue( "Uri" );
        aUrl >>= aTempURL;
    }
    catch ( const uno::Exception& rException )
    {
        SAL_INFO("package.xstor", rException);
        SAL_INFO("package.xstor", "Quiet exception");
    }

    if ( aTempURL.isEmpty() )
        throw uno::RuntimeException(); // TODO: can not create tempfile

    return aTempURL;
}

uno::Reference< io::XStream > CreateMemoryStream( const uno::Reference< uno::XComponentContext >& rContext )
{
    return uno::Reference< io::XStream >(
        rContext->getServiceManager()->createInstanceWithContext("com.sun.star.comp.MemoryStream", rContext),
        uno::UNO_QUERY_THROW);
}

} // anonymous namespace

OWriteStream_Impl::OWriteStream_Impl( OStorage_Impl* pParent,
                                      const uno::Reference< packages::XDataSinkEncrSupport >& xPackageStream,
                                      const uno::Reference< lang::XSingleServiceFactory >& xPackage,
                                      const uno::Reference< uno::XComponentContext >& rContext,
                                      bool bForceEncrypted,
                                      sal_Int32 nStorageType,
                                      bool bDefaultCompress,
                                      const uno::Reference< io::XInputStream >& xRelInfoStream )
: m_xMutex( new comphelper::RefCountedMutex )
, m_pAntiImpl( nullptr )
, m_bHasDataToFlush( false )
, m_bFlushed( false )
, m_xPackageStream( xPackageStream )
, m_xContext( rContext )
, m_pParent( pParent )
, m_bForceEncrypted( bForceEncrypted )
, m_bUseCommonEncryption( !bForceEncrypted && nStorageType == embed::StorageFormats::PACKAGE )
, m_bHasCachedEncryptionData( false )
, m_bCompressedSetExplicit( !bDefaultCompress )
, m_xPackage( xPackage )
, m_bHasInsertedStreamOptimization( false )
, m_nStorageType( nStorageType )
, m_xOrigRelInfoStream( xRelInfoStream )
, m_bOrigRelInfoBroken( false )
, m_nRelInfoStatus( RELINFO_NO_INIT )
, m_nRelId( 1 )
{
    SAL_WARN_IF( !xPackageStream.is(), "package.xstor", "No package stream is provided!" );
    SAL_WARN_IF( !xPackage.is(), "package.xstor", "No package component is provided!" );
    SAL_WARN_IF( !m_xContext.is(), "package.xstor", "No package stream is provided!" );
    OSL_ENSURE( pParent, "No parent storage is provided!" );
    OSL_ENSURE( m_nStorageType == embed::StorageFormats::OFOPXML || !m_xOrigRelInfoStream.is(), "The Relations info makes sense only for OFOPXML format!" );
}

OWriteStream_Impl::~OWriteStream_Impl()
{
    DisposeWrappers();

    if ( !m_aTempURL.isEmpty() )
    {
        KillFile( m_aTempURL, comphelper::getProcessComponentContext() );
        m_aTempURL.clear();
    }

    CleanCacheStream();
}

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

        m_xCacheStream.clear();
        m_xCacheSeek.clear();
    }
}

void OWriteStream_Impl::InsertIntoPackageFolder( const OUString& aName,
                                                  const uno::Reference< container::XNameContainer >& xParentPackageFolder )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    SAL_WARN_IF( !m_bFlushed, "package.xstor", "This method must not be called for nonflushed streams!" );
    if ( m_bFlushed )
    {
        SAL_WARN_IF( !m_xPackageStream.is(), "package.xstor", "An inserted stream is incomplete!" );
        uno::Reference< lang::XUnoTunnel > xTunnel( m_xPackageStream, uno::UNO_QUERY_THROW );
        xParentPackageFolder->insertByName( aName, uno::makeAny( xTunnel ) );

        m_bFlushed = false;
        m_bHasInsertedStreamOptimization = false;
    }
}
bool OWriteStream_Impl::IsEncrypted()
{
    if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        return false;

    if ( m_bForceEncrypted || m_bHasCachedEncryptionData )
        return true;

    if ( !m_aTempURL.isEmpty() || m_xCacheStream.is() )
        return false;

    GetStreamProperties();

    // the following value can not be cached since it can change after root commit
    bool bWasEncr = false;
    uno::Reference< beans::XPropertySet > xPropSet( m_xPackageStream, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        uno::Any aValue = xPropSet->getPropertyValue("WasEncrypted");
        if ( !( aValue >>= bWasEncr ) )
        {
            SAL_WARN( "package.xstor", "The property WasEncrypted has wrong type!" );
        }
    }

    bool bToBeEncr = false;
    for ( sal_Int32 nInd = 0; nInd < m_aProps.getLength(); nInd++ )
    {
        if ( m_aProps[nInd].Name == "Encrypted" )
        {
            if ( !( m_aProps[nInd].Value >>= bToBeEncr ) )
            {
                SAL_WARN( "package.xstor", "The property has wrong type!" );
            }
        }
    }

    // since a new key set to the package stream it should not be removed except the case when
    // the stream becomes nonencrypted
    uno::Sequence< beans::NamedValue > aKey;
    if ( bToBeEncr )
        GetEncryptionKeyProperty_Impl( xPropSet ) >>= aKey;

    // If the properties must be investigated the stream is either
    // was never changed or was changed, the parent was committed
    // and the stream was closed.
    // That means that if it is intended to use common storage key
    // it is already has no encryption but is marked to be stored
    // encrypted and the key is empty.
    if ( !bWasEncr && bToBeEncr && !aKey.getLength() )
    {
        // the stream is intended to use common storage password
        m_bUseCommonEncryption = true;
        return false;
    }
    else
        return bToBeEncr;
}

void OWriteStream_Impl::SetDecrypted()
{
    SAL_WARN_IF( m_nStorageType != embed::StorageFormats::PACKAGE, "package.xstor", "The encryption is supported only for package storages!" );
    if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        throw uno::RuntimeException();

    GetStreamProperties();

    // let the stream be modified
    FillTempGetFileName();
    m_bHasDataToFlush = true;

    // remove encryption
    m_bForceEncrypted = false;
    m_bHasCachedEncryptionData = false;
    m_aEncryptionData.clear();

    for ( sal_Int32 nInd = 0; nInd < m_aProps.getLength(); nInd++ )
    {
        if ( m_aProps[nInd].Name == "Encrypted" )
            m_aProps[nInd].Value <<= false;
    }
}

void OWriteStream_Impl::SetEncrypted( const ::comphelper::SequenceAsHashMap& aEncryptionData )
{
    SAL_WARN_IF( m_nStorageType != embed::StorageFormats::PACKAGE, "package.xstor", "The encryption is supported only for package storages!" );
    if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        throw uno::RuntimeException();

    if ( aEncryptionData.empty() )
        throw uno::RuntimeException();

    GetStreamProperties();

    // let the stream be modified
    FillTempGetFileName();
    m_bHasDataToFlush = true;

    // introduce encryption info
    for ( sal_Int32 nInd = 0; nInd < m_aProps.getLength(); nInd++ )
    {
        if ( m_aProps[nInd].Name == "Encrypted" )
            m_aProps[nInd].Value <<= true;
    }

    m_bUseCommonEncryption = false; // very important to set it to false

    m_bHasCachedEncryptionData = true;
    m_aEncryptionData = aEncryptionData;
}

void OWriteStream_Impl::DisposeWrappers()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );
    if ( m_pAntiImpl )
    {
        try {
            m_pAntiImpl->dispose();
        }
        catch ( const uno::RuntimeException& rRuntimeException )
        {
            SAL_INFO("package.xstor", "Quiet exception: " << rRuntimeException);
        }

        m_pAntiImpl = nullptr;
    }
    m_pParent = nullptr;

    if ( !m_aInputStreamsVector.empty() )
    {
        for ( auto& pStream : m_aInputStreamsVector )
        {
            if ( pStream )
            {
                pStream->InternalDispose();
                pStream = nullptr;
            }
        }

        m_aInputStreamsVector.clear();
    }
}

OUString const & OWriteStream_Impl::GetFilledTempFileIfNo( const uno::Reference< io::XInputStream >& xStream )
{
    if ( !m_aTempURL.getLength() )
    {
        OUString aTempURL = GetNewTempFileURL( m_xContext );

        try {
            if ( !aTempURL.isEmpty() && xStream.is() )
            {
                uno::Reference < ucb::XSimpleFileAccess3 > xTempAccess( ucb::SimpleFileAccess::create( ::comphelper::getProcessComponentContext() ) );

                uno::Reference< io::XOutputStream > xTempOutStream = xTempAccess->openFileWrite( aTempURL );
                if ( !xTempOutStream.is() )
                    throw io::IOException(); // TODO:
                // the current position of the original stream should be still OK, copy further
                ::comphelper::OStorageHelper::CopyInputToOutput( xStream, xTempOutStream );
                xTempOutStream->closeOutput();
                xTempOutStream.clear();
            }
        }
        catch( const packages::WrongPasswordException& rWrongPasswordException )
        {
            SAL_INFO("package.xstor", "Rethrow: " << rWrongPasswordException);
            KillFile( aTempURL, comphelper::getProcessComponentContext() );
            throw;
        }
        catch( const uno::Exception& rException )
        {
            SAL_INFO("package.xstor", "Rethrow: " << rException);
            KillFile( aTempURL, comphelper::getProcessComponentContext() );
            throw;
        }

        if ( !aTempURL.isEmpty() )
            CleanCacheStream();

        m_aTempURL = aTempURL;
    }

    return m_aTempURL;
}

OUString const & OWriteStream_Impl::FillTempGetFileName()
{
    // should try to create cache first, if the amount of contents is too big, the temp file should be taken
    if ( !m_xCacheStream.is() && m_aTempURL.isEmpty() )
    {
        uno::Reference< io::XInputStream > xOrigStream = m_xPackageStream->getDataStream();
        if ( !xOrigStream.is() )
        {
            // in case of new inserted package stream it is possible that input stream still was not set
            uno::Reference< io::XStream > xCacheStream = CreateMemoryStream( m_xContext );
            SAL_WARN_IF( !xCacheStream.is(), "package.xstor", "If the stream can not be created an exception must be thrown!" );
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
                uno::Reference< io::XStream > xCacheStream = CreateMemoryStream( m_xContext );
                SAL_WARN_IF( !xCacheStream.is(), "package.xstor", "If the stream can not be created an exception must be thrown!" );

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
                m_aTempURL = GetNewTempFileURL( m_xContext );

                try {
                    if ( !m_aTempURL.isEmpty() )
                    {
                        uno::Reference < ucb::XSimpleFileAccess3 > xTempAccess( ucb::SimpleFileAccess::create( ::comphelper::getProcessComponentContext() ) );

                        uno::Reference< io::XOutputStream > xTempOutStream = xTempAccess->openFileWrite( m_aTempURL );
                        if ( !xTempOutStream.is() )
                            throw io::IOException(); // TODO:

                        // copy stream contents to the file
                        xTempOutStream->writeBytes( aData );

                        // the current position of the original stream should be still OK, copy further
                        ::comphelper::OStorageHelper::CopyInputToOutput( xOrigStream, xTempOutStream );
                        xTempOutStream->closeOutput();
                        xTempOutStream.clear();
                    }
                }
                catch( const packages::WrongPasswordException& )
                {
                    KillFile( m_aTempURL, comphelper::getProcessComponentContext() );
                    m_aTempURL.clear();

                    throw;
                }
                catch( const uno::Exception& )
                {
                    KillFile( m_aTempURL, comphelper::getProcessComponentContext() );
                    m_aTempURL.clear();
                }
            }
        }
    }

    return m_aTempURL;
}

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
            uno::Reference < ucb::XSimpleFileAccess3 > xTempAccess( ucb::SimpleFileAccess::create( ::comphelper::getProcessComponentContext() ) );

            try
            {
                xTempStream = xTempAccess->openFileReadWrite( m_aTempURL );
            }
            catch( const uno::Exception& rException )
            {
                SAL_INFO("package.xstor", "Quiet exception: " << rException);
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
            uno::Reference < ucb::XSimpleFileAccess3 > xTempAccess( ucb::SimpleFileAccess::create( ::comphelper::getProcessComponentContext() ) );

            try
            {
                xInputStream = xTempAccess->openFileRead( m_aTempURL );
            }
            catch( const uno::Exception& rException )
            {
                SAL_INFO("package.xstor", "Quiet exception: " << rException);
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

void OWriteStream_Impl::InsertStreamDirectly( const uno::Reference< io::XInputStream >& xInStream,
                                              const uno::Sequence< beans::PropertyValue >& aProps )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() ) ;

    // this call can be made only during parent storage commit
    // the  parent storage is responsible for the correct handling
    // of deleted and renamed contents

    SAL_WARN_IF( !m_xPackageStream.is(), "package.xstor", "No package stream is set!" );

    if ( m_bHasDataToFlush )
        throw io::IOException();

    OSL_ENSURE( m_aTempURL.isEmpty() && !m_xCacheStream.is(), "The temporary must not exist!" );

    // use new file as current persistent representation
    // the new file will be removed after it's stream is closed
    m_xPackageStream->setDataStream( xInStream );

    // copy properties to the package stream
    uno::Reference< beans::XPropertySet > xPropertySet( m_xPackageStream, uno::UNO_QUERY_THROW );

    // The storage-package communication has a problem
    // the storage caches properties, thus if the package changes one of them itself
    // the storage does not know about it

    // Depending from MediaType value the package can change the compressed property itself
    // Thus if Compressed property is provided it must be set as the latest one
    bool bCompressedIsSet = false;
    bool bCompressed = false;
    OUString aComprPropName( "Compressed" );
    OUString aMedTypePropName( "MediaType" );
    for ( sal_Int32 nInd = 0; nInd < aProps.getLength(); nInd++ )
    {
        if ( aProps[nInd].Name == aComprPropName )
        {
            bCompressedIsSet = true;
            aProps[nInd].Value >>= bCompressed;
        }
        else if ( ( m_nStorageType == embed::StorageFormats::OFOPXML || m_nStorageType == embed::StorageFormats::PACKAGE )
               && aProps[nInd].Name == aMedTypePropName )
        {
            xPropertySet->setPropertyValue( aProps[nInd].Name, aProps[nInd].Value );
        }
        else if ( m_nStorageType == embed::StorageFormats::PACKAGE && aProps[nInd].Name == "UseCommonStoragePasswordEncryption" )
            aProps[nInd].Value >>= m_bUseCommonEncryption;
        else
            throw lang::IllegalArgumentException();

        // if there are cached properties update them
        if ( aProps[nInd].Name == aMedTypePropName || aProps[nInd].Name == aComprPropName )
            for ( sal_Int32 nMemInd = 0; nMemInd < m_aProps.getLength(); nMemInd++ )
            {
                if ( aProps[nInd].Name == m_aProps[nMemInd].Name )
                    m_aProps[nMemInd].Value = aProps[nInd].Value;
            }
    }

    if ( bCompressedIsSet )
    {
        xPropertySet->setPropertyValue( aComprPropName, uno::makeAny( bCompressed ) );
        m_bCompressedSetExplicit = true;
    }

    if ( m_bUseCommonEncryption )
    {
        if ( m_nStorageType != embed::StorageFormats::PACKAGE )
            throw uno::RuntimeException();

        // set to be encrypted but do not use encryption key
        xPropertySet->setPropertyValue( STORAGE_ENCRYPTION_KEYS_PROPERTY,
                                        uno::makeAny( uno::Sequence< beans::NamedValue >() ) );
        xPropertySet->setPropertyValue( "Encrypted", uno::makeAny( true ) );
    }

    // the stream should be free soon, after package is stored
    m_bHasDataToFlush = false;
    m_bFlushed = true; // will allow to use transaction on stream level if will need it
    m_bHasInsertedStreamOptimization = true;
}

void OWriteStream_Impl::Commit()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() ) ;

    SAL_WARN_IF( !m_xPackageStream.is(), "package.xstor", "No package stream is set!" );

    if ( !m_bHasDataToFlush )
        return;

    uno::Reference< packages::XDataSinkEncrSupport > xNewPackageStream;
    uno::Sequence< uno::Any > aSeq( 1 );
    aSeq[0] <<= false;

    if ( m_xCacheStream.is() )
    {
        if ( m_pAntiImpl )
            m_pAntiImpl->DeInit();

        uno::Reference< io::XInputStream > xInStream( m_xCacheStream->getInputStream(), uno::UNO_SET_THROW );

        xNewPackageStream.set( m_xPackage->createInstanceWithArguments( aSeq ), uno::UNO_QUERY_THROW );

        xNewPackageStream->setDataStream( xInStream );

        m_xCacheStream.clear();
        m_xCacheSeek.clear();

    }
    else if ( !m_aTempURL.isEmpty() )
    {
        if ( m_pAntiImpl )
            m_pAntiImpl->DeInit();

        uno::Reference< io::XInputStream > xInStream;
        try
        {
            xInStream = new OSelfTerminateFileStream(m_xContext, m_aTempURL);
        }
        catch( const uno::Exception& )
        {
        }

        if ( !xInStream.is() )
            throw io::IOException();

        xNewPackageStream.set( m_xPackage->createInstanceWithArguments( aSeq ), uno::UNO_QUERY_THROW );

        // TODO/NEW: Let the temporary file be removed after commit
        xNewPackageStream->setDataStream( xInStream );
        m_aTempURL.clear();
    }
    else // if ( m_bHasInsertedStreamOptimization )
    {
        // if the optimization is used the stream can be accessed directly
        xNewPackageStream = m_xPackageStream;
    }

    // copy properties to the package stream
    uno::Reference< beans::XPropertySet > xPropertySet( xNewPackageStream, uno::UNO_QUERY_THROW );

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
        xPropertySet->setPropertyValue( STORAGE_ENCRYPTION_KEYS_PROPERTY,
                                        uno::makeAny( uno::Sequence< beans::NamedValue >() ) );
        xPropertySet->setPropertyValue( "Encrypted",
                                        uno::makeAny( true ) );
    }
    else if ( m_bHasCachedEncryptionData )
    {
        if ( m_nStorageType != embed::StorageFormats::PACKAGE )
            throw uno::RuntimeException();

        xPropertySet->setPropertyValue( STORAGE_ENCRYPTION_KEYS_PROPERTY,
                                        uno::makeAny( m_aEncryptionData.getAsConstNamedValueList() ) );
    }

    // the stream should be free soon, after package is stored
    m_xPackageStream = xNewPackageStream;
    m_bHasDataToFlush = false;
    m_bFlushed = true; // will allow to use transaction on stream level if will need it
}

void OWriteStream_Impl::Revert()
{
    // can be called only from parent storage
    // means complete reload of the stream

    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() ) ;

    if ( !m_bHasDataToFlush )
        return; // nothing to do

    OSL_ENSURE( !m_aTempURL.isEmpty() || m_xCacheStream.is(), "The temporary must exist!" );

    if ( m_xCacheStream.is() )
    {
        m_xCacheStream.clear();
        m_xCacheSeek.clear();
    }

    if ( !m_aTempURL.isEmpty() )
    {
        KillFile( m_aTempURL, comphelper::getProcessComponentContext() );
        m_aTempURL.clear();
    }

    m_aProps.realloc( 0 );

    m_bHasDataToFlush = false;

    m_bUseCommonEncryption = true;
    m_bHasCachedEncryptionData = false;
    m_aEncryptionData.clear();

    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        // currently the relations storage is changed only on commit
        m_xNewRelInfoStream.clear();
        m_aNewRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
        if ( m_xOrigRelInfoStream.is() )
        {
            // the original stream is still here, that means that it was not parsed
            m_aOrigRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
            m_nRelInfoStatus = RELINFO_NO_INIT;
        }
        else
        {
            // the original stream was already parsed
            if ( !m_bOrigRelInfoBroken )
                m_nRelInfoStatus = RELINFO_READ;
            else
                m_nRelInfoStatus = RELINFO_BROKEN;
        }
    }
}

uno::Sequence< beans::PropertyValue > const & OWriteStream_Impl::GetStreamProperties()
{
    if ( !m_aProps.getLength() )
        m_aProps = ReadPackageStreamProperties();

    return m_aProps;
}

uno::Sequence< beans::PropertyValue > OWriteStream_Impl::InsertOwnProps(
                                                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                                                    bool bUseCommonEncryption )
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
        aResult[nLen - 1].Name = "UseCommonStoragePasswordEncryption";
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
            throw io::IOException( "Wrong relinfo stream!" );

        for ( sal_Int32 nInd = 0; nInd < nLen; nInd++ )
            if ( aResult[nInd].Name == "RelationsInfo" )
            {
                aResult[nInd].Value = aValue;
                return aResult;
            }

        aResult.realloc( ++nLen );
        aResult[nLen - 1].Name = "RelationsInfo";
        aResult[nLen - 1].Value = aValue;
    }

    return aResult;
}

bool OWriteStream_Impl::IsTransacted()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() ) ;
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
                                        "_rels/*.rels",
                                        m_xContext );

            // in case of success the stream must be thrown away, that means that the OrigRelInfo is initialized
            // the reason for this is that the original stream might not be seekable ( at the same time the new
            // provided stream must be seekable ), so it must be read only once
            m_xOrigRelInfoStream.clear();
            m_nRelInfoStatus = RELINFO_READ;
        }
        catch( const uno::Exception& rException )
        {
            SAL_INFO("package.xstor", "Quiet exception: " << rException);

            m_nRelInfoStatus = RELINFO_BROKEN;
            m_bOrigRelInfoBroken = true;
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
                                        "_rels/*.rels",
                                        m_xContext );

            m_nRelInfoStatus = RELINFO_CHANGED_STREAM_READ;
        }
        catch( const uno::Exception& )
        {
            m_nRelInfoStatus = RELINFO_CHANGED_BROKEN;
        }
    }
}

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

    uno::Reference< beans::XPropertySet > xPropSet( m_xPackageStream, uno::UNO_QUERY_THROW );
    for ( sal_Int32 nInd = 0; nInd < aResult.getLength(); nInd++ )
    {
        try {
            aResult[nInd].Value = xPropSet->getPropertyValue( aResult[nInd].Name );
        }
        catch( const uno::Exception& rException )
        {
            SAL_INFO("package.xstor", "Quiet exception: " << rException);
            SAL_WARN( "package.xstor", "A property can't be retrieved!" );
        }
    }

    return aResult;
}

void OWriteStream_Impl::CopyInternallyTo_Impl( const uno::Reference< io::XStream >& xDestStream,
                                                const ::comphelper::SequenceAsHashMap& aEncryptionData )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() ) ;

    SAL_WARN_IF( m_bUseCommonEncryption, "package.xstor", "The stream can not be encrypted!" );

    if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        throw packages::NoEncryptionException();

    if ( m_pAntiImpl )
    {
        m_pAntiImpl->CopyToStreamInternally_Impl( xDestStream );
    }
    else
    {
        uno::Reference< io::XStream > xOwnStream = GetStream( embed::ElementModes::READ, aEncryptionData, false );
        if ( !xOwnStream.is() )
            throw io::IOException(); // TODO

        OStorage_Impl::completeStorageStreamCopy_Impl( xOwnStream, xDestStream, m_nStorageType, GetAllRelationshipsIfAny() );
    }

    uno::Reference< embed::XEncryptionProtectedSource2 > xEncr( xDestStream, uno::UNO_QUERY );
    if ( xEncr.is() )
        xEncr->setEncryptionData( aEncryptionData.getAsConstNamedValueList() );
}

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
            throw io::IOException( "Wrong relinfo stream!" );
}

void OWriteStream_Impl::CopyInternallyTo_Impl( const uno::Reference< io::XStream >& xDestStream )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() ) ;

    if ( m_pAntiImpl )
    {
        m_pAntiImpl->CopyToStreamInternally_Impl( xDestStream );
    }
    else
    {
        uno::Reference< io::XStream > xOwnStream = GetStream( embed::ElementModes::READ, false );
        if ( !xOwnStream.is() )
            throw io::IOException(); // TODO

        OStorage_Impl::completeStorageStreamCopy_Impl( xOwnStream, xDestStream, m_nStorageType, GetAllRelationshipsIfAny() );
    }
}

uno::Reference< io::XStream > OWriteStream_Impl::GetStream( sal_Int32 nStreamMode, const ::comphelper::SequenceAsHashMap& aEncryptionData, bool bHierarchyAccess )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() ) ;

    SAL_WARN_IF( !m_xPackageStream.is(), "package.xstor", "No package stream is set!" );

    if ( m_pAntiImpl )
        throw io::IOException(); // TODO:

    if ( !IsEncrypted() )
        throw packages::NoEncryptionException();

    uno::Reference< io::XStream > xResultStream;

    uno::Reference< beans::XPropertySet > xPropertySet( m_xPackageStream, uno::UNO_QUERY_THROW );

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

            m_bUseCommonEncryption = false; // very important to set it to false
            m_bHasCachedEncryptionData = true;
            m_aEncryptionData = aEncryptionData;
        }
        catch( const packages::WrongPasswordException& rWrongPasswordException )
        {
            SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
            SAL_INFO("package.xstor", "Rethrow: " << rWrongPasswordException);
            throw;
        }
        catch ( const uno::Exception& rException )
        {
            SAL_INFO("package.xstor", "Quiet exception: " << rException);
            SAL_WARN( "package.xstor", "Can't write encryption related properties!" );
            SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
            throw io::IOException(); // TODO:
        }
    }

    SAL_WARN_IF( !xResultStream.is(), "package.xstor", "In case stream can not be retrieved an exception must be thrown!" );

    return xResultStream;
}

uno::Reference< io::XStream > OWriteStream_Impl::GetStream( sal_Int32 nStreamMode, bool bHierarchyAccess )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() ) ;

    SAL_WARN_IF( !m_xPackageStream.is(), "package.xstor", "No package stream is set!" );

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
            SAL_INFO("package.xstor", "Rethrow: " << rNoEncryptionException);
            throw packages::WrongPasswordException();
        }

        xResultStream = GetStream( nStreamMode, aGlobalEncryptionData, bHierarchyAccess );
    }
    else
        xResultStream = GetStream_Impl( nStreamMode, bHierarchyAccess );

    return xResultStream;
}

uno::Reference< io::XStream > OWriteStream_Impl::GetStream_Impl( sal_Int32 nStreamMode, bool bHierarchyAccess )
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
        SAL_WARN_IF( !xCompStream.is(), "package.xstor", "OInputCompStream MUST provide XStream interfaces!" );

        m_aInputStreamsVector.push_back( pStream );
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
        SAL_WARN_IF( !xSeekStream.is(), "package.xstor", "OInputSeekStream MUST provide XStream interfaces!" );

        m_aInputStreamsVector.push_back( pStream );
        return xSeekStream;
    }
    else if ( ( nStreamMode & embed::ElementModes::WRITE ) == embed::ElementModes::WRITE )
    {
        if ( !m_aInputStreamsVector.empty() )
            throw io::IOException(); // TODO:

        uno::Reference< io::XStream > xStream;
        if ( ( nStreamMode & embed::ElementModes::TRUNCATE ) == embed::ElementModes::TRUNCATE )
        {
            if ( !m_aTempURL.isEmpty() )
            {
                KillFile( m_aTempURL, comphelper::getProcessComponentContext() );
                m_aTempURL.clear();
            }
            if ( m_xCacheStream.is() )
                CleanCacheStream();

            m_bHasDataToFlush = true;

            // this call is triggered by the parent and it will recognize the change of the state
            if ( m_pParent )
                m_pParent->m_bIsModified = true;

            xStream = CreateMemoryStream( m_xContext );
            m_xCacheSeek.set( xStream, uno::UNO_QUERY_THROW );
            m_xCacheStream = xStream;
        }
        else if ( !m_bHasInsertedStreamOptimization )
        {
            if ( m_aTempURL.isEmpty() && !m_xCacheStream.is() && !( m_xPackageStream->getDataStream().is() ) )
            {
                // The stream does not exist in the storage
                m_bHasDataToFlush = true;

                // this call is triggered by the parent and it will recognize the change of the state
                if ( m_pParent )
                    m_pParent->m_bIsModified = true;
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

        SAL_WARN_IF( !xWriteStream.is(), "package.xstor", "OWriteStream MUST implement XStream && XComponent interfaces!" );

        return xWriteStream;
    }

    throw lang::IllegalArgumentException(); // TODO
}

uno::Reference< io::XInputStream > OWriteStream_Impl::GetPlainRawInStream()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() ) ;

    SAL_WARN_IF( !m_xPackageStream.is(), "package.xstor", "No package stream is set!" );

    // this method is used only internally, this stream object should not go outside of this implementation
    // if ( m_pAntiImpl )
    //  throw io::IOException(); // TODO:

    return m_xPackageStream->getPlainRawStream();
}

uno::Reference< io::XInputStream > OWriteStream_Impl::GetRawInStream()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() ) ;

    SAL_WARN_IF( !m_xPackageStream.is(), "package.xstor", "No package stream is set!" );

    if ( m_pAntiImpl )
        throw io::IOException(); // TODO:

    SAL_WARN_IF( !IsEncrypted(), "package.xstor", "Impossible to get raw representation for nonencrypted stream!" );
    if ( !IsEncrypted() )
        throw packages::NoEncryptionException();

    return m_xPackageStream->getRawStream();
}

::comphelper::SequenceAsHashMap OWriteStream_Impl::GetCommonRootEncryptionData()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() ) ;

    if ( m_nStorageType != embed::StorageFormats::PACKAGE || !m_pParent )
        throw packages::NoEncryptionException();

    return m_pParent->GetCommonRootEncryptionData();
}

void OWriteStream_Impl::InputStreamDisposed( OInputCompStream* pStream )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );
    m_aInputStreamsVector.erase(std::remove(m_aInputStreamsVector.begin(), m_aInputStreamsVector.end(), pStream ));
}

void OWriteStream_Impl::CreateReadonlyCopyBasedOnData( const uno::Reference< io::XInputStream >& xDataToCopy, const uno::Sequence< beans::PropertyValue >& aProps, uno::Reference< io::XStream >& xTargetStream )
{
    uno::Reference < io::XStream > xTempFile;
    if ( !xTargetStream.is() )
        xTempFile.set( io::TempFile::create(m_xContext), uno::UNO_QUERY );
    else
        xTempFile = xTargetStream;

    uno::Reference < io::XSeekable > xTempSeek( xTempFile, uno::UNO_QUERY_THROW );

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
        xTargetStream.set(
            static_cast< ::cppu::OWeakObject* >(
                new OInputSeekStream( xInStream, InsertOwnProps( aProps, m_bUseCommonEncryption ), m_nStorageType ) ),
            uno::UNO_QUERY_THROW );
}

void OWriteStream_Impl::GetCopyOfLastCommit( uno::Reference< io::XStream >& xTargetStream )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    SAL_WARN_IF( !m_xPackageStream.is(), "package.xstor", "The source stream for copying is incomplete!" );
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
            SAL_INFO("package.xstor", "No Element: " << rNoEncryptionException);
            throw packages::WrongPasswordException();
        }

        GetCopyOfLastCommit( xTargetStream, aGlobalEncryptionData );
    }
    else
    {
        xDataToCopy = m_xPackageStream->getDataStream();

        // in case of new inserted package stream it is possible that input stream still was not set
        GetStreamProperties();

        CreateReadonlyCopyBasedOnData( xDataToCopy, m_aProps, xTargetStream );
    }
}

void OWriteStream_Impl::GetCopyOfLastCommit( uno::Reference< io::XStream >& xTargetStream, const ::comphelper::SequenceAsHashMap& aEncryptionData )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    SAL_WARN_IF( !m_xPackageStream.is(), "package.xstor", "The source stream for copying is incomplete!" );
    if ( !m_xPackageStream.is() )
        throw uno::RuntimeException();

    if ( !IsEncrypted() )
        throw packages::NoEncryptionException();

    uno::Reference< io::XInputStream > xDataToCopy;

    if ( m_bHasCachedEncryptionData )
    {
        // TODO: introduce last committed cashed password information and use it here
        // that means "use common pass" also should be remembered on flash
        uno::Sequence< beans::NamedValue > aKey = aEncryptionData.getAsConstNamedValueList();

        uno::Reference< beans::XPropertySet > xProps( m_xPackageStream, uno::UNO_QUERY_THROW );

        bool bEncr = false;
        xProps->getPropertyValue( "Encrypted" ) >>= bEncr;
        if ( !bEncr )
            throw packages::NoEncryptionException();

        uno::Sequence< beans::NamedValue > aPackKey;
        xProps->getPropertyValue( STORAGE_ENCRYPTION_KEYS_PROPERTY ) >>= aPackKey;
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
                SAL_WARN( "package.xstor", "Encrypted ZipStream must already have input stream inside!" );
                SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
            }
        }
        catch( const uno::Exception& rException )
        {
            SAL_WARN( "package.xstor", "Can't open encrypted stream!" );
            SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
            SAL_INFO("package.xstor", "Rethrow: " << rException);
            throw;
        }

        SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
    }

    // in case of new inserted package stream it is possible that input stream still was not set
    GetStreamProperties();

    CreateReadonlyCopyBasedOnData( xDataToCopy, m_aProps, xTargetStream );
}

void OWriteStream_Impl::CommitStreamRelInfo( const uno::Reference< embed::XStorage >& xRelStorage, const OUString& aOrigStreamName, const OUString& aNewStreamName )
{
    // at this point of time the old stream must be already cleaned
    OSL_ENSURE( m_nStorageType == embed::StorageFormats::OFOPXML, "The method should be used only with OFOPXML format!" );

    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        OSL_ENSURE( !aOrigStreamName.isEmpty() && !aNewStreamName.isEmpty() && xRelStorage.is(),
                    "Wrong relation persistence information is provided!" );

        if ( !xRelStorage.is() || aOrigStreamName.isEmpty() || aNewStreamName.isEmpty() )
            throw uno::RuntimeException();

        if ( m_nRelInfoStatus == RELINFO_BROKEN || m_nRelInfoStatus == RELINFO_CHANGED_BROKEN )
            throw io::IOException(); // TODO:

        OUString aOrigRelStreamName = aOrigStreamName;
        aOrigRelStreamName += ".rels";

        OUString aNewRelStreamName = aNewStreamName;
        aNewRelStreamName += ".rels";

        bool bRenamed = aOrigRelStreamName != aNewRelStreamName;
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

                    ::comphelper::OFOPXMLHelper::WriteRelationsInfoSequence( xOutStream, m_aNewRelInfo, m_xContext );

                    // set the mediatype
                    uno::Reference< beans::XPropertySet > xPropSet( xRelsStream, uno::UNO_QUERY_THROW );
                    xPropSet->setPropertyValue(
                        "MediaType",
                        uno::makeAny( OUString("application/vnd.openxmlformats-package.relationships+xml" ) ) );

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
                xPropSet->setPropertyValue("MediaType",
                    uno::makeAny( OUString("application/vnd.openxmlformats-package.relationships+xml" ) ) );

                if ( m_nRelInfoStatus == RELINFO_CHANGED_STREAM )
                    m_nRelInfoStatus = RELINFO_NO_INIT;
                else
                {
                    // the information is already parsed and the stream is stored, no need in temporary stream any more
                    m_xNewRelInfoStream.clear();
                    m_nRelInfoStatus = RELINFO_READ;
                }
            }

            // the original stream makes no sense after this step
            m_xOrigRelInfoStream = m_xNewRelInfoStream;
            m_aOrigRelInfo = m_aNewRelInfo;
            m_bOrigRelInfoBroken = false;
            m_aNewRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
            m_xNewRelInfoStream.clear();
        }
        else
        {
            // the stream is not changed but it might be renamed
            if ( bRenamed && xRelStorage->hasByName( aOrigRelStreamName ) )
                xRelStorage->renameElement( aOrigRelStreamName, aNewRelStreamName );
        }
    }
}

// OWriteStream implementation

OWriteStream::OWriteStream( OWriteStream_Impl* pImpl, bool bTransacted )
: m_pImpl( pImpl )
, m_bInStreamDisconnected( false )
, m_bInitOnDemand( true )
, m_nInitPosition( 0 )
, m_bTransacted( bTransacted )
{
    OSL_ENSURE( pImpl, "No base implementation!" );
    OSL_ENSURE( m_pImpl->m_xMutex.is(), "No mutex!" );

    if ( !m_pImpl || !m_pImpl->m_xMutex.is() )
        throw uno::RuntimeException(); // just a disaster

    m_pData.reset(new WSInternalData_Impl(pImpl->m_xMutex, m_pImpl->m_nStorageType));
}

OWriteStream::OWriteStream( OWriteStream_Impl* pImpl, uno::Reference< io::XStream > const & xStream, bool bTransacted )
: m_pImpl( pImpl )
, m_bInStreamDisconnected( false )
, m_bInitOnDemand( false )
, m_nInitPosition( 0 )
, m_bTransacted( bTransacted )
{
    OSL_ENSURE( pImpl && xStream.is(), "No base implementation!" );
    OSL_ENSURE( m_pImpl->m_xMutex.is(), "No mutex!" );

    if ( !m_pImpl || !m_pImpl->m_xMutex.is() )
        throw uno::RuntimeException(); // just a disaster

    m_pData.reset(new WSInternalData_Impl(pImpl->m_xMutex, m_pImpl->m_nStorageType));

    if ( xStream.is() )
    {
        m_xInStream = xStream->getInputStream();
        m_xOutStream = xStream->getOutputStream();
        m_xSeekable.set( xStream, uno::UNO_QUERY );
        OSL_ENSURE( m_xInStream.is() && m_xOutStream.is() && m_xSeekable.is(), "Stream implementation is incomplete!" );
    }
}

OWriteStream::~OWriteStream()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );
    if ( m_pImpl )
    {
        m_refCount++;
        try {
            dispose();
        }
        catch( const uno::RuntimeException& rRuntimeException )
        {
            SAL_INFO("package.xstor", "Quiet exception: " << rRuntimeException);
        }
    }
}

void OWriteStream::DeInit()
{
    if ( !m_pImpl )
        return; // do nothing

    if ( m_xSeekable.is() )
        m_nInitPosition = m_xSeekable->getPosition();

    m_xInStream.clear();
    m_xOutStream.clear();
    m_xSeekable.clear();
    m_bInitOnDemand = true;
}

void OWriteStream::CheckInitOnDemand()
{
    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_bInitOnDemand )
    {
        SAL_INFO( "package.xstor", "package (mv76033) OWriteStream::CheckInitOnDemand, initializing" );
        uno::Reference< io::XStream > xStream = m_pImpl->GetTempFileAsStream();
        if ( xStream.is() )
        {
            m_xInStream.set( xStream->getInputStream(), uno::UNO_SET_THROW );
            m_xOutStream.set( xStream->getOutputStream(), uno::UNO_SET_THROW );
            m_xSeekable.set( xStream, uno::UNO_QUERY_THROW );
            m_xSeekable->seek( m_nInitPosition );

            m_nInitPosition = 0;
            m_bInitOnDemand = false;
        }
    }
}

void OWriteStream::CopyToStreamInternally_Impl( const uno::Reference< io::XStream >& xDest )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_xInStream.is() )
        throw uno::RuntimeException();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    uno::Reference< beans::XPropertySet > xDestProps( xDest, uno::UNO_QUERY_THROW );

    uno::Reference< io::XOutputStream > xDestOutStream = xDest->getOutputStream();
    if ( !xDestOutStream.is() )
        throw io::IOException(); // TODO

    sal_Int64 nCurPos = m_xSeekable->getPosition();
    m_xSeekable->seek( 0 );

    uno::Exception eThrown;
    bool bThrown = false;
    try {
        ::comphelper::OStorageHelper::CopyInputToOutput( m_xInStream, xDestOutStream );
    }
    catch ( const uno::Exception& e )
    {
        eThrown = e;
        bThrown = true;
    }

    // position-related section below is critical
    // if it fails the stream will become invalid
    try {
        m_xSeekable->seek( nCurPos );
    }
    catch ( const uno::Exception& rException )
    {
        SAL_INFO("package.xstor", "Quiet exception: " << rException);
        // TODO: set the stream in invalid state or dispose
        SAL_WARN( "package.xstor", "The stream become invalid during copiing!" );
        throw uno::RuntimeException();
    }

    if ( bThrown )
        throw eThrown;

    // now the properties can be copied
    // the order of the properties setting is not important for StorageStream API
    OUString aPropName ("Compressed");
    xDestProps->setPropertyValue( aPropName, getPropertyValue( aPropName ) );
    if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE || m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        aPropName = "MediaType";
        xDestProps->setPropertyValue( aPropName, getPropertyValue( aPropName ) );

        if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
        {
            aPropName = "UseCommonStoragePasswordEncryption";
            xDestProps->setPropertyValue( aPropName, getPropertyValue( aPropName ) );
        }
    }
}

void OWriteStream::ModifyParentUnlockMutex_Impl( ::osl::ResettableMutexGuard& aGuard )
{
    if ( m_pImpl->m_pParent )
    {
        if ( m_pImpl->m_pParent->HasModifiedListener() )
        {
            uno::Reference< util::XModifiable > xParentModif( static_cast<util::XModifiable*>(m_pImpl->m_pParent->m_pAntiImpl) );
            aGuard.clear();
            xParentModif->setModified( true );
        }
        else
            m_pImpl->m_pParent->m_bIsModified = true;
    }
}

uno::Any SAL_CALL OWriteStream::queryInterface( const uno::Type& rType )
{
    uno::Any aReturn;

    // common interfaces
    aReturn = ::cppu::queryInterface
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

    if ( aReturn.hasValue() )
        return aReturn ;

    if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        aReturn = ::cppu::queryInterface
                    (   rType
                        ,   static_cast<embed::XEncryptionProtectedSource2*> ( this )
                        ,   static_cast<embed::XEncryptionProtectedSource*> ( this ) );
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        aReturn = ::cppu::queryInterface
                    (   rType
                        ,   static_cast<embed::XRelationshipAccess*> ( this ) );
    }

    if ( aReturn.hasValue() )
        return aReturn ;

    if ( m_bTransacted )
    {
        aReturn = ::cppu::queryInterface
                    (   rType
                        ,   static_cast<embed::XTransactedObject*> ( this )
                        ,   static_cast<embed::XTransactionBroadcaster*> ( this ) );

        if ( aReturn.hasValue() )
            return aReturn ;
    }

    return OWeakObject::queryInterface( rType );
}

void SAL_CALL OWriteStream::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL OWriteStream::release() throw()
{
    OWeakObject::release();
}

uno::Sequence< uno::Type > SAL_CALL OWriteStream::getTypes()
{
    if (! m_pData->m_pTypeCollection)
    {
        ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

        if (! m_pData->m_pTypeCollection)
        {
            if ( m_bTransacted )
            {
                if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
                {
                    ::cppu::OTypeCollection aTmpCollection
                                    (   cppu::UnoType<lang::XTypeProvider>::get()
                                    ,   cppu::UnoType<io::XInputStream>::get()
                                    ,   cppu::UnoType<io::XOutputStream>::get()
                                    ,   cppu::UnoType<io::XStream>::get()
                                    ,   cppu::UnoType<io::XSeekable>::get()
                                    ,   cppu::UnoType<io::XTruncate>::get()
                                    ,   cppu::UnoType<lang::XComponent>::get()
                                    ,   cppu::UnoType<embed::XEncryptionProtectedSource2>::get()
                                    ,   cppu::UnoType<embed::XEncryptionProtectedSource>::get()
                                    ,   cppu::UnoType<embed::XExtendedStorageStream>::get()
                                    ,   cppu::UnoType<embed::XTransactedObject>::get()
                                    ,   cppu::UnoType<embed::XTransactionBroadcaster>::get());

                    m_pData->m_pTypeCollection.reset(new ::cppu::OTypeCollection
                                    (   cppu::UnoType<beans::XPropertySet>::get()
                                    ,   aTmpCollection.getTypes()));
                }
                else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
                {
                    m_pData->m_pTypeCollection.reset(new ::cppu::OTypeCollection
                                    (   cppu::UnoType<lang::XTypeProvider>::get()
                                    ,   cppu::UnoType<io::XInputStream>::get()
                                    ,   cppu::UnoType<io::XOutputStream>::get()
                                    ,   cppu::UnoType<io::XStream>::get()
                                    ,   cppu::UnoType<io::XSeekable>::get()
                                    ,   cppu::UnoType<io::XTruncate>::get()
                                    ,   cppu::UnoType<lang::XComponent>::get()
                                    ,   cppu::UnoType<embed::XRelationshipAccess>::get()
                                    ,   cppu::UnoType<embed::XExtendedStorageStream>::get()
                                    ,   cppu::UnoType<embed::XTransactedObject>::get()
                                    ,   cppu::UnoType<embed::XTransactionBroadcaster>::get()
                                    ,   cppu::UnoType<beans::XPropertySet>::get()));
                }
                else // if ( m_pData->m_nStorageType == embed::StorageFormats::ZIP )
                {
                    m_pData->m_pTypeCollection.reset(new ::cppu::OTypeCollection
                                    (   cppu::UnoType<lang::XTypeProvider>::get()
                                    ,   cppu::UnoType<io::XInputStream>::get()
                                    ,   cppu::UnoType<io::XOutputStream>::get()
                                    ,   cppu::UnoType<io::XStream>::get()
                                    ,   cppu::UnoType<io::XSeekable>::get()
                                    ,   cppu::UnoType<io::XTruncate>::get()
                                    ,   cppu::UnoType<lang::XComponent>::get()
                                    ,   cppu::UnoType<embed::XExtendedStorageStream>::get()
                                    ,   cppu::UnoType<embed::XTransactedObject>::get()
                                    ,   cppu::UnoType<embed::XTransactionBroadcaster>::get()
                                    ,   cppu::UnoType<beans::XPropertySet>::get()));
                }
            }
            else
            {
                if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
                {
                    m_pData->m_pTypeCollection.reset(new ::cppu::OTypeCollection
                                    (   cppu::UnoType<lang::XTypeProvider>::get()
                                    ,   cppu::UnoType<io::XInputStream>::get()
                                    ,   cppu::UnoType<io::XOutputStream>::get()
                                    ,   cppu::UnoType<io::XStream>::get()
                                    ,   cppu::UnoType<io::XSeekable>::get()
                                    ,   cppu::UnoType<io::XTruncate>::get()
                                    ,   cppu::UnoType<lang::XComponent>::get()
                                    ,   cppu::UnoType<embed::XEncryptionProtectedSource2>::get()
                                    ,   cppu::UnoType<embed::XEncryptionProtectedSource>::get()
                                    ,   cppu::UnoType<beans::XPropertySet>::get()));
                }
                else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
                {
                    m_pData->m_pTypeCollection.reset(new ::cppu::OTypeCollection
                                    (   cppu::UnoType<lang::XTypeProvider>::get()
                                    ,   cppu::UnoType<io::XInputStream>::get()
                                    ,   cppu::UnoType<io::XOutputStream>::get()
                                    ,   cppu::UnoType<io::XStream>::get()
                                    ,   cppu::UnoType<io::XSeekable>::get()
                                    ,   cppu::UnoType<io::XTruncate>::get()
                                    ,   cppu::UnoType<lang::XComponent>::get()
                                    ,   cppu::UnoType<embed::XRelationshipAccess>::get()
                                    ,   cppu::UnoType<beans::XPropertySet>::get()));
                }
                else // if ( m_pData->m_nStorageType == embed::StorageFormats::ZIP )
                {
                    m_pData->m_pTypeCollection.reset(new ::cppu::OTypeCollection
                                    (   cppu::UnoType<lang::XTypeProvider>::get()
                                    ,   cppu::UnoType<io::XInputStream>::get()
                                    ,   cppu::UnoType<io::XOutputStream>::get()
                                    ,   cppu::UnoType<io::XStream>::get()
                                    ,   cppu::UnoType<io::XSeekable>::get()
                                    ,   cppu::UnoType<io::XTruncate>::get()
                                    ,   cppu::UnoType<lang::XComponent>::get()
                                    ,   cppu::UnoType<beans::XPropertySet>::get()));
                }
            }
        }
    }

    return m_pData->m_pTypeCollection->getTypes() ;
}

namespace { struct lcl_ImplId : public rtl::Static< ::cppu::OImplementationId, lcl_ImplId > {}; }

uno::Sequence< sal_Int8 > SAL_CALL OWriteStream::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

sal_Int32 SAL_CALL OWriteStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    return m_xInStream->readBytes( aData, nBytesToRead );
}

sal_Int32 SAL_CALL OWriteStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    return m_xInStream->readSomeBytes( aData, nMaxBytesToRead );
}

void SAL_CALL OWriteStream::skipBytes( sal_Int32 nBytesToSkip )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    m_xInStream->skipBytes( nBytesToSkip );
}

sal_Int32 SAL_CALL OWriteStream::available(  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    return m_xInStream->available();

}

void SAL_CALL OWriteStream::closeInput(  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_bInitOnDemand && ( m_bInStreamDisconnected || !m_xInStream.is() ) )
        throw io::NotConnectedException();

    // the input part of the stream stays open for internal purposes ( to allow reading during copiing )
    // since it can not be reopened until output part is closed, it will be closed with output part.
    m_bInStreamDisconnected = true;
    // m_xInStream->closeInput();
    // m_xInStream.clear();

    if ( !m_xOutStream.is() )
        dispose();
}

uno::Reference< io::XInputStream > SAL_CALL OWriteStream::getInputStream()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_bInitOnDemand && ( m_bInStreamDisconnected || !m_xInStream.is() ) )
        return uno::Reference< io::XInputStream >();

    return uno::Reference< io::XInputStream >( static_cast< io::XInputStream* >( this ), uno::UNO_QUERY );
}

uno::Reference< io::XOutputStream > SAL_CALL OWriteStream::getOutputStream()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    try
    {
        CheckInitOnDemand();
    }
    catch( const io::IOException& r )
    {
        throw lang::WrappedTargetRuntimeException("OWriteStream::getOutputStream: Could not create backing temp file",
                static_cast < OWeakObject * > ( this ), makeAny ( r ) );
    }

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_xOutStream.is() )
        return uno::Reference< io::XOutputStream >();

    return uno::Reference< io::XOutputStream >( static_cast< io::XOutputStream* >( this ), uno::UNO_QUERY );
}

void SAL_CALL OWriteStream::writeBytes( const uno::Sequence< sal_Int8 >& aData )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    // the write method makes initialization itself, since it depends from the aData length
    // NO CheckInitOnDemand()!

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
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
        SAL_INFO( "package.xstor", "package (mv76033) OWriteStream::CheckInitOnDemand, initializing" );
        uno::Reference< io::XStream > xStream = m_pImpl->GetTempFileAsStream();
        if ( xStream.is() )
        {
            m_xInStream.set( xStream->getInputStream(), uno::UNO_SET_THROW );
            m_xOutStream.set( xStream->getOutputStream(), uno::UNO_SET_THROW );
            m_xSeekable.set( xStream, uno::UNO_QUERY_THROW );
            m_xSeekable->seek( m_nInitPosition );

            m_nInitPosition = 0;
            m_bInitOnDemand = false;
        }
    }

    if ( !m_xOutStream.is() )
        throw io::NotConnectedException();

    m_xOutStream->writeBytes( aData );
    m_pImpl->m_bHasDataToFlush = true;

    ModifyParentUnlockMutex_Impl( aGuard );
}

void SAL_CALL OWriteStream::flush()
{
    // In case stream is flushed its current version becomes visible
    // to the parent storage. Usually parent storage flushes the stream
    // during own commit but a user can explicitly flush the stream
    // so the changes will be available through cloning functionality.

    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
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

void OWriteStream::CloseOutput_Impl()
{
    // all the checks must be done in calling method

    m_xOutStream->closeOutput();
    m_xOutStream.clear();

    if ( !m_bInitOnDemand )
    {
        // after the stream is disposed it can be committed
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

void SAL_CALL OWriteStream::closeOutput()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_xOutStream.is() )
        throw io::NotConnectedException();

    CloseOutput_Impl();

    if ( m_bInStreamDisconnected || !m_xInStream.is() )
        dispose();
}

void SAL_CALL OWriteStream::seek( sal_Int64 location )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    m_xSeekable->seek( location );
}

sal_Int64 SAL_CALL OWriteStream::getPosition()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getPosition();
}

sal_Int64 SAL_CALL OWriteStream::getLength()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getLength();
}

void SAL_CALL OWriteStream::truncate()
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_xOutStream.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XTruncate > xTruncate( m_xOutStream, uno::UNO_QUERY_THROW );
    xTruncate->truncate();

    m_pImpl->m_bHasDataToFlush = true;

    ModifyParentUnlockMutex_Impl( aGuard );
}

void SAL_CALL OWriteStream::dispose()
{
    // should be an internal method since it can be called only from parent storage
    {
        ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

        if ( !m_pImpl )
        {
            SAL_INFO("package.xstor", "Disposed!");
            throw lang::DisposedException();
        }

        if ( m_xOutStream.is() )
            CloseOutput_Impl();

        if ( m_xInStream.is() )
        {
            m_xInStream->closeInput();
            m_xInStream.clear();
        }

        m_xSeekable.clear();

        m_pImpl->m_pAntiImpl = nullptr;

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
            catch( const uno::Exception& )
            {
                uno::Any aCaught( ::cppu::getCaughtException() );
                SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));
                throw lang::WrappedTargetRuntimeException("Can not commit/revert the storage!",
                                                static_cast< OWeakObject* >( this ),
                                                aCaught );
            }
        }

        m_pImpl = nullptr;
    }

    // the listener might try to get rid of parent storage, and the storage would delete this object;
    // for now the listener is just notified at the end of the method to workaround the problem
    // in future a more elegant way should be found

    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
    m_pData->m_aListenersContainer.disposeAndClear( aSource );
}

void SAL_CALL OWriteStream::addEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    m_pData->m_aListenersContainer.addInterface( cppu::UnoType<lang::XEventListener>::get(),
                                                 xListener );
}

void SAL_CALL OWriteStream::removeEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    m_pData->m_aListenersContainer.removeInterface( cppu::UnoType<lang::XEventListener>::get(),
                                                    xListener );
}

void SAL_CALL OWriteStream::setEncryptionPassword( const OUString& aPass )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    OSL_ENSURE( m_pImpl->m_xPackageStream.is(), "No package stream is set!" );

    m_pImpl->SetEncrypted( ::comphelper::OStorageHelper::CreatePackageEncryptionData( aPass ) );

    ModifyParentUnlockMutex_Impl( aGuard );
}

void SAL_CALL OWriteStream::removeEncryption()
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    OSL_ENSURE( m_pImpl->m_xPackageStream.is(), "No package stream is set!" );

    m_pImpl->SetDecrypted();

    ModifyParentUnlockMutex_Impl( aGuard );
}

void SAL_CALL OWriteStream::setEncryptionData( const uno::Sequence< beans::NamedValue >& aEncryptionData )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    OSL_ENSURE( m_pImpl->m_xPackageStream.is(), "No package stream is set!" );

    m_pImpl->SetEncrypted( aEncryptionData );

    ModifyParentUnlockMutex_Impl( aGuard );
}

sal_Bool SAL_CALL OWriteStream::hasEncryptionData()
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if (!m_pImpl)
        return false;

    bool bRet = false;

    try
    {
        bRet = m_pImpl->IsEncrypted();

        if (!bRet && m_pImpl->m_bUseCommonEncryption && m_pImpl->m_pParent)
            bRet = m_pImpl->m_pParent->m_bHasCommonEncryptionData;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        SAL_INFO("package.xstor", "Rethrow: " << rRuntimeException);
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));
        throw lang::WrappedTargetRuntimeException( "Problems on hasEncryptionData!",
                                  static_cast< ::cppu::OWeakObject* >( this ),
                                  aCaught );
    }

    return bRet;
}

sal_Bool SAL_CALL OWriteStream::hasByID(  const OUString& sID )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    try
    {
        getRelationshipByID( sID );
        return true;
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        SAL_INFO("package.xstor", "No Element: " << rNoSuchElementException);
    }

    return false;
}

OUString SAL_CALL OWriteStream::getTargetByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( aSeq[nInd].First == "Target" )
            return aSeq[nInd].Second;

    return OUString();
}

OUString SAL_CALL OWriteStream::getTypeByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( aSeq[nInd].First == "Type" )
            return aSeq[nInd].Second;

    return OUString();
}

uno::Sequence< beans::StringPair > SAL_CALL OWriteStream::getRelationshipByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
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
                if ( aSeq[nInd1][nInd2].Second == sID )
                    return aSeq[nInd1];
                break;
            }

    throw container::NoSuchElementException();
}

uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OWriteStream::getRelationshipsByType(  const OUString& sType  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
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
                if ( aSeq[nInd1][nInd2].Second == sType )
                {
                    aResult.realloc( nEntriesNum );
                    aResult[nEntriesNum-1] = aSeq[nInd1];
                }
                break;
            }

    return aResult;
}

uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OWriteStream::getAllRelationships()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    return m_pImpl->GetAllRelationshipsIfAny();
}

void SAL_CALL OWriteStream::insertRelationshipByID(  const OUString& sID, const uno::Sequence< beans::StringPair >& aEntry, sal_Bool bReplace  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    OUString aIDTag( "Id" );

    sal_Int32 nIDInd = -1;

    // TODO/LATER: in future the unification of the ID could be checked
    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    for ( sal_Int32 nInd1 = 0; nInd1 < aSeq.getLength(); nInd1++ )
        for ( sal_Int32 nInd2 = 0; nInd2 < aSeq[nInd1].getLength(); nInd2++ )
            if ( aSeq[nInd1][nInd2].First == aIDTag )
            {
                if ( aSeq[nInd1][nInd2].Second == sID )
                    nIDInd = nInd1;

                break;
            }

    if ( nIDInd != -1 && !bReplace )
        throw container::ElementExistException(); // TODO

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
        if ( aEntry[nIndOrig].First != aIDTag )
            aSeq[nIDInd][nIndTarget++] = aEntry[nIndOrig];
    }

    aSeq[nIDInd].realloc( nIndTarget );

    m_pImpl->m_aNewRelInfo = aSeq;
    m_pImpl->m_xNewRelInfoStream.clear();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

void SAL_CALL OWriteStream::removeRelationshipByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    for ( sal_Int32 nInd1 = 0; nInd1 < aSeq.getLength(); nInd1++ )
        for ( sal_Int32 nInd2 = 0; nInd2 < aSeq[nInd1].getLength(); nInd2++ )
            if ( aSeq[nInd1][nInd2].First == "Id" )
            {
                if ( aSeq[nInd1][nInd2].Second == sID )
                {
                    sal_Int32 nLength = aSeq.getLength();
                    aSeq[nInd1] = aSeq[nLength-1];
                    aSeq.realloc( nLength - 1 );

                    m_pImpl->m_aNewRelInfo = aSeq;
                    m_pImpl->m_xNewRelInfoStream.clear();
                    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;

                    // TODO/LATER: in future the unification of the ID could be checked
                    return;
                }

                break;
            }

    throw container::NoSuchElementException();
}

void SAL_CALL OWriteStream::insertRelationships(  const uno::Sequence< uno::Sequence< beans::StringPair > >& aEntries, sal_Bool bReplace  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    OUString aIDTag( "Id" );
    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    uno::Sequence< uno::Sequence< beans::StringPair > > aResultSeq( aSeq.getLength() + aEntries.getLength() );
    sal_Int32 nResultInd = 0;

    for ( sal_Int32 nIndTarget1 = 0; nIndTarget1 < aSeq.getLength(); nIndTarget1++ )
        for ( sal_Int32 nIndTarget2 = 0; nIndTarget2 < aSeq[nIndTarget1].getLength(); nIndTarget2++ )
            if ( aSeq[nIndTarget1][nIndTarget2].First == aIDTag )
            {
                sal_Int32 nIndSourceSame = -1;

                for ( sal_Int32 nIndSource1 = 0; nIndSource1 < aEntries.getLength(); nIndSource1++ )
                    for ( sal_Int32 nIndSource2 = 0; nIndSource2 < aEntries[nIndSource1].getLength(); nIndSource2++ )
                    {
                        if ( aEntries[nIndSource1][nIndSource2].First == aIDTag )
                        {
                            if ( aEntries[nIndSource1][nIndSource2].Second == aSeq[nIndTarget1][nIndTarget2].Second )
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
        bool bHasID = false;
        sal_Int32 nResInd2 = 1;

        for ( sal_Int32 nIndSource2 = 0; nIndSource2 < aEntries[nIndSource1].getLength(); nIndSource2++ )
            if ( aEntries[nIndSource1][nIndSource2].First == aIDTag )
            {
                aResultSeq[nResultInd][0] = aEntries[nIndSource1][nIndSource2];
                bHasID = true;
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
    m_pImpl->m_xNewRelInfoStream.clear();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

void SAL_CALL OWriteStream::clearRelationships()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    m_pImpl->m_aNewRelInfo.realloc( 0 );
    m_pImpl->m_xNewRelInfoStream.clear();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL OWriteStream::getPropertySetInfo()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    //TODO:
    return uno::Reference< beans::XPropertySetInfo >();
}

void SAL_CALL OWriteStream::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    m_pImpl->GetStreamProperties();
    OUString aCompressedString( "Compressed" );
    OUString aMediaTypeString( "MediaType" );
    if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE && aPropertyName == aMediaTypeString )
    {
        // if the "Compressed" property is not set explicitly, the MediaType can change the default value
        bool bCompressedValueFromType = true;
        OUString aType;
        aValue >>= aType;

        if ( !m_pImpl->m_bCompressedSetExplicit )
        {
            if ( aType == "image/jpeg" || aType == "image/png" || aType == "image/gif" )
                bCompressedValueFromType = false;
        }

        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( aPropertyName == m_pImpl->m_aProps[nInd].Name )
                m_pImpl->m_aProps[nInd].Value = aValue;
            else if ( !m_pImpl->m_bCompressedSetExplicit && aCompressedString == m_pImpl->m_aProps[nInd].Name )
                m_pImpl->m_aProps[nInd].Value <<= bCompressedValueFromType;
        }
    }
    else if ( aPropertyName == aCompressedString )
    {
        // if the "Compressed" property is not set explicitly, the MediaType can change the default value
        m_pImpl->m_bCompressedSetExplicit = true;
        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( aPropertyName == m_pImpl->m_aProps[nInd].Name )
                m_pImpl->m_aProps[nInd].Value = aValue;
        }
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE
            && aPropertyName == "UseCommonStoragePasswordEncryption" )
    {
        bool bUseCommonEncryption = false;
        if ( !(aValue >>= bUseCommonEncryption) )
            throw lang::IllegalArgumentException(); //TODO

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
                m_pImpl->m_bUseCommonEncryption = true;
            }
        }
        else
            m_pImpl->m_bUseCommonEncryption = false;
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aPropertyName == aMediaTypeString )
    {
        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( aPropertyName == m_pImpl->m_aProps[nInd].Name )
                m_pImpl->m_aProps[nInd].Value = aValue;
        }
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aPropertyName == "RelationsInfoStream" )
    {
        uno::Reference< io::XInputStream > xInRelStream;
        if ( !( aValue >>= xInRelStream ) || !xInRelStream.is() )
            throw lang::IllegalArgumentException(); // TODO

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
    else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aPropertyName == "RelationsInfo" )
    {
        if ( !(aValue >>= m_pImpl->m_aNewRelInfo) )
            throw lang::IllegalArgumentException(); // TODO
    }
    else if ( aPropertyName == "Size" )
        throw beans::PropertyVetoException(); // TODO
    else if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE
           && ( aPropertyName == "IsEncrypted" || aPropertyName == "Encrypted" ) )
        throw beans::PropertyVetoException(); // TODO
    else if ( aPropertyName == "RelId" )
    {
        aValue >>= m_pImpl->m_nRelId;
    }
    else
        throw beans::UnknownPropertyException(); // TODO

    m_pImpl->m_bHasDataToFlush = true;
    ModifyParentUnlockMutex_Impl( aGuard );
}

uno::Any SAL_CALL OWriteStream::getPropertyValue( const OUString& aProp )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( aProp == "RelId" )
    {
        return uno::makeAny( m_pImpl->GetNewRelId() );
    }

    OUString aPropertyName;
    if ( aProp == "IsEncrypted" )
        aPropertyName = "Encrypted";
    else
        aPropertyName = aProp;

    if ( ( ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE || m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
            && aPropertyName == "MediaType" )
      || ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE && aPropertyName == "Encrypted" )
      || aPropertyName == "Compressed" )
    {
        m_pImpl->GetStreamProperties();

        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( aPropertyName == m_pImpl->m_aProps[nInd].Name )
                return m_pImpl->m_aProps[nInd].Value;
        }
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE
            && aPropertyName == "UseCommonStoragePasswordEncryption" )
        return uno::makeAny( m_pImpl->m_bUseCommonEncryption );
    else if ( aPropertyName == "Size" )
    {
        bool bThrow = false;
        try
        {
            CheckInitOnDemand();
        }
        catch (const io::IOException&)
        {
            bThrow = true;
        }
        if (bThrow || !m_xSeekable.is())
            throw uno::RuntimeException();

        return uno::makeAny( m_xSeekable->getLength() );
    }

    throw beans::UnknownPropertyException(); // TODO
}

void SAL_CALL OWriteStream::addPropertyChangeListener(
    const OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    //TODO:
}

void SAL_CALL OWriteStream::removePropertyChangeListener(
    const OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    //TODO:
}

void SAL_CALL OWriteStream::addVetoableChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    //TODO:
}

void SAL_CALL OWriteStream::removeVetoableChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    //TODO:
}

//  XTransactedObject

void OWriteStream::BroadcastTransaction( sal_Int8 nMessage )
/*
    1 - preCommit
    2 - committed
    3 - preRevert
    4 - reverted
*/
{
    // no need to lock mutex here for the checking of m_pImpl, and m_pData is alive until the object is destructed
    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );

    ::cppu::OInterfaceContainerHelper* pContainer =
            m_pData->m_aListenersContainer.getContainer(
                cppu::UnoType<embed::XTransactionListener>::get());
    if ( pContainer )
    {
           ::cppu::OInterfaceIteratorHelper pIterator( *pContainer );
           while ( pIterator.hasMoreElements( ) )
           {
            OSL_ENSURE( nMessage >= 1 && nMessage <= 4, "Wrong internal notification code is used!" );

            switch( nMessage )
            {
                case STOR_MESS_PRECOMMIT:
                       static_cast<embed::XTransactionListener*>( pIterator.next( ) )->preCommit( aSource );
                    break;
                case STOR_MESS_COMMITTED:
                       static_cast<embed::XTransactionListener*>( pIterator.next( ) )->commited( aSource );
                    break;
                case STOR_MESS_PREREVERT:
                       static_cast<embed::XTransactionListener*>( pIterator.next( ) )->preRevert( aSource );
                    break;
                case STOR_MESS_REVERTED:
                       static_cast< embed::XTransactionListener*>( pIterator.next( ) )->reverted( aSource );
                    break;
            }
           }
    }
}
void SAL_CALL OWriteStream::commit()
{
    SAL_INFO( "package.xstor", "package (mv76033) OWriteStream::commit" );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_bTransacted )
        throw uno::RuntimeException();

    try {
        BroadcastTransaction( STOR_MESS_PRECOMMIT );

        ::osl::ResettableMutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

        if ( !m_pImpl )
        {
            SAL_INFO("package.xstor", "Disposed!");
            throw lang::DisposedException();
        }

        m_pImpl->Commit();

        // when the storage is committed the parent is modified
        ModifyParentUnlockMutex_Impl( aGuard );
    }
    catch( const io::IOException& rIOException )
    {
        SAL_INFO("package.xstor", "Rethrow: " << rIOException);
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        SAL_INFO("package.xstor", "Rethrow: " << rStorageWrappedTargetException);
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        SAL_INFO("package.xstor", "Rethrow: " << rRuntimeException);
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));
        throw embed::StorageWrappedTargetException( "Problems on commit!",
                                  static_cast< ::cppu::OWeakObject* >( this ),
                                  aCaught );
    }

    BroadcastTransaction( STOR_MESS_COMMITTED );
}

void SAL_CALL OWriteStream::revert()
{
    SAL_INFO( "package.xstor", "package (mv76033) OWriteStream::revert" );

    // the method removes all the changes done after last commit

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_bTransacted )
        throw uno::RuntimeException();

    BroadcastTransaction( STOR_MESS_PREREVERT );

    ::osl::ResettableMutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    try {
        m_pImpl->Revert();
    }
    catch( const io::IOException& rIOException )
    {
        SAL_INFO("package.xstor", "Rethrow: " << rIOException);
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        SAL_INFO("package.xstor", "Rethrow: " << rStorageWrappedTargetException);
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        SAL_INFO("package.xstor", "Rethrow: " << rRuntimeException);
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));
        throw embed::StorageWrappedTargetException( "Problems on revert!",
                                  static_cast< ::cppu::OWeakObject* >( this ),
                                  aCaught );
    }

    aGuard.clear();

    BroadcastTransaction( STOR_MESS_REVERTED );
}

//  XTransactionBroadcaster

void SAL_CALL OWriteStream::addTransactionListener( const uno::Reference< embed::XTransactionListener >& aListener )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_bTransacted )
        throw uno::RuntimeException();

    m_pData->m_aListenersContainer.addInterface( cppu::UnoType<embed::XTransactionListener>::get(),
                                                aListener );
}

void SAL_CALL OWriteStream::removeTransactionListener( const uno::Reference< embed::XTransactionListener >& aListener )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_bTransacted )
        throw uno::RuntimeException();

    m_pData->m_aListenersContainer.removeInterface( cppu::UnoType<embed::XTransactionListener>::get(),
                                                    aListener );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
