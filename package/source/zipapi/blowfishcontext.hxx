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
#ifndef _BLOWFISHCONTEXT_HXX
#define _BLOWFISHCONTEXT_HXX

#include <com/sun/star/xml/crypto/XCipherContext.hpp>

#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>

class BlowfishCFB8CipherContext : public cppu::WeakImplHelper1< ::com::sun::star::xml::crypto::XCipherContext >
{
    ::osl::Mutex m_aMutex;
    void* m_pCipher;

    BlowfishCFB8CipherContext()
    : m_pCipher( NULL )
    {}

public:

    virtual ~BlowfishCFB8CipherContext();

    static ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XCipherContext >
        Create( const ::com::sun::star::uno::Sequence< sal_Int8 >& aDerivedKey, const ::com::sun::star::uno::Sequence< sal_Int8 >& aInitVector, bool bEncrypt );

    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL convertWithCipherContext( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::DisposedException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL finalizeCipherContextAndDispose(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::uno::RuntimeException);
};

#endif // _BLOWFISHCONTEXT_HXX

