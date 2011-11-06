/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
