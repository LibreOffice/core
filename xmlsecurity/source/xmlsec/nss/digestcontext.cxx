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

#include <pk11pub.h>
#include "digestcontext.hxx"

using namespace ::com::sun::star;

ODigestContext::~ODigestContext()
{
    if ( m_pContext )
    {
        PK11_DestroyContext( m_pContext, PR_TRUE );
        m_pContext = nullptr;
    }
}

void SAL_CALL ODigestContext::updateDigest( const uno::Sequence< ::sal_Int8 >& aData )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bBroken )
        throw uno::RuntimeException();

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_b1KData || m_nDigested < 1024 )
    {
        uno::Sequence< sal_Int8 > aToDigest = aData;
        if ( m_b1KData && m_nDigested + aData.getLength() > 1024 )
            aToDigest.realloc( 1024 - m_nDigested );

        if ( PK11_DigestOp( m_pContext, reinterpret_cast< const unsigned char* >( aToDigest.getConstArray() ), aToDigest.getLength() ) != SECSuccess )
        {
            PK11_DestroyContext( m_pContext, PR_TRUE );
            m_pContext = nullptr;
            m_bBroken = true;
            throw uno::RuntimeException();
        }

        m_nDigested += aToDigest.getLength();
    }
}

uno::Sequence< ::sal_Int8 > SAL_CALL ODigestContext::finalizeDigestAndDispose()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bBroken )
        throw uno::RuntimeException();

    if ( m_bDisposed )
        throw lang::DisposedException();

    uno::Sequence< sal_Int8 > aResult( m_nDigestLength );
    unsigned int nResultLen = 0;
    if ( PK11_DigestFinal( m_pContext, reinterpret_cast< unsigned char* >( aResult.getArray() ), &nResultLen, aResult.getLength() ) != SECSuccess )
    {
        PK11_DestroyContext( m_pContext, PR_TRUE );
        m_pContext = nullptr;
        m_bBroken = true;
        throw uno::RuntimeException();
    }

    PK11_DestroyContext( m_pContext, PR_TRUE );
    m_pContext = nullptr;
    m_bDisposed = true;

    aResult.realloc( nResultLen );
    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
