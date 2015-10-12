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
#ifndef INCLUDED_PACKAGE_SOURCE_ZIPAPI_BLOWFISHCONTEXT_HXX
#define INCLUDED_PACKAGE_SOURCE_ZIPAPI_BLOWFISHCONTEXT_HXX

#include <com/sun/star/xml/crypto/XCipherContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>

class BlowfishCFB8CipherContext : public cppu::WeakImplHelper< ::com::sun::star::xml::crypto::XCipherContext >
{
    ::osl::Mutex m_aMutex;
    void* m_pCipher;
    bool m_bEncrypt;

    BlowfishCFB8CipherContext()
    : m_pCipher( NULL )
    , m_bEncrypt( false )
    {}

public:

    virtual ~BlowfishCFB8CipherContext();

    static ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XCipherContext >
        Create( const ::com::sun::star::uno::Sequence< sal_Int8 >& aDerivedKey, const ::com::sun::star::uno::Sequence< sal_Int8 >& aInitVector, bool bEncrypt );

    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL convertWithCipherContext( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::DisposedException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL finalizeCipherContextAndDispose(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_PACKAGE_SOURCE_ZIPAPI_BLOWFISHCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
