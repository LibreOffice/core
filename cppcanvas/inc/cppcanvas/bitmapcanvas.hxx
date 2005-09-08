/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bitmapcanvas.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:12:36 $
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

#ifndef _CPPCANVAS_BITMAPCANVAS_HXX
#define _CPPCANVAS_BITMAPCANVAS_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _BGFX_VECTOR_B2ISIZE_HXX
#include <basegfx/vector/b2isize.hxx>
#endif

#ifndef _CPPCANVAS_CANVAS_HXX
#include <cppcanvas/canvas.hxx>
#endif


/* Definition of BitmapCanvas */

namespace cppcanvas
{
    class BitmapCanvas;

    // forward declaration, since cloneBitmapCanvas() also references BitmapCanvas
    typedef ::boost::shared_ptr< BitmapCanvas > BitmapCanvasSharedPtr;

    /** BitmapCanvas interface
     */
    class BitmapCanvas : public virtual Canvas
    {
    public:
        virtual ::basegfx::B2ISize      getSize() const = 0;

        // shared_ptr does not allow for covariant return types
        BitmapCanvasSharedPtr           cloneBitmapCanvas() const
        {
            BitmapCanvasSharedPtr p( ::boost::dynamic_pointer_cast< BitmapCanvas >(this->clone()) );
            OSL_ENSURE(p.get(), "BitmapCanvas::cloneBitmapCanvas(): dynamic cast failed");
            return p;
        }
    };

}

#endif /* _CPPCANVAS_BITMAPCANVAS_HXX */
