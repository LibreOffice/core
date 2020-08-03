/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <rtl/ref.hxx>

struct VirtualBase {
    virtual ~VirtualBase() {}
};

struct IncludedDerived1: VirtualBase {
    ~IncludedDerived1() override {}; // expected-error {{unnecessary user-declared destructor [loplugin:unnecessaryoverride]}}
};

struct IncludedDerived2: VirtualBase {
    ~IncludedDerived2() override;
};

struct IncludedNotDerived {
    ~IncludedNotDerived();
};

struct Incomplete;
struct IncludedDerived3: VirtualBase {
    IncludedDerived3();
    ~IncludedDerived3() override;

private:
    IncludedDerived3(IncludedDerived3 &) = delete;
    void operator =(IncludedDerived3) = delete;

    rtl::Reference<Incomplete> m;
};

struct MarkedInlineButNotDefined {
    inline ~MarkedInlineButNotDefined();
};

template<typename T> struct TemplateBase: T {
    virtual ~TemplateBase() {}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
