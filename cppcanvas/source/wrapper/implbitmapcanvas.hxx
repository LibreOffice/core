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

#ifndef INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLBITMAPCANVAS_HXX
#define INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLBITMAPCANVAS_HXX

#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>

#include <boost/shared_ptr.hpp>
#include <basegfx/vector/b2dsize.hxx>
#include <cppcanvas/bitmapcanvas.hxx>

#include <implcanvas.hxx>


namespace cppcanvas
{
    namespace internal
    {
        // share Canvas implementation from ImplCanvas
        class ImplBitmapCanvas : public virtual BitmapCanvas, protected virtual ImplCanvas
        {
        public:
            ImplBitmapCanvas( const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::rendering::XBitmapCanvas >& rCanvas );
            virtual ~ImplBitmapCanvas();

            virtual ::basegfx::B2ISize      getSize() const SAL_OVERRIDE;

            virtual CanvasSharedPtr         clone() const SAL_OVERRIDE;

            // take compiler-provided default copy constructor
            //ImplBitmapCanvas(const ImplBitmapCanvas&);

        private:
            // default: disabled assignment
            ImplBitmapCanvas& operator=( const ImplBitmapCanvas& );

            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas >    mxBitmapCanvas;
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >          mxBitmap;
        };
    }
}

#endif // INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLBITMAPCANVAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
