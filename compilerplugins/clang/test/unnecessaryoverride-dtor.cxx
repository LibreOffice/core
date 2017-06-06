/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <salhelper/simplereferenceobject.hxx>

#include <unnecessaryoverride-dtor.hxx>

struct NonVirtualBase {};

struct NonVirtualDerived1: NonVirtualBase {
    ~NonVirtualDerived1() {} // expected-error {{unnecessary user-declared destructor [loplugin:unnecessaryoverride]}}
};

struct NonVirtualDerived2: NonVirtualBase {
    virtual ~NonVirtualDerived2() {}
};

struct PrivateDerived: VirtualBase {
private:
    ~PrivateDerived() override {}
};

struct ProtectedDerived: VirtualBase {
protected:
    ~ProtectedDerived() override {}
};

IncludedDerived2::~IncludedDerived2() {}

struct Incomplete: salhelper::SimpleReferenceObject {};

IncludedDerived3::IncludedDerived3() {}

IncludedDerived3::~IncludedDerived3() {}

// vmiklos likes these because he can quickly add a DEBUG or something similar without
// massive recompile
IncludedNotDerived::~IncludedNotDerived() {}

struct NoExSpecDerived: VirtualBase {
    ~NoExSpecDerived() override {} // expected-error {{unnecessary user-declared destructor [loplugin:unnecessaryoverride]}}
};

struct NoThrowDerived: VirtualBase {
    ~NoThrowDerived() throw () override {} // expected-error {{unnecessary user-declared destructor [loplugin:unnecessaryoverride]}}
};

struct NoexceptDerived: VirtualBase {
    ~NoexceptDerived() noexcept override {} // expected-error {{unnecessary user-declared destructor [loplugin:unnecessaryoverride]}}
};

struct NoexceptTrueDerived: VirtualBase {
    ~NoexceptTrueDerived() noexcept(true) override {} // expected-error {{unnecessary user-declared destructor [loplugin:unnecessaryoverride]}}
};

#if 0
struct NoexceptFalseBase {
    virtual ~NoexceptFalseBase() noexcept(false) {}
};

struct NoexceptFalseDerived: NoexceptFalseBase {
    ~NoexceptFalseDerived() noexcept(false) override {}
};
#endif

struct NoDtorDerived: VirtualBase {};

struct DefaultDerived1: VirtualBase {
    ~DefaultDerived1() override = default; // expected-error {{unnecessary user-declared destructor [loplugin:unnecessaryoverride]}}
};

struct DefaultDerived2: VirtualBase {
    ~DefaultDerived2() override; // expected-note {{declared here [loplugin:unnecessaryoverride]}}
};

DefaultDerived2::~DefaultDerived2() = default; // expected-error {{unnecessary user-declared destructor [loplugin:unnecessaryoverride]}}

struct EmptyDerived1: VirtualBase {
    ~EmptyDerived1() override {}; // expected-error {{unnecessary user-declared destructor [loplugin:unnecessaryoverride]}}
};

struct EmptyDerived2: VirtualBase {
    ~EmptyDerived2() override; // expected-note {{declared here [loplugin:unnecessaryoverride]}}
};

EmptyDerived2::~EmptyDerived2() {} // expected-error {{unnecessary user-declared destructor [loplugin:unnecessaryoverride]}}

struct NonEmptyDerived: VirtualBase {
    ~NonEmptyDerived() override { (void) 0; }
};

struct CatchDerived: VirtualBase {
    ~CatchDerived() override try {} catch (...) {}
};

struct DeleteBase {
    virtual ~DeleteBase() = delete;
};

struct DeleteDerived: DeleteBase {
    ~DeleteDerived() override = delete;
};

struct PureBase {
    virtual ~PureBase() = 0;
};

struct PureDerived: PureBase {
    ~PureDerived() override {} // expected-error {{unnecessary user-declared destructor [loplugin:unnecessaryoverride]}}
};

struct CompleteBase {
    ~CompleteBase() {} // expected-error {{unnecessary user-declared destructor [loplugin:unnecessaryoverride]}}
};

// <sberg> noelgrandin, there's one other corner case one can imagine:
// a class defined in a .hxx with the dtor declared (but not defined) as inline in the .hxx,
// and then defined in the cxx (making it effectively only callable from within the cxx);
// removing the dtor declaration from the class definition would change the dtor to be callable from everywhere
MarkedInlineButNotDefined::~MarkedInlineButNotDefined() {}

// avoid loplugin:unreffun:
int main() {
    (void) NonVirtualDerived1();
    (void) DefaultDerived1();
    (void) CompleteBase();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
