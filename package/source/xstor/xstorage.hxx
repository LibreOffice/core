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

#ifndef INCLUDED_PACKAGE_SOURCE_XSTOR_XSTORAGE_HXX
#define INCLUDED_PACKAGE_SOURCE_XSTOR_XSTORAGE_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/embed/XStorage2.hpp>
#include <com/sun/star/embed/XOptimizedStorage.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess2.hpp>
#include <com/sun/star/embed/XStorageRawAccess.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/XEncryptionProtectedStorage.hpp>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/packages/NoEncryptionException.hpp>
#include <com/sun/star/logging/XSimpleLogRing.hpp>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/sequenceashashmap.hxx>

#include "mutexholder.hxx"

#include <list>
#include <memory>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

#define RELINFO_NO_INIT             1
#define RELINFO_READ                2
#define RELINFO_CHANGED             3
#define RELINFO_CHANGED_STREAM      4
#define RELINFO_CHANGED_STREAM_READ 5
#define RELINFO_BROKEN              6
#define RELINFO_CHANGED_BROKEN      7

#define STOR_MESS_PRECOMMIT 1
#define STOR_MESS_COMMITED  2
#define STOR_MESS_PREREVERT 3
#define STOR_MESS_REVERTED  4

// a common implementation for an entry

struct StorInternalData_Impl;
struct OStorage_Impl;
struct OWriteStream_Impl;

struct SotElement_Impl
{
    OUString             m_aName;
    OUString             m_aOriginalName;
    bool                    m_bIsRemoved;
    bool                    m_bIsInserted;
    bool                    m_bIsStorage;

    OStorage_Impl*              m_pStorage;
    OWriteStream_Impl*          m_pStream;

public:
                                SotElement_Impl( const OUString& rName, bool bStor, bool bNew );
                                ~SotElement_Impl();
};

typedef ::std::list< SotElement_Impl* > SotElementList_Impl;

// Main storage implementation

class OStorage;

struct StorageHolder_Impl
{
    OStorage* m_pPointer;
    css::uno::WeakReference< css::embed::XStorage > m_xWeakRef;

    explicit inline StorageHolder_Impl( OStorage* pStorage );

    StorageHolder_Impl( const StorageHolder_Impl& aSH )
    : m_pPointer( aSH.m_pPointer )
    , m_xWeakRef( aSH.m_xWeakRef )
    {
    }
};

class SwitchablePersistenceStream;
struct OStorage_Impl
{
    typedef std::list<StorageHolder_Impl> StorageHoldersType;

    rtl::Reference<SotMutexHolder> m_rMutexRef;

    OStorage*                   m_pAntiImpl;         // only valid if external references exists
    StorageHoldersType          m_aReadOnlyWrapList; // only valid if readonly external reference exists

    sal_Int32                   m_nStorageMode; // open mode ( read/write/trunc/nocreate )
    bool                        m_bIsModified;  // only modified elements will be sent to the original content
    bool                        m_bBroadcastModified;  // will be set if notification is required

    bool                        m_bCommited;    // sending the streams is coordinated by the root storage of the package

    bool                        m_bIsRoot;      // marks this storage as root storages that manages all commits and reverts
    bool                        m_bListCreated;

    /// Count of registered modification listeners
    oslInterlockedCount         m_nModifiedListenerCount;
    bool                        HasModifiedListener()
    {
        return m_nModifiedListenerCount > 0 && m_pAntiImpl != nullptr;
    }

    SotElementList_Impl                         m_aChildrenList;
    SotElementList_Impl                         m_aDeletedList;

    css::uno::Reference< css::container::XNameContainer > m_xPackageFolder;
    css::uno::Reference< css::logging::XSimpleLogRing >  m_xLogRing;

    css::uno::Reference< css::lang::XSingleServiceFactory > m_xPackage;
    css::uno::Reference< css::uno::XComponentContext >  m_xContext;

    // valid only for root storage
    css::uno::Reference< css::io::XInputStream > m_xInputStream; // ??? may be stored in properties
    css::uno::Reference< css::io::XStream > m_xStream; // ??? may be stored in properties
    css::uno::Sequence< css::beans::PropertyValue > m_xProperties;
    bool m_bHasCommonEncryptionData;
    ::comphelper::SequenceAsHashMap m_aCommonEncryptionData;

    // must be empty in case of root storage
    OStorage_Impl* m_pParent;

    bool        m_bControlMediaType;
    OUString m_aMediaType;
    bool        m_bMTFallbackUsed;

    bool        m_bControlVersion;
    OUString m_aVersion;

    SwitchablePersistenceStream* m_pSwitchStream;

    sal_Int32 m_nStorageType; // the mode in which the storage is used

    // the _rels substorage that is handled in a special way in embed::StorageFormats::OFOPXML
    SotElement_Impl* m_pRelStorElement;
    css::uno::Reference< css::embed::XStorage > m_xRelStorage;
    css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > m_aRelInfo;
    css::uno::Reference< css::io::XInputStream > m_xNewRelInfoStream;
    sal_Int16 m_nRelInfoStatus;

    // Constructors
    OStorage_Impl(  css::uno::Reference< css::io::XInputStream > xInputStream,
                    sal_Int32 nMode,
                    const css::uno::Sequence< css::beans::PropertyValue >& xProperties,
                    css::uno::Reference< css::uno::XComponentContext > xContext,
                    sal_Int32 nStorageType );

    OStorage_Impl(  css::uno::Reference< css::io::XStream > xStream,
                    sal_Int32 nMode,
                    const css::uno::Sequence< css::beans::PropertyValue >& xProperties,
                    css::uno::Reference< css::uno::XComponentContext > xContext,
                    sal_Int32 nStorageType );

    // constructor for a substorage
    OStorage_Impl(  OStorage_Impl* pParent,
                    sal_Int32 nMode,
                    css::uno::Reference< css::container::XNameContainer > xPackageFolder,
                    css::uno::Reference< css::lang::XSingleServiceFactory > xPackage,
                    css::uno::Reference< css::uno::XComponentContext > xContext,
                    sal_Int32 nStorageType );

    ~OStorage_Impl();

    void AddLog( const OUString& aMessage );

    void SetReadOnlyWrap( OStorage& aStorage );
    void RemoveReadOnlyWrap( OStorage& aStorage );

    void OpenOwnPackage();
    void ReadContents();
    void ReadRelInfoIfNecessary();

    css::uno::Reference< css::uno::XComponentContext > GetComponentContext();
    SotElementList_Impl& GetChildrenList();
    void GetStorageProperties();

    css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > GetAllRelationshipsIfAny();
    void CopyLastCommitTo( const css::uno::Reference< css::embed::XStorage >& xNewStor );

    void InsertIntoPackageFolder(
            const OUString& aName,
            const css::uno::Reference< css::container::XNameContainer >& xParentPackageFolder );

    void Commit();
    void Revert();

    ::comphelper::SequenceAsHashMap GetCommonRootEncryptionData() throw ( css::packages::NoEncryptionException );

    void CopyToStorage( const css::uno::Reference< css::embed::XStorage >& xDest,
                        bool bDirect );
    void CopyStorageElement( SotElement_Impl* pElement,
                            css::uno::Reference< css::embed::XStorage > xDest,
                            const OUString& aName,
                            bool bDirect );

    SotElement_Impl* FindElement( const OUString& rName );

    SotElement_Impl* InsertStream( const OUString& aName, bool bEncr );
    SotElement_Impl* InsertRawStream( const OUString& aName, const css::uno::Reference< css::io::XInputStream >& xInStream );

    OStorage_Impl* CreateNewStorageImpl( sal_Int32 nStorageMode );
    SotElement_Impl* InsertStorage( const OUString& aName, sal_Int32 nStorageMode );
    SotElement_Impl* InsertElement( const OUString& aName, bool bIsStorage );

    void OpenSubStorage( SotElement_Impl* pElement, sal_Int32 nStorageMode );
    void OpenSubStream( SotElement_Impl* pElement );

    css::uno::Sequence< OUString > GetElementNames();

    void RemoveElement( SotElement_Impl* pElement );
    static void ClearElement( SotElement_Impl* pElement );

    void CloneStreamElement(
                    const OUString& aStreamName,
                    bool bPassProvided,
                    const ::comphelper::SequenceAsHashMap& aEncryptionData,
                    css::uno::Reference< css::io::XStream >& xTargetStream )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::WrongPasswordException,
                css::packages::NoEncryptionException,
                css::container::NoSuchElementException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException );

    void RemoveStreamRelInfo( const OUString& aOriginalName );
    void CreateRelStorage();
    void CommitStreamRelInfo( SotElement_Impl* pStreamElement );
    css::uno::Reference< css::io::XInputStream > GetRelInfoStreamForName( const OUString& aName );
    void CommitRelInfo( const css::uno::Reference< css::container::XNameContainer >& xNewPackageFolder );

    static void completeStorageStreamCopy_Impl(
        const css::uno::Reference< css::io::XStream >& xSource,
        const css::uno::Reference< css::io::XStream >& xDest,
        sal_Int32 nStorageType,
        const css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > >& aRelInfo );

};

class OStorage  : public css::lang::XTypeProvider
                , public css::embed::XStorage2
                , public css::embed::XStorageRawAccess
                , public css::embed::XTransactedObject
                , public css::embed::XTransactionBroadcaster
                , public css::util::XModifiable
                , public css::embed::XEncryptionProtectedStorage
                , public css::beans::XPropertySet
                , public css::embed::XOptimizedStorage
                , public css::embed::XRelationshipAccess
                , public css::embed::XHierarchicalStorageAccess2
                , public ::cppu::OWeakObject
{
    OStorage_Impl*  m_pImpl;
    std::unique_ptr<StorInternalData_Impl> m_pData;

protected:

    SotElement_Impl* OpenStreamElement_Impl( const OUString& aStreamName, sal_Int32 nOpenMode, bool bEncr );

    void BroadcastModifiedIfNecessary();

    void BroadcastTransaction( sal_Int8 nMessage );

    void MakeLinkToSubComponent_Impl(
                    const css::uno::Reference< css::lang::XComponent >& xComponent );

public:

    OStorage(   css::uno::Reference< css::io::XInputStream > xInputStream,
                sal_Int32 nMode,
                const css::uno::Sequence< css::beans::PropertyValue >& xProperties,
                css::uno::Reference< css::uno::XComponentContext > xContext,
                sal_Int32 nStorageType );

    OStorage(   css::uno::Reference< css::io::XStream > xStream,
                sal_Int32 nMode,
                const css::uno::Sequence< css::beans::PropertyValue >& xProperties,
                css::uno::Reference< css::uno::XComponentContext > xContext,
                sal_Int32 nStorageType );

    OStorage(   OStorage_Impl* pImpl, bool bReadOnlyWrap );

    virtual ~OStorage();

    void SAL_CALL InternalDispose( bool bNotifyImpl );

    void ChildIsDisposed( const css::uno::Reference< css::uno::XInterface >& xChild );

    sal_Int32 GetRefCount_Impl() { return m_refCount; }

    //  XInterface

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType )
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL acquire() throw() override;

    virtual void SAL_CALL release() throw() override;

    //  XTypeProvider

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;

    //  XStorage

    virtual void SAL_CALL copyToStorage( const css::uno::Reference< css::embed::XStorage >& xDest )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::io::XStream > SAL_CALL openStreamElement(
            const OUString& aStreamName, sal_Int32 nOpenMode )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::io::XStream > SAL_CALL openEncryptedStreamElement(
            const OUString& aStreamName, sal_Int32 nOpenMode, const OUString& aPass )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoEncryptionException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::embed::XStorage > SAL_CALL openStorageElement(
            const OUString& aStorName, sal_Int32 nStorageMode )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::io::XStream > SAL_CALL cloneStreamElement(
            const OUString& aStreamName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::io::XStream > SAL_CALL cloneEncryptedStreamElement(
            const OUString& aStreamName, const OUString& aPass )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoEncryptionException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL copyLastCommitTo(
            const css::uno::Reference< css::embed::XStorage >& xTargetStorage )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL copyStorageElementLastCommitTo(
            const OUString& aStorName,
            const css::uno::Reference< css::embed::XStorage >& xTargetStorage )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL isStreamElement( const OUString& aElementName )
        throw ( css::container::NoSuchElementException,
                css::lang::IllegalArgumentException,
                css::embed::InvalidStorageException,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL isStorageElement( const OUString& aElementName )
        throw ( css::container::NoSuchElementException,
                css::lang::IllegalArgumentException,
                css::embed::InvalidStorageException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removeElement( const OUString& aElementName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL renameElement( const OUString& rEleName, const OUString& rNewName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::container::ElementExistException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL copyElementTo(    const OUString& aElementName,
                                        const css::uno::Reference< css::embed::XStorage >& xDest,
                                        const OUString& aNewName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::container::ElementExistException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL moveElementTo(    const OUString& aElementName,
                                        const css::uno::Reference< css::embed::XStorage >& xDest,
                                        const OUString& rNewName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::container::ElementExistException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    //  XStorage2

    virtual css::uno::Reference< css::io::XStream > SAL_CALL openEncryptedStream( const OUString& sStreamName, ::sal_Int32 nOpenMode, const css::uno::Sequence< css::beans::NamedValue >& aEncryptionData )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoEncryptionException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::io::XStream > SAL_CALL cloneEncryptedStream( const OUString& sStreamName, const css::uno::Sequence< css::beans::NamedValue >& aEncryptionData )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoEncryptionException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    //  XStorageRawAccess

    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getPlainRawStreamElement(
            const OUString& sStreamName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getRawEncrStreamElement(
            const OUString& sStreamName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoEncryptionException,
                css::container::NoSuchElementException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL insertRawEncrStreamElement( const OUString& aStreamName,
                                const css::uno::Reference< css::io::XInputStream >& xInStream )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoRawFormatException,
                css::container::ElementExistException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

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

    //  XModifiable

    virtual sal_Bool SAL_CALL isModified()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setModified( sal_Bool bModified )
        throw ( css::beans::PropertyVetoException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL addModifyListener(
            const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removeModifyListener(
            const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    //  XNameAccess

    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
        throw ( css::container::NoSuchElementException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Type SAL_CALL getElementType()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL hasElements()
        throw ( css::uno::RuntimeException, std::exception ) override;

    //  XComponent

    virtual void SAL_CALL dispose()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL addEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removeEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    //  XEncryptionProtectedSource

    virtual void SAL_CALL setEncryptionPassword( const OUString& aPass )
        throw ( css::uno::RuntimeException,
                css::io::IOException, std::exception ) override;

    virtual void SAL_CALL removeEncryption()
        throw ( css::uno::RuntimeException,
                css::io::IOException, std::exception ) override;

    //  XEncryptionProtectedSource2

    virtual void SAL_CALL setEncryptionData(
            const css::uno::Sequence< css::beans::NamedValue >& aEncryptionData )
        throw ( css::io::IOException,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL hasEncryptionData()
        throw ( css::uno::RuntimeException, std::exception ) override;

    //  XEncryptionProtectedStorage

    virtual void SAL_CALL setEncryptionAlgorithms( const css::uno::Sequence< css::beans::NamedValue >& aAlgorithms ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< css::beans::NamedValue > SAL_CALL getEncryptionAlgorithms() throw (css::uno::RuntimeException, std::exception) override;

    //  XPropertySet

    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue )
        throw ( css::beans::UnknownPropertyException,
                css::beans::PropertyVetoException,
                css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw ( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
        throw ( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
        throw ( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& PropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
        throw ( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
        throw ( css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    //  XOptimizedStorage
    virtual void SAL_CALL insertRawNonEncrStreamElementDirect( const OUString& sStreamName, const css::uno::Reference< css::io::XInputStream >& xInStream )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoRawFormatException,
                css::container::ElementExistException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL insertStreamElementDirect( const OUString& sStreamName, const css::uno::Reference< css::io::XInputStream >& xInStream, const css::uno::Sequence< css::beans::PropertyValue >& aProps )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::ElementExistException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL copyElementDirectlyTo( const OUString& sSourceName, const css::uno::Reference< css::embed::XOptimizedStorage >& xTargetStorage, const OUString& sTargetName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::container::ElementExistException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL writeAndAttachToStream( const css::uno::Reference< css::io::XStream >& xStream )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL attachToURL( const OUString& sURL, sal_Bool bReadOnly )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Any SAL_CALL getElementPropertyValue( const OUString& sElementName, const OUString& sPropertyName )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::io::IOException,
                css::beans::UnknownPropertyException,
                css::beans::PropertyVetoException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL copyStreamElementData( const OUString& sStreamName, const css::uno::Reference< css::io::XStream >& xTargetStream )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    // XRelationshipAccess
    virtual sal_Bool SAL_CALL hasByID( const OUString& sID )
        throw ( css::io::IOException,
                css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getTargetByID( const OUString& sID )
        throw ( css::container::NoSuchElementException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getTypeByID( const OUString& sID )
        throw ( css::container::NoSuchElementException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< css::beans::StringPair > SAL_CALL getRelationshipByID( const OUString& sID )
        throw ( css::container::NoSuchElementException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > SAL_CALL getRelationshipsByType( const OUString& sType )
        throw ( css::io::IOException,
                css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > > SAL_CALL getAllRelationships(  )
        throw ( css::io::IOException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL insertRelationshipByID( const OUString& sID, const css::uno::Sequence< css::beans::StringPair >& aEntry, sal_Bool bReplace )
        throw ( css::container::ElementExistException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeRelationshipByID( const OUString& sID )
        throw ( css::container::NoSuchElementException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL insertRelationships( const css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > >& aEntries, sal_Bool bReplace )
        throw ( css::container::ElementExistException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL clearRelationships(  )
        throw ( css::io::IOException,
                css::uno::RuntimeException, std::exception) override;

    // XHierarchicalStorageAccess
    virtual css::uno::Reference< css::embed::XExtendedStorageStream > SAL_CALL openStreamElementByHierarchicalName( const OUString& sStreamPath, ::sal_Int32 nOpenMode )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::embed::XExtendedStorageStream > SAL_CALL openEncryptedStreamElementByHierarchicalName( const OUString& sStreamName, ::sal_Int32 nOpenMode, const OUString& sPassword )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoEncryptionException,
                css::packages::WrongPasswordException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeStreamElementByHierarchicalName( const OUString& sElementPath )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    // XHierarchicalStorageAccess2
    virtual css::uno::Reference< css::embed::XExtendedStorageStream > SAL_CALL openEncryptedStreamByHierarchicalName( const OUString& sStreamName, ::sal_Int32 nOpenMode, const css::uno::Sequence< css::beans::NamedValue >& aEncryptionData )
        throw ( css::embed::InvalidStorageException,
                css::lang::IllegalArgumentException,
                css::packages::NoEncryptionException,
                css::packages::WrongPasswordException,
                css::container::NoSuchElementException,
                css::io::IOException,
                css::embed::StorageWrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;
};

StorageHolder_Impl::StorageHolder_Impl( OStorage* pStorage )
: m_pPointer( pStorage )
, m_xWeakRef( css::uno::Reference< css::embed::XStorage >( pStorage ) )
{
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
