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

#ifndef _WRITESTREAM_HXX_
#define _WRITESTREAM_HXX_

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

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <comphelper/sequenceashashmap.hxx>

#include <list>

#include "ocompinstream.hxx"
#include "mutexholder.hxx"

struct PreCreationStruct
{
    SotMutexHolderRef m_rMutexRef;

    PreCreationStruct()
    : m_rMutexRef( new SotMutexHolder )
    {}

};

namespace cppu {
    class OTypeCollection;
}

namespace package {
    void StaticAddLog( const OUString& aMessage );
    bool PackageEncryptionDatasEqual( const ::comphelper::SequenceAsHashMap& aHash1, const ::comphelper::SequenceAsHashMap& aHash2 );
}

struct WSInternalData_Impl
{
    SotMutexHolderRef m_rSharedMutexRef;
    ::cppu::OTypeCollection* m_pTypeCollection;
    ::cppu::OMultiTypeInterfaceContainerHelper m_aListenersContainer; // list of listeners
    sal_Int32 m_nStorageType;

    // the mutex reference MUST NOT be empty
    WSInternalData_Impl( const SotMutexHolderRef rMutexRef, sal_Int32 nStorageType )
    : m_rSharedMutexRef( rMutexRef )
    , m_pTypeCollection( NULL )
    , m_aListenersContainer( rMutexRef->GetMutex() )
    , m_nStorageType( nStorageType )
    {}
};

typedef ::std::list< OInputCompStream* > InputStreamsList_Impl;

struct OStorage_Impl;
class OWriteStream;

struct OWriteStream_Impl : public PreCreationStruct
{
    friend struct OStorage_Impl;
    friend class OWriteStream;
    friend class OInputCompStream;

    OWriteStream*   m_pAntiImpl;
    OUString m_aTempURL;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xCacheStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable > m_xCacheSeek;

    InputStreamsList_Impl m_aInputStreamsList;

    sal_Bool                        m_bHasDataToFlush;    // only modified elements will be sent to the original content
    sal_Bool                        m_bFlushed;      // sending the streams is coordinated by the root storage of the package

    ::com::sun::star::uno::Reference< ::com::sun::star::packages::XDataSinkEncrSupport > m_xPackageStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::logging::XSimpleLogRing >  m_xLogRing;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

    OStorage_Impl* m_pParent;

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_aProps;

    sal_Bool m_bForceEncrypted;

    sal_Bool m_bUseCommonEncryption;
    sal_Bool m_bHasCachedEncryptionData;
    ::comphelper::SequenceAsHashMap m_aEncryptionData;

    sal_Bool m_bCompressedSetExplicit;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > m_xPackage;

    sal_Bool m_bHasInsertedStreamOptimization;

    sal_Int32 m_nStorageType;

    // Relations info related data, stored in *.rels file in OFOPXML format
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xOrigRelInfoStream;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > m_aOrigRelInfo;
    sal_Bool m_bOrigRelInfoBroken;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > m_aNewRelInfo;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xNewRelInfoStream;
    sal_Int16 m_nRelInfoStatus;
    sal_Int32 m_nRelId;

private:
    OUString GetFilledTempFileIfNo( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xStream );
    OUString FillTempGetFileName();
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >       GetTempFileAsStream();
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  GetTempFileAsInputStream();

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > GetStream_Impl( sal_Int32 nStreamMode,
                                                                                        sal_Bool bHierarchyAccess );

    ::comphelper::SequenceAsHashMap GetCommonRootEncryptionData() throw ( ::com::sun::star::packages::NoEncryptionException );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > ReadPackageStreamProperties();
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > InsertOwnProps(
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps,
                            sal_Bool bUseCommonEncryption );

public:
    OWriteStream_Impl(
                OStorage_Impl* pParent,
                const ::com::sun::star::uno::Reference< ::com::sun::star::packages::XDataSinkEncrSupport >& xPackageStream,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >& xPackage,
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
                sal_Bool bForceEncrypted,
                sal_Int32 nStorageType,
                sal_Bool bDefaultCompress,
                const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xRelInfoStream =
                    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >() );

    ~OWriteStream_Impl();

    void CleanCacheStream();

    void AddLog( const OUString& aMessage );

    sal_Bool UsesCommonEncryption_Impl() { return m_bUseCommonEncryption; }
    sal_Bool HasTempFile_Impl() const { return ( m_aTempURL.getLength() != 0 ); }
    sal_Bool IsTransacted();

    sal_Bool HasWriteOwner_Impl() const { return ( m_pAntiImpl != NULL ); }

    void InsertIntoPackageFolder(
            const OUString& aName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xParentPackageFolder );

    void SetToBeCommited() { m_bFlushed = sal_True; }

    sal_Bool HasCachedEncryptionData() { return m_bHasCachedEncryptionData; }
    ::comphelper::SequenceAsHashMap& GetCachedEncryptionData() { return m_aEncryptionData; }

    sal_Bool IsModified() { return m_bHasDataToFlush || m_bFlushed; }

    sal_Bool IsEncrypted();
    void SetDecrypted();
    void SetEncrypted( const ::comphelper::SequenceAsHashMap& aEncryptionData );

    void DisposeWrappers();

    void InsertStreamDirectly(
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps );

    void Commit();
    void Revert();

    void Free( sal_Bool bMust ); // allows to try to disconnect from the temporary stream
                                 // in case bMust is set to sal_True the method
                                // will throw exception in case the file is still busy

    void SetModified(); // can be done only by parent storage after renaming

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetStreamProperties();

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > GetAllRelationshipsIfAny();

    void CopyInternallyTo_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xDestStream,
                                const ::comphelper::SequenceAsHashMap& aEncryptionData );
    void CopyInternallyTo_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xDestStream );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > GetStream(
                        sal_Int32 nStreamMode,
                        const ::comphelper::SequenceAsHashMap& aEncryptionData,
                        sal_Bool bHierarchyAccess );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > GetStream(
                        sal_Int32 nStreamMode,
                        sal_Bool bHierarchyAccess );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetRawInStream();
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetPlainRawInStream();

    void InputStreamDisposed( OInputCompStream* pStream );

    void CreateReadonlyCopyBasedOnData(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xDataToCopy,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps,
                    sal_Bool bUseCommonEncryption,
                    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xTargetStream );

    void GetCopyOfLastCommit( ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xTargetStream );
    void GetCopyOfLastCommit(
                  ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xTargetStream,
                            const ::comphelper::SequenceAsHashMap& aEncryptionData );

    void CommitStreamRelInfo(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xRelStorage,
                    const OUString& aOrigStreamName,
                    const OUString& aNewStreamName );

    void ReadRelInfoIfNecessary();

    sal_Int32 GetNewRelId() { return m_nRelId ++; }
};

class OWriteStream : ::com::sun::star::lang::XTypeProvider
            , public ::com::sun::star::io::XInputStream
            , public ::com::sun::star::io::XOutputStream
            , public ::com::sun::star::embed::XExtendedStorageStream
            , public ::com::sun::star::io::XSeekable
            , public ::com::sun::star::io::XTruncate
            , public ::com::sun::star::embed::XEncryptionProtectedSource2
            , public ::com::sun::star::embed::XRelationshipAccess
            , public ::com::sun::star::embed::XTransactedObject
            , public ::com::sun::star::embed::XTransactionBroadcaster
            , public ::com::sun::star::beans::XPropertySet
            , public ::cppu::OWeakObject
{
    friend struct OWriteStream_Impl;

protected:
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > m_xInStream;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > m_xOutStream;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XSeekable > m_xSeekable;

    OWriteStream_Impl* m_pImpl;
    WSInternalData_Impl* m_pData;

    sal_Bool m_bInStreamDisconnected;
    sal_Bool m_bInitOnDemand;
    sal_Int64 m_nInitPosition;

    sal_Bool m_bTransacted;

    OWriteStream( OWriteStream_Impl* pImpl, sal_Bool bTransacted );
    OWriteStream( OWriteStream_Impl* pImpl, ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > xStream, sal_Bool bTransacted );

    void CloseOutput_Impl();

    void CopyToStreamInternally_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xStream );

    void ModifyParentUnlockMutex_Impl( ::osl::ResettableMutexGuard& aGuard );

    void BroadcastTransaction( sal_Int8 nMessage );

public:

    virtual ~OWriteStream();

    void CheckInitOnDemand();
    void DeInit();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    //  XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( ::com::sun::star::uno::RuntimeException );

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL available(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeInput(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XOutputStream
    virtual void SAL_CALL writeBytes( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL flush(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeOutput(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    //XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    //XStream
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  ) throw (::com::sun::star::uno::RuntimeException);

    // XTruncate
    virtual void SAL_CALL truncate() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    //XEncryptionProtectedSource
    virtual void SAL_CALL setEncryptionPassword( const OUString& aPass )
        throw ( ::com::sun::star::uno::RuntimeException,
                ::com::sun::star::io::IOException );
    virtual void SAL_CALL removeEncryption()
        throw ( ::com::sun::star::uno::RuntimeException,
                ::com::sun::star::io::IOException );

    //XEncryptionProtectedSource2
    virtual void SAL_CALL setEncryptionData( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aEncryptionData ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasEncryptionData() throw (::com::sun::star::uno::RuntimeException);

    //XRelationshipAccess
    virtual ::sal_Bool SAL_CALL hasByID( const OUString& sID ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getTargetByID( const OUString& sID ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getTypeByID( const OUString& sID ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > SAL_CALL getRelationshipByID( const OUString& sID ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > SAL_CALL getRelationshipsByType( const OUString& sType ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > SAL_CALL getAllRelationships(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertRelationshipByID( const OUString& sID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aEntry, ::sal_Bool bReplace ) throw (::com::sun::star::container::ElementExistException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRelationshipByID( const OUString& sID ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertRelationships( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > >& aEntries, ::sal_Bool bReplace ) throw (::com::sun::star::container::ElementExistException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clearRelationships(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    // XTransactedObject
    virtual void SAL_CALL commit()
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL revert()
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    // XTransactionBroadcaster
    virtual void SAL_CALL addTransactionListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XTransactionListener >& aListener )
        throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeTransactionListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XTransactionListener >& aListener )
        throw ( ::com::sun::star::uno::RuntimeException );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
