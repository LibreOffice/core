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

#include <emscripten/bind.h>

#include <com/sun/star/uno/Reference.hxx>
#include <sal/types.h>

template <typename T> struct emscripten::smart_ptr_trait<css::uno::Reference<T>>
{
    using PointerType = css::uno::Reference<T>;
    using element_type = T;
    static T* get(css::uno::Reference<T> const& ptr) { return ptr.get(); }
    static sharing_policy get_sharing_policy() { return sharing_policy::INTRUSIVE; }
    static css::uno::Reference<T>* share(T* v) { return new css::uno::Reference<T>(v); }
    static css::uno::Reference<T>* construct_null() { return new css::uno::Reference<T>(); }
};

namespace unoembindhelpers
{
enum class uno_Reference
{
    FromAny
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
