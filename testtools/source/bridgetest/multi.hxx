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

#ifndef INCLUDED_testtools_source_bridgetest_multi_hxx
#define INCLUDED_testtools_source_bridgetest_multi_hxx

#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "cppuhelper/implbase1.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "test/testtools/bridgetest/XMulti.hpp"

namespace testtools { namespace bridgetest {

class Multi: public cppu::WeakImplHelper1< test::testtools::bridgetest::XMulti >
{
public:
    Multi(): m_attribute1(0.0), m_attribute3(0.0) {}

    virtual double SAL_CALL getatt1()
        throw (com::sun::star::uno::RuntimeException)
    { return m_attribute1; }

    virtual void SAL_CALL setatt1(double value)
        throw (com::sun::star::uno::RuntimeException)
    { m_attribute1 = value; }

    virtual sal_Int32 SAL_CALL fn11(sal_Int32 arg)
        throw (com::sun::star::uno::RuntimeException)
    { return 11 * arg; }

    virtual OUString SAL_CALL fn12(OUString const & arg)
        throw (com::sun::star::uno::RuntimeException)
    { return OUString("12") + arg; }

    virtual sal_Int32 SAL_CALL fn21(sal_Int32 arg)
        throw (com::sun::star::uno::RuntimeException)
    { return 21 * arg; }

    virtual OUString SAL_CALL fn22(OUString const & arg)
        throw (com::sun::star::uno::RuntimeException)
    { return OUString("22") + arg; }

    virtual double SAL_CALL getatt3()
        throw (com::sun::star::uno::RuntimeException)
    { return m_attribute3; }

    virtual void SAL_CALL setatt3(double value)
        throw (com::sun::star::uno::RuntimeException)
    { m_attribute3 = value; }

    virtual sal_Int32 SAL_CALL fn31(sal_Int32 arg)
        throw (com::sun::star::uno::RuntimeException)
    { return 31 * arg; }

    virtual OUString SAL_CALL fn32(OUString const & arg)
        throw (com::sun::star::uno::RuntimeException)
    { return OUString("32") + arg; }

    virtual sal_Int32 SAL_CALL fn33()
        throw (com::sun::star::uno::RuntimeException)
    { return 33; }

    virtual sal_Int32 SAL_CALL fn41(sal_Int32 arg)
        throw (com::sun::star::uno::RuntimeException)
    { return 41 * arg; }

    virtual sal_Int32 SAL_CALL fn61(sal_Int32 arg)
        throw (com::sun::star::uno::RuntimeException)
    { return 61 * arg; }

    virtual OUString SAL_CALL fn62(OUString const & arg)
        throw (com::sun::star::uno::RuntimeException)
    { return OUString("62") + arg; }

    virtual sal_Int32 SAL_CALL fn71(sal_Int32 arg)
        throw (com::sun::star::uno::RuntimeException)
    { return 71 * arg; }

    virtual OUString SAL_CALL fn72(OUString const & arg)
        throw (com::sun::star::uno::RuntimeException)
    { return OUString("72") + arg; }

    virtual sal_Int32 SAL_CALL fn73()
        throw (com::sun::star::uno::RuntimeException)
    { return 73; }

private:
    double m_attribute1;
    double m_attribute3;
};

OUString testMulti(
    com::sun::star::uno::Reference< test::testtools::bridgetest::XMulti >
    const & multi);

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
