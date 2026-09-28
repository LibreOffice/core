/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <rtl/ref.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cpo/uno/XAggregation.hpp>

#include "tdoc_storage.hxx"

#include <mutex>

namespace tdoc_ucp {

class OfficeDocumentsManager;

class ParentStorageHolder
{
public:
    ParentStorageHolder(
        cpo::uno::Reference< css::embed::XStorage > xParentStorage,
        const OUString & rUri );

    bool isParentARootStorage() const
    { return m_bParentIsRootStorage; }
    cpo::uno::Reference< css::embed::XStorage >
    getParentStorage() const
    {
        std::scoped_lock aGuard( m_aMutex );
        return m_xParentStorage;
    }
    void clearParentStorage()
    {
        std::scoped_lock aGuard( m_aMutex );
        m_xParentStorage = nullptr;
    }

private:
    mutable std::mutex m_aMutex;
    cpo::uno::Reference< css::embed::XStorage > m_xParentStorage;
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
        const cpo::uno::Reference< cpo::uno::XComponentContext > & rxContext,
        rtl::Reference< StorageElementFactory > xFactory,
        const OUString & rUri,
        const cpo::uno::Reference< css::embed::XStorage > & xParentStorage,
        const cpo::uno::Reference< css::embed::XStorage > & xStorageToWrap );
    virtual ~Storage() override;

    // XInterface
    virtual cpo::uno::Any queryInterface(
            const cpo::uno::Type& aType ) override;
    virtual void acquire()
        noexcept override;
    virtual void release()
        noexcept override;

    // XTypeProvider (implemented by base, but needs to be overridden for
    //                delegating to aggregate)
    virtual cpo::uno::Sequence< cpo::uno::Type >
    getTypes() override;
    virtual cpo::uno::Sequence< sal_Int8 >
    getImplementationId() override;

    // XComponent ( one of XStorage bases )
    virtual void
    dispose() override;
    virtual void
    addEventListener( const cpo::uno::Reference< css::lang::XEventListener > & xListener ) override;
    virtual void
    removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XNameAccess ( one of XStorage bases )
    virtual cpo::uno::Any
    getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString >
    getElementNames() override;
    virtual bool
    hasByName( const OUString& aName ) override;

    // XElementAccess (base of XNameAccess)
    virtual cpo::uno::Type
    getElementType() override;
    virtual bool
    hasElements() override;

    // XStorage
    virtual void
    copyToStorage( const cpo::uno::Reference< css::embed::XStorage >& xDest ) override;
    virtual cpo::uno::Reference< css::io::XStream >
    openStreamElement( const OUString& aStreamName,
                       sal_Int32 nOpenMode ) override;
    virtual cpo::uno::Reference< css::io::XStream >
    openEncryptedStreamElement( const OUString& aStreamName,
                                sal_Int32 nOpenMode,
                                const OUString& aPassword ) override;
    virtual cpo::uno::Reference< css::embed::XStorage >
    openStorageElement( const OUString& aStorName,
                        sal_Int32 nOpenMode ) override;
    virtual cpo::uno::Reference< css::io::XStream >
    cloneStreamElement( const OUString& aStreamName ) override;
    virtual cpo::uno::Reference< css::io::XStream >
    cloneEncryptedStreamElement( const OUString& aStreamName,
                                 const OUString& aPassword ) override;
    virtual void
    copyLastCommitTo( const cpo::uno::Reference<
                        css::embed::XStorage >& xTargetStorage ) override;
    virtual void
    copyStorageElementLastCommitTo( const OUString& aStorName,
                                    const cpo::uno::Reference<
                                        css::embed::XStorage > &
                                            xTargetStorage ) override;
    virtual bool
    isStreamElement( const OUString& aElementName ) override;
    virtual bool
    isStorageElement( const OUString& aElementName ) override;
    virtual void
    removeElement( const OUString& aElementName ) override;
    virtual void
    renameElement( const OUString& aEleName,
                   const OUString& aNewName ) override;
    virtual void
    copyElementTo( const OUString& aElementName,
                   const cpo::uno::Reference< css::embed::XStorage >& xDest,
                   const OUString& aNewName ) override;
    virtual void
    moveElementTo( const OUString& aElementName,
                   const cpo::uno::Reference< css::embed::XStorage >& xDest,
                   const OUString& rNewName ) override;

    // XTransactedObject
    virtual void commit() override;
    virtual void revert() override;

private:
    rtl::Reference< StorageElementFactory >         m_xFactory;
    cpo::uno::Reference< cpo::uno::XAggregation >         m_xAggProxy;
    cpo::uno::Reference< css::embed::XStorage >           m_xWrappedStorage;
    cpo::uno::Reference< css::embed::XTransactedObject >  m_xWrappedTransObj;
    cpo::uno::Reference< css::lang::XComponent >          m_xWrappedComponent;
    cpo::uno::Reference< css::lang::XTypeProvider >       m_xWrappedTypeProv;
    bool                                                  m_bIsDocumentStorage;

    StorageElementFactory::StorageMap::iterator m_aContainerIt;

    friend class StorageElementFactory;
};


typedef
    cppu::WeakImplHelper<
        css::io::XOutputStream,
        css::lang::XComponent > OutputStreamUNOBase;

class OutputStream : public OutputStreamUNOBase, public ParentStorageHolder
{
public:
    OutputStream(
        const cpo::uno::Reference< cpo::uno::XComponentContext > & rxContext,
        const OUString & rUri,
        const cpo::uno::Reference< css::embed::XStorage >  & xParentStorage,
        const cpo::uno::Reference< css::io::XOutputStream > & xStreamToWrap );
    virtual ~OutputStream() override;

    // XInterface
    virtual cpo::uno::Any
    queryInterface( const cpo::uno::Type& aType ) override;

    // XTypeProvider (implemented by base, but needs to be overridden for
    //                delegating to aggregate)
    virtual cpo::uno::Sequence< cpo::uno::Type >
    getTypes() override;
    virtual cpo::uno::Sequence< sal_Int8 >
    getImplementationId() override;

    // XOutputStream
    virtual void
    writeBytes( const cpo::uno::Sequence< sal_Int8 >& aData ) override;
    virtual void
    flush(  ) override;
    // Note: We need to intercept this one.
    virtual void
    closeOutput(  ) override;

    // XComponent
    // Note: We need to intercept this one.
    virtual void
    dispose() override;
    virtual void
    addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void
    removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;

private:
    cpo::uno::Reference<
        cpo::uno::XAggregation >     m_xAggProxy;
    cpo::uno::Reference<
        css::io::XOutputStream >     m_xWrappedStream;
    cpo::uno::Reference<
        css::lang::XComponent >      m_xWrappedComponent;
    cpo::uno::Reference<
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
        const cpo::uno::Reference< cpo::uno::XComponentContext > & rxContext,
        rtl::Reference<OfficeDocumentsManager> const & docsMgr,
        const OUString & rUri,
        const cpo::uno::Reference< css::embed::XStorage >  & xParentStorage,
        const cpo::uno::Reference< css::io::XStream > & xStreamToWrap );

    virtual ~Stream() override;

    // XInterface
    virtual cpo::uno::Any
    queryInterface( const cpo::uno::Type& aType ) override;

    // XTypeProvider (implemented by base, but needs to be overridden for
    //                delegating to aggregate)
    virtual cpo::uno::Sequence< cpo::uno::Type >
    getTypes() override;
    virtual cpo::uno::Sequence< sal_Int8 >
    getImplementationId() override;

    // XStream
    virtual cpo::uno::Reference< css::io::XInputStream >
    getInputStream() override;

    virtual cpo::uno::Reference< css::io::XOutputStream >
    getOutputStream() override;

    // XOutputStream
    virtual void
    writeBytes( const cpo::uno::Sequence< sal_Int8 >& aData ) override;

    virtual void
    flush() override;

    virtual void
    closeOutput() override;

    // XTruncate
    virtual void
    truncate() override;

    // XInputStream
    virtual sal_Int32
    readBytes( cpo::uno::Sequence< sal_Int8 >& aData,
               sal_Int32 nBytesToRead ) override;

    virtual sal_Int32
    readSomeBytes( cpo::uno::Sequence< sal_Int8 >& aData,
                   sal_Int32 nMaxBytesToRead ) override;

    virtual void
    skipBytes( sal_Int32 nBytesToSkip ) override;

    virtual sal_Int32
    available() override;

    virtual void
    closeInput() override;

    // XComponent
    // Note: We need to intercept this one.
    virtual void
    dispose() override;
    virtual void
    addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void
    removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;

private:
    /// @throws css::io::IOException
    void commitChanges();

    rtl::Reference<OfficeDocumentsManager> m_docsMgr;
    OUString m_uri;
    cpo::uno::Reference<
        cpo::uno::XAggregation >     m_xAggProxy;
    cpo::uno::Reference<
        css::io::XStream >           m_xWrappedStream;
    cpo::uno::Reference<
        css::io::XOutputStream >     m_xWrappedOutputStream;
    cpo::uno::Reference<
        css::io::XTruncate >         m_xWrappedTruncate;
    cpo::uno::Reference<
        css::io::XInputStream >      m_xWrappedInputStream;
    cpo::uno::Reference<
        css::lang::XComponent >      m_xWrappedComponent;
    cpo::uno::Reference<
        css::lang::XTypeProvider >   m_xWrappedTypeProv;
};

} // namespace tdoc_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
