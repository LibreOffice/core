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
#ifndef INCLUDED_PACKAGE_INC_ENCRYPTIONDATA_HXX
#define INCLUDED_PACKAGE_INC_ENCRYPTIONDATA_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/weak.hxx>

#include <optional>
#include <tuple>

class BaseEncryptionData : public cppu::OWeakObject
{
public:
    css::uno::Sequence< sal_Int8 > m_aSalt;
    css::uno::Sequence< sal_Int8 > m_aInitVector;
    css::uno::Sequence< sal_Int8 > m_aDigest;
    ::std::optional<sal_Int32> m_oPBKDFIterationCount;
    ::std::optional<::std::tuple<sal_Int32, sal_Int32, sal_Int32>> m_oArgon2Args;

    BaseEncryptionData()
    {
    }

    BaseEncryptionData( const BaseEncryptionData& aData )
    : cppu::OWeakObject()
    , m_aSalt( aData.m_aSalt )
    , m_aInitVector( aData.m_aInitVector )
    , m_aDigest( aData.m_aDigest )
    , m_oPBKDFIterationCount(aData.m_oPBKDFIterationCount)
    , m_oArgon2Args(aData.m_oArgon2Args)
    {}
};

class EncryptionData final : public BaseEncryptionData
{
public:
    css::uno::Sequence < sal_Int8 > m_aKey;
    sal_Int32 m_nEncAlg;
    ::std::optional<sal_Int32> m_oCheckAlg;
    sal_Int32 m_nDerivedKeySize;
    sal_Int32 m_nStartKeyGenID;
    bool m_bTryWrongSHA1;

    EncryptionData(const BaseEncryptionData& aData,
        const css::uno::Sequence<sal_Int8>& aKey, sal_Int32 const nEncAlg,
        ::std::optional<sal_Int32> const oCheckAlg,
        sal_Int32 const nDerivedKeySize, sal_Int32 const nStartKeyGenID,
        bool const bTryWrongSHA1)
    : BaseEncryptionData( aData )
    , m_aKey( aKey )
    , m_nEncAlg( nEncAlg )
    , m_oCheckAlg( oCheckAlg )
    , m_nDerivedKeySize( nDerivedKeySize )
    , m_nStartKeyGenID( nStartKeyGenID )
    , m_bTryWrongSHA1(bTryWrongSHA1)
    {}

    EncryptionData( const EncryptionData& aData )
    : BaseEncryptionData( aData )
    , m_aKey( aData.m_aKey )
    , m_nEncAlg( aData.m_nEncAlg )
    , m_oCheckAlg( aData.m_oCheckAlg )
    , m_nDerivedKeySize( aData.m_nDerivedKeySize )
    , m_nStartKeyGenID( aData.m_nStartKeyGenID )
    , m_bTryWrongSHA1(aData.m_bTryWrongSHA1)
    {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
