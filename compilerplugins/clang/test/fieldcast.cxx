/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if defined _WIN32 //TODO, #include <sys/file.h>
// expected-no-diagnostics
#else

#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <memory>

struct Foo
{
    virtual ~Foo();
};
struct Bar : public Foo
{
};

class Test1
{
    // expected-error@+1 {{cast Bar [loplugin:fieldcast]}}
    Foo* m_p;
    void test1() { (void)dynamic_cast<Bar*>(m_p); }
};

class Test2
{
    // expected-error@+1 {{cast Bar [loplugin:fieldcast]}}
    rtl::Reference<Foo> m_p;
    void test1() { (void)dynamic_cast<Bar*>(m_p.get()); }
};

class Test3
{
    // no warning expected, casting to a less specific type
    rtl::Reference<Bar> m_p;
    void test1() { (void)static_cast<Foo*>(m_p.get()); }
};

class Test4
{
    // expected-error@+1 {{cast Bar [loplugin:fieldcast]}}
    std::unique_ptr<Foo> m_p;
    void test1() { (void)dynamic_cast<Bar*>(m_p.get()); }
};

class Test5
{
    // expected-error@+1 {{cast Bar [loplugin:fieldcast]}}
    std::shared_ptr<Foo> m_p;
    void test1() { (void)dynamic_cast<Bar*>(m_p.get()); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
