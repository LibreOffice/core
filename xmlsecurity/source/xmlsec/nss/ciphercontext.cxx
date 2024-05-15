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

#include <sal/config.h>

#include <com/sun/star/lang/DisposedException.hpp>
#include <rtl/random.h>
#include <rtl/ref.hxx>
#include <sal/log.hxx>

#include "ciphercontext.hxx"
#include <nss.h> // for NSS_VMINOR
#include <pk11pub.h>

constexpr size_t nAESGCMIVSize = 12;
constexpr size_t nAESGCMTagSize = 16;

using namespace ::com::sun::star;

uno::Reference< xml::crypto::XCipherContext > OCipherContext::Create( CK_MECHANISM_TYPE nNSSCipherID, const uno::Sequence< ::sal_Int8 >& aKey, const uno::Sequence< ::sal_Int8 >& aInitializationVector, bool bEncryption, bool bW3CPadding )
{
    ::rtl::Reference< OCipherContext > xResult = new OCipherContext;

    xResult->m_pSlot = PK11_GetBestSlot( nNSSCipherID, nullptr );
    if (!xResult->m_pSlot)
    {
        SAL_WARN("xmlsecurity.nss", "PK11_GetBestSlot failed");
        throw uno::RuntimeException(u"PK11_GetBestSlot failed"_ustr);
    }

    SECItem aKeyItem = { siBuffer,
        const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(aKey.getConstArray())),
        sal::static_int_cast<unsigned>(aKey.getLength()) };
    xResult->m_pSymKey = PK11_ImportSymKey(xResult->m_pSlot, nNSSCipherID,
        PK11_OriginDerive, bEncryption ? CKA_ENCRYPT : CKA_DECRYPT, &aKeyItem, nullptr);
    if (!xResult->m_pSymKey)
    {
        SAL_WARN("xmlsecurity.nss", "PK11_ImportSymKey failed");
        throw uno::RuntimeException(u"PK11_ImportSymKey failed"_ustr);
    }

    if (nNSSCipherID == CKM_AES_GCM)
    {
        // TODO: when runtime requirements are raised to NSS 3.52,
        // cleanup according to
        // https://fedoraproject.org/wiki/Changes/NssGCMParams
#if NSS_VMINOR >= 52
        xResult->m_pSecParam = SECITEM_AllocItem(nullptr, nullptr, sizeof(CK_NSS_GCM_PARAMS));
#else
        xResult->m_pSecParam = SECITEM_AllocItem(nullptr, nullptr, sizeof(CK_GCM_PARAMS));
#endif
        if (!xResult->m_pSecParam)
        {
            SAL_WARN("xmlsecurity.nss", "SECITEM_AllocItem failed");
            throw uno::RuntimeException(u"SECITEM_AllocItem failed"_ustr);
        }
        assert(aInitializationVector.getLength() == nAESGCMIVSize);
        xResult->m_AESGCMIV = aInitializationVector;
#if NSS_VMINOR >= 52
        auto *const pParams = reinterpret_cast<CK_NSS_GCM_PARAMS*>(xResult->m_pSecParam->data);
#else
        auto *const pParams = reinterpret_cast<CK_GCM_PARAMS*>(xResult->m_pSecParam->data);
#endif
        pParams->pIv = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(xResult->m_AESGCMIV.getConstArray()));
        pParams->ulIvLen = sal::static_int_cast<unsigned>(xResult->m_AESGCMIV.getLength());
        pParams->pAAD = nullptr;
        pParams->ulAADLen = 0;
        pParams->ulTagBits = nAESGCMTagSize * 8;
    }
    else
    {
        SECItem aIVItem = { siBuffer,
            const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(aInitializationVector.getConstArray())),
            sal::static_int_cast<unsigned>(aInitializationVector.getLength()) };
        xResult->m_pSecParam = PK11_ParamFromIV(nNSSCipherID, &aIVItem);
        if (!xResult->m_pSecParam)
        {
            SAL_WARN("xmlsecurity.nss", "PK11_ParamFromIV failed");
            throw uno::RuntimeException(u"PK11_ParamFromIV failed"_ustr);
        }

        xResult->m_pContext = PK11_CreateContextBySymKey( nNSSCipherID, bEncryption ? CKA_ENCRYPT : CKA_DECRYPT, xResult->m_pSymKey, xResult->m_pSecParam);
        if (!xResult->m_pContext)
        {
            SAL_WARN("xmlsecurity.nss", "PK11_CreateContextBySymKey failed");
            throw uno::RuntimeException(u"PK11_CreateContextBySymKey failed"_ustr);
        }
    }

    xResult->m_bEncryption = bEncryption;
    xResult->m_bW3CPadding = bW3CPadding;
    xResult->m_bPadding = bW3CPadding || ( PK11_GetPadMechanism( nNSSCipherID ) == nNSSCipherID );
    // in NSS 3.94, a global default value of 8 is returned for CKM_AES_GCM
    xResult->m_nBlockSize = nNSSCipherID == CKM_AES_GCM ? 16 : PK11_GetBlockSize(nNSSCipherID, xResult->m_pSecParam);
    if (SAL_MAX_INT8 < xResult->m_nBlockSize)
    {
        SAL_WARN("xmlsecurity.nss", "PK11_GetBlockSize unexpected result");
        throw uno::RuntimeException(u"PK11_GetBlockSize unexpected result"_ustr);
    }
    return xResult;
}

void OCipherContext::Dispose()
{
    if ( m_pContext )
    {
        PK11_DestroyContext( m_pContext, PR_TRUE );
        m_pContext = nullptr;
    }

    if ( m_pSecParam )
    {
        SECITEM_FreeItem( m_pSecParam, PR_TRUE );
        m_pSecParam = nullptr;
    }

    if ( m_pSymKey )
    {
        PK11_FreeSymKey( m_pSymKey );
        m_pSymKey = nullptr;
    }

    if ( m_pSlot )
    {
        PK11_FreeSlot( m_pSlot );
        m_pSlot = nullptr;
    }

    m_bDisposed = true;
}

uno::Sequence< ::sal_Int8 > SAL_CALL OCipherContext::convertWithCipherContext( const uno::Sequence< ::sal_Int8 >& aData )
{
    std::unique_lock aGuard( m_aMutex );

    if ( m_bBroken )
        throw uno::RuntimeException();

    if ( m_bDisposed )
        throw lang::DisposedException();

    if (m_AESGCMIV.getLength())
    {
        if (SAL_MAX_INT32 - nAESGCMIVSize - nAESGCMTagSize <= static_cast<size_t>(m_aLastBlock.getLength()) + static_cast<size_t>(aData.getLength()))
        {
            m_bBroken = true;
            throw uno::RuntimeException(u"overflow"_ustr);
        }
        m_aLastBlock.realloc(m_aLastBlock.getLength() + aData.getLength());
        memcpy(m_aLastBlock.getArray() + m_aLastBlock.getLength() - aData.getLength(), aData.getConstArray(), aData.getLength());
        return {};
    }

    uno::Sequence< sal_Int8 > aToConvert;
    if ( aData.hasElements() )
    {
        sal_Int32 nOldLastBlockLen = m_aLastBlock.getLength();

        sal_Int32 nAvailableData = nOldLastBlockLen + aData.getLength();
        sal_Int32 nToConvertLen;
        if ( m_bEncryption || !m_bW3CPadding )
        {
            assert(nOldLastBlockLen < m_nBlockSize);
            if ( nAvailableData % m_nBlockSize == 0 )
                nToConvertLen = nAvailableData;
            else if ( nAvailableData < m_nBlockSize )
                nToConvertLen = 0;
            else
                nToConvertLen = nAvailableData - nAvailableData % m_nBlockSize;
        }
        else
        {
            assert(nOldLastBlockLen < m_nBlockSize * 2);
            // decryption with W3C padding needs at least one block for finalizing
            if ( nAvailableData < m_nBlockSize * 2 )
                nToConvertLen = 0;
            else
                nToConvertLen = nAvailableData - nAvailableData % m_nBlockSize - m_nBlockSize;
        }

        aToConvert.realloc( nToConvertLen );
        if ( nToConvertLen == 0 )
        {
            m_aLastBlock.realloc( nOldLastBlockLen + aData.getLength() );
            memcpy( m_aLastBlock.getArray() + nOldLastBlockLen, aData.getConstArray(), aData.getLength() );
            // aToConvert stays empty
        }
        else if ( nToConvertLen < nOldLastBlockLen )
        {
            memcpy( aToConvert.getArray(), m_aLastBlock.getConstArray(), nToConvertLen );
            memcpy( m_aLastBlock.getArray(), m_aLastBlock.getConstArray() + nToConvertLen, nOldLastBlockLen - nToConvertLen );
            m_aLastBlock.realloc( nOldLastBlockLen - nToConvertLen + aData.getLength() );
            memcpy( m_aLastBlock.getArray() + nOldLastBlockLen - nToConvertLen, aData.getConstArray(), aData.getLength() );
        }
        else
        {
            memcpy( aToConvert.getArray(), m_aLastBlock.getConstArray(), nOldLastBlockLen );
            if ( nToConvertLen > nOldLastBlockLen )
                memcpy( aToConvert.getArray() + nOldLastBlockLen, aData.getConstArray(), nToConvertLen - nOldLastBlockLen );
            m_aLastBlock.realloc( nAvailableData - nToConvertLen );
            memcpy( m_aLastBlock.getArray(), aData.getConstArray() + nToConvertLen - nOldLastBlockLen, nAvailableData - nToConvertLen );
        }
    }

    uno::Sequence< sal_Int8 > aResult;
    assert(aToConvert.getLength() % m_nBlockSize == 0);
    if ( aToConvert.hasElements() )
    {
        int nResultLen = 0;
        aResult.realloc( aToConvert.getLength() + m_nBlockSize );
        if ( PK11_CipherOp( m_pContext, reinterpret_cast< unsigned char* >( aResult.getArray() ), &nResultLen, aResult.getLength(), reinterpret_cast< const unsigned char* >( aToConvert.getConstArray() ), aToConvert.getLength() ) != SECSuccess )
        {
            m_bBroken = true;
            Dispose();
            throw uno::RuntimeException(u"PK11_CipherOp failed"_ustr);
        }

        m_nConverted += aToConvert.getLength();
        aResult.realloc( nResultLen );
    }

    return aResult;
}

uno::Sequence< ::sal_Int8 > SAL_CALL OCipherContext::finalizeCipherContextAndDispose()
{
    std::unique_lock aGuard( m_aMutex );

    if ( m_bBroken )
        throw uno::RuntimeException();

    if ( m_bDisposed )
        throw lang::DisposedException();

    if (m_AESGCMIV.getLength())
    {
        uno::Sequence<sal_Int8> aResult;
        unsigned outLen;
        if (m_bEncryption)
        {
            assert(sal::static_int_cast<size_t>(m_aLastBlock.getLength()) <= SAL_MAX_INT32 - nAESGCMIVSize - nAESGCMTagSize);
            // add space for IV and tag
            aResult.realloc(m_aLastBlock.getLength() + nAESGCMIVSize + nAESGCMTagSize);
            // W3C xmlenc-core1 requires the IV preceding the ciphertext,
            // but NSS doesn't do it, so copy it manually
            memcpy(aResult.getArray(), m_AESGCMIV.getConstArray(), nAESGCMIVSize);
            if (PK11_Encrypt(m_pSymKey, CKM_AES_GCM, m_pSecParam,
                    reinterpret_cast<unsigned char*>(aResult.getArray() + nAESGCMIVSize),
                    &outLen, aResult.getLength() - nAESGCMIVSize,
                    reinterpret_cast<unsigned char const*>(m_aLastBlock.getConstArray()),
                    m_aLastBlock.getLength()) != SECSuccess)
            {
                m_bBroken = true;
                Dispose();
                throw uno::RuntimeException(u"PK11_Encrypt failed"_ustr);
            }
            assert(outLen == sal::static_int_cast<unsigned>(aResult.getLength() - nAESGCMIVSize));
        }
        else if (nAESGCMIVSize + nAESGCMTagSize < sal::static_int_cast<size_t>(m_aLastBlock.getLength()))
        {
            if (0 != memcmp(m_AESGCMIV.getConstArray(), m_aLastBlock.getConstArray(), nAESGCMIVSize))
            {
                m_bBroken = true;
                Dispose();
                throw uno::RuntimeException(u"inconsistent IV"_ustr);
            }
            aResult.realloc(m_aLastBlock.getLength() - nAESGCMIVSize - nAESGCMTagSize);
            if (PK11_Decrypt(m_pSymKey, CKM_AES_GCM, m_pSecParam,
                    reinterpret_cast<unsigned char*>(aResult.getArray()),
                    &outLen, aResult.getLength(),
                    reinterpret_cast<unsigned char const*>(m_aLastBlock.getConstArray() + nAESGCMIVSize),
                    m_aLastBlock.getLength() - nAESGCMIVSize) != SECSuccess)
            {
                m_bBroken = true;
                Dispose();
                throw uno::RuntimeException(u"PK11_Decrypt failed"_ustr);
            }
            assert(outLen == sal::static_int_cast<unsigned>(aResult.getLength()));
        }
        else
        {
            m_bBroken = true;
            Dispose();
            throw uno::RuntimeException(u"incorrect size of input"_ustr);
        }
        Dispose();
        return aResult;
    }

    assert(m_nBlockSize <= SAL_MAX_INT8);
    assert(m_nConverted % m_nBlockSize == 0); // whole blocks are converted
    sal_Int32 nSizeForPadding = ( m_nConverted + m_aLastBlock.getLength() ) % m_nBlockSize;

    // if it is decryption, the amount of data should be rounded to the block size even in case of padding
    if ( ( !m_bPadding || !m_bEncryption ) && nSizeForPadding )
        throw uno::RuntimeException(u"The data should contain complete blocks only."_ustr );

    if ( m_bW3CPadding && m_bEncryption )
    {
        // in this case the last block should be smaller than standard block
        // it will be increased with the padding
        assert(m_aLastBlock.getLength() < m_nBlockSize);

        // W3CPadding handling for encryption
        sal_Int32 nPaddingSize = m_nBlockSize - nSizeForPadding;
        sal_Int32 nOldLastBlockLen = m_aLastBlock.getLength();
        m_aLastBlock.realloc( nOldLastBlockLen + nPaddingSize );
        auto pLastBlock = m_aLastBlock.getArray();

        if ( nPaddingSize > 1 )
        {
            if (rtl_random_getBytes(nullptr, pLastBlock + nOldLastBlockLen, nPaddingSize - 1) != rtl_Random_E_None)
            {
                throw uno::RuntimeException(u"rtl_random_getBytes failed"_ustr);
            }
        }
        pLastBlock[m_aLastBlock.getLength() - 1] = static_cast< sal_Int8 >( nPaddingSize );
    }

    // finally should the last block be smaller than two standard blocks
    assert(m_aLastBlock.getLength() < m_nBlockSize * 2);

    uno::Sequence< sal_Int8 > aResult;
    if ( m_aLastBlock.hasElements() )
    {
        int nPrefResLen = 0;
        aResult.realloc( m_aLastBlock.getLength() + m_nBlockSize );
        if ( PK11_CipherOp( m_pContext, reinterpret_cast< unsigned char* >( aResult.getArray() ), &nPrefResLen, aResult.getLength(), reinterpret_cast< const unsigned char* >( m_aLastBlock.getConstArray() ), m_aLastBlock.getLength() ) != SECSuccess )
        {
            m_bBroken = true;
            Dispose();
            throw uno::RuntimeException(u"PK11_CipherOp failed"_ustr);
        }

        aResult.realloc( nPrefResLen );
        m_aLastBlock.realloc( 0 );
    }

    sal_Int32 nPrefixLen = aResult.getLength();
    aResult.realloc( nPrefixLen + m_nBlockSize * 2 );
    unsigned nFinalLen = 0;
    if ( PK11_DigestFinal( m_pContext, reinterpret_cast< unsigned char* >( aResult.getArray() + nPrefixLen ), &nFinalLen, aResult.getLength() - nPrefixLen ) != SECSuccess )
    {
        m_bBroken = true;
        Dispose();
        throw uno::RuntimeException(u"PK11_DigestFinal failed"_ustr);
    }

    aResult.realloc( nPrefixLen + nFinalLen );

    if ( m_bW3CPadding && !m_bEncryption )
    {
        // W3CPadding handling for decryption
        // aResult should have enough data, except if the input was completely empty

        // see https://www.w3.org/TR/xmlenc-core1/#sec-Alg-Block
        if (aResult.getLength() < m_nBlockSize
            || aResult[aResult.getLength()-1] <= 0
            || m_nBlockSize < aResult[aResult.getLength()-1])
        {
            m_bBroken = true;
            Dispose();
            throw uno::RuntimeException(u"incorrect size of padding"_ustr);
        }

        aResult.realloc(aResult.getLength() - aResult[aResult.getLength()-1]);
    }

    Dispose();

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
