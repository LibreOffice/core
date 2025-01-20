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
#include <sal/types.h>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/threadpool.hxx>
#include "setupvcl.hxx"

namespace {

class Protector: public CppUnit::Protector {
public:
    Protector() { test::setUpVcl(); }
    Protector(const Protector&) = delete;
    Protector& operator=(const Protector&) = delete;

private:
    virtual ~Protector() override {
        DeInitVCL();
        // for the 6 tests that use it
        comphelper::ThreadPool::getSharedOptimalPool().shutdown();
    }

    virtual bool protect(
        CppUnit::Functor const & functor, CppUnit::ProtectorContext const &)
        override
    { return functor(); }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT CppUnit::Protector *
vclbootstrapprotector() {
    return new Protector;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
