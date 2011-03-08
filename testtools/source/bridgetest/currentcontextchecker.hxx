/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_TESTTOOLS_SOURCE_BRIDGETEST_CURRENTCONTEXTCHECKER_HXX
#define INCLUDED_TESTTOOLS_SOURCE_BRIDGETEST_CURRENTCONTEXTCHECKER_HXX

#include "sal/config.h"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "cppuhelper/implbase1.hxx"
#include "osl/diagnose.hxx"
#include "sal/types.h"
#include "test/testtools/bridgetest/XCurrentContextChecker.hpp"

namespace testtools { namespace bridgetest {

class CurrentContextChecker:
    public ::osl::DebugBase< CurrentContextChecker >,
    public ::cppu::WeakImplHelper1<
        ::test::testtools::bridgetest::XCurrentContextChecker >
{
public:
    CurrentContextChecker();

    virtual ~CurrentContextChecker();

    virtual ::sal_Bool SAL_CALL perform(
        ::com::sun::star::uno::Reference<
            ::test::testtools::bridgetest::XCurrentContextChecker > const &
            other,
        ::sal_Int32 setSteps, ::sal_Int32 checkSteps)
        throw (::com::sun::star::uno::RuntimeException);

private:
    CurrentContextChecker(CurrentContextChecker &); // not defined
    void operator =(CurrentContextChecker &); // not defined

    bool performCheck(
        ::com::sun::star::uno::Reference<
            ::test::testtools::bridgetest::XCurrentContextChecker > const &
            other,
        ::sal_Int32 setSteps, ::sal_Int32 checkSteps);
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
