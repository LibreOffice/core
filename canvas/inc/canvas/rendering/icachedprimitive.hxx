/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: icachedprimitive.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:44:56 $
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

#ifndef INCLUDED_CANVAS_ICACHEDPRIMITIVE_HXX
#define INCLUDED_CANVAS_ICACHEDPRIMITIVE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <boost/shared_ptr.hpp>


namespace basegfx
{
    class B2IRange;
    class B2IPoint;
}

namespace canvas
{
    /** Objects with this interface are returned from every Bitmap
        render operation.

        These objects can be used to implement the
        rendering::XCachedPrimitive interface, which in turn caches
        render state and objects to facilitate quick redraws.
     */
    struct ICachedPrimitive
    {
        virtual ~ICachedPrimitive() {}

        /** Redraw the primitive with the given view state

            Note that the primitive will <em>always</em> be redrawn on
            the bitmap it was created from.
         */
        virtual sal_Int8 redraw( const ::com::sun::star::rendering::ViewState& aState ) const = 0;
    };

    typedef ::boost::shared_ptr< ICachedPrimitive > ICachedPrimitiveSharedPtr;
}

#endif /* INCLUDED_CANVAS_ICACHEDPRIMITIVE_HXX */
