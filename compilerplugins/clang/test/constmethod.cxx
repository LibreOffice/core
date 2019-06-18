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
        int & foo_int_ref() const;
        int const & foo_const_int_ref() const;
        int * foo_int_ptr() const;
        int const * foo_const_int_ptr() const;
    };
    std::unique_ptr<Impl> pImpl;
    int* m_pint;
    VclPtr<OutputDevice> m_pvcl;

    void GetFoo1() {
        pImpl->foo_notconst();
    }
    void GetFoo2() {
        pImpl->foo_const();
    }
    int& GetFoo3() {
        return pImpl->foo_int_ref();
    }
    int const & GetFoo3a() { // expected-error {{this method can be const [loplugin:constmethod]}}
        return pImpl->foo_const_int_ref();
    }
    int* GetFoo3b() {
        return pImpl->foo_int_ptr();
    }
    int const * GetFoo3c() { // expected-error {{this method can be const [loplugin:constmethod]}}
        return pImpl->foo_const_int_ptr();
    }
    Impl* GetFoo4() {
        return pImpl.get(); // no warning expected
    }
    int* GetFoo5() {
        return m_pint; // no warning expected
    }
    int& GetFoo6() {
        return *m_pint; // no warning expected
    }
    OutputDevice* GetFoo7() {
        return m_pvcl; // no warning expected
    }
};
