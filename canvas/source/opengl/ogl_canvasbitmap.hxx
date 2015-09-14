/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CANVAS_SOURCE_OPENGL_OGL_CANVASBITMAP_HXX
#define INCLUDED_CANVAS_SOURCE_OPENGL_OGL_CANVASBITMAP_HXX

#include <cppuhelper/compbase.hxx>

#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <canvas/base/integerbitmapbase.hxx>
#include <canvas/base/basemutexhelper.hxx>
#include <basegfx/vector/b2isize.hxx>

#include "ogl_bitmapcanvashelper.hxx"
#include "ogl_spritecanvas.hxx"


/* Definition of CanvasBitmap class */

namespace oglcanvas
{
    typedef ::cppu::WeakComponentImplHelper< css::rendering::XBitmapCanvas,
                                             css::rendering::XIntegerBitmap > CanvasBitmapBase_Base;
    typedef ::canvas::IntegerBitmapBase<
        canvas::BitmapCanvasBase2<
            ::canvas::BaseMutexHelper< CanvasBitmapBase_Base >,
            BitmapCanvasHelper,
            ::osl::MutexGuard,
            ::cppu::OWeakObject> > CanvasBitmapBaseT;

    class CanvasBitmap : public CanvasBitmapBaseT
    {
    public:
        /** Create a canvas bitmap for the given surface

            @param rSize
            Size of the bitmap

            @param rDevice
            Reference device, with which bitmap should be compatible
         */
        CanvasBitmap( const css::geometry::IntegerSize2D&              rSize,
                      const SpriteCanvasRef&                           rDevice,
                      SpriteDeviceHelper&                              rDeviceHelper,
                      bool                                             bHasAlpha );

        /** Create verbatim copy (including all recorded actions)
         */
        CanvasBitmap( const CanvasBitmap& rSrc );

        /// Dispose all internal references
        virtual void disposeThis() SAL_OVERRIDE;

        /** Write out recorded actions
         */
        bool renderRecordedActions() const;

    private:
        /** MUST hold here, too, since CanvasHelper only contains a
            raw pointer (without refcounting)
        */
        SpriteCanvasRef mpDevice;
        bool            mbHasAlpha;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
