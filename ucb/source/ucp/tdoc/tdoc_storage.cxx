/*************************************************************************
 *
 *  $RCSfile: tdoc_storage.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 17:40:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <memory>

#include "osl/mutex.hxx"

#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/embed/ElementModes.hpp"
#include "com/sun/star/embed/XTransactedObject.hpp"
#include "com/sun/star/io/IOException.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/reflection/XProxyFactory.hpp"
#include "com/sun/star/uno/XAggregation.hpp"

#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implbase2.hxx"

#include "tdoc_uri.hxx"
#include "tdoc_docmgr.hxx"

#include "tdoc_storage.hxx"

using namespace com::sun;
using namespace com::sun::star;

using namespace tdoc_ucp;

namespace tdoc_ucp
{
    struct MutexHolder
    {
        osl::Mutex m_aMutex;
    };

    //=======================================================================

    class ParentStorageHolder : public MutexHolder
    {
    public:
        ParentStorageHolder(
            const uno::Reference< embed::XStorage > & xParentStorage,
            const rtl::OUString & rUri );

        bool isRootParentStorage() const
        { return m_bParentStorageIsRoot; }
        uno::Reference< embed::XStorage > getParentStorage() const
        { return m_xParentStorage; }
        void setParentStorage( const uno::Reference< embed::XStorage > & xStg )
        { osl::MutexGuard aGuard( m_aMutex ); m_xParentStorage = xStg; }

    private:
        uno::Reference< embed::XStorage > m_xParentStorage;
        bool                              m_bParentStorageIsRoot;
    };

    //=======================================================================

    typedef
        cppu::WeakImplHelper2<
            com::sun::star::embed::XStorage,
            com::sun::star::embed::XTransactedObject > StorageUNOBase;

    class Storage : public StorageUNOBase, public ParentStorageHolder
    {
    public:
        Storage(
            const uno::Reference< lang::XMultiServiceFactory > & xSMgr,
            const rtl::Reference< StorageElementFactory >  & xFactory,
            const rtl::OUString & rUri,
            const uno::Reference< embed::XStorage > & xParentStorage,
            const uno::Reference< embed::XStorage > & xStorageToWrap );
        virtual ~Storage();

        // XInterface
        virtual uno::Any SAL_CALL queryInterface( const uno::Type& aType )
            throw ( uno::RuntimeException );
        virtual void SAL_CALL acquire()
            throw ();
        virtual void SAL_CALL release()
            throw ();

        // XStorage
        virtual void SAL_CALL
        copyToStorage( const uno::Reference< embed::XStorage >& xDest )
            throw ( embed::InvalidStorageException,
                    lang::IllegalArgumentException,
                    io::IOException,
                    embed::StorageWrappedTargetException,
                    uno::RuntimeException );
        virtual uno::Reference< io::XStream > SAL_CALL
        openStreamElement( const ::rtl::OUString& aStreamName,
                           sal_Int32 nOpenMode )
            throw ( embed::InvalidStorageException,
                    lang::IllegalArgumentException,
                    packages::WrongPasswordException,
                    io::IOException,
                    embed::StorageWrappedTargetException,
                    uno::RuntimeException );
        virtual uno::Reference< io::XStream > SAL_CALL
        openEncryptedStreamElement( const ::rtl::OUString& aStreamName,
                                    sal_Int32 nOpenMode,
                                    const ::rtl::OUString& aPassword )
            throw ( embed::InvalidStorageException,
                    lang::IllegalArgumentException,
                    packages::NoEncryptionException,
                    packages::WrongPasswordException,
                    io::IOException,
                    embed::StorageWrappedTargetException,
                    uno::RuntimeException );
        virtual uno::Reference< embed::XStorage > SAL_CALL
        openStorageElement( const ::rtl::OUString& aStorName,
                            sal_Int32 nOpenMode )
            throw ( embed::InvalidStorageException,
                    lang::IllegalArgumentException,
                    io::IOException,
                    embed::StorageWrappedTargetException,
                    uno::RuntimeException );
        virtual uno::Reference< io::XStream > SAL_CALL
        cloneStreamElement( const ::rtl::OUString& aStreamName )
            throw ( embed::InvalidStorageException,
                    lang::IllegalArgumentException,
                    packages::WrongPasswordException,
                    io::IOException,
                    embed::StorageWrappedTargetException,
                    uno::RuntimeException );
        virtual uno::Reference< io::XStream > SAL_CALL
        cloneEncryptedStreamElement( const ::rtl::OUString& aStreamName,
                                     const ::rtl::OUString& aPassword )
            throw ( embed::InvalidStorageException,
                    lang::IllegalArgumentException,
                    packages::NoEncryptionException,
                    packages::WrongPasswordException,
                    io::IOException,
                    embed::StorageWrappedTargetException,
                    uno::RuntimeException );
        virtual void SAL_CALL
        copyLastCommitTo( const uno::Reference< embed::XStorage >& xTargetStorage )
            throw ( embed::InvalidStorageException,
                    lang::IllegalArgumentException,
                    io::IOException,
                    embed::StorageWrappedTargetException,
                    uno::RuntimeException );
        virtual void SAL_CALL
        copyStorageElementLastCommitTo( const ::rtl::OUString& aStorName,
                                        const uno::Reference< embed::XStorage >& xTargetStorage )
            throw ( embed::InvalidStorageException,
                    lang::IllegalArgumentException,
                    io::IOException,
                    embed::StorageWrappedTargetException,
                    uno::RuntimeException );
        virtual sal_Bool SAL_CALL
        isStreamElement( const ::rtl::OUString& aElementName )
            throw ( container::NoSuchElementException,
                    lang::IllegalArgumentException,
                    embed::InvalidStorageException,
                    uno::RuntimeException );
        virtual sal_Bool SAL_CALL
        isStorageElement( const ::rtl::OUString& aElementName )
            throw ( container::NoSuchElementException,
                    lang::IllegalArgumentException,
                    embed::InvalidStorageException,
                    uno::RuntimeException );
        virtual void SAL_CALL
        removeElement( const ::rtl::OUString& aElementName )
            throw ( embed::InvalidStorageException,
                    lang::IllegalArgumentException,
                    container::NoSuchElementException,
                    io::IOException,
                    embed::StorageWrappedTargetException,
                    uno::RuntimeException );
        virtual void SAL_CALL
        renameElement( const ::rtl::OUString& aEleName,
                       const ::rtl::OUString& aNewName )
            throw ( embed::InvalidStorageException,
                    lang::IllegalArgumentException,
                    container::NoSuchElementException,
                    container::ElementExistException,
                    io::IOException,
                    embed::StorageWrappedTargetException,
                    uno::RuntimeException );
        virtual void SAL_CALL
        copyElementTo( const ::rtl::OUString& aElementName,
                       const uno::Reference< embed::XStorage >& xDest,
                       const ::rtl::OUString& aNewName )
            throw ( embed::InvalidStorageException,
                    lang::IllegalArgumentException,
                    container::NoSuchElementException,
                    container::ElementExistException,
                    io::IOException,
                    embed::StorageWrappedTargetException,
                    uno::RuntimeException );
        virtual void SAL_CALL
        moveElementTo( const ::rtl::OUString& aElementName,
                       const uno::Reference< embed::XStorage >& xDest,
                       const ::rtl::OUString& rNewName )
            throw ( embed::InvalidStorageException,
                    lang::IllegalArgumentException,
                    container::NoSuchElementException,
                    container::ElementExistException,
                    io::IOException,
                    embed::StorageWrappedTargetException,
                    uno::RuntimeException );
        virtual void SAL_CALL
        insertRawEncrStreamElement( const ::rtl::OUString& aStreamName,
                                    const uno::Reference<
                                        io::XInputStream >& xInStream )
            throw ( embed::InvalidStorageException,
                    lang::IllegalArgumentException,
                    packages::NoRawFormatException,
                    container::ElementExistException,
                    io::IOException,
                    embed::StorageWrappedTargetException,
                    uno::RuntimeException );

        // XNameAccess
        virtual uno::Any SAL_CALL
        getByName( const ::rtl::OUString& aName )
            throw ( container::NoSuchElementException,
                    lang::WrappedTargetException,
                    uno::RuntimeException );
        virtual uno::Sequence< ::rtl::OUString > SAL_CALL
        getElementNames()
            throw ( uno::RuntimeException );
        virtual ::sal_Bool SAL_CALL
        hasByName( const ::rtl::OUString& aName )
            throw ( uno::RuntimeException );
        virtual uno::Type SAL_CALL
        getElementType()
            throw ( uno::RuntimeException );
        virtual ::sal_Bool SAL_CALL
        hasElements()
            throw ( uno::RuntimeException );

        // XComponent
        virtual void SAL_CALL
        dispose()
            throw ( uno::RuntimeException );
        virtual void SAL_CALL
        addEventListener( const uno::Reference< lang::XEventListener >& xListener )
            throw ( uno::RuntimeException );
        virtual void SAL_CALL
        removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
            throw (uno::RuntimeException);

        // XTransactedObject
        virtual void SAL_CALL commit()
            throw ( io::IOException,
                    lang::WrappedTargetException,
                    uno::RuntimeException );
        virtual void SAL_CALL revert()
            throw ( io::IOException,
                    lang::WrappedTargetException,
                    uno::RuntimeException );

    private:
        Storage( const rtl::Reference< Storage > & rFactory ); // n.i.

        // not done in ctor to avoid refcounting problems.
        void setDelegator();

        rtl::Reference< StorageElementFactory >     m_xFactory;
        uno::Reference< embed::XStorage >           m_xWrappedStorage;
        uno::Reference< uno::XAggregation >         m_xWrappedAggregate;
        bool                                        m_bDelegatorSet;
        StorageElementFactory::StorageMap::iterator m_aContainerIt;

        friend class StorageElementFactory;
        friend class std::auto_ptr< Storage >;
    };

    //=======================================================================

    typedef
        cppu::WeakImplHelper1<
            com::sun::star::io::XInputStream > InputStreamUNOBase;

    class InputStream : public InputStreamUNOBase, public ParentStorageHolder
    {
    public:
        InputStream(
            const uno::Reference< lang::XMultiServiceFactory > & xSMgr,
            const rtl::OUString & rUri,
            const uno::Reference< embed::XStorage >  & xParentStorage,
            const uno::Reference< io::XInputStream > & xStreamToWrap );
        virtual ~InputStream();

        // XInterface
        virtual uno::Any SAL_CALL queryInterface( const uno::Type& aType )
            throw ( uno::RuntimeException );

        // XInputStream
        virtual sal_Int32 SAL_CALL
        readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
            throw ( io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException,
                    uno::RuntimeException );
        virtual sal_Int32 SAL_CALL
        readSomeBytes( uno::Sequence< sal_Int8 >& aData,
                       sal_Int32 nMaxBytesToRead )
            throw ( io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException,
                    uno::RuntimeException );
        virtual void SAL_CALL
        skipBytes( sal_Int32 nBytesToSkip )
            throw ( io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException,
                    uno::RuntimeException );
        virtual sal_Int32 SAL_CALL
        available()
            throw ( io::NotConnectedException,
                    io::IOException,
                    uno::RuntimeException );
        // Note: We need to intercept this one.
        virtual void SAL_CALL
        closeInput()
            throw ( io::NotConnectedException,
                    io::IOException,
                    uno::RuntimeException );
    private:
        uno::Reference< io::XInputStream >   m_xWrappedStream;
        uno::Reference< uno::XAggregation >  m_xWrappedAggregate;
    };

    //=======================================================================

    typedef
        cppu::WeakImplHelper1<
            com::sun::star::io::XOutputStream > OutputStreamUNOBase;

    class OutputStream : public OutputStreamUNOBase, public ParentStorageHolder
    {
    public:
        OutputStream(
            const uno::Reference< lang::XMultiServiceFactory > & xSMgr,
            const rtl::OUString & rUri,
            const uno::Reference< embed::XStorage >  & xParentStorage,
            const uno::Reference< io::XOutputStream > & xStreamToWrap );
        virtual ~OutputStream();

        // XInterface
        virtual uno::Any SAL_CALL queryInterface( const uno::Type& aType )
            throw ( uno::RuntimeException );

        // XOutputStream
        virtual void SAL_CALL
        writeBytes( const uno::Sequence< sal_Int8 >& aData )
            throw ( io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException,
                    uno::RuntimeException );
        virtual void SAL_CALL
        flush(  )
            throw ( io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException,
                    uno::RuntimeException );
        // Note: We need to intercept this one.
        virtual void SAL_CALL
        closeOutput(  )
            throw ( io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException,
                    uno::RuntimeException );
    private:
        uno::Reference< io::XOutputStream >   m_xWrappedStream;
        uno::Reference< uno::XAggregation >  m_xWrappedAggregate;
    };

} // namespace tdoc_ucp

//=========================================================================
//=========================================================================
//
// StorageElementFactory Implementation.
//
//=========================================================================
//=========================================================================

StorageElementFactory::StorageElementFactory(
    const uno::Reference< lang::XMultiServiceFactory > & xSMgr,
    const rtl::Reference< OfficeDocumentsManager > & xDocsMgr )
: m_xDocsMgr( xDocsMgr ),
  m_xSMgr( xSMgr )
{
}

//=========================================================================
StorageElementFactory::~StorageElementFactory()
{
    OSL_ENSURE( m_aMap.size() == 0,
        "StorageElementFactory::~StorageElementFactory - Dangling storages!" );
}

//=========================================================================
uno::Reference< embed::XStorage >
StorageElementFactory::createTemporaryStorage()
    throw ( uno::Exception,
            uno::RuntimeException )
{
    uno::Reference< embed::XStorage > xStorage;
    uno::Reference< lang::XSingleServiceFactory > xStorageFac;
    if ( m_xSMgr.is() )
    {
        xStorageFac = uno::Reference< lang::XSingleServiceFactory >(
               m_xSMgr->createInstance(
                   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                       "com.sun.star.embed.StorageFactory" ) ) ),
               uno::UNO_QUERY );
    }

    OSL_ENSURE( xStorageFac.is(), "Can't create storage factory!" );
    if ( xStorageFac.is() )
        xStorage = uno::Reference< embed::XStorage >(
                            xStorageFac->createInstance(),
                            uno::UNO_QUERY );

    if ( !xStorage.is() )
        throw uno::RuntimeException();

    return xStorage;
}

//=========================================================================
uno::Reference< embed::XStorage >
StorageElementFactory::createStorage( const rtl::OUString & rUri,
                                      StorageAccessMode eMode )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( ( eMode != READ ) &&
         ( eMode != READ_ONLY ) &&
         ( eMode != READ_WRITE_NOCREATE ) &&
         ( eMode != READ_WRITE_CREATE ) )
        throw lang::IllegalArgumentException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "Invalid open mode!" ) ),
            uno::Reference< uno::XInterface >(),
            sal_Int16( 2 ) );

    Uri aUri( rUri );
    if ( aUri.isRoot() )
    {
        throw lang::IllegalArgumentException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "Root never has a storage!" ) ),
            uno::Reference< uno::XInterface >(),
            sal_Int16( 1 ) );
    }

    rtl::OUString aUriKey
        ( ( rUri.getStr()[ rUri.getLength() - 1 ] == sal_Unicode( '/' ) )
          ? rUri.copy( 0, rUri.getLength() - 1 )
          : rUri );

    StorageMap::iterator aIt ( m_aMap.begin() );
    StorageMap::iterator aEnd( m_aMap.end() );

    while ( aIt != aEnd )
    {
        if ( (*aIt).first.first == aUriKey )
        {
            // URI matches. Now, check open mode.
            bool bMatch = true;
            switch ( eMode )
            {
                case READ:
                    // No need to check; storage is at least readable.
                    bMatch = true;
                    break;

                case READ_ONLY:
                    // If found storage is writable, it cannot be used.
                    bMatch = !(*aIt).first.second;
                    break;

                case READ_WRITE_NOCREATE:
                case READ_WRITE_CREATE:
                    // If found storage is writable, it can be used.
                    // If not, a new one must be created.
                    bMatch = (*aIt).first.second;
                    break;
            }

            if ( bMatch )
                break;
        }
        ++aIt;
    }

    if ( aIt == aEnd )
    {
        uno::Reference< embed::XStorage > xParentStorage;

        // documents never have a parent storage.
        if ( !aUri.isDocument() )
        {
            xParentStorage = queryParentStorage( aUriKey, eMode );

            if ( !xParentStorage.is() )
            {
                // requested to create new storage, but failed?
                OSL_ENSURE( eMode != READ_WRITE_CREATE,
                            "Unable to create parent storage!" );
                return xParentStorage;
            }
        }

        uno::Reference< embed::XStorage > xStorage
            = queryStorage( xParentStorage, aUriKey, eMode );

        if ( !xStorage.is() )
        {
            // requested to create new storage, but failed?
            OSL_ENSURE( eMode != READ_WRITE_CREATE,
                        "Unable to create storage!" );
            return xStorage;
        }

        std::auto_ptr< Storage > xElement(
            new Storage( m_xSMgr, this, aUriKey, xParentStorage, xStorage ) );

        bool bWritable = ( ( eMode == READ_WRITE_NOCREATE )
                            || ( eMode == READ_WRITE_CREATE ) );

        aIt = m_aMap.insert(
            StorageMap::value_type(
                std::pair< rtl::OUString, bool >( aUriKey, bWritable ),
                xElement.get() ) ).first;

        aIt->second->m_aContainerIt = aIt;
        xElement.release();
        return aIt->second;
    }
    else if ( osl_incrementInterlockedCount( &aIt->second->m_refCount ) > 1 )
    {
        rtl::Reference< Storage > xElement( aIt->second );
        osl_decrementInterlockedCount( &aIt->second->m_refCount );
        return aIt->second;
    }
    else
    {
        osl_decrementInterlockedCount( &aIt->second->m_refCount );
        aIt->second->m_aContainerIt = m_aMap.end();

        uno::Reference< embed::XStorage > xParentStorage;

        // documents never have a parent storage.
        if ( !aUri.isDocument() )
        {
            xParentStorage = queryParentStorage( aUriKey, eMode );

            if ( !xParentStorage.is() )
            {
                // requested to create new storage, but failed?
                OSL_ENSURE( eMode != READ_WRITE_CREATE,
                            "Unable to create parent storage!" );
                return xParentStorage;
            }
        }

        uno::Reference< embed::XStorage > xStorage
            = queryStorage( xParentStorage, aUriKey, eMode );

        if ( !xStorage.is() )
        {
            // requested to create new storage, but failed?
            OSL_ENSURE( eMode != READ_WRITE_CREATE,
                        "Unable to create storage!" );
            return xStorage;
        }

        aIt->second
            = new Storage( m_xSMgr, this, aUriKey, xParentStorage, xStorage );
        aIt->second->m_aContainerIt = aIt;
        return aIt->second;
    }
}

//=========================================================================
uno::Reference< io::XInputStream >
StorageElementFactory::createInputStream( const rtl::OUString & rUri,
                                          const rtl::OUString & rPassword )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            packages::WrongPasswordException,
            uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    uno::Reference< embed::XStorage > xParentStorage
        = queryParentStorage( rUri, READ );

    // Each stream must have a parent storage.
    if ( !xParentStorage.is() )
        return uno::Reference< io::XInputStream >();

    uno::Reference< io::XStream > xStream
        = queryStream( xParentStorage, rUri, rPassword, READ );

    if ( !xStream.is() )
        return uno::Reference< io::XInputStream >();

    return uno::Reference< io::XInputStream >(
        new InputStream(
            m_xSMgr, rUri, xParentStorage, xStream->getInputStream() ) );
}

//=========================================================================
uno::Reference< io::XOutputStream >
StorageElementFactory::createOutputStream( const rtl::OUString & rUri,
                                           const rtl::OUString & rPassword )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            packages::WrongPasswordException,
            uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    uno::Reference< embed::XStorage > xParentStorage
        = queryParentStorage( rUri, READ_WRITE_CREATE );

    // Each stream must have a parent storage.
    if ( !xParentStorage.is() )
    {
        OSL_ENSURE( false,
                    "StorageElementFactory::createOutputStream - "
                    "Unable to create parent storage!" );
        return uno::Reference< io::XOutputStream >();
    }

    uno::Reference< io::XStream > xStream
        = queryStream( xParentStorage, rUri, rPassword, READ_WRITE_CREATE );

    if ( !xStream.is() )
    {
        OSL_ENSURE( false,
                    "StorageElementFactory::createOutputStream - "
                    "Unable to create stream!" );
        return uno::Reference< io::XOutputStream >();
    }

    return uno::Reference< io::XOutputStream >(
        new OutputStream(
            m_xSMgr, rUri, xParentStorage, xStream->getOutputStream() ) );
}

//=========================================================================
void StorageElementFactory::releaseElement( Storage * pElement ) SAL_THROW( () )
{
    OSL_ASSERT( pElement );
    osl::MutexGuard aGuard( m_aMutex );
    if ( pElement->m_aContainerIt != m_aMap.end() )
        m_aMap.erase( pElement->m_aContainerIt );
}

//=========================================================================
//
// Non-UNO interface
//
//=========================================================================

uno::Reference< embed::XStorage > StorageElementFactory::queryParentStorage(
        const rtl::OUString & rUri, StorageAccessMode eMode )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    uno::Reference< embed::XStorage > xParentStorage;

    Uri aUri( rUri );
    Uri aParentUri( aUri.getParentUri() );
    if ( !aParentUri.isRoot() )
    {
        xParentStorage = createStorage( aUri.getParentUri(), eMode );
        OSL_ENSURE( xParentStorage.is()
                    // requested to create new storage, but failed?
                    || ( eMode != READ_WRITE_CREATE ),
                    "StorageElementFactory::queryParentStorage - No storage!" );
    }
    return xParentStorage;
}

//=========================================================================
uno::Reference< embed::XStorage > StorageElementFactory::queryStorage(
        const uno::Reference< embed::XStorage > & xParentStorage,
        const rtl::OUString & rUri,
        StorageAccessMode eMode )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    uno::Reference< embed::XStorage > xStorage;

    Uri aUri( rUri );

    if ( !xParentStorage.is() )
    {
        // document storage

        xStorage = m_xDocsMgr->queryStorage( aUri.getDocumentId() );

        if ( !xStorage.is() )
        {
            if ( eMode == READ_WRITE_CREATE )
                throw lang::IllegalArgumentException(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "Invalid open mode: document storages cannot be "
                        "created!" ) ),
                    uno::Reference< uno::XInterface >(),
                    sal_Int16( 2 ) );
            else
                throw embed::InvalidStorageException(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "Invalid document id!" ) ),
                    uno::Reference< uno::XInterface >() );
        }

        // match xStorage's open mode against requested open mode

        uno::Reference< beans::XPropertySet > xPropSet(
            xStorage, uno::UNO_QUERY );
        OSL_ENSURE( xPropSet.is(),
                    "StorageElementFactory::queryStorage - "
                    "No XPropertySet interface!" );
        try
        {
            uno::Any aPropValue = xPropSet->getPropertyValue(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "OpenMode" ) ) );

            sal_Int32 nOpenMode;
            if ( aPropValue >>= nOpenMode )
            {
                switch ( eMode )
                {
                    case READ:
                        if ( !( nOpenMode
                                    & embed::ElementModes::READ ) )
                        {
                            // document opened, but not readable.
                            throw embed::InvalidStorageException(
                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "Storage is open, but not readable!" ) ),
                                uno::Reference< uno::XInterface >() );
                        }
                        // storage okay
                        break;

                    case READ_ONLY:
                        if ( nOpenMode & embed::ElementModes::WRITE )
                        {
                            // document opened, but it's writable.
                            throw embed::InvalidStorageException(
                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "Storage is open, but writable!" ) ),
                                uno::Reference< uno::XInterface >() );
                        }
                        // storage okay
                        break;

                    case READ_WRITE_NOCREATE:
                    case READ_WRITE_CREATE:
                        if ( !( nOpenMode
                                    & embed::ElementModes::WRITE ) )
                        {
                            // document opened, but not writable.
                            throw embed::InvalidStorageException(
                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                    "Storage is open, but not writable!" ) ),
                                uno::Reference< uno::XInterface >() );
                        }
                        // storage okay
                        break;
                }
            }
            else
            {
                OSL_ENSURE(
                    false, "Bug! Value of property OpenMode has wrong type!" );

                throw uno::RuntimeException(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "Bug! Value of property OpenMode has wrong type!" ) ),
                    uno::Reference< uno::XInterface >() );
            }
        }
        catch ( beans::UnknownPropertyException const & e )
        {
            OSL_ENSURE( false, "Property OpenMode not supported!" );

            throw embed::StorageWrappedTargetException(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "Bug! Value of property OpenMode has wrong type!" ) ),
                    uno::Reference< uno::XInterface >(),
                    uno::makeAny( e ) );
        }
        catch ( lang::WrappedTargetException const & e )
        {
            OSL_ENSURE( false, "Caught WrappedTargetException!" );

            throw embed::StorageWrappedTargetException(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "WrappedTargetException during getPropertyValue!" ) ),
                    uno::Reference< uno::XInterface >(),
                    uno::makeAny( e ) );
        }
    }
    else
    {
        // sub storage

        sal_Int32 nOpenMode;
        switch ( eMode )
        {
            case READ:
            case READ_ONLY:
                nOpenMode = embed::ElementModes::READ
                            | embed::ElementModes::NOCREATE;
                break;

            case READ_WRITE_NOCREATE:
                nOpenMode = embed::ElementModes::READWRITE
                            | embed::ElementModes::NOCREATE;
                break;

            case READ_WRITE_CREATE:
                nOpenMode = embed::ElementModes::READWRITE;
                break;
        }

        const rtl::OUString & rName = aUri.getDecodedName();
        try
        {
            xStorage = xParentStorage->openStorageElement( rName, nOpenMode );
        }
        catch ( io::IOException const & )
        {
            if ( ( eMode == READ ) || ( eMode == READ_ONLY ) )
            {
                // Distinguish between 'not existing' and 'access denied'
                // (somebody else already opened storage for writing?) and
                // "existing, but stream".

                bool bAccessDenied = true;
                try
                {
                    // true: element exists and is a storage, but access is
                    // denied false: element exists, but is a stream
                    bAccessDenied = xParentStorage->isStorageElement( rName );
                }
                catch ( container::NoSuchElementException const & )
                {
                    // element does not exist
                    bAccessDenied = false;
                }

                if ( bAccessDenied )
                {
                    // Clone
                    xStorage = createTemporaryStorage();
                    xParentStorage->copyStorageElementLastCommitTo( rName, xStorage );
                }
            }
            else
            {
                throw;
            }
        }
    }

    OSL_ENSURE( xStorage.is() || ( eMode != READ_WRITE_CREATE ),
                "StorageElementFactory::queryStorage - No storage!" );
    return xStorage;
}

//=========================================================================
uno::Reference< io::XStream >
StorageElementFactory::queryStream(
                const uno::Reference< embed::XStorage > & xParentStorage,
                const rtl::OUString & rUri,
                const rtl::OUString & rPassword,
                StorageAccessMode eMode )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            packages::WrongPasswordException,
            uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !xParentStorage.is() )
    {
        throw lang::IllegalArgumentException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "No parent storage!" ) ),
            uno::Reference< uno::XInterface >(),
            sal_Int16( 2 ) );
    }

    Uri aUri( rUri );
    if ( aUri.isRoot() )
    {
        throw lang::IllegalArgumentException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "Root never is a stream!" ) ),
            uno::Reference< uno::XInterface >(),
            sal_Int16( 2 ) );
    }
    else if ( aUri.isDocument() )
    {
        throw lang::IllegalArgumentException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "A document never is a stream!" ) ),
            uno::Reference< uno::XInterface >(),
            sal_Int16( 2 ) );
    }

    // @@@ Sophisticated object reusage (like for storages) needed???

    sal_Int32 nOpenMode;
    switch ( eMode )
    {
        case READ:
        case READ_ONLY:
            nOpenMode = embed::ElementModes::READ
                        | embed::ElementModes::NOCREATE;
            break;

        case READ_WRITE_NOCREATE:
            nOpenMode = embed::ElementModes::READWRITE
                        | embed::ElementModes::NOCREATE;
            break;

        case READ_WRITE_CREATE:
            nOpenMode = embed::ElementModes::READWRITE;
            break;
    }

    uno::Reference< io::XStream > xStream;
    if ( rPassword.getLength() > 0 )
    {
        try
        {
            xStream = xParentStorage->openEncryptedStreamElement(
                                                     aUri.getDecodedName(),
                                                     nOpenMode,
                                                     rPassword );
        }
        catch ( packages::NoEncryptionException const & )
        {
            xStream = xParentStorage->openStreamElement( aUri.getDecodedName(),
                                                         nOpenMode );
        }
    }
    else
    {
        xStream = xParentStorage->openStreamElement( aUri.getDecodedName(),
                                                     nOpenMode );
    }

    if ( !xStream.is() )
    {
        throw embed::InvalidStorageException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "No stream!" ) ),
            uno::Reference< uno::XInterface >() );
    }

    return xStream;
}

//=========================================================================
//=========================================================================
//
// ParentStorageHolder Implementation.
//
//=========================================================================
//=========================================================================

ParentStorageHolder::ParentStorageHolder(
            const uno::Reference< embed::XStorage > & xParentStorage,
            const rtl::OUString & rUri )
: m_xParentStorage( xParentStorage ),
  m_bParentStorageIsRoot( false )
{
    Uri aUri( rUri );
    if ( aUri.isDocument() ) // This correct; aUri.isRoot() is not!
        m_bParentStorageIsRoot = true;
}

//=========================================================================
//=========================================================================
//
// Storage Implementation.
//
//=========================================================================
//=========================================================================

Storage::Storage( const uno::Reference< lang::XMultiServiceFactory > & xSMgr,
                  const rtl::Reference< StorageElementFactory > & xFactory,
                  const rtl::OUString & rUri,
                  const uno::Reference< embed::XStorage > & xParentStorage,
                  const uno::Reference< embed::XStorage > & xStorageToWrap )
: ParentStorageHolder( xParentStorage, Uri( rUri ).getParentUri() ),
  m_xFactory( xFactory ),
  m_xWrappedStorage( xStorageToWrap ),
  m_bDelegatorSet( false )
{
    OSL_ENSURE( m_xWrappedStorage.is(),
                "Storage::Storage: No storage to wrap!" );

    // Use proxy factory service to create aggregatable proxy.
    try
    {
        uno::Reference< reflection::XProxyFactory > xProxyFac(
            xSMgr->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.reflection.ProxyFactory" ) ) ),
            uno::UNO_QUERY );
        if ( xProxyFac.is() )
        {
            m_xWrappedAggregate
                = xProxyFac->createProxy( m_xWrappedStorage );
            m_xWrappedStorage
                = uno::Reference< embed::XStorage >(
                    m_xWrappedAggregate, uno::UNO_QUERY );
        }
    }
    catch ( uno::Exception const & )
    {
        OSL_ENSURE( false, "Storage::Storage: Caught exception!" );
    }

    OSL_ENSURE( m_xWrappedAggregate.is(),
                "Storage::Storage: Wrapped storage cannot be aggregated!" );
}

//=========================================================================
// virtual
Storage::~Storage()
{
    if ( m_xWrappedAggregate.is() )
        m_xWrappedAggregate->setDelegator(
            uno::Reference< uno::XInterface >() );

    if ( m_xWrappedStorage.is() )
    {
        // "Auto-dispose"...
        uno::Reference< lang::XComponent >
            xC( m_xWrappedStorage, uno::UNO_QUERY );
        OSL_ENSURE ( xC.is(), "Required interface XComponent missing!" );

        try
        {
            xC->dispose();
        }
        catch ( lang::DisposedException const & )
        {
            // might happen.
        }
        catch ( ... )
        {
            OSL_ENSURE( false, "Storage::~Storage - Caught exception!" );
        }
    }
}

//=========================================================================
void Storage::setDelegator()
{
    if ( !m_bDelegatorSet )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( !m_bDelegatorSet )
        {
            m_xWrappedAggregate->setDelegator(
                static_cast< cppu::OWeakObject * >( this ) );
            m_bDelegatorSet = true;
        }
    }
}

//=========================================================================
//
// uno::XInterface
//
//=========================================================================

// virtual
uno::Any SAL_CALL Storage::queryInterface( const uno::Type& aType )
    throw ( uno::RuntimeException)
{
    // First, try to use interfaces implemented by myself and base class(es)
    uno::Any aRet = StorageUNOBase::queryInterface( aType );

    if ( aRet.hasValue() )
        return aRet;

    // Late init: Set this as delegator for wrapped storage.
    setDelegator();

    // Try to use requested interface from aggregated storage
    return m_xWrappedAggregate->queryAggregation( aType );
}

//=========================================================================
// virtual
void SAL_CALL Storage::acquire()
    throw ()
{
    osl_incrementInterlockedCount( &m_refCount );
}

//=========================================================================
// virtual
void SAL_CALL Storage::release()
    throw ()
{
    if ( osl_decrementInterlockedCount( &m_refCount ) == 0 )
    {
        m_xFactory->releaseElement( this );
        delete this;
    }
}

//=========================================================================
//
// embed::XStorage
//
//=========================================================================

// virtual
void SAL_CALL Storage::copyToStorage(
        const uno::Reference< embed::XStorage >& xDest )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    m_xWrappedStorage->copyToStorage( xDest );
}

//=========================================================================
// virtual
uno::Reference< io::XStream > SAL_CALL Storage::openStreamElement(
        const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            packages::WrongPasswordException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->openStreamElement( aStreamName, nOpenMode );
}

//=========================================================================
// virtual
uno::Reference< io::XStream > SAL_CALL Storage::openEncryptedStreamElement(
        const ::rtl::OUString& aStreamName,
        sal_Int32 nOpenMode,
        const ::rtl::OUString& aPassword )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            packages::NoEncryptionException,
            packages::WrongPasswordException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->openEncryptedStreamElement(
        aStreamName, nOpenMode, aPassword );
}

//=========================================================================
// virtual
uno::Reference< embed::XStorage > SAL_CALL Storage::openStorageElement(
        const ::rtl::OUString& aStorName, sal_Int32 nOpenMode )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->openStorageElement( aStorName, nOpenMode );
}

//=========================================================================
// virtual
uno::Reference< io::XStream > SAL_CALL Storage::cloneStreamElement(
        const ::rtl::OUString& aStreamName )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            packages::WrongPasswordException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->cloneStreamElement( aStreamName );
}

//=========================================================================
// virtual
uno::Reference< io::XStream > SAL_CALL Storage::cloneEncryptedStreamElement(
        const ::rtl::OUString& aStreamName,
        const ::rtl::OUString& aPassword )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            packages::NoEncryptionException,
            packages::WrongPasswordException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->cloneEncryptedStreamElement( aStreamName, aPassword );
}

//=========================================================================
// virtual
void SAL_CALL Storage::copyLastCommitTo(
        const uno::Reference< embed::XStorage >& xTargetStorage )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException)
{
    m_xWrappedStorage->copyLastCommitTo( xTargetStorage );
}

//=========================================================================
// virtual
void SAL_CALL Storage::copyStorageElementLastCommitTo(
        const ::rtl::OUString& aStorName,
        const uno::Reference< embed::XStorage >& xTargetStorage )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException)
{
    m_xWrappedStorage->copyStorageElementLastCommitTo( aStorName, xTargetStorage );
}

//=========================================================================
// virtual
sal_Bool SAL_CALL Storage::isStreamElement(
        const ::rtl::OUString& aElementName )
    throw ( container::NoSuchElementException,
            lang::IllegalArgumentException,
            embed::InvalidStorageException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->isStreamElement( aElementName );
}

//=========================================================================
// virtual
sal_Bool SAL_CALL Storage::isStorageElement(
        const ::rtl::OUString& aElementName )
    throw ( container::NoSuchElementException,
            lang::IllegalArgumentException,
            embed::InvalidStorageException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->isStorageElement( aElementName );
}

//=========================================================================
// virtual
void SAL_CALL Storage::removeElement( const ::rtl::OUString& aElementName )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            container::NoSuchElementException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    m_xWrappedStorage->removeElement( aElementName );
}

//=========================================================================
// virtual
void SAL_CALL Storage::renameElement( const ::rtl::OUString& aEleName,
                                      const ::rtl::OUString& aNewName )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            container::NoSuchElementException,
            container::ElementExistException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    m_xWrappedStorage->renameElement( aEleName, aNewName );
}

//=========================================================================
// virtual
void SAL_CALL Storage::copyElementTo(
        const ::rtl::OUString& aElementName,
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
    m_xWrappedStorage->copyElementTo( aElementName, xDest, aNewName );
}

//=========================================================================
// virtual
void SAL_CALL Storage::moveElementTo(
        const ::rtl::OUString& aElementName,
        const uno::Reference< embed::XStorage >& xDest,
        const ::rtl::OUString& rNewName )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            container::NoSuchElementException,
            container::ElementExistException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    m_xWrappedStorage->moveElementTo( aElementName, xDest, rNewName );
}

//=========================================================================
// virtual
void SAL_CALL Storage::insertRawEncrStreamElement(
        const ::rtl::OUString& aStreamName,
        const uno::Reference< io::XInputStream >& xInStream )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            packages::NoRawFormatException,
            container::ElementExistException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    m_xWrappedStorage->insertRawEncrStreamElement( aStreamName, xInStream );
}

//=========================================================================
//
// XNameAccess
//
//=========================================================================
// virtual
uno::Any SAL_CALL Storage::getByName( const ::rtl::OUString& aName )
    throw ( container::NoSuchElementException,
            lang::WrappedTargetException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->getByName( aName );
}

//=========================================================================
// virtual
uno::Sequence< ::rtl::OUString > SAL_CALL Storage::getElementNames()
    throw ( uno::RuntimeException )
{
    return m_xWrappedStorage->getElementNames();
}

//=========================================================================
// virtual
::sal_Bool SAL_CALL Storage::hasByName( const ::rtl::OUString& aName )
    throw ( uno::RuntimeException )
{
    return m_xWrappedStorage->hasByName( aName );
}

//=========================================================================
// virtual
uno::Type SAL_CALL Storage::getElementType()
    throw ( uno::RuntimeException )
{
    return m_xWrappedStorage->getElementType();
}

//=========================================================================
// virtual
::sal_Bool SAL_CALL Storage::hasElements()
    throw ( uno::RuntimeException )
{
    return m_xWrappedStorage->hasElements();
}

//=========================================================================
//
// XComponent
//
//=========================================================================
// virtual
void SAL_CALL Storage::dispose()
    throw ( uno::RuntimeException )
{
    // Not implemented yet, may be it is just enough to redirect the call to m_xWrappedStorage
    OSL_ENSURE( sal_False, "Storage::dispose() is not implemented!" );
}

//=========================================================================
// virtual
void SAL_CALL Storage::addEventListener(
        const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
    // Not implemented yet, may be it is just enough to redirect the call to m_xWrappedStorage
    OSL_ENSURE( sal_False, "Storage::addEventListener() is not implemented!" );
}
//=========================================================================
// virtual
void SAL_CALL Storage::removeEventListener(
        const uno::Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    // Not implemented yet, may be it is just enough to redirect the call to m_xWrappedStorage
    OSL_ENSURE( sal_False, "Storage::removeEventListener() is not implemented!" );
}


//=========================================================================
//
// embed::XTransactedObject
//
//=========================================================================

// virtual
void SAL_CALL Storage::commit()
    throw ( io::IOException,
            lang::WrappedTargetException,
            uno::RuntimeException )
{
    // Never commit a root storage (-> has no parent)!
    // Would lead in writing the whole document to disk.

    uno::Reference< embed::XStorage > xParentStorage = getParentStorage();
    if ( xParentStorage.is() )
    {
        // Important: don't use 'normal' queryInterface-based ctor here
        // to obtain XTransactedObject reference. wrapped storage is aggregated.
        // qI call will lead to Storage::qI, which returns 'this' -> endless
        // recursion!!!
        uno::Reference< embed::XTransactedObject > xTA(
            m_xWrappedAggregate->queryAggregation(
                getCppuType( static_cast<
                    const uno::Reference<
                        embed::XTransactedObject > * >( 0 ) ) ),
            uno::UNO_QUERY );
        OSL_ENSURE( xTA.is(), "No XTransactedObject interface!" );

        if ( xTA.is() )
            xTA->commit();

        if ( !isRootParentStorage() )
        {
            uno::Reference< embed::XTransactedObject > xParentTA(
                xParentStorage, uno::UNO_QUERY );
            OSL_ENSURE( xParentTA.is(), "No XTransactedObject interface!" );

            if ( xParentTA.is() )
                xParentTA->commit();
        }
    }
}

//=========================================================================
// virtual
void SAL_CALL Storage::revert()
    throw ( io::IOException,
            lang::WrappedTargetException,
            uno::RuntimeException )
{
    uno::Reference< embed::XStorage > xParentStorage = getParentStorage();
    if ( xParentStorage.is() )
    {
        // Important: don't use 'normal' queryInterface-based ctor here
        // to obtain XTransactedObject reference. wrapped storage is aggregated.
        // qI call will lead to Storage::qI, which returns 'this' -> endless
        // recursion!!!
        uno::Reference< embed::XTransactedObject > xTA(
            m_xWrappedAggregate->queryAggregation(
                getCppuType( static_cast<
                    const uno::Reference<
                        embed::XTransactedObject > * >( 0 ) ) ),
            uno::UNO_QUERY );
        OSL_ENSURE( xTA.is(), "No XTransactedObject interface!" );

        if ( xTA.is() )
            xTA->revert();

        if ( !isRootParentStorage() )
        {
            uno::Reference< embed::XTransactedObject > xParentTA(
                xParentStorage, uno::UNO_QUERY );
            OSL_ENSURE( xParentTA.is(), "No XTransactedObject interface!" );

            if ( xParentTA.is() )
                xParentTA->revert();
        }
    }
}

//=========================================================================
//=========================================================================
//
// InputStream Implementation.
//
//=========================================================================
//=========================================================================

InputStream::InputStream(
            const uno::Reference< lang::XMultiServiceFactory > & xSMgr,
            const rtl::OUString & rUri,
            const uno::Reference< embed::XStorage >  & xParentStorage,
            const uno::Reference< io::XInputStream > & xStreamToWrap )
: ParentStorageHolder( xParentStorage, Uri( rUri ).getParentUri() ),
  m_xWrappedStream( xStreamToWrap )
{
    OSL_ENSURE( m_xWrappedStream.is(),
                "InputStream::InputStream: No stream to wrap!" );

    // Use proxy factory service to create aggregatable proxy.
    try
    {
        uno::Reference< reflection::XProxyFactory > xProxyFac(
            xSMgr->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.reflection.ProxyFactory" ) ) ),
            uno::UNO_QUERY );
        if ( xProxyFac.is() )
        {
            m_xWrappedAggregate
                = xProxyFac->createProxy( m_xWrappedStream );
            m_xWrappedStream
                = uno::Reference< io::XInputStream >(
                    m_xWrappedAggregate, uno::UNO_QUERY );
        }
    }
    catch ( uno::Exception const & )
    {
        OSL_ENSURE( false, "InputStream::InputStream: Caught exception!" );
    }

    OSL_ENSURE( m_xWrappedAggregate.is(),
            "InputStream::InputStream: Wrapped stream cannot be aggregated!" );

    if ( m_xWrappedAggregate.is() )
    {
        osl_incrementInterlockedCount( &m_refCount );
        m_xWrappedAggregate->setDelegator(
            static_cast< cppu::OWeakObject * >( this ) );
        osl_decrementInterlockedCount( &m_refCount );
    }
}

//=========================================================================
// virtual
InputStream::~InputStream()
{
    if ( m_xWrappedAggregate.is() )
        m_xWrappedAggregate->setDelegator(
            uno::Reference< uno::XInterface >() );
}

//=========================================================================
//
// uno::XInterface
//
//=========================================================================

// virtual
uno::Any SAL_CALL InputStream::queryInterface( const uno::Type& aType )
    throw ( uno::RuntimeException)
{
    uno::Any aRet = InputStreamUNOBase::queryInterface( aType );

    if ( aRet.hasValue() )
        return aRet;

    if ( m_xWrappedAggregate.is() )
        return m_xWrappedAggregate->queryAggregation( aType );
    else
        return uno::Any();
}

//=========================================================================
//
// io::XInputStream
//
//=========================================================================

// virtual
sal_Int32 SAL_CALL
InputStream::readBytes( uno::Sequence< sal_Int8 >& aData,
                        sal_Int32 nBytesToRead )
    throw ( io::NotConnectedException,
            io::BufferSizeExceededException,
            io::IOException,
            uno::RuntimeException )
{
    return m_xWrappedStream->readBytes( aData, nBytesToRead );
}

//=========================================================================
// virtual
sal_Int32 SAL_CALL
InputStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData,
                            sal_Int32 nMaxBytesToRead )
    throw ( io::NotConnectedException,
            io::BufferSizeExceededException,
            io::IOException,
            uno::RuntimeException )
{
    return m_xWrappedStream->readSomeBytes( aData, nMaxBytesToRead );
}

//=========================================================================
// virtual
void SAL_CALL
InputStream::skipBytes( sal_Int32 nBytesToSkip )
    throw ( io::NotConnectedException,
            io::BufferSizeExceededException,
            io::IOException,
            uno::RuntimeException )
{
    m_xWrappedStream->skipBytes( nBytesToSkip );
}

//=========================================================================
// virtual
sal_Int32 SAL_CALL
InputStream::available()
    throw ( io::NotConnectedException,
            io::IOException,
            uno::RuntimeException )
{
    return m_xWrappedStream->available();
}

//=========================================================================
// virtual
void SAL_CALL
InputStream::closeInput()
    throw ( io::NotConnectedException,
            io::IOException,
            uno::RuntimeException )
{
    m_xWrappedStream->closeInput();

    // Release parent storage.
    // Now, that the stream is closed it is not needed any longer.
    setParentStorage( uno::Reference< embed::XStorage >() );
}

//=========================================================================
//=========================================================================
//
// OutputStream Implementation.
//
//=========================================================================
//=========================================================================

OutputStream::OutputStream(
            const uno::Reference< lang::XMultiServiceFactory > & xSMgr,
            const rtl::OUString & rUri,
            const uno::Reference< embed::XStorage >  & xParentStorage,
            const uno::Reference< io::XOutputStream > & xStreamToWrap )
: ParentStorageHolder( xParentStorage, Uri( rUri ).getParentUri() ),
  m_xWrappedStream( xStreamToWrap )
{
    OSL_ENSURE( m_xWrappedStream.is(),
                "OutputStream::OutputStream: No stream to wrap!" );

    // Use proxy factory service to create aggregatable proxy.
    try
    {
        uno::Reference< reflection::XProxyFactory > xProxyFac(
            xSMgr->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.reflection.ProxyFactory" ) ) ),
            uno::UNO_QUERY );
        if ( xProxyFac.is() )
        {
            m_xWrappedAggregate
                = xProxyFac->createProxy( m_xWrappedStream );
            m_xWrappedStream
                = uno::Reference< io::XOutputStream >(
                    m_xWrappedAggregate, uno::UNO_QUERY );
        }
    }
    catch ( uno::Exception const & )
    {
        OSL_ENSURE( false, "OutputStream::OutputStream: Caught exception!" );
    }

    OSL_ENSURE( m_xWrappedAggregate.is(),
            "OutputStream::OutputStream: Wrapped stream cannot be aggregated!" );

    if ( m_xWrappedAggregate.is() )
    {
        osl_incrementInterlockedCount( &m_refCount );
        m_xWrappedAggregate->setDelegator(
            static_cast< cppu::OWeakObject * >( this ) );
        osl_decrementInterlockedCount( &m_refCount );
    }
}

//=========================================================================
// virtual
OutputStream::~OutputStream()
{
    if ( m_xWrappedAggregate.is() )
        m_xWrappedAggregate->setDelegator(
            uno::Reference< uno::XInterface >() );
}

//=========================================================================
//
// uno::XInterface
//
//=========================================================================

// virtual
uno::Any SAL_CALL OutputStream::queryInterface( const uno::Type& aType )
    throw ( uno::RuntimeException)
{
    uno::Any aRet = OutputStreamUNOBase::queryInterface( aType );

    if ( aRet.hasValue() )
        return aRet;

    if ( m_xWrappedAggregate.is() )
        return m_xWrappedAggregate->queryAggregation( aType );
    else
        return uno::Any();
}

//=========================================================================
//
// io::XOutputStream
//
//=========================================================================

// virtual
void SAL_CALL
OutputStream::writeBytes( const uno::Sequence< sal_Int8 >& aData )
    throw ( io::NotConnectedException,
            io::BufferSizeExceededException,
            io::IOException,
            uno::RuntimeException )
{
    m_xWrappedStream->writeBytes( aData );
}

//=========================================================================
// virtual
void SAL_CALL
OutputStream::flush(  )
    throw ( io::NotConnectedException,
            io::BufferSizeExceededException,
            io::IOException,
            uno::RuntimeException )
{
    m_xWrappedStream->flush();
}

//=========================================================================
// virtual
void SAL_CALL
OutputStream::closeOutput(  )
    throw ( io::NotConnectedException,
            io::BufferSizeExceededException,
            io::IOException,
            uno::RuntimeException )
{
    m_xWrappedStream->closeOutput();

    // Release parent storage.
    // Now, that the stream is closed it is not needed any longer.
    setParentStorage( uno::Reference< embed::XStorage >() );
}
