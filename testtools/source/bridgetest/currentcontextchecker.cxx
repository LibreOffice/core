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

#include "currentcontextchecker.hxx"

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <cppu/unotype.hxx>
#include <cppuhelper/implbase.hxx>
#include <osl/diagnose.h>
#include <osl/diagnose.hxx>
#include <rtl/string.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <test/testtools/bridgetest/XCurrentContextChecker.hpp>
#include <uno/current_context.hxx>

namespace {

constexpr OUStringLiteral KEY = u"testtools.bridgetest.Key";
constexpr OUStringLiteral VALUE = u"good";

class CurrentContext:
    public ::osl::DebugBase< CurrentContext >,
    public ::cppu::WeakImplHelper< css::uno::XCurrentContext >
{
public:
    CurrentContext();

    CurrentContext(const CurrentContext&) = delete;
    CurrentContext& operator=(const CurrentContext&) = delete;

    virtual css::uno::Any SAL_CALL getValueByName(OUString const & Name) override;
};

CurrentContext::CurrentContext() {}

css::uno::Any CurrentContext::getValueByName(OUString const & Name)
{
    return Name == KEY ? css::uno::makeAny(OUString(VALUE)) : css::uno::Any();
}

}

testtools::bridgetest::CurrentContextChecker::CurrentContextChecker() {}

testtools::bridgetest::CurrentContextChecker::~CurrentContextChecker() {}

sal_Bool testtools::bridgetest::CurrentContextChecker::perform(
    css::uno::Reference<
        ::test::testtools::bridgetest::XCurrentContextChecker > const & other,
    ::sal_Int32 setSteps, ::sal_Int32 checkSteps)
{
    if (setSteps == 0) {
        css::uno::ContextLayer layer(new CurrentContext);
        return performCheck(other, setSteps, checkSteps);
    } else {
        return performCheck(other, setSteps, checkSteps);
    }
}

bool testtools::bridgetest::CurrentContextChecker::performCheck(
    css::uno::Reference<
        ::test::testtools::bridgetest::XCurrentContextChecker > const & other,
    ::sal_Int32 setSteps, ::sal_Int32 checkSteps)
{
    OSL_ASSERT(other.is() && checkSteps >= 0);
    if (checkSteps == 0) {
        css::uno::Reference< css::uno::XCurrentContext > context(
            css::uno::getCurrentContext());
        if (!context.is()) {
            return false;
        }
        css::uno::Any a(context->getValueByName(KEY));
        if (a.getValueType() != ::cppu::UnoType< OUString >::get()) {
            return false;
        }
        OUString s;
        OSL_VERIFY(a >>= s);
        return s == VALUE;
    } else {
        return other->perform(
            this, setSteps >= 0 ? setSteps - 1 : -1, checkSteps - 1);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
