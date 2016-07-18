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

#ifndef INCLUDED_PACKAGE_SOURCE_XSTOR_OWRITEABLESTREAM_HXX
#define INCLUDED_PACKAGE_SOURCE_XSTOR_OWRITEABLESTREAM_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/packages/XDataSinkEncrSupport.hpp>
#include <com/sun/star/packages/NoEncryptionException.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/embed/XEncryptionProtectedSource2.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <com/sun/star/embed/XExtendedStorageStream.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/logging/XSimpleLogRing.hpp>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <comphelper/sequenceashashmap.hxx>

#include <list>
#include <memory>

#include "ocompinstream.hxx"
#include "mutexholder.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

struct MutexHolder
{
    const rtl::Reference<SotMutexHolder> m_rMutexRef;

    MutexHolder() : m_rMutexRef( new SotMutexHolder ) {}
};

namespace package {
    bool PackageEncryptionDatasEqual( const ::comphelper::SequenceAsHashMap& aHash1, const ::comphelper::SequenceAsHashMap& aHash2 );
}

struct WSInternalData_Impl;

typedef ::std::list< OInputCompStream* > InputStreamsList_Impl;

struct OStorage_Impl;
class OWriteStream;

struct OWriteStream_Impl : public MutexHolder
{
    friend struct OStorage_Impl;
    friend class OWriteStream;
    friend class OInputCompStream;

    OWriteStream*   m_pAntiImpl;
    OUString m_aTempURL;

    css::uno::Reference< css::io::XStream > m_xCacheStream;
    css::uno::Reference< css::io::XSeekable > m_xCacheSeek;

    InputStreamsList_Impl m_aInputStreamsList;

    bool                        m_bHasDataToFlush;    // only modified elements will be sent to the original content
    bool                        m_bFlushed;      // sending the streams is coordinated by the root storage of the package

    css::uno::Reference< css::packages::XDataSinkEncrSupport > m_xPackageStream;
    css::uno::Reference< css::logging::XSimpleLogRing >  m_xLogRing;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    OStorage_Impl* m_pParent;

    css::uno::Sequence< css::beans::PropertyValue > m_aProps;

    bool m_bForceEncrypted;

    bool m_bUseCommonEncryption;
    bool m_bHasCachedEncryptionData;
    ::comphelper::SequenceAsHashMap m_aEncryptionData;

    bool m_bCompressedSetExplicit;

    css::uno::Reference< css::lang::XSingleServiceFactory > m_xPackage;

    bool m_bHasInsertedStreamOptimization;

    sal_Int32 m_nStorageType;

    // Relations info related data, stored in *.rels file in OFOPXML format
    css::uno::Reference< css::io::XInputStream > m_xOrigRelInfoStream;
    css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > m_aOrigRelInfo;
    bool m_bOrigRelInfoBroken;

    css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > m_aNewRelInfo;
    css::uno::Reference< css::io::XInputStream > m_xNewRelInfoStream;
    sal_Int16 m_nRelInfoStatus;
    sal_Int32 m_nRelId;

private:
    OUString const & GetFilledTempFileIfNo( const css::uno::Reference< css::io::XInputStream >& xStream );
    OUString const & FillTempGetFileName();
    css::uno::Reference< css::io::XStream >       GetTempFileAsStream();
    css::uno::Reference< css::io::XInputStream >  GetTempFileAsInputStream();

    css::uno::Reference< css::io::XStream > GetStream_Impl( sal_Int32 nStreamMode,
                                                                                        bool bHierarchyAccess );

    ::comphelper::SequenceAsHashMap GetCommonRootEncryptionData() throw ( css::packages::NoEncryptionException );

    css::uno::Sequence< css::beans::PropertyValue > ReadPackageStreamProperties();
    css::uno::Sequence< css::beans::PropertyValue > InsertOwnProps(
                            const css::uno::Sequence< css::beans::PropertyValue >& aProps,
                            bool bUseCommonEncryption );

public:
    OWriteStream_Impl(
                OStorage_Impl* pParent,
                const css::uno::Reference< css::packages::XDataSinkEncrSupport >& xPackageStream,
                const css::uno::Reference< css::lang::XSingleServiceFactory >& xPackage,
                const css::uno::Reference< css::uno::XComponentContext >& xContext,
                bool bForceEncrypted,
                sal_Int32 nStorageType,
                bool bDefaultCompress,
                const css::uno::Reference< css::io::XInputStream >& xRelInfoStream =
                    css::uno::Reference< css::io::XInputStream >() );

    ~OWriteStream_Impl();

    void CleanCacheStream();

    void AddLog( const OUString& aMessage );

    bool UsesCommonEncryption_Impl() { return m_bUseCommonEncryption; }
    bool HasTempFile_Impl() const { return ( m_aTempURL.getLength() != 0 ); }
    bool IsTransacted();

    bool HasWriteOwner_Impl() const { return ( m_pAntiImpl != nullptr ); }

    void InsertIntoPackageFolder(
            const OUString& aName,
            const css::uno::Reference< css::container::XNameContainer >& xParentPackageFolder );

    void SetToBeCommited() { m_bFlushed = true; }

    bool HasCachedEncryptionData() { return m_bHasCachedEncryptionData; }
    ::comphelper::SequenceAsHashMap& GetCachedEncryptionData() { return m_aEncryptionData; }

    bool IsModified() { return m_bHasDataToFlush || m_bFlushed; }

    bool IsEncrypted();
    void SetDecrypted();
    void SetEncrypted( const ::comphelper::SequenceAsHashMap& aEncryptionData );

    void DisposeWrappers();

    void InsertStreamDirectly(
            const css::uno::Reference< css::io::XInputStream >& xInStream,
            const css::uno::Sequence< css::beans::PropertyValue >& aProps );

    void Commit();
    void Revert();

    css::uno::Sequence< css::beans::PropertyValue > const & GetStreamProperties();

    css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > GetAllRelationshipsIfAny();

    void CopyInternallyTo_Impl( const css::uno::Reference< css::io::XStream >& xDestStream,
                                const ::comphelper::SequenceAsHashMap& aEncryptionData );
    void CopyInternallyTo_Impl( const css::uno::Reference< css::io::XStream >& xDestStream );

    css::uno::Reference< css::io::XStream > GetStream(
                        sal_Int32 nStreamMode,
                        const ::comphelper::SequenceAsHashMap& aEncryptionData,
                        bool bHierarchyAccess );

    css::uno::Reference< css::io::XStream > GetStream(
                        sal_Int32 nStreamMode,
                        bool bHierarchyAccess );

    css::uno::Reference< css::io::XInputStream > GetRawInStream();
    css::uno::Reference< css::io::XInputStream > GetPlainRawInStream();

    void InputStreamDisposed( OInputCompStream* pStream );

    void CreateReadonlyCopyBasedOnData(
                    const css::uno::Reference< css::io::XInputStream >& xDataToCopy,
                    const css::uno::Sequence< css::beans::PropertyValue >& aProps,
                    bool bUseCommonEncryption,
                    css::uno::Reference< css::io::XStream >& xTargetStream );

    void GetCopyOfLastCommit( css::uno::Reference< css::io::XStream >& xTargetStream );
    void GetCopyOfLastCommit(
                  css::uno::Reference< css::io::XStream >& xTargetStream,
                            const ::comphelper::SequenceAsHashMap& aEncryptionData );

    void CommitStreamRelInfo(
                    const css::uno::Reference< css::embed::XStorage >& xRelStorage,
                    const OUString& aOrigStreamName,
                    const OUString& aNewStreamName );

    void ReadRelInfoIfNecessary();

    sal_Int32 GetNewRelId() { return m_nRelId ++; }
};

class OWriteStream : public css::lang::XTypeProvider
            , public css::io::XInputStream
            , public css::io::XOutputStream
            , public css::embed::XExtendedStorageStream
            , public css::io::XSeekable
            , public css::io::XTruncate
            , public css::embed::XEncryptionProtectedSource2
            , public css::embed::XRelationshipAccess
            , public css::embed::XTransactedObject
            , public css::embed::XTransactionBroadcaster
            , public css::beans::XPropertySet
            , public ::cppu::OWeakObject
{
    friend struct OWriteStream_Impl;

protected:
    css::uno::Reference < css::io::XInputStream > m_xInStream;
    css::uno::Reference < css::io::XOutputStream > m_xOutStream;
    css::uno::Reference < css::io::XSeekable > m_xSeekable;

    OWriteStream_Impl* m_pImpl;
    std::unique_ptr<WSInternalData_Impl> m_pData;

    bool m_bInStreamDisconnected;
    bool m_bInitOnDemand;
    sal_Int64 m_nInitPosition;

    bool m_bTransacted;

    OWriteStream( OWriteStream_Impl* pImpl, bool bTransacted );
    OWriteStream( OWriteStream_Impl* pImpl, css::uno::Reference< css::io::XStream > const & xStream, bool bTransacted );

    void CloseOutput_Impl();

    void CopyToStreamInternally_Impl( const css::uno::Reference< css::io::XStream >& xStream );

    void ModifyParentUnlockMutex_Impl( ::osl::ResettableMutexGuard& aGuard );

    void BroadcastTransaction( sal_Int8 nMessage );

public:

    virtual ~OWriteStream();

    void CheckInitOnDemand();
    void DeInit();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    //  XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL available(  )
        throw(css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL closeInput(  )
        throw(css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException, std::exception) override;

    // XOutputStream
    virtual void SAL_CALL writeBytes( const css::uno::Sequence< sal_Int8 >& aData ) throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL flush(  ) throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL closeOutput(  ) throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;

    //XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getPosition() throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getLength() throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;

    //XStream
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getInputStream(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL getOutputStream(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XTruncate
    virtual void SAL_CALL truncate() throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;

    //XComponent
    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    //XEncryptionProtectedSource
    virtual void SAL_CALL setEncryptionPassword( const OUString& aPass )
        throw ( css::uno::RuntimeException,
                css::io::IOException, std::exception ) override;
    virtual void SAL_CALL removeEncryption()
        throw ( css::uno::RuntimeException,
                css::io::IOException, std::exception ) override;

    //XEncryptionProtectedSource2
    virtual void SAL_CALL setEncryptionData( const css::uno::Sequence< css::beans::NamedValue >& aEncryptionData ) throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasEncryptionData() throw (css::uno::RuntimeException, std::exception) override;

    //XRelationshipAccess
    virtual sal_Bool SAL_CALL hasByID( const OUString& sID ) throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTargetByID( const OUString& sID ) throw (css::container::NoSuchElementException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTypeByID( const OUString& sID ) throw (css::container::NoSuchElementException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::StringPair > SAL_CALL getRelationshipByID( const OUString& sID ) throw (css::container::NoSuchElementException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > SAL_CALL getRelationshipsByType( const OUString& sType ) throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > SAL_CALL getAllRelationships(  ) throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertRelationshipByID( const OUString& sID, const css::uno::Sequence< css::beans::StringPair >& aEntry, sal_Bool bReplace ) throw (css::container::ElementExistException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeRelationshipByID( const OUString& sID ) throw (css::container::NoSuchElementException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertRelationships( const css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > >& aEntries, sal_Bool bReplace ) throw (css::container::ElementExistException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearRelationships(  ) throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw ( css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw ( css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw ( css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw ( css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw ( css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw ( css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception ) override;

    // XTransactedObject
    virtual void SAL_CALL commit()
        throw ( css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL revert()
        throw ( css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    // XTransactionBroadcaster
    virtual void SAL_CALL addTransactionListener(
            const css::uno::Reference< css::embed::XTransactionListener >& aListener )
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeTransactionListener(
            const css::uno::Reference< css::embed::XTransactionListener >& aListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
