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
#ifndef INCLUDED_PACKAGE_SOURCE_ZIPAPI_SHA1CONTEXT_HXX
#define INCLUDED_PACKAGE_SOURCE_ZIPAPI_SHA1CONTEXT_HXX

#include <com/sun/star/xml/crypto/XDigestContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>

class StarOfficeSHA1DigestContext
    : public cppu::WeakImplHelper<css::xml::crypto::XDigestContext>
{
    ::osl::Mutex m_aMutex;
    void* m_pDigest;

    StarOfficeSHA1DigestContext()
    : m_pDigest( nullptr )
    {}

public:

    virtual ~StarOfficeSHA1DigestContext() override;

    static css::uno::Reference< css::xml::crypto::XDigestContext > Create();

    virtual void SAL_CALL updateDigest( const css::uno::Sequence< ::sal_Int8 >& aData ) override;
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL finalizeDigestAndDispose() override;

};

class CorrectSHA1DigestContext
    : public cppu::WeakImplHelper<css::xml::crypto::XDigestContext>
{
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;

    CorrectSHA1DigestContext();

public:

    virtual ~CorrectSHA1DigestContext() override;

    static css::uno::Reference<css::xml::crypto::XDigestContext> Create();

    virtual void SAL_CALL updateDigest(const css::uno::Sequence<::sal_Int8>& rData) override;
    virtual css::uno::Sequence<::sal_Int8> SAL_CALL finalizeDigestAndDispose() override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
