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

#include <sal/config.h>

#include <cassert>
#include <memory>
#include <sal/log.hxx>

#include <com/sun/star/packages/NoEncryptionException.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
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

#include <comphelper/memorystream.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <comphelper/refcountedmutex.hxx>
#include <comphelper/sequence.hxx>

#include <comphelper/diagnose_ex.hxx>

#include <PackageConstants.hxx>
#include <utility>

#include "selfterminatefilestream.hxx"
#include "owriteablestream.hxx"
#include "oseekinstream.hxx"
#include "xstorage.hxx"

// since the copying uses 32000 blocks usually, it makes sense to have a smaller size
#define MAX_STORCACHE_SIZE 30000

using namespace ::com::sun::star;

namespace package
{

static void CopyInputToOutput(
    const css::uno::Reference< css::io::XInputStream >& xInput,
    SvStream& rOutput )
{
    static const sal_Int32 nConstBufferSize = 32000;

    if (auto pByteReader = dynamic_cast< comphelper::ByteReader* >( xInput.get() ))
    {
        sal_Int32 nRead;
        sal_Int8 aTempBuf[ nConstBufferSize ];
        do
        {
            nRead = pByteReader->readSomeBytes ( aTempBuf, nConstBufferSize );
            rOutput.WriteBytes ( aTempBuf, nRead );
        }
        while ( nRead == nConstBufferSize );
    }
    else
    {
        sal_Int32 nRead;
        uno::Sequence < sal_Int8 > aSequence ( nConstBufferSize );

        do
        {
            nRead = xInput->readBytes ( aSequence, nConstBufferSize );
            rOutput.WriteBytes ( aSequence.getConstArray(), nRead );
        }
        while ( nRead == nConstBufferSize );
    }
}

bool PackageEncryptionDataLessOrEqual( const ::comphelper::SequenceAsHashMap& aHash1, const ::comphelper::SequenceAsHashMap& aHash2 )
{
    // tdf#93389: aHash2 may contain more than in aHash1, if it contains also data for other package
    // formats (as in case of autorecovery)
    bool bResult = !aHash1.empty() && aHash1.size() <= aHash2.size();
    for ( ::comphelper::SequenceAsHashMap::const_iterator aIter = aHash1.begin();
          bResult && aIter != aHash1.end();
          ++aIter )
    {
        uno::Sequence< sal_Int8 > aKey1;
        bResult = ( ( aIter->second >>= aKey1 ) && aKey1.hasElements() );
        if ( bResult )
        {
            const uno::Sequence< sal_Int8 > aKey2 = aHash2.getUnpackedValueOrDefault( aIter->first.maString, uno::Sequence< sal_Int8 >() );
            bResult = aKey1.getLength() == aKey2.getLength()
                && std::equal(std::cbegin(aKey1), std::cend(aKey1), aKey2.begin(), aKey2.end());
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
        xPropertySet->setPropertyValue( STORAGE_ENCRYPTION_KEYS_PROPERTY, uno::Any( aKey ) );
    }
    catch ( const uno::Exception& ex )
    {
        TOOLS_WARN_EXCEPTION( "package.xstor", "Can't write encryption related properties");
        throw io::IOException(ex.Message); // TODO
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
    catch ( const uno::Exception& ex )
    {
        TOOLS_WARN_EXCEPTION( "package.xstor", "Can't get encryption related properties");
        throw io::IOException(ex.Message); // TODO
    }
}

bool SequencesEqual( const uno::Sequence< sal_Int8 >& aSequence1, const uno::Sequence< sal_Int8 >& aSequence2 )
{
    return aSequence1.getLength() == aSequence2.getLength()
        && std::equal(aSequence1.begin(), aSequence1.end(), aSequence2.begin(), aSequence2.end());
}

bool SequencesEqual( const uno::Sequence< beans::NamedValue >& aSequence1, const uno::Sequence< beans::NamedValue >& aSequence2 )
{
    if ( aSequence1.getLength() != aSequence2.getLength() )
        return false;

    for ( const auto& rProp1 : aSequence1 )
    {
        bool bHasMember = false;
        uno::Sequence< sal_Int8 > aMember1;
        sal_Int32 nMember1 = 0;
        if ( rProp1.Value >>= aMember1 )
        {
            for ( const auto& rProp2 : aSequence2 )
            {
                if ( rProp1.Name == rProp2.Name )
                {
                    bHasMember = true;

                    uno::Sequence< sal_Int8 > aMember2;
                    if ( !( rProp2.Value >>= aMember2 ) || !SequencesEqual( aMember1, aMember2 ) )
                        return false;
                }
            }
        }
        else if ( rProp1.Value >>= nMember1 )
        {
            for ( const auto& rProp2 : aSequence2 )
            {
                if ( rProp1.Name == rProp2.Name )
                {
                    bHasMember = true;

                    sal_Int32 nMember2 = 0;
                    if ( !( rProp2.Value >>= nMember2 ) || nMember1 != nMember2 )
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

const beans::StringPair* lcl_findPairByName(const uno::Sequence<beans::StringPair>& rSeq, const OUString& rName)
{
    return std::find_if(rSeq.begin(), rSeq.end(),
        [&rName](const beans::StringPair& rPair) { return rPair.First == rName; });
}

} // anonymous namespace

OWriteStream_Impl::OWriteStream_Impl( OStorage_Impl* pParent,
                                      const uno::Reference< packages::XDataSinkEncrSupport >& xPackageStream,
                                      const uno::Reference< lang::XSingleServiceFactory >& xPackage,
                                      uno::Reference< uno::XComponentContext > xContext,
                                      bool bForceEncrypted,
                                      sal_Int32 nStorageType,
                                      bool bDefaultCompress,
                                      uno::Reference< io::XInputStream > xRelInfoStream )
: m_xMutex( new comphelper::RefCountedMutex )
, m_pAntiImpl( nullptr )
, m_bHasDataToFlush( false )
, m_bFlushed( false )
, m_xPackageStream( xPackageStream )
, m_xContext(std::move( xContext ))
, m_pParent( pParent )
, m_bForceEncrypted( bForceEncrypted )
, m_bUseCommonEncryption( !bForceEncrypted && nStorageType == embed::StorageFormats::PACKAGE )
, m_bHasCachedEncryptionData( false )
, m_bCompressedSetExplicit( !bDefaultCompress )
, m_xPackage( xPackage )
, m_bHasInsertedStreamOptimization( false )
, m_nStorageType( nStorageType )
, m_xOrigRelInfoStream(std::move( xRelInfoStream ))
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

    m_oTempFile.reset();

    CleanCacheStream();
}

void OWriteStream_Impl::CleanCacheStream()
{
    if ( !m_xCacheStream.is() )
        return;

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

void OWriteStream_Impl::InsertIntoPackageFolder( const OUString& aName,
                                                  const uno::Reference< container::XNameContainer >& xParentPackageFolder )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    SAL_WARN_IF( !m_bFlushed, "package.xstor", "This method must not be called for nonflushed streams!" );
    if ( m_bFlushed )
    {
        SAL_WARN_IF( !m_xPackageStream.is(), "package.xstor", "An inserted stream is incomplete!" );
        uno::Reference< uno::XInterface > xTmp( m_xPackageStream, uno::UNO_QUERY_THROW );
        xParentPackageFolder->insertByName( aName, uno::Any( xTmp ) );

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

    if ( m_oTempFile.has_value() || m_xCacheStream.is() )
        return false;

    GetStreamProperties();

    // the following value can not be cached since it can change after root commit
    bool bWasEncr = false;
    uno::Reference< beans::XPropertySet > xPropSet( m_xPackageStream, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        uno::Any aValue = xPropSet->getPropertyValue(u"WasEncrypted"_ustr);
        if ( !( aValue >>= bWasEncr ) )
        {
            SAL_WARN( "package.xstor", "The property WasEncrypted has wrong type!" );
        }
    }

    bool bToBeEncr = false;
    for (const auto& rProp : m_aProps)
    {
        if ( rProp.Name == "Encrypted" )
        {
            if ( !( rProp.Value >>= bToBeEncr ) )
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
    if ( !bWasEncr && bToBeEncr && !aKey.hasElements() )
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

    for ( auto& rProp : asNonConstRange(m_aProps) )
    {
        if ( rProp.Name == "Encrypted" )
            rProp.Value <<= false;
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
    for ( auto& rProp : asNonConstRange(m_aProps) )
    {
        if ( rProp.Name == "Encrypted" )
            rProp.Value <<= true;
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
        catch ( const uno::RuntimeException& )
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Quiet exception");
        }

        m_pAntiImpl = nullptr;
    }
    m_pParent = nullptr;

    if ( m_aInputStreamsVector.empty() )
        return;

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

void OWriteStream_Impl::GetFilledTempFileIfNo( const uno::Reference< io::XInputStream >& xStream )
{
    if ( !m_oTempFile.has_value() )
    {
        m_oTempFile.emplace();

        try {
            if ( xStream.is() )
            {
                // the current position of the original stream should be still OK, copy further
                package::CopyInputToOutput( xStream, *m_oTempFile->GetStream(StreamMode::READWRITE) );
            }
        }
        catch( const packages::WrongPasswordException& )
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            m_oTempFile.reset();
            throw;
        }
        catch( const uno::Exception& )
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            m_oTempFile.reset();
            throw;
        }

        if ( m_oTempFile.has_value() )
            CleanCacheStream();
    }
}

void OWriteStream_Impl::FillTempGetFileName()
{
    // should try to create cache first, if the amount of contents is too big, the temp file should be taken
    if ( !m_xCacheStream.is() && !m_oTempFile.has_value() )
    {
        uno::Reference< io::XInputStream > xOrigStream = m_xPackageStream->getDataStream();
        if ( !xOrigStream.is() )
        {
            // in case of new inserted package stream it is possible that input stream still was not set
            rtl::Reference< comphelper::UNOMemoryStream > xCacheStream = new comphelper::UNOMemoryStream();
            m_xCacheSeek = xCacheStream;
            m_xCacheStream = std::move(xCacheStream);
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
                rtl::Reference< comphelper::UNOMemoryStream > xCacheStream = new comphelper::UNOMemoryStream();

                if ( nRead )
                {
                    uno::Reference< io::XOutputStream > xOutStream( xCacheStream->getOutputStream(), uno::UNO_SET_THROW );
                    xOutStream->writeBytes( aData );
                }
                m_xCacheSeek = xCacheStream;
                m_xCacheStream = std::move(xCacheStream);
                m_xCacheSeek->seek( 0 );
            }
            else if ( !m_oTempFile.has_value() )
            {
                m_oTempFile.emplace();

                try {
                    // copy stream contents to the file
                    SvStream* pStream = m_oTempFile->GetStream(StreamMode::READWRITE);
                    pStream->WriteBytes( aData.getConstArray(), aData.getLength() );

                    // the current position of the original stream should be still OK, copy further
                    package::CopyInputToOutput( xOrigStream, *pStream );
                }
                catch( const packages::WrongPasswordException& )
                {
                    m_oTempFile.reset();
                    throw;
                }
                catch( const uno::Exception& )
                {
                    m_oTempFile.reset();
                }
            }
        }
    }
}

uno::Reference< io::XStream > OWriteStream_Impl::GetTempFileAsStream()
{
    uno::Reference< io::XStream > xTempStream;

    if ( !m_xCacheStream.is() )
    {
        if ( !m_oTempFile.has_value() )
            FillTempGetFileName();

        if ( m_oTempFile.has_value() )
        {
            // the temporary file is not used if the cache is used
            try
            {
                SvStream* pStream = m_oTempFile->GetStream(StreamMode::READWRITE);
                pStream->Seek(0);
                xTempStream = new utl::OStreamWrapper(pStream, /*bOwner*/false);
            }
            catch( const uno::Exception& )
            {
                TOOLS_INFO_EXCEPTION("package.xstor", "Quiet exception");
            }
        }
    }

    if ( m_xCacheStream.is() )
        xTempStream = m_xCacheStream;

    // the method must always return a stream
    // in case the stream can not be open
    // an exception should be thrown
    if ( !xTempStream.is() )
        throw io::IOException(u"no temp stream"_ustr); //TODO:

    return xTempStream;
}

uno::Reference< io::XInputStream > OWriteStream_Impl::GetTempFileAsInputStream()
{
    uno::Reference< io::XInputStream > xInputStream;

    if ( !m_xCacheStream.is() )
    {
        if ( !m_oTempFile.has_value() )
            FillTempGetFileName();

        if ( m_oTempFile.has_value() )
        {
            // the temporary file is not used if the cache is used
            try
            {
                SvStream* pStream = m_oTempFile->GetStream(StreamMode::READWRITE);
                pStream->Seek(0);
                xInputStream = new utl::OStreamWrapper(pStream, /*bOwner*/false);
            }
            catch( const uno::Exception& )
            {
                TOOLS_INFO_EXCEPTION("package.xstor", "Quiet exception");
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
        throw io::IOException(u"m_bHasDataToFlush==true"_ustr);

    OSL_ENSURE( !m_oTempFile.has_value() && !m_xCacheStream.is(), "The temporary must not exist!" );

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
    OUString aComprPropName( u"Compressed"_ustr );
    OUString aMedTypePropName( u"MediaType"_ustr );
    for ( const auto& rProp : aProps )
    {
        if ( rProp.Name == aComprPropName )
        {
            bCompressedIsSet = true;
            rProp.Value >>= bCompressed;
        }
        else if ( ( m_nStorageType == embed::StorageFormats::OFOPXML || m_nStorageType == embed::StorageFormats::PACKAGE )
               && rProp.Name == aMedTypePropName )
        {
            xPropertySet->setPropertyValue( rProp.Name, rProp.Value );
        }
        else if ( m_nStorageType == embed::StorageFormats::PACKAGE && rProp.Name == "UseCommonStoragePasswordEncryption" )
            rProp.Value >>= m_bUseCommonEncryption;
        else
            throw lang::IllegalArgumentException();

        // if there are cached properties update them
        if ( rProp.Name == aMedTypePropName || rProp.Name == aComprPropName )
            for ( auto& rMemProp : asNonConstRange(m_aProps) )
            {
                if ( rProp.Name == rMemProp.Name )
                    rMemProp.Value = rProp.Value;
            }
    }

    if ( bCompressedIsSet )
    {
        xPropertySet->setPropertyValue( aComprPropName, uno::Any( bCompressed ) );
        m_bCompressedSetExplicit = true;
    }

    if ( m_bUseCommonEncryption )
    {
        if ( m_nStorageType != embed::StorageFormats::PACKAGE )
            throw uno::RuntimeException();

        // set to be encrypted but do not use encryption key
        xPropertySet->setPropertyValue( STORAGE_ENCRYPTION_KEYS_PROPERTY,
                                        uno::Any( uno::Sequence< beans::NamedValue >() ) );
        xPropertySet->setPropertyValue( u"Encrypted"_ustr, uno::Any( true ) );
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
    uno::Sequence< uno::Any > aSeq{ uno::Any(false) };

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
    else if ( m_oTempFile.has_value() )
    {
        if ( m_pAntiImpl )
            m_pAntiImpl->DeInit();

        rtl::Reference< OSelfTerminateFileStream > xInStream;
        try
        {
            xInStream = new OSelfTerminateFileStream(m_xContext, std::move(*m_oTempFile));
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("package", "");
        }

        if ( !xInStream.is() )
            throw io::IOException();

        xNewPackageStream.set( m_xPackage->createInstanceWithArguments( aSeq ), uno::UNO_QUERY_THROW );

        // TODO/NEW: Let the temporary file be removed after commit
        xNewPackageStream->setDataStream( xInStream );
        m_oTempFile.reset();
    }
    else // if ( m_bHasInsertedStreamOptimization )
    {
        // if the optimization is used the stream can be accessed directly
        xNewPackageStream = m_xPackageStream;
    }

    // copy properties to the package stream
    uno::Reference< beans::XPropertySet > xPropertySet( xNewPackageStream, uno::UNO_QUERY_THROW );

    for ( auto& rProp : asNonConstRange(m_aProps) )
    {
        if ( rProp.Name == "Size" )
        {
            if ( m_pAntiImpl && !m_bHasInsertedStreamOptimization && m_pAntiImpl->m_xSeekable.is() )
            {
                rProp.Value <<= m_pAntiImpl->m_xSeekable->getLength();
                xPropertySet->setPropertyValue( rProp.Name, rProp.Value );
            }
        }
        else
            xPropertySet->setPropertyValue( rProp.Name, rProp.Value );
    }

    if ( m_bUseCommonEncryption )
    {
        if ( m_nStorageType != embed::StorageFormats::PACKAGE )
            throw uno::RuntimeException();

        // set to be encrypted but do not use encryption key
        xPropertySet->setPropertyValue( STORAGE_ENCRYPTION_KEYS_PROPERTY,
                                        uno::Any( uno::Sequence< beans::NamedValue >() ) );
        xPropertySet->setPropertyValue( u"Encrypted"_ustr,
                                        uno::Any( true ) );
    }
    else if ( m_bHasCachedEncryptionData )
    {
        if ( m_nStorageType != embed::StorageFormats::PACKAGE )
            throw uno::RuntimeException();

        xPropertySet->setPropertyValue( STORAGE_ENCRYPTION_KEYS_PROPERTY,
                                        uno::Any( m_aEncryptionData.getAsConstNamedValueList() ) );
    }

    // the stream should be free soon, after package is stored
    m_xPackageStream = std::move(xNewPackageStream);
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

    OSL_ENSURE( m_oTempFile.has_value() || m_xCacheStream.is(), "The temporary must exist!" );

    if ( m_xCacheStream.is() )
    {
        m_xCacheStream.clear();
        m_xCacheSeek.clear();
    }

    m_oTempFile.reset();

    m_aProps.realloc( 0 );

    m_bHasDataToFlush = false;

    m_bUseCommonEncryption = true;
    m_bHasCachedEncryptionData = false;
    m_aEncryptionData.clear();

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        return;

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

uno::Sequence< beans::PropertyValue > const & OWriteStream_Impl::GetStreamProperties()
{
    if ( !m_aProps.hasElements() )
        m_aProps = ReadPackageStreamProperties();

    return m_aProps;
}

uno::Sequence< beans::PropertyValue > OWriteStream_Impl::InsertOwnProps(
                                                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                                                    bool bUseCommonEncryption )
{
    uno::Sequence< beans::PropertyValue > aResult( aProps );
    beans::PropertyValue aPropVal;

    if ( m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        aPropVal.Name = "UseCommonStoragePasswordEncryption";
        aPropVal.Value <<= bUseCommonEncryption;
    }
    else if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        ReadRelInfoIfNecessary();

        aPropVal.Name = "RelationsInfo";
        if ( m_nRelInfoStatus == RELINFO_READ )
            aPropVal.Value <<= m_aOrigRelInfo;
        else if ( m_nRelInfoStatus == RELINFO_CHANGED_STREAM_READ || m_nRelInfoStatus == RELINFO_CHANGED )
            aPropVal.Value <<= m_aNewRelInfo;
        else // m_nRelInfoStatus == RELINFO_CHANGED_BROKEN || m_nRelInfoStatus == RELINFO_BROKEN
            throw io::IOException( u"Wrong relinfo stream!"_ustr );
    }
    if (!aPropVal.Name.isEmpty())
    {
        sal_Int32 i = 0;
        for (auto p = aResult.getConstArray(); i < aResult.getLength(); ++i)
            if (p[i].Name == aPropVal.Name)
                break;
        if (i == aResult.getLength())
            aResult.realloc(i + 1);
        aResult.getArray()[i] = std::move(aPropVal);
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
                                        u"_rels/*.rels",
                                        m_xContext );

            // in case of success the stream must be thrown away, that means that the OrigRelInfo is initialized
            // the reason for this is that the original stream might not be seekable ( at the same time the new
            // provided stream must be seekable ), so it must be read only once
            m_xOrigRelInfoStream.clear();
            m_nRelInfoStatus = RELINFO_READ;
        }
        catch( const uno::Exception& )
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Quiet exception");

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
                                        u"_rels/*.rels",
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
    assert(nPropNum >= 2);
    uno::Sequence< beans::PropertyValue > aResult( nPropNum );
    auto aResultRange = asNonConstRange(aResult);

    // The "Compressed" property must be set after "MediaType" property,
    // since the setting of the last one can change the value of the first one
    static constexpr OUStringLiteral sMediaType = u"MediaType";
    static constexpr OUString sCompressed = u"Compressed"_ustr;
    static constexpr OUString sSize = u"Size"_ustr;
    static constexpr OUStringLiteral sEncrypted = u"Encrypted";
    if ( m_nStorageType == embed::StorageFormats::OFOPXML || m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        aResultRange[0].Name = sMediaType;
        aResultRange[1].Name = sCompressed;
        aResultRange[2].Name = sSize;

        if ( m_nStorageType == embed::StorageFormats::PACKAGE )
            aResultRange[3].Name = sEncrypted;
    }
    else
    {
        aResultRange[0].Name = sCompressed;
        aResultRange[1].Name = sSize;
    }

    // TODO: may be also raw stream should be marked

    uno::Reference< beans::XPropertySet > xPropSet( m_xPackageStream, uno::UNO_QUERY_THROW );
    for ( auto& rProp : aResultRange )
    {
        try {
            rProp.Value = xPropSet->getPropertyValue( rProp.Name );
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION( "package.xstor", "A property can't be retrieved" );
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
            throw io::IOException( u"Wrong relinfo stream!"_ustr );
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
        if ( !::package::PackageEncryptionDataLessOrEqual( m_aEncryptionData, aEncryptionData ) )
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
        catch( const packages::WrongPasswordException& )
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
            throw;
        }
        catch ( const uno::Exception& ex )
        {
            TOOLS_WARN_EXCEPTION("package.xstor", "GetStream: decrypting stream failed");
            SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
            throw io::IOException(ex.Message); // TODO:
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
        catch( const packages::NoEncryptionException& )
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
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
        if ( m_xCacheStream.is() || m_oTempFile.has_value() )
            xInStream = GetTempFileAsInputStream(); //TODO:
        else
            xInStream = m_xPackageStream->getDataStream();

        // The stream does not exist in the storage
        if ( !xInStream.is() )
            throw io::IOException();

        rtl::Reference<OInputCompStream> pStream = new OInputCompStream( *this, xInStream, InsertOwnProps( m_aProps, m_bUseCommonEncryption ), m_nStorageType );
        m_aInputStreamsVector.push_back( pStream.get() );
        return pStream;
    }
    else if ( ( nStreamMode & embed::ElementModes::READWRITE ) == embed::ElementModes::SEEKABLEREAD )
    {
        if ( !m_xCacheStream.is() && !m_oTempFile.has_value() && !( m_xPackageStream->getDataStream().is() ) )
        {
            // The stream does not exist in the storage
            throw io::IOException();
        }

        uno::Reference< io::XInputStream > xInStream = GetTempFileAsInputStream(); //TODO:

        if ( !xInStream.is() )
            throw io::IOException();

        rtl::Reference<OInputSeekStream> pStream = new OInputSeekStream( *this, xInStream, InsertOwnProps( m_aProps, m_bUseCommonEncryption ), m_nStorageType );
        m_aInputStreamsVector.push_back( pStream.get() );
        return pStream;
    }
    else if ( ( nStreamMode & embed::ElementModes::WRITE ) == embed::ElementModes::WRITE )
    {
        if ( !m_aInputStreamsVector.empty() )
            throw io::IOException(); // TODO:

        uno::Reference< io::XStream > xStream;
        if ( ( nStreamMode & embed::ElementModes::TRUNCATE ) == embed::ElementModes::TRUNCATE )
        {
            m_oTempFile.reset();
            if ( m_xCacheStream.is() )
                CleanCacheStream();

            m_bHasDataToFlush = true;

            // this call is triggered by the parent and it will recognize the change of the state
            if ( m_pParent )
                m_pParent->m_bIsModified = true;

            rtl::Reference<comphelper::UNOMemoryStream> xMemStream = new comphelper::UNOMemoryStream();
            xStream = xMemStream;
            m_xCacheSeek = xMemStream;
            m_xCacheStream = xStream;
        }
        else if ( !m_bHasInsertedStreamOptimization )
        {
            if ( !m_oTempFile.has_value() && !m_xCacheStream.is() && !( m_xPackageStream->getDataStream().is() ) )
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

        rtl::Reference<OWriteStream> tmp;
        assert(m_xMutex.is() && "No mutex!");
        if ( !xStream.is() )
            tmp = new OWriteStream( *this, bHierarchyAccess );
        else
            tmp = new OWriteStream( *this, xStream, bHierarchyAccess );

        m_pAntiImpl = tmp.get();
        return tmp;
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
    std::erase(m_aInputStreamsVector, pStream);
}

void OWriteStream_Impl::CreateReadonlyCopyBasedOnData( const uno::Reference< io::XInputStream >& xDataToCopy, const uno::Sequence< beans::PropertyValue >& aProps, uno::Reference< io::XStream >& xTargetStream )
{
    uno::Reference < io::XStream > xTempFile;
    if ( !xTargetStream.is() )
        xTempFile = new utl::TempFileFastService;
    else
        xTempFile = xTargetStream;

    uno::Reference < io::XSeekable > xTempSeek( xTempFile, uno::UNO_QUERY_THROW );

    uno::Reference < io::XOutputStream > xTempOut(xTempFile->getOutputStream(), uno::UNO_SET_THROW);

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
            new OInputSeekStream( xInStream, InsertOwnProps( aProps, m_bUseCommonEncryption ), m_nStorageType ) );
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
        catch( const packages::NoEncryptionException& )
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "No Element");
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
        xProps->getPropertyValue( u"Encrypted"_ustr ) >>= bEncr;
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
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION( "package.xstor", "Can't open encrypted stream");
            SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
            throw;
        }

        SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< beans::NamedValue >() );
    }

    // in case of new inserted package stream it is possible that input stream still was not set
    GetStreamProperties();

    CreateReadonlyCopyBasedOnData( xDataToCopy, m_aProps, xTargetStream );
}

void OWriteStream_Impl::CommitStreamRelInfo( const uno::Reference< embed::XStorage >& xRelStorage, std::u16string_view aOrigStreamName, std::u16string_view aNewStreamName )
{
    // at this point of time the old stream must be already cleaned
    OSL_ENSURE( m_nStorageType == embed::StorageFormats::OFOPXML, "The method should be used only with OFOPXML format!" );

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        return;

    OSL_ENSURE( !aOrigStreamName.empty() && !aNewStreamName.empty() && xRelStorage.is(),
                "Wrong relation persistence information is provided!" );

    if ( !xRelStorage.is() || aOrigStreamName.empty() || aNewStreamName.empty() )
        throw uno::RuntimeException();

    if ( m_nRelInfoStatus == RELINFO_BROKEN || m_nRelInfoStatus == RELINFO_CHANGED_BROKEN )
        throw io::IOException(); // TODO:

    OUString aOrigRelStreamName = OUString::Concat(aOrigStreamName) + ".rels";
    OUString aNewRelStreamName = OUString::Concat(aNewStreamName) + ".rels";

    bool bRenamed = aOrigRelStreamName != aNewRelStreamName;
    if ( m_nRelInfoStatus == RELINFO_CHANGED
      || m_nRelInfoStatus == RELINFO_CHANGED_STREAM_READ
      || m_nRelInfoStatus == RELINFO_CHANGED_STREAM )
    {
        if ( bRenamed && xRelStorage->hasByName( aOrigRelStreamName ) )
            xRelStorage->removeElement( aOrigRelStreamName );

        if ( m_nRelInfoStatus == RELINFO_CHANGED )
        {
            if ( m_aNewRelInfo.hasElements() )
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
                    u"MediaType"_ustr,
                    uno::Any( u"application/vnd.openxmlformats-package.relationships+xml"_ustr ) );

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
            xPropSet->setPropertyValue(u"MediaType"_ustr,
                uno::Any( u"application/vnd.openxmlformats-package.relationships+xml"_ustr ) );

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

// OWriteStream implementation

OWriteStream::OWriteStream( OWriteStream_Impl& rImpl, bool bTransacted )
: m_pImpl( &rImpl )
, m_xSharedMutex( rImpl.m_xMutex )
, m_aListenersContainer( rImpl.m_xMutex->GetMutex() )
, m_nStorageType( m_pImpl->m_nStorageType )
, m_bInStreamDisconnected( false )
, m_bInitOnDemand( true )
, m_nInitPosition( 0 )
, m_bTransacted( bTransacted )
{
}

OWriteStream::OWriteStream( OWriteStream_Impl& rImpl, uno::Reference< io::XStream > const & xStream, bool bTransacted )
: m_pImpl( &rImpl )
, m_xSharedMutex( rImpl.m_xMutex )
, m_aListenersContainer( rImpl.m_xMutex->GetMutex() )
, m_nStorageType( m_pImpl->m_nStorageType )
, m_bInStreamDisconnected( false )
, m_bInitOnDemand( false )
, m_nInitPosition( 0 )
, m_bTransacted( bTransacted )
{
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
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );
    if ( m_pImpl )
    {
        osl_atomic_increment(&m_refCount);
        try {
            dispose();
        }
        catch( const uno::RuntimeException& )
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Quiet exception");
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

    if ( !m_bInitOnDemand )
        return;

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

void OWriteStream::CopyToStreamInternally_Impl( const uno::Reference< io::XStream >& xDest )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

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
    catch ( const uno::Exception& )
    {
        // TODO: set the stream in invalid state or dispose
        TOOLS_WARN_EXCEPTION( "package.xstor", "The stream become invalid during copying" );
        throw uno::RuntimeException();
    }

    if ( bThrown )
        throw eThrown;

    // now the properties can be copied
    // the order of the properties setting is not important for StorageStream API
    OUString aPropName (u"Compressed"_ustr);
    xDestProps->setPropertyValue( aPropName, getPropertyValue( aPropName ) );
    if ( m_nStorageType == embed::StorageFormats::PACKAGE || m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        aPropName = "MediaType";
        xDestProps->setPropertyValue( aPropName, getPropertyValue( aPropName ) );

        if ( m_nStorageType == embed::StorageFormats::PACKAGE )
        {
            aPropName = "UseCommonStoragePasswordEncryption";
            xDestProps->setPropertyValue( aPropName, getPropertyValue( aPropName ) );
        }
    }
}

void OWriteStream::ModifyParentUnlockMutex_Impl(osl::ClearableMutexGuard& aGuard)
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
    // common interfaces
    uno::Any aReturn = ::cppu::queryInterface
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

    if ( m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        aReturn = ::cppu::queryInterface
                    (   rType
                        ,   static_cast<embed::XEncryptionProtectedSource2*> ( this )
                        ,   static_cast<embed::XEncryptionProtectedSource*> ( this ) );
    }
    else if ( m_nStorageType == embed::StorageFormats::OFOPXML )
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

void SAL_CALL OWriteStream::acquire() noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL OWriteStream::release() noexcept
{
    OWeakObject::release();
}

uno::Sequence< uno::Type > SAL_CALL OWriteStream::getTypes()
{
    if (! m_oTypeCollection)
    {
        ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

        if (! m_oTypeCollection)
        {
            if ( m_bTransacted )
            {
                if ( m_nStorageType == embed::StorageFormats::PACKAGE )
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

                    m_oTypeCollection.emplace(
                                        cppu::UnoType<beans::XPropertySet>::get()
                                    ,   aTmpCollection.getTypes());
                }
                else if ( m_nStorageType == embed::StorageFormats::OFOPXML )
                {
                    m_oTypeCollection.emplace(
                                        cppu::UnoType<lang::XTypeProvider>::get()
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
                                    ,   cppu::UnoType<beans::XPropertySet>::get());
                }
                else // if ( m_pData->m_nStorageType == embed::StorageFormats::ZIP )
                {
                    m_oTypeCollection.emplace(
                                        cppu::UnoType<lang::XTypeProvider>::get()
                                    ,   cppu::UnoType<io::XInputStream>::get()
                                    ,   cppu::UnoType<io::XOutputStream>::get()
                                    ,   cppu::UnoType<io::XStream>::get()
                                    ,   cppu::UnoType<io::XSeekable>::get()
                                    ,   cppu::UnoType<io::XTruncate>::get()
                                    ,   cppu::UnoType<lang::XComponent>::get()
                                    ,   cppu::UnoType<embed::XExtendedStorageStream>::get()
                                    ,   cppu::UnoType<embed::XTransactedObject>::get()
                                    ,   cppu::UnoType<embed::XTransactionBroadcaster>::get()
                                    ,   cppu::UnoType<beans::XPropertySet>::get());
                }
            }
            else
            {
                if ( m_nStorageType == embed::StorageFormats::PACKAGE )
                {
                    m_oTypeCollection.emplace(
                                        cppu::UnoType<lang::XTypeProvider>::get()
                                    ,   cppu::UnoType<io::XInputStream>::get()
                                    ,   cppu::UnoType<io::XOutputStream>::get()
                                    ,   cppu::UnoType<io::XStream>::get()
                                    ,   cppu::UnoType<io::XSeekable>::get()
                                    ,   cppu::UnoType<io::XTruncate>::get()
                                    ,   cppu::UnoType<lang::XComponent>::get()
                                    ,   cppu::UnoType<embed::XEncryptionProtectedSource2>::get()
                                    ,   cppu::UnoType<embed::XEncryptionProtectedSource>::get()
                                    ,   cppu::UnoType<beans::XPropertySet>::get());
                }
                else if ( m_nStorageType == embed::StorageFormats::OFOPXML )
                {
                    m_oTypeCollection.emplace(
                                        cppu::UnoType<lang::XTypeProvider>::get()
                                    ,   cppu::UnoType<io::XInputStream>::get()
                                    ,   cppu::UnoType<io::XOutputStream>::get()
                                    ,   cppu::UnoType<io::XStream>::get()
                                    ,   cppu::UnoType<io::XSeekable>::get()
                                    ,   cppu::UnoType<io::XTruncate>::get()
                                    ,   cppu::UnoType<lang::XComponent>::get()
                                    ,   cppu::UnoType<embed::XRelationshipAccess>::get()
                                    ,   cppu::UnoType<beans::XPropertySet>::get());
                }
                else // if ( m_pData->m_nStorageType == embed::StorageFormats::ZIP )
                {
                    m_oTypeCollection.emplace(
                                        cppu::UnoType<lang::XTypeProvider>::get()
                                    ,   cppu::UnoType<io::XInputStream>::get()
                                    ,   cppu::UnoType<io::XOutputStream>::get()
                                    ,   cppu::UnoType<io::XStream>::get()
                                    ,   cppu::UnoType<io::XSeekable>::get()
                                    ,   cppu::UnoType<io::XTruncate>::get()
                                    ,   cppu::UnoType<lang::XComponent>::get()
                                    ,   cppu::UnoType<beans::XPropertySet>::get());
                }
            }
        }
    }

    return m_oTypeCollection->getTypes() ;
}

uno::Sequence< sal_Int8 > SAL_CALL OWriteStream::getImplementationId()
{
    static const comphelper::UnoIdInit lcl_ImplId;
    return lcl_ImplId.getSeq();
}

sal_Int32 SAL_CALL OWriteStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    return m_xInStream->readBytes( aData, nBytesToRead );
}

sal_Int32 SAL_CALL OWriteStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    return m_xInStream->readSomeBytes( aData, nMaxBytesToRead );
}

sal_Int32 OWriteStream::readSomeBytes(sal_Int8* pData, sal_Int32 nBytesToRead)
{
    osl::MutexGuard aGuard(m_xSharedMutex->GetMutex());

    CheckInitOnDemand();

    if (!m_xInStream.is())
        throw io::NotConnectedException();

    if (auto pByteReader = dynamic_cast<comphelper::ByteReader*>(m_xInStream.get()))
        return pByteReader->readSomeBytes(pData, nBytesToRead);

    uno::Sequence<sal_Int8> aData;
    sal_Int32 nRead = m_xInStream->readSomeBytes(aData, nBytesToRead);
    std::copy_n(aData.getConstArray(), nRead, pData);

    return nRead;
}

void SAL_CALL OWriteStream::skipBytes( sal_Int32 nBytesToSkip )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    m_xInStream->skipBytes( nBytesToSkip );
}

sal_Int32 SAL_CALL OWriteStream::available(  )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    return m_xInStream->available();

}

void SAL_CALL OWriteStream::closeInput(  )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_bInitOnDemand && ( m_bInStreamDisconnected || !m_xInStream.is() ) )
        throw io::NotConnectedException();

    // the input part of the stream stays open for internal purposes (to allow reading during copying)
    // since it can not be reopened until output part is closed, it will be closed with output part.
    m_bInStreamDisconnected = true;
    // m_xInStream->closeInput();
    // m_xInStream.clear();

    if ( !m_xOutStream.is() )
        dispose();
}

uno::Reference< io::XInputStream > SAL_CALL OWriteStream::getInputStream()
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_bInitOnDemand && ( m_bInStreamDisconnected || !m_xInStream.is() ) )
        return uno::Reference< io::XInputStream >();

    return this;
}

uno::Reference< io::XOutputStream > SAL_CALL OWriteStream::getOutputStream()
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    try
    {
        CheckInitOnDemand();
    }
    catch( const io::IOException& r )
    {
        throw lang::WrappedTargetRuntimeException(u"OWriteStream::getOutputStream: Could not create backing temp file"_ustr,
                getXWeak(), css::uno::Any ( r ) );
    }

    if ( !m_xOutStream.is() )
        return uno::Reference< io::XOutputStream >();

    return this;
}

void OWriteStream::CheckInitOnWriteDemand(sal_Int32 dataSize)
{
    // write methods need a different initialization, since they depend on data length

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
            if (nPos + dataSize > MAX_STORCACHE_SIZE)
            {
                // disconnect the cache and copy the data to the temporary file
                m_xSeekable->seek( 0 );

                // it is enough to copy the cached stream, the cache should already contain everything
                m_pImpl->GetFilledTempFileIfNo( m_xInStream );
                if ( m_pImpl->m_oTempFile.has_value() )
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
        SAL_INFO("package.xstor", "OWriteStream::CheckInitOnWriteDemand: initializing");
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

void SAL_CALL OWriteStream::writeBytes( const uno::Sequence< sal_Int8 >& aData )
{
    osl::ClearableMutexGuard aGuard(m_xSharedMutex->GetMutex());

    CheckInitOnWriteDemand(aData.getLength());

    if ( !m_xOutStream.is() )
        throw io::NotConnectedException();

    m_xOutStream->writeBytes( aData );
    m_pImpl->m_bHasDataToFlush = true;

    ModifyParentUnlockMutex_Impl( aGuard );
}

void OWriteStream::writeBytes( const sal_Int8* pData, sal_Int32 nBytesToWrite )
{
    assert(nBytesToWrite >= 0);

    osl::ClearableMutexGuard aGuard(m_xSharedMutex->GetMutex());

    CheckInitOnWriteDemand(nBytesToWrite);

    if ( !m_xOutStream.is() )
        throw io::NotConnectedException();

    if (auto pByteWriter = dynamic_cast< comphelper::ByteWriter* >( m_xOutStream.get() ))
        pByteWriter->writeBytes(pData, nBytesToWrite);
    else
    {
        uno::Sequence<sal_Int8> aData(pData, nBytesToWrite);
        m_xOutStream->writeBytes( aData );
    }
    m_pImpl->m_bHasDataToFlush = true;

    ModifyParentUnlockMutex_Impl( aGuard );
}

void SAL_CALL OWriteStream::flush()
{
    // In case stream is flushed its current version becomes visible
    // to the parent storage. Usually parent storage flushes the stream
    // during own commit but a user can explicitly flush the stream
    // so the changes will be available through cloning functionality.

    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

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

    if ( m_bInitOnDemand )
        return;

    // after the stream is disposed it can be committed
    // so transport correct size property
    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    for ( auto& rProp : asNonConstRange(m_pImpl->m_aProps) )
    {
        if ( rProp.Name == "Size" )
            rProp.Value <<= m_xSeekable->getLength();
    }
}

void SAL_CALL OWriteStream::closeOutput()
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_xOutStream.is() )
        throw io::NotConnectedException();

    CloseOutput_Impl();

    if ( m_bInStreamDisconnected || !m_xInStream.is() )
        dispose();
}

void SAL_CALL OWriteStream::seek( sal_Int64 location )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    m_xSeekable->seek( location );
}

sal_Int64 SAL_CALL OWriteStream::getPosition()
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getPosition();
}

sal_Int64 SAL_CALL OWriteStream::getLength()
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    CheckInitOnDemand();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getLength();
}

void SAL_CALL OWriteStream::truncate()
{
    osl::ClearableMutexGuard aGuard(m_xSharedMutex->GetMutex());

    CheckInitOnDemand();

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
        ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

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
                throw lang::WrappedTargetRuntimeException(u"Can not commit/revert the storage!"_ustr,
                                                getXWeak(),
                                                aCaught );
            }
        }

        m_pImpl = nullptr;
    }

    // the listener might try to get rid of parent storage, and the storage would delete this object;
    // for now the listener is just notified at the end of the method to workaround the problem
    // in future a more elegant way should be found

    lang::EventObject aSource( getXWeak() );
    m_aListenersContainer.disposeAndClear( aSource );
}

void SAL_CALL OWriteStream::addEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    m_aListenersContainer.addInterface( cppu::UnoType<lang::XEventListener>::get(),
                                                 xListener );
}

void SAL_CALL OWriteStream::removeEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    m_aListenersContainer.removeInterface( cppu::UnoType<lang::XEventListener>::get(),
                                                    xListener );
}

void SAL_CALL OWriteStream::setEncryptionPassword( const OUString& aPass )
{
    osl::ClearableMutexGuard aGuard(m_xSharedMutex->GetMutex());

    CheckInitOnDemand();

    OSL_ENSURE( m_pImpl->m_xPackageStream.is(), "No package stream is set!" );

    m_pImpl->SetEncrypted( ::comphelper::OStorageHelper::CreatePackageEncryptionData( aPass ) );

    ModifyParentUnlockMutex_Impl( aGuard );
}

void SAL_CALL OWriteStream::removeEncryption()
{
    osl::ClearableMutexGuard aGuard(m_xSharedMutex->GetMutex());

    CheckInitOnDemand();

    OSL_ENSURE( m_pImpl->m_xPackageStream.is(), "No package stream is set!" );

    m_pImpl->SetDecrypted();

    ModifyParentUnlockMutex_Impl( aGuard );
}

void SAL_CALL OWriteStream::setEncryptionData( const uno::Sequence< beans::NamedValue >& aEncryptionData )
{
    osl::ClearableMutexGuard aGuard(m_xSharedMutex->GetMutex());

    CheckInitOnDemand();

    OSL_ENSURE( m_pImpl->m_xPackageStream.is(), "No package stream is set!" );

    m_pImpl->SetEncrypted( aEncryptionData );

    ModifyParentUnlockMutex_Impl( aGuard );
}

sal_Bool SAL_CALL OWriteStream::hasEncryptionData()
{
    osl::ClearableMutexGuard aGuard(m_xSharedMutex->GetMutex());

    if (!m_pImpl)
        return false;

    bool bRet = false;

    try
    {
        bRet = m_pImpl->IsEncrypted();

        if (!bRet && m_pImpl->m_bUseCommonEncryption && m_pImpl->m_pParent)
            bRet = m_pImpl->m_pParent->m_bHasCommonEncryptionData;
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));
        throw lang::WrappedTargetRuntimeException( u"Problems on hasEncryptionData!"_ustr,
                                  getXWeak(),
                                  aCaught );
    }

    return bRet;
}

sal_Bool SAL_CALL OWriteStream::hasByID(  const OUString& sID )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    try
    {
        getRelationshipByID( sID );
        return true;
    }
    catch( const container::NoSuchElementException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "No Element");
    }

    return false;
}

OUString SAL_CALL OWriteStream::getTargetByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    const uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    auto pRel = lcl_findPairByName(aSeq, u"Target"_ustr);
    if (pRel != aSeq.end())
        return pRel->Second;

    return OUString();
}

OUString SAL_CALL OWriteStream::getTypeByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    const uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    auto pRel = lcl_findPairByName(aSeq, u"Type"_ustr);
    if (pRel != aSeq.end())
        return pRel->Second;

    return OUString();
}

uno::Sequence< beans::StringPair > SAL_CALL OWriteStream::getRelationshipByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    // TODO/LATER: in future the unification of the ID could be checked
    const uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    const beans::StringPair aIDRel(u"Id"_ustr, sID);
    auto pRel = std::find_if(aSeq.begin(), aSeq.end(),
        [&aIDRel](const uno::Sequence<beans::StringPair>& rRel) {
            return std::find(rRel.begin(), rRel.end(), aIDRel) != rRel.end(); });
    if (pRel != aSeq.end())
        return *pRel;

    throw container::NoSuchElementException();
}

uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OWriteStream::getRelationshipsByType(  const OUString& sType  )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    // TODO/LATER: in future the unification of the ID could be checked
    const uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    const beans::StringPair aTypeRel(u"Type"_ustr, sType);
    std::vector< uno::Sequence<beans::StringPair> > aResult;
    aResult.reserve(aSeq.getLength());

    std::copy_if(aSeq.begin(), aSeq.end(), std::back_inserter(aResult),
        [&aTypeRel](const uno::Sequence<beans::StringPair>& rRel) {
            return std::find(rRel.begin(), rRel.end(), aTypeRel) != rRel.end(); });

    return comphelper::containerToSequence(aResult);
}

uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OWriteStream::getAllRelationships()
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    return m_pImpl->GetAllRelationshipsIfAny();
}

void SAL_CALL OWriteStream::insertRelationshipByID(  const OUString& sID, const uno::Sequence< beans::StringPair >& aEntry, sal_Bool bReplace  )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    const beans::StringPair aIDRel(u"Id"_ustr, sID);

    uno::Sequence<beans::StringPair>* pPair = nullptr;

    // TODO/LATER: in future the unification of the ID could be checked
    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
    {
        const auto& rRel = aSeq[nInd];
        if (std::find(rRel.begin(), rRel.end(), aIDRel) != rRel.end())
            pPair = &aSeq.getArray()[nInd];
    }

    if ( pPair && !bReplace )
        throw container::ElementExistException(); // TODO

    if ( !pPair )
    {
        sal_Int32 nIDInd = aSeq.getLength();
        aSeq.realloc( nIDInd + 1 );
        pPair = &aSeq.getArray()[nIDInd];
    }

    std::vector<beans::StringPair> aResult;
    aResult.reserve(aEntry.getLength() + 1);

    aResult.push_back(aIDRel);
    std::copy_if(aEntry.begin(), aEntry.end(), std::back_inserter(aResult),
        [](const beans::StringPair& rRel) { return rRel.First != "Id"; });

    *pPair = comphelper::containerToSequence(aResult);

    m_pImpl->m_aNewRelInfo = std::move(aSeq);
    m_pImpl->m_xNewRelInfoStream.clear();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

void SAL_CALL OWriteStream::removeRelationshipByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    const beans::StringPair aIDRel(u"Id"_ustr, sID);
    auto pRel = std::find_if(std::cbegin(aSeq), std::cend(aSeq),
        [&aIDRel](const uno::Sequence< beans::StringPair >& rRel) {
            return std::find(rRel.begin(), rRel.end(), aIDRel) != rRel.end(); });
    if (pRel != std::cend(aSeq))
    {
        auto nInd = static_cast<sal_Int32>(std::distance(std::cbegin(aSeq), pRel));
        comphelper::removeElementAt(aSeq, nInd);

        m_pImpl->m_aNewRelInfo = std::move(aSeq);
        m_pImpl->m_xNewRelInfoStream.clear();
        m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;

        // TODO/LATER: in future the unification of the ID could be checked
        return;
    }

    throw container::NoSuchElementException();
}

void SAL_CALL OWriteStream::insertRelationships(  const uno::Sequence< uno::Sequence< beans::StringPair > >& aEntries, sal_Bool bReplace  )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    OUString aIDTag( u"Id"_ustr );
    const uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    std::vector< uno::Sequence<beans::StringPair> > aResultVec;
    aResultVec.reserve(aSeq.getLength() + aEntries.getLength());

    std::copy_if(aSeq.begin(), aSeq.end(), std::back_inserter(aResultVec),
        [&aIDTag, &aEntries, bReplace](const uno::Sequence<beans::StringPair>& rTargetRel) {
            auto pTargetPair = lcl_findPairByName(rTargetRel, aIDTag);
            if (pTargetPair == rTargetRel.end())
                return false;

            bool bIsSourceSame = std::any_of(aEntries.begin(), aEntries.end(),
                [&pTargetPair](const uno::Sequence<beans::StringPair>& rSourceEntry) {
                    return std::find(rSourceEntry.begin(), rSourceEntry.end(), *pTargetPair) != rSourceEntry.end(); });

            if ( bIsSourceSame && !bReplace )
                throw container::ElementExistException();

            // if no such element in the provided sequence
            return !bIsSourceSame;
        });

    std::transform(aEntries.begin(), aEntries.end(), std::back_inserter(aResultVec),
        [&aIDTag](const uno::Sequence<beans::StringPair>& rEntry) -> uno::Sequence<beans::StringPair> {
            auto pPair = lcl_findPairByName(rEntry, aIDTag);
            if (pPair == rEntry.end())
                throw io::IOException(); // TODO: illegal relation ( no ID )

            auto aResult = comphelper::sequenceToContainer<std::vector<beans::StringPair>>(rEntry);
            auto nIDInd = std::distance(rEntry.begin(), pPair);
            std::rotate(aResult.begin(), std::next(aResult.begin(), nIDInd), std::next(aResult.begin(), nIDInd + 1));

            return comphelper::containerToSequence(aResult);
        });

    m_pImpl->m_aNewRelInfo = comphelper::containerToSequence(aResultVec);
    m_pImpl->m_xNewRelInfoStream.clear();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

void SAL_CALL OWriteStream::clearRelationships()
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    m_pImpl->m_aNewRelInfo.realloc( 0 );
    m_pImpl->m_xNewRelInfoStream.clear();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL OWriteStream::getPropertySetInfo()
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    //TODO:
    return uno::Reference< beans::XPropertySetInfo >();
}

void SAL_CALL OWriteStream::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    osl::ClearableMutexGuard aGuard(m_xSharedMutex->GetMutex());

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    m_pImpl->GetStreamProperties();
    static constexpr OUString aCompressedString( u"Compressed"_ustr );
    static constexpr OUString aMediaTypeString( u"MediaType"_ustr );
    if ( m_nStorageType == embed::StorageFormats::PACKAGE && aPropertyName == aMediaTypeString )
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

        for ( auto& rProp : asNonConstRange(m_pImpl->m_aProps) )
        {
            if ( aPropertyName == rProp.Name )
                rProp.Value = aValue;
            else if ( !m_pImpl->m_bCompressedSetExplicit && aCompressedString == rProp.Name )
                rProp.Value <<= bCompressedValueFromType;
        }
    }
    else if ( aPropertyName == aCompressedString )
    {
        // if the "Compressed" property is not set explicitly, the MediaType can change the default value
        m_pImpl->m_bCompressedSetExplicit = true;
        for ( auto& rProp : asNonConstRange(m_pImpl->m_aProps) )
        {
            if ( aPropertyName == rProp.Name )
                rProp.Value = aValue;
        }
    }
    else if ( m_nStorageType == embed::StorageFormats::PACKAGE
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
    else if ( m_nStorageType == embed::StorageFormats::OFOPXML && aPropertyName == aMediaTypeString )
    {
        for ( auto& rProp : asNonConstRange(m_pImpl->m_aProps) )
        {
            if ( aPropertyName == rProp.Name )
                rProp.Value = aValue;
        }
    }
    else if ( m_nStorageType == embed::StorageFormats::OFOPXML && aPropertyName == "RelationsInfoStream" )
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

        m_pImpl->m_xNewRelInfoStream = std::move(xInRelStream);
        m_pImpl->m_aNewRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
        m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED_STREAM;
    }
    else if ( m_nStorageType == embed::StorageFormats::OFOPXML && aPropertyName == "RelationsInfo" )
    {
        if ( !(aValue >>= m_pImpl->m_aNewRelInfo) )
            throw lang::IllegalArgumentException(); // TODO
    }
    else if ( aPropertyName == "Size" )
        throw beans::PropertyVetoException(); // TODO
    else if ( m_nStorageType == embed::StorageFormats::PACKAGE
           && ( aPropertyName == "IsEncrypted" || aPropertyName == "Encrypted" ) )
        throw beans::PropertyVetoException(); // TODO
    else if ( aPropertyName == "RelId" )
    {
        aValue >>= m_pImpl->m_nRelId;
    }
    else
        throw beans::UnknownPropertyException(aPropertyName); // TODO

    m_pImpl->m_bHasDataToFlush = true;
    ModifyParentUnlockMutex_Impl( aGuard );
}

uno::Any SAL_CALL OWriteStream::getPropertyValue( const OUString& aProp )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( aProp == "RelId" )
    {
        return uno::Any( m_pImpl->GetNewRelId() );
    }

    OUString aPropertyName;
    if ( aProp == "IsEncrypted" )
        aPropertyName = "Encrypted";
    else
        aPropertyName = aProp;

    if ( ( ( m_nStorageType == embed::StorageFormats::PACKAGE || m_nStorageType == embed::StorageFormats::OFOPXML )
            && aPropertyName == "MediaType" )
      || ( m_nStorageType == embed::StorageFormats::PACKAGE && aPropertyName == "Encrypted" )
      || aPropertyName == "Compressed" )
    {
        m_pImpl->GetStreamProperties();

        auto pProp = std::find_if(std::cbegin(m_pImpl->m_aProps), std::cend(m_pImpl->m_aProps),
            [&aPropertyName](const css::beans::PropertyValue& rProp){ return aPropertyName == rProp.Name; });
        if (pProp != std::cend(m_pImpl->m_aProps))
            return pProp->Value;
    }
    else if ( m_nStorageType == embed::StorageFormats::PACKAGE
            && aPropertyName == "UseCommonStoragePasswordEncryption" )
        return uno::Any( m_pImpl->m_bUseCommonEncryption );
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

        return uno::Any( m_xSeekable->getLength() );
    }

    throw beans::UnknownPropertyException(aPropertyName); // TODO
}

void SAL_CALL OWriteStream::addPropertyChangeListener(
    const OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

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
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

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
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

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
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

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

    lang::EventObject aSource( getXWeak() );

    comphelper::OInterfaceContainerHelper2* pContainer =
            m_aListenersContainer.getContainer(
                cppu::UnoType<embed::XTransactionListener>::get());
    if ( !pContainer )
           return;

    comphelper::OInterfaceIteratorHelper2 pIterator( *pContainer );
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

        osl::ClearableMutexGuard aGuard(m_xSharedMutex->GetMutex());

        if ( !m_pImpl )
        {
            SAL_INFO("package.xstor", "Disposed!");
            throw lang::DisposedException();
        }

        m_pImpl->Commit();

        // when the storage is committed the parent is modified
        ModifyParentUnlockMutex_Impl( aGuard );
    }
    catch( const io::IOException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));
        throw embed::StorageWrappedTargetException( u"Problems on commit!"_ustr,
                                  getXWeak(),
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

    {
        osl::MutexGuard aGuard(m_xSharedMutex->GetMutex());

        if (!m_pImpl)
        {
            SAL_INFO("package.xstor", "Disposed!");
            throw lang::DisposedException();
        }

        try {
            m_pImpl->Revert();
        }
        catch (const io::IOException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
        }
        catch (const embed::StorageWrappedTargetException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
        }
        catch (const uno::RuntimeException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
        }
        catch (const uno::Exception&)
        {
            uno::Any aCaught(::cppu::getCaughtException());
            SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));
            throw embed::StorageWrappedTargetException(u"Problems on revert!"_ustr,
                getXWeak(),
                aCaught);
        }
    }

    BroadcastTransaction( STOR_MESS_REVERTED );
}

//  XTransactionBroadcaster

void SAL_CALL OWriteStream::addTransactionListener( const uno::Reference< embed::XTransactionListener >& aListener )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_bTransacted )
        throw uno::RuntimeException();

    m_aListenersContainer.addInterface( cppu::UnoType<embed::XTransactionListener>::get(),
                                                aListener );
}

void SAL_CALL OWriteStream::removeTransactionListener( const uno::Reference< embed::XTransactionListener >& aListener )
{
    ::osl::MutexGuard aGuard( m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", "Disposed!");
        throw lang::DisposedException();
    }

    if ( !m_bTransacted )
        throw uno::RuntimeException();

    m_aListenersContainer.removeInterface( cppu::UnoType<embed::XTransactionListener>::get(),
                                                    aListener );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
