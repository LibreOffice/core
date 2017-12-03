/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/Protector.h>
#include <cppunittester/protectorfactory.hxx>
#include <sal/types.h>
#include <test/setupvcl.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/threadpool.hxx>

#include "isheadless.hxx"

namespace {

class Protector: public CppUnit::Protector {
public:
    Protector() { test::setUpVcl(); }
    Protector(const Protector&) = delete;
    Protector& operator=(const Protector&) = delete;

private:
    virtual ~Protector() override {
#if defined(__COVERITY__)
        try {
#endif
            DeInitVCL();
            // for the 6 tests that use it
            comphelper::ThreadPool::getSharedOptimalPool().shutdown();
#if defined(__COVERITY__)
        } catch (const std::exception& e) {
            SAL_WARN("vcl.app", "Fatal exception: " << e.what());
            std::terminate();
        }
#endif
    }

    virtual bool protect(
        CppUnit::Functor const & functor, CppUnit::ProtectorContext const &)
        override
    { return functor(); }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT CppUnit::Protector * SAL_CALL
vclbootstrapprotector() {
    return new Protector;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
