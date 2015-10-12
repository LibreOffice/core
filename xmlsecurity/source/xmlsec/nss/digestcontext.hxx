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

#ifndef INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_DIGESTCONTEXT_HXX
#define INCLUDED_XMLSECURITY_SOURCE_XMLSEC_NSS_DIGESTCONTEXT_HXX

#include <com/sun/star/xml/crypto/XDigestContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>

class ODigestContext : public cppu::WeakImplHelper< ::com::sun::star::xml::crypto::XDigestContext >
{
private:
    ::osl::Mutex m_aMutex;

    PK11Context* m_pContext;
    sal_Int32 m_nDigestLength;
    bool m_b1KData;
    sal_Int32 m_nDigested;

    bool m_bDisposed;
    bool m_bBroken;

public:
    ODigestContext( PK11Context* pContext, sal_Int32 nDigestLength, bool b1KData )
    : m_pContext( pContext )
    , m_nDigestLength( nDigestLength )
    , m_b1KData( b1KData )
    , m_nDigested( 0 )
    , m_bDisposed( false )
    , m_bBroken( false )
    {}

    virtual ~ODigestContext();


    // XDigestContext
    virtual void SAL_CALL updateDigest( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL finalizeDigestAndDispose() throw (::com::sun::star::lang::DisposedException, ::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
