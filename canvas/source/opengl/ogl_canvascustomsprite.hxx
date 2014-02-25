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

#include <cppuhelper/compbase2.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/rendering/XCustomSprite.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <canvas/base/disambiguationhelper.hxx>

#include "ogl_spritecanvas.hxx"
#include "ogl_canvashelper.hxx"


namespace oglcanvas
{
    typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::rendering::XCustomSprite,
                                              ::com::sun::star::rendering::XCanvas > CanvasCustomSpriteBase_Base;
    typedef ::canvas::CanvasBase<
         ::canvas::DisambiguationHelper< CanvasCustomSpriteBase_Base >,
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
        CanvasCustomSprite( const ::com::sun::star::geometry::RealSize2D&   rSpriteSize,
                            const SpriteCanvasRef&                          rRefDevice,
                            SpriteDeviceHelper&                             rDeviceHelper );

        virtual void disposeThis();

        // XSprite
        virtual void SAL_CALL setAlpha( double alpha ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL move( const ::com::sun::star::geometry::RealPoint2D&  aNewPos, const ::com::sun::star::rendering::ViewState&  viewState, const ::com::sun::star::rendering::RenderState& renderState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL transform( const ::com::sun::star::geometry::AffineMatrix2D& aTransformation ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL clip( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >& aClip ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setPriority( double nPriority ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL show() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL hide() throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XCustomSprite
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas > SAL_CALL getContentCanvas() throw (::com::sun::star::uno::RuntimeException, std::exception);

        double getPriority() const { return mfPriority; }

        /// Render sprite content at sprite position
        bool renderSprite() const;

    private:
        /** MUST hold here, too, since CanvasHelper only contains a
            raw pointer (without refcounting)
        */
        SpriteCanvasRef                              mpSpriteCanvas;
        const ::com::sun::star::geometry::RealSize2D maSize;

        ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XPolyPolygon2D > mxClip;
        ::com::sun::star::geometry::AffineMatrix2D        maTransformation;
        ::basegfx::B2DPoint                               maPosition;
        double                                            mfAlpha;
        double                                            mfPriority;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
