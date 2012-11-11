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

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/UseBackupException.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
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

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/logfile.hxx>
#include <rtl/instance.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/ofopxmlhelper.hxx>

#include "xstorage.hxx"
#include "owriteablestream.hxx"
#include "disposelistener.hxx"
#include "switchpersistencestream.hxx"
#include "ohierarchyholder.hxx"

using namespace ::com::sun::star;

//=========================================================

typedef ::std::list< uno::WeakReference< lang::XComponent > > WeakComponentList;

struct StorInternalData_Impl
{
    SotMutexHolderRef m_rSharedMutexRef;
    ::cppu::OMultiTypeInterfaceContainerHelper m_aListenersContainer; // list of listeners
    ::cppu::OTypeCollection* m_pTypeCollection;
    sal_Bool m_bIsRoot;
    sal_Int32 m_nStorageType; // the mode in wich the storage is used
    sal_Bool m_bReadOnlyWrap;

    OChildDispListener_Impl* m_pSubElDispListener;

    WeakComponentList m_aOpenSubComponentsList;

    ::rtl::Reference< OHierarchyHolder_Impl > m_rHierarchyHolder;

    // the mutex reference MUST NOT be empty
    StorInternalData_Impl( const SotMutexHolderRef& rMutexRef, sal_Bool bRoot, sal_Int32 nStorageType, sal_Bool bReadOnlyWrap )
    : m_rSharedMutexRef( rMutexRef )
    , m_aListenersContainer( rMutexRef->GetMutex() )
    , m_pTypeCollection( NULL )
    , m_bIsRoot( bRoot )
    , m_nStorageType( nStorageType )
    , m_bReadOnlyWrap( bReadOnlyWrap )
    , m_pSubElDispListener( NULL )
    {}

    ~StorInternalData_Impl();
};

//=========================================================
::rtl::OUString GetNewTempFileURL( const uno::Reference< lang::XMultiServiceFactory > xFactory );

// static
void OStorage_Impl::completeStorageStreamCopy_Impl(
                            const uno::Reference< io::XStream >& xSource,
                            const uno::Reference< io::XStream >& xDest,
                            sal_Int32 nStorageType,
                            const uno::Sequence< uno::Sequence< beans::StringPair > >& aRelInfo )
{
        uno::Reference< beans::XPropertySet > xSourceProps( xSource, uno::UNO_QUERY );
        uno::Reference< beans::XPropertySet > xDestProps( xDest, uno::UNO_QUERY );
        if ( !xSourceProps.is() || !xDestProps.is() )
            throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        uno::Reference< io::XOutputStream > xDestOutStream = xDest->getOutputStream();
        if ( !xDestOutStream.is() )
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        uno::Reference< io::XInputStream > xSourceInStream = xSource->getInputStream();
        if ( !xSourceInStream.is() )
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        // TODO: headers of encripted streams should be copied also
        ::comphelper::OStorageHelper::CopyInputToOutput( xSourceInStream, xDestOutStream );

        uno::Sequence< ::rtl::OUString > aPropNames( 1 );
        aPropNames[0] = "Compressed";

        if ( nStorageType == embed::StorageFormats::PACKAGE )
        {
            aPropNames.realloc( 3 );
            aPropNames[1] = "MediaType";
            aPropNames[2] = "UseCommonStoragePasswordEncryption";
        }
        else if ( nStorageType == embed::StorageFormats::OFOPXML )
        {
            // TODO/LATER: in future it might make sence to provide the stream if there is one
            uno::Reference< embed::XRelationshipAccess > xRelAccess( xDest, uno::UNO_QUERY_THROW );
            xRelAccess->clearRelationships();
            xRelAccess->insertRelationships( aRelInfo, sal_False );

            aPropNames.realloc( 2 );
            aPropNames[1] = "MediaType";
        }

        for ( int ind = 0; ind < aPropNames.getLength(); ind++ )
            xDestProps->setPropertyValue( aPropNames[ind], xSourceProps->getPropertyValue( aPropNames[ind] ) );
}

uno::Reference< io::XInputStream > GetSeekableTempCopy( uno::Reference< io::XInputStream > xInStream,
                                                        uno::Reference< lang::XMultiServiceFactory > xFactory )
{
    uno::Reference < io::XOutputStream > xTempOut(
                        io::TempFile::create(comphelper::getComponentContext(xFactory)),
                        uno::UNO_QUERY );
    uno::Reference < io::XInputStream > xTempIn( xTempOut, uno::UNO_QUERY );

    if ( !xTempOut.is() || !xTempIn.is() )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    ::comphelper::OStorageHelper::CopyInputToOutput( xInStream, xTempOut );
    xTempOut->closeOutput();

    return xTempIn;
}

StorInternalData_Impl::~StorInternalData_Impl()
{
    if ( m_pTypeCollection )
        delete m_pTypeCollection;
}


SotElement_Impl::SotElement_Impl( const ::rtl::OUString& rName, sal_Bool bStor, sal_Bool bNew )
: m_aName( rName )
, m_aOriginalName( rName )
, m_bIsRemoved( sal_False )
, m_bIsInserted( bNew )
, m_bIsStorage( bStor )
, m_pStorage( NULL )
, m_pStream( NULL )
{
}

SotElement_Impl::~SotElement_Impl()
{
    if ( m_pStorage )
        delete m_pStorage;

    if ( m_pStream )
        delete m_pStream;
}

//-----------------------------------------------
// most of properties are holt by the storage but are not used
OStorage_Impl::OStorage_Impl(   uno::Reference< io::XInputStream > xInputStream,
                                sal_Int32 nMode,
                                uno::Sequence< beans::PropertyValue > xProperties,
                                uno::Reference< lang::XMultiServiceFactory > xFactory,
                                sal_Int32 nStorageType )
: m_rMutexRef( new SotMutexHolder )
, m_pAntiImpl( NULL )
, m_nStorageMode( nMode & ~embed::ElementModes::SEEKABLE )
, m_bIsModified( ( nMode & ( embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE ) ) == ( embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE ) )
, m_bBroadcastModified( sal_False )
, m_bCommited( sal_False )
, m_bIsRoot( sal_True )
, m_bListCreated( sal_False )
, m_xFactory( xFactory )
, m_xProperties( xProperties )
, m_bHasCommonEncryptionData( sal_False )
, m_pParent( NULL )
, m_bControlMediaType( sal_False )
, m_bMTFallbackUsed( sal_False )
, m_bControlVersion( sal_False )
, m_pSwitchStream( NULL )
, m_nStorageType( nStorageType )
, m_pRelStorElement( NULL )
, m_nRelInfoStatus( RELINFO_NO_INIT )
{
    // all the checks done below by assertion statements must be done by factory
    OSL_ENSURE( xInputStream.is(), "No input stream is provided!\n" );

    m_pSwitchStream = (SwitchablePersistenceStream*) new SwitchablePersistenceStream( xFactory, xInputStream );
    m_xInputStream = m_pSwitchStream->getInputStream();

    if ( m_nStorageMode & embed::ElementModes::WRITE )
    {
        // check that the stream allows to write
        OSL_FAIL( "No stream for writing is provided!\n" );
    }
}

//-----------------------------------------------
// most of properties are holt by the storage but are not used
OStorage_Impl::OStorage_Impl(   uno::Reference< io::XStream > xStream,
                                sal_Int32 nMode,
                                uno::Sequence< beans::PropertyValue > xProperties,
                                uno::Reference< lang::XMultiServiceFactory > xFactory,
                                sal_Int32 nStorageType )
: m_rMutexRef( new SotMutexHolder )
, m_pAntiImpl( NULL )
, m_nStorageMode( nMode & ~embed::ElementModes::SEEKABLE )
, m_bIsModified( ( nMode & ( embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE ) ) == ( embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE ) )
, m_bBroadcastModified( sal_False )
, m_bCommited( sal_False )
, m_bIsRoot( sal_True )
, m_bListCreated( sal_False )
, m_xFactory( xFactory )
, m_xProperties( xProperties )
, m_bHasCommonEncryptionData( sal_False )
, m_pParent( NULL )
, m_bControlMediaType( sal_False )
, m_bMTFallbackUsed( sal_False )
, m_bControlVersion( sal_False )
, m_pSwitchStream( NULL )
, m_nStorageType( nStorageType )
, m_pRelStorElement( NULL )
, m_nRelInfoStatus( RELINFO_NO_INIT )
{
    // all the checks done below by assertion statements must be done by factory
    OSL_ENSURE( xStream.is(), "No stream is provided!\n" );

    if ( m_nStorageMode & embed::ElementModes::WRITE )
    {
        m_pSwitchStream = (SwitchablePersistenceStream*) new SwitchablePersistenceStream( xFactory, xStream );
        m_xStream = static_cast< io::XStream* >( m_pSwitchStream );
    }
    else
    {
        m_pSwitchStream = (SwitchablePersistenceStream*) new SwitchablePersistenceStream( xFactory,
                                                                                          xStream->getInputStream() );
        m_xInputStream = m_pSwitchStream->getInputStream();
    }
}

//-----------------------------------------------
OStorage_Impl::OStorage_Impl(   OStorage_Impl* pParent,
                                sal_Int32 nMode,
                                uno::Reference< container::XNameContainer > xPackageFolder,
                                uno::Reference< lang::XSingleServiceFactory > xPackage,
                                uno::Reference< lang::XMultiServiceFactory > xFactory,
                                sal_Int32 nStorageType )
: m_rMutexRef( new SotMutexHolder )
, m_pAntiImpl( NULL )
, m_nStorageMode( nMode & ~embed::ElementModes::SEEKABLE )
, m_bIsModified( ( nMode & ( embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE ) ) == ( embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE ) )
, m_bBroadcastModified( sal_False )
, m_bCommited( sal_False )
, m_bIsRoot( sal_False )
, m_bListCreated( sal_False )
, m_xPackageFolder( xPackageFolder )
, m_xPackage( xPackage )
, m_xFactory( xFactory )
, m_bHasCommonEncryptionData( sal_False )
, m_pParent( pParent ) // can be empty in case of temporary readonly substorages and relation storage
, m_bControlMediaType( sal_False )
, m_bMTFallbackUsed( sal_False )
, m_bControlVersion( sal_False )
, m_pSwitchStream( NULL )
, m_nStorageType( nStorageType )
, m_pRelStorElement( NULL )
, m_nRelInfoStatus( RELINFO_NO_INIT )
{
    OSL_ENSURE( xPackageFolder.is(), "No package folder!\n" );
}

//-----------------------------------------------
OStorage_Impl::~OStorage_Impl()
{
    {
        ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
        if ( m_pAntiImpl ) // root storage wrapper must set this member to NULL before destruction of object
        {
            OSL_ENSURE( !m_bIsRoot, "The root storage wrapper must be disposed already" );

            try {
                m_pAntiImpl->InternalDispose( sal_False );
            }
            catch ( const uno::Exception& rException )
            {
                AddLog( rException.Message );
                AddLog( OSL_LOG_PREFIX "Quiet exception" );
            }
            m_pAntiImpl = NULL;
        }
        else if ( !m_aReadOnlyWrapList.empty() )
        {
            for ( OStorageList_Impl::iterator pStorageIter = m_aReadOnlyWrapList.begin();
                  pStorageIter != m_aReadOnlyWrapList.end(); ++pStorageIter )
            {
                uno::Reference< embed::XStorage > xTmp = pStorageIter->m_xWeakRef;
                if ( xTmp.is() )
                    try {
                        pStorageIter->m_pPointer->InternalDispose( sal_False );
                    } catch( const uno::Exception& rException )
                    {
                        AddLog( rException.Message );
                        AddLog( OSL_LOG_PREFIX "Quiet exception" );
                    }
            }

            m_aReadOnlyWrapList.clear();
        }

        m_pParent = NULL;
    }

    for ( SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
          pElementIter != m_aChildrenList.end(); ++pElementIter )
        delete *pElementIter;

    m_aChildrenList.clear();

    for ( SotElementList_Impl::iterator pDeletedIter = m_aDeletedList.begin();
          pDeletedIter != m_aDeletedList.end(); ++pDeletedIter )
        delete *pDeletedIter;

    m_aDeletedList.clear();

    if ( m_nStorageType == embed::StorageFormats::OFOPXML && m_pRelStorElement )
    {
        delete m_pRelStorElement;
        m_pRelStorElement = NULL;
    }

    m_xPackageFolder = uno::Reference< container::XNameContainer >();
    m_xPackage = uno::Reference< lang::XSingleServiceFactory >();

    ::rtl::OUString aPropertyName = "URL";
    for ( sal_Int32 aInd = 0; aInd < m_xProperties.getLength(); ++aInd )
    {
        if ( m_xProperties[aInd].Name.equals( aPropertyName ) )
        {
            // the storage is URL based so all the streams are opened by factory and should be closed
            try
            {
                if ( m_xInputStream.is() )
                {
                    m_xInputStream->closeInput();
                    m_xInputStream = uno::Reference< io::XInputStream >();
                }

                if ( m_xStream.is() )
                {
                    uno::Reference< io::XInputStream > xInStr = m_xStream->getInputStream();
                    if ( xInStr.is() )
                        xInStr->closeInput();

                    uno::Reference< io::XOutputStream > xOutStr = m_xStream->getOutputStream();
                    if ( xOutStr.is() )
                        xOutStr->closeOutput();

                    m_xStream = uno::Reference< io::XStream >();
                }
            }
            catch( const uno::Exception& rException )
            {
                AddLog( OSL_LOG_PREFIX "Quiet exception" );
                AddLog( rException.Message );
            }
        }
    }
}

//-----------------------------------------------
void OStorage_Impl::AddLog( const ::rtl::OUString& aMessage )
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
void OStorage_Impl::SetReadOnlyWrap( OStorage& aStorage )
{
    // Weak reference is used inside the holder so the refcount must not be zero at this point
    OSL_ENSURE( aStorage.GetRefCount_Impl(), "There must be a reference alive to use this method!\n" );
    m_aReadOnlyWrapList.push_back( StorageHolder_Impl( &aStorage ) );
}

//-----------------------------------------------
void OStorage_Impl::RemoveReadOnlyWrap( OStorage& aStorage )
{
    for ( OStorageList_Impl::iterator pStorageIter = m_aReadOnlyWrapList.begin();
      pStorageIter != m_aReadOnlyWrapList.end();)
    {
        uno::Reference< embed::XStorage > xTmp = pStorageIter->m_xWeakRef;
        if ( !xTmp.is() || pStorageIter->m_pPointer == &aStorage )
        {
            try {
                pStorageIter->m_pPointer->InternalDispose( sal_False );
            } catch( const uno::Exception& rException )
            {
                AddLog( OSL_LOG_PREFIX "Quiet exception" );
                AddLog( rException.Message );
            }

            OStorageList_Impl::iterator pIterToDelete( pStorageIter );
            ++pStorageIter;
            m_aReadOnlyWrapList.erase( pIterToDelete );
        }
        else
            ++pStorageIter;
    }
}

//-----------------------------------------------
void OStorage_Impl::OpenOwnPackage()
{
    OSL_ENSURE( m_bIsRoot, "Opening of the package has no sence!\n" );

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xPackageFolder.is() )
    {
        if ( !m_xPackage.is() )
        {
            uno::Sequence< uno::Any > aArguments( 2 );
            if ( m_nStorageMode & embed::ElementModes::WRITE )
                aArguments[ 0 ] <<= m_xStream;
            else
            {
                OSL_ENSURE( m_xInputStream.is(), "Input stream must be set for readonly access!\n" );
                aArguments[ 0 ] <<= m_xInputStream;
                // TODO: if input stream is not seekable or XSeekable interface is supported
                // on XStream object a wrapper must be used
            }

            // do not allow elements to remove themself from the old container in case of insertion to another container
            aArguments[ 1 ] <<= beans::NamedValue( "AllowRemoveOnInsert",
                                                    uno::makeAny( (sal_Bool)sal_False ) );

            sal_Int32 nArgNum = 2;
            for ( sal_Int32 aInd = 0; aInd < m_xProperties.getLength(); aInd++ )
            {
                if ( m_xProperties[aInd].Name == "RepairPackage"
                  || m_xProperties[aInd].Name == "ProgressHandler" )
                {
                    beans::NamedValue aNamedValue( m_xProperties[aInd].Name,
                                                    m_xProperties[aInd].Value );
                    aArguments.realloc( ++nArgNum );
                    aArguments[nArgNum-1] <<= aNamedValue;
                }
                else if ( m_xProperties[aInd].Name == "Password" )
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
                aNamedValue.Name = OUString( "StorageFormat" );
                aNamedValue.Value <<= OUString( "OFOPXMLFormat" );
                aArguments.realloc( ++nArgNum );
                aArguments[nArgNum-1] <<= aNamedValue;
            }

            m_xPackage = uno::Reference< lang::XSingleServiceFactory > (
                                        GetServiceFactory()->createInstanceWithArguments(
                                            "com.sun.star.packages.comp.ZipPackage",
                                            aArguments ),
                                        uno::UNO_QUERY );
        }

        uno::Reference< container::XHierarchicalNameAccess > xHNameAccess( m_xPackage, uno::UNO_QUERY );
        OSL_ENSURE( xHNameAccess.is(), "The package could not be created!\n" );

        if ( xHNameAccess.is() )
        {
            uno::Any aFolder = xHNameAccess->getByHierarchicalName("/");
            aFolder >>= m_xPackageFolder;
        }
    }

    OSL_ENSURE( m_xPackageFolder.is(), "The package root folder can not be opened!\n" );
    if ( !m_xPackageFolder.is() )
        throw embed::InvalidStorageException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
}

//-----------------------------------------------
uno::Reference< lang::XMultiServiceFactory > OStorage_Impl::GetServiceFactory()
{
    if ( m_xFactory.is() )
        return m_xFactory;

    return ::comphelper::getProcessServiceFactory();
}

//-----------------------------------------------
SotElementList_Impl& OStorage_Impl::GetChildrenList()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    ReadContents();
    return m_aChildrenList;
}

//-----------------------------------------------
void OStorage_Impl::GetStorageProperties()
{
    if ( m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        uno::Reference< beans::XPropertySet > xProps( m_xPackageFolder, uno::UNO_QUERY_THROW );

        if ( !m_bControlMediaType )
        {
            uno::Reference< beans::XPropertySet > xPackageProps( m_xPackage, uno::UNO_QUERY_THROW );
            xPackageProps->getPropertyValue( MEDIATYPE_FALLBACK_USED_PROPERTY ) >>= m_bMTFallbackUsed;

            xProps->getPropertyValue( "MediaType" ) >>= m_aMediaType;
            m_bControlMediaType = sal_True;
        }

        if ( !m_bControlVersion )
        {
            xProps->getPropertyValue( "Version" ) >>= m_aVersion;
            m_bControlVersion = sal_True;
        }
    }

    // the properties of OFOPXML will be handled directly
}

//-----------------------------------------------
void OStorage_Impl::ReadRelInfoIfNecessary()
{
    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        return;

    if ( m_nRelInfoStatus == RELINFO_NO_INIT )
    {
        // Init from original stream
        uno::Reference< io::XInputStream > xRelInfoStream = GetRelInfoStreamForName( ::rtl::OUString() );
        if ( xRelInfoStream.is() )
            m_aRelInfo = ::comphelper::OFOPXMLHelper::ReadRelationsInfoSequence(
                                    xRelInfoStream,
                                    "_rels/.rels",
                                    comphelper::getComponentContext(m_xFactory) );

        m_nRelInfoStatus = RELINFO_READ;
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
void OStorage_Impl::ReadContents()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bListCreated )
        return;

    if ( m_bIsRoot )
        OpenOwnPackage();

    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xPackageFolder, uno::UNO_QUERY );
    if ( !xEnumAccess.is() )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    uno::Reference< container::XEnumeration > xEnum = xEnumAccess->createEnumeration();
    if ( !xEnum.is() )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    m_bListCreated = sal_True;

    while( xEnum->hasMoreElements() )
    {
        try {
            uno::Reference< container::XNamed > xNamed;
            xEnum->nextElement() >>= xNamed;

            if ( !xNamed.is() )
            {
                OSL_FAIL( "XNamed is not supported!\n" );
                throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
            }

            ::rtl::OUString aName = xNamed->getName();
            OSL_ENSURE( !aName.isEmpty(), "Empty name!\n" );

            uno::Reference< container::XNameContainer > xNameContainer( xNamed, uno::UNO_QUERY );

            SotElement_Impl* pNewElement = new SotElement_Impl( aName, xNameContainer.is(), sal_False );
            if ( m_nStorageType == embed::StorageFormats::OFOPXML && aName == "_rels" )
            {
                if ( !pNewElement->m_bIsStorage )
                    throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: Unexpected format

                m_pRelStorElement = pNewElement;
                CreateRelStorage();
            }
            else
            {
                if ( ( m_nStorageMode & embed::ElementModes::TRUNCATE ) == embed::ElementModes::TRUNCATE )
                {
                    // if a storage is truncated all of it elements are marked as deleted
                    pNewElement->m_bIsRemoved = sal_True;
                }

                m_aChildrenList.push_back( pNewElement );
            }
        }
        catch( const container::NoSuchElementException& rNoSuchElementException )
        {
            AddLog( rNoSuchElementException.Message );
            AddLog( OSL_LOG_PREFIX "NoSuchElement" );

            OSL_FAIL( "hasMoreElements() implementation has problems!\n" );
            break;
        }
    }
    if ( ( m_nStorageMode & embed::ElementModes::TRUNCATE ) == embed::ElementModes::TRUNCATE )
    {
        // if a storage is truncated the relations information should be cleaned
        m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
        m_aRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
        m_nRelInfoStatus = RELINFO_CHANGED;
    }

    // cache changeable folder properties
    GetStorageProperties();
}

//-----------------------------------------------
void OStorage_Impl::CopyToStorage( const uno::Reference< embed::XStorage >& xDest, sal_Bool bDirect )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    uno::Reference< beans::XPropertySet > xPropSet( xDest, uno::UNO_QUERY );
    if ( !xPropSet.is() )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 );

    sal_Int32 nDestMode = embed::ElementModes::READ;
    xPropSet->getPropertyValue( "OpenMode" ) >>= nDestMode;

    if ( !( nDestMode & embed::ElementModes::WRITE ) )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access_denied

    ReadContents();

    if ( !m_xPackageFolder.is() )
        throw embed::InvalidStorageException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    for ( SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
          pElementIter != m_aChildrenList.end(); ++pElementIter )
    {
        if ( !(*pElementIter)->m_bIsRemoved )
            CopyStorageElement( *pElementIter, xDest, (*pElementIter)->m_aName, bDirect );
    }

    // move storage properties to the destination one ( means changeable properties )
    if ( m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        ::rtl::OUString aMediaTypeString = "MediaType";
        ::rtl::OUString aVersionString = "Version";
        xPropSet->setPropertyValue( aMediaTypeString, uno::makeAny( m_aMediaType ) );
        xPropSet->setPropertyValue( aVersionString, uno::makeAny( m_aVersion ) );
    }

    if ( m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        // if this is a root storage, the common key from current one should be moved there
        sal_Bool bIsRoot = sal_False;
        ::rtl::OUString aRootString = "IsRoot";
        if ( ( xPropSet->getPropertyValue( aRootString ) >>= bIsRoot ) && bIsRoot )
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
            catch( const packages::NoEncryptionException& rNoEncryptionException )
            {
                AddLog( rNoEncryptionException.Message );
                AddLog( OSL_LOG_PREFIX "No Encryption" );
            }
        }
    }
    else if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {

        // TODO/LATER: currently the optimization is not active
        // uno::Reference< io::XInputStream > xRelInfoStream = GetRelInfoStreamForName( ::rtl::OUString() ); // own stream
        // if ( xRelInfoStream.is() )
        // {
        //  // Relations info stream is a writeonly property, introduced only to optimyze copying
        //  // Should be used carefuly since no check for stream consistency is done, and the stream must not stay locked
        //
        //  ::rtl::OUString aRelInfoString = "RelationsInfoStream";
        //  xPropSet->setPropertyValue( aRelInfoString, uno::makeAny( GetSeekableTempCopy( xRelInfoStream, m_xFactory ) ) );
        // }

        uno::Reference< embed::XRelationshipAccess > xRels( xDest, uno::UNO_QUERY );
        if ( !xRels.is() )
            throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 );

        xRels->insertRelationships( GetAllRelationshipsIfAny(), sal_False );
    }

    // if possible the destination storage should be commited after successful copying
    uno::Reference< embed::XTransactedObject > xObjToCommit( xDest, uno::UNO_QUERY );
    if ( xObjToCommit.is() )
        xObjToCommit->commit();
}

//-----------------------------------------------
void OStorage_Impl::CopyStorageElement( SotElement_Impl* pElement,
                                        uno::Reference< embed::XStorage > xDest,
                                        ::rtl::OUString aName,
                                        sal_Bool bDirect )
{
    OSL_ENSURE( xDest.is(), "No destination storage!\n" );
    OSL_ENSURE( !aName.isEmpty(), "Empty element name!\n" );

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    uno::Reference< container::XNameAccess > xDestAccess( xDest, uno::UNO_QUERY );
    if ( !xDestAccess.is() )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    if ( xDestAccess->hasByName( aName )
      && !( pElement->m_bIsStorage && xDest->isStorageElement( aName ) ) )
        xDest->removeElement( aName );

    if ( pElement->m_bIsStorage )
    {
        uno::Reference< embed::XStorage > xSubDest =
                                    xDest->openStorageElement(  aName,
                                                                embed::ElementModes::WRITE );

        OSL_ENSURE( xSubDest.is(), "No destination substorage!\n" );

        if ( !pElement->m_pStorage )
        {
            OpenSubStorage( pElement, embed::ElementModes::READ );
            if ( !pElement->m_pStorage )
                throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        }

        pElement->m_pStorage->CopyToStorage( xSubDest, bDirect );
    }
    else
    {
        if ( !pElement->m_pStream )
        {
            OpenSubStream( pElement );
            if ( !pElement->m_pStream )
                throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        }

        if ( !pElement->m_pStream->IsEncrypted() )
        {
            if ( bDirect )
            {
                // fill in the properties for the stream
                uno::Sequence< beans::PropertyValue > aStrProps(0);
                uno::Sequence< beans::PropertyValue > aSrcPkgProps = pElement->m_pStream->GetStreamProperties();
                sal_Int32 nNum = 0;
                for ( int ind = 0; ind < aSrcPkgProps.getLength(); ind++ )
                {
                    if ( aSrcPkgProps[ind].Name == "MediaType" || aSrcPkgProps[ind].Name == "Compressed" )
                    {
                        aStrProps.realloc( ++nNum );
                        aStrProps[nNum-1].Name = aSrcPkgProps[ind].Name;
                        aStrProps[nNum-1].Value = aSrcPkgProps[ind].Value;
                    }
                }

                if ( m_nStorageType == embed::StorageFormats::PACKAGE )
                {
                    aStrProps.realloc( ++nNum );
                    aStrProps[nNum-1].Name = "UseCommonStoragePasswordEncryption";
                    aStrProps[nNum-1].Value <<= (sal_Bool)( pElement->m_pStream->UsesCommonEncryption_Impl() );
                }
                else if ( m_nStorageType == embed::StorageFormats::OFOPXML )
                {
                    // TODO/LATER: currently the optimization is not active
                    // uno::Reference< io::XInputStream > xInStream = GetRelInfoStreamForName( ::rtl::OUString() ); // own rels stream
                    // if ( xInStream.is() )
                    // {
                    //  aStrProps.realloc( ++nNum );
                    //  aStrProps[nNum-1].Name = "RelationsInfoStream";
                    //  aStrProps[nNum-1].Value <<= GetSeekableTempCopy( xInStream, m_xFactory );
                    // }

                    uno::Reference< embed::XRelationshipAccess > xRels( xDest, uno::UNO_QUERY );
                    if ( !xRels.is() )
                        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 0 );

                    xRels->insertRelationships( GetAllRelationshipsIfAny(), sal_False );
                }

                uno::Reference< embed::XOptimizedStorage > xOptDest( xDest, uno::UNO_QUERY_THROW );
                uno::Reference < io::XInputStream > xInputToInsert;

                if ( pElement->m_pStream->HasTempFile_Impl() || !pElement->m_pStream->m_xPackageStream.is() )
                {
                    OSL_ENSURE( pElement->m_pStream->m_xPackageStream.is(), "No package stream!" );

                    // if the stream is modified - the temporary file must be used for insertion
                    xInputToInsert = pElement->m_pStream->GetTempFileAsInputStream();
                }
                else
                {
                    // for now get just nonseekable access to the stream
                    // TODO/LATER: the raw stream can be used

                    xInputToInsert = pElement->m_pStream->m_xPackageStream->getDataStream();
                }

                if ( !xInputToInsert.is() )
                        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

                xOptDest->insertStreamElementDirect( aName, xInputToInsert, aStrProps );
            }
            else
            {
                uno::Reference< io::XStream > xSubStr =
                                            xDest->openStreamElement( aName,
                                            embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
                OSL_ENSURE( xSubStr.is(), "No destination substream!\n" );

                pElement->m_pStream->CopyInternallyTo_Impl( xSubStr );
            }
        }
        else if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        {
            OSL_FAIL( "Encryption is only supported in package storage!\n" );
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        }
        else if ( pElement->m_pStream->HasCachedEncryptionData()
             && ( pElement->m_pStream->IsModified() || pElement->m_pStream->HasWriteOwner_Impl() ) )
        {
            ::comphelper::SequenceAsHashMap aCommonEncryptionData;
            sal_Bool bHasCommonEncryptionData = sal_False;
            try
            {
                aCommonEncryptionData = GetCommonRootEncryptionData();
                bHasCommonEncryptionData = sal_True;
            }
            catch( const packages::NoEncryptionException& rNoEncryptionException )
            {
                AddLog( rNoEncryptionException.Message );
                AddLog( OSL_LOG_PREFIX "No Encryption" );
            }

            if ( bHasCommonEncryptionData && ::package::PackageEncryptionDatasEqual( pElement->m_pStream->GetCachedEncryptionData(), aCommonEncryptionData ) )
            {
                // If the stream can be opened with the common storage password
                // it must be stored with the common storage password as well
                uno::Reference< io::XStream > xDestStream =
                                            xDest->openStreamElement( aName,
                                                embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );

                pElement->m_pStream->CopyInternallyTo_Impl( xDestStream );

                uno::Reference< beans::XPropertySet > xProps( xDestStream, uno::UNO_QUERY_THROW );
                xProps->setPropertyValue(
                    "UseCommonStoragePasswordEncryption",
                    uno::Any( (sal_Bool) sal_True ) );
            }
            else
            {
                // the stream is already opened for writing or was changed
                uno::Reference< embed::XStorage2 > xDest2( xDest, uno::UNO_QUERY_THROW );
                uno::Reference< io::XStream > xSubStr =
                                            xDest2->openEncryptedStream( aName,
                                                embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE,
                                                pElement->m_pStream->GetCachedEncryptionData().getAsConstNamedValueList() );
                OSL_ENSURE( xSubStr.is(), "No destination substream!\n" );

                pElement->m_pStream->CopyInternallyTo_Impl( xSubStr, pElement->m_pStream->GetCachedEncryptionData() );
            }
        }
        else
        {
            // the stream is not opened at all, so it can be just opened for reading
            try
            {
                // If the stream can be opened with the common storage password
                // it must be stored with the common storage password as well

                uno::Reference< io::XStream > xOwnStream = pElement->m_pStream->GetStream( embed::ElementModes::READ,
                                                                                            sal_False );
                uno::Reference< io::XStream > xDestStream =
                                            xDest->openStreamElement( aName,
                                                embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
                OSL_ENSURE( xDestStream.is(), "No destination substream!\n" );
                completeStorageStreamCopy_Impl( xOwnStream, xDestStream, m_nStorageType, GetAllRelationshipsIfAny() );

                uno::Reference< beans::XPropertySet > xProps( xDestStream, uno::UNO_QUERY_THROW );
                xProps->setPropertyValue(
                    "UseCommonStoragePasswordEncryption",
                    uno::Any( (sal_Bool) sal_True ) );
            }
            catch( const packages::WrongPasswordException& rWrongPasswordException )
            {
                AddLog( rWrongPasswordException.Message );
                AddLog( OSL_LOG_PREFIX "Handled exception" );

                // If the common storage password does not allow to open the stream
                // it could be copyed in raw way, the problem is that the StartKey should be the same
                // in the ODF1.2 package, so an invalid package could be produced if the stream
                // is copied from ODF1.1 package, where it is allowed to have different StartKeys
                uno::Reference< embed::XStorageRawAccess > xRawDest( xDest, uno::UNO_QUERY_THROW );
                uno::Reference< io::XInputStream > xRawInStream = pElement->m_pStream->GetRawInStream();
                xRawDest->insertRawEncrStreamElement( aName, xRawInStream );
            }
        }
    }
}

//-----------------------------------------------
uno::Sequence< uno::Sequence< beans::StringPair > > OStorage_Impl::GetAllRelationshipsIfAny()
{
    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        return uno::Sequence< uno::Sequence< beans::StringPair > >();

    ReadRelInfoIfNecessary();

    if ( m_nRelInfoStatus == RELINFO_READ
      || m_nRelInfoStatus == RELINFO_CHANGED_STREAM_READ || m_nRelInfoStatus == RELINFO_CHANGED )
        return m_aRelInfo;
    else // m_nRelInfoStatus == RELINFO_CHANGED_BROKEN || m_nRelInfoStatus == RELINFO_BROKEN
            throw io::IOException( OSL_LOG_PREFIX "Wrong relinfo stream!",
                                    uno::Reference< uno::XInterface >() );
}

//-----------------------------------------------
void OStorage_Impl::CopyLastCommitTo( const uno::Reference< embed::XStorage >& xNewStor )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    OSL_ENSURE( m_xPackageFolder.is(), "A commited storage is incomplete!\n" );
    if ( !m_xPackageFolder.is() )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    OStorage_Impl aTempRepresent( NULL,
                                embed::ElementModes::READ,
                                m_xPackageFolder,
                                m_xPackage,
                                m_xFactory,
                                m_nStorageType);

    // TODO/LATER: could use direct copying
    aTempRepresent.CopyToStorage( xNewStor, sal_False );
}

//-----------------------------------------------
void OStorage_Impl::InsertIntoPackageFolder( const ::rtl::OUString& aName,
                                             const uno::Reference< container::XNameContainer >& xParentPackageFolder )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    OSL_ENSURE( m_xPackageFolder.is(), "An inserted storage is incomplete!\n" );
    uno::Reference< lang::XUnoTunnel > xTunnel( m_xPackageFolder, uno::UNO_QUERY );
    if ( !xTunnel.is() )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    xParentPackageFolder->insertByName( aName, uno::makeAny( xTunnel ) );

    m_bCommited = sal_False;
}

//-----------------------------------------------
void OStorage_Impl::Commit()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_bIsModified )
        return;

    // in case of a new empty storage it is possible that the contents are still not read
    // ( the storage of course has no contents, but the initialization is postponed till the first use,
    //   thus if a new storage was created and commited immediatelly it must be initialized here )
    ReadContents();

    // if storage is commited it should have a valid Package representation
    OSL_ENSURE( m_xPackageFolder.is(), "The package representation should exist!\n" );
    if ( !m_xPackageFolder.is() )
        throw embed::InvalidStorageException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    OSL_ENSURE( m_nStorageMode & embed::ElementModes::WRITE,
                "Commit of readonly storage, should be detected before!\n" );

    uno::Reference< container::XNameContainer > xNewPackageFolder;

    // here the storage will switch to the temporary package folder
    // if the storage was already commited and the parent was not commited after that
    // the switch should not be done since the package folder in use is a temporary one;
    // it can be detected by m_bCommited flag ( root storage doesn't need temporary representation )
    if ( !m_bCommited && !m_bIsRoot )
    {
        uno::Sequence< uno::Any > aSeq( 1 );
        aSeq[0] <<= sal_True;

        xNewPackageFolder = uno::Reference< container::XNameContainer >(
                                                    m_xPackage->createInstanceWithArguments( aSeq ),
                                                    uno::UNO_QUERY );
    }
    else
        xNewPackageFolder = m_xPackageFolder;

    // remove replaced removed elements
    for ( SotElementList_Impl::iterator pDeletedIter = m_aDeletedList.begin();
          pDeletedIter != m_aDeletedList.end();
          ++pDeletedIter )
    {

        if ( m_nStorageType == embed::StorageFormats::OFOPXML && !(*pDeletedIter)->m_bIsStorage )
            RemoveStreamRelInfo( (*pDeletedIter)->m_aOriginalName );

        // the removed elements are not in new temporary storage
        if ( m_bCommited || m_bIsRoot )
            xNewPackageFolder->removeByName( (*pDeletedIter)->m_aOriginalName );
        delete *pDeletedIter;
        *pDeletedIter = NULL;
    }
    m_aDeletedList.clear();

    // remove removed elements
    SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
    while (  pElementIter != m_aChildrenList.end() )
    {
        // renamed and inserted elements must be really inserted to package later
        // since thay can conflict with removed elements

        if ( (*pElementIter)->m_bIsRemoved )
        {
            if ( m_nStorageType == embed::StorageFormats::OFOPXML && !(*pElementIter)->m_bIsStorage )
                RemoveStreamRelInfo( (*pElementIter)->m_aOriginalName );

            // the removed elements are not in new temporary storage
            if ( m_bCommited || m_bIsRoot )
                xNewPackageFolder->removeByName( (*pElementIter)->m_aOriginalName );

            SotElement_Impl* pToDelete = *pElementIter;

            ++pElementIter; // to let the iterator be valid it should be increased before removing

            m_aChildrenList.remove( pToDelete );
            delete pToDelete;
        }
        else
            ++pElementIter;
    }

    // there should be no more deleted elements
    for ( pElementIter = m_aChildrenList.begin(); pElementIter != m_aChildrenList.end(); ++pElementIter )
    {
        // if it is a 'duplicate commit' inserted elements must be really inserted to package later
        // since thay can conflict with renamed elements

        if ( !(*pElementIter)->m_bIsInserted )
        {
            // for now stream is opened in direct mode that means that in case
            // storage is commited all the streams from it are commited in current state.
            // following two steps are separated to allow easily implement transacted mode
            // for streams if we need it in future.
            // Only hierarchical access uses transacted streams currently
            if ( !(*pElementIter)->m_bIsStorage && (*pElementIter)->m_pStream
              && !(*pElementIter)->m_pStream->IsTransacted() )
                (*pElementIter)->m_pStream->Commit();

            // if the storage was not open, there is no need to commit it ???
            // the storage should be checked that it is commited
            if ( (*pElementIter)->m_bIsStorage && (*pElementIter)->m_pStorage && (*pElementIter)->m_pStorage->m_bCommited )
            {
                // it's temporary PackageFolder should be inserted instead of current one
                // also the new copy of PackageFolder should be used by the children storages

                // the renamed elements are not in new temporary storage
                if ( m_bCommited || m_bIsRoot )
                    xNewPackageFolder->removeByName( (*pElementIter)->m_aOriginalName );

                (*pElementIter)->m_pStorage->InsertIntoPackageFolder( (*pElementIter)->m_aName, xNewPackageFolder );
            }
            else if ( !(*pElementIter)->m_bIsStorage && (*pElementIter)->m_pStream && (*pElementIter)->m_pStream->m_bFlushed )
            {
                if ( m_nStorageType == embed::StorageFormats::OFOPXML )
                    CommitStreamRelInfo( *pElementIter );

                // the renamed elements are not in new temporary storage
                if ( m_bCommited || m_bIsRoot )
                    xNewPackageFolder->removeByName( (*pElementIter)->m_aOriginalName );

                (*pElementIter)->m_pStream->InsertIntoPackageFolder( (*pElementIter)->m_aName, xNewPackageFolder );
            }
            else if ( !m_bCommited && !m_bIsRoot )
            {
                // the element must be just copied to the new temporary package folder
                // the connection with the original package should not be lost just because
                // the element is still refered by the folder in the original hierarchy
                uno::Any aPackageElement = m_xPackageFolder->getByName( (*pElementIter)->m_aOriginalName );
                xNewPackageFolder->insertByName( (*pElementIter)->m_aName, aPackageElement );
            }
            else if ( (*pElementIter)->m_aName.compareTo( (*pElementIter)->m_aOriginalName ) )
            {
                // this is the case when xNewPackageFolder refers to m_xPackageFolder
                // in case the name was changed and it is not a changed storage - rename the element
                uno::Reference< container::XNamed > xNamed;
                uno::Any aPackageElement = xNewPackageFolder->getByName( (*pElementIter)->m_aOriginalName );
                xNewPackageFolder->removeByName( (*pElementIter)->m_aOriginalName );
                xNewPackageFolder->insertByName( (*pElementIter)->m_aName, aPackageElement );

                if ( m_nStorageType == embed::StorageFormats::OFOPXML && !(*pElementIter)->m_bIsStorage )
                {
                    if ( !(*pElementIter)->m_pStream )
                    {
                        OpenSubStream( *pElementIter );
                        if ( !(*pElementIter)->m_pStream )
                            throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
                    }

                    CommitStreamRelInfo( *pElementIter );
                }
            }

            (*pElementIter)->m_aOriginalName = (*pElementIter)->m_aName;
        }
    }

    for ( pElementIter = m_aChildrenList.begin(); pElementIter != m_aChildrenList.end(); ++pElementIter )
    {
        // now inserted elements can be inserted to the package
        if ( (*pElementIter)->m_bIsInserted )
        {
            (*pElementIter)->m_aOriginalName = (*pElementIter)->m_aName;
            uno::Reference< lang::XUnoTunnel > xNewElement;

            if ( (*pElementIter)->m_bIsStorage )
            {
                if ( (*pElementIter)->m_pStorage->m_bCommited )
                {
                    OSL_ENSURE( (*pElementIter)->m_pStorage, "An inserted storage is incomplete!\n" );
                    if ( !(*pElementIter)->m_pStorage )
                        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

                    (*pElementIter)->m_pStorage->InsertIntoPackageFolder( (*pElementIter)->m_aName, xNewPackageFolder );

                    (*pElementIter)->m_bIsInserted = sal_False;
                }
            }
            else
            {
                OSL_ENSURE( (*pElementIter)->m_pStream, "An inserted stream is incomplete!\n" );
                if ( !(*pElementIter)->m_pStream )
                    throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

                if ( !(*pElementIter)->m_pStream->IsTransacted() )
                    (*pElementIter)->m_pStream->Commit();

                if ( (*pElementIter)->m_pStream->m_bFlushed )
                {
                    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
                        CommitStreamRelInfo( *pElementIter );

                    (*pElementIter)->m_pStream->InsertIntoPackageFolder( (*pElementIter)->m_aName, xNewPackageFolder );

                    (*pElementIter)->m_bIsInserted = sal_False;
                }
            }
        }
    }

    if ( m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        // move properties to the destination package folder
        uno::Reference< beans::XPropertySet > xProps( xNewPackageFolder, uno::UNO_QUERY );
        if ( !xProps.is() )
            throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        xProps->setPropertyValue( "MediaType", uno::makeAny( m_aMediaType ) );
        xProps->setPropertyValue( "Version", uno::makeAny( m_aVersion ) );
    }

    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
        CommitRelInfo( xNewPackageFolder ); // store own relations and commit complete relations storage

    if ( m_bIsRoot )
    {
        uno::Reference< util::XChangesBatch > xChangesBatch( m_xPackage, uno::UNO_QUERY );

        OSL_ENSURE( xChangesBatch.is(), "Impossible to commit package!\n" );
        if ( !xChangesBatch.is() )
            throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        try
        {
            xChangesBatch->commitChanges();
        }
        catch( const lang::WrappedTargetException& r )
        {
            // the wrapped UseBackupException means that the target medium can be corrupted
            embed::UseBackupException aException;
            if ( r.TargetException >>= aException )
            {
                m_xStream = uno::Reference< io::XStream >();
                m_xInputStream = uno::Reference< io::XInputStream >();
                throw aException;
            }

            AddLog( aException.Message );
            AddLog( OSL_LOG_PREFIX "Rethrow" );
            throw;
        }
    }
    else if ( !m_bCommited )
    {
        m_xPackageFolder = xNewPackageFolder;
        m_bCommited = sal_True;
    }

    // after commit the mediatype treated as the correct one
    m_bMTFallbackUsed = sal_False;
}

//-----------------------------------------------
void OStorage_Impl::Revert()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !( m_nStorageMode & embed::ElementModes::WRITE ) )
        return; // nothing to do

    // all the children must be removed
    // they will be created later on demand

    SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
    while (  pElementIter != m_aChildrenList.end() )
    {
        if ( (*pElementIter)->m_bIsInserted )
        {
            SotElement_Impl* pToDelete = *pElementIter;

            ++pElementIter; // to let the iterator be valid it should be increased before removing

            m_aChildrenList.remove( pToDelete );
            delete pToDelete;
        }
        else
        {
            ClearElement( *pElementIter );

            (*pElementIter)->m_aName = (*pElementIter)->m_aOriginalName;
            (*pElementIter)->m_bIsRemoved = sal_False;

            ++pElementIter;
        }
    }

    // return replaced removed elements
    for ( SotElementList_Impl::iterator pDeletedIter = m_aDeletedList.begin();
          pDeletedIter != m_aDeletedList.end();
          ++pDeletedIter )
    {
        m_aChildrenList.push_back( (*pDeletedIter) );

        ClearElement( *pDeletedIter );

        (*pDeletedIter)->m_aName = (*pDeletedIter)->m_aOriginalName;
        (*pDeletedIter)->m_bIsRemoved = sal_False;
    }
    m_aDeletedList.clear();

    m_bControlMediaType = sal_False;
    m_bControlVersion = sal_False;

    GetStorageProperties();

    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        // currently the relations storage is changed only on commit
        m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
        m_aRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
        m_nRelInfoStatus = RELINFO_NO_INIT;
    }
}

//-----------------------------------------------
::comphelper::SequenceAsHashMap OStorage_Impl::GetCommonRootEncryptionData()
    throw ( packages::NoEncryptionException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        throw packages::NoEncryptionException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    if ( m_bIsRoot )
    {
        if ( !m_bHasCommonEncryptionData )
            throw packages::NoEncryptionException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        return m_aCommonEncryptionData;
    }
    else
    {
        if ( !m_pParent )
            throw packages::NoEncryptionException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        return m_pParent->GetCommonRootEncryptionData();
    }
}

//-----------------------------------------------
SotElement_Impl* OStorage_Impl::FindElement( const ::rtl::OUString& rName )
{
    OSL_ENSURE( !rName.isEmpty(), "Name is empty!" );

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    ReadContents();

    for ( SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
          pElementIter != m_aChildrenList.end(); ++pElementIter )
    {
        if ( (*pElementIter)->m_aName == rName && !(*pElementIter)->m_bIsRemoved )
            return *pElementIter;
    }

    return NULL;
}

//-----------------------------------------------
SotElement_Impl* OStorage_Impl::InsertStream( ::rtl::OUString aName, sal_Bool bEncr )
{
    OSL_ENSURE( m_xPackage.is(), "Not possible to refer to package as to factory!\n" );
    if ( !m_xPackage.is() )
        throw embed::InvalidStorageException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    uno::Sequence< uno::Any > aSeq( 1 );
    aSeq[0] <<= sal_False;
    uno::Reference< lang::XUnoTunnel > xNewElement( m_xPackage->createInstanceWithArguments( aSeq ),
                                                    uno::UNO_QUERY );

    OSL_ENSURE( xNewElement.is(), "Not possible to create a new stream!\n" );
    if ( !xNewElement.is() )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    uno::Reference< packages::XDataSinkEncrSupport > xPackageSubStream( xNewElement, uno::UNO_QUERY );
    if ( !xPackageSubStream.is() )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    OSL_ENSURE( m_nStorageType == embed::StorageFormats::PACKAGE || !bEncr, "Only package storage supports encryption!\n" );
    if ( m_nStorageType != embed::StorageFormats::PACKAGE && bEncr )
        throw packages::NoEncryptionException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    // the mode is not needed for storage stream internal implementation
    SotElement_Impl* pNewElement = InsertElement( aName, sal_False );
    pNewElement->m_pStream = new OWriteStream_Impl( this, xPackageSubStream, m_xPackage, m_xFactory, bEncr, m_nStorageType, sal_True );

    m_aChildrenList.push_back( pNewElement );
    m_bIsModified = sal_True;
    m_bBroadcastModified = sal_True;

    return pNewElement;
}

//-----------------------------------------------
SotElement_Impl* OStorage_Impl::InsertRawStream( ::rtl::OUString aName, const uno::Reference< io::XInputStream >& xInStream )
{
    // insert of raw stream means insert and commit
    OSL_ENSURE( m_xPackage.is(), "Not possible to refer to package as to factory!\n" );
    if ( !m_xPackage.is() )
        throw embed::InvalidStorageException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    if ( m_nStorageType != embed::StorageFormats::PACKAGE )
        throw packages::NoEncryptionException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    uno::Reference< io::XSeekable > xSeek( xInStream, uno::UNO_QUERY );
    uno::Reference< io::XInputStream > xInStrToInsert = xSeek.is() ? xInStream :
                                                                     GetSeekableTempCopy( xInStream, GetServiceFactory() );

    uno::Sequence< uno::Any > aSeq( 1 );
    aSeq[0] <<= sal_False;
    uno::Reference< lang::XUnoTunnel > xNewElement( m_xPackage->createInstanceWithArguments( aSeq ),
                                                    uno::UNO_QUERY );

    OSL_ENSURE( xNewElement.is(), "Not possible to create a new stream!\n" );
    if ( !xNewElement.is() )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    uno::Reference< packages::XDataSinkEncrSupport > xPackageSubStream( xNewElement, uno::UNO_QUERY );
    if ( !xPackageSubStream.is() )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    xPackageSubStream->setRawStream( xInStrToInsert );

    // the mode is not needed for storage stream internal implementation
    SotElement_Impl* pNewElement = InsertElement( aName, sal_False );
    pNewElement->m_pStream = new OWriteStream_Impl( this, xPackageSubStream, m_xPackage, m_xFactory, sal_True, m_nStorageType, sal_False );
    // the stream is inserted and must be treated as a commited one
    pNewElement->m_pStream->SetToBeCommited();

    m_aChildrenList.push_back( pNewElement );
    m_bIsModified = sal_True;
    m_bBroadcastModified = sal_True;

    return pNewElement;
}

//-----------------------------------------------
OStorage_Impl* OStorage_Impl::CreateNewStorageImpl( sal_Int32 nStorageMode )
{
    OSL_ENSURE( m_xPackage.is(), "Not possible to refer to package as to factory!\n" );
    if ( !m_xPackage.is() )
        throw embed::InvalidStorageException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    uno::Sequence< uno::Any > aSeq( 1 );
    aSeq[0] <<= sal_True;
    uno::Reference< lang::XUnoTunnel > xNewElement( m_xPackage->createInstanceWithArguments( aSeq ),
                                                    uno::UNO_QUERY );

    OSL_ENSURE( xNewElement.is(), "Not possible to create a new storage!\n" );
    if ( !xNewElement.is() )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    uno::Reference< container::XNameContainer > xPackageSubFolder( xNewElement, uno::UNO_QUERY );
    if ( !xPackageSubFolder.is() )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    OStorage_Impl* pResult =
            new OStorage_Impl( this, nStorageMode, xPackageSubFolder, m_xPackage, m_xFactory, m_nStorageType );
    pResult->m_bIsModified = sal_True;

    return pResult;
}

//-----------------------------------------------
SotElement_Impl* OStorage_Impl::InsertStorage( ::rtl::OUString aName, sal_Int32 nStorageMode )
{
    SotElement_Impl* pNewElement = InsertElement( aName, sal_True );

    pNewElement->m_pStorage = CreateNewStorageImpl( nStorageMode );

    m_aChildrenList.push_back( pNewElement );

    return pNewElement;
}

//-----------------------------------------------
SotElement_Impl* OStorage_Impl::InsertElement( ::rtl::OUString aName, sal_Bool bIsStorage )
{
    OSL_ENSURE( FindElement( aName ) == NULL, "Should not try to insert existing element" );

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    SotElement_Impl* pDeletedElm = NULL;

    for ( SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
          pElementIter != m_aChildrenList.end(); ++pElementIter )
    {
        if ( (*pElementIter)->m_aName == aName )
        {
            OSL_ENSURE( (*pElementIter)->m_bIsRemoved, "Try to insert an element instead of existing one!\n" );
            if ( (*pElementIter)->m_bIsRemoved )
            {
                OSL_ENSURE( !(*pElementIter)->m_bIsInserted, "Inserted elements must be deleted immediatelly!\n" );
                pDeletedElm = *pElementIter;
                break;
            }
        }
    }

    if ( pDeletedElm )
    {
        if ( pDeletedElm->m_bIsStorage )
            OpenSubStorage( pDeletedElm, embed::ElementModes::READWRITE );
        else
            OpenSubStream( pDeletedElm );

        m_aChildrenList.remove( pDeletedElm );  // correct usage of list ???
        m_aDeletedList.push_back( pDeletedElm );
    }

    // create new element
    return new SotElement_Impl( aName, bIsStorage, sal_True );
}

//-----------------------------------------------
void OStorage_Impl::OpenSubStorage( SotElement_Impl* pElement, sal_Int32 nStorageMode )
{
    OSL_ENSURE( pElement, "pElement is not set!\n" );
    OSL_ENSURE( pElement->m_bIsStorage, "Storage flag is not set!\n" );

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !pElement->m_pStorage )
    {
        OSL_ENSURE( !pElement->m_bIsInserted, "Inserted element must be created already!\n" );

        uno::Reference< lang::XUnoTunnel > xTunnel;
        m_xPackageFolder->getByName( pElement->m_aOriginalName ) >>= xTunnel;
        if ( !xTunnel.is() )
            throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        uno::Reference< container::XNameContainer > xPackageSubFolder( xTunnel, uno::UNO_QUERY );

        OSL_ENSURE( xPackageSubFolder.is(), "Can not get XNameContainer interface from folder!\n" );

        if ( !xPackageSubFolder.is() )
            throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        pElement->m_pStorage = new OStorage_Impl( this, nStorageMode, xPackageSubFolder, m_xPackage, m_xFactory, m_nStorageType );
    }
}

//-----------------------------------------------
void OStorage_Impl::OpenSubStream( SotElement_Impl* pElement )
{
    OSL_ENSURE( pElement, "pElement is not set!\n" );
    OSL_ENSURE( !pElement->m_bIsStorage, "Storage flag is set!\n" );

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !pElement->m_pStream )
    {
        OSL_ENSURE( !pElement->m_bIsInserted, "Inserted element must be created already!\n" );

        uno::Reference< lang::XUnoTunnel > xTunnel;
        m_xPackageFolder->getByName( pElement->m_aOriginalName ) >>= xTunnel;
        if ( !xTunnel.is() )
            throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        uno::Reference< packages::XDataSinkEncrSupport > xPackageSubStream( xTunnel, uno::UNO_QUERY );
        if ( !xPackageSubStream.is() )
            throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        // the stream can never be inserted here, because inserted stream element holds the stream till commit or destruction
        pElement->m_pStream = new OWriteStream_Impl( this, xPackageSubStream, m_xPackage, m_xFactory, sal_False, m_nStorageType, sal_False, GetRelInfoStreamForName( pElement->m_aOriginalName ) );
    }
}

//-----------------------------------------------
uno::Sequence< ::rtl::OUString > OStorage_Impl::GetElementNames()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    ReadContents();

    sal_uInt32 nSize = m_aChildrenList.size();
    uno::Sequence< ::rtl::OUString > aElementNames( nSize );

    sal_uInt32 nInd = 0;
    for ( SotElementList_Impl::iterator pElementIter = m_aChildrenList.begin();
          pElementIter != m_aChildrenList.end(); ++pElementIter )
    {
        if ( !(*pElementIter)->m_bIsRemoved )
            aElementNames[nInd++] = (*pElementIter)->m_aName;
    }

    aElementNames.realloc( nInd );
    return aElementNames;
}

//-----------------------------------------------
void OStorage_Impl::RemoveElement( SotElement_Impl* pElement )
{
    OSL_ENSURE( pElement, "Element must be provided!" );

    if ( !pElement )
        return;

    if ( (pElement->m_pStorage && ( pElement->m_pStorage->m_pAntiImpl || !pElement->m_pStorage->m_aReadOnlyWrapList.empty() ))
      || (pElement->m_pStream && ( pElement->m_pStream->m_pAntiImpl || !pElement->m_pStream->m_aInputStreamsList.empty() )) )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: Access denied

    if ( pElement->m_bIsInserted )
    {
        m_aChildrenList.remove( pElement );
        delete pElement; // ???
    }
    else
    {
        pElement->m_bIsRemoved = sal_True;
        ClearElement( pElement );
    }

    // TODO/OFOPXML: the rel stream should be removed as well
}

//-----------------------------------------------
void OStorage_Impl::ClearElement( SotElement_Impl* pElement )
{
    if ( pElement->m_pStorage )
    {
        delete pElement->m_pStorage;
        pElement->m_pStorage = NULL;
    }

    if ( pElement->m_pStream )
    {
        delete pElement->m_pStream;
        pElement->m_pStream = NULL;
    }
}

//-----------------------------------------------
void OStorage_Impl::CloneStreamElement( const ::rtl::OUString& aStreamName,
                                        sal_Bool bEncryptionDataProvided,
                                        const ::comphelper::SequenceAsHashMap& aEncryptionData,
                                        uno::Reference< io::XStream >& xTargetStream )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    SotElement_Impl *pElement = FindElement( aStreamName );
    if ( !pElement )
    {
        // element does not exist, throw exception
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access_denied
    }
    else if ( pElement->m_bIsStorage )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    if ( !pElement->m_pStream )
        OpenSubStream( pElement );

    if ( pElement->m_pStream && pElement->m_pStream->m_xPackageStream.is() )
    {
        // the existence of m_pAntiImpl of the child is not interesting,
        // the copy will be created internally

        // usual copying is not applicable here, only last flushed version of the
        // child stream should be used for copiing. Probably the children m_xPackageStream
        // can be used as a base of a new stream, that would be copied to result
        // storage. The only problem is that some package streams can be accessed from outside
        // at the same time ( now solwed by wrappers that remember own position ).

        if ( bEncryptionDataProvided )
            pElement->m_pStream->GetCopyOfLastCommit( xTargetStream, aEncryptionData );
        else
            pElement->m_pStream->GetCopyOfLastCommit( xTargetStream );
    }
    else
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: general_error
}

//-----------------------------------------------
void OStorage_Impl::RemoveStreamRelInfo( const ::rtl::OUString& aOriginalName )
{
    // this method should be used only in OStorage_Impl::Commit() method
    // the aOriginalName can be empty, in this case the storage relation info should be removed

    if ( m_nStorageType == embed::StorageFormats::OFOPXML && m_xRelStorage.is() )
    {
        ::rtl::OUString aRelStreamName = aOriginalName;
        aRelStreamName += ".rels";

        if ( m_xRelStorage->hasByName( aRelStreamName ) )
            m_xRelStorage->removeElement( aRelStreamName );
    }
}

//-----------------------------------------------
void OStorage_Impl::CreateRelStorage()
{
    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        return;

    if ( !m_xRelStorage.is() )
    {
        if ( !m_pRelStorElement )
        {
            m_pRelStorElement = new SotElement_Impl( "_rels", sal_True, sal_True );
            m_pRelStorElement->m_pStorage = CreateNewStorageImpl( embed::ElementModes::WRITE );
            if ( m_pRelStorElement->m_pStorage )
                m_pRelStorElement->m_pStorage->m_pParent = NULL; // the relation storage is completely controlled by parent
        }

        if ( !m_pRelStorElement->m_pStorage )
            OpenSubStorage( m_pRelStorElement, embed::ElementModes::WRITE );

        if ( !m_pRelStorElement->m_pStorage )
            throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        OStorage* pResultStorage = new OStorage( m_pRelStorElement->m_pStorage, sal_False );
        m_xRelStorage = uno::Reference< embed::XStorage >( (embed::XStorage*) pResultStorage );
    }
}

//-----------------------------------------------
void OStorage_Impl::CommitStreamRelInfo( SotElement_Impl* pStreamElement )
{
    // this method should be used only in OStorage_Impl::Commit() method

    // the stream element must be provided
    if ( !pStreamElement )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    if ( m_nStorageType == embed::StorageFormats::OFOPXML && pStreamElement->m_pStream )
    {
        OSL_ENSURE( !pStreamElement->m_aName.isEmpty(), "The name must not be empty!\n" );

        if ( !m_xRelStorage.is() )
        {
            // Create new rels storage, this is commit scenario so it must be possible
            CreateRelStorage();
        }

        pStreamElement->m_pStream->CommitStreamRelInfo( m_xRelStorage, pStreamElement->m_aOriginalName, pStreamElement->m_aName );
    }
}

//-----------------------------------------------
uno::Reference< io::XInputStream > OStorage_Impl::GetRelInfoStreamForName( const ::rtl::OUString& aName )
{
    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        ReadContents();
        if ( m_xRelStorage.is() )
        {
            ::rtl::OUString aRelStreamName = aName;
            aRelStreamName += ".rels";
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

//-----------------------------------------------
void OStorage_Impl::CommitRelInfo( const uno::Reference< container::XNameContainer >& xNewPackageFolder )
{
    // this method should be used only in OStorage_Impl::Commit() method
    ::rtl::OUString aRelsStorName("_rels");

    if ( !xNewPackageFolder.is() )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        if ( m_nRelInfoStatus == RELINFO_BROKEN || m_nRelInfoStatus == RELINFO_CHANGED_BROKEN )
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        if ( m_nRelInfoStatus == RELINFO_CHANGED
          || m_nRelInfoStatus == RELINFO_CHANGED_STREAM_READ
          || m_nRelInfoStatus == RELINFO_CHANGED_STREAM )
        {
            if ( m_nRelInfoStatus == RELINFO_CHANGED )
            {
                if ( m_aRelInfo.getLength() )
                {
                    CreateRelStorage();

                    uno::Reference< io::XStream > xRelsStream =
                        m_xRelStorage->openStreamElement( ".rels" ,
                                                            embed::ElementModes::TRUNCATE | embed::ElementModes::READWRITE );

                    uno::Reference< io::XOutputStream > xOutStream = xRelsStream->getOutputStream();
                    if ( !xOutStream.is() )
                        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

                    ::comphelper::OFOPXMLHelper::WriteRelationsInfoSequence( xOutStream, m_aRelInfo, comphelper::getComponentContext(m_xFactory) );

                    // set the mediatype
                    uno::Reference< beans::XPropertySet > xPropSet( xRelsStream, uno::UNO_QUERY_THROW );
                    xPropSet->setPropertyValue(
                        "MediaType",
                        uno::makeAny( OUString( "application/vnd.openxmlformats-package.relationships+xml" ) ) );

                    m_nRelInfoStatus = RELINFO_READ;
                }
                else if ( m_xRelStorage.is() )
                    RemoveStreamRelInfo( ::rtl::OUString() ); // remove own rel info
            }
            else if ( m_nRelInfoStatus == RELINFO_CHANGED_STREAM_READ
                      || m_nRelInfoStatus == RELINFO_CHANGED_STREAM )
            {
                CreateRelStorage();

                uno::Reference< io::XStream > xRelsStream =
                    m_xRelStorage->openStreamElement( ".rels",
                                                        embed::ElementModes::TRUNCATE | embed::ElementModes::READWRITE );

                uno::Reference< io::XOutputStream > xOutputStream = xRelsStream->getOutputStream();
                if ( !xOutputStream.is() )
                    throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

                uno::Reference< io::XSeekable > xSeek( m_xNewRelInfoStream, uno::UNO_QUERY_THROW );
                xSeek->seek( 0 );
                ::comphelper::OStorageHelper::CopyInputToOutput( m_xNewRelInfoStream, xOutputStream );

                // set the mediatype
                uno::Reference< beans::XPropertySet > xPropSet( xRelsStream, uno::UNO_QUERY_THROW );
                xPropSet->setPropertyValue(
                    "MediaType",
                    uno::makeAny( OUString( "application/vnd.openxmlformats-package.relationships+xml" ) ) );

                m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
                  if ( m_nRelInfoStatus == RELINFO_CHANGED_STREAM )
                {
                    m_aRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
                    m_nRelInfoStatus = RELINFO_NO_INIT;
                }
                else
                    m_nRelInfoStatus = RELINFO_READ;
            }
        }

        if ( m_xRelStorage.is() )
        {
            if ( m_xRelStorage->hasElements() )
            {
                uno::Reference< embed::XTransactedObject > xTrans( m_xRelStorage, uno::UNO_QUERY_THROW );
                if ( xTrans.is() )
                    xTrans->commit();
            }

            if ( xNewPackageFolder.is() && xNewPackageFolder->hasByName( aRelsStorName ) )
                xNewPackageFolder->removeByName( aRelsStorName );

            if ( !m_xRelStorage->hasElements() )
            {
                // the empty relations storage should not be created
                delete m_pRelStorElement;
                m_pRelStorElement = NULL;
                m_xRelStorage = uno::Reference< embed::XStorage >();
            }
            else if ( m_pRelStorElement && m_pRelStorElement->m_pStorage && xNewPackageFolder.is() )
                m_pRelStorElement->m_pStorage->InsertIntoPackageFolder( aRelsStorName, xNewPackageFolder );
        }
    }
}

//=====================================================
// OStorage implementation
//=====================================================

//-----------------------------------------------
OStorage::OStorage( uno::Reference< io::XInputStream > xInputStream,
                    sal_Int32 nMode,
                    uno::Sequence< beans::PropertyValue > xProperties,
                    uno::Reference< lang::XMultiServiceFactory > xFactory,
                    sal_Int32 nStorageType )
: m_pImpl( new OStorage_Impl( xInputStream, nMode, xProperties, xFactory, nStorageType ) )
{
    m_pImpl->m_pAntiImpl = this;
    m_pData = new StorInternalData_Impl( m_pImpl->m_rMutexRef, m_pImpl->m_bIsRoot, m_pImpl->m_nStorageType, sal_False );
}

//-----------------------------------------------
OStorage::OStorage( uno::Reference< io::XStream > xStream,
                    sal_Int32 nMode,
                    uno::Sequence< beans::PropertyValue > xProperties,
                    uno::Reference< lang::XMultiServiceFactory > xFactory,
                    sal_Int32 nStorageType )
: m_pImpl( new OStorage_Impl( xStream, nMode, xProperties, xFactory, nStorageType ) )
{
    m_pImpl->m_pAntiImpl = this;
    m_pData = new StorInternalData_Impl( m_pImpl->m_rMutexRef, m_pImpl->m_bIsRoot, m_pImpl->m_nStorageType, sal_False );
}

//-----------------------------------------------
OStorage::OStorage( OStorage_Impl* pImpl, sal_Bool bReadOnlyWrap )
: m_pImpl( pImpl )
{
    // this call can be done only from OStorage_Impl implementation to create child storage
    OSL_ENSURE( m_pImpl && m_pImpl->m_rMutexRef.Is(), "The provided pointer & mutex MUST NOT be empty!\n" );

    m_pData = new StorInternalData_Impl( m_pImpl->m_rMutexRef, m_pImpl->m_bIsRoot, m_pImpl->m_nStorageType, bReadOnlyWrap );

    OSL_ENSURE( ( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) == embed::ElementModes::WRITE ||
                    m_pData->m_bReadOnlyWrap,
                "The wrapper can not allow writing in case implementation does not!\n" );

    if ( !bReadOnlyWrap )
        m_pImpl->m_pAntiImpl = this;
}

//-----------------------------------------------
OStorage::~OStorage()
{
    {
        ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );
        if ( m_pImpl )
        {
            m_refCount++; // to call dispose
            try {
                dispose();
            }
            catch( const uno::RuntimeException& rRuntimeException )
            {
                m_pImpl->AddLog( rRuntimeException.Message );
                m_pImpl->AddLog( OSL_LOG_PREFIX "Handled exception" );
            }
        }
    }

    if ( m_pData )
    {
        if ( m_pData->m_pSubElDispListener )
        {
            m_pData->m_pSubElDispListener->release();
            m_pData->m_pSubElDispListener = NULL;
        }

        if ( m_pData->m_pTypeCollection )
        {
            delete m_pData->m_pTypeCollection;
            m_pData->m_pTypeCollection = NULL;
        }

        delete m_pData;
    }
}

//-----------------------------------------------
void SAL_CALL OStorage::InternalDispose( sal_Bool bNotifyImpl )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::InternalDispose" );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    // the source object is also a kind of locker for the current object
    // since the listeners could dispose the object while being notified
       lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
    m_pData->m_aListenersContainer.disposeAndClear( aSource );

    if ( m_pData->m_bReadOnlyWrap )
    {
        OSL_ENSURE( !m_pData->m_aOpenSubComponentsList.size() || m_pData->m_pSubElDispListener,
                    "If any subelements are open the listener must exist!\n" );

        if ( m_pData->m_pSubElDispListener )
        {
            m_pData->m_pSubElDispListener->OwnerIsDisposed();

            // iterate through m_pData->m_aOpenSubComponentsList
            // deregister m_pData->m_pSubElDispListener and dispose all of them
            if ( !m_pData->m_aOpenSubComponentsList.empty() )
            {
                for ( WeakComponentList::iterator pCompIter = m_pData->m_aOpenSubComponentsList.begin();
                      pCompIter != m_pData->m_aOpenSubComponentsList.end(); ++pCompIter )
                {
                    uno::Reference< lang::XComponent > xTmp = (*pCompIter);
                    if ( xTmp.is() )
                    {
                        xTmp->removeEventListener( uno::Reference< lang::XEventListener >(
                                    static_cast< lang::XEventListener* >( m_pData->m_pSubElDispListener ) ) );

                        try {
                            xTmp->dispose();
                        } catch( const uno::Exception& rException )
                        {
                            m_pImpl->AddLog( rException.Message );
                            m_pImpl->AddLog( OSL_LOG_PREFIX "Quiet exception" );
                        }
                    }
                }

                m_pData->m_aOpenSubComponentsList.clear();
            }
        }

        if ( bNotifyImpl )
            m_pImpl->RemoveReadOnlyWrap( *this );
    }
    else
    {
        m_pImpl->m_pAntiImpl = NULL;

        if ( bNotifyImpl )
        {
            if ( m_pData->m_bIsRoot )
                delete m_pImpl;
            else
            {
                // the noncommited changes for the storage must be removed
                m_pImpl->Revert();
            }
        }
    }

    m_pImpl = NULL;
}

//-----------------------------------------------
void OStorage::ChildIsDisposed( const uno::Reference< uno::XInterface >& xChild )
{
    // this method can only be called by child disposing listener

    // this method must not contain any locking
    // the locking is done in the listener

    if ( !m_pData->m_aOpenSubComponentsList.empty() )
    {
        for ( WeakComponentList::iterator pCompIter = m_pData->m_aOpenSubComponentsList.begin();
              pCompIter != m_pData->m_aOpenSubComponentsList.end(); )
        {
            uno::Reference< lang::XComponent > xTmp = (*pCompIter);
            if ( !xTmp.is() || xTmp == xChild )
            {
                WeakComponentList::iterator pIterToRemove = pCompIter;
                ++pCompIter;
                m_pData->m_aOpenSubComponentsList.erase( pIterToRemove );
            }
            else
                ++pCompIter;
        }
    }
}

//-----------------------------------------------
void OStorage::BroadcastModifiedIfNecessary()
{
    // no need to lock mutex here for the checking of m_pImpl, and m_pData is alive until the object is destructed
    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( !m_pImpl->m_bBroadcastModified )
        return;

    m_pImpl->m_bBroadcastModified = sal_False;

    OSL_ENSURE( !m_pData->m_bReadOnlyWrap, "The storage can not be modified at all!\n" );

       lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );

       ::cppu::OInterfaceContainerHelper* pContainer =
            m_pData->m_aListenersContainer.getContainer(
                ::getCppuType( ( const uno::Reference< util::XModifyListener >*) NULL ) );
       if ( pContainer )
    {
           ::cppu::OInterfaceIteratorHelper pIterator( *pContainer );
           while ( pIterator.hasMoreElements( ) )
           {
               ( ( util::XModifyListener* )pIterator.next( ) )->modified( aSource );
           }
    }
}

//-----------------------------------------------
void OStorage::BroadcastTransaction( sal_Int8 nMessage )
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
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    OSL_ENSURE( !m_pData->m_bReadOnlyWrap, "The storage can not be modified at all!\n" );

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
SotElement_Impl* OStorage::OpenStreamElement_Impl( const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode, sal_Bool bEncr )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    OSL_ENSURE( !m_pData->m_bReadOnlyWrap || ( nOpenMode & embed::ElementModes::WRITE ) != embed::ElementModes::WRITE,
                "An element can not be opened for writing in readonly storage!\n" );

    SotElement_Impl *pElement = m_pImpl->FindElement( aStreamName );
    if ( !pElement )
    {
        // element does not exist, check if creation is allowed
        if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE )
          || (( nOpenMode & embed::ElementModes::WRITE ) != embed::ElementModes::WRITE )
          || ( nOpenMode & embed::ElementModes::NOCREATE ) == embed::ElementModes::NOCREATE )
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access_denied

        // create a new StreamElement and insert it into the list
        pElement = m_pImpl->InsertStream( aStreamName, bEncr );
    }
    else if ( pElement->m_bIsStorage )
    {
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    OSL_ENSURE( pElement, "In case element can not be created an exception must be thrown!" );

    if ( !pElement->m_pStream )
        m_pImpl->OpenSubStream( pElement );

    if ( !pElement->m_pStream )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    return pElement;
}

//-----------------------------------------------
void OStorage::MakeLinkToSubComponent_Impl( const uno::Reference< lang::XComponent >& xComponent )
{
    if ( !xComponent.is() )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    if ( !m_pData->m_pSubElDispListener )
    {
        m_pData->m_pSubElDispListener = new OChildDispListener_Impl( *this );
        m_pData->m_pSubElDispListener->acquire();
    }

    xComponent->addEventListener( uno::Reference< lang::XEventListener >(
        static_cast< ::cppu::OWeakObject* >( m_pData->m_pSubElDispListener ), uno::UNO_QUERY ) );

    m_pData->m_aOpenSubComponentsList.push_back( xComponent );
}

//____________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Any SAL_CALL OStorage::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
{
    uno::Any aReturn;

    // common interfaces
    aReturn <<= ::cppu::queryInterface
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

    if ( aReturn.hasValue() == sal_True )
        return aReturn ;

    aReturn <<= ::cppu::queryInterface
                (   rType
                ,   static_cast<embed::XHierarchicalStorageAccess*> ( this )
                ,   static_cast<embed::XHierarchicalStorageAccess2*> ( this ) );

    if ( aReturn.hasValue() == sal_True )
        return aReturn ;

    if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        if ( m_pData->m_bIsRoot )
        {
            aReturn <<= ::cppu::queryInterface
                        (   rType
                        ,   static_cast<embed::XStorageRawAccess*> ( this )
                        ,   static_cast<embed::XEncryptionProtectedSource*> ( this )
                        ,   static_cast<embed::XEncryptionProtectedSource2*> ( this )
                        ,   static_cast<embed::XEncryptionProtectedStorage*> ( this ) );
        }
        else
        {
            aReturn <<= ::cppu::queryInterface
                        (   rType
                        ,   static_cast<embed::XStorageRawAccess*> ( this ) );
        }
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        aReturn <<= ::cppu::queryInterface
                    (   rType
                    ,   static_cast<embed::XRelationshipAccess*> ( this ) );
    }

    if ( aReturn.hasValue() == sal_True )
        return aReturn ;

    return OWeakObject::queryInterface( rType );
}

//-----------------------------------------------
void SAL_CALL OStorage::acquire() throw()
{
    OWeakObject::acquire();
}

//-----------------------------------------------
void SAL_CALL OStorage::release() throw()
{
    OWeakObject::release();
}

//____________________________________________________________________________________________________
//  XTypeProvider
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Sequence< uno::Type > SAL_CALL OStorage::getTypes()
        throw( uno::RuntimeException )
{
    if ( m_pData->m_pTypeCollection == NULL )
    {
        ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

        if ( m_pData->m_pTypeCollection == NULL )
        {
            if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
            {
                if ( m_pData->m_bIsRoot )
                {
                    m_pData->m_pTypeCollection = new ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XStorage >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XStorage2 >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XStorageRawAccess >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XTransactedObject >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XTransactionBroadcaster >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< util::XModifiable >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XEncryptionProtectedStorage >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XEncryptionProtectedSource2 >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XEncryptionProtectedSource >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< beans::XPropertySet >* )NULL ) );
                }
                else
                {
                    m_pData->m_pTypeCollection = new ::cppu::OTypeCollection
                                    (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XStorage >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XStorage2 >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XStorageRawAccess >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XTransactedObject >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< embed::XTransactionBroadcaster >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< util::XModifiable >* )NULL )
                                    ,   ::getCppuType( ( const uno::Reference< beans::XPropertySet >* )NULL ) );
                }
            }
            else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
            {
                m_pData->m_pTypeCollection = new ::cppu::OTypeCollection
                                (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                ,   ::getCppuType( ( const uno::Reference< embed::XStorage >* )NULL )
                                ,   ::getCppuType( ( const uno::Reference< embed::XTransactedObject >* )NULL )
                                ,   ::getCppuType( ( const uno::Reference< embed::XTransactionBroadcaster >* )NULL )
                                ,   ::getCppuType( ( const uno::Reference< util::XModifiable >* )NULL )
                                ,   ::getCppuType( ( const uno::Reference< embed::XRelationshipAccess >* )NULL )
                                ,   ::getCppuType( ( const uno::Reference< beans::XPropertySet >* )NULL ) );
            }
            else
            {
                m_pData->m_pTypeCollection = new ::cppu::OTypeCollection
                                (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                ,   ::getCppuType( ( const uno::Reference< embed::XStorage >* )NULL )
                                ,   ::getCppuType( ( const uno::Reference< embed::XTransactedObject >* )NULL )
                                ,   ::getCppuType( ( const uno::Reference< embed::XTransactionBroadcaster >* )NULL )
                                ,   ::getCppuType( ( const uno::Reference< util::XModifiable >* )NULL )
                                ,   ::getCppuType( ( const uno::Reference< beans::XPropertySet >* )NULL ) );
            }
        }
    }

    return m_pData->m_pTypeCollection->getTypes() ;
}

namespace { struct lcl_ImplId : public rtl::Static< ::cppu::OImplementationId, lcl_ImplId > {}; }

//-----------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL OStorage::getImplementationId()
        throw( uno::RuntimeException )
{
    ::cppu::OImplementationId &rID = lcl_ImplId::get();
    return rID.getImplementationId();
}

//____________________________________________________________________________________________________
//  XStorage
//____________________________________________________________________________________________________


//-----------------------------------------------
void SAL_CALL OStorage::copyToStorage( const uno::Reference< embed::XStorage >& xDest )
        throw ( embed::InvalidStorageException,
                io::IOException,
                lang::IllegalArgumentException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::copyToStorage" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( !xDest.is() || xDest == uno::Reference< uno::XInterface >( static_cast< OWeakObject*> ( this ), uno::UNO_QUERY ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 );

    try {
        m_pImpl->CopyToStorage( xDest, sal_False );
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't copy storage!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL OStorage::openStreamElement(
    const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::openStreamElement" );

    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aStreamName == "_rels" )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 ); // unacceptable element name

    if ( ( nOpenMode & embed::ElementModes::WRITE ) && m_pData->m_bReadOnlyWrap )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access denied

    uno::Reference< io::XStream > xResult;
    try
    {
        SotElement_Impl *pElement = OpenStreamElement_Impl( aStreamName, nOpenMode, sal_False );
        OSL_ENSURE( pElement && pElement->m_pStream, "In case element can not be created an exception must be thrown!" );

        xResult = pElement->m_pStream->GetStream( nOpenMode, sal_False );
        OSL_ENSURE( xResult.is(), "The method must throw exception instead of removing empty result!\n" );

        if ( m_pData->m_bReadOnlyWrap )
        {
            // before the storage disposes the stream it must deregister itself as listener
            uno::Reference< lang::XComponent > xStreamComponent( xResult, uno::UNO_QUERY );
            if ( !xStreamComponent.is() )
                throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

            MakeLinkToSubComponent_Impl( xStreamComponent );
        }
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const packages::WrongPasswordException& rWrongPasswordException )
    {
        m_pImpl->AddLog( rWrongPasswordException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException(OSL_LOG_PREFIX "Can't open stream element!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    aGuard.clear();

    BroadcastModifiedIfNecessary();

    return xResult;
}

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL OStorage::openEncryptedStreamElement(
    const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode, const ::rtl::OUString& aPass )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::openEncryptedStreamElement" );

    return openEncryptedStream( aStreamName, nOpenMode, ::comphelper::OStorageHelper::CreatePackageEncryptionData( aPass ) );
}

//-----------------------------------------------
uno::Reference< embed::XStorage > SAL_CALL OStorage::openStorageElement(
            const ::rtl::OUString& aStorName, sal_Int32 nStorageMode )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::openStorageElement" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aStorName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStorName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aStorName == "_rels" )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 ); // unacceptable storage name

    if ( ( nStorageMode & embed::ElementModes::WRITE ) && m_pData->m_bReadOnlyWrap )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access denied

    if ( ( nStorageMode & embed::ElementModes::TRUNCATE )
      && !( nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access denied

    // it's allways possible to read written storage in this implementation
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
                throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access_denied

            // create a new StorageElement and insert it into the list
            pElement = m_pImpl->InsertStorage( aStorName, nStorageMode );
        }
        else if ( !pElement->m_bIsStorage )
        {
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        }
        else if ( pElement->m_pStorage )
        {
            // storage has already been opened; it may be opened another time, if it the mode allows to do so
            if ( pElement->m_pStorage->m_pAntiImpl )
            {
                throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access_denied
            }
            else if ( !pElement->m_pStorage->m_aReadOnlyWrapList.empty()
                    && ( nStorageMode & embed::ElementModes::WRITE ) )
            {
                throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access_denied
            }
            else
            {
                // in case parent storage allows writing the readonly mode of the child storage is
                // virtual, that means that it is just enough to change the flag to let it be writable
                // and since there is no AntiImpl nobody should be notified about it
                pElement->m_pStorage->m_nStorageMode = nStorageMode | embed::ElementModes::READ;

                if ( ( nStorageMode & embed::ElementModes::TRUNCATE ) )
                {
                    for ( SotElementList_Impl::iterator pElementIter = pElement->m_pStorage->m_aChildrenList.begin();
                           pElementIter != pElement->m_pStorage->m_aChildrenList.end(); )
                       {
                        SotElement_Impl* pElementToDel = (*pElementIter);
                        ++pElementIter;

                        m_pImpl->RemoveElement( pElementToDel );
                       }
                }
            }
        }

        if ( !pElement->m_pStorage )
            m_pImpl->OpenSubStorage( pElement, nStorageMode );

        if ( !pElement->m_pStorage )
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: general_error

        sal_Bool bReadOnlyWrap = ( ( nStorageMode & embed::ElementModes::WRITE ) != embed::ElementModes::WRITE );
        OStorage* pResultStorage = new OStorage( pElement->m_pStorage, bReadOnlyWrap );
        xResult = uno::Reference< embed::XStorage >( (embed::XStorage*) pResultStorage );

        if ( bReadOnlyWrap )
        {
            // Before this call is done the object must be refcounted already
            pElement->m_pStorage->SetReadOnlyWrap( *pResultStorage );

            // before the storage disposes the stream it must deregister itself as listener
            uno::Reference< lang::XComponent > xStorageComponent( xResult, uno::UNO_QUERY );
            if ( !xStorageComponent.is() )
                throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

            MakeLinkToSubComponent_Impl( xStorageComponent );
        }
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't open storage!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    return xResult;
}

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL OStorage::cloneStreamElement( const ::rtl::OUString& aStreamName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::cloneStreamElement" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aStreamName == "_rels" )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 ); // unacceptable storage name

    try
    {
        uno::Reference< io::XStream > xResult;
        m_pImpl->CloneStreamElement( aStreamName, sal_False, ::comphelper::SequenceAsHashMap(), xResult );
        if ( !xResult.is() )
            throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        return xResult;
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const packages::WrongPasswordException& rWrongPasswordException )
    {
        m_pImpl->AddLog( rWrongPasswordException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't clone stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL OStorage::cloneEncryptedStreamElement(
    const ::rtl::OUString& aStreamName,
    const ::rtl::OUString& aPass )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::cloneEncryptedStreamElement" );

    return cloneEncryptedStream( aStreamName, ::comphelper::OStorageHelper::CreatePackageEncryptionData( aPass ) );
}

//-----------------------------------------------
void SAL_CALL OStorage::copyLastCommitTo(
            const uno::Reference< embed::XStorage >& xTargetStorage )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::copyLastCommitTo" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    try
    {
        m_pImpl->CopyLastCommitTo( xTargetStorage );
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't copy last commit version!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

}

//-----------------------------------------------
void SAL_CALL OStorage::copyStorageElementLastCommitTo(
            const ::rtl::OUString& aStorName,
            const uno::Reference< embed::XStorage >& xTargetStorage )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::copyStorageElementLastCommitTo" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!");
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aStorName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStorName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aStorName == "_rels" )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 ); // unacceptable storage name

    // it's allways possible to read written storage in this implementation
    sal_Int32 nStorageMode = embed::ElementModes::READ;

    try
    {
        SotElement_Impl *pElement = m_pImpl->FindElement( aStorName );
        if ( !pElement )
        {
            // element does not exist, throw exception
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access_denied
        }
        else if ( !pElement->m_bIsStorage )
        {
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        }

        if ( !pElement->m_pStorage )
            m_pImpl->OpenSubStorage( pElement, nStorageMode );

        uno::Reference< embed::XStorage > xResult;
        if ( pElement->m_pStorage )
        {
            // the existence of m_pAntiImpl of the child is not interesting,
            // the copy will be created internally

            pElement->m_pStorage->CopyLastCommitTo( xTargetStorage );
        }
        else
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: general_error
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't copy last commit element version!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

//-----------------------------------------------
sal_Bool SAL_CALL OStorage::isStreamElement( const ::rtl::OUString& aElementName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aElementName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aElementName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aElementName == "_rels" )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 ); // unacceptable name

    SotElement_Impl* pElement = NULL;

    try
    {
        pElement = m_pImpl->FindElement( aElementName );
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        m_pImpl->AddLog( rNoSuchElementException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException( OSL_LOG_PREFIX "Can't detect whether it is a stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    if ( !pElement )
        throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); //???

    return !pElement->m_bIsStorage;
}

//-----------------------------------------------
sal_Bool SAL_CALL OStorage::isStorageElement( const ::rtl::OUString& aElementName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aElementName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aElementName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aElementName == "_rels" )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 );

    SotElement_Impl* pElement = NULL;

    try
    {
        pElement = m_pImpl->FindElement( aElementName );
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        m_pImpl->AddLog( rNoSuchElementException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException( OSL_LOG_PREFIX "can't detect whether it is a storage",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    if ( !pElement )
        throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); //???

    return pElement->m_bIsStorage;
}

//-----------------------------------------------
void SAL_CALL OStorage::removeElement( const ::rtl::OUString& aElementName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::removeElement" );

    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aElementName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aElementName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aElementName == "_rels" )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 ); // TODO: unacceptable name

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access denied

    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( aElementName );

        if ( !pElement )
            throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); //???

        m_pImpl->RemoveElement( pElement );

        m_pImpl->m_bIsModified = sal_True;
        m_pImpl->m_bBroadcastModified = sal_True;
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        m_pImpl->AddLog( rNoSuchElementException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't remove element!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    aGuard.clear();

    BroadcastModifiedIfNecessary();
}

//-----------------------------------------------
void SAL_CALL OStorage::renameElement( const ::rtl::OUString& aElementName, const ::rtl::OUString& aNewName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::renameElement" );

    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aElementName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aElementName, sal_False )
      || aNewName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aNewName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && ( aElementName == "_rels" || aNewName == "_rels" ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 0 ); // TODO: unacceptable element name

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access denied

    try
    {
        SotElement_Impl* pRefElement = m_pImpl->FindElement( aNewName );
        if ( pRefElement )
            throw container::ElementExistException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); //???

        SotElement_Impl* pElement = m_pImpl->FindElement( aElementName );
        if ( !pElement )
            throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); //???

        pElement->m_aName = aNewName;

        m_pImpl->m_bIsModified = sal_True;
        m_pImpl->m_bBroadcastModified = sal_True;
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        m_pImpl->AddLog( rNoSuchElementException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::ElementExistException& rElementExistException )
    {
        m_pImpl->AddLog( rElementExistException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't rename element!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    aGuard.clear();

    BroadcastModifiedIfNecessary();
}

//-----------------------------------------------
void SAL_CALL OStorage::copyElementTo(  const ::rtl::OUString& aElementName,
                                        const uno::Reference< embed::XStorage >& xDest,
                                        const ::rtl::OUString& aNewName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::copyElementTo" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aElementName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aElementName, sal_False )
      || aNewName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aNewName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( !xDest.is() )
        // || xDest == uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( this ), uno::UNO_QUERY ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 2 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && ( aElementName == "_rels" || aNewName == "_rels" ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 0 ); // unacceptable element name

    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( aElementName );
        if ( !pElement )
            throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        uno::Reference< XNameAccess > xNameAccess( xDest, uno::UNO_QUERY );
        if ( !xNameAccess.is() )
            throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        if ( xNameAccess->hasByName( aNewName ) )
            throw container::ElementExistException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        m_pImpl->CopyStorageElement( pElement, xDest, aNewName, sal_False );
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        m_pImpl->AddLog( rNoSuchElementException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::ElementExistException& rElementExistException )
    {
        m_pImpl->AddLog( rElementExistException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't copy element!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}


//-----------------------------------------------
void SAL_CALL OStorage::moveElementTo(  const ::rtl::OUString& aElementName,
                                        const uno::Reference< embed::XStorage >& xDest,
                                        const ::rtl::OUString& aNewName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::moveElementTo" );

    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aElementName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aElementName, sal_False )
      || aNewName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aNewName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( !xDest.is() || xDest == uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( this ), uno::UNO_QUERY ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 2 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && ( aElementName == "_rels" || aNewName == "_rels" ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 0 ); // unacceptable element name

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access denied

    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( aElementName );
        if ( !pElement )
            throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); //???

        uno::Reference< XNameAccess > xNameAccess( xDest, uno::UNO_QUERY );
        if ( !xNameAccess.is() )
            throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        if ( xNameAccess->hasByName( aNewName ) )
            throw container::ElementExistException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        m_pImpl->CopyStorageElement( pElement, xDest, aNewName, sal_False );

        m_pImpl->RemoveElement( pElement );

        m_pImpl->m_bIsModified = sal_True;
        m_pImpl->m_bBroadcastModified = sal_True;
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        m_pImpl->AddLog( rNoSuchElementException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::ElementExistException& rElementExistException )
    {
        m_pImpl->AddLog( rElementExistException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't move element!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    aGuard.clear();

    BroadcastModifiedIfNecessary();
}

//____________________________________________________________________________________________________
//  XStorage2
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL OStorage::openEncryptedStream(
    const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode, const uno::Sequence< beans::NamedValue >& aEncryptionData )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::openEncryptedStream" );

    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        packages::NoEncryptionException();

    if ( ( nOpenMode & embed::ElementModes::WRITE ) && m_pData->m_bReadOnlyWrap )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access denied

    if ( !aEncryptionData.getLength() )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 3 );

    uno::Reference< io::XStream > xResult;
    try
    {
        SotElement_Impl *pElement = OpenStreamElement_Impl( aStreamName, nOpenMode, sal_True );
        OSL_ENSURE( pElement && pElement->m_pStream, "In case element can not be created an exception must be thrown!" );

        xResult = pElement->m_pStream->GetStream( nOpenMode, aEncryptionData, sal_False );
        OSL_ENSURE( xResult.is(), "The method must throw exception instead of removing empty result!\n" );

        if ( m_pData->m_bReadOnlyWrap )
        {
            // before the storage disposes the stream it must deregister itself as listener
            uno::Reference< lang::XComponent > xStreamComponent( xResult, uno::UNO_QUERY );
            if ( !xStreamComponent.is() )
                throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

            MakeLinkToSubComponent_Impl( xStreamComponent );
        }
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const packages::NoEncryptionException& rNoEncryptionException )
    {
        m_pImpl->AddLog( rNoEncryptionException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const packages::WrongPasswordException& rWrongPasswordException )
    {
        m_pImpl->AddLog( rWrongPasswordException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't open encrypted stream stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    aGuard.clear();

    BroadcastModifiedIfNecessary();

    return xResult;
}

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL OStorage::cloneEncryptedStream(
    const ::rtl::OUString& aStreamName,
    const uno::Sequence< beans::NamedValue >& aEncryptionData )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::cloneEncryptedStream" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        packages::NoEncryptionException();

    if ( !aEncryptionData.getLength() )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 2 );

    try
    {
        uno::Reference< io::XStream > xResult;
        m_pImpl->CloneStreamElement( aStreamName, sal_True, aEncryptionData, xResult );
        if ( !xResult.is() )
            throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        return xResult;
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const packages::NoEncryptionException& rNoEncryptionException )
    {
        m_pImpl->AddLog( rNoEncryptionException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const packages::WrongPasswordException& rWrongPasswordException )
    {
        m_pImpl->AddLog( rWrongPasswordException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't clone encrypted stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}


//____________________________________________________________________________________________________
//  XStorageRawAccess
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OStorage::getPlainRawStreamElement(
            const ::rtl::OUString& sStreamName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::getPlainRawStreamElement" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // the interface is not supported and must not be accessible

    if ( sStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( sStreamName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    uno::Reference < io::XInputStream > xTempIn;
    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( sStreamName );
        if ( !pElement )
            throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        if ( !pElement->m_pStream )
        {
            m_pImpl->OpenSubStream( pElement );
            if ( !pElement->m_pStream )
                throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        }

        uno::Reference< io::XInputStream > xRawInStream = pElement->m_pStream->GetPlainRawInStream();
        if ( !xRawInStream.is() )
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        uno::Reference < io::XOutputStream > xTempOut(
                            io::TempFile::create(comphelper::getComponentContext(m_pImpl->GetServiceFactory())),
                            uno::UNO_QUERY );
        xTempIn = uno::Reference < io::XInputStream >( xTempOut, uno::UNO_QUERY );
        uno::Reference < io::XSeekable > xSeek( xTempOut, uno::UNO_QUERY );

        if ( !xTempOut.is() || !xTempIn.is() || !xSeek.is() )
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        // Copy temporary file to a new one
        ::comphelper::OStorageHelper::CopyInputToOutput( xRawInStream, xTempOut );
        xTempOut->closeOutput();
        xSeek->seek( 0 );
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        m_pImpl->AddLog( rNoSuchElementException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't get plain raw stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    return xTempIn;
}

//-----------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OStorage::getRawEncrStreamElement(
            const ::rtl::OUString& sStreamName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                container::NoSuchElementException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::getRawEncrStreamElement" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw packages::NoEncryptionException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    if ( sStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( sStreamName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    uno::Reference < io::XInputStream > xTempIn;
    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( sStreamName );
        if ( !pElement )
            throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        if ( !pElement->m_pStream )
        {
            m_pImpl->OpenSubStream( pElement );
            if ( !pElement->m_pStream )
                throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        }

        if ( !pElement->m_pStream->IsEncrypted() )
            throw packages::NoEncryptionException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        uno::Reference< io::XInputStream > xRawInStream = pElement->m_pStream->GetRawInStream();
        if ( !xRawInStream.is() )
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        uno::Reference < io::XOutputStream > xTempOut(
                            io::TempFile::create(comphelper::getComponentContext(m_pImpl->GetServiceFactory())),
                            uno::UNO_QUERY );
        xTempIn = uno::Reference < io::XInputStream >( xTempOut, uno::UNO_QUERY );
        uno::Reference < io::XSeekable > xSeek( xTempOut, uno::UNO_QUERY );

        if ( !xTempOut.is() || !xTempIn.is() || !xSeek.is() )
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        // Copy temporary file to a new one
        ::comphelper::OStorageHelper::CopyInputToOutput( xRawInStream, xTempOut );
        xTempOut->closeOutput();
        xSeek->seek( 0 );

    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const packages::NoEncryptionException& rNoEncryptionException )
    {
        m_pImpl->AddLog( rNoEncryptionException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        m_pImpl->AddLog( rNoSuchElementException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't get raw stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    return xTempIn;
}

//-----------------------------------------------
void SAL_CALL OStorage::insertRawEncrStreamElement( const ::rtl::OUString& aStreamName,
                                const uno::Reference< io::XInputStream >& xInStream )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoRawFormatException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::insertRawEncrStreamElement" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw packages::NoEncryptionException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    if ( aStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( !xInStream.is() )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 2 );

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access denied

    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( aStreamName );
        if ( pElement )
            throw container::ElementExistException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        m_pImpl->InsertRawStream( aStreamName, xInStream );
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const packages::NoRawFormatException& rNoRawFormatException )
    {
        m_pImpl->AddLog( rNoRawFormatException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::ElementExistException& rElementExistException )
    {
        m_pImpl->AddLog( rElementExistException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't insert raw stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

//____________________________________________________________________________________________________
//  XTransactedObject
//____________________________________________________________________________________________________

//-----------------------------------------------
void SAL_CALL OStorage::commit()
        throw ( io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::commit" );

    uno::Reference< util::XModifiable > xParentModif;

    try {
        BroadcastTransaction( STOR_MESS_PRECOMMIT );

        ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

        if ( !m_pImpl )
        {
            ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
            throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        }

        if ( m_pData->m_bReadOnlyWrap )
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access_denied

        m_pImpl->Commit(); // the root storage initiates the storing to source

        // when the storage is commited the parent is modified
        if ( m_pImpl->m_pParent && m_pImpl->m_pParent->m_pAntiImpl )
            xParentModif = (util::XModifiable*)m_pImpl->m_pParent->m_pAntiImpl;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
        m_pImpl->AddLog( rException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

        uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Problems on commit!",
                                  uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ),
                                  aCaught );
    }

    setModified( sal_False );
    if ( xParentModif.is() )
        xParentModif->setModified( sal_True );

    BroadcastTransaction( STOR_MESS_COMMITED );
}

//-----------------------------------------------
void SAL_CALL OStorage::revert()
        throw ( io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::revert" );

    // the method removes all the changes done after last commit

    BroadcastTransaction( STOR_MESS_PREREVERT );

    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    for ( SotElementList_Impl::iterator pElementIter = m_pImpl->m_aChildrenList.begin();
          pElementIter != m_pImpl->m_aChildrenList.end(); ++pElementIter )
    {
        if ( ((*pElementIter)->m_pStorage
                && ( (*pElementIter)->m_pStorage->m_pAntiImpl || !(*pElementIter)->m_pStorage->m_aReadOnlyWrapList.empty() ))
          || ((*pElementIter)->m_pStream
                  && ( (*pElementIter)->m_pStream->m_pAntiImpl || !(*pElementIter)->m_pStream->m_aInputStreamsList.empty()) ) )
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access denied
    }

    if ( m_pData->m_bReadOnlyWrap || !m_pImpl->m_bListCreated )
        return; // nothing to do

    try {
        m_pImpl->Revert();
        m_pImpl->m_bIsModified = sal_False;
        m_pImpl->m_bBroadcastModified = sal_True;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
        m_pImpl->AddLog( rException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

        uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Problems on revert!",
                                  uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ),
                                  aCaught );
    }

    aGuard.clear();

    setModified( sal_False );
    BroadcastTransaction( STOR_MESS_REVERTED );
}

//____________________________________________________________________________________________________
//  XTransactionBroadcaster
//____________________________________________________________________________________________________

//-----------------------------------------------
void SAL_CALL OStorage::addTransactionListener( const uno::Reference< embed::XTransactionListener >& aListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    m_pData->m_aListenersContainer.addInterface( ::getCppuType((const uno::Reference< embed::XTransactionListener >*)0),
                                                aListener );
}

//-----------------------------------------------
void SAL_CALL OStorage::removeTransactionListener( const uno::Reference< embed::XTransactionListener >& aListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    m_pData->m_aListenersContainer.removeInterface( ::getCppuType((const uno::Reference< embed::XTransactionListener >*)0),
                                                    aListener );
}

//____________________________________________________________________________________________________
//  XModifiable
//  TODO: if there will be no demand on this interface it will be removed from implementation,
//        I do not want to remove it now since it is still possible that it will be inserted
//        to the service back.
//____________________________________________________________________________________________________

//-----------------------------------------------
sal_Bool SAL_CALL OStorage::isModified()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    return m_pImpl->m_bIsModified;
}


//-----------------------------------------------
void SAL_CALL OStorage::setModified( sal_Bool bModified )
        throw ( beans::PropertyVetoException,
                uno::RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_bReadOnlyWrap )
        throw beans::PropertyVetoException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access denied

    if ( m_pImpl->m_bIsModified != bModified )
        m_pImpl->m_bIsModified = bModified;

    aGuard.clear();
    if ( bModified )
    {
        m_pImpl->m_bBroadcastModified = sal_True;
        BroadcastModifiedIfNecessary();
    }
}

//-----------------------------------------------
void SAL_CALL OStorage::addModifyListener(
            const uno::Reference< util::XModifyListener >& aListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    m_pData->m_aListenersContainer.addInterface(
                                ::getCppuType( ( const uno::Reference< util::XModifyListener >* )0 ), aListener );
}


//-----------------------------------------------
void SAL_CALL OStorage::removeModifyListener(
            const uno::Reference< util::XModifyListener >& aListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    m_pData->m_aListenersContainer.removeInterface(
                                ::getCppuType( ( const uno::Reference< util::XModifyListener >* )0 ), aListener );
}

//____________________________________________________________________________________________________
//  XNameAccess
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Any SAL_CALL OStorage::getByName( const ::rtl::OUString& aName )
        throw ( container::NoSuchElementException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::getByName" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aName == "_rels" )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 ); // unacceptable element name

    uno::Any aResult;
    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( aName );
        if ( !pElement )
            throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        if ( pElement->m_bIsStorage )
            aResult <<= openStorageElement( aName, embed::ElementModes::READ );
        else
            aResult <<= openStreamElement( aName, embed::ElementModes::READ );
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        m_pImpl->AddLog( rNoSuchElementException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::WrappedTargetException& rWrappedTargetException )
    {
        m_pImpl->AddLog( rWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
           m_pImpl->AddLog( rException.Message );
           m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

           uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetException( OSL_LOG_PREFIX "Can not open storage!\n",
                                            uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                uno::UNO_QUERY ),
                                            aCaught );
    }

    return aResult;
}


//-----------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OStorage::getElementNames()
        throw ( uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::getElementNames" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    try
    {
        return m_pImpl->GetElementNames();
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch ( const uno::Exception& rException )
    {
           m_pImpl->AddLog( rException.Message );
           m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

           uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException( OSL_LOG_PREFIX "Can not open storage!\n",
                                            uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                uno::UNO_QUERY ),
                                            aCaught );
    }
}


//-----------------------------------------------
sal_Bool SAL_CALL OStorage::hasByName( const ::rtl::OUString& aName )
        throw ( uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::hasByName" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aName.isEmpty() )
        return sal_False;

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aName == "_rels" )
        return sal_False;

    SotElement_Impl* pElement = NULL;
    try
    {
        pElement = m_pImpl->FindElement( aName );
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch ( const uno::Exception& rException )
    {
           m_pImpl->AddLog( rException.Message );
           m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

           uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException( OSL_LOG_PREFIX "Can not open storage!\n",
                                            uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                uno::UNO_QUERY ),
                                            aCaught );
    }

    return ( pElement != NULL );
}


//-----------------------------------------------
uno::Type SAL_CALL OStorage::getElementType()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    // it is a multitype container
    return uno::Type();
}


//-----------------------------------------------
sal_Bool SAL_CALL OStorage::hasElements()
        throw ( uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::hasElements" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    try
    {
        return ( m_pImpl->GetChildrenList().size() != 0 );
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
           m_pImpl->AddLog( rException.Message );
           m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

           uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException( OSL_LOG_PREFIX "Can not open storage!\n",
                                            uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                uno::UNO_QUERY ),
                                            aCaught );
    }
}


//____________________________________________________________________________________________________
//  XComponent
//____________________________________________________________________________________________________

//-----------------------------------------------
void SAL_CALL OStorage::dispose()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    try
    {
        InternalDispose( sal_True );
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
           m_pImpl->AddLog( rException.Message );
           m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

           uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException( OSL_LOG_PREFIX "Can not open storage!\n",
                                            uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                uno::UNO_QUERY ),
                                            aCaught );
    }
}

//-----------------------------------------------
void SAL_CALL OStorage::addEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    m_pData->m_aListenersContainer.addInterface(
                                ::getCppuType( ( const uno::Reference< lang::XEventListener >* )0 ), xListener );
}

//-----------------------------------------------
void SAL_CALL OStorage::removeEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    m_pData->m_aListenersContainer.removeInterface(
                                ::getCppuType( ( const uno::Reference< lang::XEventListener >* )0 ), xListener );
}

//____________________________________________________________________________________________________
//  XEncryptionProtectedSource
//____________________________________________________________________________________________________

void SAL_CALL OStorage::setEncryptionPassword( const ::rtl::OUString& aPass )
    throw ( uno::RuntimeException,
            io::IOException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::setEncryptionPassword" );
    setEncryptionData( ::comphelper::OStorageHelper::CreatePackageEncryptionData( aPass ) );
}

//-----------------------------------------------
void SAL_CALL OStorage::removeEncryption()
    throw ( uno::RuntimeException,
            io::IOException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::removeEncryption" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // the interface must be visible only for package storage

    OSL_ENSURE( m_pData->m_bIsRoot, "removeEncryption() method is not available for nonroot storages!\n" );
    if ( m_pData->m_bIsRoot )
    {
        try {
            m_pImpl->ReadContents();
        }
        catch ( const uno::RuntimeException& rRuntimeException )
        {
            m_pImpl->AddLog( rRuntimeException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
            throw;
        }
        catch ( const uno::Exception& rException )
        {
            m_pImpl->AddLog( rException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

            uno::Any aCaught( ::cppu::getCaughtException() );
            throw lang::WrappedTargetRuntimeException( OSL_LOG_PREFIX "Can not open package!\n",
                                                uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                    uno::UNO_QUERY ),
                                                aCaught );
        }

        // TODO: check if the password is valid
        // update all streams that was encrypted with old password

        uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY_THROW );
        try
        {
            xPackPropSet->setPropertyValue( STORAGE_ENCRYPTION_KEYS_PROPERTY,
                                            uno::makeAny( uno::Sequence< beans::NamedValue >() ) );

            m_pImpl->m_bHasCommonEncryptionData = sal_False;
            m_pImpl->m_aCommonEncryptionData.clear();
        }
        catch( const uno::RuntimeException& rRException )
        {
            m_pImpl->AddLog( rRException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

            OSL_ENSURE( sal_False, "The call must not fail, it is pretty simple!" );
            throw;
        }
        catch( const uno::Exception& rException )
        {
            m_pImpl->AddLog( rException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

            OSL_FAIL( "The call must not fail, it is pretty simple!" );
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        }
    }
}

//____________________________________________________________________________________________________
//  XEncryptionProtectedSource2
//____________________________________________________________________________________________________

void SAL_CALL OStorage::setEncryptionData( const uno::Sequence< beans::NamedValue >& aEncryptionData )
    throw ( io::IOException,
            uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::setEncryptionData" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // the interface must be visible only for package storage

    if ( !aEncryptionData.getLength() )
        throw uno::RuntimeException( OSL_LOG_PREFIX "Unexpected empty encryption data!", uno::Reference< uno::XInterface >() );

    OSL_ENSURE( m_pData->m_bIsRoot, "setEncryptionData() method is not available for nonroot storages!\n" );
    if ( m_pData->m_bIsRoot )
    {
        try {
            m_pImpl->ReadContents();
        }
        catch ( const uno::RuntimeException& rRuntimeException )
        {
            m_pImpl->AddLog( rRuntimeException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
            throw;
        }
        catch ( const uno::Exception& rException )
        {
            m_pImpl->AddLog( rException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

            uno::Any aCaught( ::cppu::getCaughtException() );
            throw lang::WrappedTargetRuntimeException( OSL_LOG_PREFIX "Can not open package!\n",
                                uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ), uno::UNO_QUERY ),
                                aCaught );
        }

        uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY_THROW );
        try
        {
            ::comphelper::SequenceAsHashMap aEncryptionMap( aEncryptionData );
            xPackPropSet->setPropertyValue( STORAGE_ENCRYPTION_KEYS_PROPERTY,
                                            uno::makeAny( aEncryptionMap.getAsConstNamedValueList() ) );

            m_pImpl->m_bHasCommonEncryptionData = sal_True;
            m_pImpl->m_aCommonEncryptionData = aEncryptionMap;
        }
        catch( const uno::Exception& rException )
        {
            m_pImpl->AddLog( rException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        }
    }
}

sal_Bool SAL_CALL OStorage::hasEncryptionData()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    return m_pImpl && m_pImpl->m_bHasCommonEncryptionData;
}


//____________________________________________________________________________________________________
//  XEncryptionProtectedStorage
//____________________________________________________________________________________________________

//-----------------------------------------------
void SAL_CALL OStorage::setEncryptionAlgorithms( const uno::Sequence< beans::NamedValue >& aAlgorithms )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::setEncryptionAlgorithms" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // the interface must be visible only for package storage

    if ( !aAlgorithms.getLength() )
        throw uno::RuntimeException( OSL_LOG_PREFIX "Unexpected empty encryption algorithms list!", uno::Reference< uno::XInterface >() );

    OSL_ENSURE( m_pData->m_bIsRoot, "setEncryptionAlgorithms() method is not available for nonroot storages!\n" );
    if ( m_pData->m_bIsRoot )
    {
        try {
            m_pImpl->ReadContents();
        }
        catch ( const uno::RuntimeException& aRuntimeException )
        {
            m_pImpl->AddLog( aRuntimeException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
            throw;
        }
        catch ( const uno::Exception& aException )
        {
            m_pImpl->AddLog( aException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

            uno::Any aCaught( ::cppu::getCaughtException() );
            throw lang::WrappedTargetException( OSL_LOG_PREFIX "Can not open package!\n",
                                                uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                    uno::UNO_QUERY ),
                                                aCaught );
        }

        uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY_THROW );
        try
        {
            xPackPropSet->setPropertyValue( ENCRYPTION_ALGORITHMS_PROPERTY,
                                            uno::makeAny( aAlgorithms ) );
        }
        catch ( const uno::RuntimeException& aRuntimeException )
        {
            m_pImpl->AddLog( aRuntimeException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
            throw;
        }
        catch( const lang::IllegalArgumentException& aIAException )
        {
            m_pImpl->AddLog( aIAException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

            throw;
        }
        catch( const uno::Exception& aException )
        {
            m_pImpl->AddLog( aException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        }
    }
}

//-----------------------------------------------
uno::Sequence< beans::NamedValue > SAL_CALL OStorage::getEncryptionAlgorithms()
    throw (uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::getEncryptionAlgorithms" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // the interface must be visible only for package storage

    uno::Sequence< beans::NamedValue > aResult;
    OSL_ENSURE( m_pData->m_bIsRoot, "getEncryptionAlgorithms() method is not available for nonroot storages!\n" );
    if ( m_pData->m_bIsRoot )
    {
        try {
            m_pImpl->ReadContents();
        }
        catch ( const uno::RuntimeException& aRuntimeException )
        {
            m_pImpl->AddLog( aRuntimeException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
            throw;
        }
        catch ( const uno::Exception& aException )
        {
            m_pImpl->AddLog( aException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

            uno::Any aCaught( ::cppu::getCaughtException() );
            throw lang::WrappedTargetException( OSL_LOG_PREFIX "Can not open package!\n",
                                                uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                    uno::UNO_QUERY ),
                                                aCaught );
        }

        uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY_THROW );
        try
        {
            xPackPropSet->getPropertyValue( ENCRYPTION_ALGORITHMS_PROPERTY ) >>= aResult;
        }
        catch ( const uno::RuntimeException& aRuntimeException )
        {
            m_pImpl->AddLog( aRuntimeException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
            throw;
        }
        catch( const uno::Exception& aException )
        {
            m_pImpl->AddLog( aException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        }
    }

    return aResult;
}


//____________________________________________________________________________________________________
//  XPropertySet
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OStorage::getPropertySetInfo()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    //TODO:
    return uno::Reference< beans::XPropertySetInfo >();
}


//-----------------------------------------------
void SAL_CALL OStorage::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
        throw ( beans::UnknownPropertyException,
                beans::PropertyVetoException,
                lang::IllegalArgumentException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::setPropertyValue" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    //TODO: think about interaction handler

    // WORKAROUND:
    // The old document might have no version in the manifest.xml, so we have to allow to set the version
    // even for readonly storages, so that the version from content.xml can be used.
    if ( m_pData->m_bReadOnlyWrap && aPropertyName != "Version" )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: Access denied

    if ( m_pData->m_nStorageType == embed::StorageFormats::ZIP )
        throw beans::UnknownPropertyException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    else if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE )
    {
        if ( aPropertyName == "MediaType" )
        {
            aValue >>= m_pImpl->m_aMediaType;
            m_pImpl->m_bControlMediaType = sal_True;

            m_pImpl->m_bBroadcastModified = sal_True;
            m_pImpl->m_bIsModified = sal_True;
        }
        else if ( aPropertyName == "Version" )
        {
            aValue >>= m_pImpl->m_aVersion;
            m_pImpl->m_bControlVersion = sal_True;

            // this property can be set even for readonly storage
            if ( !m_pData->m_bReadOnlyWrap )
            {
                m_pImpl->m_bBroadcastModified = sal_True;
                m_pImpl->m_bIsModified = sal_True;
            }
        }
        else if ( ( m_pData->m_bIsRoot && ( aPropertyName == HAS_ENCRYPTED_ENTRIES_PROPERTY
                                    || aPropertyName == HAS_NONENCRYPTED_ENTRIES_PROPERTY
                                    || aPropertyName == IS_INCONSISTENT_PROPERTY
                                    || aPropertyName == "URL"
                                    || aPropertyName == "RepairPackage" ) )
           || aPropertyName == "IsRoot"
           || aPropertyName == MEDIATYPE_FALLBACK_USED_PROPERTY )
            throw beans::PropertyVetoException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        else
            throw beans::UnknownPropertyException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }
    else if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        if ( aPropertyName == "RelationsInfoStream" )
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
                    throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 0 );
                }

                m_pImpl->m_xNewRelInfoStream = xInRelStream;
                m_pImpl->m_aRelInfo = uno::Sequence< uno::Sequence< beans::StringPair > >();
                m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED_STREAM;
                m_pImpl->m_bBroadcastModified = sal_True;
                m_pImpl->m_bIsModified = sal_True;
            }
            else
                throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 0 );
        }
        else if ( aPropertyName == "RelationsInfo" )
        {
            if ( aValue >>= m_pImpl->m_aRelInfo )
            {
                m_pImpl->m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
                m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
                m_pImpl->m_bBroadcastModified = sal_True;
                m_pImpl->m_bIsModified = sal_True;
            }
            else
                throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 0 );
        }
        else if ( ( m_pData->m_bIsRoot && ( aPropertyName == "URL" || aPropertyName == "RepairPackage") )
                 || aPropertyName == "IsRoot" )
            throw beans::PropertyVetoException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        else
            throw beans::UnknownPropertyException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }
    else
        throw beans::UnknownPropertyException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    BroadcastModifiedIfNecessary();
}


//-----------------------------------------------
uno::Any SAL_CALL OStorage::getPropertyValue( const ::rtl::OUString& aPropertyName )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::getPropertyValue" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE
      && ( aPropertyName == "MediaType" || aPropertyName == MEDIATYPE_FALLBACK_USED_PROPERTY || aPropertyName == "Version" ) )
    {
        try
        {
            m_pImpl->ReadContents();
        }
        catch ( const uno::RuntimeException& rRuntimeException )
        {
            m_pImpl->AddLog( rRuntimeException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
            throw;
        }
        catch ( const uno::Exception& rException )
        {
            m_pImpl->AddLog( rException.Message );
            m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

            uno::Any aCaught( ::cppu::getCaughtException() );
            throw lang::WrappedTargetException(
                                        "Can't read contents!",
                                        uno::Reference< XInterface >( static_cast< OWeakObject* >( this ), uno::UNO_QUERY ),
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
            for ( sal_Int32 aInd = 0; aInd < m_pImpl->m_xProperties.getLength(); aInd++ )
            {
                if ( m_pImpl->m_xProperties[aInd].Name.equals( aPropertyName ) )
                    return m_pImpl->m_xProperties[aInd].Value;
            }

            if ( aPropertyName == "URL" )
                return uno::makeAny( ::rtl::OUString() );

            return uno::makeAny( sal_False ); // RepairPackage
        }
        else if ( m_pData->m_nStorageType == embed::StorageFormats::PACKAGE
          && ( aPropertyName == HAS_ENCRYPTED_ENTRIES_PROPERTY
            || aPropertyName == HAS_NONENCRYPTED_ENTRIES_PROPERTY
            || aPropertyName == IS_INCONSISTENT_PROPERTY ) )
        {
            try {
                m_pImpl->ReadContents();
                uno::Reference< beans::XPropertySet > xPackPropSet( m_pImpl->m_xPackage, uno::UNO_QUERY );
                if ( !xPackPropSet.is() )
                    throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

                return xPackPropSet->getPropertyValue( aPropertyName );
            }
            catch ( const uno::RuntimeException& rRuntimeException )
            {
                m_pImpl->AddLog( rRuntimeException.Message );
                m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
                throw;
            }
            catch ( const uno::Exception& rException )
            {
                m_pImpl->AddLog( rException.Message );
                m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

                uno::Any aCaught( ::cppu::getCaughtException() );
                throw lang::WrappedTargetException( OSL_LOG_PREFIX "Can not open package!\n",
                                                    uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                        uno::UNO_QUERY ),
                                                    aCaught );
            }
        }
    }

    throw beans::UnknownPropertyException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
}


//-----------------------------------------------
void SAL_CALL OStorage::addPropertyChangeListener(
    const ::rtl::OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OStorage::removePropertyChangeListener(
    const ::rtl::OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OStorage::addVetoableChangeListener(
    const ::rtl::OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OStorage::removeVetoableChangeListener(
    const ::rtl::OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    //TODO:
}

//____________________________________________________________________________________________________
//  XRelationshipAccess
//____________________________________________________________________________________________________

// TODO/LATER: the storage and stream implementations of this interface are very similar, they could use a helper class

//-----------------------------------------------
sal_Bool SAL_CALL OStorage::hasByID(  const ::rtl::OUString& sID )
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    try
    {
        getRelationshipByID( sID );
        return sal_True;
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        m_pImpl->AddLog( rNoSuchElementException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Quiet exception" );
    }

    return sal_False;
}

//-----------------------------------------------
::rtl::OUString SAL_CALL OStorage::getTargetByID(  const ::rtl::OUString& sID  )
        throw ( container::NoSuchElementException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( aSeq[nInd].First == "Target" )
            return aSeq[nInd].Second;

    return ::rtl::OUString();
}

//-----------------------------------------------
::rtl::OUString SAL_CALL OStorage::getTypeByID(  const ::rtl::OUString& sID  )
        throw ( container::NoSuchElementException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( aSeq[nInd].First == "Type" )
            return aSeq[nInd].Second;

    return ::rtl::OUString();
}

//-----------------------------------------------
uno::Sequence< beans::StringPair > SAL_CALL OStorage::getRelationshipByID(  const ::rtl::OUString& sID  )
        throw ( container::NoSuchElementException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

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

    throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
}

//-----------------------------------------------
uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OStorage::getRelationshipsByType(  const ::rtl::OUString& sType  )
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    uno::Sequence< uno::Sequence< beans::StringPair > > aResult;
    sal_Int32 nEntriesNum = 0;

    // TODO/LATER: in future the unification of the ID could be checked
    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    for ( sal_Int32 nInd1 = 0; nInd1 < aSeq.getLength(); nInd1++ )
        for ( sal_Int32 nInd2 = 0; nInd2 < aSeq[nInd1].getLength(); nInd2++ )
            if ( aSeq[nInd1][nInd2].First == "Type" )
            {
                // the type is usually an URL, so the check should be case insensitive
                if ( aSeq[nInd1][nInd2].Second.equalsIgnoreAsciiCase( sType ) )
                {
                    aResult.realloc( ++nEntriesNum );
                    aResult[nEntriesNum-1] = aSeq[nInd1];
                }
                break;
            }

    return aResult;
}

//-----------------------------------------------
uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OStorage::getAllRelationships()
        throw (io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    return m_pImpl->GetAllRelationshipsIfAny();
}

//-----------------------------------------------
void SAL_CALL OStorage::insertRelationshipByID(  const ::rtl::OUString& sID, const uno::Sequence< beans::StringPair >& aEntry, ::sal_Bool bReplace  )
        throw ( container::ElementExistException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    OUString aIDTag( "Id" );

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
        throw container::ElementExistException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );


    m_pImpl->m_aRelInfo = aSeq;
    m_pImpl->m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

//-----------------------------------------------
void SAL_CALL OStorage::removeRelationshipByID(  const ::rtl::OUString& sID  )
        throw ( container::NoSuchElementException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

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

                    m_pImpl->m_aRelInfo = aSeq;
                    m_pImpl->m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
                    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;

                    // TODO/LATER: in future the unification of the ID could be checked
                    return;
                }

                break;
            }

    throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
}

//-----------------------------------------------
void SAL_CALL OStorage::insertRelationships(  const uno::Sequence< uno::Sequence< beans::StringPair > >& aEntries, ::sal_Bool bReplace  )
        throw ( container::ElementExistException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    OUString aIDTag( "Id" );
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
                                    throw container::ElementExistException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

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
                throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: illegal relation ( no ID )

        if ( !bHasID )
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: illegal relations

        nResultInd++;
    }

    aResultSeq.realloc( nResultInd );
    m_pImpl->m_aRelInfo = aResultSeq;
    m_pImpl->m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

//-----------------------------------------------
void SAL_CALL OStorage::clearRelationships()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    m_pImpl->m_aRelInfo.realloc( 0 );
    m_pImpl->m_xNewRelInfoStream = uno::Reference< io::XInputStream >();
    m_pImpl->m_nRelInfoStatus = RELINFO_CHANGED;
}

//____________________________________________________________________________________________________
//  XOptimizedStorage
//____________________________________________________________________________________________________
//-----------------------------------------------
void SAL_CALL OStorage::insertRawNonEncrStreamElementDirect(
            const ::rtl::OUString& /*sStreamName*/,
            const uno::Reference< io::XInputStream >& /*xInStream*/ )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoRawFormatException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    // not implemented currently because there is still no demand
    // might need to be implemented if direct copying of compressed streams is used
    throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
}

//-----------------------------------------------
void SAL_CALL OStorage::insertStreamElementDirect(
            const ::rtl::OUString& aStreamName,
            const uno::Reference< io::XInputStream >& xInStream,
            const uno::Sequence< beans::PropertyValue >& aProps )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::insertStreamElementDirect" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aStreamName == "_rels" )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 ); // unacceptable storage name

    if ( m_pData->m_bReadOnlyWrap )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: access denied

    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( aStreamName );

        if ( pElement )
            throw container::ElementExistException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        pElement = OpenStreamElement_Impl( aStreamName, embed::ElementModes::READWRITE, sal_False );
        OSL_ENSURE( pElement && pElement->m_pStream, "In case element can not be created an exception must be thrown!" );

        pElement->m_pStream->InsertStreamDirectly( xInStream, aProps );
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::ElementExistException& rElementExistException )
    {
        m_pImpl->AddLog( rElementExistException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't insert stream directly!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

//-----------------------------------------------
void SAL_CALL OStorage::copyElementDirectlyTo(
            const ::rtl::OUString& aElementName,
            const uno::Reference< embed::XOptimizedStorage >& xDest,
            const ::rtl::OUString& aNewName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::copyElementDirectlyTo" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aElementName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aElementName, sal_False )
      || aNewName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aNewName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( !xDest.is() || xDest == uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( this ), uno::UNO_QUERY ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 2 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && ( aElementName == "_rels" || aNewName == "_rels" ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 0 ); // unacceptable name

    try
    {
        SotElement_Impl* pElement = m_pImpl->FindElement( aElementName );
        if ( !pElement )
            throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        uno::Reference< XNameAccess > xNameAccess( xDest, uno::UNO_QUERY );
        if ( !xNameAccess.is() )
            throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        if ( xNameAccess->hasByName( aNewName ) )
            throw container::ElementExistException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        // let the element be copied directly
        uno::Reference< embed::XStorage > xStorDest( xDest, uno::UNO_QUERY_THROW );
        m_pImpl->CopyStorageElement( pElement, xStorDest, aNewName, sal_True );
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        m_pImpl->AddLog( rNoSuchElementException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::ElementExistException& rElementExistException )
    {
        m_pImpl->AddLog( rElementExistException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't copy element direcly!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

//-----------------------------------------------
void SAL_CALL OStorage::writeAndAttachToStream( const uno::Reference< io::XStream >& xStream )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::writeAndAttachToStream" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( !m_pData->m_bIsRoot )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 0 );

    if ( !m_pImpl->m_pSwitchStream )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    try
    {
        m_pImpl->m_pSwitchStream->CopyAndSwitchPersistenceTo( xStream );
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't write and attach to stream!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

}

//-----------------------------------------------
void SAL_CALL OStorage::attachToURL( const ::rtl::OUString& sURL,
                                    sal_Bool bReadOnly )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::attachToURL" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( !m_pData->m_bIsRoot )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 0 );

    if ( !m_pImpl->m_pSwitchStream )
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    uno::Reference < ucb::XSimpleFileAccess2 > xAccess(
        ucb::SimpleFileAccess::create(
            comphelper::getComponentContext(m_pImpl->m_xFactory) ) );

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
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't attach to URL!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

//-----------------------------------------------
uno::Any SAL_CALL OStorage::getElementPropertyValue( const ::rtl::OUString& aElementName, const ::rtl::OUString& aPropertyName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                io::IOException,
                beans::UnknownPropertyException,
                beans::PropertyVetoException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OStorage::getElementPropertyValue" );

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aElementName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aElementName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aElementName == "_rels" )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 ); // TODO: unacceptable name

    try
    {
        SotElement_Impl *pElement = m_pImpl->FindElement( aElementName );
        if ( !pElement )
            throw container::NoSuchElementException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        // TODO/LATER: Currently it is only implemented for MediaType property of substorages, might be changed in future
        if ( !pElement->m_bIsStorage || m_pData->m_nStorageType != embed::StorageFormats::PACKAGE || aPropertyName != "MediaType" )
            throw beans::PropertyVetoException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        if ( !pElement->m_pStorage )
            m_pImpl->OpenSubStorage( pElement, embed::ElementModes::READ );

        if ( !pElement->m_pStorage )
            throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // TODO: general_error

        pElement->m_pStorage->ReadContents();
        return uno::makeAny( pElement->m_pStorage->m_aMediaType );
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const container::NoSuchElementException& rNoSuchElementException )
    {
        m_pImpl->AddLog( rNoSuchElementException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const beans::UnknownPropertyException& rUnknownPropertyException )
    {
        m_pImpl->AddLog( rUnknownPropertyException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const beans::PropertyVetoException& rPropertyVetoException )
    {
        m_pImpl->AddLog( rPropertyVetoException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't get element property!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

//-----------------------------------------------
void SAL_CALL OStorage::copyStreamElementData( const ::rtl::OUString& aStreamName, const uno::Reference< io::XStream >& xTargetStream )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aStreamName.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamName, sal_False ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( m_pData->m_nStorageType == embed::StorageFormats::OFOPXML && aStreamName == "_rels" )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 ); // unacceptable name

    if ( !xTargetStream.is() )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 2 );

    try
    {
        uno::Reference< io::XStream > xNonconstRef = xTargetStream;
        m_pImpl->CloneStreamElement( aStreamName, sal_False, ::comphelper::SequenceAsHashMap(), xNonconstRef );

        OSL_ENSURE( xNonconstRef == xTargetStream, "The provided stream reference seems not be filled in correctly!\n" );
        if ( xNonconstRef != xTargetStream )
            throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // if the stream reference is set it must not be changed!
    }
    catch( const embed::InvalidStorageException& rInvalidStorageException )
    {
        m_pImpl->AddLog( rInvalidStorageException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const lang::IllegalArgumentException& rIllegalArgumentException )
    {
        m_pImpl->AddLog( rIllegalArgumentException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const packages::WrongPasswordException& rWrongPasswordException )
    {
        m_pImpl->AddLog( rWrongPasswordException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const io::IOException& rIOException )
    {
        m_pImpl->AddLog( rIOException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const embed::StorageWrappedTargetException& rStorageWrappedTargetException )
    {
        m_pImpl->AddLog( rStorageWrappedTargetException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::RuntimeException& rRuntimeException )
    {
        m_pImpl->AddLog( rRuntimeException.Message );
        m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );
        throw;
    }
    catch( const uno::Exception& rException )
    {
          m_pImpl->AddLog( rException.Message );
          m_pImpl->AddLog( OSL_LOG_PREFIX "Rethrow" );

          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException( OSL_LOG_PREFIX "Can't copy stream data!",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }


}

//____________________________________________________________________________________________________
// XHierarchicalStorageAccess
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Reference< embed::XExtendedStorageStream > SAL_CALL OStorage::openStreamElementByHierarchicalName( const ::rtl::OUString& aStreamPath, ::sal_Int32 nOpenMode )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aStreamPath.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamPath, sal_True ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE )
      && ( nOpenMode & embed::ElementModes::WRITE ) )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // Access denied

    OStringList_Impl aListPath = OHierarchyHolder_Impl::GetListPathFromString( aStreamPath );
    OSL_ENSURE( aListPath.size(), "The result list must not be empty!" );

    uno::Reference< embed::XExtendedStorageStream > xResult;
    if ( aListPath.size() == 1 )
    {
        // that must be a direct request for a stream
        // the transacted version of the stream should be opened

        SotElement_Impl *pElement = OpenStreamElement_Impl( aStreamPath, nOpenMode, sal_False );
        OSL_ENSURE( pElement && pElement->m_pStream, "In case element can not be created an exception must be thrown!" );

        xResult = uno::Reference< embed::XExtendedStorageStream >(
                        pElement->m_pStream->GetStream( nOpenMode, sal_True ),
                        uno::UNO_QUERY_THROW );
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
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    return xResult;
}

//-----------------------------------------------
uno::Reference< embed::XExtendedStorageStream > SAL_CALL OStorage::openEncryptedStreamElementByHierarchicalName( const ::rtl::OUString& aStreamPath, ::sal_Int32 nOpenMode, const ::rtl::OUString& sPassword )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    return openEncryptedStreamByHierarchicalName( aStreamPath, nOpenMode, ::comphelper::OStorageHelper::CreatePackageEncryptionData( sPassword ) );
}

//-----------------------------------------------
void SAL_CALL OStorage::removeStreamElementByHierarchicalName( const ::rtl::OUString& aStreamPath )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( aStreamPath.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamPath, sal_True ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE ) )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // Access denied

    OStringList_Impl aListPath = OHierarchyHolder_Impl::GetListPathFromString( aStreamPath );
    OSL_ENSURE( aListPath.size(), "The result list must not be empty!" );

    if ( !m_pData->m_rHierarchyHolder.is() )
        m_pData->m_rHierarchyHolder = new OHierarchyHolder_Impl(
            uno::Reference< embed::XStorage >( static_cast< embed::XStorage* >( this ) ) );

    m_pData->m_rHierarchyHolder->RemoveStreamHierarchically( aListPath );
}

//____________________________________________________________________________________________________
// XHierarchicalStorageAccess2
//____________________________________________________________________________________________________

uno::Reference< embed::XExtendedStorageStream > SAL_CALL OStorage::openEncryptedStreamByHierarchicalName( const ::rtl::OUString& aStreamPath, ::sal_Int32 nOpenMode, const uno::Sequence< beans::NamedValue >& aEncryptionData )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    if ( m_pData->m_nStorageType != embed::StorageFormats::PACKAGE )
        throw packages::NoEncryptionException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    if ( aStreamPath.isEmpty() || !::comphelper::OStorageHelper::IsValidZipEntryFileName( aStreamPath, sal_True ) )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX "Unexpected entry name syntax.", uno::Reference< uno::XInterface >(), 1 );

    if ( !aEncryptionData.getLength() )
        throw lang::IllegalArgumentException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 3 );

    if ( !( m_pImpl->m_nStorageMode & embed::ElementModes::WRITE )
      && ( nOpenMode & embed::ElementModes::WRITE ) )
        throw io::IOException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() ); // Access denied

    OStringList_Impl aListPath = OHierarchyHolder_Impl::GetListPathFromString( aStreamPath );
    OSL_ENSURE( aListPath.size(), "The result list must not be empty!" );

    uno::Reference< embed::XExtendedStorageStream > xResult;
    if ( aListPath.size() == 1 )
    {
        // that must be a direct request for a stream
        // the transacted version of the stream should be opened

        SotElement_Impl *pElement = OpenStreamElement_Impl( aStreamPath, nOpenMode, sal_True );
        OSL_ENSURE( pElement && pElement->m_pStream, "In case element can not be created an exception must be thrown!" );

        xResult = uno::Reference< embed::XExtendedStorageStream >(
                        pElement->m_pStream->GetStream( nOpenMode, aEncryptionData, sal_True ),
                        uno::UNO_QUERY_THROW );
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
        throw uno::RuntimeException( OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    return xResult;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
