/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CANVAS_SOURCE_OPENGL_OGL_CANVASCUSTOMSPRITE_HXX
#define INCLUDED_CANVAS_SOURCE_OPENGL_OGL_CANVASCUSTOMSPRITE_HXX

#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/rendering/XCustomSprite.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <canvas/base/basemutexhelper.hxx>

#include "ogl_spritecanvas.hxx"
#include "ogl_canvashelper.hxx"


namespace oglcanvas
{
    typedef ::cppu::WeakComponentImplHelper< css::rendering::XCustomSprite,
                                             css::rendering::XCanvas > CanvasCustomSpriteBase_Base;
    typedef ::canvas::CanvasBase<
         ::canvas::BaseMutexHelper< CanvasCustomSpriteBase_Base >,
         CanvasHelper,
         ::osl::MutexGuard,
         ::cppu::OWeakObject >                          CanvasCustomSpriteBaseT;

    /* Definition of CanvasCustomSprite class */

    class CanvasCustomSprite : public CanvasCustomSpriteBaseT
    {
    public:
        /** Create a custom sprite

            @param rSpriteSize
            Size of the sprite in pixel

            @param rRefDevice
            Associated output device

            @param rSpriteCanvas
            Target canvas

            @param rDevice
            Target DX device
         */
        CanvasCustomSprite( const css::geometry::RealSize2D&                rSpriteSize,
                            const SpriteCanvasRef&                          rRefDevice,
                            SpriteDeviceHelper&                             rDeviceHelper );

        virtual void disposeThis() override;

        // XSprite
        virtual void SAL_CALL setAlpha( double alpha ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL move( const css::geometry::RealPoint2D&  aNewPos, const css::rendering::ViewState&  viewState, const css::rendering::RenderState& renderState ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL transform( const css::geometry::AffineMatrix2D& aTransformation ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL clip( const css::uno::Reference< css::rendering::XPolyPolygon2D >& aClip ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPriority( double nPriority ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL show() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL hide() throw (css::uno::RuntimeException, std::exception) override;

        // XCustomSprite
        virtual css::uno::Reference< css::rendering::XCanvas > SAL_CALL getContentCanvas() throw (css::uno::RuntimeException, std::exception) override;

        double getPriority() const { return mfPriority; }

        /// Render sprite content at sprite position
        bool renderSprite() const;

    private:
        /** MUST hold here, too, since CanvasHelper only contains a
            raw pointer (without refcounting)
        */
        SpriteCanvasRef                              mpSpriteCanvas;
        const css::geometry::RealSize2D              maSize;

        css::uno::Reference< css::rendering::XPolyPolygon2D > mxClip;
        css::geometry::AffineMatrix2D                maTransformation;
        ::basegfx::B2DPoint                          maPosition;
        double                                       mfAlpha;
        double                                       mfPriority;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
