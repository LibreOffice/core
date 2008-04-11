/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bitmapcanvas.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _CPPCANVAS_BITMAPCANVAS_HXX
#define _CPPCANVAS_BITMAPCANVAS_HXX

#include <sal/types.h>
#include <osl/diagnose.h>

#include <boost/shared_ptr.hpp>
#include <basegfx/vector/b2isize.hxx>
#include <cppcanvas/canvas.hxx>


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
