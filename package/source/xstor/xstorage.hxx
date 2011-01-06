/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __XSTORAGE_HXX_
#define __XSTORAGE_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/embed/XStorage2.hpp>
#include <com/sun/star/embed/XOptimizedStorage.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess2.hpp>
#include <com/sun/star/embed/XStorageRawAccess.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/XEncryptionProtectedSource2.hpp>
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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/packages/NoEncryptionException.hpp>
#include <com/sun/star/logging/XSimpleLogRing.hpp>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/sequenceashashmap.hxx>

#include "mutexholder.hxx"

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

namespace cppu
{
    class OTypeCollection;
}

//================================================
// a common implementation for an entry

struct StorInternalData_Impl;
struct OStorage_Impl;
struct OWriteStream_Impl;

struct SotElement_Impl
{
    ::rtl::OUString             m_aName;
    ::rtl::OUString             m_aOriginalName;
    sal_Bool                    m_bIsRemoved;
    sal_Bool                    m_bIsInserted;
    sal_Bool                    m_bIsStorage;

    OStorage_Impl*              m_pStorage;
    OWriteStream_Impl*          m_pStream;

public:
                                SotElement_Impl( const ::rtl::OUString& rName, sal_Bool bStor, sal_Bool bNew );
                                ~SotElement_Impl();
};

#include <list>
typedef ::std::list< SotElement_Impl* > SotElementList_Impl;

//=========================================================================
// Main storage implementation

class OStorage;

struct StorageHolder_Impl
{
    OStorage* m_pPointer;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::embed::XStorage > m_xWeakRef;

    StorageHolder_Impl( OStorage* pStorage )
    : m_pPointer( pStorage )
    , m_xWeakRef( ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >(
                                                (::com::sun::star::embed::XStorage*)pStorage ) )
    {
    }

    StorageHolder_Impl( const StorageHolder_Impl& aSH )
    : m_pPointer( aSH.m_pPointer )
    , m_xWeakRef( aSH.m_xWeakRef )
    {
    }
};

typedef ::std::list< StorageHolder_Impl > OStorageList_Impl;

class SwitchablePersistenceStream;
struct OStorage_Impl
{
    SotMutexHolderRef           m_rMutexRef;

    OStorage*                   m_pAntiImpl;         // only valid if external references exists
    OStorageList_Impl           m_aReadOnlyWrapList; // only valid if readonly external reference exists

    sal_Int32                   m_nStorageMode; // open mode ( read/write/trunc/nocreate )
    sal_Bool                    m_bIsModified;  // only modified elements will be sent to the original content
    sal_Bool                    m_bBroadcastModified;  // will be set if notification is required
    sal_Bool                    m_bCommited;    // sending the streams is coordinated by the root storage of the package

    sal_Bool                    m_bIsRoot;      // marks this storage as root storages that manages all commits and reverts
    sal_Bool                    m_bListCreated;


    SotElementList_Impl                         m_aChildrenList;
    SotElementList_Impl                         m_aDeletedList;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > m_xPackageFolder;
    ::com::sun::star::uno::Reference< ::com::sun::star::logging::XSimpleLogRing >  m_xLogRing;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > m_xPackage;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  m_xFactory;

    // valid only for root storage
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xInputStream; // ??? may be stored in properties
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xStream; // ??? may be stored in properties
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_xProperties;
    sal_Bool m_bHasCommonEncryptionData;
    ::comphelper::SequenceAsHashMap m_aCommonEncryptionData;

    // must be empty in case of root storage
    OStorage_Impl* m_pParent;

    sal_Bool        m_bControlMediaType;
    ::rtl::OUString m_aMediaType;
    sal_Bool        m_bMTFallbackUsed;

    sal_Bool        m_bControlVersion;
    ::rtl::OUString m_aVersion;

    SwitchablePersistenceStream* m_pSwitchStream;

    sal_Int32 m_nStorageType; // the mode in wich the storage is used

    // the _rels substorage that is handled in a special way in embed::StorageFormats::OFOPXML
    SotElement_Impl* m_pRelStorElement;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xRelStorage;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > m_aRelInfo;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xNewRelInfoStream;
    sal_Int16 m_nRelInfoStatus;

    //////////////////////////////////////////
    // Constructors

    OStorage_Impl(  ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInputStream,
                    sal_Int32 nMode,
                    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > xProperties,
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory,
                    sal_Int32 nStorageType );

    OStorage_Impl(  ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > xStream,
                    sal_Int32 nMode,
                    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > xProperties,
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory,
                    sal_Int32 nStorageType );

    // constructor for a substorage
    OStorage_Impl(  OStorage_Impl* pParent,
                    sal_Int32 nMode,
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > xPackageFolder,
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > xPackage,
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory,
                    sal_Int32 nStorageType );

    ~OStorage_Impl();

    void AddLog( const ::rtl::OUString& aMessage );

    void SetReadOnlyWrap( OStorage& aStorage );
    void RemoveReadOnlyWrap( OStorage& aStorage );

    void OpenOwnPackage();
    void ReadContents();
    void ReadRelInfoIfNecessary();

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > GetServiceFactory();
    SotElementList_Impl& GetChildrenList();
    void GetStorageProperties();

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > GetAllRelationshipsIfAny();
    void CopyLastCommitTo( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xNewStor );
    void CopyLastCommitTo( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xNewStor,
                            const ::rtl::OUString& aPass );

    void InsertIntoPackageFolder(
            const ::rtl::OUString& aName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xParentPackageFolder );

    void Commit();
    void Revert();

    ::comphelper::SequenceAsHashMap GetCommonRootEncryptionData() throw ( ::com::sun::star::packages::NoEncryptionException );

    void CopyToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                        sal_Bool bDirect );
    void CopyStorageElement( SotElement_Impl* pElement,
                            ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > xDest,
                            ::rtl::OUString aName,
                            sal_Bool bDirect );

    void SetModified( sal_Bool bModified );

    SotElement_Impl* FindElement( const ::rtl::OUString& rName );


    SotElement_Impl* InsertStream( ::rtl::OUString aName, sal_Bool bEncr );
    SotElement_Impl* InsertRawStream( ::rtl::OUString aName, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream );

    OStorage_Impl* CreateNewStorageImpl( sal_Int32 nStorageMode );
    SotElement_Impl* InsertStorage( ::rtl::OUString aName, sal_Int32 nStorageMode );
    SotElement_Impl* InsertElement( ::rtl::OUString aName, sal_Bool bIsStorage );

    void OpenSubStorage( SotElement_Impl* pElement, sal_Int32 nStorageMode );
    void OpenSubStream( SotElement_Impl* pElement );

    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetElementNames();

    void RemoveElement( SotElement_Impl* pElement );
    void ClearElement( SotElement_Impl* pElement );
    void DisposeChildren();

    void CloneStreamElement(
                    const ::rtl::OUString& aStreamName,
                    sal_Bool bPassProvided,
                    const ::comphelper::SequenceAsHashMap& aEncryptionData,
                    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xTargetStream )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    void RemoveStreamRelInfo( const ::rtl::OUString& aOriginalName );
    void CreateRelStorage();
    void CommitStreamRelInfo( SotElement_Impl* pStreamElement );
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetRelInfoStreamForName( const ::rtl::OUString& aName );
    void CommitRelInfo( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xNewPackageFolder );

    static void completeStorageStreamCopy_Impl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xSource,
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xDest,
        sal_Int32 nStorageType,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > >& aRelInfo );

};


class OStorage  : public ::com::sun::star::lang::XTypeProvider
                , public ::com::sun::star::embed::XStorage2
                , public ::com::sun::star::embed::XStorageRawAccess
                , public ::com::sun::star::embed::XTransactedObject
                , public ::com::sun::star::embed::XTransactionBroadcaster
                , public ::com::sun::star::util::XModifiable
                // , public ::com::sun::star::container::XNameAccess
                // , public ::com::sun::star::lang::XComponent
                , public ::com::sun::star::embed::XEncryptionProtectedSource2
                , public ::com::sun::star::beans::XPropertySet
                , public ::com::sun::star::embed::XOptimizedStorage
                , public ::com::sun::star::embed::XRelationshipAccess
                , public ::com::sun::star::embed::XHierarchicalStorageAccess2
                , public ::cppu::OWeakObject
{
    OStorage_Impl*  m_pImpl;
    StorInternalData_Impl* m_pData;

protected:

    void Commit_Impl();

    SotElement_Impl* OpenStreamElement_Impl( const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode, sal_Bool bEncr );

    void BroadcastModifiedIfNecessary();

    void BroadcastTransaction( sal_Int8 nMessage );

    void MakeLinkToSubComponent_Impl(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xComponent );

public:

    OStorage(   ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInputStream,
                sal_Int32 nMode,
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > xProperties,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory,
                sal_Int32 nStorageType );

    OStorage(   ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > xStream,
                sal_Int32 nMode,
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > xProperties,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory,
                sal_Int32 nStorageType );

    OStorage(   OStorage_Impl* pImpl, sal_Bool bReadOnlyWrap );

    virtual ~OStorage();

    void SAL_CALL InternalDispose( sal_Bool bNotifyImpl );

    void ChildIsDisposed( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xChild );

    sal_Int32 GetRefCount_Impl() { return m_refCount; }

    //____________________________________________________________________________________________________
    //  XInterface
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL acquire() throw();

    virtual void SAL_CALL release() throw();

    //____________________________________________________________________________________________________
    //  XTypeProvider
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XStorage
    //____________________________________________________________________________________________________

    virtual void SAL_CALL copyToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL openStreamElement(
            const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL openEncryptedStreamElement(
            const ::rtl::OUString& aStreamName, sal_Int32 nOpenMode, const ::rtl::OUString& aPass )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL openStorageElement(
            const ::rtl::OUString& aStorName, sal_Int32 nStorageMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL cloneStreamElement(
            const ::rtl::OUString& aStreamName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL cloneEncryptedStreamElement(
            const ::rtl::OUString& aStreamName, const ::rtl::OUString& aPass )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL copyLastCommitTo(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xTargetStorage )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL copyStorageElementLastCommitTo(
            const ::rtl::OUString& aStorName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xTargetStorage )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isStreamElement( const ::rtl::OUString& aElementName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isStorageElement( const ::rtl::OUString& aElementName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeElement( const ::rtl::OUString& aElementName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL renameElement( const ::rtl::OUString& rEleName, const ::rtl::OUString& rNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL copyElementTo(    const ::rtl::OUString& aElementName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                                        const ::rtl::OUString& aNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL moveElementTo(    const ::rtl::OUString& aElementName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xDest,
                                        const ::rtl::OUString& rNewName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XStorage2
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL openEncryptedStream( const ::rtl::OUString& sStreamName, ::sal_Int32 nOpenMode, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aEncryptionData )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL cloneEncryptedStream( const ::rtl::OUString& sStreamName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aEncryptionData )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XStorageRawAccess
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getPlainRawStreamElement(
            const ::rtl::OUString& sStreamName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getRawEncrStreamElement(
            const ::rtl::OUString& sStreamName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL insertRawEncrStreamElement( const ::rtl::OUString& aStreamName,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoRawFormatException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    // XTransactedObject
    //____________________________________________________________________________________________________

    virtual void SAL_CALL commit()
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL revert()
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    // XTransactionBroadcaster
    //____________________________________________________________________________________________________

    virtual void SAL_CALL addTransactionListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XTransactionListener >& aListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeTransactionListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XTransactionListener >& aListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XModifiable
    //____________________________________________________________________________________________________

    virtual sal_Bool SAL_CALL isModified()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setModified( sal_Bool bModified )
        throw ( ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addModifyListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeModifyListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XNameAccess
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL hasElements()
        throw ( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XComponent
    //____________________________________________________________________________________________________

    virtual void SAL_CALL dispose()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XEncryptionProtectedSource
    //____________________________________________________________________________________________________

    virtual void SAL_CALL setEncryptionPassword( const ::rtl::OUString& aPass )
        throw ( ::com::sun::star::uno::RuntimeException,
                ::com::sun::star::io::IOException );

    virtual void SAL_CALL removeEncryption()
        throw ( ::com::sun::star::uno::RuntimeException,
                ::com::sun::star::io::IOException );

    //____________________________________________________________________________________________________
    //  XEncryptionProtectedSource2
    //____________________________________________________________________________________________________

    virtual void SAL_CALL setEncryptionData(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aEncryptionData )
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException );


    //____________________________________________________________________________________________________
    //  XPropertySet
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addPropertyChangeListener(
            const ::rtl::OUString& aPropertyName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removePropertyChangeListener(
            const ::rtl::OUString& aPropertyName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addVetoableChangeListener(
            const ::rtl::OUString& PropertyName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw ( ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XOptimizedStorage
    //____________________________________________________________________________________________________
    virtual void SAL_CALL insertRawNonEncrStreamElementDirect( const ::rtl::OUString& sStreamName, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoRawFormatException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL insertStreamElementDirect( const ::rtl::OUString& sStreamName, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL copyElementDirectlyTo( const ::rtl::OUString& sSourceName, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XOptimizedStorage >& xTargetStorage, const ::rtl::OUString& sTargetName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL writeAndAttachToStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xStream )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL attachToURL( const ::rtl::OUString& sURL, sal_Bool bReadOnly )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Any SAL_CALL getElementPropertyValue( const ::rtl::OUString& sElementName, const ::rtl::OUString& sPropertyName )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL copyStreamElementData( const ::rtl::OUString& sStreamName, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xTargetStream )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    // XRelationshipAccess
    //____________________________________________________________________________________________________

    virtual ::sal_Bool SAL_CALL hasByID( const ::rtl::OUString& sID )
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getTargetByID( const ::rtl::OUString& sID )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getTypeByID( const ::rtl::OUString& sID )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > SAL_CALL getRelationshipByID( const ::rtl::OUString& sID )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > SAL_CALL getRelationshipsByType( const ::rtl::OUString& sType )
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > > SAL_CALL getAllRelationships(  )
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL insertRelationshipByID( const ::rtl::OUString& sID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aEntry, ::sal_Bool bReplace )
        throw ( ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeRelationshipByID( const ::rtl::OUString& sID )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL insertRelationships( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair > >& aEntries, ::sal_Bool bReplace )
        throw ( ::com::sun::star::container::ElementExistException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL clearRelationships(  )
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    // XHierarchicalStorageAccess
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream > SAL_CALL openStreamElementByHierarchicalName( const ::rtl::OUString& sStreamPath, ::sal_Int32 nOpenMode )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream > SAL_CALL openEncryptedStreamElementByHierarchicalName( const ::rtl::OUString& sStreamName, ::sal_Int32 nOpenMode, const ::rtl::OUString& sPassword )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeStreamElementByHierarchicalName( const ::rtl::OUString& sElementPath )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    // XHierarchicalStorageAccess2
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XExtendedStorageStream > SAL_CALL openEncryptedStreamByHierarchicalName( const ::rtl::OUString& sStreamName, ::sal_Int32 nOpenMode, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aEncryptionData )
        throw ( ::com::sun::star::embed::InvalidStorageException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::embed::StorageWrappedTargetException,
                ::com::sun::star::uno::RuntimeException );
};


#endif

