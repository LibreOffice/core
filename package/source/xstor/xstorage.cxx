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

#include <cassert>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/InvalidStorageException.hpp>
#include <com/sun/star/embed/UseBackupException.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/embed/StorageWrappedTargetException.hpp>
#include <com/sun/star/packages/NoRawFormatException.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/XCloneable.hpp>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <PackageConstants.hxx>

#include <comphelper/sequence.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/instance.hxx>

#include <comphelper/storagehelper.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <tools/diagnose_ex.h>

#include "xstorage.hxx"
#include "owriteablestream.hxx"
#include "disposelistener.hxx"
#include "switchpersistencestream.hxx"
#include "ohierarchyholder.hxx"

using namespace ::com::sun::star;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

struct StorInternalData_Impl
{
    rtl::Reference<comphelper::RefCountedMutex> m_xSharedMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper m_aListenersContainer; // list of listeners
    ::std::unique_ptr< ::cppu::OTypeCollection> m_pTypeCollection;
    bool m_bIsRoot;
    sal_Int32 m_nStorageType; // the mode in which the storage is used
    bool m_bReadOnlyWrap;

    ::rtl::Reference<OChildDispListener_Impl> m_pSubElDispListener;

    ::std::vector< uno::WeakReference< lang::XComponent > > m_aOpenSubComponentsVector;

    ::rtl::Reference< OHierarchyHolder_Impl > m_rHierarchyHolder;

    // the mutex reference MUST NOT be empty
    StorInternalData_Impl( const rtl::Reference<comphelper::RefCountedMutex>& rMutexRef, bool bRoot, sal_Int32 nStorageType, bool bReadOnlyWrap )
    : m_xSharedMutex( rMutexRef )
    , m_aListenersContainer( rMutexRef->GetMutex() )
    , m_pTypeCollection()
    , m_bIsRoot( bRoot )
    , m_nStorageType( nStorageType )
    , m_bReadOnlyWrap( bReadOnlyWrap )
    , m_pSubElDispListener()
    {}
};

// static
void OStorage_Impl::completeStorageStreamCopy_Impl(
                            const uno::Reference< io::XStream >& xSource,
                            const uno::Reference< io::XStream >& xDest,
                            sal_Int32 nStorageType,
                            const uno::Sequence< uno::Sequence< beans::StringPair > >& aRelInfo )
{
        uno::Reference< beans::XPropertySet > xSourceProps( xSource, uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xDestProps( xDest, uno::UNO_QUERY_THROW );

        uno::Reference< io::XOutputStream > xDestOutStream = xDest->getOutputStream();
        if ( !xDestOutStream.is() )
            throw io::IOException( THROW_WHERE );

        uno::Reference< io::XInputStream > xSourceInStream = xSource->getInputStream();
        if ( !xSourceInStream.is() )
            throw io::IOException( THROW_WHERE );

        // TODO: headers of encrypted streams should be copied also
        ::comphelper::OStorageHelper::CopyInputToOutput( xSourceInStream, xDestOutStream );

        uno::Sequence<OUString> aPropNames { "Compressed" };

        if ( nStorageType == embed::StorageFormats::PACKAGE )
        {
            aPropNames.realloc( 3 );
            aPropNames[1] = "MediaType";
            aPropNames[2] = "UseCommonStoragePasswordEncryption";
        }
        else if ( nStorageType == embed::StorageFormats::OFOPXML )
        {
            // TODO/LATER: in future it might make sense to provide the stream if there is one
            uno::Reference< embed::XRelationshipAccess > xRelAccess( xDest, uno::UNO_QUERY_THROW );
            xRelAccess->clearRelationships();
            xRelAccess->insertRelationships( aRelInfo, false );

            aPropNames.realloc( 2 );
            aPropNames[1] = "MediaType";
        }

        for ( const auto& rPropName : std::as_const(aPropNames) )
            xDestProps->setPropertyValue( rPropName, xSourceProps->getPropertyValue( rPropName ) );
}

static uno::Reference< io::XInputStream > GetSeekableTempCopy( const uno::Reference< io::XInputStream >& xInStream,
                                                        const uno::Reference< uno::XComponentContext >& xContext )
{
    uno::Reference < io::XTempFile > xTempFile = io::TempFile::create(xContext);
    uno::Reference < io::XOutputStream > xTempOut = xTempFile->getOutputStream();
    uno::Reference < io::XInputStream > xTempIn = xTempFile->getInputStream();

    if ( !xTempOut.is() || !xTempIn.is() )
        throw io::IOException( THROW_WHERE );

    ::comphelper::OStorageHelper::CopyInputToOutput( xInStream, xTempOut );
    xTempOut->closeOutput();

    return xTempIn;
}

SotElement_Impl::SotElement_Impl(const OUString& rName, bool bStor, bool bNew)
    : m_aOriginalName(rName)
    , m_bIsRemoved(false)
    , m_bIsInserted(bNew)
    , m_bIsStorage(bStor)
{
}

// most of properties are holt by the storage but are not used
OStorage_Impl::OStorage_Impl(   uno::Reference< io::XInputStream > const & xInputStream,
                                sal_Int32 nMode,
                                const uno::Sequence< beans::PropertyValue >& xProperties,
                                uno::Reference< uno::XComponentContext > const & xContext,
                                sal_Int32 nStorageType )
: m_xMutex( new comphelper::RefCountedMutex )
, m_pAntiImpl( nullptr )
, m_nStorageMode( nMode & ~embed::ElementModes::SEEKABLE )
, m_bIsModified( ( nMode & ( embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE ) ) == ( embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE ) )
, m_bBroadcastModified( false )
, m_bCommited( false )
, m_bIsRoot( true )
, m_bListCreated( false )
, m_nModifiedListenerCount( 0 )
, m_xContext( xContext )
, m_xProperties( xProperties )
, m_bHasCommonEncryptionData( false )
, m_pParent( nullptr )
, m_bControlMediaType( false )
, m_bMTFallbackUsed( false )
, m_bControlVersion( false )
, m_pSwitchStream( nullptr )
, m_nStorageType( nStorageType )
, m_pRelStorElement( nullptr )
, m_nRelInfoStatus( RELINFO_NO_INIT )
{
    // all the checks done below by assertion statements must be done by factory
    SAL_WARN_IF( !xInputStream.is(), "package.xstor", "No input stream is provided!" );
    assert(xContext.is());

    m_pSwitchStream = new SwitchablePersistenceStream(xContext, xInputStream);
    m_xInputStream = m_pSwitchStream->getInputStream();

    if ( m_nStorageMode & embed::ElementModes::WRITE )
    {
        // check that the stream allows to write
        SAL_WARN( "package.xstor", "No stream for writing is provided!" );
    }
}

// most of properties are holt by the storage but are not used
OStorage_Impl::OStorage_Impl(   uno::Reference< io::XStream > const & xStream,
                                sal_Int32 nMode,
                                const uno::Sequence< beans::PropertyValue >& xProperties,
                                uno::Reference< uno::XComponentContext > const & xContext,
                                sal_Int32 nStorageType )
: m_xMutex( new comphelper::RefCountedMutex )
, m_pAntiImpl( nullptr )
, m_nStorageMode( nMode & ~embed::ElementModes::SEEKABLE )
, m_bIsModified( ( nMode & ( embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE ) ) == ( embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE ) )
, m_bBroadcastModified( false )
, m_bCommited( false )
, m_bIsRoot( true )
, m_bListCreated( false )
, m_nModifiedListenerCount( 0 )
, m_xContext( xContext )
, m_xProperties( xProperties )
, m_bHasCommonEncryptionData( false )
, m_pParent( nullptr )
, m_bControlMediaType( false )
, m_bMTFallbackUsed( false )
, m_bControlVersion( false )
, m_pSwitchStream( nullptr )
, m_nStorageType( nStorageType )
, m_pRelStorElement( nullptr )
, m_nRelInfoStatus( RELINFO_NO_INIT )
{
    // all the checks done below by assertion statements must be done by factory
    SAL_WARN_IF( !xStream.is(), "package.xstor", "No stream is provided!" );
    assert(xContext.is());

    if ( m_nStorageMode & embed::ElementModes::WRITE )
    {
        m_pSwitchStream = new SwitchablePersistenceStream(xContext, xStream);
        m_xStream = static_cast< io::XStream* >( m_pSwitchStream );
    }
    else
    {
        m_pSwitchStream = new SwitchablePersistenceStream(xContext, xStream->getInputStream());
        m_xInputStream = m_pSwitchStream->getInputStream();
    }
}

OStorage_Impl::OStorage_Impl(   OStorage_Impl* pParent,
                                sal_Int32 nMode,
                                uno::Reference< container::XNameContainer > const & xPackageFolder,
                                uno::Reference< lang::XSingleServiceFactory > const & xPackage,
                                uno::Reference< uno::XComponentContext > const & xContext,
                                sal_Int32 nStorageType )
: m_xMutex( new comphelper::RefCountedMutex )
, m_pAntiImpl( nullptr )
, m_nStorageMode( nMode & ~embed::ElementModes::SEEKABLE )
, m_bIsModified( ( nMode & ( embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE ) ) == ( embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE ) )
, m_bBroadcastModified( false )
, m_bCommited( false )
, m_bIsRoot( false )
, m_bListCreated( false )
, m_nModifiedListenerCount( 0 )
, m_xPackageFolder( xPackageFolder )
, m_xPackage( xPackage )
, m_xContext( xContext )
, m_bHasCommonEncryptionData( false )
, m_pParent( pParent ) // can be empty in case of temporary readonly substorages and relation storage
, m_bControlMediaType( false )
, m_bMTFallbackUsed( false )
, m_bControlVersion( false )
, m_pSwitchStream( nullptr )
, m_nStorageType( nStorageType )
, m_pRelStorElement( nullptr )
, m_nRelInfoStatus( RELINFO_NO_INIT )
{
    SAL_WARN_IF( !xPackageFolder.is(), "package.xstor", "No package folder!" );
    assert(xContext.is());
}

OStorage_Impl::~OStorage_Impl()
{
    {
        ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );
        if ( m_pAntiImpl ) // root storage wrapper must set this member to NULL before destruction of object
        {
            SAL_WARN_IF( m_bIsRoot, "package.xstor", "The root storage wrapper must be disposed already" );

            try {
                m_pAntiImpl->InternalDispose( false );
            }
            catch ( const uno::Exception& )
            {
                TOOLS_INFO_EXCEPTION("package.xstor", "Quiet exception");
            }
            m_pAntiImpl = nullptr;
        }
        else if ( !m_aReadOnlyWrapVector.empty() )
        {
            for ( auto& rStorage : m_aReadOnlyWrapVector )
            {
                uno::Reference< embed::XStorage > xTmp = rStorage.m_xWeakRef;
                if ( xTmp.is() )
                    try {
                        rStorage.m_pPointer->InternalDispose( false );
                    } catch( const uno::Exception& )
                    {
                        TOOLS_INFO_EXCEPTION("package.xstor", "Quiet exception");
                    }
            }

            m_aReadOnlyWrapVector.clear();
        }

        m_pParent = nullptr;
    }

    for (const auto & pair : m_aChildrenMap)
        for (auto pElement : pair.second)
            delete pElement;
    m_aChildrenMap.clear();

    std::for_each(m_aDeletedVector.begin(), m_aDeletedVector.end(), std::default_delete<SotElement_Impl>());
    m_aDeletedVector.clear();

    if ( m_nStorageType == embed::StorageFormats::OFOPXML && m_pRelStorElement )
    {
        delete m_pRelStorElement;
        m_pRelStorElement = nullptr;
    }

    m_xPackageFolder.clear();
    m_xPackage.clear();

    OUString aPropertyName = "URL";
    for ( const auto& rProp : std::as_const(m_xProperties) )
    {
        if ( rProp.Name == aPropertyName )
        {
            // the storage is URL based so all the streams are opened by factory and should be closed
            try
            {
                if ( m_xInputStream.is() )
                {
                    m_xInputStream->closeInput();
                    m_xInputStream.clear();
                }

                if ( m_xStream.is() )
                {
                    uno::Reference< io::XInputStream > xInStr = m_xStream->getInputStream();
                    if ( xInStr.is() )
                        xInStr->closeInput();

                    uno::Reference< io::XOutputStream > xOutStr = m_xStream->getOutputStream();
                    if ( xOutStr.is() )
                        xOutStr->closeOutput();

                    m_xStream.clear();
                }
            }
            catch (const uno::Exception&)
            {
                TOOLS_INFO_EXCEPTION("package.xstor", "Quiet exception");
            }
        }
    }
}

void OStorage_Impl::SetReadOnlyWrap( OStorage& aStorage )
{
    // Weak reference is used inside the holder so the refcount must not be zero at this point
    OSL_ENSURE( aStorage.GetRefCount_Impl(), "There must be a reference alive to use this method!" );
    m_aReadOnlyWrapVector.emplace_back( &aStorage );
}

void OStorage_Impl::RemoveReadOnlyWrap( OStorage& aStorage )
{
    for ( StorageHoldersType::iterator pStorageIter = m_aReadOnlyWrapVector.begin();
      pStorageIter != m_aReadOnlyWrapVector.end();)
    {
        uno::Reference< embed::XStorage > xTmp = pStorageIter->m_xWeakRef;
        if ( !xTmp.is() || pStorageIter->m_pPointer == &aStorage )
        {
            try {
                pStorageIter->m_pPointer->InternalDispose( false );
            } catch( const uno::Exception& )
            {
                TOOLS_INFO_EXCEPTION("package.xstor", "Quiet exception");
            }

            pStorageIter = m_aReadOnlyWrapVector.erase(pStorageIter);
        }
        else
            ++pStorageIter;
    }
}

void OStorage_Impl::OpenOwnPackage()
{
    SAL_WARN_IF( !m_bIsRoot, "package.xstor", "Opening of the package has no sense!" );

    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( !m_xPackageFolder.is() )
    {
        if ( !m_xPackage.is() )
        {
            uno::Sequence< uno::Any > aArguments( 2 );
            if ( m_nStorageMode & embed::ElementModes::WRITE )
                aArguments[ 0 ] <<= m_xStream;
            else
            {
                SAL_WARN_IF( !m_xInputStream.is(), "package.xstor", "Input stream must be set for readonly access!" );
                aArguments[ 0 ] <<= m_xInputStream;
                // TODO: if input stream is not seekable or XSeekable interface is supported
                // on XStream object a wrapper must be used
            }

            // do not allow elements to remove themself from the old container in case of insertion to another container
            aArguments[ 1 ] <<= beans::NamedValue( "AllowRemoveOnInsert",
                                                    uno::makeAny( false ) );

            sal_Int32 nArgNum = 2;
            for ( const auto& rProp : std::as_const(m_xProperties) )
            {
                if ( rProp.Name == "RepairPackage"
                  || rProp.Name == "ProgressHandler"
                  || rProp.Name == "NoFileSync" )
                {
                    // Forward these to the package.
                    beans::NamedValue aNamedValue( rProp.Name, rProp.Value );
                    aArguments.realloc( ++nArgNum );
                    aArguments[nArgNum-1] <<= aNamedValue;
                }
                else if ( rProp.Name == "Password" )
                {
                    // TODO: implement password setting for documents
                    // the password entry must be removed after setting
                }
            }

            if ( m_nStorageType == embed::StorageFormats::ZIP )
            {
                // let the package support only plain zip format
                beans::NamedValue aNamedValue;
                aNamedValue.Name = "StorageFormat";
                aNamedValue.Value <<= OUString( "ZipFormat" );
                aArguments.realloc( ++nArgNum );
                aArguments[nArgNum-1] <<= aNamedValue;
            }
            else if ( m_nStorageType == embed::StorageFormats::OFOPXML )
            {
                // let the package support OFOPXML media type handling
                beans::NamedValue aNamedValue;
                aNamedValue.Name = "StorageFormat";
                aNamedValue.Value <<= OUString( "OFOPXMLFormat" );
                aArguments.realloc( ++nArgNum );
                aArguments[nArgNum-1] <<= aNamedValue;
            }

            m_xPackage.set( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                               "com.sun.star.packages.comp.ZipPackage", aArguments, m_xContext),
                            uno::UNO_QUERY );
        }

        uno::Reference< container::XHierarchicalNameAccess > xHNameAccess( m_xPackage, uno::UNO_QUERY );
        SAL_WARN_IF( !xHNameAccess.is(), "package.xstor", "The package could not be created!" );

        if ( xHNameAccess.is() )
        {
            uno::Any aFolder = xHNameAccess->getByHierarchicalName("/");
            aFolder >>= m_xPackageFolder;
        }
    }

    SAL_WARN_IF( !m_xPackageFolder.is(), "package.xstor", "The package root folder can not be opened!" );
    if ( !m_xPackageFolder.is() )
        throw embed::InvalidStorageException( THROW_WHERE );
}

bool OStorage_Impl::HasChildren()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    ReadContents();
    return !m_aChildrenMap.empty();
}

void OStorage_Impl::GetStorageProperties()
{
    if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        return;

    uno::Reference< beans::XPropertySet > xProps( m_xPackageFolder, uno::UNO_QUERY_THROW );

    if ( !m_bControlMediaType )
    {
        uno::Reference< beans::XPropertySet > xPackageProps( m_xPackage, uno::UNO_QUERY_THROW );
        xPackageProps->getPropertyValue( MEDIATYPE_FALLBACK_USED_PROPERTY ) >>= m_bMTFallbackUsed;

        xProps->getPropertyValue( "MediaType" ) >>= m_aMediaType;
        m_bControlMediaType = true;
    }

    if ( !m_bControlVersion )
    {
        xProps->getPropertyValue( "Version" ) >>= m_aVersion;
        m_bControlVersion = true;
    }

    // the properties of OFOPXML will be handled directly
}

void OStorage_Impl::ReadRelInfoIfNecessary()
{
    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        return;

    if ( m_nRelInfoStatus == RELINFO_NO_INIT )
    {
        // Init from original stream
        uno::Reference< io::XInputStream > xRelInfoStream = GetRelInfoStreamForName( OUString() );
        try
        {
            if ( xRelInfoStream.is() )
                m_aRelInfo = ::comphelper::OFOPXMLHelper::ReadRelationsInfoSequence(
                                    xRelInfoStream,
                                    "_rels/.rels",
                                    m_xContext );
            m_nRelInfoStatus = RELINFO_READ;
        }
        catch (css::uno::Exception &)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "");
        }
    }
    else if ( m_nRelInfoStatus == RELINFO_CHANGED_STREAM )
    {
        // Init from the new stream
        try
        {
            if ( m_xNewRelInfoStream.is() )
                m_aRelInfo = ::comphelper::OFOPXMLHelper::ReadRelationsInfoSequence(
                                        m_xNewRelInfoStream,
                                        "_rels/.rels",
                                        m_xContext );

            m_nRelInfoStatus = RELINFO_CHANGED_STREAM_READ;
        }
        catch( const uno::Exception& )
        {
            m_nRelInfoStatus = RELINFO_CHANGED_BROKEN;
        }
    }
}

void OStorage_Impl::ReadContents()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( m_bListCreated )
        return;

    if ( m_bIsRoot )
        OpenOwnPackage();

    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xPackageFolder, uno::UNO_QUERY_THROW );
    uno::Reference< container::XEnumeration > xEnum = xEnumAccess->createEnumeration();
    if ( !xEnum.is() )
        throw uno::RuntimeException( THROW_WHERE );

    m_bListCreated = true;

    while( xEnum->hasMoreElements() )
    {
        try {
            uno::Reference< container::XNamed > xNamed;
            xEnum->nextElement() >>= xNamed;

            if ( !xNamed.is() )
            {
                SAL_WARN( "package.xstor", "XNamed is not supported!" );
                throw uno::RuntimeException( THROW_WHERE );
            }

            OUString aName = xNamed->getName();
            SAL_WARN_IF( aName.isEmpty(), "package.xstor", "Empty name!" );

            uno::Reference< container::XNameContainer > xNameContainer( xNamed, uno::UNO_QUERY );

            std::unique_ptr<SotElement_Impl> xNewElement(new SotElement_Impl(aName, xNameContainer.is(), false));
            if ( m_nStorageType == embed::StorageFormats::OFOPXML && aName == "_rels" )
            {
                if (!xNewElement->m_bIsStorage)
                    throw io::IOException( THROW_WHERE ); // TODO: Unexpected format

                m_pRelStorElement = xNewElement.release();
                CreateRelStorage();
            }
            else
            {
                if ( ( m_nStorageMode & embed::ElementModes::TRUNCATE ) == embed::ElementModes::TRUNCATE )
                {
                    // if a storage is truncated all of it elements are marked as deleted
                    xNewElement->m_bIsRemoved = true;
                }

                m_aChildrenMap[aName].push_back(xNewElement.release());
            }
        }
        catch( const container::NoSuchElementException& )
        {
            TOOLS_WARN_EXCEPTION( "package.xstor", "hasMoreElements() implementation has problems!");
            break;
        }
    }
    if ( ( m_nStorageMode & embed::ElementModes::TRUNCATE ) == embed::ElementModes::TRUNCATE )
    {
        // if a storage is truncated the relations information should be cleaned
        m_xNewRelInfoStream.clear();
        m_aRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
        m_nRelInfoStatus = RELINFO_CHANGED;
    }

    // cache changeable folder properties
    GetStorageProperties();
}

void OStorage_Impl::CopyToStorage( const uno::Reference< embed::XStorage >& xDest, bool bDirect )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    uno::Reference< beans::XPropertySet > xPropSet( xDest, uno::UNO_QUERY );
    if ( !xPropSet.is() )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );

    sal_Int32 nDestMode = embed::ElementModes::READ;
    xPropSet->getPropertyValue( "OpenMode" ) >>= nDestMode;

    if ( !( nDestMode & embed::ElementModes::WRITE ) )
        throw io::IOException( THROW_WHERE ); // TODO: access_denied

    ReadContents();

    if ( !m_xPackageFolder.is() )
        throw embed::InvalidStorageException( THROW_WHERE );

    for ( const auto& pair : m_aChildrenMap )
        for (auto pElement : pair.second)
        {
            if ( !pElement->m_bIsRemoved )
                CopyStorageElement( pElement, xDest, /*aName*/pair.first, bDirect );
        }

    // move storage properties to the destination one ( means changeable properties )
    if ( m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        xPropSet->setPropertyValue( "MediaType", uno::makeAny( m_aMediaType ) );
        xPropSet->setPropertyValue( "Version", uno::makeAny( m_aVersion ) );
    }

    if ( m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        // if this is a root storage, the common key from current one should be moved there
        bool bIsRoot = false;
        if ( ( xPropSet->getPropertyValue( "IsRoot" ) >>= bIsRoot ) && bIsRoot )
        {
            try
            {
                uno::Reference< embed::XEncryptionProtectedStorage > xEncr( xDest, uno::UNO_QUERY );
                if ( xEncr.is() )
                {
                    xEncr->setEncryptionData( GetCommonRootEncryptionData().getAsConstNamedValueList() );

                    uno::Sequence< beans::NamedValue > aAlgorithms;
                    uno::Reference< beans::XPropertySet > xPackPropSet( m_xPackage, uno::UNO_QUERY_THROW );
                    xPackPropSet->getPropertyValue( ENCRYPTION_ALGORITHMS_PROPERTY )
                        >>= aAlgorithms;
                    xEncr->setEncryptionAlgorithms( aAlgorithms );
                }
            }
            catch( const packages::NoEncryptionException& )
            {
                TOOLS_INFO_EXCEPTION("package.xstor", "No Encryption");
            }
        }
    }
    else if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {

        // TODO/LATER: currently the optimization is not active
        // uno::Reference< io::XInputStream > xRelInfoStream = GetRelInfoStreamForName( OUString() ); // own stream
        // if ( xRelInfoStream.is() )
        // {
        //  // Relations info stream is a writeonly property, introduced only to optimize copying
        //  // Should be used carefully since no check for stream consistency is done, and the stream must not stay locked

        //  OUString aRelInfoString = "RelationsInfoStream";
        //  xPropSet->setPropertyValue( aRelInfoString, uno::makeAny( GetSeekableTempCopy( xRelInfoStream, m_xFactory ) ) );
        // }

        uno::Reference< embed::XRelationshipAccess > xRels( xDest, uno::UNO_QUERY );
        if ( !xRels.is() )
            throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );

        xRels->insertRelationships( GetAllRelationshipsIfAny(), false );
    }

    // if possible the destination storage should be committed after successful copying
    uno::Reference< embed::XTransactedObject > xObjToCommit( xDest, uno::UNO_QUERY );
    if ( xObjToCommit.is() )
        xObjToCommit->commit();
}

void OStorage_Impl::CopyStorageElement( SotElement_Impl* pElement,
                                        const uno::Reference< embed::XStorage >& xDest,
                                        const OUString& aName,
                                        bool bDirect )
{
    SAL_WARN_IF( !xDest.is(), "package.xstor", "No destination storage!" );
    SAL_WARN_IF( aName.isEmpty(), "package.xstor", "Empty element name!" );

    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    uno::Reference< container::XNameAccess > xDestAccess( xDest, uno::UNO_QUERY_THROW );
    if ( xDestAccess->hasByName( aName )
      && !( pElement->m_bIsStorage && xDest->isStorageElement( aName ) ) )
        xDest->removeElement( aName );

    if ( pElement->m_bIsStorage )
    {
        uno::Reference< embed::XStorage > xSubDest =
                                    xDest->openStorageElement(  aName,
                                                                embed::ElementModes::WRITE );

        SAL_WARN_IF( !xSubDest.is(), "package.xstor", "No destination substorage!" );

        if (!pElement->m_xStorage)
        {
            OpenSubStorage( pElement, embed::ElementModes::READ );
            if (!pElement->m_xStorage)
                throw io::IOException( THROW_WHERE );
        }

        pElement->m_xStorage->CopyToStorage(xSubDest, bDirect);
    }
    else
    {
        if (!pElement->m_xStream)
        {
            OpenSubStream( pElement );
            if (!pElement->m_xStream)
                throw io::IOException( THROW_WHERE );
        }

        if (!pElement->m_xStream->IsEncrypted())
        {
            if ( bDirect )
            {
                // fill in the properties for the stream
                uno::Sequence< beans::PropertyValue > aStrProps(0);
                const uno::Sequence< beans::PropertyValue > aSrcPkgProps = pElement->m_xStream->GetStreamProperties();
                sal_Int32 nNum = 0;
                for ( const auto& rSrcPkgProp : aSrcPkgProps )
                {
                    if ( rSrcPkgProp.Name == "MediaType" || rSrcPkgProp.Name == "Compressed" )
                    {
                        aStrProps.realloc( ++nNum );
                        aStrProps[nNum-1].Name = rSrcPkgProp.Name;
                        aStrProps[nNum-1].Value = rSrcPkgProp.Value;
                    }
                }

                if ( m_nStorageType == embed::StorageFormats::PACKAGE )
                {
                    aStrProps.realloc( ++nNum );
                    aStrProps[nNum-1].Name = "UseCommonStoragePasswordEncryption";
                    aStrProps[nNum-1].Value <<= pElement->m_xStream->UsesCommonEncryption_Impl();
                }
                else if ( m_nStorageType == embed::StorageFormats::OFOPXML )
                {
                    // TODO/LATER: currently the optimization is not active
                    // uno::Reference< io::XInputStream > xInStream = GetRelInfoStreamForName( OUString() ); // own rels stream
                    // if ( xInStream.is() )
                    // {
                    //  aStrProps.realloc( ++nNum );
                    //  aStrProps[nNum-1].Name = "RelationsInfoStream";
                    //  aStrProps[nNum-1].Value <<= GetSeekableTempCopy( xInStream, m_xFactory );
                    // }

                    uno::Reference< embed::XRelationshipAccess > xRels( xDest, uno::UNO_QUERY );
                    if ( !xRels.is() )
                        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );

                    xRels->insertRelationships( GetAllRelationshipsIfAny(), false );
                }

                uno::Reference< embed::XOptimizedStorage > xOptDest( xDest, uno::UNO_QUERY_THROW );
                uno::Reference < io::XInputStream > xInputToInsert;

                if (pElement->m_xStream->HasTempFile_Impl() || !pElement->m_xStream->m_xPackageStream.is())
                {
                    SAL_WARN_IF(!pElement->m_xStream->m_xPackageStream.is(), "package.xstor", "No package stream!");

                    // if the stream is modified - the temporary file must be used for insertion
                    xInputToInsert = pElement->m_xStream->GetTempFileAsInputStream();
                }
                else
                {
                    // for now get just nonseekable access to the stream
                    // TODO/LATER: the raw stream can be used

                    xInputToInsert = pElement->m_xStream->m_xPackageStream->getDataStream();
                }

                if ( !xInputToInsert.is() )
                        throw io::IOException( THROW_WHERE );

                xOptDest->insertStreamElementDirect( aName, xInputToInsert, aStrProps );
            }
            else
            {
                uno::Reference< io::XStream > xSubStr =
                                            xDest->openStreamElement( aName,
                                            embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
                SAL_WARN_IF( !xSubStr.is(), "package.xstor", "No destination substream!" );

                pElement->m_xStream->CopyInternallyTo_Impl(xSubStr);
            }
        }
        else if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        {
            SAL_WARN( "package.xstor", "Encryption is only supported in package storage!" );
            throw io::IOException( THROW_WHERE );
        }
        else if ( pElement->m_xStream->HasCachedEncryptionData()
             && ( pElement->m_xStream->IsModified() || pElement->m_xStream->HasWriteOwner_Impl() ) )
        {
            ::comphelper::SequenceAsHashMap aCommonEncryptionData;
            bool bHasCommonEncryptionData = false;
            try
            {
                aCommonEncryptionData = GetCommonRootEncryptionData();
                bHasCommonEncryptionData = true;
            }
            catch( const packages::NoEncryptionException& )
            {
                TOOLS_INFO_EXCEPTION("package.xstor", "No Encryption");
            }

            if (bHasCommonEncryptionData && ::package::PackageEncryptionDataLessOrEqual(pElement->m_xStream->GetCachedEncryptionData(), aCommonEncryptionData))
            {
                // If the stream can be opened with the common storage password
                // it must be stored with the common storage password as well
                uno::Reference< io::XStream > xDestStream =
                                            xDest->openStreamElement( aName,
                                                embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );

                pElement->m_xStream->CopyInternallyTo_Impl( xDestStream );

                uno::Reference< beans::XPropertySet > xProps( xDestStream, uno::UNO_QUERY_THROW );
                xProps->setPropertyValue(
                    "UseCommonStoragePasswordEncryption",
                    uno::Any( true ) );
            }
            else
            {
                // the stream is already opened for writing or was changed
                uno::Reference< embed::XStorage2 > xDest2( xDest, uno::UNO_QUERY_THROW );
                uno::Reference< io::XStream > xSubStr =
                                            xDest2->openEncryptedStream( aName,
                                                embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE,
                                                pElement->m_xStream->GetCachedEncryptionData().getAsConstNamedValueList() );
                SAL_WARN_IF( !xSubStr.is(), "package.xstor", "No destination substream!" );

                pElement->m_xStream->CopyInternallyTo_Impl(xSubStr, pElement->m_xStream->GetCachedEncryptionData());
            }
        }
        else
        {
            // the stream is not opened at all, so it can be just opened for reading
            try
            {
                // If the stream can be opened with the common storage password
                // it must be stored with the common storage password as well

                uno::Reference< io::XStream > xOwnStream = pElement->m_xStream->GetStream(embed::ElementModes::READ,
                                                                                          false);
                uno::Reference< io::XStream > xDestStream =
                                            xDest->openStreamElement( aName,
                                                embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
                SAL_WARN_IF( !xDestStream.is(), "package.xstor", "No destination substream!" );
                completeStorageStreamCopy_Impl( xOwnStream, xDestStream, m_nStorageType, GetAllRelationshipsIfAny() );

                uno::Reference< beans::XPropertySet > xProps( xDestStream, uno::UNO_QUERY_THROW );
                xProps->setPropertyValue(
                    "UseCommonStoragePasswordEncryption",
                    uno::Any( true ) );
            }
            catch( const packages::WrongPasswordException& )
            {
                TOOLS_INFO_EXCEPTION("package.xstor", "Handled exception");

                // If the common storage password does not allow to open the stream
                // it could be copied in raw way, the problem is that the StartKey should be the same
                // in the ODF1.2 package, so an invalid package could be produced if the stream
                // is copied from ODF1.1 package, where it is allowed to have different StartKeys
                uno::Reference< embed::XStorageRawAccess > xRawDest( xDest, uno::UNO_QUERY_THROW );
                uno::Reference< io::XInputStream > xRawInStream = pElement->m_xStream->GetRawInStream();
                xRawDest->insertRawEncrStreamElement( aName, xRawInStream );
            }
        }
    }
}

uno::Sequence< uno::Sequence< beans::StringPair > > OStorage_Impl::GetAllRelationshipsIfAny()
{
    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        return uno::Sequence< uno::Sequence< beans::StringPair > >();

    ReadRelInfoIfNecessary();

    if ( m_nRelInfoStatus != RELINFO_READ
         && m_nRelInfoStatus != RELINFO_CHANGED_STREAM_READ
         && m_nRelInfoStatus != RELINFO_CHANGED )
            throw io::IOException( THROW_WHERE "Wrong relinfo stream!" );
    // m_nRelInfoStatus == RELINFO_CHANGED_BROKEN || m_nRelInfoStatus == RELINFO_BROKEN
    return m_aRelInfo;
}

void OStorage_Impl::CopyLastCommitTo( const uno::Reference< embed::XStorage >& xNewStor )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    SAL_WARN_IF( !m_xPackageFolder.is(), "package.xstor", "A committed storage is incomplete!" );
    if ( !m_xPackageFolder.is() )
        throw uno::RuntimeException( THROW_WHERE );

    OStorage_Impl aTempRepresent( nullptr,
                                embed::ElementModes::READ,
                                m_xPackageFolder,
                                m_xPackage,
                                m_xContext,
                                m_nStorageType);

    // TODO/LATER: could use direct copying
    aTempRepresent.CopyToStorage( xNewStor, false );
}

void OStorage_Impl::InsertIntoPackageFolder( const OUString& aName,
                                             const uno::Reference< container::XNameContainer >& xParentPackageFolder )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    SAL_WARN_IF( !m_xPackageFolder.is(), "package.xstor", "An inserted storage is incomplete!" );
    uno::Reference< lang::XUnoTunnel > xTunnel( m_xPackageFolder, uno::UNO_QUERY_THROW );
    xParentPackageFolder->insertByName( aName, uno::makeAny( xTunnel ) );

    m_bCommited = false;
}

void OStorage_Impl::Commit()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( !m_bIsModified )
        return;

    // in case of a new empty storage it is possible that the contents are still not read
    // ( the storage of course has no contents, but the initialization is postponed till the first use,
    //   thus if a new storage was created and committed immediately it must be initialized here )
    ReadContents();

    // if storage is committed it should have a valid Package representation
    SAL_WARN_IF( !m_xPackageFolder.is(), "package.xstor", "The package representation should exist!" );
    if ( !m_xPackageFolder.is() )
        throw embed::InvalidStorageException( THROW_WHERE );

    OSL_ENSURE( m_nStorageMode & embed::ElementModes::WRITE,
                "Commit of readonly storage, should be detected before!" );

    uno::Reference< container::XNameContainer > xNewPackageFolder;

    // here the storage will switch to the temporary package folder
    // if the storage was already committed and the parent was not committed after that
    // the switch should not be done since the package folder in use is a temporary one;
    // it can be detected by m_bCommited flag ( root storage doesn't need temporary representation )
    if ( !m_bCommited && !m_bIsRoot )
    {
        uno::Sequence< uno::Any > aSeq( 1 );
        aSeq[0] <<= true;

        xNewPackageFolder.set( m_xPackage->createInstanceWithArguments( aSeq ),
                               uno::UNO_QUERY );
    }
    else
        xNewPackageFolder = m_xPackageFolder;

    // remove replaced removed elements
    for ( auto& pDeleted : m_aDeletedVector )
    {

        if ( m_nStorageType == embed::StorageFormats::OFOPXML && !pDeleted->m_bIsStorage )
            RemoveStreamRelInfo( pDeleted->m_aOriginalName );

        // the removed elements are not in new temporary storage
        if ( m_bCommited || m_bIsRoot )
            xNewPackageFolder->removeByName( pDeleted->m_aOriginalName );
        delete pDeleted;
        pDeleted = nullptr;
    }
    m_aDeletedVector.clear();

    // remove removed elements
    for (auto mapIt = m_aChildrenMap.begin(); mapIt != m_aChildrenMap.end(); )
    {
        for (auto it = mapIt->second.begin(); it != mapIt->second.end(); )
        {
            // renamed and inserted elements must be really inserted to package later
            // since they can conflict with removed elements
            auto & pElement = *it;
            if ( pElement->m_bIsRemoved )
            {
                if ( m_nStorageType == embed::StorageFormats::OFOPXML && !pElement->m_bIsStorage )
                    RemoveStreamRelInfo( pElement->m_aOriginalName );

                // the removed elements are not in new temporary storage
                if ( m_bCommited || m_bIsRoot )
                    xNewPackageFolder->removeByName( pElement->m_aOriginalName );

                delete pElement;
                it = mapIt->second.erase(it);
            }
            else
                ++it;
        }
        if (mapIt->second.empty())
            mapIt = m_aChildrenMap.erase(mapIt);
        else
            ++mapIt;
    }


    // there should be no more deleted elements
    for ( const auto& pair : m_aChildrenMap )
        for (auto pElement : pair.second)
        {
            // if it is a 'duplicate commit' inserted elements must be really inserted to package later
            // since they can conflict with renamed elements
            if ( !pElement->m_bIsInserted )
            {
                // for now stream is opened in direct mode that means that in case
                // storage is committed all the streams from it are committed in current state.
                // following two steps are separated to allow easily implement transacted mode
                // for streams if we need it in future.
                // Only hierarchical access uses transacted streams currently
                if ( !pElement->m_bIsStorage && pElement->m_xStream
                  && !pElement->m_xStream->IsTransacted() )
                    pElement->m_xStream->Commit();

                // if the storage was not open, there is no need to commit it ???
                // the storage should be checked that it is committed
                if (pElement->m_bIsStorage && pElement->m_xStorage && pElement->m_xStorage->m_bCommited)
                {
                    // it's temporary PackageFolder should be inserted instead of current one
                    // also the new copy of PackageFolder should be used by the children storages

                    // the renamed elements are not in new temporary storage
                    if ( m_bCommited || m_bIsRoot )
                        xNewPackageFolder->removeByName( pElement->m_aOriginalName );

                    pElement->m_xStorage->InsertIntoPackageFolder(/*aName*/pair.first, xNewPackageFolder);
                }
                else if (!pElement->m_bIsStorage && pElement->m_xStream && pElement->m_xStream->m_bFlushed)
                {
                    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
                        CommitStreamRelInfo( /*aName*/pair.first, pElement );

                    // the renamed elements are not in new temporary storage
                    if ( m_bCommited || m_bIsRoot )
                        xNewPackageFolder->removeByName( pElement->m_aOriginalName );

                    pElement->m_xStream->InsertIntoPackageFolder(/*aName*/pair.first, xNewPackageFolder);
                }
                else if ( !m_bCommited && !m_bIsRoot )
                {
                    // the element must be just copied to the new temporary package folder
                    // the connection with the original package should not be lost just because
                    // the element is still referred by the folder in the original hierarchy
                    uno::Any aPackageElement = m_xPackageFolder->getByName( pElement->m_aOriginalName );
                    xNewPackageFolder->insertByName( /*aName*/pair.first, aPackageElement );
                }
                else if ( pair.first != pElement->m_aOriginalName )
                {
                    // this is the case when xNewPackageFolder refers to m_xPackageFolder
                    // in case the name was changed and it is not a changed storage - rename the element
                    uno::Any aPackageElement = xNewPackageFolder->getByName( pElement->m_aOriginalName );
                    xNewPackageFolder->removeByName( pElement->m_aOriginalName );
                    xNewPackageFolder->insertByName( /*aName*/pair.first, aPackageElement );

                    if ( m_nStorageType == embed::StorageFormats::OFOPXML && !pElement->m_bIsStorage )
                    {
                        if (!pElement->m_xStream)
                        {
                            OpenSubStream( pElement );
                            if (!pElement->m_xStream)
                                throw uno::RuntimeException( THROW_WHERE );
                        }

                        CommitStreamRelInfo( /*aName*/pair.first, pElement );
                    }
                }

                pElement->m_aOriginalName = pair.first;
            }
        }

    for ( const auto& pair : m_aChildrenMap )
        for (auto pElement : pair.second)
        {
            // now inserted elements can be inserted to the package
            if ( pElement->m_bIsInserted )
            {
                pElement->m_aOriginalName = pair.first;

                if ( pElement->m_bIsStorage )
                {
                    if (pElement->m_xStorage->m_bCommited)
                    {
                        OSL_ENSURE(pElement->m_xStorage, "An inserted storage is incomplete!");
                        if (!pElement->m_xStorage)
                            throw uno::RuntimeException( THROW_WHERE );

                        pElement->m_xStorage->InsertIntoPackageFolder(/*aName*/pair.first, xNewPackageFolder);

                        pElement->m_bIsInserted = false;
                    }
                }
                else
                {
                    OSL_ENSURE(pElement->m_xStream, "An inserted stream is incomplete!");
                    if (!pElement->m_xStream)
                        throw uno::RuntimeException( THROW_WHERE );

                    if (!pElement->m_xStream->IsTransacted())
                        pElement->m_xStream->Commit();

                    if (pElement->m_xStream->m_bFlushed)
                    {
                        if ( m_nStorageType == embed::StorageFormats::OFOPXML )
                            CommitStreamRelInfo( /*aName*/pair.first, pElement );

                        pElement->m_xStream->InsertIntoPackageFolder( /*aName*/pair.first, xNewPackageFolder );

                        pElement->m_bIsInserted = false;
                    }
                }
            }
        }

    if ( m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        // move properties to the destination package folder
        uno::Reference< beans::XPropertySet > xProps( xNewPackageFolder, uno::UNO_QUERY_THROW );
        xProps->setPropertyValue( "MediaType", uno::makeAny( m_aMediaType ) );
        xProps->setPropertyValue( "Version", uno::makeAny( m_aVersion ) );
    }

    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
        CommitRelInfo( xNewPackageFolder ); // store own relations and commit complete relations storage

    if ( m_bIsRoot )
    {
        uno::Reference< util::XChangesBatch > xChangesBatch( m_xPackage, uno::UNO_QUERY_THROW );
        try
        {
            xChangesBatch->commitChanges();
        }
        catch( const lang::WrappedTargetException& r )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            // the wrapped UseBackupException means that the target medium can be corrupted
            embed::UseBackupException aException;
            if ( r.TargetException >>= aException )
            {
                m_xStream.clear();
                m_xInputStream.clear();
                throw aException;
            }

            SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(ex));
            throw;
        }
    }
    else if ( !m_bCommited )
    {
        m_xPackageFolder = xNewPackageFolder;
        m_bCommited = true;
    }

    // after commit the mediatype treated as the correct one
    m_bMTFallbackUsed = false;
}

void OStorage_Impl::Revert()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if ( !( m_nStorageMode & embed::ElementModes::WRITE ) )
        return; // nothing to do

    // all the children must be removed
    // they will be created later on demand

    // rebuild the map - cannot do it in-place, because we're changing some of the key values
    std::unordered_map<OUString, std::vector<SotElement_Impl*>> oldMap;
    std::swap(oldMap, m_aChildrenMap);

    for (const auto & rPair : oldMap)
        for (auto pElement : rPair.second)
        {
            if ( pElement->m_bIsInserted )
                delete pElement;
            else
            {
                ClearElement( pElement );

                pElement->m_bIsRemoved = false;

                m_aChildrenMap[pElement->m_aOriginalName].push_back(pElement);
            }
        }

    // return replaced removed elements
    for ( auto& pDeleted : m_aDeletedVector )
    {
        m_aChildrenMap[pDeleted->m_aOriginalName].push_back(pDeleted);

        ClearElement( pDeleted );

        pDeleted->m_bIsRemoved = false;
    }
    m_aDeletedVector.clear();

    m_bControlMediaType = false;
    m_bControlVersion = false;

    GetStorageProperties();

    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        // currently the relations storage is changed only on commit
        m_xNewRelInfoStream.clear();
        m_aRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
        m_nRelInfoStatus = RELINFO_NO_INIT;
    }
}

::comphelper::SequenceAsHashMap OStorage_Impl::GetCommonRootEncryptionData()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() ) ;

    if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        throw packages::NoEncryptionException( THROW_WHERE );

    if ( m_bIsRoot )
    {
        if ( !m_bHasCommonEncryptionData )
            throw packages::NoEncryptionException( THROW_WHERE );

        return m_aCommonEncryptionData;
    }
    else
    {
        if ( !m_pParent )
            throw packages::NoEncryptionException( THROW_WHERE );

        return m_pParent->GetCommonRootEncryptionData();
    }
}

SotElement_Impl* OStorage_Impl::FindElement( const OUString& rName )
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    SAL_WARN_IF( rName.isEmpty(), "package.xstor", "Name is empty!" );

    ReadContents();

    auto mapIt = m_aChildrenMap.find(rName);
    if (mapIt == m_aChildrenMap.end())
        return nullptr;
    for (auto pElement : mapIt->second)
        if (!pElement->m_bIsRemoved)
            return pElement;

    return nullptr;
}

SotElement_Impl* OStorage_Impl::InsertStream( const OUString& aName, bool bEncr )
{
    SAL_WARN_IF( !m_xPackage.is(), "package.xstor", "Not possible to refer to package as to factory!" );
    if ( !m_xPackage.is() )
        throw embed::InvalidStorageException( THROW_WHERE);

    uno::Sequence< uno::Any > aSeq( 1 );
    aSeq[0] <<= false;
    uno::Reference< lang::XUnoTunnel > xNewElement( m_xPackage->createInstanceWithArguments( aSeq ),
                                                    uno::UNO_QUERY );

    SAL_WARN_IF( !xNewElement.is(), "package.xstor", "Not possible to create a new stream!" );
    if ( !xNewElement.is() )
        throw io::IOException( THROW_WHERE );

    uno::Reference< packages::XDataSinkEncrSupport > xPackageSubStream( xNewElement, uno::UNO_QUERY_THROW );

    OSL_ENSURE( m_nStorageType == embed::StorageFormats::PACKAGE || !bEncr, "Only package storage supports encryption!" );
    if ( m_nStorageType != embed::StorageFormats::PACKAGE && bEncr )
        throw packages::NoEncryptionException( THROW_WHERE );

    // the mode is not needed for storage stream internal implementation
    SotElement_Impl* pNewElement = InsertElement( aName, false );
    pNewElement->m_xStream.reset(new OWriteStream_Impl(this, xPackageSubStream, m_xPackage, m_xContext, bEncr, m_nStorageType, true));

    m_aChildrenMap[aName].push_back( pNewElement );
    m_bIsModified = true;
    m_bBroadcastModified = true;

    return pNewElement;
}

void OStorage_Impl::InsertRawStream( const OUString& aName, const uno::Reference< io::XInputStream >& xInStream )
{
    // insert of raw stream means insert and commit
    SAL_WARN_IF( !m_xPackage.is(), "package.xstor", "Not possible to refer to package as to factory!" );
    if ( !m_xPackage.is() )
        throw embed::InvalidStorageException( THROW_WHERE );

    if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        throw packages::NoEncryptionException( THROW_WHERE );

    uno::Reference< io::XSeekable > xSeek( xInStream, uno::UNO_QUERY );
    uno::Reference< io::XInputStream > xInStrToInsert = xSeek.is() ? xInStream :
                                                                     GetSeekableTempCopy( xInStream, m_xContext );

    uno::Sequence< uno::Any > aSeq( 1 );
    aSeq[0] <<= false;
    uno::Reference< lang::XUnoTunnel > xNewElement( m_xPackage->createInstanceWithArguments( aSeq ),
                                                    uno::UNO_QUERY );

    SAL_WARN_IF( !xNewElement.is(), "package.xstor", "Not possible to create a new stream!" );
    if ( !xNewElement.is() )
        throw io::IOException( THROW_WHERE );

    uno::Reference< packages::XDataSinkEncrSupport > xPackageSubStream( xNewElement, uno::UNO_QUERY_THROW );
    xPackageSubStream->setRawStream( xInStrToInsert );

    // the mode is not needed for storage stream internal implementation
    SotElement_Impl* pNewElement = InsertElement( aName, false );
    pNewElement->m_xStream.reset(new OWriteStream_Impl(this, xPackageSubStream, m_xPackage, m_xContext, true, m_nStorageType, false));
    // the stream is inserted and must be treated as a committed one
    pNewElement->m_xStream->SetToBeCommited();

    m_aChildrenMap[aName].push_back( pNewElement );
    m_bIsModified = true;
    m_bBroadcastModified = true;
}

std::unique_ptr<OStorage_Impl> OStorage_Impl::CreateNewStorageImpl( sal_Int32 nStorageMode )
{
    SAL_WARN_IF( !m_xPackage.is(), "package.xstor", "Not possible to refer to package as to factory!" );
    if ( !m_xPackage.is() )
        throw embed::InvalidStorageException( THROW_WHERE );

    uno::Sequence< uno::Any > aSeq( 1 );
    aSeq[0] <<= true;
    uno::Reference< lang::XUnoTunnel > xNewElement( m_xPackage->createInstanceWithArguments( aSeq ),
                                                    uno::UNO_QUERY );

    SAL_WARN_IF( !xNewElement.is(), "package.xstor", "Not possible to create a new storage!" );
    if ( !xNewElement.is() )
        throw io::IOException( THROW_WHERE );

    uno::Reference< container::XNameContainer > xPackageSubFolder( xNewElement, uno::UNO_QUERY_THROW );
    std::unique_ptr<OStorage_Impl> pResult(
            new OStorage_Impl( this, nStorageMode, xPackageSubFolder, m_xPackage, m_xContext, m_nStorageType ));
    pResult->m_bIsModified = true;

    return pResult;
}

SotElement_Impl* OStorage_Impl::InsertStorage( const OUString& aName, sal_Int32 nStorageMode )
{
    SotElement_Impl* pNewElement = InsertElement( aName, true );

    pNewElement->m_xStorage = CreateNewStorageImpl(nStorageMode);

    m_aChildrenMap[aName].push_back( pNewElement );

    return pNewElement;
}

SotElement_Impl* OStorage_Impl::InsertElement( const OUString& aName, bool bIsStorage )
{
    assert( FindElement(aName) == nullptr && "Should not try to insert existing element");

    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    SotElement_Impl* pDeletedElm = nullptr;

    auto it = m_aChildrenMap.find(aName);
    if (it != m_aChildrenMap.end())
        for (auto pElement : it->second)
        {
            SAL_WARN_IF( !pElement->m_bIsRemoved, "package.xstor", "Try to insert an element instead of existing one!" );
            if ( pElement->m_bIsRemoved )
            {
                SAL_WARN_IF( pElement->m_bIsInserted, "package.xstor", "Inserted elements must be deleted immediately!" );
                pDeletedElm = pElement;
            }
        }

    if ( pDeletedElm )
    {
        if ( pDeletedElm->m_bIsStorage )
            OpenSubStorage( pDeletedElm, embed::ElementModes::READWRITE );
        else
            OpenSubStream( pDeletedElm );

        auto & rVec = m_aChildrenMap[aName];
        rVec.erase(std::remove(rVec.begin(), rVec.end(), pDeletedElm), rVec.end());
        if (rVec.empty())
            m_aChildrenMap.erase(aName);
        m_aDeletedVector.push_back( pDeletedElm );
    }

    // create new element
    return new SotElement_Impl( aName, bIsStorage, true );
}

void OStorage_Impl::OpenSubStorage( SotElement_Impl* pElement, sal_Int32 nStorageMode )
{
    SAL_WARN_IF( !pElement, "package.xstor", "pElement is not set!" );
    SAL_WARN_IF( !pElement->m_bIsStorage, "package.xstor", "Storage flag is not set!" );

    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if (!pElement->m_xStorage)
    {
        SAL_WARN_IF( pElement->m_bIsInserted, "package.xstor", "Inserted element must be created already!" );

        uno::Reference< lang::XUnoTunnel > xTunnel;
        m_xPackageFolder->getByName( pElement->m_aOriginalName ) >>= xTunnel;
        if ( !xTunnel.is() )
            throw container::NoSuchElementException( THROW_WHERE );

        uno::Reference< container::XNameContainer > xPackageSubFolder( xTunnel, uno::UNO_QUERY_THROW );
        pElement->m_xStorage.reset(new OStorage_Impl(this, nStorageMode, xPackageSubFolder, m_xPackage, m_xContext, m_nStorageType));
    }
}

void OStorage_Impl::OpenSubStream( SotElement_Impl* pElement )
{
    SAL_WARN_IF( !pElement, "package.xstor", "pElement is not set!" );
    SAL_WARN_IF( pElement->m_bIsStorage, "package.xstor", "Storage flag is set!" );

    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    if (pElement->m_xStream)
        return;

    SAL_WARN_IF( pElement->m_bIsInserted, "package.xstor", "Inserted element must be created already!" );

    uno::Reference< lang::XUnoTunnel > xTunnel;
    m_xPackageFolder->getByName( pElement->m_aOriginalName ) >>= xTunnel;
    if ( !xTunnel.is() )
        throw container::NoSuchElementException( THROW_WHERE );

    uno::Reference< packages::XDataSinkEncrSupport > xPackageSubStream( xTunnel, uno::UNO_QUERY_THROW );

    // the stream can never be inserted here, because inserted stream element holds the stream till commit or destruction
    pElement->m_xStream.reset(new OWriteStream_Impl(this, xPackageSubStream, m_xPackage, m_xContext, false, m_nStorageType, false, GetRelInfoStreamForName(pElement->m_aOriginalName)));
}

uno::Sequence< OUString > OStorage_Impl::GetElementNames()
{
    ::osl::MutexGuard aGuard( m_xMutex->GetMutex() );

    ReadContents();

    std::vector< OUString > aElementNames;
    aElementNames.reserve( m_aChildrenMap.size() );

    for ( const auto& pair : m_aChildrenMap )
        for (auto pElement : pair.second)
        {
            if ( !pElement->m_bIsRemoved )
                aElementNames.push_back(pair.first);
        }

    return comphelper::containerToSequence(aElementNames);
}

void OStorage_Impl::RemoveElement( OUString const & rName, SotElement_Impl* pElement )
{
    assert(pElement);

    if ( (pElement->m_xStorage && ( pElement->m_xStorage->m_pAntiImpl || !pElement->m_xStorage->m_aReadOnlyWrapVector.empty() ))
      || (pElement->m_xStream && ( pElement->m_xStream->m_pAntiImpl || !pElement->m_xStream->m_aInputStreamsVector.empty() )) )
        throw io::IOException( THROW_WHERE ); // TODO: Access denied

    auto mapIt = m_aChildrenMap.find(rName);
    for (auto it = mapIt->second.begin(); it != mapIt->second.end(); ++it)
        if (pElement == *it)
        {
            if ( pElement->m_bIsInserted )
            {
                delete pElement;
                mapIt->second.erase(std::remove(mapIt->second.begin(), mapIt->second.end(), pElement), mapIt->second.end());
                if (mapIt->second.empty())
                    m_aChildrenMap.erase(mapIt);
            }
            else
            {
                pElement->m_bIsRemoved = true;
                ClearElement( pElement );
            }
            return;
        }
    assert(false && "not found");

    // TODO/OFOPXML: the rel stream should be removed as well
}

void OStorage_Impl::ClearElement( SotElement_Impl* pElement )
{
    pElement->m_xStorage.reset();
    pElement->m_xStream.reset();
}

void OStorage_Impl::CloneStreamElement( const OUString& aStreamName,
                                        bool bEncryptionDataProvided,
                                        const ::comphelper::SequenceAsHashMap& aEncryptionData,
                                        uno::Reference< io::XStream >& xTargetStream )
{
    SotElement_Impl *pElement = FindElement( aStreamName );
    if ( !pElement )
    {
        // element does not exist, throw exception
        throw io::IOException( THROW_WHERE ); // TODO: access_denied
    }
    else if ( pElement->m_bIsStorage )
        throw io::IOException( THROW_WHERE );

    if (!pElement->m_xStream)
        OpenSubStream( pElement );

    if (!pElement->m_xStream || !pElement->m_xStream->m_xPackageStream.is())
        throw io::IOException( THROW_WHERE ); // TODO: general_error

    // the existence of m_pAntiImpl of the child is not interesting,
    // the copy will be created internally

    // usual copying is not applicable here, only last flushed version of the
    // child stream should be used for copying. Probably the children m_xPackageStream
    // can be used as a base of a new stream, that would be copied to result
    // storage. The only problem is that some package streams can be accessed from outside
    // at the same time (now solved by wrappers that remember own position).

    if (bEncryptionDataProvided)
        pElement->m_xStream->GetCopyOfLastCommit(xTargetStream, aEncryptionData);
    else
        pElement->m_xStream->GetCopyOfLastCommit(xTargetStream);
}

void OStorage_Impl::RemoveStreamRelInfo( const OUString& aOriginalName )
{
    // this method should be used only in OStorage_Impl::Commit() method
    // the aOriginalName can be empty, in this case the storage relation info should be removed

    if ( m_nStorageType == embed::StorageFormats::OFOPXML && m_xRelStorage.is() )
    {
        OUString aRelStreamName = aOriginalName + ".rels";

        if ( m_xRelStorage->hasByName( aRelStreamName ) )
            m_xRelStorage->removeElement( aRelStreamName );
    }
}

void OStorage_Impl::CreateRelStorage()
{
    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        return;

    if ( m_xRelStorage.is() )
        return;

    if ( !m_pRelStorElement )
    {
        m_pRelStorElement = new SotElement_Impl( "_rels", true, true );
        m_pRelStorElement->m_xStorage = CreateNewStorageImpl(embed::ElementModes::WRITE);
        if (m_pRelStorElement->m_xStorage)
            m_pRelStorElement->m_xStorage->m_pParent = nullptr; // the relation storage is completely controlled by parent
    }

    if (!m_pRelStorElement->m_xStorage)
        OpenSubStorage( m_pRelStorElement, embed::ElementModes::WRITE );

    if (!m_pRelStorElement->m_xStorage)
        throw uno::RuntimeException( THROW_WHERE );

    OStorage* pResultStorage = new OStorage(m_pRelStorElement->m_xStorage.get(), false);
    m_xRelStorage.set( static_cast<embed::XStorage*>(pResultStorage) );
}

void OStorage_Impl::CommitStreamRelInfo( const OUString &rName, SotElement_Impl const * pStreamElement )
{
    // this method should be used only in OStorage_Impl::Commit() method

    // the stream element must be provided
    if ( !pStreamElement )
        throw uno::RuntimeException( THROW_WHERE );

    if (m_nStorageType == embed::StorageFormats::OFOPXML && pStreamElement->m_xStream)
    {
        SAL_WARN_IF( rName.isEmpty(), "package.xstor", "The name must not be empty!" );

        if ( !m_xRelStorage.is() )
        {
            // Create new rels storage, this is commit scenario so it must be possible
            CreateRelStorage();
        }

        pStreamElement->m_xStream->CommitStreamRelInfo(m_xRelStorage, pStreamElement->m_aOriginalName, rName);
    }
}

uno::Reference< io::XInputStream > OStorage_Impl::GetRelInfoStreamForName( const OUString& aName )
{
    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        ReadContents();
        if ( m_xRelStorage.is() )
        {
            OUString aRelStreamName = aName + ".rels";
            if ( m_xRelStorage->hasByName( aRelStreamName ) )
            {
                uno::Reference< io::XStream > xStream = m_xRelStorage->openStreamElement( aRelStreamName, embed::ElementModes::READ );
                if ( xStream.is() )
                    return xStream->getInputStream();
            }
        }
    }

    return uno::Reference< io::XInputStream >();
}

void OStorage_Impl::CommitRelInfo( const uno::Reference< container::XNameContainer >& xNewPackageFolder )
{
    // this method should be used only in OStorage_Impl::Commit() method
    OUString aRelsStorName("_rels");

    if ( !xNewPackageFolder.is() )
        throw uno::RuntimeException( THROW_WHERE );

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        return;

    if ( m_nRelInfoStatus == RELINFO_BROKEN || m_nRelInfoStatus == RELINFO_CHANGED_BROKEN )
        throw io::IOException( THROW_WHERE );

    if (m_nRelInfoStatus == RELINFO_CHANGED)
    {
        if (m_aRelInfo.hasElements())
        {
            CreateRelStorage();

            uno::Reference<io::XStream> xRelsStream = m_xRelStorage->openStreamElement(
                ".rels", embed::ElementModes::TRUNCATE | embed::ElementModes::READWRITE);

            uno::Reference<io::XOutputStream> xOutStream = xRelsStream->getOutputStream();
            if (!xOutStream.is())
                throw uno::RuntimeException(THROW_WHERE);

            ::comphelper::OFOPXMLHelper::WriteRelationsInfoSequence(xOutStream, m_aRelInfo,
                                                                    m_xContext);

            // set the mediatype
            uno::Reference<beans::XPropertySet> xPropSet(xRelsStream, uno::UNO_QUERY_THROW);
            xPropSet->setPropertyValue(
                "MediaType", uno::makeAny(OUString(
                                 "application/vnd.openxmlformats-package.relationships+xml")));

            m_nRelInfoStatus = RELINFO_READ;
        }
        else if (m_xRelStorage.is())
            RemoveStreamRelInfo(OUString()); // remove own rel info
    }
    else if (m_nRelInfoStatus == RELINFO_CHANGED_STREAM_READ
             || m_nRelInfoStatus == RELINFO_CHANGED_STREAM)
    {
        CreateRelStorage();

        uno::Reference<io::XStream> xRelsStream = m_xRelStorage->openStreamElement(
            ".rels", embed::ElementModes::TRUNCATE | embed::ElementModes::READWRITE);

        uno::Reference<io::XOutputStream> xOutputStream = xRelsStream->getOutputStream();
        if (!xOutputStream.is())
            throw uno::RuntimeException(THROW_WHERE);

        uno::Reference<io::XSeekable> xSeek(m_xNewRelInfoStream, uno::UNO_QUERY_THROW);
        xSeek->seek(0);
        ::comphelper::OStorageHelper::CopyInputToOutput(m_xNewRelInfoStream, xOutputStream);

        // set the mediatype
        uno::Reference<beans::XPropertySet> xPropSet(xRelsStream, uno::UNO_QUERY_THROW);
        xPropSet->setPropertyValue(
            "MediaType",
            uno::makeAny(OUString("application/vnd.openxmlformats-package.relationships+xml")));

        m_xNewRelInfoStream.clear();
        if (m_nRelInfoStatus == RELINFO_CHANGED_STREAM)
        {
            m_aRelInfo = uno::Sequence<uno::Sequence<beans::StringPair>>();
            m_nRelInfoStatus = RELINFO_NO_INIT;
        }
        else
            m_nRelInfoStatus = RELINFO_READ;
    }

    if ( !m_xRelStorage.is() )
        return;

    if ( m_xRelStorage->hasElements() )
    {
        uno::Reference< embed::XTransactedObject > xTrans( m_xRelStorage, uno::UNO_QUERY_THROW );
        xTrans->commit();
    }

    if ( xNewPackageFolder.is() && xNewPackageFolder->hasByName( aRelsStorName ) )
        xNewPackageFolder->removeByName( aRelsStorName );

    if ( !m_xRelStorage->hasElements() )
    {
        // the empty relations storage should not be created
        delete m_pRelStorElement;
        m_pRelStorElement = nullptr;
        m_xRelStorage.clear();
    }
    else if ( m_pRelStorElement && m_pRelStorElement->m_xStorage && xNewPackageFolder.is() )
        m_pRelStorElement->m_xStorage->InsertIntoPackageFolder( aRelsStorName, xNewPackageFolder );
}

// OStorage implementation

OStorage::OStorage( uno::Reference< io::XInputStream > const & xInputStream,
                    sal_Int32 nMode,
                    const uno::Sequence< beans::PropertyValue >& xProperties,
                    uno::Reference< uno::XComponentContext > const & xContext,
                    sal_Int32 nStorageType )
: m_pImpl( new OStorage_Impl( xInputStream, nMode, xProperties, xContext, nStorageType ) )
{
    m_pImpl->m_pAntiImpl = this;
    m_pData.reset(new StorInternalData_Impl( m_pImpl->m_xMutex, m_pImpl->m_bIsRoot, m_pImpl->m_nStorageType, false));
}

OStorage::OStorage( uno::Reference< io::XStream > const & xStream,
                    sal_Int32 nMode,
                    const uno::Sequence< beans::PropertyValue >& xProperties,
                    uno::Reference< uno::XComponentContext > const & xContext,
                    sal_Int32 nStorageType )
: m_pImpl( new OStorage_Impl( xStream, nMode, xProperties, xContext, nStorageType ) )
{
    m_pImpl->m_pAntiImpl = this;
    m_pData.reset(new StorInternalData_Impl( m_pImpl->m_xMutex, m_pImpl->m_bIsRoot, m_pImpl->m_nStorageType, false));
}

OStorage::OStorage( OStorage_Impl* pImpl, bool bReadOnlyWrap )
: m_pImpl( pImpl )
{
    // this call can be done only from OStorage_Impl implementation to create child storage
    OSL_ENSURE( m_pImpl && m_pImpl->m_xMutex.is(), "The provided pointer & mutex MUST NOT be empty!" );

    m_pData.reset(new StorInternalData_Impl( m_pImpl->m_xMutex, m_pImpl->m_bIsRoot, m_pImpl->m_nStorageType, bReadOnlyWrap));

    OSL_ENSURE( ( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) == embed::ElementModes::WRITE ||
                    m_pData->m_bReadOnlyWrap,
                "The wrapper can not allow writing in case implementation does not!" );

    if ( !bReadOnlyWrap )
        m_pImpl->m_pAntiImpl = this;
}

OStorage::~OStorage()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );
    if ( m_pImpl )
    {
        osl_atomic_increment(&m_refCount); // to call dispose
        try {
            dispose();
        }
        catch( const uno::RuntimeException& )
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Handled exception");
        }
    }
}

void OStorage::InternalDispose( bool bNotifyImpl )
{
    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    // the source object is also a kind of locker for the current object
    // since the listeners could dispose the object while being notified
    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
    m_pData->m_aListenersContainer.disposeAndClear( aSource );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    m_pImpl->m_nModifiedListenerCount = 0;

    if ( m_pData->m_bReadOnlyWrap )
    {
        OSL_ENSURE( m_pData->m_aOpenSubComponentsVector.empty() || m_pData->m_pSubElDispListener,
                    "If any subelements are open the listener must exist!" );

        if (m_pData->m_pSubElDispListener)
        {
            m_pData->m_pSubElDispListener->OwnerIsDisposed();

            // iterate through m_pData->m_aOpenSubComponentsVector
            // deregister m_pData->m_pSubElDispListener and dispose all of them
            if ( !m_pData->m_aOpenSubComponentsVector.empty() )
            {
                for ( const auto& pComp : m_pData->m_aOpenSubComponentsVector )
                {
                    uno::Reference< lang::XComponent > xTmp = pComp;
                    if ( xTmp.is() )
                    {
                        xTmp->removeEventListener( uno::Reference< lang::XEventListener >(
                            static_cast< lang::XEventListener* >( m_pData->m_pSubElDispListener.get())));

                        try {
                            xTmp->dispose();
                        } catch( const uno::Exception& )
                        {
                            TOOLS_INFO_EXCEPTION("package.xstor", "Quiet exception");
                        }
                    }
                }

                m_pData->m_aOpenSubComponentsVector.clear();
            }
        }

        if ( bNotifyImpl )
            m_pImpl->RemoveReadOnlyWrap( *this );
    }
    else
    {
        m_pImpl->m_pAntiImpl = nullptr;

        if ( bNotifyImpl )
        {
            if ( m_pData->m_bIsRoot )
                delete m_pImpl;
            else
            {
                // the non-committed changes for the storage must be removed
                m_pImpl->Revert();
            }
        }
    }

    m_pImpl = nullptr;
}

void OStorage::ChildIsDisposed( const uno::Reference< uno::XInterface >& xChild )
{
    // this method can only be called by child disposing listener

    // this method must not contain any locking
    // the locking is done in the listener

    auto& rVec = m_pData->m_aOpenSubComponentsVector;
    rVec.erase(std::remove_if(rVec.begin(), rVec.end(),
        [&xChild](const uno::Reference<lang::XComponent>& xTmp) {
            return !xTmp.is() || xTmp == xChild;
        }),
        rVec.end());
}

void OStorage::BroadcastModifiedIfNecessary()
{
    // no need to lock mutex here for the checking of m_pImpl, and m_pData is alive until the object is destructed
    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( !m_pImpl->m_bBroadcastModified )
        return;

    m_pImpl->m_bBroadcastModified = false;

    SAL_WARN_IF( m_pData->m_bReadOnlyWrap, "package.xstor", "The storage can not be modified at all!" );

    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );

    ::cppu::OInterfaceContainerHelper* pContainer =
            m_pData->m_aListenersContainer.getContainer(
                cppu::UnoType<util::XModifyListener>::get());
    if ( pContainer )
    {
           ::cppu::OInterfaceIteratorHelper pIterator( *pContainer );
           while ( pIterator.hasMoreElements( ) )
           {
               static_cast<util::XModifyListener*>( pIterator.next( ) )->modified( aSource );
           }
    }
}

void OStorage::BroadcastTransaction( sal_Int8 nMessage )
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
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    SAL_WARN_IF( m_pData->m_bReadOnlyWrap, "package.xstor", "The storage can not be modified at all!" );

    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );

    ::cppu::OInterfaceContainerHelper* pContainer =
            m_pData->m_aListenersContainer.getContainer(
                cppu::UnoType<embed::XTransactionListener>::get());
    if ( !pContainer )
           return;

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
                   static_cast<embed::XTransactionListener*>( pIterator.next( ) )->reverted( aSource );
                break;
        }
    }
}

SotElement_Impl* OStorage::OpenStreamElement_Impl( const OUString& aStreamName, sal_Int32 nOpenMode, bool bEncr )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    OSL_ENSURE( !m_pData->m_bReadOnlyWrap || ( nOpenMode & embed::ElementModes::WRITE ) != embed::ElementModes::WRITE,
                "An element can not be opened for writing in readonly storage!" );

    SotElement_Impl *pElement = m_pImpl->FindElement( aStreamName );
    if ( !pElement )
    {
        // element does not exist, check if creation is allowed
        if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE )
          || (( nOpenMode & embed::ElementModes::WRITE ) != embed::ElementModes::WRITE )
          || ( nOpenMode & embed::ElementModes::NOCREATE ) == embed::ElementModes::NOCREATE )
        {
            throw io::IOException("Element does not exist and cannot be "
                    "created: \"" + aStreamName + "\""); // TODO: access_denied
        }

        // create a new StreamElement and insert it into the list
        pElement = m_pImpl->InsertStream( aStreamName, bEncr );
    }
    else if ( pElement->m_bIsStorage )
    {
        throw io::IOException( THROW_WHERE );
    }

    SAL_WARN_IF( !pElement, "package.xstor", "In case element can not be created an exception must be thrown!" );

    if (!pElement->m_xStream)
        m_pImpl->OpenSubStream( pElement );

    if (!pElement->m_xStream)
        throw io::IOException( THROW_WHERE );

    return pElement;
}

void OStorage::MakeLinkToSubComponent_Impl( const uno::Reference< lang::XComponent >& xComponent )
{
    if ( !xComponent.is() )
        throw uno::RuntimeException( THROW_WHERE );

    if (!m_pData->m_pSubElDispListener)
    {
        m_pData->m_pSubElDispListener = new OChildDispListener_Impl( *this );
    }

    xComponent->addEventListener( uno::Reference< lang::XEventListener >(
        static_cast< ::cppu::OWeakObject* >(m_pData->m_pSubElDispListener.get()), uno::UNO_QUERY));

    m_pData->m_aOpenSubComponentsVector.emplace_back(xComponent );
}

//  XInterface

uno::Any SAL_CALL OStorage::queryInterface( const uno::Type& rType )
{
    // common interfaces
    uno::Any aReturn = ::cppu::queryInterface
                (   rType
                ,   static_cast<lang::XTypeProvider*> ( this )
                ,   static_cast<embed::XStorage*> ( this )
                ,   static_cast<embed::XStorage2*> ( this )
                ,   static_cast<embed::XTransactedObject*> ( this )
                ,   static_cast<embed::XTransactionBroadcaster*> ( this )
                ,   static_cast<util::XModifiable*> ( this )
                ,   static_cast<container::XNameAccess*> ( this )
                ,   static_cast<container::XElementAccess*> ( this )
                ,   static_cast<lang::XComponent*> ( this )
                ,   static_cast<beans::XPropertySet*> ( this )
                ,   static_cast<embed::XOptimizedStorage*> ( this ) );

    if ( aReturn.hasValue() )
        return aReturn ;

    aReturn = ::cppu::queryInterface
                (   rType
                ,   static_cast<embed::XHierarchicalStorageAccess*> ( this )
                ,   static_cast<embed::XHierarchicalStorageAccess2*> ( this ) );

    if ( aReturn.hasValue() )
        return aReturn ;

    if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        if ( m_pData->m_bIsRoot )
        {
            aReturn = ::cppu::queryInterface
                        (   rType
                        ,   static_cast<embed::XStorageRawAccess*> ( this )
                        ,   static_cast<embed::XEncryptionProtectedSource*> ( this )
                        ,   static_cast<embed::XEncryptionProtectedSource2*> ( this )
                        ,   static_cast<embed::XEncryptionProtectedStorage*> ( this ) );
        }
        else
        {
            aReturn = ::cppu::queryInterface
                        (   rType
                        ,   static_cast<embed::XStorageRawAccess*> ( this ) );
        }
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        aReturn = ::cppu::queryInterface
                    (   rType
                    ,   static_cast<embed::XRelationshipAccess*> ( this ) );
    }

    if ( aReturn.hasValue() )
        return aReturn ;

    return OWeakObject::queryInterface( rType );
}

void SAL_CALL OStorage::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL OStorage::release() throw()
{
    OWeakObject::release();
}

//  XTypeProvider
uno::Sequence< uno::Type > SAL_CALL OStorage::getTypes()
{
    if (! m_pData->m_pTypeCollection)
    {
        ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

        if (! m_pData->m_pTypeCollection)
        {
            if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
            {
                if ( m_pData->m_bIsRoot )
                {
                    m_pData->m_pTypeCollection.reset(new ::cppu::OTypeCollection
                                    (   cppu::UnoType<lang::XTypeProvider>::get()
                                    ,   cppu::UnoType<embed::XStorage>::get()
                                    ,   cppu::UnoType<embed::XStorage2>::get()
                                    ,   cppu::UnoType<embed::XStorageRawAccess>::get()
                                    ,   cppu::UnoType<embed::XTransactedObject>::get()
                                    ,   cppu::UnoType<embed::XTransactionBroadcaster>::get()
                                    ,   cppu::UnoType<util::XModifiable>::get()
                                    ,   cppu::UnoType<embed::XEncryptionProtectedStorage>::get()
                                    ,   cppu::UnoType<embed::XEncryptionProtectedSource2>::get()
                                    ,   cppu::UnoType<embed::XEncryptionProtectedSource>::get()
                                    ,   cppu::UnoType<beans::XPropertySet>::get()));
                }
                else
                {
                    m_pData->m_pTypeCollection.reset(new ::cppu::OTypeCollection
                                    (   cppu::UnoType<lang::XTypeProvider>::get()
                                    ,   cppu::UnoType<embed::XStorage>::get()
                                    ,   cppu::UnoType<embed::XStorage2>::get()
                                    ,   cppu::UnoType<embed::XStorageRawAccess>::get()
                                    ,   cppu::UnoType<embed::XTransactedObject>::get()
                                    ,   cppu::UnoType<embed::XTransactionBroadcaster>::get()
                                    ,   cppu::UnoType<util::XModifiable>::get()
                                    ,   cppu::UnoType<beans::XPropertySet>::get()));
                }
            }
            else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
            {
                m_pData->m_pTypeCollection.reset(new ::cppu::OTypeCollection
                                (   cppu::UnoType<lang::XTypeProvider>::get()
                                ,   cppu::UnoType<embed::XStorage>::get()
                                ,   cppu::UnoType<embed::XTransactedObject>::get()
                                ,   cppu::UnoType<embed::XTransactionBroadcaster>::get()
                                ,   cppu::UnoType<util::XModifiable>::get()
                                ,   cppu::UnoType<embed::XRelationshipAccess>::get()
                                ,   cppu::UnoType<beans::XPropertySet>::get()));
            }
            else
            {
                m_pData->m_pTypeCollection.reset(new ::cppu::OTypeCollection
                                (   cppu::UnoType<lang::XTypeProvider>::get()
                                ,   cppu::UnoType<embed::XStorage>::get()
                                ,   cppu::UnoType<embed::XTransactedObject>::get()
                                ,   cppu::UnoType<embed::XTransactionBroadcaster>::get()
                                ,   cppu::UnoType<util::XModifiable>::get()
                                ,   cppu::UnoType<beans::XPropertySet>::get()));
            }
        }
    }

    return m_pData->m_pTypeCollection->getTypes() ;
}

namespace { struct lcl_ImplId : public rtl::Static< ::cppu::OImplementationId, lcl_ImplId > {}; }

uno::Sequence< sal_Int8 > SAL_CALL OStorage::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

//  XStorage
void SAL_CALL OStorage::copyToStorage( const uno::Reference< embed::XStorage >& xDest )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( !xDest.is() || xDest == uno::Reference< uno::XInterface >( static_cast< OWeakObject*> ( this ), uno::UNO_QUERY ) )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );

    try {
        m_pImpl->CopyToStorage( xDest, false );
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const io::IOException& )
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

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't copy storage!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

uno::Reference< io::XStream > SAL_CALL OStorage::openStreamElement(
    const OUString& aStreamName, sal_Int32 nOpenMode )
{
    osl::ClearableMutexGuard aGuard(m_pData->m_xSharedMutex->GetMutex());

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aStreamName == "_rels" )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 1 ); // unacceptable element name

    if ( ( nOpenMode & embed::ElementModes::WRITE ) && m_pData->m_bReadOnlyWrap )
        throw io::IOException( THROW_WHERE ); // TODO: access denied

    uno::Reference< io::XStream > xResult;
    try
    {
        SotElement_Impl *pElement = OpenStreamElement_Impl( aStreamName, nOpenMode, false );
        OSL_ENSURE(pElement && pElement->m_xStream, "In case element can not be created an exception must be thrown!");

        xResult = pElement->m_xStream->GetStream(nOpenMode, false);
        SAL_WARN_IF( !xResult.is(), "package.xstor", "The method must throw exception instead of removing empty result!" );

        if ( m_pData->m_bReadOnlyWrap )
        {
            // before the storage disposes the stream it must deregister itself as listener
            uno::Reference< lang::XComponent > xStreamComponent( xResult, uno::UNO_QUERY_THROW );
            MakeLinkToSubComponent_Impl( xStreamComponent );
        }
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const packages::WrongPasswordException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const io::IOException& )
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

        throw embed::StorageWrappedTargetException(THROW_WHERE "Can't open stream element!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    aGuard.clear();

    BroadcastModifiedIfNecessary();

    return xResult;
}

uno::Reference< io::XStream > SAL_CALL OStorage::openEncryptedStreamElement(
    const OUString& aStreamName, sal_Int32 nOpenMode, const OUString& aPass )
{
    return openEncryptedStream( aStreamName, nOpenMode, ::comphelper::OStorageHelper::CreatePackageEncryptionData( aPass ) );
}

uno::Reference< embed::XStorage > SAL_CALL OStorage::openStorageElement(
            const OUString& aStorName, sal_Int32 nStorageMode )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aStorName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStorName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aStorName == "_rels" )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 1 ); // unacceptable storage name

    if ( ( nStorageMode & embed::ElementModes::WRITE ) && m_pData->m_bReadOnlyWrap )
        throw io::IOException( THROW_WHERE ); // TODO: access denied

    if ( ( nStorageMode & embed::ElementModes::TRUNCATE )
      && !( nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException( THROW_WHERE ); // TODO: access denied

    // it's always possible to read written storage in this implementation
    nStorageMode |= embed::ElementModes::READ;

    uno::Reference< embed::XStorage > xResult;
    try
    {
        SotElement_Impl *pElement = m_pImpl->FindElement( aStorName );
        if ( !pElement )
        {
            // element does not exist, check if creation is allowed
            if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE )
              || (( nStorageMode & embed::ElementModes::WRITE ) != embed::ElementModes::WRITE )
              || ( nStorageMode & embed::ElementModes::NOCREATE ) == embed::ElementModes::NOCREATE )
                throw io::IOException( THROW_WHERE ); // TODO: access_denied

            // create a new StorageElement and insert it into the list
            pElement = m_pImpl->InsertStorage( aStorName, nStorageMode );
        }
        else if ( !pElement->m_bIsStorage )
        {
            throw io::IOException( THROW_WHERE );
        }
        else if (pElement->m_xStorage)
        {
            // storage has already been opened; it may be opened another time, if it the mode allows to do so
            if (pElement->m_xStorage->m_pAntiImpl)
            {
                throw io::IOException( THROW_WHERE ); // TODO: access_denied
            }
            else if ( !pElement->m_xStorage->m_aReadOnlyWrapVector.empty()
                    && ( nStorageMode & embed::ElementModes::WRITE ) )
            {
                throw io::IOException( THROW_WHERE ); // TODO: access_denied
            }
            else
            {
                // in case parent storage allows writing the readonly mode of the child storage is
                // virtual, that means that it is just enough to change the flag to let it be writable
                // and since there is no AntiImpl nobody should be notified about it
                pElement->m_xStorage->m_nStorageMode = nStorageMode | embed::ElementModes::READ;

                if ( nStorageMode & embed::ElementModes::TRUNCATE )
                {
                    for (const auto & rPair : pElement->m_xStorage->m_aChildrenMap)
                        for (auto pElementToDel : rPair.second)
                            m_pImpl->RemoveElement( /*aName*/rPair.first, pElementToDel );
                }
            }
        }

        if (!pElement->m_xStorage)
            m_pImpl->OpenSubStorage(pElement, nStorageMode);

        if (!pElement->m_xStorage)
            throw io::IOException( THROW_WHERE ); // TODO: general_error

        bool bReadOnlyWrap = ( ( nStorageMode & embed::ElementModes::WRITE ) != embed::ElementModes::WRITE );
        OStorage* pResultStorage = new OStorage(pElement->m_xStorage.get(), bReadOnlyWrap);
        xResult.set( static_cast<embed::XStorage*>(pResultStorage) );

        if ( bReadOnlyWrap )
        {
            // Before this call is done the object must be refcounted already
            pElement->m_xStorage->SetReadOnlyWrap(*pResultStorage);

            // before the storage disposes the stream it must deregister itself as listener
            uno::Reference< lang::XComponent > xStorageComponent( xResult, uno::UNO_QUERY_THROW );
            MakeLinkToSubComponent_Impl( xStorageComponent );
        }
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const io::IOException& )
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

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't open storage!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    return xResult;
}

uno::Reference< io::XStream > SAL_CALL OStorage::cloneStreamElement( const OUString& aStreamName )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aStreamName == "_rels" )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 1 ); // unacceptable storage name

    try
    {
        uno::Reference< io::XStream > xResult;
        m_pImpl->CloneStreamElement( aStreamName, false, ::comphelper::SequenceAsHashMap(), xResult );
        if ( !xResult.is() )
            throw uno::RuntimeException( THROW_WHERE );
        return xResult;
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const packages::WrongPasswordException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
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

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't clone stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

uno::Reference< io::XStream > SAL_CALL OStorage::cloneEncryptedStreamElement(
    const OUString& aStreamName,
    const OUString& aPass )
{
    return cloneEncryptedStream( aStreamName, ::comphelper::OStorageHelper::CreatePackageEncryptionData( aPass ) );
}

void SAL_CALL OStorage::copyLastCommitTo(
            const uno::Reference< embed::XStorage >& xTargetStorage )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    try
    {
        m_pImpl->CopyLastCommitTo( xTargetStorage );
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const io::IOException& )
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

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't copy last commit version!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

}

void SAL_CALL OStorage::copyStorageElementLastCommitTo(
            const OUString& aStorName,
            const uno::Reference< embed::XStorage >& xTargetStorage )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aStorName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStorName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aStorName == "_rels" )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 1 ); // unacceptable storage name

    try
    {
        SotElement_Impl *pElement = m_pImpl->FindElement( aStorName );
        if ( !pElement )
        {
            // element does not exist, throw exception
            throw io::IOException( THROW_WHERE ); // TODO: access_denied
        }
        else if ( !pElement->m_bIsStorage )
        {
            throw io::IOException( THROW_WHERE );
        }

        if (!pElement->m_xStorage)
            m_pImpl->OpenSubStorage( pElement, embed::ElementModes::READ );

        if (!pElement->m_xStorage)
            throw io::IOException( THROW_WHERE ); // TODO: general_error

        // the existence of m_pAntiImpl of the child is not interesting,
        // the copy will be created internally

        pElement->m_xStorage->CopyLastCommitTo(xTargetStorage);
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
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

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't copy last commit element version!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

sal_Bool SAL_CALL OStorage::isStreamElement( const OUString& aElementName )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aElementName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aElementName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aElementName == "_rels" )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 1 ); // unacceptable name

    SotElement_Impl* pElement = nullptr;

    try
    {
        pElement = m_pImpl->FindElement( aElementName );
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const container::NoSuchElementException& )
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

        throw lang::WrappedTargetRuntimeException( THROW_WHERE "Can't detect whether it is a stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    if ( !pElement )
        throw container::NoSuchElementException( THROW_WHERE ); //???

    return !pElement->m_bIsStorage;
}

sal_Bool SAL_CALL OStorage::isStorageElement( const OUString& aElementName )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aElementName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aElementName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aElementName == "_rels" )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );

    SotElement_Impl* pElement = nullptr;

    try
    {
        pElement = m_pImpl->FindElement( aElementName );
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const container::NoSuchElementException& )
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

        throw lang::WrappedTargetRuntimeException( THROW_WHERE "can't detect whether it is a storage",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    if ( !pElement )
        throw container::NoSuchElementException( THROW_WHERE ); //???

    return pElement->m_bIsStorage;
}

void SAL_CALL OStorage::removeElement( const OUString& aElementName )
{
    {
        osl::MutexGuard aGuard(m_pData->m_xSharedMutex->GetMutex());

        if (!m_pImpl)
        {
            SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
            throw lang::DisposedException(THROW_WHERE);
        }

        if (aElementName.isEmpty()
            || !::comphelper::OStorageHelper::IsValidZipEntryFileName(aElementName, false))
            throw lang::IllegalArgumentException(THROW_WHERE "Unexpected entry name syntax.",
                                                 uno::Reference<uno::XInterface>(), 1);

        if (m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aElementName == "_rels")
            throw lang::IllegalArgumentException(THROW_WHERE, uno::Reference<uno::XInterface>(),
                                                 1); // TODO: unacceptable name

        if (!(m_pImpl->m_nStorageMode & embed::ElementModes::WRITE))
            throw io::IOException(THROW_WHERE); // TODO: access denied

        try
        {
            auto pElement = m_pImpl->FindElement(aElementName);
            if ( !pElement )
                throw container::NoSuchElementException(THROW_WHERE); //???

            m_pImpl->RemoveElement(aElementName, pElement);

            m_pImpl->m_bIsModified = true;
            m_pImpl->m_bBroadcastModified = true;
        }
        catch (const embed::InvalidStorageException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
        }
        catch (const lang::IllegalArgumentException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
        }
        catch (const container::NoSuchElementException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
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

            throw embed::StorageWrappedTargetException(THROW_WHERE "Can't remove element!",
                                                       uno::Reference<io::XInputStream>(), aCaught);
        }
    }

    BroadcastModifiedIfNecessary();
}

void SAL_CALL OStorage::renameElement( const OUString& aElementName, const OUString& aNewName )
{
    {
        osl::MutexGuard aGuard(m_pData->m_xSharedMutex->GetMutex());

        if (!m_pImpl)
        {
            SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
            throw lang::DisposedException(THROW_WHERE);
        }

        if (aElementName.isEmpty()
            || !::comphelper::OStorageHelper::IsValidZipEntryFileName(aElementName, false)
            || aNewName.isEmpty()
            || !::comphelper::OStorageHelper::IsValidZipEntryFileName(aNewName, false))
            throw lang::IllegalArgumentException(THROW_WHERE "Unexpected entry name syntax.",
                                                 uno::Reference<uno::XInterface>(), 1);

        if (m_pData->m_nStorageType == embed::StorageFormats::OFOPXML
            && (aElementName == "_rels" || aNewName == "_rels"))
            throw lang::IllegalArgumentException(THROW_WHERE, uno::Reference<uno::XInterface>(),
                                                 0); // TODO: unacceptable element name

        if (!(m_pImpl->m_nStorageMode & embed::ElementModes::WRITE))
            throw io::IOException(THROW_WHERE); // TODO: access denied

        try
        {
            SotElement_Impl* pRefElement = m_pImpl->FindElement(aNewName);
            if (pRefElement)
                throw container::ElementExistException(THROW_WHERE); //???

            auto pElement = m_pImpl->FindElement( aElementName );
            if ( !pElement )
                throw container::NoSuchElementException(THROW_WHERE); //???

            auto mapIt = m_pImpl->m_aChildrenMap.find(aElementName);
            auto rVec = mapIt->second;
            for (auto it = rVec.begin(); it != rVec.end(); ++it)
                if (pElement == *it)
                {
                    rVec.erase(std::remove(rVec.begin(), rVec.end(), pElement), rVec.end());
                    if (rVec.empty())
                        m_pImpl->m_aChildrenMap.erase(mapIt);
                    break;
                }
            m_pImpl->m_aChildrenMap[aNewName].push_back(pElement);
            m_pImpl->m_bIsModified = true;
            m_pImpl->m_bBroadcastModified = true;
        }
        catch (const embed::InvalidStorageException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
        }
        catch (const lang::IllegalArgumentException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
        }
        catch (const container::NoSuchElementException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
        }
        catch (const container::ElementExistException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
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

            throw embed::StorageWrappedTargetException(THROW_WHERE "Can't rename element!",
                                                       uno::Reference<io::XInputStream>(), aCaught);
        }
    }

    BroadcastModifiedIfNecessary();
}

void SAL_CALL OStorage::copyElementTo(  const OUString& aElementName,
                                        const uno::Reference< embed::XStorage >& xDest,
                                        const OUString& aNewName )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aElementName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aElementName, false )
      || aNewName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aNewName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( !xDest.is() )
        // || xDest == uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( this ), uno::UNO_QUERY ) )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 2 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && ( aElementName == "_rels" || aNewName == "_rels" ) )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 0 ); // unacceptable element name

    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( aElementName );
        if ( !pElement )
            throw container::NoSuchElementException( THROW_WHERE );

        uno::Reference< XNameAccess > xNameAccess( xDest, uno::UNO_QUERY_THROW );
        if ( xNameAccess->hasByName( aNewName ) )
            throw container::ElementExistException( THROW_WHERE );

        m_pImpl->CopyStorageElement( pElement, xDest, aNewName, false );
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const container::NoSuchElementException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const container::ElementExistException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const io::IOException& )
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

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't copy element!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

void SAL_CALL OStorage::moveElementTo(  const OUString& aElementName,
                                        const uno::Reference< embed::XStorage >& xDest,
                                        const OUString& aNewName )
{
    {
        osl::MutexGuard aGuard(m_pData->m_xSharedMutex->GetMutex());

        if (!m_pImpl)
        {
            SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
            throw lang::DisposedException(THROW_WHERE);
        }

        if (aElementName.isEmpty()
            || !::comphelper::OStorageHelper::IsValidZipEntryFileName(aElementName, false)
            || aNewName.isEmpty()
            || !::comphelper::OStorageHelper::IsValidZipEntryFileName(aNewName, false))
            throw lang::IllegalArgumentException(THROW_WHERE "Unexpected entry name syntax.",
                                                 uno::Reference<uno::XInterface>(), 1);

        if (!xDest.is()
            || xDest
                   == uno::Reference<uno::XInterface>(static_cast<OWeakObject*>(this),
                                                      uno::UNO_QUERY))
            throw lang::IllegalArgumentException(THROW_WHERE, uno::Reference<uno::XInterface>(), 2);

        if (m_pData->m_nStorageType == embed::StorageFormats::OFOPXML
            && (aElementName == "_rels" || aNewName == "_rels"))
            throw lang::IllegalArgumentException(THROW_WHERE, uno::Reference<uno::XInterface>(),
                                                 0); // unacceptable element name

        if (!(m_pImpl->m_nStorageMode & embed::ElementModes::WRITE))
            throw io::IOException(THROW_WHERE); // TODO: access denied

        try
        {
            auto pElement = m_pImpl->FindElement( aElementName );
            if ( !pElement )
                throw container::NoSuchElementException(THROW_WHERE); //???

            uno::Reference<XNameAccess> xNameAccess(xDest, uno::UNO_QUERY_THROW);
            if (xNameAccess->hasByName(aNewName))
                throw container::ElementExistException(THROW_WHERE);

            m_pImpl->CopyStorageElement(pElement, xDest, aNewName, false);

            m_pImpl->RemoveElement(aElementName, pElement);

            m_pImpl->m_bIsModified = true;
            m_pImpl->m_bBroadcastModified = true;
        }
        catch (const embed::InvalidStorageException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
        }
        catch (const lang::IllegalArgumentException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
        }
        catch (const container::NoSuchElementException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
        }
        catch (const container::ElementExistException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
        }
        catch (const embed::StorageWrappedTargetException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
            throw;
        }
        catch (const io::IOException&)
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

            throw embed::StorageWrappedTargetException(THROW_WHERE "Can't move element!",
                                                       uno::Reference<io::XInputStream>(), aCaught);
        }
    }

    BroadcastModifiedIfNecessary();
}

//  XStorage2
uno::Reference< io::XStream > SAL_CALL OStorage::openEncryptedStream(
    const OUString& aStreamName, sal_Int32 nOpenMode, const uno::Sequence< beans::NamedValue >& aEncryptionData )
{
    osl::ClearableMutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( ( nOpenMode & embed::ElementModes::WRITE ) && m_pData->m_bReadOnlyWrap )
        throw io::IOException( THROW_WHERE ); // TODO: access denied

    if ( !aEncryptionData.hasElements() )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 3 );

    uno::Reference< io::XStream > xResult;
    try
    {
        SotElement_Impl *pElement = OpenStreamElement_Impl( aStreamName, nOpenMode, true );
        OSL_ENSURE(pElement && pElement->m_xStream, "In case element can not be created an exception must be thrown!");

        xResult = pElement->m_xStream->GetStream(nOpenMode, aEncryptionData, false);
        SAL_WARN_IF( !xResult.is(), "package.xstor", "The method must throw exception instead of removing empty result!" );

        if ( m_pData->m_bReadOnlyWrap )
        {
            // before the storage disposes the stream it must deregister itself as listener
            uno::Reference< lang::XComponent > xStreamComponent( xResult, uno::UNO_QUERY_THROW );
            MakeLinkToSubComponent_Impl( xStreamComponent );
        }
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const packages::NoEncryptionException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const packages::WrongPasswordException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const io::IOException& )
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

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't open encrypted stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    aGuard.clear();

    BroadcastModifiedIfNecessary();

    return xResult;
}

uno::Reference< io::XStream > SAL_CALL OStorage::cloneEncryptedStream(
    const OUString& aStreamName,
    const uno::Sequence< beans::NamedValue >& aEncryptionData )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( !aEncryptionData.hasElements() )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 2 );

    try
    {
        uno::Reference< io::XStream > xResult;
        m_pImpl->CloneStreamElement( aStreamName, true, aEncryptionData, xResult );
        if ( !xResult.is() )
            throw uno::RuntimeException( THROW_WHERE );
        return xResult;
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const packages::NoEncryptionException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const packages::WrongPasswordException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
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

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't clone encrypted stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

//  XStorageRawAccess
uno::Reference< io::XInputStream > SAL_CALL OStorage::getPlainRawStreamElement(
            const OUString& sStreamName )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( THROW_WHERE ); // the interface is not supported and must not be accessible

    if ( sStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( sStreamName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    uno::Reference < io::XInputStream > xTempIn;
    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( sStreamName );
        if ( !pElement )
            throw container::NoSuchElementException( THROW_WHERE );

        if (!pElement->m_xStream)
        {
            m_pImpl->OpenSubStream( pElement );
            if (!pElement->m_xStream)
                throw io::IOException( THROW_WHERE );
        }

        uno::Reference<io::XInputStream> xRawInStream = pElement->m_xStream->GetPlainRawInStream();
        if ( !xRawInStream.is() )
            throw io::IOException( THROW_WHERE );

        uno::Reference < io::XTempFile > xTempFile = io::TempFile::create( m_pImpl->m_xContext );
        uno::Reference < io::XOutputStream > xTempOut = xTempFile->getOutputStream();
        xTempIn = xTempFile->getInputStream();
        uno::Reference < io::XSeekable > xSeek( xTempOut, uno::UNO_QUERY );

        if ( !xTempOut.is() || !xTempIn.is() || !xSeek.is() )
            throw io::IOException( THROW_WHERE );

        // Copy temporary file to a new one
        ::comphelper::OStorageHelper::CopyInputToOutput( xRawInStream, xTempOut );
        xTempOut->closeOutput();
        xSeek->seek( 0 );
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const container::NoSuchElementException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const io::IOException& )
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

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't get plain raw stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    return xTempIn;
}

uno::Reference< io::XInputStream > SAL_CALL OStorage::getRawEncrStreamElement(
            const OUString& sStreamName )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw packages::NoEncryptionException( THROW_WHERE );

    if ( sStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( sStreamName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    uno::Reference < io::XInputStream > xTempIn;
    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( sStreamName );
        if ( !pElement )
            throw container::NoSuchElementException( THROW_WHERE );

        if (!pElement->m_xStream)
        {
            m_pImpl->OpenSubStream( pElement );
            if (!pElement->m_xStream)
                throw io::IOException( THROW_WHERE );
        }

        if (!pElement->m_xStream->IsEncrypted())
            throw packages::NoEncryptionException( THROW_WHERE );

        uno::Reference< io::XInputStream > xRawInStream = pElement->m_xStream->GetRawInStream();
        if ( !xRawInStream.is() )
            throw io::IOException( THROW_WHERE );

        uno::Reference < io::XTempFile > xTempFile = io::TempFile::create(m_pImpl->m_xContext);
        uno::Reference < io::XOutputStream > xTempOut = xTempFile->getOutputStream();
        xTempIn = xTempFile->getInputStream();
        uno::Reference < io::XSeekable > xSeek( xTempOut, uno::UNO_QUERY );

        if ( !xTempOut.is() || !xTempIn.is() || !xSeek.is() )
            throw io::IOException( THROW_WHERE );

        // Copy temporary file to a new one
        ::comphelper::OStorageHelper::CopyInputToOutput( xRawInStream, xTempOut );
        xTempOut->closeOutput();
        xSeek->seek( 0 );

    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const packages::NoEncryptionException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const container::NoSuchElementException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow");
        throw;
    }
    catch( const io::IOException& )
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

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't get raw stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    return xTempIn;
}

void SAL_CALL OStorage::insertRawEncrStreamElement( const OUString& aStreamName,
                                const uno::Reference< io::XInputStream >& xInStream )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw embed::InvalidStorageException( THROW_WHERE );

    if ( aStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( !xInStream.is() )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 2 );

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException( THROW_WHERE ); // TODO: access denied

    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( aStreamName );
        if ( pElement )
            throw container::ElementExistException( THROW_WHERE );

        m_pImpl->InsertRawStream( aStreamName, xInStream );
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const packages::NoRawFormatException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const container::ElementExistException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const io::IOException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't insert raw stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

//  XTransactedObject
void SAL_CALL OStorage::commit()
{
    uno::Reference< util::XModifiable > xParentModif;

    try {
        BroadcastTransaction( STOR_MESS_PRECOMMIT );

        ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

        if ( !m_pImpl )
        {
            SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
            throw lang::DisposedException( THROW_WHERE );
        }

        if ( m_pData->m_bReadOnlyWrap )
            throw io::IOException( THROW_WHERE ); // TODO: access_denied

        m_pImpl->Commit(); // the root storage initiates the storing to source

        // when the storage is committed the parent is modified
        if ( m_pImpl->m_pParent && m_pImpl->m_pParent->m_pAntiImpl )
            xParentModif = static_cast<util::XModifiable*>(m_pImpl->m_pParent->m_pAntiImpl);
    }
    catch( const io::IOException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw embed::StorageWrappedTargetException( THROW_WHERE "Problems on commit!",
                                  static_cast< ::cppu::OWeakObject* >( this ),
                                  aCaught );
    }

    setModified( false );
    if ( xParentModif.is() )
        xParentModif->setModified( true );

    BroadcastTransaction( STOR_MESS_COMMITTED );
}

void SAL_CALL OStorage::revert()
{
    // the method removes all the changes done after last commit

    BroadcastTransaction( STOR_MESS_PREREVERT );

    {
        osl::MutexGuard aGuard(m_pData->m_xSharedMutex->GetMutex());

        if (!m_pImpl)
        {
            SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
            throw lang::DisposedException(THROW_WHERE);
        }

        for (const auto & rPair : m_pImpl->m_aChildrenMap)
            for (auto pElement : rPair.second)
            {
                bool bThrow = (pElement->m_xStorage
                        && (pElement->m_xStorage->m_pAntiImpl
                            || !pElement->m_xStorage->m_aReadOnlyWrapVector.empty()))
                       || (pElement->m_xStream
                           && (pElement->m_xStream->m_pAntiImpl
                               || !pElement->m_xStream->m_aInputStreamsVector.empty()));
                if (bThrow)
                    throw io::IOException(THROW_WHERE); // TODO: access denied
            }

        if (m_pData->m_bReadOnlyWrap || !m_pImpl->m_bListCreated)
            return; // nothing to do

        try
        {
            m_pImpl->Revert();
            m_pImpl->m_bIsModified = false;
            m_pImpl->m_bBroadcastModified = true;
        }
        catch (const io::IOException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
            throw;
        }
        catch (const embed::StorageWrappedTargetException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
            throw;
        }
        catch (const uno::RuntimeException&)
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
            throw;
        }
        catch (const uno::Exception&)
        {
            uno::Any aCaught(::cppu::getCaughtException());
            SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

            throw embed::StorageWrappedTargetException(THROW_WHERE "Problems on revert!",
                                                       static_cast<::cppu::OWeakObject*>(this),
                                                       aCaught);
        }
    }

    setModified( false );
    BroadcastTransaction( STOR_MESS_REVERTED );
}

//  XTransactionBroadcaster
void SAL_CALL OStorage::addTransactionListener( const uno::Reference< embed::XTransactionListener >& aListener )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    m_pData->m_aListenersContainer.addInterface( cppu::UnoType<embed::XTransactionListener>::get(),
                                                aListener );
}

void SAL_CALL OStorage::removeTransactionListener( const uno::Reference< embed::XTransactionListener >& aListener )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    m_pData->m_aListenersContainer.removeInterface( cppu::UnoType<embed::XTransactionListener>::get(),
                                                    aListener );
}

//  XModifiable
//  TODO: if there will be no demand on this interface it will be removed from implementation,
//        I do not want to remove it now since it is still possible that it will be inserted
//        to the service back.

sal_Bool SAL_CALL OStorage::isModified()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    return m_pImpl->m_bIsModified;
}

void SAL_CALL OStorage::setModified( sal_Bool bModified )
{
    {
        osl::MutexGuard aGuard(m_pData->m_xSharedMutex->GetMutex());

        if (!m_pImpl)
        {
            SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
            throw lang::DisposedException(THROW_WHERE);
        }

        if (m_pData->m_bReadOnlyWrap)
            throw beans::PropertyVetoException(THROW_WHERE); // TODO: access denied

        if (m_pImpl->m_bIsModified != bool(bModified))
            m_pImpl->m_bIsModified = bModified;
    }

    if ( bModified )
    {
        m_pImpl->m_bBroadcastModified = true;
        BroadcastModifiedIfNecessary();
    }
}

void SAL_CALL OStorage::addModifyListener(
            const uno::Reference< util::XModifyListener >& aListener )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    osl_atomic_increment( &m_pImpl->m_nModifiedListenerCount );
    m_pData->m_aListenersContainer.addInterface(
                                cppu::UnoType<util::XModifyListener>::get(), aListener );
}

void SAL_CALL OStorage::removeModifyListener(
            const uno::Reference< util::XModifyListener >& aListener )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    osl_atomic_decrement( &m_pImpl->m_nModifiedListenerCount );
    m_pData->m_aListenersContainer.removeInterface(
                                cppu::UnoType<util::XModifyListener>::get(), aListener );
}

//  XNameAccess

uno::Any SAL_CALL OStorage::getByName( const OUString& aName )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aName == "_rels" )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 1 ); // unacceptable element name

    uno::Any aResult;
    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( aName );
        if ( !pElement )
            throw container::NoSuchElementException( THROW_WHERE );

        if ( pElement->m_bIsStorage )
            aResult <<= openStorageElement( aName, embed::ElementModes::READ );
        else
            aResult <<= openStreamElement( aName, embed::ElementModes::READ );
    }
    catch( const container::NoSuchElementException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const lang::WrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw lang::WrappedTargetException( THROW_WHERE "Can not open storage!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
    }

    return aResult;
}

uno::Sequence< OUString > SAL_CALL OStorage::getElementNames()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    try
    {
        return m_pImpl->GetElementNames();
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch ( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw lang::WrappedTargetRuntimeException( THROW_WHERE "Can not open storage!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
    }
}

sal_Bool SAL_CALL OStorage::hasByName( const OUString& aName )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aName.isEmpty() )
        return false;

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aName == "_rels" )
        return false;

    SotElement_Impl* pElement = nullptr;
    try
    {
        pElement = m_pImpl->FindElement( aName );
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch ( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw lang::WrappedTargetRuntimeException( THROW_WHERE "Can not open storage!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
    }

    return ( pElement != nullptr );
}

uno::Type SAL_CALL OStorage::getElementType()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    // it is a multitype container
    return uno::Type();
}

sal_Bool SAL_CALL OStorage::hasElements()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    try
    {
        return m_pImpl->HasChildren();
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw lang::WrappedTargetRuntimeException( THROW_WHERE "Can not open storage!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
    }
}

//  XComponent
void SAL_CALL OStorage::dispose()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    try
    {
        InternalDispose( true );
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw lang::WrappedTargetRuntimeException( THROW_WHERE "Can not open storage!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
    }
}

void SAL_CALL OStorage::addEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    m_pData->m_aListenersContainer.addInterface(
                                cppu::UnoType<lang::XEventListener>::get(), xListener );
}

void SAL_CALL OStorage::removeEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    m_pData->m_aListenersContainer.removeInterface(
                                cppu::UnoType<lang::XEventListener>::get(), xListener );
}

//  XEncryptionProtectedSource

void SAL_CALL OStorage::setEncryptionPassword( const OUString& aPass )
{
    setEncryptionData( ::comphelper::OStorageHelper::CreatePackageEncryptionData( aPass ) );
}

void SAL_CALL OStorage::removeEncryption()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw uno::RuntimeException( THROW_WHERE ); // the interface must be visible only for package storage

    SAL_WARN_IF( !m_pData->m_bIsRoot, "package.xstor", "removeEncryption() method is not available for nonroot storages!" );
    if ( !m_pData->m_bIsRoot )
        return;

    try {
        m_pImpl->ReadContents();
    }
    catch ( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch ( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw lang::WrappedTargetRuntimeException( THROW_WHERE "Can not open package!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
    }

    // TODO: check if the password is valid
    // update all streams that was encrypted with old password

    uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY_THROW );
    try
    {
        xPackPropSet->setPropertyValue( STORAGE_ENCRYPTION_KEYS_PROPERTY,
                                        uno::makeAny( uno::Sequence< beans::NamedValue >() ) );

        m_pImpl->m_bHasCommonEncryptionData = false;
        m_pImpl->m_aCommonEncryptionData.clear();
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_WARN_EXCEPTION( "package.xstor", "The call must not fail, it is pretty simple!" );
        throw;
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "package.xstor", "The call must not fail, it is pretty simple!" );
        throw io::IOException( THROW_WHERE );
    }
}

//  XEncryptionProtectedSource2

void SAL_CALL OStorage::setEncryptionData( const uno::Sequence< beans::NamedValue >& aEncryptionData )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw uno::RuntimeException( THROW_WHERE ); // the interface must be visible only for package storage

    if ( !aEncryptionData.hasElements() )
        throw uno::RuntimeException( THROW_WHERE "Unexpected empty encryption data!" );

    SAL_WARN_IF( !m_pData->m_bIsRoot, "package.xstor", "setEncryptionData() method is not available for nonroot storages!" );
    if ( !m_pData->m_bIsRoot )
        return;

    try {
        m_pImpl->ReadContents();
    }
    catch ( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch ( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw lang::WrappedTargetRuntimeException( THROW_WHERE "Can not open package!",
                            static_cast< OWeakObject* >( this ),
                            aCaught );
    }

    uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY_THROW );
    try
    {
        ::comphelper::SequenceAsHashMap aEncryptionMap( aEncryptionData );
        xPackPropSet->setPropertyValue( STORAGE_ENCRYPTION_KEYS_PROPERTY,
                                        uno::makeAny( aEncryptionMap.getAsConstNamedValueList() ) );

        m_pImpl->m_bHasCommonEncryptionData = true;
        m_pImpl->m_aCommonEncryptionData = aEncryptionMap;
    }
    catch( const uno::Exception& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:" );

        throw io::IOException( THROW_WHERE );
    }
}

sal_Bool SAL_CALL OStorage::hasEncryptionData()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    return m_pImpl && m_pImpl->m_bHasCommonEncryptionData;
}

//  XEncryptionProtectedStorage

void SAL_CALL OStorage::setEncryptionAlgorithms( const uno::Sequence< beans::NamedValue >& aAlgorithms )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw uno::RuntimeException( THROW_WHERE ); // the interface must be visible only for package storage

    if ( !aAlgorithms.hasElements() )
        throw uno::RuntimeException( THROW_WHERE "Unexpected empty encryption algorithms list!" );

    SAL_WARN_IF( !m_pData->m_bIsRoot, "package.xstor", "setEncryptionAlgorithms() method is not available for nonroot storages!" );
    if ( !m_pData->m_bIsRoot )
        return;

    try {
        m_pImpl->ReadContents();
    }
    catch ( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch ( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw lang::WrappedTargetRuntimeException( THROW_WHERE "Can not open package!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
    }

    uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY_THROW );
    try
    {
        xPackPropSet->setPropertyValue( ENCRYPTION_ALGORITHMS_PROPERTY,
                                        uno::makeAny( aAlgorithms ) );
    }
    catch ( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw lang::WrappedTargetRuntimeException( THROW_WHERE "Can not open package!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
    }
}

void SAL_CALL OStorage::setGpgProperties( const uno::Sequence< uno::Sequence< beans::NamedValue > >& aProps )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw uno::RuntimeException( THROW_WHERE ); // the interface must be visible only for package storage

    if ( !aProps.hasElements() )
        throw uno::RuntimeException( THROW_WHERE "Unexpected empty encryption algorithms list!" );

    SAL_WARN_IF( !m_pData->m_bIsRoot, "package.xstor", "setGpgProperties() method is not available for nonroot storages!" );
    if ( !m_pData->m_bIsRoot )
        return;

    try {
        m_pImpl->ReadContents();
    }
    catch ( const uno::RuntimeException& aRuntimeException )
    {
        SAL_INFO("package.xstor", "Rethrow: " << aRuntimeException.Message);
        throw;
    }
    catch ( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw lang::WrappedTargetRuntimeException( THROW_WHERE "Can not open package!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
    }

    uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY_THROW );
    try
    {
        xPackPropSet->setPropertyValue( ENCRYPTION_GPG_PROPERTIES,
                                        uno::makeAny( aProps ) );
    }
    catch ( const uno::RuntimeException& aRuntimeException )
    {
        SAL_INFO("package.xstor", "Rethrow: " << aRuntimeException.Message);
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw lang::WrappedTargetRuntimeException( THROW_WHERE "Can not open package!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
    }
}

uno::Sequence< beans::NamedValue > SAL_CALL OStorage::getEncryptionAlgorithms()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw uno::RuntimeException( THROW_WHERE ); // the interface must be visible only for package storage

    uno::Sequence< beans::NamedValue > aResult;
    SAL_WARN_IF( !m_pData->m_bIsRoot, "package.xstor", "getEncryptionAlgorithms() method is not available for nonroot storages!" );
    if ( m_pData->m_bIsRoot )
    {
        try {
            m_pImpl->ReadContents();
        }
        catch ( const uno::RuntimeException& )
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
            throw;
        }
        catch ( const uno::Exception& )
        {
            uno::Any aCaught( ::cppu::getCaughtException() );
            SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

            throw lang::WrappedTargetRuntimeException( THROW_WHERE "Can not open package!",
                                                static_cast< OWeakObject* >( this ),
                                                aCaught );
        }

        uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY_THROW );
        try
        {
            xPackPropSet->getPropertyValue( ENCRYPTION_ALGORITHMS_PROPERTY ) >>= aResult;
        }
        catch ( const uno::RuntimeException& )
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
            throw;
        }
        catch( const uno::Exception& )
        {
            uno::Any aCaught( ::cppu::getCaughtException() );
            SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

            throw lang::WrappedTargetRuntimeException( THROW_WHERE "Can not open package!",
                                                static_cast< OWeakObject* >( this ),
                                                aCaught );
        }
    }

    return aResult;
}

//  XPropertySet

uno::Reference< beans::XPropertySetInfo > SAL_CALL OStorage::getPropertySetInfo()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    //TODO:
    return uno::Reference< beans::XPropertySetInfo >();
}

void SAL_CALL OStorage::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    //TODO: think about interaction handler

    // WORKAROUND:
    // The old document might have no version in the manifest.xml, so we have to allow to set the version
    // even for readonly storages, so that the version from content.xml can be used.
    if ( m_pData->m_bReadOnlyWrap && aPropertyName != "Version" )
        throw uno::RuntimeException( THROW_WHERE ); // TODO: Access denied

    if ( m_pData->m_nStorageType == embed::StorageFormats::ZIP )
        throw beans::UnknownPropertyException( aPropertyName );
    else if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        if ( aPropertyName == "MediaType" )
        {
            aValue >>= m_pImpl->m_aMediaType;
            m_pImpl->m_bControlMediaType = true;

            m_pImpl->m_bBroadcastModified = true;
            m_pImpl->m_bIsModified = true;
        }
        else if ( aPropertyName == "Version" )
        {
            aValue >>= m_pImpl->m_aVersion;
            m_pImpl->m_bControlVersion = true;

            // this property can be set even for readonly storage
            if ( !m_pData->m_bReadOnlyWrap )
            {
                m_pImpl->m_bBroadcastModified = true;
                m_pImpl->m_bIsModified = true;
            }
        }
        else if ( ( m_pData->m_bIsRoot && ( aPropertyName == HAS_ENCRYPTED_ENTRIES_PROPERTY
                                    || aPropertyName == HAS_NONENCRYPTED_ENTRIES_PROPERTY
                                    || aPropertyName == IS_INCONSISTENT_PROPERTY
                                    || aPropertyName == "URL"
                                    || aPropertyName == "RepairPackage"
                                    || aPropertyName == ENCRYPTION_GPG_PROPERTIES) )
           || aPropertyName == "IsRoot"
           || aPropertyName == MEDIATYPE_FALLBACK_USED_PROPERTY )
            throw beans::PropertyVetoException( THROW_WHERE );
        else
            throw beans::UnknownPropertyException( aPropertyName );
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        if ( aPropertyName == "RelationsInfoStream" )
        {
            uno::Reference< io::XInputStream > xInRelStream;
            if ( !( aValue >>= xInRelStream ) || !xInRelStream.is() )
                throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );

            uno::Reference< io::XSeekable > xSeek( xInRelStream, uno::UNO_QUERY );
            if ( !xSeek.is() )
            {
                // currently this is an internal property that is used for optimization
                // and the stream must support XSeekable interface
                // TODO/LATER: in future it can be changed if property is used from outside
                throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );
            }

            m_pImpl->m_xNewRelInfoStream = xInRelStream;
            m_pImpl->m_aRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
            m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED_STREAM;
            m_pImpl->m_bBroadcastModified = true;
            m_pImpl->m_bIsModified = true;
        }
        else if ( aPropertyName == "RelationsInfo" )
        {
            if ( !(aValue >>= m_pImpl->m_aRelInfo) )
                throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );

            m_pImpl->m_xNewRelInfoStream.clear();
            m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
            m_pImpl->m_bBroadcastModified = true;
            m_pImpl->m_bIsModified = true;
        }
        else if ( ( m_pData->m_bIsRoot && ( aPropertyName == "URL" || aPropertyName == "RepairPackage") )
                 || aPropertyName == "IsRoot" )
            throw beans::PropertyVetoException( THROW_WHERE );
        else
            throw beans::UnknownPropertyException( aPropertyName );
    }
    else
        throw beans::UnknownPropertyException( aPropertyName );

    BroadcastModifiedIfNecessary();
}

uno::Any SAL_CALL OStorage::getPropertyValue( const OUString& aPropertyName )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE
      && ( aPropertyName == "MediaType" || aPropertyName == MEDIATYPE_FALLBACK_USED_PROPERTY || aPropertyName == "Version" ) )
    {
        try
        {
            m_pImpl->ReadContents();
        }
        catch ( const uno::RuntimeException& )
        {
            TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
            throw;
        }
        catch ( const uno::Exception& )
        {
            uno::Any aCaught( ::cppu::getCaughtException() );
            SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

            throw lang::WrappedTargetException(
                                        "Can't read contents!",
                                        static_cast< OWeakObject* >( this ),
                                        aCaught );
        }

        if ( aPropertyName == "MediaType" )
            return uno::makeAny( m_pImpl->m_aMediaType );
        else if ( aPropertyName == "Version" )
            return uno::makeAny( m_pImpl->m_aVersion );
        else
            return uno::makeAny( m_pImpl->m_bMTFallbackUsed );
    }
    else if ( aPropertyName == "IsRoot" )
    {
        return uno::makeAny( m_pData->m_bIsRoot );
    }
    else if ( aPropertyName == "OpenMode" )
    {
        return uno::makeAny( m_pImpl->m_nStorageMode );
    }
    else if ( m_pData->m_bIsRoot )
    {
        if ( aPropertyName == "URL"
          || aPropertyName == "RepairPackage" )
        {
            auto pProp = std::find_if(std::cbegin(m_pImpl->m_xProperties), std::cend(m_pImpl->m_xProperties),
                [&aPropertyName](const css::beans::PropertyValue& rProp) { return rProp.Name == aPropertyName; });
            if (pProp != std::cend(m_pImpl->m_xProperties))
                return pProp->Value;

            if ( aPropertyName == "URL" )
                return uno::makeAny( OUString() );

            return uno::makeAny( false ); // RepairPackage
        }
        else if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE
          && ( aPropertyName == HAS_ENCRYPTED_ENTRIES_PROPERTY
            || aPropertyName == HAS_NONENCRYPTED_ENTRIES_PROPERTY
            || aPropertyName == ENCRYPTION_GPG_PROPERTIES
            || aPropertyName == IS_INCONSISTENT_PROPERTY ) )
        {
            try {
                m_pImpl->ReadContents();
                uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY_THROW );
                return xPackPropSet->getPropertyValue( aPropertyName );
            }
            catch ( const uno::RuntimeException& )
            {
                TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
                throw;
            }
            catch ( const uno::Exception& )
            {
                uno::Any aCaught( ::cppu::getCaughtException() );
                SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

                throw lang::WrappedTargetException( THROW_WHERE "Can not open package!",
                                                    static_cast< OWeakObject* >( this ),
                                                    aCaught );
            }
        }
    }

    throw beans::UnknownPropertyException(aPropertyName);
}

void SAL_CALL OStorage::addPropertyChangeListener(
    const OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    //TODO:
}

void SAL_CALL OStorage::removePropertyChangeListener(
    const OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    //TODO:
}

void SAL_CALL OStorage::addVetoableChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    //TODO:
}

void SAL_CALL OStorage::removeVetoableChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    //TODO:
}

//  XRelationshipAccess

// TODO/LATER: the storage and stream implementations of this interface are very similar, they could use a helper class

sal_Bool SAL_CALL OStorage::hasByID(  const OUString& sID )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( THROW_WHERE );

    try
    {
        getRelationshipByID( sID );
        return true;
    }
    catch( const container::NoSuchElementException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
    }

    return false;
}

namespace
{

const beans::StringPair* lcl_findPairByName(const uno::Sequence<beans::StringPair>& rSeq, const OUString& rName)
{
    return std::find_if(rSeq.begin(), rSeq.end(), [&rName](const beans::StringPair& rPair) { return rPair.First == rName; });
}

}

OUString SAL_CALL OStorage::getTargetByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( THROW_WHERE );

    const uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    auto pRel = lcl_findPairByName(aSeq, "Target");
    if (pRel != aSeq.end())
        return pRel->Second;

    return OUString();
}

OUString SAL_CALL OStorage::getTypeByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( THROW_WHERE );

    const uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    auto pRel = lcl_findPairByName(aSeq, "Type");
    if (pRel != aSeq.end())
        return pRel->Second;

    return OUString();
}

uno::Sequence< beans::StringPair > SAL_CALL OStorage::getRelationshipByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( THROW_WHERE );

    // TODO/LATER: in future the unification of the ID could be checked
    const uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    const beans::StringPair aIDRel("Id", sID);

    auto pRel = std::find_if(aSeq.begin(), aSeq.end(),
        [&aIDRel](const uno::Sequence<beans::StringPair>& rRel) {
            return std::find(rRel.begin(), rRel.end(), aIDRel) != rRel.end(); });
    if (pRel != aSeq.end())
        return *pRel;

    throw container::NoSuchElementException( THROW_WHERE );
}

uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OStorage::getRelationshipsByType(  const OUString& sType  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( THROW_WHERE );

    // TODO/LATER: in future the unification of the ID could be checked
    const uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    std::vector< uno::Sequence< beans::StringPair > > aResult;
    aResult.reserve(aSeq.getLength());

    std::copy_if(aSeq.begin(), aSeq.end(), std::back_inserter(aResult),
        [&sType](const uno::Sequence<beans::StringPair>& rRel) {
            auto pRel = lcl_findPairByName(rRel, "Type");
            return pRel != rRel.end()
                // the type is usually a URL, so the check should be case insensitive
                && pRel->Second.equalsIgnoreAsciiCase( sType );
        });

    return comphelper::containerToSequence(aResult);
}

uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OStorage::getAllRelationships()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( THROW_WHERE );

    uno::Sequence< uno::Sequence< beans::StringPair > > aRet;
    try
    {
        aRet = m_pImpl->GetAllRelationshipsIfAny();
    }
    catch (const io::IOException&)
    {
        throw;
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception &)
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(THROW_WHERE "Can't getAllRelationships!",
                                                 uno::Reference< uno::XInterface >(),
                                                 aCaught);
    }

    return aRet;
}

void SAL_CALL OStorage::insertRelationshipByID(  const OUString& sID, const uno::Sequence< beans::StringPair >& aEntry, sal_Bool bReplace  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( THROW_WHERE );

    const beans::StringPair aIDRel("Id", sID);

    sal_Int32 nIDInd = -1;

    // TODO/LATER: in future the unification of the ID could be checked
    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
    {
        const auto& rRel = aSeq[nInd];
        if (std::find(rRel.begin(), rRel.end(), aIDRel) != rRel.end())
            nIDInd = nInd;
    }

    if ( nIDInd != -1 && !bReplace )
        throw container::ElementExistException( THROW_WHERE );

    if ( nIDInd == -1 )
    {
        nIDInd = aSeq.getLength();
        aSeq.realloc( nIDInd + 1 );
    }

    std::vector<beans::StringPair> aResult;
    aResult.reserve(aEntry.getLength() + 1);

    aResult.push_back(aIDRel);
    std::copy_if(aEntry.begin(), aEntry.end(), std::back_inserter(aResult),
        [](const beans::StringPair& rPair) { return rPair.First != "Id"; });

    aSeq[nIDInd] = comphelper::containerToSequence(aResult);

    m_pImpl->m_aRelInfo = aSeq;
    m_pImpl->m_xNewRelInfoStream.clear();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

void SAL_CALL OStorage::removeRelationshipByID(  const OUString& sID  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( THROW_WHERE );

    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    const beans::StringPair aIDRel("Id", sID);
    auto pRel = std::find_if(std::cbegin(aSeq), std::cend(aSeq),
        [&aIDRel](const uno::Sequence< beans::StringPair >& rRel) {
            return std::find(rRel.begin(), rRel.end(), aIDRel) != rRel.end(); });
    if (pRel != std::cend(aSeq))
    {
        auto nInd = static_cast<sal_Int32>(std::distance(std::cbegin(aSeq), pRel));
        comphelper::removeElementAt(aSeq, nInd);

        m_pImpl->m_aRelInfo = aSeq;
        m_pImpl->m_xNewRelInfoStream.clear();
        m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;

        // TODO/LATER: in future the unification of the ID could be checked
        return;
    }

    throw container::NoSuchElementException( THROW_WHERE );
}

void SAL_CALL OStorage::insertRelationships(  const uno::Sequence< uno::Sequence< beans::StringPair > >& aEntries, sal_Bool bReplace  )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( THROW_WHERE );

    OUString aIDTag( "Id" );
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
                throw container::ElementExistException( THROW_WHERE );

            // if no such element in the provided sequence
            return !bIsSourceSame;
        });

    std::transform(aEntries.begin(), aEntries.end(), std::back_inserter(aResultVec),
        [&aIDTag](const uno::Sequence<beans::StringPair>& rEntry) -> uno::Sequence<beans::StringPair> {
            auto pPair = lcl_findPairByName(rEntry, aIDTag);
            if (pPair == rEntry.end())
                throw io::IOException( THROW_WHERE ); // TODO: illegal relation ( no ID )

            auto aResult = comphelper::sequenceToContainer<std::vector<beans::StringPair>>(rEntry);
            auto nIDInd = std::distance(rEntry.begin(), pPair);
            std::rotate(aResult.begin(), std::next(aResult.begin(), nIDInd), std::next(aResult.begin(), nIDInd + 1));

            return comphelper::containerToSequence(aResult);
        });

    m_pImpl->m_aRelInfo = comphelper::containerToSequence(aResultVec);
    m_pImpl->m_xNewRelInfoStream.clear();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

void SAL_CALL OStorage::clearRelationships()
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( THROW_WHERE );

    m_pImpl->m_aRelInfo.realloc( 0 );
    m_pImpl->m_xNewRelInfoStream.clear();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

//  XOptimizedStorage
void SAL_CALL OStorage::insertRawNonEncrStreamElementDirect(
            const OUString& /*sStreamName*/,
            const uno::Reference< io::XInputStream >& /*xInStream*/ )
{
    // not implemented currently because there is still no demand
    // might need to be implemented if direct copying of compressed streams is used
    throw io::IOException( THROW_WHERE );
}

void SAL_CALL OStorage::insertStreamElementDirect(
            const OUString& aStreamName,
            const uno::Reference< io::XInputStream >& xInStream,
            const uno::Sequence< beans::PropertyValue >& aProps )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aStreamName == "_rels" )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 1 ); // unacceptable storage name

    if ( m_pData->m_bReadOnlyWrap )
        throw io::IOException( THROW_WHERE ); // TODO: access denied

    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( aStreamName );

        if ( pElement )
            throw container::ElementExistException( THROW_WHERE );

        pElement = OpenStreamElement_Impl( aStreamName, embed::ElementModes::READWRITE, false );
        OSL_ENSURE(pElement && pElement->m_xStream, "In case element can not be created an exception must be thrown!");

        pElement->m_xStream->InsertStreamDirectly(xInStream, aProps);
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const container::ElementExistException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const io::IOException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't insert stream directly!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

void SAL_CALL OStorage::copyElementDirectlyTo(
            const OUString& aElementName,
            const uno::Reference< embed::XOptimizedStorage >& xDest,
            const OUString& aNewName )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aElementName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aElementName, false )
      || aNewName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aNewName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( !xDest.is() || xDest == uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( this ), uno::UNO_QUERY ) )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 2 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && ( aElementName == "_rels" || aNewName == "_rels" ) )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 0 ); // unacceptable name

    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( aElementName );
        if ( !pElement )
            throw container::NoSuchElementException( THROW_WHERE );

        uno::Reference< XNameAccess > xNameAccess( xDest, uno::UNO_QUERY_THROW );
        if ( xNameAccess->hasByName( aNewName ) )
            throw container::ElementExistException( THROW_WHERE );

        // let the element be copied directly
        uno::Reference< embed::XStorage > xStorDest( xDest, uno::UNO_QUERY_THROW );
        m_pImpl->CopyStorageElement( pElement, xStorDest, aNewName, true );
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const container::NoSuchElementException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const container::ElementExistException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const io::IOException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't copy element directly!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

void SAL_CALL OStorage::writeAndAttachToStream( const uno::Reference< io::XStream >& xStream )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( !m_pData->m_bIsRoot )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );

    if ( !m_pImpl->m_pSwitchStream )
        throw uno::RuntimeException( THROW_WHERE );

    try
    {
        m_pImpl->m_pSwitchStream->CopyAndSwitchPersistenceTo( xStream );
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const io::IOException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:" );
        throw;
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't write and attach to stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

}

void SAL_CALL OStorage::attachToURL( const OUString& sURL,
                                    sal_Bool bReadOnly )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( !m_pData->m_bIsRoot )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );

    if ( !m_pImpl->m_pSwitchStream )
        throw uno::RuntimeException( THROW_WHERE );

    uno::Reference < ucb::XSimpleFileAccess3 > xAccess(
        ucb::SimpleFileAccess::create( m_pImpl->m_xContext ) );

    try
    {
        if ( bReadOnly )
        {
            uno::Reference< io::XInputStream > xInputStream = xAccess->openFileRead( sURL );
            m_pImpl->m_pSwitchStream->SwitchPersistenceTo( xInputStream );
        }
        else
        {
            uno::Reference< io::XStream > xStream = xAccess->openFileReadWrite( sURL );
            m_pImpl->m_pSwitchStream->SwitchPersistenceTo( xStream );
        }
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const io::IOException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't attach to URL!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

uno::Any SAL_CALL OStorage::getElementPropertyValue( const OUString& aElementName, const OUString& aPropertyName )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aElementName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aElementName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aElementName == "_rels" )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 1 ); // TODO: unacceptable name

    try
    {
        SotElement_Impl *pElement = m_pImpl->FindElement( aElementName );
        if ( !pElement )
            throw container::NoSuchElementException( THROW_WHERE );

        // TODO/LATER: Currently it is only implemented for MediaType property of substorages, might be changed in future
        if ( !pElement->m_bIsStorage || m_pData->m_nStorageType != embed::StorageFormats::PACKAGE || aPropertyName != "MediaType" )
            throw beans::PropertyVetoException( THROW_WHERE );

        if (!pElement->m_xStorage)
            m_pImpl->OpenSubStorage( pElement, embed::ElementModes::READ );

        if (!pElement->m_xStorage)
            throw io::IOException( THROW_WHERE ); // TODO: general_error

        pElement->m_xStorage->ReadContents();
        return uno::makeAny(pElement->m_xStorage->m_aMediaType);
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const container::NoSuchElementException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const beans::UnknownPropertyException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const beans::PropertyVetoException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const io::IOException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't get element property!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

void SAL_CALL OStorage::copyStreamElementData( const OUString& aStreamName, const uno::Reference< io::XStream >& xTargetStream )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamName, false ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aStreamName == "_rels" )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 1 ); // unacceptable name

    if ( !xTargetStream.is() )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 2 );

    try
    {
        uno::Reference< io::XStream > xNonconstRef = xTargetStream;
        m_pImpl->CloneStreamElement( aStreamName, false, ::comphelper::SequenceAsHashMap(), xNonconstRef );

        SAL_WARN_IF( xNonconstRef != xTargetStream, "package.xstor", "The provided stream reference seems not be filled in correctly!" );
        if ( xNonconstRef != xTargetStream )
            throw uno::RuntimeException( THROW_WHERE ); // if the stream reference is set it must not be changed!
    }
    catch( const embed::InvalidStorageException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const packages::WrongPasswordException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const io::IOException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const embed::StorageWrappedTargetException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_INFO_EXCEPTION("package.xstor", "Rethrow:");
        throw;
    }
    catch( const uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        SAL_INFO("package.xstor", "Rethrow: " << exceptionToString(aCaught));

        throw embed::StorageWrappedTargetException( THROW_WHERE "Can't copy stream data!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

}

// XHierarchicalStorageAccess
uno::Reference< embed::XExtendedStorageStream > SAL_CALL OStorage::openStreamElementByHierarchicalName( const OUString& aStreamPath, ::sal_Int32 nOpenMode )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aStreamPath.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamPath, true ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE )
      && ( nOpenMode & embed::ElementModes::WRITE ) )
        throw io::IOException( THROW_WHERE ); // Access denied

    std::vector<OUString> aListPath = OHierarchyHolder_Impl::GetListPathFromString( aStreamPath );
    OSL_ENSURE( aListPath.size(), "The result list must not be empty!" );

    uno::Reference< embed::XExtendedStorageStream > xResult;
    if ( aListPath.size() == 1 )
    {
        try
        {
            // that must be a direct request for a stream
            // the transacted version of the stream should be opened

            SotElement_Impl *pElement = OpenStreamElement_Impl( aStreamPath, nOpenMode, false );
            assert(pElement && pElement->m_xStream && "In case element can not be created an exception must be thrown!");

            xResult.set(pElement->m_xStream->GetStream(nOpenMode, true),
                        uno::UNO_QUERY_THROW);
        }
        catch ( const container::NoSuchElementException & )
        {
            throw io::IOException( THROW_WHERE ); // file not found
        }
    }
    else
    {
        // there are still storages in between
        if ( !m_pData->m_rHierarchyHolder.is() )
            m_pData->m_rHierarchyHolder = new OHierarchyHolder_Impl(
                uno::Reference< embed::XStorage >( static_cast< embed::XStorage* >( this ) ) );

        xResult = m_pData->m_rHierarchyHolder->GetStreamHierarchically(
                                                ( m_pImpl->m_nStorageMode & embed::ElementModes::READWRITE ),
                                                aListPath,
                                                nOpenMode );
    }

    if ( !xResult.is() )
        throw uno::RuntimeException( THROW_WHERE );

    return xResult;
}

uno::Reference< embed::XExtendedStorageStream > SAL_CALL OStorage::openEncryptedStreamElementByHierarchicalName( const OUString& aStreamPath, ::sal_Int32 nOpenMode, const OUString& sPassword )
{
    return openEncryptedStreamByHierarchicalName( aStreamPath, nOpenMode, ::comphelper::OStorageHelper::CreatePackageEncryptionData( sPassword ) );
}

void SAL_CALL OStorage::removeStreamElementByHierarchicalName( const OUString& aStreamPath )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( aStreamPath.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamPath, true ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException( THROW_WHERE ); // Access denied

    std::vector<OUString> aListPath = OHierarchyHolder_Impl::GetListPathFromString( aStreamPath );
    OSL_ENSURE( aListPath.size(), "The result list must not be empty!" );

    if ( !m_pData->m_rHierarchyHolder.is() )
        m_pData->m_rHierarchyHolder = new OHierarchyHolder_Impl(
            uno::Reference< embed::XStorage >( static_cast< embed::XStorage* >( this ) ) );

    m_pData->m_rHierarchyHolder->RemoveStreamHierarchically( aListPath );
}

// XHierarchicalStorageAccess2
uno::Reference< embed::XExtendedStorageStream > SAL_CALL OStorage::openEncryptedStreamByHierarchicalName( const OUString& aStreamPath, ::sal_Int32 nOpenMode, const uno::Sequence< beans::NamedValue >& aEncryptionData )
{
    ::osl::MutexGuard aGuard( m_pData->m_xSharedMutex->GetMutex() );

    if ( !m_pImpl )
    {
        SAL_INFO("package.xstor", THROW_WHERE "Disposed!");
        throw lang::DisposedException( THROW_WHERE );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw packages::NoEncryptionException( THROW_WHERE );

    if ( aStreamPath.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamPath, true ) )
        throw lang::IllegalArgumentException( THROW_WHERE "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( !aEncryptionData.hasElements() )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 3 );

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE )
      && ( nOpenMode & embed::ElementModes::WRITE ) )
        throw io::IOException( THROW_WHERE ); // Access denied

    std::vector<OUString> aListPath = OHierarchyHolder_Impl::GetListPathFromString( aStreamPath );
    OSL_ENSURE( aListPath.size(), "The result list must not be empty!" );

    uno::Reference< embed::XExtendedStorageStream > xResult;
    if ( aListPath.size() == 1 )
    {
        // that must be a direct request for a stream
        // the transacted version of the stream should be opened

        SotElement_Impl *pElement = OpenStreamElement_Impl( aStreamPath, nOpenMode, true );
        OSL_ENSURE(pElement && pElement->m_xStream, "In case element can not be created an exception must be thrown!");

        xResult.set(pElement->m_xStream->GetStream(nOpenMode, aEncryptionData, true),
                    uno::UNO_QUERY_THROW);
    }
    else
    {
        // there are still storages in between
        if ( !m_pData->m_rHierarchyHolder.is() )
            m_pData->m_rHierarchyHolder = new OHierarchyHolder_Impl(
                uno::Reference< embed::XStorage >( static_cast< embed::XStorage* >( this ) ) );

        xResult = m_pData->m_rHierarchyHolder->GetStreamHierarchically(
                                                ( m_pImpl->m_nStorageMode & embed::ElementModes::READWRITE ),
                                                aListPath,
                                                nOpenMode,
                                                aEncryptionData );
    }

    if ( !xResult.is() )
        throw uno::RuntimeException( THROW_WHERE );

    return xResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
