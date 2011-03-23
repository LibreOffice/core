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

#include <rtl/cipher.h>
#include <rtl/ref.hxx>

#include "blowfishcontext.hxx"

using namespace ::com::sun::star;

// static
uno::Reference< xml::crypto::XCipherContext > BlowfishCFB8CipherContext::Create( const uno::Sequence< sal_Int8 >& aDerivedKey, const uno::Sequence< sal_Int8 >& aInitVector, bool bEncrypt )
{
    ::rtl::Reference< BlowfishCFB8CipherContext > xResult = new BlowfishCFB8CipherContext();
    xResult->m_pCipher = rtl_cipher_create( rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeStream );
    if ( !xResult->m_pCipher )
        throw uno::RuntimeException( ::rtl::OUString::createFromAscii( "Can not create cipher!\n" ),
                                     uno::Reference< XInterface >() );

    if ( rtl_Cipher_E_None != rtl_cipher_init(
                                xResult->m_pCipher,
                                bEncrypt ? rtl_Cipher_DirectionEncode : rtl_Cipher_DirectionDecode,
                                reinterpret_cast< const sal_uInt8* >( aDerivedKey.getConstArray() ),
                                aDerivedKey.getLength(),
                                reinterpret_cast< const sal_uInt8* >( aInitVector.getConstArray() ),
                                aInitVector.getLength() ) )
    {
        throw uno::RuntimeException( ::rtl::OUString::createFromAscii( "Can not initialize cipher!\n" ),
                                     uno::Reference< XInterface >() );
    }

    xResult->m_bEncrypt = bEncrypt;

    return uno::Reference< xml::crypto::XCipherContext >( xResult.get() );
}

BlowfishCFB8CipherContext::~BlowfishCFB8CipherContext()
{
    if ( m_pCipher )
    {
        rtl_cipher_destroy ( m_pCipher );
        m_pCipher = NULL;
    }
}

uno::Sequence< sal_Int8 > SAL_CALL BlowfishCFB8CipherContext::convertWithCipherContext( const uno::Sequence< ::sal_Int8 >& aData )
    throw( lang::IllegalArgumentException, lang::DisposedException, uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_pCipher )
        throw lang::DisposedException();

    uno::Sequence< sal_Int8 > aResult( aData.getLength() );
    rtlCipherError nError = rtl_Cipher_E_None;

    if ( m_bEncrypt )
    {
        rtl_cipher_encode( m_pCipher,
                          aData.getConstArray(),
                          aData.getLength(),
                          reinterpret_cast< sal_uInt8* >( aResult.getArray() ),
                          aResult.getLength() );
    }
    else
    {
        rtl_cipher_decode( m_pCipher,
                          aData.getConstArray(),
                          aData.getLength(),
                          reinterpret_cast< sal_uInt8* >( aResult.getArray() ),
                          aResult.getLength() );
    }

    if ( rtl_Cipher_E_None != nError )
    {
        throw uno::RuntimeException( ::rtl::OUString::createFromAscii( "Can not decrypt/encrypt with cipher!\n" ),
                                     uno::Reference< uno::XInterface >() );
    }

    return aResult;
}

uno::Sequence< ::sal_Int8 > SAL_CALL BlowfishCFB8CipherContext::finalizeCipherContextAndDispose()
    throw( lang::DisposedException, uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_pCipher )
        throw lang::DisposedException();

    rtl_cipher_destroy ( m_pCipher );
    m_pCipher = NULL;

    return uno::Sequence< sal_Int8 >();
}


