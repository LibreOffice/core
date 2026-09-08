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

#ifndef INCLUDED_PACKAGE_SOURCE_XSTOR_OWRITEABLESTREAM_HXX
#define INCLUDED_PACKAGE_SOURCE_XSTOR_OWRITEABLESTREAM_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/packages/XDataSinkEncrSupport.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/embed/XEncryptionProtectedSource2.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <com/sun/star/embed/XExtendedStorageStream.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/StringPair.hpp>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/multicontainer2.hxx>

#include <comphelper/bytereader.hxx>
#include <comphelper/refcountedmutex.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/tempfile.hxx>

#include <vector>
#include <memory>
#include <string_view>

#include "ocompinstream.hxx"

namespace cpo::uno { class XComponentContext; }

namespace package {
    // all data in aHash1 is contained in aHash2
    bool PackageEncryptionDataLessOrEqual( const ::comphelper::SequenceAsHashMap& aHash1, const ::comphelper::SequenceAsHashMap& aHash2 );
}

struct OStorage_Impl;
class OWriteStream;

struct OWriteStream_Impl
{
    rtl::Reference<comphelper::RefCountedMutex> m_xMutex;

    friend struct OStorage_Impl;
    friend class OWriteStream;
    friend class OInputCompStream;

    OWriteStream*   m_pAntiImpl;
    std::optional<utl::TempFileFast> m_oTempFile;

    cpo::uno::Reference< css::io::XStream > m_xCacheStream;
    cpo::uno::Reference< css::io::XSeekable > m_xCacheSeek;

    std::vector< OInputCompStream* > m_aInputStreamsVector;

    bool                        m_bHasDataToFlush;    // only modified elements will be sent to the original content
    bool                        m_bFlushed;      // sending the streams is coordinated by the root storage of the package

    cpo::uno::Reference< css::packages::XDataSinkEncrSupport > m_xPackageStream;

    cpo::uno::Reference< cpo::uno::XComponentContext > m_xContext;

    OStorage_Impl* m_pParent;

    cpo::uno::Sequence< css::beans::PropertyValue > m_aProps;

    bool m_bForceEncrypted;

    bool m_bUseCommonEncryption;
    bool m_bHasCachedEncryptionData;
    ::comphelper::SequenceAsHashMap m_aEncryptionData;

    bool m_bCompressedSetExplicit;

    cpo::uno::Reference< css::lang::XSingleServiceFactory > m_xPackage;

    bool m_bHasInsertedStreamOptimization;

    sal_Int32 m_nStorageType;

    // Relations info related data, stored in *.rels file in OFOPXML format
    cpo::uno::Reference< css::io::XInputStream > m_xOrigRelInfoStream;
    cpo::uno::Sequence< cpo::uno::Sequence< css::beans::StringPair > > m_aOrigRelInfo;
    bool m_bOrigRelInfoBroken;

    cpo::uno::Sequence< cpo::uno::Sequence< css::beans::StringPair > > m_aNewRelInfo;
    cpo::uno::Reference< css::io::XInputStream > m_xNewRelInfoStream;
    sal_Int16 m_nRelInfoStatus;
    sal_Int32 m_nRelId;

private:
    void GetFilledTempFileIfNo( const cpo::uno::Reference< css::io::XInputStream >& xStream );
    void FillTempGetFileName();
    cpo::uno::Reference< css::io::XStream >       GetTempFileAsStream();
    cpo::uno::Reference< css::io::XInputStream >  GetTempFileAsInputStream();

    cpo::uno::Reference< css::io::XStream > GetStream_Impl( sal_Int32 nStreamMode,
                                                                                        bool bHierarchyAccess );

    /// @throws css::packages::NoEncryptionException
    ::comphelper::SequenceAsHashMap GetCommonRootEncryptionData();

    cpo::uno::Sequence< css::beans::PropertyValue > ReadPackageStreamProperties();
    cpo::uno::Sequence< css::beans::PropertyValue > InsertOwnProps(
                            const cpo::uno::Sequence< css::beans::PropertyValue >& aProps,
                            bool bUseCommonEncryption );

public:
    OWriteStream_Impl(
                OStorage_Impl* pParent,
                const cpo::uno::Reference< css::packages::XDataSinkEncrSupport >& xPackageStream,
                const cpo::uno::Reference< css::lang::XSingleServiceFactory >& xPackage,
                cpo::uno::Reference< cpo::uno::XComponentContext > xContext,
                bool bForceEncrypted,
                sal_Int32 nStorageType,
                bool bDefaultCompress,
                cpo::uno::Reference< css::io::XInputStream > xRelInfoStream =
                    cpo::uno::Reference< css::io::XInputStream >() );

    ~OWriteStream_Impl();

    void CleanCacheStream();

    bool UsesCommonEncryption_Impl() const { return m_bUseCommonEncryption; }
    bool HasTempFile_Impl() const { return m_oTempFile.has_value(); }
    bool IsTransacted();

    bool HasWriteOwner_Impl() const { return ( m_pAntiImpl != nullptr ); }

    void InsertIntoPackageFolder(
            const OUString& aName,
            const cpo::uno::Reference< css::container::XNameContainer >& xParentPackageFolder );

    void SetToBeCommited() { m_bFlushed = true; }

    bool HasCachedEncryptionData() const { return m_bHasCachedEncryptionData; }
    ::comphelper::SequenceAsHashMap& GetCachedEncryptionData() { return m_aEncryptionData; }

    bool IsModified() const { return m_bHasDataToFlush || m_bFlushed; }

    bool IsEncrypted();
    void SetDecrypted();
    void SetEncrypted( const ::comphelper::SequenceAsHashMap& aEncryptionData );

    void DisposeWrappers();

    void InsertStreamDirectly(
            const cpo::uno::Reference< css::io::XInputStream >& xInStream,
            const cpo::uno::Sequence< css::beans::PropertyValue >& aProps );

    void Commit();
    void Revert();

    cpo::uno::Sequence< css::beans::PropertyValue > const & GetStreamProperties();

    cpo::uno::Sequence< cpo::uno::Sequence< css::beans::StringPair > > GetAllRelationshipsIfAny();

    void CopyInternallyTo_Impl( const cpo::uno::Reference< css::io::XStream >& xDestStream,
                                const ::comphelper::SequenceAsHashMap& aEncryptionData );
    void CopyInternallyTo_Impl( const cpo::uno::Reference< css::io::XStream >& xDestStream );

    cpo::uno::Reference< css::io::XStream > GetStream(
                        sal_Int32 nStreamMode,
                        const ::comphelper::SequenceAsHashMap& aEncryptionData,
                        bool bHierarchyAccess );

    cpo::uno::Reference< css::io::XStream > GetStream(
                        sal_Int32 nStreamMode,
                        bool bHierarchyAccess );

    cpo::uno::Reference< css::io::XInputStream > GetRawInStream();
    cpo::uno::Reference< css::io::XInputStream > GetPlainRawInStream();

    void InputStreamDisposed( OInputCompStream* pStream );

    void CreateReadonlyCopyBasedOnData(
                    const cpo::uno::Reference< css::io::XInputStream >& xDataToCopy,
                    const cpo::uno::Sequence< css::beans::PropertyValue >& aProps,
                    cpo::uno::Reference< css::io::XStream >& xTargetStream );

    void GetCopyOfLastCommit( cpo::uno::Reference< css::io::XStream >& xTargetStream );
    void GetCopyOfLastCommit(
                  cpo::uno::Reference< css::io::XStream >& xTargetStream,
                            const ::comphelper::SequenceAsHashMap& aEncryptionData );

    void CommitStreamRelInfo(
                    const cpo::uno::Reference< css::embed::XStorage >& xRelStorage,
                    std::u16string_view aOrigStreamName,
                    std::u16string_view aNewStreamName );

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
            , public comphelper::ByteReader
            , public comphelper::ByteWriter
{
    friend struct OWriteStream_Impl;

protected:
    cpo::uno::Reference < css::io::XInputStream > m_xInStream;
    cpo::uno::Reference < css::io::XOutputStream > m_xOutStream;
    cpo::uno::Reference < css::io::XSeekable > m_xSeekable;

    OWriteStream_Impl* m_pImpl;
    rtl::Reference<comphelper::RefCountedMutex> m_xSharedMutex;
    ::std::optional< ::cppu::OTypeCollection> m_oTypeCollection;
    comphelper::OMultiTypeInterfaceContainerHelper2 m_aListenersContainer; // list of listeners
    sal_Int32 m_nStorageType;

    bool m_bInStreamDisconnected;
    bool m_bInitOnDemand;
    sal_Int64 m_nInitPosition;

    bool m_bTransacted;

    OWriteStream( OWriteStream_Impl& rImpl, bool bTransacted );
    OWriteStream( OWriteStream_Impl& rImpl, cpo::uno::Reference< css::io::XStream > const & xStream, bool bTransacted );

    void CloseOutput_Impl();

    void CopyToStreamInternally_Impl( const cpo::uno::Reference< css::io::XStream >& xStream );

    void ModifyParentUnlockMutex_Impl(osl::ClearableMutexGuard& aGuard);

    void BroadcastTransaction( sal_Int8 nMessage );

public:

    virtual ~OWriteStream() override;

    // XInterface
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type& rType ) override;
    virtual void acquire() noexcept override;
    virtual void release() noexcept override;

    //  XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;
    virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override;

    // XInputStream
    virtual sal_Int32 readBytes( cpo::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) override;
    virtual sal_Int32 readSomeBytes( cpo::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) override;
    virtual void skipBytes( sal_Int32 nBytesToSkip ) override;
    virtual sal_Int32 available(  ) override;
    virtual void closeInput(  ) override;

    // XOutputStream
    virtual void writeBytes( const cpo::uno::Sequence< sal_Int8 >& aData ) override;
    virtual void flush(  ) override;
    virtual void closeOutput(  ) override;

    //XSeekable
    virtual void seek( sal_Int64 location ) override;
    virtual sal_Int64 getPosition() override;
    virtual sal_Int64 getLength() override;

    //XStream
    virtual cpo::uno::Reference< css::io::XInputStream > getInputStream(  ) override;
    virtual cpo::uno::Reference< css::io::XOutputStream > getOutputStream(  ) override;

    // XTruncate
    virtual void truncate() override;

    //XComponent
    virtual void dispose() override;
    virtual void addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;

    //XEncryptionProtectedSource
    virtual void setEncryptionPassword( const OUString& aPass ) override;
    virtual void removeEncryption() override;

    //XEncryptionProtectedSource2
    virtual void setEncryptionData( const cpo::uno::Sequence< css::beans::NamedValue >& aEncryptionData ) override;
    virtual bool hasEncryptionData() override;

    //XRelationshipAccess
    virtual bool hasByID( const OUString& sID ) override;
    virtual OUString getTargetByID( const OUString& sID ) override;
    virtual OUString getTypeByID( const OUString& sID ) override;
    virtual cpo::uno::Sequence< css::beans::StringPair > getRelationshipByID( const OUString& sID ) override;
    virtual cpo::uno::Sequence< cpo::uno::Sequence< css::beans::StringPair > > getRelationshipsByType( const OUString& sType ) override;
    virtual cpo::uno::Sequence< cpo::uno::Sequence< css::beans::StringPair > > getAllRelationships(  ) override;
    virtual void insertRelationshipByID( const OUString& sID, const cpo::uno::Sequence< css::beans::StringPair >& aEntry, bool bReplace ) override;
    virtual void removeRelationshipByID( const OUString& sID ) override;
    virtual void insertRelationships( const cpo::uno::Sequence< cpo::uno::Sequence< css::beans::StringPair > >& aEntries, bool bReplace ) override;
    virtual void clearRelationships(  ) override;

    //XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() override;
    virtual void setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    virtual void addPropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void removePropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void addVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void removeVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XTransactedObject
    virtual void commit() override;
    virtual void revert() override;

    // XTransactionBroadcaster
    virtual void addTransactionListener(
            const cpo::uno::Reference< css::embed::XTransactionListener >& aListener ) override;
    virtual void removeTransactionListener(
            const cpo::uno::Reference< css::embed::XTransactionListener >& aListener ) override;

    // comphelper::ByteReader
    virtual sal_Int32 readSomeBytes(sal_Int8* aData, sal_Int32 nBytesToRead) override;

    // comphelper::ByteWriter
    virtual void writeBytes(const sal_Int8* aData, sal_Int32 nBytesToWrite) override;

private:
    void CheckInitOnDemand();
    void CheckInitOnWriteDemand(sal_Int32 dataSize);
    void DeInit();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
