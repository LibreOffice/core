/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bitmap.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _CPPCANVAS_BITMAP_HXX
#define _CPPCANVAS_BITMAP_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <boost/shared_ptr.hpp>
#include <cppcanvas/canvasgraphic.hxx>
#include <cppcanvas/bitmapcanvas.hxx>

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XBitmap;
} } } }


/* Definition of Bitmap interface */

namespace cppcanvas
{

    /** This interface defines a Bitmap canvas object

        Consider this object part of the view, and not of the model
        data, as this bitmap can only be painted on its parent canvas
     */
    class Bitmap : public virtual CanvasGraphic
    {
    public:
        /** Render to parent canvas, with global alpha.

            This method renders the content to the parent canvas,
            i.e. the canvas this object was constructed for.

            @param nAlphaModulation
            Global alpha value, with which each pixel alpha value gets
            multiplied. For a normal, opaque bitmap, this will make
            the bitmap appear transparent with the given alpha value
            (value must be in the range [0,1]).

            @return whether the rendering finished successfully.
         */
        virtual bool                        drawAlphaModulated( double nAlphaModulation ) const = 0;

        virtual BitmapCanvasSharedPtr       getBitmapCanvas() const = 0;

        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XBitmap >  getUNOBitmap() const = 0;
    };

    typedef ::boost::shared_ptr< ::cppcanvas::Bitmap > BitmapSharedPtr;
}

#endif /* _CPPCANVAS_BITMAP_HXX */
