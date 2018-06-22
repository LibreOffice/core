/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <rtl/ref.hxx>

struct Foo
{
    void acquire();
    void release();
};
class Foo16
{
    std::unique_ptr<int> m_pbar1;
    rtl::Reference<Foo> m_pbar2;
    Foo16()
        : m_pbar1(nullptr)
        // expected-error@-1 {{no need to explicitly init this with nullptr, just use default constructor [loplugin:simplifyconstruct]}}
        , m_pbar2(nullptr)
    // expected-error@-1 {{no need to explicitly init this with nullptr, just use default constructor [loplugin:simplifyconstruct]}}
    {
    }
};
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
