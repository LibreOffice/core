/*************************************************************************
 *
 *  $RCSfile: tdoc_stgelems.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-05-28 15:16:58 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef INCLUDED_TDOC_STGELEMS_HXX
#define INCLUDED_TDOC_STGELEMS_HXX

#include <memory>

#include "osl/mutex.hxx"
#include "rtl/ref.hxx"

#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/implbase5.hxx"

#include "com/sun/star/embed/XStorage.hpp"
#include "com/sun/star/embed/XTransactedObject.hpp"
#include "com/sun/star/io/XOutputStream.hpp"
#include "com/sun/star/io/XStream.hpp"
#include "com/sun/star/io/XTruncate.hpp"
#include "com/sun/star/lang/XComponent.hpp"

#include "tdoc_storage.hxx"

namespace tdoc_ucp {

struct MutexHolder
{
    osl::Mutex m_aMutex;
};

//=======================================================================

class ParentStorageHolder : public MutexHolder
{
public:
    ParentStorageHolder(
        const com::sun::star::uno::Reference<
            com::sun::star::embed::XStorage > & xParentStorage,
        const rtl::OUString & rUri );

    bool isRootParentStorage() const
    { return m_bParentStorageIsRoot; }
    com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
    getParentStorage() const
    { return m_xParentStorage; }
    void setParentStorage( const com::sun::star::uno::Reference<
                            com::sun::star::embed::XStorage > & xStg )
    { osl::MutexGuard aGuard( m_aMutex ); m_xParentStorage = xStg; }

private:
    com::sun::star::uno::Reference<
        com::sun::star::embed::XStorage > m_xParentStorage;
    bool                                  m_bParentStorageIsRoot;
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
        const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > & xSMgr,
        const rtl::Reference< StorageElementFactory >  & xFactory,
        const rtl::OUString & rUri,
        const com::sun::star::uno::Reference<
            com::sun::star::embed::XStorage > & xParentStorage,
        const com::sun::star::uno::Reference<
            com::sun::star::embed::XStorage > & xStorageToWrap );
    virtual ~Storage();

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL queryInterface(
            const com::sun::star::uno::Type& aType )
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire()
        throw ();
    virtual void SAL_CALL release()
        throw ();

    // XTypeProvider (implemnented by base, but needs to be overridden for
    //                delegating to aggregate)
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw ( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw ( com::sun::star::uno::RuntimeException );

    // XComponent ( one of XStorage bases )
    virtual void SAL_CALL
    dispose()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XEventListener > & xListener )
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& aListener )
        throw ( com::sun::star::uno::RuntimeException );

    // XNameAccess ( one of XStorage bases )
    virtual com::sun::star::uno::Any SAL_CALL
    getByName( const rtl::OUString& aName )
        throw ( com::sun::star::container::NoSuchElementException,
                com::sun::star::lang::WrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getElementNames()
        throw ( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    hasByName( const rtl::OUString& aName )
        throw ( com::sun::star::uno::RuntimeException );

    // XElementAccess (base of XNameAccess)
    virtual com::sun::star::uno::Type SAL_CALL
    getElementType()
        throw ( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    hasElements()
        throw ( com::sun::star::uno::RuntimeException );

    // XStorage
    virtual void SAL_CALL
    copyToStorage( const com::sun::star::uno::Reference<
                    com::sun::star::embed::XStorage >& xDest )
        throw ( com::sun::star::embed::InvalidStorageException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::io::IOException,
                com::sun::star::embed::StorageWrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference< com::sun::star::io::XStream > SAL_CALL
    openStreamElement( const ::rtl::OUString& aStreamName,
                       sal_Int32 nOpenMode )
        throw ( com::sun::star::embed::InvalidStorageException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::packages::WrongPasswordException,
                com::sun::star::io::IOException,
                com::sun::star::embed::StorageWrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference< com::sun::star::io::XStream > SAL_CALL
    openEncryptedStreamElement( const ::rtl::OUString& aStreamName,
                                sal_Int32 nOpenMode,
                                const ::rtl::OUString& aPassword )
        throw ( com::sun::star::embed::InvalidStorageException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::packages::NoEncryptionException,
                com::sun::star::packages::WrongPasswordException,
                com::sun::star::io::IOException,
                com::sun::star::embed::StorageWrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference< com::sun::star::embed::XStorage > SAL_CALL
    openStorageElement( const ::rtl::OUString& aStorName,
                        sal_Int32 nOpenMode )
        throw ( com::sun::star::embed::InvalidStorageException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::io::IOException,
                com::sun::star::embed::StorageWrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference< com::sun::star::io::XStream > SAL_CALL
    cloneStreamElement( const ::rtl::OUString& aStreamName )
        throw ( com::sun::star::embed::InvalidStorageException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::packages::WrongPasswordException,
                com::sun::star::io::IOException,
                com::sun::star::embed::StorageWrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference< com::sun::star::io::XStream > SAL_CALL
    cloneEncryptedStreamElement( const ::rtl::OUString& aStreamName,
                                 const ::rtl::OUString& aPassword )
        throw ( com::sun::star::embed::InvalidStorageException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::packages::NoEncryptionException,
                com::sun::star::packages::WrongPasswordException,
                com::sun::star::io::IOException,
                com::sun::star::embed::StorageWrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    copyLastCommitTo( const com::sun::star::uno::Reference<
                        com::sun::star::embed::XStorage >& xTargetStorage )
        throw ( com::sun::star::embed::InvalidStorageException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::io::IOException,
                com::sun::star::embed::StorageWrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    copyStorageElementLastCommitTo( const ::rtl::OUString& aStorName,
                                    const com::sun::star::uno::Reference<
                                        com::sun::star::embed::XStorage > &
                                            xTargetStorage )
        throw ( com::sun::star::embed::InvalidStorageException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::io::IOException,
                com::sun::star::embed::StorageWrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isStreamElement( const ::rtl::OUString& aElementName )
        throw ( com::sun::star::container::NoSuchElementException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::embed::InvalidStorageException,
                com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    isStorageElement( const ::rtl::OUString& aElementName )
        throw ( com::sun::star::container::NoSuchElementException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::embed::InvalidStorageException,
                com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeElement( const ::rtl::OUString& aElementName )
        throw ( com::sun::star::embed::InvalidStorageException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::container::NoSuchElementException,
                com::sun::star::io::IOException,
                com::sun::star::embed::StorageWrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    renameElement( const ::rtl::OUString& aEleName,
                   const ::rtl::OUString& aNewName )
        throw ( com::sun::star::embed::InvalidStorageException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::container::NoSuchElementException,
                com::sun::star::container::ElementExistException,
                com::sun::star::io::IOException,
                com::sun::star::embed::StorageWrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    copyElementTo( const ::rtl::OUString& aElementName,
                   const com::sun::star::uno::Reference<
                    com::sun::star::embed::XStorage >& xDest,
                   const ::rtl::OUString& aNewName )
        throw ( com::sun::star::embed::InvalidStorageException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::container::NoSuchElementException,
                com::sun::star::container::ElementExistException,
                com::sun::star::io::IOException,
                com::sun::star::embed::StorageWrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    moveElementTo( const ::rtl::OUString& aElementName,
                   const com::sun::star::uno::Reference<
                    com::sun::star::embed::XStorage >& xDest,
                   const ::rtl::OUString& rNewName )
        throw ( com::sun::star::embed::InvalidStorageException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::container::NoSuchElementException,
                com::sun::star::container::ElementExistException,
                com::sun::star::io::IOException,
                com::sun::star::embed::StorageWrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    insertRawEncrStreamElement( const ::rtl::OUString& aStreamName,
                                const com::sun::star::uno::Reference<
                                    com::sun::star::io::XInputStream >& xInStream )
        throw ( com::sun::star::embed::InvalidStorageException,
                com::sun::star::lang::IllegalArgumentException,
                com::sun::star::packages::NoRawFormatException,
                com::sun::star::container::ElementExistException,
                com::sun::star::io::IOException,
                com::sun::star::embed::StorageWrappedTargetException,
                com::sun::star::uno::RuntimeException );

    // XTransactedObject
    virtual void SAL_CALL commit()
        throw ( com::sun::star::io::IOException,
                com::sun::star::lang::WrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL revert()
        throw ( com::sun::star::io::IOException,
                com::sun::star::lang::WrappedTargetException,
                com::sun::star::uno::RuntimeException );

private:
    Storage( const rtl::Reference< Storage > & rFactory ); // n.i.

    rtl::Reference< StorageElementFactory >         m_xFactory;
    com::sun::star::uno::Reference<
        com::sun::star::uno::XAggregation >         m_xAggProxy;
    com::sun::star::uno::Reference<
        com::sun::star::embed::XStorage >           m_xWrappedStorage;
    com::sun::star::uno::Reference<
        com::sun::star::embed::XTransactedObject >  m_xWrappedTransObj;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XComponent >          m_xWrappedComponent;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XTypeProvider >       m_xWrappedTypeProv;

    StorageElementFactory::StorageMap::iterator m_aContainerIt;

    friend class StorageElementFactory;
    friend class std::auto_ptr< Storage >;
};

//=======================================================================

typedef
    cppu::WeakImplHelper2<
        com::sun::star::io::XOutputStream,
        com::sun::star::lang::XComponent > OutputStreamUNOBase;

class OutputStream : public OutputStreamUNOBase, public ParentStorageHolder
{
public:
    OutputStream(
        const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > & xSMgr,
        const rtl::OUString & rUri,
        const com::sun::star::uno::Reference<
            com::sun::star::embed::XStorage >  & xParentStorage,
        const com::sun::star::uno::Reference<
            com::sun::star::io::XOutputStream > & xStreamToWrap );
    virtual ~OutputStream();

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type& aType )
        throw ( com::sun::star::uno::RuntimeException );

    // XTypeProvider (implemnented by base, but needs to be overridden for
    //                delegating to aggregate)
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw ( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw ( com::sun::star::uno::RuntimeException );

    // XOutputStream
    virtual void SAL_CALL
    writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData )
        throw ( com::sun::star::io::NotConnectedException,
                com::sun::star::io::BufferSizeExceededException,
                com::sun::star::io::IOException,
                com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    flush(  )
        throw ( com::sun::star::io::NotConnectedException,
                com::sun::star::io::BufferSizeExceededException,
                com::sun::star::io::IOException,
                com::sun::star::uno::RuntimeException );
    // Note: We need to intercept this one.
    virtual void SAL_CALL
    closeOutput(  )
        throw ( com::sun::star::io::NotConnectedException,
                com::sun::star::io::BufferSizeExceededException,
                com::sun::star::io::IOException,
                com::sun::star::uno::RuntimeException );

    // XComponent
    // Note: We need to intercept this one.
    virtual void SAL_CALL
    dispose()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
                        com::sun::star::lang::XEventListener >& xListener )
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& aListener )
        throw ( com::sun::star::uno::RuntimeException );

private:
    com::sun::star::uno::Reference<
        com::sun::star::uno::XAggregation >     m_xAggProxy;
    com::sun::star::uno::Reference<
        com::sun::star::io::XOutputStream >     m_xWrappedStream;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XComponent >      m_xWrappedComponent;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XTypeProvider >   m_xWrappedTypeProv;
};

//=======================================================================

typedef cppu::WeakImplHelper5< com::sun::star::io::XStream,
                               com::sun::star::io::XOutputStream,
                               com::sun::star::io::XTruncate,
                               com::sun::star::io::XInputStream,
                               com::sun::star::lang::XComponent >
        StreamUNOBase;

class Stream : public StreamUNOBase, public ParentStorageHolder
{
public:
    Stream(
        const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > & xSMgr,
        const rtl::OUString & rUri,
        const com::sun::star::uno::Reference<
            com::sun::star::embed::XStorage >  & xParentStorage,
        const com::sun::star::uno::Reference<
            com::sun::star::io::XStream > & xStreamToWrap );

    virtual ~Stream();

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type& aType )
        throw ( com::sun::star::uno::RuntimeException );

    // XTypeProvider (implemnented by base, but needs to be overridden for
    //                delegating to aggregate)
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes()
        throw ( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw ( com::sun::star::uno::RuntimeException );

    // XStream
    virtual com::sun::star::uno::Reference<
        com::sun::star::io::XInputStream > SAL_CALL
    getInputStream()
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
        com::sun::star::io::XOutputStream > SAL_CALL
    getOutputStream()
        throw( com::sun::star::uno::RuntimeException );

    // XOutputStream
    virtual void SAL_CALL
    writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData )
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::BufferSizeExceededException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    flush()
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::BufferSizeExceededException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    closeOutput()
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException );

    // XTruncate
    virtual void SAL_CALL
    truncate()
        throw( com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException );

    // XInputStream
    virtual sal_Int32 SAL_CALL
    readBytes( com::sun::star::uno::Sequence< sal_Int8 >& aData,
               sal_Int32 nBytesToRead )
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::BufferSizeExceededException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL
    readSomeBytes( com::sun::star::uno::Sequence< sal_Int8 >& aData,
                   sal_Int32 nMaxBytesToRead )
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::BufferSizeExceededException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    skipBytes( sal_Int32 nBytesToSkip )
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::BufferSizeExceededException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL
    available()
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    closeInput()
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException );

    // XComponent
    // Note: We need to intercept this one.
    virtual void SAL_CALL
    dispose()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
            com::sun::star::lang::XEventListener >& xListener )
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
            com::sun::star::lang::XEventListener >& aListener )
        throw ( com::sun::star::uno::RuntimeException );

private:
    void commitChanges()
        throw( com::sun::star::io::IOException );

    com::sun::star::uno::Reference<
        com::sun::star::uno::XAggregation >     m_xAggProxy;
    com::sun::star::uno::Reference<
        com::sun::star::io::XStream >           m_xWrappedStream;
    com::sun::star::uno::Reference<
        com::sun::star::io::XOutputStream >     m_xWrappedOutputStream;
    com::sun::star::uno::Reference<
        com::sun::star::io::XTruncate >         m_xWrappedTruncate;
    com::sun::star::uno::Reference<
        com::sun::star::io::XInputStream >      m_xWrappedInputStream;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XComponent >      m_xWrappedComponent;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XTypeProvider >   m_xWrappedTypeProv;
};

} // namespace tdoc_ucp

#endif /* !INCLUDED_TDOC_STGELEMS_HXX */
