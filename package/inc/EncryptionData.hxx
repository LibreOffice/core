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
#ifndef _ENCRYPTION_DATA_HXX_
#define _ENCRYPTION_DATA_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/weak.hxx>

class BaseEncryptionData : public cppu::OWeakObject
{
public:
    ::com::sun::star::uno::Sequence< sal_uInt8 > m_aSalt;
    ::com::sun::star::uno::Sequence< sal_uInt8 > m_aInitVector;
    ::com::sun::star::uno::Sequence< sal_uInt8 > m_aDigest;
    sal_Int32 m_nIterationCount;

    BaseEncryptionData()
    : m_nIterationCount ( 0 ){}

    BaseEncryptionData( const BaseEncryptionData& aData )
    : m_aSalt( aData.m_aSalt )
    , m_aInitVector( aData.m_aInitVector )
    , m_aDigest( aData.m_aDigest )
    , m_nIterationCount( aData.m_nIterationCount )
    {}
};

class EncryptionData : public BaseEncryptionData
{
public:
    ::com::sun::star::uno::Sequence < sal_Int8 > m_aKey;
    sal_Int32 m_nEncAlg;
    sal_Int32 m_nCheckAlg;
    sal_Int32 m_nDerivedKeySize;

    EncryptionData( const BaseEncryptionData& aData, const ::com::sun::star::uno::Sequence< sal_Int8 >& aKey, sal_Int32 nEncAlg, sal_Int32 nCheckAlg, sal_Int32 nDerivedKeySize )
    : BaseEncryptionData( aData )
    , m_aKey( aKey )
    , m_nEncAlg( nEncAlg )
    , m_nCheckAlg( nCheckAlg )
    , m_nDerivedKeySize( nDerivedKeySize )
    {}

    EncryptionData( const EncryptionData& aData )
    : BaseEncryptionData( aData )
    , m_aKey( aData.m_aKey )
    , m_nEncAlg( aData.m_nEncAlg )
    , m_nCheckAlg( aData.m_nCheckAlg )
    , m_nDerivedKeySize( aData.m_nDerivedKeySize )
    {}
};

#endif
