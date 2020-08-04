/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cppuhelper/compbase.hxx>

#include <com/sun/star/rendering/XCustomSprite.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>

#include <basegfx/point/b2dpoint.hxx>

#include <base/basemutexhelper.hxx>

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
        virtual void SAL_CALL setAlpha( double alpha ) override;
        virtual void SAL_CALL move( const css::geometry::RealPoint2D&  aNewPos, const css::rendering::ViewState&  viewState, const css::rendering::RenderState& renderState ) override;
        virtual void SAL_CALL transform( const css::geometry::AffineMatrix2D& aTransformation ) override;
        virtual void SAL_CALL clip( const css::uno::Reference< css::rendering::XPolyPolygon2D >& aClip ) override;
        virtual void SAL_CALL setPriority( double nPriority ) override;
        virtual void SAL_CALL show() override;
        virtual void SAL_CALL hide() override;

        // XCustomSprite
        virtual css::uno::Reference< css::rendering::XCanvas > SAL_CALL getContentCanvas() override;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
