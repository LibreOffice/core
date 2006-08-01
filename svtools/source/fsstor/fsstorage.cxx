/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fsstorage.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 11:15:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XPROGRESSHANDLER_HPP_
#include <com/sun/star/ucb/XProgressHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_INTERACTIVEIODEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_IOERRORCODE_HPP_
#include <com/sun/star/ucb/IOErrorCode.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETRUNTIMEEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif

#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif

#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif


#ifndef _COMPHELPER_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COMPHELPER_STORAGEHELPER_HXX
#include <comphelper/storagehelper.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
#endif

#include <tools/urlobj.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <ucbhelper/fileidentifierconverter.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/content.hxx>

#include "fsstorage.hxx"
#include "oinputstreamcontainer.hxx"

using namespace ::com::sun::star;

//=========================================================

// TODO: move to a standard helper
sal_Bool isLocalFile_Impl( ::rtl::OUString aURL )
{
    ::rtl::OUString aSystemPath;
    ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
    if ( !pBroker )
        throw uno::RuntimeException();

    uno::Reference< ::com::sun::star::ucb::XContentProviderManager > xManager =
                pBroker->getContentProviderManagerInterface();
    try
    {
        aSystemPath = ::ucb::getSystemPathFromFileURL( xManager, aURL );
    }
    catch ( uno::Exception& )
    {
    }

    return ( aSystemPath.getLength() != 0 );
}


//=========================================================

struct FSStorage_Impl
{
    ::rtl::OUString m_aURL;

    ::ucb::Content* m_pContent;
    sal_Int32 m_nMode;

    ::cppu::OInterfaceContainerHelper* m_pListenersContainer; // list of listeners
    ::cppu::OTypeCollection* m_pTypeCollection;

    uno::Reference< lang::XMultiServiceFactory > m_xFactory;


    FSStorage_Impl( const ::rtl::OUString& aURL, sal_Int32 nMode, uno::Reference< lang::XMultiServiceFactory > xFactory )
    : m_aURL( aURL )
    , m_pContent( NULL )
    , m_nMode( nMode )
    , m_pListenersContainer( NULL )
    , m_pTypeCollection( NULL )
    , m_xFactory( xFactory )
    {
        OSL_ENSURE( m_aURL.getLength(), "The URL must not be empty" );
    }

    FSStorage_Impl( const ::ucb::Content& aContent, sal_Int32 nMode, uno::Reference< lang::XMultiServiceFactory > xFactory )
    : m_aURL( aContent.getURL() )
    , m_pContent( new ::ucb::Content( aContent ) )
    , m_nMode( nMode )
    , m_pListenersContainer( NULL )
    , m_pTypeCollection( NULL )
    , m_xFactory( xFactory )
    {
        OSL_ENSURE( m_aURL.getLength(), "The URL must not be empty" );
    }

    ~FSStorage_Impl();
};

//=========================================================

FSStorage_Impl::~FSStorage_Impl()
{
    if ( m_pListenersContainer )
        delete m_pListenersContainer;
    if ( m_pTypeCollection )
        delete m_pTypeCollection;
    if ( m_pContent )
        delete m_pContent;
}

//=====================================================
// FSStorage implementation
//=====================================================

//-----------------------------------------------
FSStorage::FSStorage( const ::ucb::Content& aContent,
                    sal_Int32 nMode,
                    uno::Sequence< beans::PropertyValue >,
                    uno::Reference< lang::XMultiServiceFactory > xFactory )
: m_pImpl( new FSStorage_Impl( aContent, nMode, xFactory ) )
{
    // TODO: use properties
    if ( !xFactory.is() )
        throw uno::RuntimeException();

    GetContent();
}

//-----------------------------------------------
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

//-----------------------------------------------
sal_Bool FSStorage::MakeFolderNoUI( const String& rFolder, sal_Bool )
{
       INetURLObject aURL( rFolder );
    ::rtl::OUString aTitle = aURL.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
    aURL.removeSegment();
    ::ucb::Content aParent;
    ::ucb::Content aResultContent;

       if ( ::ucb::Content::create( aURL.GetMainURL( INetURLObject::NO_DECODE ),
                                 uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
                                 aParent ) )
        return ::utl::UCBContentHelper::MakeFolder( aParent, aTitle, aResultContent, sal_False );

    return sal_False;
}

//-----------------------------------------------
::ucb::Content* FSStorage::GetContent()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_pImpl->m_pContent )
    {
        uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;

        try
        {
            m_pImpl->m_pContent = new ::ucb::Content( m_pImpl->m_aURL, xDummyEnv );
        }
        catch( uno::Exception& )
        {
        }
    }

    return m_pImpl->m_pContent;
}

//-----------------------------------------------
void FSStorage::CopyStreamToSubStream( const ::rtl::OUString& aSourceURL,
                                        const uno::Reference< embed::XStorage >& xDest,
                                        const ::rtl::OUString& aNewEntryName )
{
    if ( !xDest.is() )
        throw uno::RuntimeException();

    uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;
    ::ucb::Content aSourceContent( aSourceURL, xDummyEnv );
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

//-----------------------------------------------
void FSStorage::CopyContentToStorage_Impl( ::ucb::Content* pContent, const uno::Reference< embed::XStorage >& xDest )
{
    if ( !pContent )
        throw uno::RuntimeException();

    // get list of contents of the Content
    // create cursor for access to children
    uno::Sequence< ::rtl::OUString > aProps( 2 );
    ::rtl::OUString* pProps = aProps.getArray();
    pProps[0] = ::rtl::OUString::createFromAscii( "TargetURL" );
    pProps[1] = ::rtl::OUString::createFromAscii( "IsFolder" );
    ::ucb::ResultSetInclude eInclude = ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS;

    try
    {
        uno::Reference< sdbc::XResultSet > xResultSet = pContent->createCursor( aProps, eInclude );
        uno::Reference< ::com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );
        uno::Reference< sdbc::XRow > xRow( xResultSet, uno::UNO_QUERY );
        if ( xResultSet.is() )
        {
            // go through the list: insert files as streams, insert folders as substorages using recursion
            while ( xResultSet->next() )
            {
                ::rtl::OUString aSourceURL( xRow->getString( 1 ) );
                sal_Bool bIsFolder( xRow->getBoolean(2) );

                // TODO/LATER: not sure whether the entry name must be encoded
                ::rtl::OUString aNewEntryName( INetURLObject( aSourceURL ).getName( INetURLObject::LAST_SEGMENT,
                                                                                    true,
                                                                                    INetURLObject::NO_DECODE ) );
                if ( bIsFolder )
                {
                    uno::Reference< embed::XStorage > xSubStorage = xDest->openStorageElement( aNewEntryName,
                                                                                                embed::ElementModes::READWRITE );
                    if ( !xSubStorage.is() )
                        throw uno::RuntimeException();

                    uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;
                    ::ucb::Content aSourceContent( aSourceURL, xDummyEnv );
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
    catch( ::com::sun::star::ucb::InteractiveIOException& r )
    {
        if ( r.Code == ::com::sun::star::ucb::IOErrorCode_NOT_EXISTING )
            OSL_ENSURE( sal_False, "The folder does not exist!\n" );
        else
            throw;
    }
}

//____________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Any SAL_CALL FSStorage::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
{
    uno::Any aReturn;
    aReturn <<= ::cppu::queryInterface
                (   rType
                ,   static_cast<lang::XTypeProvider*> ( this )
                ,   static_cast<embed::XStorage*> ( this )
                ,   static_cast<container::XNameAccess*> ( this )
                ,   static_cast<container::XElementAccess*> ( this )
                ,   static_cast<lang::XComponent*> ( this )
                ,   static_cast<beans::XPropertySet*> ( this ) );

    if ( aReturn.hasValue() == sal_True )
        return aReturn ;

    return OWeakObject::queryInterface( rType );
}

//-----------------------------------------------
void SAL_CALL FSStorage::acquire() throw()
{
    OWeakObject::acquire();
}

//-----------------------------------------------
void SAL_CALL FSStorage::release() throw()
{
    OWeakObject::release();
}

//____________________________________________________________________________________________________
//  XTypeProvider
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Sequence< uno::Type > SAL_CALL FSStorage::getTypes()
        throw( uno::RuntimeException )
{
    if ( m_pImpl->m_pTypeCollection == NULL )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( m_pImpl->m_pTypeCollection == NULL )
        {
            m_pImpl->m_pTypeCollection = new ::cppu::OTypeCollection
                                (   ::getCppuType( ( const uno::Reference< lang::XTypeProvider >* )NULL )
                                ,   ::getCppuType( ( const uno::Reference< embed::XStorage >* )NULL )
                                ,   ::getCppuType( ( const uno::Reference< beans::XPropertySet >* )NULL ) );
        }
    }

    return m_pImpl->m_pTypeCollection->getTypes() ;
}

//-----------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL FSStorage::getImplementationId()
        throw( uno::RuntimeException )
{
    static ::cppu::OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ) ;

        if ( pID == NULL )
        {
            static ::cppu::OImplementationId aID( sal_False ) ;
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;

}

//____________________________________________________________________________________________________
//  XStorage
//____________________________________________________________________________________________________

//-----------------------------------------------
void SAL_CALL FSStorage::copyToStorage( const uno::Reference< embed::XStorage >& xDest )
        throw ( embed::InvalidStorageException,
                io::IOException,
                lang::IllegalArgumentException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
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
        throw embed::StorageWrappedTargetException( ::rtl::OUString::createFromAscii( "Can't copy raw stream" ),
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL FSStorage::openStreamElement(
    const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
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

    uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv; // TODO: provide InteractionHandler if any
    uno::Reference< io::XStream > xResult;
    try
    {
        if ( nOpenMode & embed::ElementModes::WRITE )
        {
            if ( isLocalFile_Impl( aFileURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
            {
                uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xSimpleFileAccess(
                    m_pImpl->m_xFactory->createInstance(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.SimpleFileAccess" ) ) ),
                    uno::UNO_QUERY_THROW );
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
                        xResult = uno::Reference < io::XStream >( new ::utl::OStreamWrapper( *pStream ) );
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

            ::ucb::Content aResultContent( aFileURL.GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv );
            uno::Reference< io::XInputStream > xInStream = aResultContent.openStream();
            xResult = static_cast< io::XStream* >( new OInputStreamContainer( xInStream ) );
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
        throw embed::StorageWrappedTargetException( ::rtl::OUString::createFromAscii( "Can't copy raw stream" ),
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    return xResult;
}

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL FSStorage::openEncryptedStreamElement(
    const ::rtl::OUString&, sal_Int32, const ::rtl::OUString& )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    throw packages::NoEncryptionException();
}

//-----------------------------------------------
uno::Reference< embed::XStorage > SAL_CALL FSStorage::openStorageElement(
            const ::rtl::OUString& aStorName, sal_Int32 nStorageMode )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
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

    sal_Bool bFolderExists = ::utl::UCBContentHelper::IsFolder( aFolderURL.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( !bFolderExists && ::utl::UCBContentHelper::IsDocument( aFolderURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        throw io::IOException(); // TODO:

    if ( ( nStorageMode & embed::ElementModes::NOCREATE ) && !bFolderExists )
        throw io::IOException(); // TODO:

    uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv; // TODO: provide InteractionHandler if any
    uno::Reference< embed::XStorage > xResult;
    try
    {
        if ( nStorageMode & embed::ElementModes::WRITE )
        {
            if ( ( nStorageMode & embed::ElementModes::TRUNCATE ) && bFolderExists )
            {
                ::utl::UCBContentHelper::Kill( aFolderURL.GetMainURL( INetURLObject::NO_DECODE ) );
                bFolderExists =
                    MakeFolderNoUI( aFolderURL.GetMainURL( INetURLObject::NO_DECODE ), sal_True ); // TODO: not atomar :(
            }
            else if ( !bFolderExists )
            {
                bFolderExists =
                    MakeFolderNoUI( aFolderURL.GetMainURL( INetURLObject::NO_DECODE ), sal_True ); // TODO: not atomar :(
            }
        }
        else if ( ( nStorageMode & embed::ElementModes::TRUNCATE ) )
            throw io::IOException(); // TODO: access denied

        if ( !bFolderExists )
            throw io::IOException(); // there is no such folder

        ::ucb::Content aResultContent( aFolderURL.GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv );
        xResult = uno::Reference< embed::XStorage >(
                            static_cast< OWeakObject* >( new FSStorage( aResultContent,
                                                                        nStorageMode,
                                                                        uno::Sequence< beans::PropertyValue >(),
                                                                        m_pImpl->m_xFactory ) ),
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
        throw embed::StorageWrappedTargetException( ::rtl::OUString::createFromAscii( "Can't copy raw stream" ),
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    return xResult;
}

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL FSStorage::cloneStreamElement( const ::rtl::OUString& aStreamName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
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
        uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;
        ::ucb::Content aResultContent( aFileURL.GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv );
        uno::Reference< io::XInputStream > xInStream = aResultContent.openStream();

        xTempResult = uno::Reference < io::XStream >(
                    m_pImpl->m_xFactory->createInstance ( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
                    uno::UNO_QUERY_THROW );
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
        throw embed::StorageWrappedTargetException( ::rtl::OUString::createFromAscii( "Can't copy raw stream" ),
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }

    return xTempResult;
}

//-----------------------------------------------
uno::Reference< io::XStream > SAL_CALL FSStorage::cloneEncryptedStreamElement(
    const ::rtl::OUString&,
    const ::rtl::OUString& )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                packages::NoEncryptionException,
                packages::WrongPasswordException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    throw packages::NoEncryptionException();
}

//-----------------------------------------------
void SAL_CALL FSStorage::copyLastCommitTo(
            const uno::Reference< embed::XStorage >& xTargetStorage )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    copyToStorage( xTargetStorage );
}

//-----------------------------------------------
void SAL_CALL FSStorage::copyStorageElementLastCommitTo(
            const ::rtl::OUString& aStorName,
            const uno::Reference< embed::XStorage >& xTargetStorage )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    uno::Reference< embed::XStorage > xSourceStor( openStorageElement( aStorName, embed::ElementModes::READ ),
                                                    uno::UNO_QUERY_THROW );
    xSourceStor->copyToStorage( xTargetStorage );
}

//-----------------------------------------------
sal_Bool SAL_CALL FSStorage::isStreamElement( const ::rtl::OUString& aElementName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                uno::RuntimeException )
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

//-----------------------------------------------
sal_Bool SAL_CALL FSStorage::isStorageElement( const ::rtl::OUString& aElementName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                uno::RuntimeException )
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

//-----------------------------------------------
void SAL_CALL FSStorage::removeElement( const ::rtl::OUString& aElementName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
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

//-----------------------------------------------
void SAL_CALL FSStorage::renameElement( const ::rtl::OUString& aElementName, const ::rtl::OUString& aNewName )
        throw ( embed::InvalidStorageException,
                lang::IllegalArgumentException,
                container::NoSuchElementException,
                container::ElementExistException,
                io::IOException,
                embed::StorageWrappedTargetException,
                uno::RuntimeException )
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
        uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;
        ::ucb::Content aSourceContent( aOldURL.GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv );

        if ( !GetContent()->transferContent( aSourceContent,
                                            ::ucb::InsertOperation_MOVE,
                                            aNewName,
                                            ::com::sun::star::ucb::NameClash::ERROR ) )
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
        throw embed::StorageWrappedTargetException( ::rtl::OUString::createFromAscii( "Can't copy raw stream" ),
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

//-----------------------------------------------
void SAL_CALL FSStorage::copyElementTo( const ::rtl::OUString& aElementName,
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
        uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > xDummyEnv;
        if ( ::utl::UCBContentHelper::IsFolder( aOwnURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        {
            ::ucb::Content aSourceContent( aOwnURL.GetMainURL( INetURLObject::NO_DECODE ), xDummyEnv );
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
        throw embed::StorageWrappedTargetException( ::rtl::OUString::createFromAscii( "Can't copy raw stream" ),
                                                 uno::Reference< io::XInputStream >(),
                                                 aCaught );
    }
}

//-----------------------------------------------
void SAL_CALL FSStorage::moveElementTo( const ::rtl::OUString& aElementName,
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
    ::osl::MutexGuard aGuard( m_aMutex );
    copyElementTo( aElementName, xDest, aNewName );

    INetURLObject aOwnURL( m_pImpl->m_aURL );
    aOwnURL.Append( aElementName );
    if ( !::utl::UCBContentHelper::Kill( aOwnURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        throw io::IOException(); // TODO: error handling
}

//____________________________________________________________________________________________________
//  XNameAccess
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Any SAL_CALL FSStorage::getByName( const ::rtl::OUString& aName )
        throw ( container::NoSuchElementException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !GetContent() )
        throw io::IOException(); // TODO: error handling

    if ( !aName.getLength() )
        throw lang::IllegalArgumentException();

    INetURLObject aURL( m_pImpl->m_aURL );
    aURL.Append( aName );

    uno::Any aResult;
    try
    {
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
    catch( container::NoSuchElementException& )
    {
        throw;
    }
    catch( lang::WrappedTargetException& )
    {
        throw;
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch ( uno::Exception& )
    {
           uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetException( ::rtl::OUString::createFromAscii( "Can not open element!\n" ),
                                            uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                uno::UNO_QUERY ),
                                            aCaught );
    }

    return aResult;
}


//-----------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL FSStorage::getElementNames()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !GetContent() )
        throw io::IOException(); // TODO: error handling

    uno::Sequence< ::rtl::OUString > aProps( 1 );
    ::rtl::OUString* pProps = aProps.getArray();
    pProps[0] = ::rtl::OUString::createFromAscii( "Title" );
    ::ucb::ResultSetInclude eInclude = ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS;

    uno::Sequence< ::rtl::OUString > aResult;
    sal_Int32 nSize = 0;

    try
    {
        uno::Reference< sdbc::XResultSet > xResultSet = GetContent()->createCursor( aProps, eInclude );
        uno::Reference< ::com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );
        uno::Reference< sdbc::XRow > xRow( xResultSet, uno::UNO_QUERY );
        if ( xResultSet.is() )
        {
            // go through the list
            while ( xResultSet->next() )
            {
                ::rtl::OUString aName( xRow->getString( 1 ) );
                aResult.realloc( ++nSize );
                aResult[nSize-1] = aName;
            }
        }
    }
    catch( ::com::sun::star::ucb::InteractiveIOException& r )
    {
        if ( r.Code == ::com::sun::star::ucb::IOErrorCode_NOT_EXISTING )
            OSL_ENSURE( sal_False, "The folder does not exist!\n" );
        else
        {
               uno::Any aCaught( ::cppu::getCaughtException() );
            throw lang::WrappedTargetRuntimeException( ::rtl::OUString::createFromAscii( "Can not open storage!\n" ),
                                            uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                uno::UNO_QUERY ),
                                            aCaught );
        }
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch ( uno::Exception& )
    {
           uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException( ::rtl::OUString::createFromAscii( "Can not open storage!\n" ),
                                            uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                uno::UNO_QUERY ),
                                            aCaught );
    }

    return aResult;
}


//-----------------------------------------------
sal_Bool SAL_CALL FSStorage::hasByName( const ::rtl::OUString& aName )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    try
    {
        if ( !GetContent() )
            throw io::IOException(); // TODO: error handling

        if ( !aName.getLength() )
            throw lang::IllegalArgumentException();
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch ( uno::Exception& )
    {
           uno::Any aCaught( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException( ::rtl::OUString::createFromAscii( "Can not open storage!\n" ),
                                            uno::Reference< uno::XInterface >(  static_cast< OWeakObject* >( this ),
                                                                                uno::UNO_QUERY ),
                                            aCaught );
    }

    INetURLObject aURL( m_pImpl->m_aURL );
    aURL.Append( aName );

    return ( ::utl::UCBContentHelper::IsFolder( aURL.GetMainURL( INetURLObject::NO_DECODE ) )
      || ::utl::UCBContentHelper::IsDocument( aURL.GetMainURL( INetURLObject::NO_DECODE ) ) );
}

//-----------------------------------------------
uno::Type SAL_CALL FSStorage::getElementType()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    // it is a multitype container
    return uno::Type();
}

//-----------------------------------------------
sal_Bool SAL_CALL FSStorage::hasElements()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !GetContent() )
        throw io::IOException(); // TODO: error handling

    uno::Sequence< ::rtl::OUString > aProps( 1 );
    aProps[0] = ::rtl::OUString::createFromAscii( "TargetURL" );
    ::ucb::ResultSetInclude eInclude = ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS;

    try
    {
        uno::Reference< sdbc::XResultSet > xResultSet = GetContent()->createCursor( aProps, eInclude );
        return ( xResultSet.is() && xResultSet->next() );
    }
    catch( uno::Exception& )
    {
        throw uno::RuntimeException();
    }
}


//____________________________________________________________________________________________________
//  XDisposable
//____________________________________________________________________________________________________

//-----------------------------------------------
void SAL_CALL FSStorage::dispose()
        throw ( uno::RuntimeException )
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
    m_pImpl = NULL;
}

//-----------------------------------------------
void SAL_CALL FSStorage::addEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_pImpl->m_pListenersContainer )
        m_pImpl->m_pListenersContainer = new ::cppu::OInterfaceContainerHelper( m_aMutex );

    m_pImpl->m_pListenersContainer->addInterface( xListener );
}

//-----------------------------------------------
void SAL_CALL FSStorage::removeEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( m_pImpl->m_pListenersContainer )
        m_pImpl->m_pListenersContainer->removeInterface( xListener );
}

//____________________________________________________________________________________________________
//  XPropertySet
//____________________________________________________________________________________________________

//-----------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL FSStorage::getPropertySetInfo()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
    return uno::Reference< beans::XPropertySetInfo >();
}


//-----------------------------------------------
void SAL_CALL FSStorage::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& )
        throw ( beans::UnknownPropertyException,
                beans::PropertyVetoException,
                lang::IllegalArgumentException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( aPropertyName.equalsAscii( "URL" ) || aPropertyName.equalsAscii( "OpenMode" ) )
        throw beans::PropertyVetoException(); // TODO
    else
        throw beans::UnknownPropertyException(); // TODO
}


//-----------------------------------------------
uno::Any SAL_CALL FSStorage::getPropertyValue( const ::rtl::OUString& aPropertyName )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( aPropertyName.equalsAscii( "URL" ) )
        return uno::makeAny( m_pImpl->m_aURL );
    else if ( aPropertyName.equalsAscii( "OpenMode" ) )
        return uno::makeAny( m_pImpl->m_nMode );

    throw beans::UnknownPropertyException(); // TODO
}


//-----------------------------------------------
void SAL_CALL FSStorage::addPropertyChangeListener(
            const ::rtl::OUString& /*aPropertyName*/,
            const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


//-----------------------------------------------
void SAL_CALL FSStorage::removePropertyChangeListener(
            const ::rtl::OUString& /*aPropertyName*/,
            const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


//-----------------------------------------------
void SAL_CALL FSStorage::addVetoableChangeListener(
            const ::rtl::OUString& /*PropertyName*/,
            const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


//-----------------------------------------------
void SAL_CALL FSStorage::removeVetoableChangeListener(
            const ::rtl::OUString& /*PropertyName*/,
            const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


