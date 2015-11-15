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
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

#include <comphelper/fileurl.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <tools/urlobj.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <ucbhelper/content.hxx>

#include "fsstorage.hxx"
#include "oinputstreamcontainer.hxx"
#include "ostreamcontainer.hxx"

using namespace ::com::sun::star;

struct FSStorage_Impl
{
    OUString m_aURL;

    ::ucbhelper::Content* m_pContent;
    sal_Int32 m_nMode;

    ::cppu::OInterfaceContainerHelper* m_pListenersContainer; // list of listeners
    ::cppu::OTypeCollection* m_pTypeCollection;

    uno::Reference< uno::XComponentContext > m_xContext;


    FSStorage_Impl( const ::ucbhelper::Content& aContent, sal_Int32 nMode, uno::Reference< uno::XComponentContext > xContext )
    : m_aURL( aContent.getURL() )
    , m_pContent( new ::ucbhelper::Content( aContent ) )
    , m_nMode( nMode )
    , m_pListenersContainer( nullptr )
    , m_pTypeCollection( nullptr )
    , m_xContext( xContext )
    {
        OSL_ENSURE( !m_aURL.isEmpty(), "The URL must not be empty" );
    }

    ~FSStorage_Impl();

    // Copy assignment is forbidden and not implemented.
    FSStorage_Impl (const FSStorage_Impl &) = delete;
    FSStorage_Impl & operator= (const FSStorage_Impl &) = delete;
};

FSStorage_Impl::~FSStorage_Impl()
{
    delete m_pListenersContainer;
    delete m_pTypeCollection;
    delete m_pContent;
}

FSStorage::FSStorage( const ::ucbhelper::Content& aContent,
                    sal_Int32 nMode,
                    uno::Reference< uno::XComponentContext > xContext )
: m_pImpl( new FSStorage_Impl( aContent, nMode, xContext ) )
{
    // TODO: use properties
    if ( !xContext.is() )
        throw uno::RuntimeException();

    GetContent();
}

FSStorage::~FSStorage()
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_refCount++; // to call dispose
        try {
            dispose();
        }
        catch( uno::RuntimeException& )
        {}
    }
}

bool FSStorage::MakeFolderNoUI( const OUString& rFolder )
{
    INetURLObject aURL( rFolder );
    OUString aTitle = aURL.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
    aURL.removeSegment();
    ::ucbhelper::Content aParent;
    ::ucbhelper::Content aResultContent;

       if ( ::ucbhelper::Content::create( aURL.GetMainURL( INetURLObject::NO_DECODE ),
                                 uno::Reference< ucb::XCommandEnvironment >(),
                                 comphelper::getProcessComponentContext(),
                                 aParent ) )
        return ::utl::UCBContentHelper::MakeFolder( aParent, aTitle, aResultContent );

    return false;
}

::ucbhelper::Content* FSStorage::GetContent()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_pImpl->m_pContent )
    {
        uno::Reference< ucb::XCommandEnvironment > xDummyEnv;

        try
        {
            m_pImpl->m_pContent = new ::ucbhelper::Content( m_pImpl->m_aURL, xDummyEnv, comphelper::getProcessComponentContext() );
        }
        catch( uno::Exception& )
        {
        }
    }

    return m_pImpl->m_pContent;
}

void FSStorage::CopyStreamToSubStream( const OUString& aSourceURL,
                                        const uno::Reference< embed::XStorage >& xDest,
                                        const OUString& aNewEntryName )
{
    if ( !xDest.is() )
        throw uno::RuntimeException();

    uno::Reference< ucb::XCommandEnvironment > xDummyEnv;
    ::ucbhelper::Content aSourceContent( aSourceURL, xDummyEnv, comphelper::getProcessComponentContext() );
    uno::Reference< io::XInputStream > xSourceInput = aSourceContent.openStream();

    if ( !xSourceInput.is() )
        throw io::IOException(); // TODO: error handling

    uno::Reference< io::XStream > xSubStream = xDest->openStreamElement(
                                                aNewEntryName,
                                                embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
    if ( !xSubStream.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XOutputStream > xDestOutput = xSubStream->getOutputStream();
    if ( !xDestOutput.is() )
        throw uno::RuntimeException();

    ::comphelper::OStorageHelper::CopyInputToOutput( xSourceInput, xDestOutput );
    xDestOutput->closeOutput();
}

void FSStorage::CopyContentToStorage_Impl( ::ucbhelper::Content* pContent, const uno::Reference< embed::XStorage >& xDest )
{
    if ( !pContent )
        throw uno::RuntimeException();

    // get list of contents of the Content
    // create cursor for access to children
    uno::Sequence< OUString > aProps( 2 );
    OUString* pProps = aProps.getArray();
    pProps[0] = "TargetURL";
    pProps[1] = "IsFolder";
    ::ucbhelper::ResultSetInclude eInclude = ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS;

    try
    {
        uno::Reference< sdbc::XResultSet > xResultSet = pContent->createCursor( aProps, eInclude );
        uno::Reference< ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );
        uno::Reference< sdbc::XRow > xRow( xResultSet, uno::UNO_QUERY );
        if ( xResultSet.is() )
        {
            // go through the list: insert files as streams, insert folders as substorages using recursion
            while ( xResultSet->next() )
            {
                OUString aSourceURL( xRow->getString( 1 ) );
                bool bIsFolder( xRow->getBoolean(2) );

                // TODO/LATER: not sure whether the entry name must be encoded
                OUString aNewEntryName( INetURLObject( aSourceURL ).getName( INetURLObject::LAST_SEGMENT,
                                                                                    true,
                                                                                    INetURLObject::NO_DECODE ) );
                if ( bIsFolder )
                {
                    uno::Reference< embed::XStorage > xSubStorage = xDest->openStorageElement( aNewEntryName,
                                                                                                embed::ElementModes::READWRITE );
                    if ( !xSubStorage.is() )
                        throw uno::RuntimeException();

                    uno::Reference< ucb::XCommandEnvironment > xDummyEnv;
                    ::ucbhelper::Content aSourceContent( aSourceURL, xDummyEnv, comphelper::getProcessComponentContext() );
                    CopyContentToStorage_Impl( &aSourceContent, xSubStorage );
                }
                else
                {
                    CopyStreamToSubStream( aSourceURL, xDest, aNewEntryName );
                }
            }
        }

        uno::Reference< embed::XTransactedObject > xTransact( xDest, uno::UNO_QUERY );
        if ( xTransact.is() )
            xTransact->commit();
    }
    catch( ucb::InteractiveIOException& r )
    {
        if ( r.Code == ucb::IOErrorCode_NOT_EXISTING )
            OSL_FAIL( "The folder does not exist!\n" );
        else
            throw;
    }
}

//  XInterface

uno::Any SAL_CALL FSStorage::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException, std::exception )
{
    uno::Any aReturn;
    aReturn <<= ::cppu::queryInterface
                (   rType
                ,   static_cast<lang::XTypeProvider*> ( this )
                ,   static_cast<embed::XStorage*> ( this )
                ,   static_cast<embed::XHierarchicalStorageAccess*> ( this )
                ,   static_cast<container::XNameAccess*> ( this )
                ,   static_cast<container::XElementAccess*> ( this )
                ,   static_cast<lang::XComponent*> ( this )
                ,   static_cast<beans::XPropertySet*> ( this ) );

    if ( aReturn.hasValue() )
        return aReturn ;

    return OWeakObject::queryInterface( rType );
}

void SAL_CALL FSStorage::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL FSStorage::release() throw()
{
    OWeakObject::release();
}

//  XTypeProvider

uno::Sequence< uno::Type > SAL_CALL FSStorage::getTypes()
        throw( uno::RuntimeException, std::exception )
{
    if ( m_pImpl->m_pTypeCollection == nullptr )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_pImpl->m_pTypeCollection == nullptr )
        {
            m_pImpl->m_pTypeCollection = new ::cppu::OTypeCollection
                                (   cppu::UnoType<lang::XTypeProvider>::get()
                                ,   cppu::UnoType<embed::XStorage>::get()
                                ,   cppu::UnoType<embed::XHierarchicalStorageAccess>::get()
                                ,   cppu::UnoType<beans::XPropertySet>::get());
        }
    }

    return m_pImpl->m_pTypeCollection->getTypes() ;
}

uno::Sequence< sal_Int8 > SAL_CALL FSStorage::getImplementationId()
        throw( uno::RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}

//  XStorage

void SAL_CALL FSStorage::copyToStorage( const uno::Reference< embed::XStorage >& xDest )
        throw ( embed::InvalidStorageException,
                io::IOException,
                lang::IllegalArgumentException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !xDest.is() || xDest == uno::Reference< uno::XInterface >( static_cast< OWeakObject*> ( this ), uno::UNO_QUERY ) )
        throw lang::IllegalArgumentException(); // TODO:

    if ( !GetContent() )
        throw io::IOException(); // TODO: error handling

    try
    {
        CopyContentToStorage_Impl( GetContent(), xDest );
    }
    catch( embed::InvalidStorageException& )
    {
        throw;
    }
    catch( lang::IllegalArgumentException& )
    {
        throw;
    }
    catch( embed::StorageWrappedTargetException& )
    {
        throw;
    }
    catch( io::IOException& )
    {
        throw;
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException("Can't copy raw stream",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

uno::Reference< io::XStream > SAL_CALL FSStorage::openStreamElement(
    const OUString& aStreamName, sal_Int32 nOpenMode )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !GetContent() )
        throw io::IOException(); // TODO: error handling

    // TODO/LATER: may need possibility to create folder if it was removed, since the folder can not be locked
    INetURLObject aFileURL( m_pImpl->m_aURL );
    aFileURL.Append( aStreamName );

    if ( ::utl::UCBContentHelper::IsFolder( aFileURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        throw io::IOException();

    if ( ( nOpenMode & embed::ElementModes::NOCREATE )
      && !::utl::UCBContentHelper::IsDocument( aFileURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        throw io::IOException(); // TODO:

    uno::Reference< ucb::XCommandEnvironment > xDummyEnv; // TODO: provide InteractionHandler if any
    uno::Reference< io::XStream > xResult;
    try
    {
        if ( nOpenMode & embed::ElementModes::WRITE )
        {
            if ( aFileURL.GetProtocol() == INetProtocol::File )
            {
                uno::Reference<ucb::XSimpleFileAccess3> xSimpleFileAccess(
                    ucb::SimpleFileAccess::create( m_pImpl->m_xContext ) );
                xResult = xSimpleFileAccess->openFileReadWrite( aFileURL.GetMainURL( INetURLObject::NO_DECODE ) );
            }
            else
            {
                // TODO: test whether it really works for http and fwp
                SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aFileURL.GetMainURL( INetURLObject::NO_DECODE ),
                                                                          STREAM_STD_WRITE );
                if ( pStream )
                {
                    if ( !pStream->GetError() )
                        xResult.set( new ::utl::OStreamWrapper( *pStream ) );
                    else
                        delete pStream;
                }
            }

            if ( !xResult.is() )
                throw io::IOException();

            if ( ( nOpenMode & embed::ElementModes::TRUNCATE ) )
            {
                uno::Reference< io::XTruncate > xTrunc( xResult->getOutputStream(), uno::UNO_QUERY_THROW );
                xTrunc->truncate();
            }
        }
        else
        {
            if ( ( nOpenMode & embed::ElementModes::TRUNCATE )
              || !::utl::UCBContentHelper::IsDocument( aFileURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
                throw io::IOException(); // TODO: access denied

            ::ucbhelper::Content aResultContent( aFileURL.GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv, comphelper::getProcessComponentContext() );
            uno::Reference< io::XInputStream > xInStream = aResultContent.openStream();
            xResult = static_cast< io::XStream* >( new OFSInputStreamContainer( xInStream ) );
        }
    }
    catch( embed::InvalidStorageException& )
    {
        throw;
    }
    catch( lang::IllegalArgumentException& )
    {
        throw;
    }
    catch( packages::WrongPasswordException& )
    {
        throw;
    }
    catch( embed::StorageWrappedTargetException& )
    {
        throw;
    }
    catch( io::IOException& )
    {
        throw;
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException("Can't copy raw stream",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    return xResult;
}

uno::Reference< io::XStream > SAL_CALL FSStorage::openEncryptedStreamElement(
    const OUString&, sal_Int32, const OUString& )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    throw packages::NoEncryptionException();
}

uno::Reference< embed::XStorage > SAL_CALL FSStorage::openStorageElement(
            const OUString& aStorName, sal_Int32 nStorageMode )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !GetContent() )
        throw io::IOException(); // TODO: error handling

    if ( ( nStorageMode & embed::ElementModes::WRITE )
      && !( m_pImpl->m_nMode & embed::ElementModes::WRITE ) )
          throw io::IOException(); // TODO: error handling

    // TODO/LATER: may need possibility to create folder if it was removed, since the folder can not be locked
    INetURLObject aFolderURL( m_pImpl->m_aURL );
    aFolderURL.Append( aStorName );

    bool bFolderExists = ::utl::UCBContentHelper::IsFolder( aFolderURL.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( !bFolderExists && ::utl::UCBContentHelper::IsDocument( aFolderURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        throw io::IOException(); // TODO:

    if ( ( nStorageMode & embed::ElementModes::NOCREATE ) && !bFolderExists )
        throw io::IOException(); // TODO:

    uno::Reference< ucb::XCommandEnvironment > xDummyEnv; // TODO: provide InteractionHandler if any
    uno::Reference< embed::XStorage > xResult;
    try
    {
        if ( nStorageMode & embed::ElementModes::WRITE )
        {
            if ( ( nStorageMode & embed::ElementModes::TRUNCATE ) && bFolderExists )
            {
                ::utl::UCBContentHelper::Kill( aFolderURL.GetMainURL( INetURLObject::NO_DECODE ) );
                bFolderExists =
                    MakeFolderNoUI( aFolderURL.GetMainURL( INetURLObject::NO_DECODE ) ); // TODO: not atomic :(
            }
            else if ( !bFolderExists )
            {
                bFolderExists =
                    MakeFolderNoUI( aFolderURL.GetMainURL( INetURLObject::NO_DECODE ) ); // TODO: not atomic :(
            }
        }
        else if ( ( nStorageMode & embed::ElementModes::TRUNCATE ) )
            throw io::IOException(); // TODO: access denied

        if ( !bFolderExists )
            throw io::IOException(); // there is no such folder

        ::ucbhelper::Content aResultContent( aFolderURL.GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv, comphelper::getProcessComponentContext() );
        xResult.set( static_cast< OWeakObject* >( new FSStorage( aResultContent,
                                                                 nStorageMode,
                                                                 m_pImpl->m_xContext ) ),
                     uno::UNO_QUERY );
    }
    catch( embed::InvalidStorageException& )
    {
        throw;
    }
    catch( lang::IllegalArgumentException& )
    {
        throw;
    }
    catch( embed::StorageWrappedTargetException& )
    {
        throw;
    }
    catch( io::IOException& )
    {
        throw;
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException("Can't copy raw stream",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    return xResult;
}

uno::Reference< io::XStream > SAL_CALL FSStorage::cloneStreamElement( const OUString& aStreamName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !GetContent() )
        throw io::IOException(); // TODO: error handling

    // TODO/LATER: may need possibility to create folder if it was removed, since the folder can not be locked
    INetURLObject aFileURL( m_pImpl->m_aURL );
    aFileURL.Append( aStreamName );

    uno::Reference < io::XStream > xTempResult;
    try
    {
        uno::Reference< ucb::XCommandEnvironment > xDummyEnv;
        ::ucbhelper::Content aResultContent( aFileURL.GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv, comphelper::getProcessComponentContext() );
        uno::Reference< io::XInputStream > xInStream = aResultContent.openStream();

        xTempResult = io::TempFile::create(m_pImpl->m_xContext);
        uno::Reference < io::XOutputStream > xTempOut = xTempResult->getOutputStream();
        uno::Reference < io::XInputStream > xTempIn = xTempResult->getInputStream();

        if ( !xTempOut.is() || !xTempIn.is() )
            throw io::IOException();

        ::comphelper::OStorageHelper::CopyInputToOutput( xInStream, xTempOut );
        xTempOut->closeOutput();
    }
    catch( embed::InvalidStorageException& )
    {
        throw;
    }
    catch( lang::IllegalArgumentException& )
    {
        throw;
    }
    catch( packages::WrongPasswordException& )
    {
        throw;
    }
    catch( io::IOException& )
    {
        throw;
    }
    catch( embed::StorageWrappedTargetException& )
    {
        throw;
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException("Can't copy raw stream",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    return xTempResult;
}

uno::Reference< io::XStream > SAL_CALL FSStorage::cloneEncryptedStreamElement(
    const OUString&,
    const OUString& )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    throw packages::NoEncryptionException();
}

void SAL_CALL FSStorage::copyLastCommitTo(
            const uno::Reference< embed::XStorage >& xTargetStorage )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    copyToStorage( xTargetStorage );
}

void SAL_CALL FSStorage::copyStorageElementLastCommitTo(
            const OUString& aStorName,
            const uno::Reference< embed::XStorage >& xTargetStorage )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    uno::Reference< embed::XStorage > xSourceStor( openStorageElement( aStorName, embed::ElementModes::READ ),
                                                    uno::UNO_QUERY_THROW );
    xSourceStor->copyToStorage( xTargetStorage );
}

sal_Bool SAL_CALL FSStorage::isStreamElement( const OUString& aElementName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !GetContent() )
        throw embed::InvalidStorageException(); // TODO: error handling

    INetURLObject aURL( m_pImpl->m_aURL );
    aURL.Append( aElementName );

    return !::utl::UCBContentHelper::IsFolder( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
}

sal_Bool SAL_CALL FSStorage::isStorageElement( const OUString& aElementName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !GetContent() )
        throw embed::InvalidStorageException(); // TODO: error handling

    INetURLObject aURL( m_pImpl->m_aURL );
    aURL.Append( aElementName );

    return ::utl::UCBContentHelper::IsFolder( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
}

void SAL_CALL FSStorage::removeElement( const OUString& aElementName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !GetContent() )
        throw io::IOException(); // TODO: error handling

    INetURLObject aURL( m_pImpl->m_aURL );
    aURL.Append( aElementName );

    if ( !::utl::UCBContentHelper::IsFolder( aURL.GetMainURL( INetURLObject::NO_DECODE ) )
      && !::utl::UCBContentHelper::IsDocument( aURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        throw container::NoSuchElementException(); // TODO:

    ::utl::UCBContentHelper::Kill( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
}

void SAL_CALL FSStorage::renameElement( const OUString& aElementName, const OUString& aNewName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !GetContent() )
        throw io::IOException(); // TODO: error handling

    INetURLObject aOldURL( m_pImpl->m_aURL );
    aOldURL.Append( aElementName );

    INetURLObject aNewURL( m_pImpl->m_aURL );
    aNewURL.Append( aNewName );

    if ( !::utl::UCBContentHelper::IsFolder( aOldURL.GetMainURL( INetURLObject::NO_DECODE ) )
      && !::utl::UCBContentHelper::IsDocument( aOldURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        throw container::NoSuchElementException(); // TODO:

    if ( ::utl::UCBContentHelper::IsFolder( aNewURL.GetMainURL( INetURLObject::NO_DECODE ) )
      || ::utl::UCBContentHelper::IsDocument( aNewURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
          throw container::ElementExistException(); // TODO:

    try
    {
        uno::Reference< ucb::XCommandEnvironment > xDummyEnv;
        ::ucbhelper::Content aSourceContent( aOldURL.GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv, comphelper::getProcessComponentContext() );

        if ( !GetContent()->transferContent( aSourceContent,
                                            ::ucbhelper::InsertOperation_MOVE,
                                            aNewName,
                                            ucb::NameClash::ERROR ) )
            throw io::IOException(); // TODO: error handling
    }
    catch( embed::InvalidStorageException& )
    {
        throw;
    }
    catch( lang::IllegalArgumentException& )
    {
        throw;
    }
    catch( container::NoSuchElementException& )
    {
        throw;
    }
    catch( container::ElementExistException& )
    {
        throw;
    }
    catch( io::IOException& )
    {
        throw;
    }
    catch( embed::StorageWrappedTargetException& )
    {
        throw;
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException("Can't copy raw stream",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

void SAL_CALL FSStorage::copyElementTo( const OUString& aElementName,
                                        const uno::Reference< embed::XStorage >& xDest,
                                        const OUString& aNewName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !xDest.is() )
        throw uno::RuntimeException();

    if ( !GetContent() )
        throw io::IOException(); // TODO: error handling

    INetURLObject aOwnURL( m_pImpl->m_aURL );
    aOwnURL.Append( aElementName );

    if ( xDest->hasByName( aNewName ) )
          throw container::ElementExistException(); // TODO:

    try
    {
        uno::Reference< ucb::XCommandEnvironment > xDummyEnv;
        if ( ::utl::UCBContentHelper::IsFolder( aOwnURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        {
            ::ucbhelper::Content aSourceContent( aOwnURL.GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv, comphelper::getProcessComponentContext() );
            uno::Reference< embed::XStorage > xDestSubStor(
                                    xDest->openStorageElement( aNewName, embed::ElementModes::READWRITE ),
                                    uno::UNO_QUERY_THROW );

            CopyContentToStorage_Impl( &aSourceContent, xDestSubStor );
        }
        else if ( ::utl::UCBContentHelper::IsDocument( aOwnURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        {
            CopyStreamToSubStream( aOwnURL.GetMainURL( INetURLObject::NO_DECODE ), xDest, aNewName );
        }
        else
            throw container::NoSuchElementException(); // TODO:
    }
    catch( embed::InvalidStorageException& )
    {
        throw;
    }
    catch( lang::IllegalArgumentException& )
    {
        throw;
    }
    catch( container::NoSuchElementException& )
    {
        throw;
    }
    catch( container::ElementExistException& )
    {
        throw;
    }
    catch( embed::StorageWrappedTargetException& )
    {
        throw;
    }
    catch( io::IOException& )
    {
        throw;
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException("Can't copy raw stream",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

void SAL_CALL FSStorage::moveElementTo( const OUString& aElementName,
                                        const uno::Reference< embed::XStorage >& xDest,
                                        const OUString& aNewName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    copyElementTo( aElementName, xDest, aNewName );

    INetURLObject aOwnURL( m_pImpl->m_aURL );
    aOwnURL.Append( aElementName );
    if ( !::utl::UCBContentHelper::Kill( aOwnURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        throw io::IOException(); // TODO: error handling
}

//  XNameAccess

uno::Any SAL_CALL FSStorage::getByName( const OUString& aName )
        throw ( container::NoSuchElementException,
                lang::WrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( aName.isEmpty() )
        throw lang::IllegalArgumentException();

    uno::Any aResult;
    try
    {
        if ( !GetContent() )
            throw io::IOException(); // TODO: error handling

        INetURLObject aURL( m_pImpl->m_aURL );
        aURL.Append( aName );


        if ( ::utl::UCBContentHelper::IsFolder( aURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        {
            aResult <<= openStorageElement( aName, embed::ElementModes::READ );
        }
        else if ( ::utl::UCBContentHelper::IsDocument( aURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        {
            aResult <<= openStreamElement( aName, embed::ElementModes::READ );
        }
        else
            throw container::NoSuchElementException(); // TODO:
    }
    catch (const container::NoSuchElementException&)
    {
        throw;
    }
    catch (const lang::WrappedTargetException&)
    {
        throw;
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetException( "Can not open element!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
    }

    return aResult;
}


uno::Sequence< OUString > SAL_CALL FSStorage::getElementNames()
        throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    uno::Sequence< OUString > aResult;

    try
    {
        if ( !GetContent() )
            throw io::IOException(); // TODO: error handling

        uno::Sequence<OUString> aProps { "Title" };
        ::ucbhelper::ResultSetInclude eInclude = ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS;

        sal_Int32 nSize = 0;
        uno::Reference< sdbc::XResultSet > xResultSet = GetContent()->createCursor( aProps, eInclude );
        uno::Reference< ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );
        uno::Reference< sdbc::XRow > xRow( xResultSet, uno::UNO_QUERY );
        if ( xResultSet.is() )
        {
            // go through the list
            while ( xResultSet->next() )
            {
                OUString aName( xRow->getString( 1 ) );
                aResult.realloc( ++nSize );
                aResult[nSize-1] = aName;
            }
        }
    }
    catch( const ucb::InteractiveIOException& r )
    {
        if ( r.Code == ucb::IOErrorCode_NOT_EXISTING )
            OSL_FAIL( "The folder does not exist!\n" );
        else
        {
            uno::Any aCaught( ::cppu::getCaughtException() );
            throw lang::WrappedTargetRuntimeException( "Can not open storage!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
        }
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException( "Can not open storage!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
    }

    return aResult;
}

sal_Bool SAL_CALL FSStorage::hasByName( const OUString& aName )
        throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    try
    {
        if ( !GetContent() )
            throw io::IOException(); // TODO: error handling

        if ( aName.isEmpty() )
            throw lang::IllegalArgumentException();
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch ( uno::Exception& )
    {
        uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException( "Can not open storage!",
                                            static_cast< OWeakObject* >( this ),
                                            aCaught );
    }

    INetURLObject aURL( m_pImpl->m_aURL );
    aURL.Append( aName );

    return ( ::utl::UCBContentHelper::IsFolder( aURL.GetMainURL( INetURLObject::NO_DECODE ) )
      || ::utl::UCBContentHelper::IsDocument( aURL.GetMainURL( INetURLObject::NO_DECODE ) ) );
}

uno::Type SAL_CALL FSStorage::getElementType()
        throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    // it is a multitype container
    return uno::Type();
}

sal_Bool SAL_CALL FSStorage::hasElements()
        throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    try
    {
        if ( !GetContent() )
            throw io::IOException(); // TODO: error handling

        uno::Sequence<OUString> aProps { "TargetURL" };
        ::ucbhelper::ResultSetInclude eInclude = ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS;

        uno::Reference< sdbc::XResultSet > xResultSet = GetContent()->createCursor( aProps, eInclude );
        return ( xResultSet.is() && xResultSet->next() );
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
        throw uno::RuntimeException();
    }
}

//  XDisposable
void SAL_CALL FSStorage::dispose()
        throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( m_pImpl->m_pListenersContainer )
    {
        lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
        m_pImpl->m_pListenersContainer->disposeAndClear( aSource );
    }

    delete m_pImpl;
    m_pImpl = nullptr;
}

void SAL_CALL FSStorage::addEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_pImpl->m_pListenersContainer )
        m_pImpl->m_pListenersContainer = new ::cppu::OInterfaceContainerHelper( m_aMutex );

    m_pImpl->m_pListenersContainer->addInterface( xListener );
}

void SAL_CALL FSStorage::removeEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( m_pImpl->m_pListenersContainer )
        m_pImpl->m_pListenersContainer->removeInterface( xListener );
}

//  XPropertySet

uno::Reference< beans::XPropertySetInfo > SAL_CALL FSStorage::getPropertySetInfo()
        throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
    return uno::Reference< beans::XPropertySetInfo >();
}


void SAL_CALL FSStorage::setPropertyValue( const OUString& aPropertyName, const uno::Any& )
        throw ( beans::UnknownPropertyException,
                beans::PropertyVetoException,
                lang::IllegalArgumentException,
                lang::WrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( aPropertyName == "URL" || aPropertyName == "OpenMode" )
        throw beans::PropertyVetoException(); // TODO
    else
        throw beans::UnknownPropertyException(); // TODO
}


uno::Any SAL_CALL FSStorage::getPropertyValue( const OUString& aPropertyName )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( aPropertyName == "URL" )
        return uno::makeAny( m_pImpl->m_aURL );
    else if ( aPropertyName == "OpenMode" )
        return uno::makeAny( m_pImpl->m_nMode );

    throw beans::UnknownPropertyException(); // TODO
}


void SAL_CALL FSStorage::addPropertyChangeListener(
            const OUString& /*aPropertyName*/,
            const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


void SAL_CALL FSStorage::removePropertyChangeListener(
            const OUString& /*aPropertyName*/,
            const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


void SAL_CALL FSStorage::addVetoableChangeListener(
            const OUString& /*PropertyName*/,
            const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


void SAL_CALL FSStorage::removeVetoableChangeListener(
            const OUString& /*PropertyName*/,
            const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}

//  XHierarchicalStorageAccess
uno::Reference< embed::XExtendedStorageStream > SAL_CALL FSStorage::openStreamElementByHierarchicalName( const OUString& sStreamPath, ::sal_Int32 nOpenMode )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( sStreamPath.toChar() == '/' )
        throw lang::IllegalArgumentException();

    if ( !GetContent() )
        throw io::IOException(); // TODO: error handling

    INetURLObject aBaseURL( m_pImpl->m_aURL );
    if ( !aBaseURL.setFinalSlash() )
        throw uno::RuntimeException();

    OUString aFileURL = INetURLObject::GetAbsURL(
                aBaseURL.GetMainURL( INetURLObject::NO_DECODE ),
                sStreamPath );

    if ( ::utl::UCBContentHelper::IsFolder( aFileURL ) )
        throw io::IOException();

    if ( ( nOpenMode & embed::ElementModes::NOCREATE )
      && !::utl::UCBContentHelper::IsDocument( aFileURL ) )
        throw io::IOException(); // TODO:

    uno::Reference< ucb::XCommandEnvironment > xDummyEnv; // TODO: provide InteractionHandler if any
    uno::Reference< io::XStream > xResult;
    try
    {
        if ( nOpenMode & embed::ElementModes::WRITE )
        {
            if ( comphelper::isFileUrl( aFileURL ) )
            {
                uno::Reference<ucb::XSimpleFileAccess3> xSimpleFileAccess(
                    ucb::SimpleFileAccess::create( m_pImpl->m_xContext ) );
                uno::Reference< io::XStream > xStream =
                    xSimpleFileAccess->openFileReadWrite( aFileURL );

                xResult = static_cast< io::XStream* >( new OFSStreamContainer( xStream ) );
            }
            else
            {
                // TODO: test whether it really works for http and fwp
                SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aFileURL,
                                                                          STREAM_STD_WRITE );
                if ( pStream )
                {
                    if ( !pStream->GetError() )
                    {
                        uno::Reference< io::XStream > xStream =
                            uno::Reference < io::XStream >( new ::utl::OStreamWrapper( *pStream ) );
                        xResult = static_cast< io::XStream* >( new OFSStreamContainer( xStream ) );
                    }
                    else
                        delete pStream;
                }
            }

            if ( !xResult.is() )
                throw io::IOException();

            if ( ( nOpenMode & embed::ElementModes::TRUNCATE ) )
            {
                uno::Reference< io::XTruncate > xTrunc( xResult->getOutputStream(), uno::UNO_QUERY_THROW );
                xTrunc->truncate();
            }
        }
        else
        {
            if ( ( nOpenMode & embed::ElementModes::TRUNCATE )
              || !::utl::UCBContentHelper::IsDocument( aFileURL ) )
                throw io::IOException(); // TODO: access denied

            ::ucbhelper::Content aResultContent( aFileURL, xDummyEnv, comphelper::getProcessComponentContext() );
            uno::Reference< io::XInputStream > xInStream = aResultContent.openStream();
            xResult = static_cast< io::XStream* >( new OFSInputStreamContainer( xInStream ) );
        }
    }
    catch( embed::InvalidStorageException& )
    {
        throw;
    }
    catch( lang::IllegalArgumentException& )
    {
        throw;
    }
    catch( packages::WrongPasswordException& )
    {
        throw;
    }
    catch( embed::StorageWrappedTargetException& )
    {
        throw;
    }
    catch( io::IOException& )
    {
        throw;
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
          uno::Any aCaught( ::cppu::getCaughtException() );
        throw embed::StorageWrappedTargetException("Can't copy raw stream",
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    return uno::Reference< embed::XExtendedStorageStream >( xResult, uno::UNO_QUERY_THROW );
}

uno::Reference< embed::XExtendedStorageStream > SAL_CALL FSStorage::openEncryptedStreamElementByHierarchicalName( const OUString& /*sStreamName*/, ::sal_Int32 /*nOpenMode*/, const OUString& /*sPassword*/ )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    throw packages::NoEncryptionException();
}

void SAL_CALL FSStorage::removeStreamElementByHierarchicalName( const OUString& sStreamPath )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !GetContent() )
        throw io::IOException(); // TODO: error handling

    // TODO/LATER: may need possibility to create folder if it was removed, since the folder can not be locked
    INetURLObject aBaseURL( m_pImpl->m_aURL );
    if ( !aBaseURL.setFinalSlash() )
        throw uno::RuntimeException();

    OUString aFileURL = INetURLObject::GetAbsURL(
                aBaseURL.GetMainURL( INetURLObject::NO_DECODE ),
                sStreamPath );

    if ( !::utl::UCBContentHelper::IsDocument( aFileURL ) )
    {
        if ( ::utl::UCBContentHelper::IsFolder( aFileURL ) )
            throw lang::IllegalArgumentException();
        else
            throw container::NoSuchElementException(); // TODO:
    }

    if ( !::utl::UCBContentHelper::Kill( aFileURL ) )
        throw io::IOException(); // TODO: error handling
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
