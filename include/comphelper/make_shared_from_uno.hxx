/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
#ifndef INCLUDED_COMPHELPER_MAKE_SHARED_FROM_UNO_HXX
#define INCLUDED_COMPHELPER_MAKE_SHARED_FROM_UNO_HXX

#include <functional>
#include <memory>

namespace comphelper {

/// @internal
namespace detail {
/// @internal
template <typename T> struct ReleaseFunc : ::std::unary_function<T *, void> {
    void operator()( T * p ) const { p->release(); }
};
} // namespace detail

/** Makes a std::shared_ptr from a ref-counted UNO object pointer.
    This makes sense if the object is used via UNO (implementing some X
    interface) and also internally using its implementation class, e.g.

    <pre>
        std::shared_ptr<MyUnoImpl> const ptr(
            comphelper::make_shared_from_UNO( new MyUnoImpl ) );
        ...
        xUno->callingUno( uno::Reference<XSomeInterface>( ptr.get() ) );
        ...
        takeSharedPtr( ptr );
        ...
    </pre>

    @attention The shared_ptr operates on a separate reference counter, so
               weak pointers (std::weak_ptr) are invalidated when the last
               shared_ptr is destroyed, although the UNO object may still be
               alive.

    @param p object pointer
    @return shared_ptr to object
*/
template <typename T>
inline std::shared_ptr<T> make_shared_from_UNO( T * p )
{
    p->acquire();
    return std::shared_ptr<T>( p, detail::ReleaseFunc<T>() );
}

} // namespace comphelper

#endif // ! defined(INCLUDED_COMPHELPER_MAKE_SHARED_FROM_UNO_HXX)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
