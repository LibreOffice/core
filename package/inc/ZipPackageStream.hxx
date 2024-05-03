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
#ifndef INCLUDED_PACKAGE_INC_ZIPPACKAGESTREAM_HXX
#define INCLUDED_PACKAGE_INC_ZIPPACKAGESTREAM_HXX

#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/packages/XDataSinkEncrSupport.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "ZipPackageEntry.hxx"
#include <rtl/ref.hxx>
#include <cppuhelper/implbase.hxx>

#include "EncryptionData.hxx"


#define PACKAGE_STREAM_NOTSET           0
#define PACKAGE_STREAM_PACKAGEMEMBER    1
#define PACKAGE_STREAM_DETECT           2
#define PACKAGE_STREAM_DATA             3
#define PACKAGE_STREAM_RAW              4

class ZipPackage;
struct ZipEntry;
class ZipPackageStream final : public cppu::ImplInheritanceHelper
<
    ZipPackageEntry,
    css::io::XActiveDataSink,
    css::packages::XDataSinkEncrSupport
>
{
private:
    css::uno::Reference < css::io::XInputStream > m_xStream;
    ZipPackage          &m_rZipPackage;
    bool                 m_bToBeCompressed, m_bToBeEncrypted, m_bHaveOwnKey, m_bIsEncrypted;

    ::rtl::Reference< BaseEncryptionData > m_xBaseEncryptionData;
    css::uno::Sequence< css::beans::NamedValue > m_aStorageEncryptionKeys;
    css::uno::Sequence< sal_Int8 > m_aEncryptionKey;

    sal_Int32 m_nImportedStartKeyAlgorithm;
    sal_Int32 m_nImportedEncryptionAlgorithm;
    ::std::optional<sal_Int32> m_oImportedChecksumAlgorithm;
    sal_Int32 m_nImportedDerivedKeySize;

    sal_uInt8   m_nStreamMode;
    sal_uInt32  m_nMagicalHackPos;
    sal_uInt32  m_nMagicalHackSize;
    sal_Int64   m_nOwnStreamOrigSize;

    bool m_bHasSeekable;
    bool m_bCompressedIsSetFromOutside;
    bool m_bFromManifest;
    bool m_bUseWinEncoding;
    bool m_bRawStream;

    /// Check that m_xStream implements io::XSeekable and return it
    css::uno::Reference< css::io::XInputStream > const & GetOwnSeekStream();
    /// get raw data using unbuffered stream
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::io::XInputStream > getRawData();

public:
    bool IsPackageMember () const { return m_nStreamMode == PACKAGE_STREAM_PACKAGEMEMBER;}

    bool IsFromManifest() const { return m_bFromManifest; }
    void SetFromManifest( bool bValue ) { m_bFromManifest = bValue; }

    enum class Bugs { None, WinEncodingWrongSHA1, WrongSHA1 };
    ::rtl::Reference<EncryptionData> GetEncryptionData(Bugs bugs = Bugs::None);

    css::uno::Sequence<sal_Int8> GetEncryptionKey(Bugs bugs = Bugs::None);

    sal_Int32 GetStartKeyGenID() const;

    sal_Int32 GetEncryptionAlgorithm() const;
    sal_Int32 GetIVSize() const;

    void SetToBeCompressed (bool bNewValue) { m_bToBeCompressed = bNewValue;}
    void SetIsEncrypted (bool bNewValue) { m_bIsEncrypted = bNewValue;}
    void SetImportedStartKeyAlgorithm( sal_Int32 nAlgorithm ) { m_nImportedStartKeyAlgorithm = nAlgorithm; }
    void SetImportedEncryptionAlgorithm( sal_Int32 nAlgorithm ) { m_nImportedEncryptionAlgorithm = nAlgorithm; }
    void SetImportedChecksumAlgorithm(::std::optional<sal_Int32> const& roAlgorithm) { m_oImportedChecksumAlgorithm = roAlgorithm; }
    void SetImportedDerivedKeySize( sal_Int32 nSize ) { m_nImportedDerivedKeySize = nSize; }
    void SetToBeEncrypted (bool bNewValue)
    {
        m_bToBeEncrypted  = bNewValue;
        if ( m_bToBeEncrypted && !m_xBaseEncryptionData.is())
            m_xBaseEncryptionData = new BaseEncryptionData;
        else if ( !m_bToBeEncrypted && m_xBaseEncryptionData.is() )
            m_xBaseEncryptionData.clear();
    }
    void SetPackageMember (bool bNewValue);

    void setInitialisationVector (const css::uno::Sequence < sal_Int8 >& rNewVector )
    { m_xBaseEncryptionData->m_aInitVector = rNewVector;}
    void setSalt (const css::uno::Sequence < sal_Int8 >& rNewSalt )
    { m_xBaseEncryptionData->m_aSalt = rNewSalt;}
    void setDigest (const css::uno::Sequence < sal_Int8 >& rNewDigest )
    { m_xBaseEncryptionData->m_aDigest = rNewDigest;}
    void setIterationCount(::std::optional<sal_Int32> const oNewCount)
    {
        m_xBaseEncryptionData->m_oPBKDFIterationCount = oNewCount;
    }
    void setArgon2Args(::std::optional<::std::tuple<sal_Int32, sal_Int32, sal_Int32>> const oArgon2Args)
    {
        m_xBaseEncryptionData->m_oArgon2Args = oArgon2Args;
    }
    void setSize (const sal_Int64 nNewSize);

    ZipPackageStream( ZipPackage & rNewPackage,
                      const css::uno::Reference < css::uno::XComponentContext >& xContext,
                      sal_Int32 nFormat,
                      bool bAllowRemoveOnInsert );
    virtual ~ZipPackageStream() override;

    css::uno::Reference< css::io::XInputStream > GetRawEncrStreamNoHeaderCopy();
    css::uno::Reference< css::io::XInputStream > TryToGetRawFromDataStream(bool bAddHeaderForEncr );

    bool ParsePackageRawStream();
    virtual bool saveChild( const OUString &rPath,
                            std::vector < css::uno::Sequence < css::beans::PropertyValue > > &rManList,
                            ZipOutputStream & rZipOut,
                            const css::uno::Sequence < sal_Int8 >& rEncryptionKey,
                            ::std::optional<sal_Int32> oPBKDF2IterationCount,
                            ::std::optional<::std::tuple<sal_Int32, sal_Int32, sal_Int32>> oArgon2Args) override;

    void setZipEntryOnLoading( const ZipEntry &rInEntry);
    void successfullyWritten( ZipEntry const *pEntry );

    // XActiveDataSink
    virtual void SAL_CALL setInputStream( const css::uno::Reference< css::io::XInputStream >& aStream ) override;
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getInputStream(  ) override;

    // XDataSinkEncrSupport
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getDataStream() override;
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getRawStream() override;
    virtual void SAL_CALL setDataStream(
                    const css::uno::Reference< css::io::XInputStream >& aStream ) override;
    virtual void SAL_CALL setRawStream(
                    const css::uno::Reference< css::io::XInputStream >& aStream ) override;
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getPlainRawStream() override;

    // XPropertySet
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
