/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <vcl/vclptr.hxx>
#include <vcl/vclreferencebase.hxx>

// Minimal VirtualDevice stub — the plugin checks for Class("VirtualDevice").GlobalNamespace()
class VirtualDevice : public VclReferenceBase
{
public:
    void dispose() override { VclReferenceBase::dispose(); }
    ~VirtualDevice() override { disposeOnce(); }
};

// --- Cases that SHOULD warn (local variable) ---

// Local VclPtr<VirtualDevice> created via ::Create(), not disposed, not returned
void bad_local()
{
    VclPtr<VirtualDevice>
        pVDev // expected-error {{use ScopedVclPtr<VirtualDevice> instead of VclPtr<VirtualDevice> for local variables to prevent GDI handle leaks [loplugin:scopedvclptr]}}
        = VclPtr<VirtualDevice>::Create();
    (void)pVDev;
}

// --- Cases that SHOULD warn (return type) ---

// Function returning VclPtr<VirtualDevice> should return ScopedVclPtr
VclPtr<VirtualDevice>
bad_return_type() // expected-error {{use ScopedVclPtr<VirtualDevice> as return type instead of VclPtr<VirtualDevice> to prevent GDI handle leaks [loplugin:scopedvclptr]}}
{
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    return pVDev;
}

// --- Cases that should NOT warn ---

// ScopedVclPtr local is fine
void good_scoped()
{
    ScopedVclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    (void)pVDev;
}

// Factory returning ScopedVclPtr: no warnings at all
ScopedVclPtr<VirtualDevice> good_factory_scoped()
{
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    return pVDev;
}

// Using VclPtr in return expression is NOT the same as returning it
VirtualDevice* bad_use_in_return()
{
    VclPtr<VirtualDevice>
        pVDev // expected-error {{use ScopedVclPtr<VirtualDevice> instead of VclPtr<VirtualDevice> for local variables to prevent GDI handle leaks [loplugin:scopedvclptr]}}
        = VclPtr<VirtualDevice>::Create();
    return pVDev.get();
}

// Explicit disposeAndClear: developer manages lifecycle manually
void good_explicit_dispose()
{
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    (void)pVDev;
    pVDev.disposeAndClear();
}

// Borrowing: initialized from a function call, not VclPtr::Create()
VclPtr<VirtualDevice> GetSharedDevice();
void good_borrow()
{
    VclPtr<VirtualDevice> pVDev = GetSharedDevice();
    (void)pVDev;
}

// No initializer: variable declared but not created via ::Create()
void good_no_init()
{
    VclPtr<VirtualDevice> pVDev;
    (void)pVDev;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
