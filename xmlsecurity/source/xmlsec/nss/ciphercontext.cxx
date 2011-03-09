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

#include <rtl/ref.hxx>
#include "ciphercontext.hxx"

using namespace ::com::sun::star;

uno::Reference< xml::crypto::XCipherContext > OCipherContext::Create( CK_MECHANISM_TYPE nNSSCipherID, const uno::Sequence< ::sal_Int8 >& aKey, const uno::Sequence< ::sal_Int8 >& aInitializationVector, bool bEncryption )
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
                    xResult->m_bPadding = ( PK11_GetPadMechanism( nNSSCipherID ) == nNSSCipherID );
                    xResult->m_nBlockSize = PK11_GetBlockSize( nNSSCipherID, xResult->m_pSecParam );
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

    if ( !m_bPadding && aData.getLength() % m_nBlockSize )
        throw lang::IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CBC without padding is used, the data should contain complete blocks only." ) ), uno::Reference< uno::XInterface >(), 1 );

    int nResultLen = 0;
    uno::Sequence< sal_Int8 > aResult( aData.getLength() );
    if ( PK11_CipherOp( m_pContext, reinterpret_cast< unsigned char* >( aResult.getArray() ), &nResultLen, aResult.getLength(), const_cast< unsigned char* >( reinterpret_cast< const unsigned char* >( aData.getConstArray() ) ), aData.getLength() ) != SECSuccess )
    {
        m_bBroken = true;
        Dispose();
        throw uno::RuntimeException();
    }

    aResult.realloc( nResultLen );
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

    unsigned nResultLen = 0;
    uno::Sequence< sal_Int8 > aResult( m_nBlockSize );
    if ( PK11_DigestFinal( m_pContext, reinterpret_cast< unsigned char* >( aResult.getArray() ), &nResultLen, aResult.getLength() ) != SECSuccess )
    {
        m_bBroken = true;
        Dispose();
        throw uno::RuntimeException();
    }

    aResult.realloc( nResultLen );
    return aResult;

}

