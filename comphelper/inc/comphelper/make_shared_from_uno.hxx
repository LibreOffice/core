/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: make_shared_from_uno.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-10-11 08:57:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

