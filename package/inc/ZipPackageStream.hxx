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
#ifndef _ZIP_PACKAGE_STREAM_HXX
#define _ZIP_PACKAGE_STREAM_HXX

#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/packages/XDataSinkEncrSupport.hpp>
#include <ZipPackageEntry.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/implbase2.hxx>

#include <EncryptionData.hxx>
#include <mutexholder.hxx>

#define PACKAGE_STREAM_NOTSET           0
#define PACKAGE_STREAM_PACKAGEMEMBER    1
#define PACKAGE_STREAM_DETECT           2
#define PACKAGE_STREAM_DATA             3
#define PACKAGE_STREAM_RAW              4

class ZipPackage;
struct ZipEntry;
class ZipPackageStream : public cppu::ImplInheritanceHelper2
<
    ZipPackageEntry,
    ::com::sun::star::io::XActiveDataSink,
    ::com::sun::star::packages::XDataSinkEncrSupport
>
{
protected:
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xStream;
    const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > m_xFactory;
    ZipPackage          &rZipPackage;
    sal_Bool            bToBeCompressed, bToBeEncrypted, bHaveOwnKey, bIsEncrypted;

    ::rtl::Reference< BaseEncryptionData > m_xBaseEncryptionData;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > m_aStorageEncryptionKeys;
    ::com::sun::star::uno::Sequence< sal_Int8 > m_aEncryptionKey;

    sal_Int32 m_nImportedStartKeyAlgorithm;
    sal_Int32 m_nImportedEncryptionAlgorithm;
    sal_Int32 m_nImportedChecksumAlgorithm;
    sal_Int32 m_nImportedDerivedKeySize;

    sal_uInt8   m_nStreamMode;
    sal_uInt32  m_nMagicalHackPos;
    sal_uInt32  m_nMagicalHackSize;

    sal_Bool m_bHasSeekable;

    sal_Bool m_bCompressedIsSetFromOutside;

    sal_Bool m_bFromManifest;

    bool m_bUseWinEncoding;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetOwnSeekStream();

public:
    sal_Bool HasOwnKey () const  { return bHaveOwnKey;}
    sal_Bool IsToBeCompressed () const { return bToBeCompressed;}
    sal_Bool IsToBeEncrypted () const { return bToBeEncrypted;}
    sal_Bool IsEncrypted () const    { return bIsEncrypted;}
    sal_Bool IsPackageMember () const { return m_nStreamMode == PACKAGE_STREAM_PACKAGEMEMBER;}

    sal_Bool IsFromManifest() const { return m_bFromManifest; }
    void SetFromManifest( sal_Bool bValue ) { m_bFromManifest = bValue; }

    ::rtl::Reference< EncryptionData > GetEncryptionData( bool bWinEncoding = false );

    ::com::sun::star::uno::Sequence< sal_Int8 > GetEncryptionKey( bool bWinEncoding = false );

    sal_Int32 GetStartKeyGenID();

    const com::sun::star::uno::Sequence < sal_Int8 > getInitialisationVector () const
    { return m_xBaseEncryptionData->m_aInitVector;}
    const com::sun::star::uno::Sequence < sal_Int8 > getDigest () const
    { return m_xBaseEncryptionData->m_aDigest;}
    const com::sun::star::uno::Sequence < sal_Int8 > getSalt () const
    { return m_xBaseEncryptionData->m_aSalt;}
    sal_Int32 getIterationCount () const
    { return m_xBaseEncryptionData->m_nIterationCount;}
    sal_Int64 getSize () const
    { return aEntry.nSize;}

    sal_uInt8 GetStreamMode() const { return m_nStreamMode; }
    sal_uInt32 GetMagicalHackPos() const { return m_nMagicalHackPos; }
    sal_uInt32 GetMagicalHackSize() const { return m_nMagicalHackSize; }
    sal_Int32 GetEncryptionAlgorithm() const;
    sal_Int32 GetBlockSize() const;

    void SetToBeCompressed (sal_Bool bNewValue) { bToBeCompressed = bNewValue;}
    void SetIsEncrypted (sal_Bool bNewValue) { bIsEncrypted = bNewValue;}
    void SetImportedStartKeyAlgorithm( sal_Int32 nAlgorithm ) { m_nImportedStartKeyAlgorithm = nAlgorithm; }
    void SetImportedEncryptionAlgorithm( sal_Int32 nAlgorithm ) { m_nImportedEncryptionAlgorithm = nAlgorithm; }
    void SetImportedChecksumAlgorithm( sal_Int32 nAlgorithm ) { m_nImportedChecksumAlgorithm = nAlgorithm; }
    void SetImportedDerivedKeySize( sal_Int32 nSize ) { m_nImportedDerivedKeySize = nSize; }
    void SetToBeEncrypted (sal_Bool bNewValue)
    {
        bToBeEncrypted  = bNewValue;
        if ( bToBeEncrypted && !m_xBaseEncryptionData.is())
            m_xBaseEncryptionData = new BaseEncryptionData;
        else if ( !bToBeEncrypted && m_xBaseEncryptionData.is() )
            m_xBaseEncryptionData.clear();
    }
    void SetPackageMember (sal_Bool bNewValue);

    void setKey (const com::sun::star::uno::Sequence < sal_Int8 >& rNewKey )
    { m_aEncryptionKey = rNewKey; m_aStorageEncryptionKeys.realloc( 0 ); }
    void setInitialisationVector (const com::sun::star::uno::Sequence < sal_Int8 >& rNewVector )
    { m_xBaseEncryptionData->m_aInitVector = rNewVector;}
    void setSalt (const com::sun::star::uno::Sequence < sal_Int8 >& rNewSalt )
    { m_xBaseEncryptionData->m_aSalt = rNewSalt;}
    void setDigest (const com::sun::star::uno::Sequence < sal_Int8 >& rNewDigest )
    { m_xBaseEncryptionData->m_aDigest = rNewDigest;}
    void setIterationCount (const sal_Int32 nNewCount)
    { m_xBaseEncryptionData->m_nIterationCount = nNewCount;}
    void setSize (const sal_Int64 nNewSize);

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetOwnStreamNoWrap() { return xStream; }

    void CloseOwnStreamIfAny();

    ZipPackageStream ( ZipPackage & rNewPackage,
                        const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& xFactory,
                        sal_Bool bAllowRemoveOnInsert );
    virtual ~ZipPackageStream( void );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetRawEncrStreamNoHeaderCopy();
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > TryToGetRawFromDataStream(
                                                                                    sal_Bool bAddHeaderForEncr );

    sal_Bool ParsePackageRawStream();

    void setZipEntryOnLoading( const ZipEntry &rInEntry);
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getRawData()
        throw(::com::sun::star::uno::RuntimeException);

    static const ::com::sun::star::uno::Sequence < sal_Int8 >& static_getImplementationId();

    // XActiveDataSink
    virtual void SAL_CALL setInputStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& aStream )
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(  )
        throw(::com::sun::star::uno::RuntimeException);

    // XDataSinkEncrSupport
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getDataStream()
        throw ( ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getRawStream()
        throw ( ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setDataStream(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& aStream )
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setRawStream(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& aStream )
        throw ( ::com::sun::star::packages::EncryptionNotAllowedException,
                ::com::sun::star::packages::NoRawFormatException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getPlainRawStream()
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException );

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
        throw(::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw (::com::sun::star::uno::RuntimeException);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
