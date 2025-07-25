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

#pragma once

#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XCustomSprite.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <basegfx/point/b2dpoint.hxx>

#include <base/basemutexhelper.hxx>
#include <base/canvascustomspritebase.hxx>

#include <memory>
#include <vcl/cairo.hxx>

#include "cairo_sprite.hxx"
#include "cairo_canvashelper.hxx"
#include "cairo_repainttarget.hxx"
#include "cairo_spritehelper.hxx"
#include "cairo_spritecanvas.hxx"
#include "vcl_canvas/canvas.hxx"
#include "vcl_canvas/customsprite.hxx"


namespace vcl_cairocanvas
{
    /* typedef ::cppu::WeakComponentImplHelper< css::rendering::XCustomSprite,
                                             css::rendering::XBitmapCanvas,
                                             css::rendering::XIntegerBitmap,
                                             css::lang::XServiceInfo >  CanvasCustomSpriteBase_Base; */

    class CanvasCustomSpriteSpriteBase_Base : public ::vcl_canvas::CustomSprite,
                                              public ::vcl_canvas::Canvas,
                                              public Sprite,
                                              public SurfaceProvider
    {
    protected:
        mutable osl::Mutex m_aMutex;
    };

    typedef ::vcl_canvas::CanvasCustomSpriteBase< CanvasCustomSpriteSpriteBase_Base,
                                              SpriteHelper,
                                              CanvasHelper,
                                              ::osl::MutexGuard,
                                              ::cppu::OWeakObject >                     CanvasCustomSpriteBaseT;

    /* Definition of CanvasCustomSprite class */

    class CanvasCustomSprite : public CanvasCustomSpriteBaseT,
                               public RepaintTarget,
                               public std::enable_shared_from_this<CanvasCustomSprite>
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
        CanvasCustomSprite( const css::geometry::RealSize2D&   rSpriteSize,
                            const SpriteCanvasSharedPtr&       rRefDevice );

        virtual void disposeThis() override;
        virtual void dispose() override;
        std::shared_ptr<CanvasCustomSpriteBaseT> getShared() override
        {
            return shared_from_this();
        }

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname           Base doing refcount          Base implementing the XComponent interface
        //                                          |                    |                         |
        //                                          V                    V                         V
        // DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( CanvasCustomSprite, CanvasCustomSpriteBase_Base, ::cppu::WeakComponentImplHelperBase )

        // XServiceInfo
        /* virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override; */

        // Sprite
        virtual void redraw( const ::cairo::CairoSharedPtr& pCairo,
                             bool                           bBufferedUpdate ) const override;
        virtual void redraw( const ::cairo::CairoSharedPtr& pCairo,
                             const ::basegfx::B2DPoint&     rOrigOutputPos,
                             bool                           bBufferedUpdate ) const override;

        // RepaintTarget
        virtual bool repaint( const ::cairo::SurfaceSharedPtr&                pSurface,
                              const css::rendering::ViewState&   viewState,
                              const css::rendering::RenderState& renderState ) override;

        // SurfaceProvider
        virtual ::cairo::SurfaceSharedPtr getSurface() override;
        virtual ::cairo::SurfaceSharedPtr createSurface( const ::basegfx::B2ISize& rSize, int aContent ) override;
        virtual ::cairo::SurfaceSharedPtr createSurface( ::Bitmap& rBitmap ) override;
        virtual ::cairo::SurfaceSharedPtr changeSurface() override;
        virtual OutputDevice* getOutputDevice() override;

    private:
        /** MUST hold here, too, since CanvasHelper only contains a
            raw pointer (without refcounting)
        */
        SpriteCanvasSharedPtr     mpSpriteCanvas;
        ::cairo::SurfaceSharedPtr mpBufferSurface;
        ::basegfx::B2ISize        maSize;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
