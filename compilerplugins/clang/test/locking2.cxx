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

#include <mutex>
#include <memory>
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

namespace test2
{
struct Foo
{
    std::mutex m_aMutex;
    int m_foo;
    std::shared_ptr<Foo> m_foo2;

    int bar3()
    {
        std::scoped_lock guard(m_aMutex);
        // expected-error@+1 {{locked m_foo [loplugin:locking2]}}
        return m_foo;
    }
    std::shared_ptr<Foo> bar4()
    {
        std::scoped_lock guard(m_aMutex);
        // expected-error@+1 {{locked m_foo2 [loplugin:locking2]}}
        return m_foo2;
    }
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
