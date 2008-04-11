/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: make_shared_from_uno.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#if ! defined(INCLUDED_COMPHELPER_MAKE_SHARED_FROM_UNO_HXX)
#define INCLUDED_COMPHELPER_MAKE_SHARED_FROM_UNO_HXX

#include "boost/shared_ptr.hpp"
#include <functional>

namespace comphelper {

/// @internal
namespace detail {
/// @internal
template <typename T> struct ReleaseFunc : ::std::unary_function<T *, void> {
    void operator()( T * p ) const { p->release(); }
};
} // namespace detail

/** Makes a boost::shared_ptr from a ref-counted UNO object pointer.
    This makes sense if the object is used via UNO (implementing some X
    interface) and also internally using its implementation class, e.g.

    <pre>
        boost::shared_ptr<MyUnoImpl> const ptr(
            comphelper::make_shared_from_UNO( new MyUnoImpl ) );
        ...
        xUno->callingUno( uno::Reference<XSomeInterface>( ptr.get() ) );
        ...
        takeSharedPtr( ptr );
        ...
    </pre>

    @attention The shared_ptr operates on a separate reference counter, so
               weak pointers (boost::weak_ptr) are invalidated when the last
               shared_ptr is destroyed, although the UNO object may still be
               alive.

    @param p object pointer
    @return shared_ptr to object
*/
template <typename T>
inline ::boost::shared_ptr<T> make_shared_from_UNO( T * p )
{
    p->acquire();
    return ::boost::shared_ptr<T>( p, detail::ReleaseFunc<T>() );
}

} // namespace comphelper

#endif // ! defined(INCLUDED_COMPHELPER_MAKE_SHARED_FROM_UNO_HXX)

