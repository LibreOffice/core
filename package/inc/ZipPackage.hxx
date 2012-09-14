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
#ifndef _ZIP_PACKAGE_HXX
#define _ZIP_PACKAGE_HXX

#include <cppuhelper/implbase7.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <HashMaps.hxx>
#include <osl/file.h>
#include <mutexholder.hxx>
#include <vector>

class ZipOutputStream;
class ZipPackageFolder;
class ZipFile;
namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace io { class XStream; class XOutputStream; class XInputStream; class XSeekable; class XActiveDataStreamer; }
    namespace lang { class XMultiServiceFactory; }
    namespace task { class XInteractionHandler; }
} } }
enum SegmentEnum
{
    e_Aborted = -1000,
    e_Retry,
    e_Finished,
    e_Success = 0
};

enum InitialisationMode
{
    e_IMode_None,
    e_IMode_URL,
    e_IMode_XInputStream,
    e_IMode_XStream
};

class ZipPackage : public cppu::WeakImplHelper7
                    <
                       com::sun::star::lang::XInitialization,
                       com::sun::star::lang::XSingleServiceFactory,
                       com::sun::star::lang::XUnoTunnel,
                       com::sun::star::lang::XServiceInfo,
                       com::sun::star::container::XHierarchicalNameAccess,
                       com::sun::star::util::XChangesBatch,
                       com::sun::star::beans::XPropertySet
                    >
{
protected:
    SotMutexHolderRef m_aMutexHolder;

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > m_aStorageEncryptionKeys;
    ::com::sun::star::uno::Sequence< sal_Int8 > m_aEncryptionKey;

    FolderHash        m_aRecent;
    ::rtl::OUString   m_aURL;

    sal_Int32         m_nStartKeyGenerationID;
    sal_Int32         m_nChecksumDigestID;
    sal_Int32         m_nCommonEncryptionID;
    sal_Bool          m_bHasEncryptedEntries;
    sal_Bool          m_bHasNonEncryptedEntries;

    sal_Bool          m_bInconsistent;
    sal_Bool          m_bForceRecovery;

    sal_Bool          m_bMediaTypeFallbackUsed;
    sal_Int32         m_nFormat;
    sal_Bool          m_bAllowRemoveOnInsert;

    InitialisationMode m_eMode;

    ::com::sun::star::uno::Reference < com::sun::star::container::XNameContainer > m_xRootFolder;
    ::com::sun::star::uno::Reference < com::sun::star::io::XStream > m_xStream;
    ::com::sun::star::uno::Reference < com::sun::star::io::XInputStream > m_xContentStream;
    ::com::sun::star::uno::Reference < com::sun::star::io::XSeekable > m_xContentSeek;
    const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    ZipPackageFolder *m_pRootFolder;
    ZipFile          *m_pZipFile;

    sal_Bool isLocalFile() const;

    void parseManifest();
    void parseContentType();
    void getZipFileContents();

    void WriteMimetypeMagicFile( ZipOutputStream& aZipOut );
    void WriteManifest( ZipOutputStream& aZipOut, const ::std::vector< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& aManList );
    void WriteContentTypes( ZipOutputStream& aZipOut, const ::std::vector< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& aManList );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > writeTempFile();
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XActiveDataStreamer > openOriginalForOutput();
    void DisconnectFromTargetAndThrowException_Impl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xTempStream );

public:
    ZipPackage( const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > &xNewFactory );
    virtual ~ZipPackage( void );
    ZipFile& getZipFile() { return *m_pZipFile;}
    sal_Int32 getFormat() const { return m_nFormat; }

    sal_Int32 GetStartKeyGenID() const { return m_nStartKeyGenerationID; }
    sal_Int32 GetEncAlgID() const { return m_nCommonEncryptionID; }
    sal_Int32 GetChecksumAlgID() const { return m_nChecksumDigestID; }
    sal_Int32 GetDefaultDerivedKeySize() const { return m_nCommonEncryptionID == ::com::sun::star::xml::crypto::CipherID::AES_CBC_W3C_PADDING ? 32 : 16; }

    SotMutexHolderRef GetSharedMutexRef() { return m_aMutexHolder; }

    void ConnectTo( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream );
    const ::com::sun::star::uno::Sequence< sal_Int8 > GetEncryptionKey();

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    // XHierarchicalNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByHierarchicalName( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);
    // XSingleServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance(  )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    // XChangesBatch
    virtual void SAL_CALL commitChanges(  )
        throw(::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasPendingChanges(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::util::ElementChange > SAL_CALL getPendingChanges(  )
        throw(::com::sun::star::uno::RuntimeException);
    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
        throw(::com::sun::star::uno::RuntimeException);
    com::sun::star::uno::Sequence < sal_Int8 > getUnoTunnelImplementationId( void )
        throw(::com::sun::star::uno::RuntimeException);
    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException);

    // Uno componentiseralation
    static ::rtl::OUString static_getImplementationName();
    static ::com::sun::star::uno::Sequence < ::rtl::OUString > static_getSupportedServiceNames();
    static ::com::sun::star::uno::Reference < com::sun::star::lang::XSingleServiceFactory > createServiceFactory( com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > const & rServiceFactory );
    sal_Bool SAL_CALL static_supportsService(rtl::OUString const & rServiceName);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
