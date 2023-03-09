/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <mutex>
#include <osl/mutex.hxx>

static std::mutex gSolarMutex;

class SolarMutexGuard
{
    std::unique_lock<std::mutex> lock;

public:
    SolarMutexGuard()
        : lock(gSolarMutex)
    {
    }
};

namespace test1
{
struct Foo
{
    int m_foo;
    int bar1()
    // expected-error@+1 {{unnecessary locking [loplugin:unnecessarylocking]}}
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
int free_function() { return 1; }

struct Foo
{
    std::mutex m_aMutex;
    osl::Mutex m_aOslMutex;
    int m_foo;

    int bar1()
    // expected-error@+1 {{unnecessary locking [loplugin:unnecessarylocking]}}
    {
        std::unique_lock guard(m_aMutex);
        return 1;
    }
    int bar2()
    // expected-error@+1 {{unnecessary locking [loplugin:unnecessarylocking]}}
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
    int bar4()
    // expected-error@+1 {{unnecessary locking [loplugin:unnecessarylocking]}}
    {
        ::osl::Guard<::osl::Mutex> aGuard(m_aOslMutex);
        return 1;
    }
    int bar5()
    {
        // expected-error@+1 {{unnecessary locking [loplugin:unnecessarylocking]}}
        {
            std::unique_lock guard(m_aMutex);
            return free_function();
        }
    }
    osl::Mutex& getOslMutex() { return m_aOslMutex; }
    int bar6()
    // expected-error@+1 {{unnecessary locking [loplugin:unnecessarylocking]}}
    {
        ::osl::Guard<::osl::Mutex> aGuard(getOslMutex());
        return 1;
    }
};
}

// Calling anything on VCLUnoHelper means we need the SolarMutex
class VCLUnoHelper
{
public:
    static int CreateToolkit();
};
namespace test4
{
// no warning expected
void bar1()
{
    SolarMutexGuard guard;
    VCLUnoHelper::CreateToolkit();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
