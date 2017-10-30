/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <vcl/vclptr.hxx>

class OutputDevice;

struct Class1
{
    struct Impl {
        void foo_notconst();
        void foo_const() const;
        int & foo_both();
        int const & foo_both() const;
    };
    std::unique_ptr<Impl> pImpl;
    int* m_pint;
    VclPtr<OutputDevice> m_pvcl;

    void foo1() {
        pImpl->foo_notconst();
    }
    void foo2() { // expected-error {{this method can be const [loplugin:constmethod]}}
        pImpl->foo_const();
    }
    // TODO this should trigger a warning, but doesn't
    void foo3() {
        pImpl->foo_both();
    }
    Impl* foo4() {
        return pImpl.get(); // no warning expected
    }
    int* foo5() {
        return m_pint; // no warning expected
    }
    OutputDevice* foo6() {
        return m_pvcl; // no warning expected
    }
};
