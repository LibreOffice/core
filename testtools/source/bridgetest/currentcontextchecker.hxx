/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_TESTTOOLS_SOURCE_BRIDGETEST_CURRENTCONTEXTCHECKER_HXX
#define INCLUDED_TESTTOOLS_SOURCE_BRIDGETEST_CURRENTCONTEXTCHECKER_HXX

#include "sal/config.h"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "cppuhelper/implbase1.hxx"
#ifndef OSL_DIAGNOSE_HXX_INCLUDED
#include "osl/diagnose.hxx"
#endif
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
