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

#ifndef INCLUDED_CPPCANVAS_BITMAP_HXX
#define INCLUDED_CPPCANVAS_BITMAP_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <cppcanvas/canvasgraphic.hxx>
#include <cppcanvas/bitmapcanvas.hxx>
#include <memory>

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

        virtual css::uno::Reference< css::rendering::XBitmap >  getUNOBitmap() const = 0;
    };

    typedef std::shared_ptr< ::cppcanvas::Bitmap > BitmapSharedPtr;
}

#endif // INCLUDED_CPPCANVAS_BITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
