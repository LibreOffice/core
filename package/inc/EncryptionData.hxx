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

class BaseEncryptionData : public cppu::OWeakObject
{
public:
    css::uno::Sequence< sal_Int8 > m_aSalt;
    css::uno::Sequence< sal_Int8 > m_aInitVector;
    css::uno::Sequence< sal_Int8 > m_aDigest;
    sal_Int32 m_nIterationCount;

    BaseEncryptionData()
    : m_nIterationCount ( 0 ){}

    BaseEncryptionData( const BaseEncryptionData& aData )
    : cppu::OWeakObject()
    , m_aSalt( aData.m_aSalt )
    , m_aInitVector( aData.m_aInitVector )
    , m_aDigest( aData.m_aDigest )
    , m_nIterationCount( aData.m_nIterationCount )
    {}
};

class EncryptionData : public BaseEncryptionData
{
public:
    css::uno::Sequence < sal_Int8 > m_aKey;
    sal_Int32 m_nEncAlg;
    sal_Int32 m_nCheckAlg;
    sal_Int32 m_nDerivedKeySize;
    sal_Int32 m_nStartKeyGenID;

    EncryptionData( const BaseEncryptionData& aData, const css::uno::Sequence< sal_Int8 >& aKey, sal_Int32 nEncAlg, sal_Int32 nCheckAlg, sal_Int32 nDerivedKeySize, sal_Int32 nStartKeyGenID )
    : BaseEncryptionData( aData )
    , m_aKey( aKey )
    , m_nEncAlg( nEncAlg )
    , m_nCheckAlg( nCheckAlg )
    , m_nDerivedKeySize( nDerivedKeySize )
    , m_nStartKeyGenID( nStartKeyGenID )
    {}

    EncryptionData( const EncryptionData& aData )
    : BaseEncryptionData( aData )
    , m_aKey( aData.m_aKey )
    , m_nEncAlg( aData.m_nEncAlg )
    , m_nCheckAlg( aData.m_nCheckAlg )
    , m_nDerivedKeySize( aData.m_nDerivedKeySize )
    , m_nStartKeyGenID( aData.m_nStartKeyGenID )
    {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
