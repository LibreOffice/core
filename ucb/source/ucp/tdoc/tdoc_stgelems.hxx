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

#ifndef INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_STGELEMS_HXX
#define INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_STGELEMS_HXX

#include <memory>

#include "osl/mutex.hxx"
#include "rtl/ref.hxx"

#include <cppuhelper/implbase.hxx>

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



class ParentStorageHolder : public MutexHolder
{
public:
    ParentStorageHolder(
        const css::uno::Reference< css::embed::XStorage > & xParentStorage,
        const OUString & rUri );

    bool isParentARootStorage() const
    { return m_bParentIsRootStorage; }
    css::uno::Reference< css::embed::XStorage >
    getParentStorage() const
    { return m_xParentStorage; }
    void setParentStorage( const css::uno::Reference< css::embed::XStorage > & xStg )
    { osl::MutexGuard aGuard( m_aMutex ); m_xParentStorage = xStg; }

private:
    css::uno::Reference< css::embed::XStorage > m_xParentStorage;
    bool                                  m_bParentIsRootStorage;
};



typedef
    cppu::WeakImplHelper<
        css::embed::XStorage,
        css::embed::XTransactedObject > StorageUNOBase;

class Storage : public StorageUNOBase, public ParentStorageHolder
{
public:
    Storage(
        const css::uno::Reference< css::uno::XComponentContext > & rxContext,
        const rtl::Reference< StorageElementFactory >  & xFactory,
        const OUString & rUri,
        const css::uno::Reference< css::embed::XStorage > & xParentStorage,
        const css::uno::Reference< css::embed::XStorage > & xStorageToWrap );
    virtual ~Storage();

    bool isDocumentStorage() const { return m_bIsDocumentStorage; }

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface(
            const css::uno::Type& aType )
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw () override;
    virtual void SAL_CALL release()
        throw () override;

    // XTypeProvider (implemnented by base, but needs to be overridden for
    //                delegating to aggregate)
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XComponent ( one of XStorage bases )
    virtual void SAL_CALL
    dispose()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener > & xListener )
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XNameAccess ( one of XStorage bases )
    virtual css::uno::Any SAL_CALL
    getByName( const OUString& aName )
        throw ( css::container::NoSuchElementException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
    getElementNames()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    hasByName( const OUString& aName )
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XElementAccess (base of XNameAccess)
    virtual css::uno::Type SAL_CALL
    getElementType()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    hasElements()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XStorage
    virtual void SAL_CALL
    copyToStorage( const css::uno::Reference< css::embed::XStorage >& xDest )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::io::XStream > SAL_CALL
    openStreamElement( const OUString& aStreamName,
                       sal_Int32 nOpenMode )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::io::XStream > SAL_CALL
    openEncryptedStreamElement( const OUString& aStreamName,
                                sal_Int32 nOpenMode,
                                const OUString& aPassword )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoEncryptionException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::embed::XStorage > SAL_CALL
    openStorageElement( const OUString& aStorName,
                        sal_Int32 nOpenMode )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::io::XStream > SAL_CALL
    cloneStreamElement( const OUString& aStreamName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::io::XStream > SAL_CALL
    cloneEncryptedStreamElement( const OUString& aStreamName,
                                 const OUString& aPassword )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoEncryptionException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    copyLastCommitTo( const css::uno::Reference<
                        css::embed::XStorage >& xTargetStorage )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    copyStorageElementLastCommitTo( const OUString& aStorName,
                                    const css::uno::Reference<
                                        css::embed::XStorage > &
                                            xTargetStorage )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    isStreamElement( const OUString& aElementName )
        throw ( css::container::NoSuchElementException,
                css::lang::IllegalArgumentException,
                css::embed::InvalidStorageException,
                css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    isStorageElement( const OUString& aElementName )
        throw ( css::container::NoSuchElementException,
                css::lang::IllegalArgumentException,
                css::embed::InvalidStorageException,
                css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    removeElement( const OUString& aElementName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    renameElement( const OUString& aEleName,
                   const OUString& aNewName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::container::ElementExistException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    copyElementTo( const OUString& aElementName,
                   const css::uno::Reference< css::embed::XStorage >& xDest,
                   const OUString& aNewName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::container::ElementExistException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    moveElementTo( const OUString& aElementName,
                   const css::uno::Reference< css::embed::XStorage >& xDest,
                   const OUString& rNewName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::container::ElementExistException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    // XTransactedObject
    virtual void SAL_CALL commit()
        throw ( css::io::IOException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL revert()
        throw ( css::io::IOException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

private:
    explicit Storage( const rtl::Reference< Storage > & rFactory ); // n.i.

    rtl::Reference< StorageElementFactory >         m_xFactory;
    css::uno::Reference< css::uno::XAggregation >         m_xAggProxy;
    css::uno::Reference< css::embed::XStorage >           m_xWrappedStorage;
    css::uno::Reference< css::embed::XTransactedObject >  m_xWrappedTransObj;
    css::uno::Reference< css::lang::XComponent >          m_xWrappedComponent;
    css::uno::Reference< css::lang::XTypeProvider >       m_xWrappedTypeProv;
    bool                                            m_bIsDocumentStorage;

    StorageElementFactory::StorageMap::iterator m_aContainerIt;

    friend class StorageElementFactory;
    friend class std::unique_ptr< Storage >;
};



typedef
    cppu::WeakImplHelper<
        css::io::XOutputStream,
        css::lang::XComponent > OutputStreamUNOBase;

class OutputStream : public OutputStreamUNOBase, public ParentStorageHolder
{
public:
    OutputStream(
        const css::uno::Reference< css::uno::XComponentContext > & rxContext,
        const OUString & rUri,
        const css::uno::Reference< css::embed::XStorage >  & xParentStorage,
        const css::uno::Reference< css::io::XOutputStream > & xStreamToWrap );
    virtual ~OutputStream();

    // XInterface
    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type& aType )
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XTypeProvider (implemnented by base, but needs to be overridden for
    //                delegating to aggregate)
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XOutputStream
    virtual void SAL_CALL
    writeBytes( const css::uno::Sequence< sal_Int8 >& aData )
        throw ( css::io::NotConnectedException,
                css::io::BufferSizeExceededException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    flush(  )
        throw ( css::io::NotConnectedException,
                css::io::BufferSizeExceededException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception ) override;
    // Note: We need to intercept this one.
    virtual void SAL_CALL
    closeOutput(  )
        throw ( css::io::NotConnectedException,
                css::io::BufferSizeExceededException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception ) override;

    // XComponent
    // Note: We need to intercept this one.
    virtual void SAL_CALL
    dispose()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

private:
    css::uno::Reference<
        css::uno::XAggregation >     m_xAggProxy;
    css::uno::Reference<
        css::io::XOutputStream >     m_xWrappedStream;
    css::uno::Reference<
        css::lang::XComponent >      m_xWrappedComponent;
    css::uno::Reference<
        css::lang::XTypeProvider >   m_xWrappedTypeProv;
};



typedef cppu::WeakImplHelper< css::io::XStream,
                               css::io::XOutputStream,
                               css::io::XTruncate,
                               css::io::XInputStream,
                               css::lang::XComponent >
        StreamUNOBase;

class Stream : public StreamUNOBase, public ParentStorageHolder
{
public:
    Stream(
        const css::uno::Reference< css::uno::XComponentContext > & rxContext,
        const OUString & rUri,
        const css::uno::Reference< css::embed::XStorage >  & xParentStorage,
        const css::uno::Reference< css::io::XStream > & xStreamToWrap );

    virtual ~Stream();

    // XInterface
    virtual css::uno::Any SAL_CALL
    queryInterface( const css::uno::Type& aType )
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XTypeProvider (implemnented by base, but needs to be overridden for
    //                delegating to aggregate)
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XStream
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL
    getInputStream()
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL
    getOutputStream()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XOutputStream
    virtual void SAL_CALL
    writeBytes( const css::uno::Sequence< sal_Int8 >& aData )
        throw( css::io::NotConnectedException,
               css::io::BufferSizeExceededException,
               css::io::IOException,
               css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    flush()
        throw( css::io::NotConnectedException,
               css::io::BufferSizeExceededException,
               css::io::IOException,
               css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    closeOutput()
        throw( css::io::NotConnectedException,
               css::io::IOException,
               css::uno::RuntimeException, std::exception ) override;

    // XTruncate
    virtual void SAL_CALL
    truncate()
        throw( css::io::IOException,
               css::uno::RuntimeException, std::exception ) override;

    // XInputStream
    virtual sal_Int32 SAL_CALL
    readBytes( css::uno::Sequence< sal_Int8 >& aData,
               sal_Int32 nBytesToRead )
        throw( css::io::NotConnectedException,
               css::io::BufferSizeExceededException,
               css::io::IOException,
               css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
    readSomeBytes( css::uno::Sequence< sal_Int8 >& aData,
                   sal_Int32 nMaxBytesToRead )
        throw( css::io::NotConnectedException,
               css::io::BufferSizeExceededException,
               css::io::IOException,
               css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
    skipBytes( sal_Int32 nBytesToSkip )
        throw( css::io::NotConnectedException,
               css::io::BufferSizeExceededException,
               css::io::IOException,
               css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
    available()
        throw( css::io::NotConnectedException,
               css::io::IOException,
               css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
    closeInput()
        throw( css::io::NotConnectedException,
               css::io::IOException,
               css::uno::RuntimeException, std::exception ) override;

    // XComponent
    // Note: We need to intercept this one.
    virtual void SAL_CALL
    dispose()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

private:
    void commitChanges()
        throw( css::io::IOException );

    css::uno::Reference<
        css::uno::XAggregation >     m_xAggProxy;
    css::uno::Reference<
        css::io::XStream >           m_xWrappedStream;
    css::uno::Reference<
        css::io::XOutputStream >     m_xWrappedOutputStream;
    css::uno::Reference<
        css::io::XTruncate >         m_xWrappedTruncate;
    css::uno::Reference<
        css::io::XInputStream >      m_xWrappedInputStream;
    css::uno::Reference<
        css::lang::XComponent >      m_xWrappedComponent;
    css::uno::Reference<
        css::lang::XTypeProvider >   m_xWrappedTypeProv;
};

} // namespace tdoc_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_STGELEMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
