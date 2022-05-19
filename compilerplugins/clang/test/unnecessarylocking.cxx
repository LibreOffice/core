/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <mutex>

static std::mutex gSolarMutex;

class SolarMutexGuard : public std::unique_lock<std::mutex>
{
public:
    SolarMutexGuard()
        : std::unique_lock<std::mutex>(gSolarMutex)
    {
    }
};

namespace test1
{
struct Foo
{
    int m_foo;
    // expected-error@+1 {{unnecessary locking [loplugin:unnecessarylocking]}}
    int bar1()
    {
        SolarMutexGuard guard;
        return 1;
    }
    // no warning expected
    int bar2()
    {
        SolarMutexGuard guard;
        return m_foo;
    }
};
}

namespace test2
{
struct Foo
{
    std::mutex m_aMutex;
    int m_foo;

    // expected-error@+1 {{unnecessary locking [loplugin:unnecessarylocking]}}
    int bar1()
    {
        std::unique_lock guard(m_aMutex);
        return 1;
    }
    // expected-error@+1 {{unnecessary locking [loplugin:unnecessarylocking]}}
    int bar2()
    {
        std::scoped_lock guard(m_aMutex);
        return 1;
    }
    // no warning expected
    int bar3()
    {
        std::scoped_lock guard(m_aMutex);
        return m_foo;
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
