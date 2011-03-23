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

#include <precompiled_xmlsecurity.hxx>

#include <osl/time.h>
#include <rtl/random.h>
#include <rtl/ref.hxx>

#include "ciphercontext.hxx"

using namespace ::com::sun::star;

uno::Reference< xml::crypto::XCipherContext > OCipherContext::Create( CK_MECHANISM_TYPE nNSSCipherID, const uno::Sequence< ::sal_Int8 >& aKey, const uno::Sequence< ::sal_Int8 >& aInitializationVector, bool bEncryption, bool bW3CPadding )
{
    ::rtl::Reference< OCipherContext > xResult = new OCipherContext;

    xResult->m_pSlot = PK11_GetBestSlot( nNSSCipherID, NULL );
    if ( xResult->m_pSlot )
    {
        SECItem aKeyItem = { siBuffer, const_cast< unsigned char* >( reinterpret_cast< const unsigned char* >( aKey.getConstArray() ) ), aKey.getLength() };
        xResult->m_pSymKey = PK11_ImportSymKey( xResult->m_pSlot, nNSSCipherID, PK11_OriginDerive, bEncryption ? CKA_ENCRYPT : CKA_DECRYPT, &aKeyItem, NULL );
        if ( xResult->m_pSymKey )
        {
            SECItem aIVItem = { siBuffer, const_cast< unsigned char* >( reinterpret_cast< const unsigned char* >( aInitializationVector.getConstArray() ) ), aInitializationVector.getLength() };
            xResult->m_pSecParam = PK11_ParamFromIV( nNSSCipherID, &aIVItem );
            if ( xResult->m_pSecParam )
            {
                xResult->m_pContext = PK11_CreateContextBySymKey( nNSSCipherID, bEncryption ? CKA_ENCRYPT : CKA_DECRYPT, xResult->m_pSymKey, xResult->m_pSecParam);
                if ( xResult->m_pContext )
                {
                    xResult->m_bEncryption = bEncryption;
                    xResult->m_bW3CPadding = bW3CPadding;
                    xResult->m_bPadding = bW3CPadding || ( PK11_GetPadMechanism( nNSSCipherID ) == nNSSCipherID );
                    xResult->m_nBlockSize = PK11_GetBlockSize( nNSSCipherID, xResult->m_pSecParam );
                    if ( xResult->m_nBlockSize <= SAL_MAX_INT8 )
                        return xResult.get();
                }
            }
        }
    }

    return uno::Reference< xml::crypto::XCipherContext >();
}

void OCipherContext::Dispose()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pContext )
    {
        PK11_DestroyContext( m_pContext, PR_TRUE );
        m_pContext = NULL;
    }

    if ( m_pSecParam )
    {
        SECITEM_FreeItem( m_pSecParam, PR_TRUE );
        m_pSecParam = NULL;
    }

    if ( m_pSymKey )
    {
        PK11_FreeSymKey( m_pSymKey );
        m_pSymKey = NULL;
    }

    if ( m_pSlot )
    {
        PK11_FreeSlot( m_pSlot );
        m_pSlot = NULL;
    }

    m_bDisposed = true;
}

uno::Sequence< ::sal_Int8 > SAL_CALL OCipherContext::convertWithCipherContext( const uno::Sequence< ::sal_Int8 >& aData )
    throw ( lang::IllegalArgumentException, lang::DisposedException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bBroken )
        throw uno::RuntimeException();

    if ( m_bDisposed )
        throw lang::DisposedException();

    uno::Sequence< sal_Int8 > aToConvert;
    if ( aData.getLength() )
    {
        sal_Int32 nOldLastBlockLen = m_aLastBlock.getLength();
        OSL_ENSURE( nOldLastBlockLen <= m_nBlockSize, "Unexpected last block size!" );

        sal_Int32 nAvailableData = nOldLastBlockLen + aData.getLength();
        sal_Int32 nToConvertLen = nAvailableData;
        if ( m_bEncryption || !m_bW3CPadding )
        {
            if ( nAvailableData % m_nBlockSize == 0 )
                nToConvertLen = nAvailableData;
            else if ( nAvailableData < m_nBlockSize )
                nToConvertLen = 0;
            else
                nToConvertLen = nAvailableData - nAvailableData % m_nBlockSize;
        }
        else
        {
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
            rtl_copyMemory( m_aLastBlock.getArray() + nOldLastBlockLen, aData.getConstArray(), aData.getLength() );
            // aToConvert stays empty
        }
        else if ( nToConvertLen < nOldLastBlockLen )
        {
            rtl_copyMemory( aToConvert.getArray(), m_aLastBlock.getConstArray(), nToConvertLen );
            rtl_copyMemory( m_aLastBlock.getArray(), m_aLastBlock.getConstArray() + nToConvertLen, nOldLastBlockLen - nToConvertLen );
            m_aLastBlock.realloc( nOldLastBlockLen - nToConvertLen + aData.getLength() );
            rtl_copyMemory( m_aLastBlock.getArray() + nOldLastBlockLen - nToConvertLen, aData.getConstArray(), aData.getLength() );
        }
        else
        {
            rtl_copyMemory( aToConvert.getArray(), m_aLastBlock.getConstArray(), nOldLastBlockLen );
            if ( nToConvertLen > nOldLastBlockLen )
                rtl_copyMemory( aToConvert.getArray() + nOldLastBlockLen, aData.getConstArray(), nToConvertLen - nOldLastBlockLen );
            m_aLastBlock.realloc( nAvailableData - nToConvertLen );
            rtl_copyMemory( m_aLastBlock.getArray(), aData.getConstArray() + nToConvertLen - nOldLastBlockLen, nAvailableData - nToConvertLen );
        }
    }

    uno::Sequence< sal_Int8 > aResult;
    OSL_ENSURE( aToConvert.getLength() % m_nBlockSize == 0, "Unexpected size of the data to encrypt!" );
    if ( aToConvert.getLength() )
    {
        int nResultLen = 0;
        aResult.realloc( aToConvert.getLength() + m_nBlockSize );
        if ( PK11_CipherOp( m_pContext, reinterpret_cast< unsigned char* >( aResult.getArray() ), &nResultLen, aResult.getLength(), const_cast< unsigned char* >( reinterpret_cast< const unsigned char* >( aToConvert.getConstArray() ) ), aToConvert.getLength() ) != SECSuccess )
        {
            m_bBroken = true;
            Dispose();
            throw uno::RuntimeException();
        }

        m_nConverted += aToConvert.getLength();
        aResult.realloc( nResultLen );
    }

    return aResult;
}

uno::Sequence< ::sal_Int8 > SAL_CALL OCipherContext::finalizeCipherContextAndDispose()
    throw (lang::DisposedException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bBroken )
        throw uno::RuntimeException();

    if ( m_bDisposed )
        throw lang::DisposedException();

    OSL_ENSURE( m_nBlockSize <= SAL_MAX_INT8, "Unexpected block size!" );
    OSL_ENSURE( m_nConverted % m_nBlockSize == 0, "Unexpected amount of bytes is already converted!" );
    sal_Int32 nSizeForPadding = ( m_nConverted + m_aLastBlock.getLength() ) % m_nBlockSize;

    // if it is decryption, the amount of data should be rounded to the block size even in case of padding
    if ( ( !m_bPadding || !m_bEncryption ) && nSizeForPadding )
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "The data should contain complete blocks only." ) ), uno::Reference< uno::XInterface >() );

    if ( m_bW3CPadding && m_bEncryption )
    {
        // in this case the last block should be smaller than standtard block
        // it will be increased with the padding
        OSL_ENSURE( m_aLastBlock.getLength() < m_nBlockSize, "Unexpected size of cashed incomplete last block!" );

        // W3CPadding handling for encryption
        sal_Int32 nPaddingSize = m_nBlockSize - nSizeForPadding;
        sal_Int32 nOldLastBlockLen = m_aLastBlock.getLength();
        m_aLastBlock.realloc( nOldLastBlockLen + nPaddingSize );

        if ( nPaddingSize > 1 )
        {
            TimeValue aTime;
            osl_getSystemTime( &aTime );
            rtlRandomPool aRandomPool = rtl_random_createPool();
            rtl_random_addBytes( aRandomPool, &aTime, 8 );
            rtl_random_getBytes( aRandomPool, m_aLastBlock.getArray() + nOldLastBlockLen, nPaddingSize - 1 );
            rtl_random_destroyPool ( aRandomPool );
        }
        m_aLastBlock[m_aLastBlock.getLength() - 1] = nPaddingSize;
    }

    // finally should the last block be smaller than two standard blocks
    OSL_ENSURE( m_aLastBlock.getLength() < m_nBlockSize * 2 , "Unexpected size of cashed incomplete last block!" );

    uno::Sequence< sal_Int8 > aResult;
    if ( m_aLastBlock.getLength() )
    {
        int nPrefResLen = 0;
        aResult.realloc( m_aLastBlock.getLength() + m_nBlockSize );
        if ( PK11_CipherOp( m_pContext, reinterpret_cast< unsigned char* >( aResult.getArray() ), &nPrefResLen, aResult.getLength(), const_cast< unsigned char* >( reinterpret_cast< const unsigned char* >( m_aLastBlock.getConstArray() ) ), m_aLastBlock.getLength() ) != SECSuccess )
        {
            m_bBroken = true;
            Dispose();
            throw uno::RuntimeException();
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
        throw uno::RuntimeException();
    }

    aResult.realloc( nPrefixLen + nFinalLen );

    if ( m_bW3CPadding && !m_bEncryption )
    {
        // W3CPadding handling for decryption
        // aResult should have anough data, since we let m_aLastBlock be big enough in case of decryption
        OSL_ENSURE( aResult.getLength() >= m_nBlockSize, "Not enough data to handle the padding!" );

        sal_Int8 nBytesToRemove = aResult[aResult.getLength() - 1];
        if ( nBytesToRemove <= 0 || nBytesToRemove > aResult.getLength() )
        {
            m_bBroken = true;
            Dispose();
            throw uno::RuntimeException();
        }

        aResult.realloc( aResult.getLength() - nBytesToRemove );
    }

    Dispose();

    return aResult;
}

