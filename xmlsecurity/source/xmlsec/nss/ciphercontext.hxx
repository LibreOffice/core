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

#ifndef _CIPHERCONTEXT_HXX
#define _CIPHERCONTEXT_HXX

#include <com/sun/star/xml/crypto/XCipherContext.hpp>

#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>
#include <pk11pub.h>

class OCipherContext : public cppu::WeakImplHelper1< ::com::sun::star::xml::crypto::XCipherContext >
{
private:
    ::osl::Mutex m_aMutex;

    PK11SlotInfo* m_pSlot;
    PK11SymKey* m_pSymKey;
    SECItem* m_pSecParam;
    PK11Context* m_pContext;
    sal_Int32 m_nBlockSize;
    bool m_bPadding;

    bool m_bDisposed;
    bool m_bBroken;

    void Dispose();

    OCipherContext()
    : m_pSlot( NULL )
    , m_pSymKey( NULL )
    , m_pSecParam( NULL )
    , m_pContext( NULL )
    , m_bPadding( false )
    , m_nBlockSize( 0 )
    , m_bDisposed( false )
    , m_bBroken( false )
    {}

public:

    virtual ~OCipherContext()
    {
        Dispose();
    }

    static ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XCipherContext > Create( CK_MECHANISM_TYPE nNSSCipherID, const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aKey, const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aInitializationVector, bool bEncryption );

    // XCipherContext
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL convertWithCipherContext( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::DisposedException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL finalizeCipherContextAndDispose(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::uno::RuntimeException);
};

#endif

