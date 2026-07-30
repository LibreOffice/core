/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#pragma once

#include <sal/config.h>

#include "ViewState.hxx"
#include <com/sun/star/uno/XInterface.hdl>
#include <com/sun/star/uno/Reference.h>
#include <cppu/macros.hxx>
#include <cpo/uno/Type.hxx>
#include <cppu/unotype.hxx>
#include <sal/types.h>

#include <type_traits>

namespace cpo::uno
{
class Type;
}

namespace vclcanvas
{
/** Interface for cached repaint of already drawn XCanvas
    primitives.<p>

    This interface provides a method to quickly redraw some
    XCanvas primitives, using cached data.<p>
 */
class XCachedPrimitive : public ::css::uno::XInterface
{
public:
    XCachedPrimitive() = default;
    XCachedPrimitive(XCachedPrimitive const&) = default;
    XCachedPrimitive(XCachedPrimitive&&) = default;
    XCachedPrimitive& operator=(XCachedPrimitive const&) = default;
    XCachedPrimitive& operator=(XCachedPrimitive&&) = default;

    /** Redraw the cached primitive.<p>

        Redraw the cached primitive, with a possibly new view state.<p>

        @param aState
        The view state to be used when redrawing the primitive. The
        view state is of course permitted to differ from the one used
        for initial rendering.

        @return a value of type RepaintResult, denoting
        the result of this operation.

        @throws com::sun::star::lang::IllegalArgumentException
        if the view transformation matrix is singular.
     */
    virtual ::sal_Int8 redraw(const ::vclcanvas::ViewState& aState) = 0;

    static inline ::cpo::uno::Type const& static_type(void* = nullptr);

protected:
    ~XCachedPrimitive() noexcept {} // avoid warnings about virtual members and non-virtual dtor
};

inline ::cpo::uno::Type const&
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::vclcanvas::XCachedPrimitive const*)
{
    static typelib_TypeDescriptionReference* the_type = nullptr;
    if (!the_type)
    {
        typelib_static_type_init(&the_type, typelib_TypeClass_INTERFACE,
                                 "vclcanvas.XCachedPrimitive");
    }
    return *reinterpret_cast<::cpo::uno::Type*>(&the_type);
}
}

::cpo::uno::Type const& ::vclcanvas::XCachedPrimitive::static_type(SAL_UNUSED_PARAMETER void*)
{
    return ::cppu::UnoType<::vclcanvas::XCachedPrimitive>::get();
}

namespace cppu::detail
{
template <> struct IsUnoInterfaceType<::vclcanvas::XCachedPrimitive> : ::std::true_type
{
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
