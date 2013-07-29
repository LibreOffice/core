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
        throw uno::RuntimeException("Can not create cipher!",
                                     uno::Reference< XInterface >() );

    if ( rtl_Cipher_E_None != rtl_cipher_init(
                                xResult->m_pCipher,
                                bEncrypt ? rtl_Cipher_DirectionEncode : rtl_Cipher_DirectionDecode,
                                reinterpret_cast< const sal_uInt8* >( aDerivedKey.getConstArray() ),
                                aDerivedKey.getLength(),
                                reinterpret_cast< const sal_uInt8* >( aInitVector.getConstArray() ),
                                aInitVector.getLength() ) )
    {
        throw uno::RuntimeException("Can not initialize cipher!",
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
        nError = rtl_cipher_encode( m_pCipher,
                          aData.getConstArray(),
                          aData.getLength(),
                          reinterpret_cast< sal_uInt8* >( aResult.getArray() ),
                          aResult.getLength() );
    }
    else
    {
        nError = rtl_cipher_decode( m_pCipher,
                          aData.getConstArray(),
                          aData.getLength(),
                          reinterpret_cast< sal_uInt8* >( aResult.getArray() ),
                          aResult.getLength() );
    }

    if ( rtl_Cipher_E_None != nError )
    {
        throw uno::RuntimeException("Can not decrypt/encrypt with cipher!",
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
