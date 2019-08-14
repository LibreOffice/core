/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <utility>

namespace test1
{
void foo(css::uno::Sequence<css::uno::Reference<css::uno::XInterface>>& aSeq)
{
    // expected-error@+1 {{use std::as_const, or make range var const, to avoid creating a copy of the Sequence [loplugin:sequenceloop]}}
    for (const auto& x : aSeq)
        x.get();
    // no warning expected
    for (auto& x : aSeq)
        x.get();
    for (const auto& x : std::as_const(aSeq))
        x.get();
}
// no warning expected
void foo2(const css::uno::Sequence<css::uno::Reference<css::uno::XInterface>>& aSeq)
{
    for (const auto& x : aSeq)
        x.get();
}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
