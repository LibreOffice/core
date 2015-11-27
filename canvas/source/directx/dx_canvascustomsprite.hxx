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

#ifndef INCLUDED_CANVAS_SOURCE_DIRECTX_DX_CANVASCUSTOMSPRITE_HXX
#define INCLUDED_CANVAS_SOURCE_DIRECTX_DX_CANVASCUSTOMSPRITE_HXX

#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/rendering/XCustomSprite.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <canvas/base/basemutexhelper.hxx>
#include <canvas/base/canvascustomspritebase.hxx>

#include "dx_sprite.hxx"
#include "dx_surfacebitmap.hxx"
#include "dx_bitmapcanvashelper.hxx"
#include "dx_spritehelper.hxx"
#include "dx_spritecanvas.hxx"


namespace dxcanvas
{
    typedef ::cppu::WeakComponentImplHelper< css::rendering::XCustomSprite,
                                             css::rendering::XBitmapCanvas,
                                             css::rendering::XIntegerBitmap,
                                             css::lang::XServiceInfo >  CanvasCustomSpriteBase_Base;
    /** Mixin Sprite

        Have to mixin the Sprite interface before deriving from
        ::canvas::CanvasCustomSpriteBase, as this template should
        already implement some of those interface methods.

        The reason why this appears kinda convoluted is the fact that
        we cannot specify non-IDL types as WeakComponentImplHelper
        template args, and furthermore, don't want to derive
        ::canvas::CanvasCustomSpriteBase directly from
        ::canvas::Sprite (because derivees of
        ::canvas::CanvasCustomSpriteBase have to explicitly forward
        the XInterface methods (e.g. via DECLARE_UNO3_AGG_DEFAULTS)
        anyway). Basically, ::canvas::CanvasCustomSpriteBase should
        remain a base class that provides implementation, not to
        enforce any specific interface on its derivees.
     */
    class CanvasCustomSpriteSpriteBase_Base : public ::canvas::BaseMutexHelper< CanvasCustomSpriteBase_Base >,
                                                 public Sprite
    {
    };

    typedef ::canvas::CanvasCustomSpriteBase< CanvasCustomSpriteSpriteBase_Base,
                                              SpriteHelper,
                                              BitmapCanvasHelper,
                                              ::osl::MutexGuard,
                                              ::cppu::OWeakObject >                     CanvasCustomSpriteBaseT;

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
        CanvasCustomSprite( const css::geometry::RealSize2D&   rSpriteSize,
                            const SpriteCanvasRef&                          rRefDevice,
                            const IDXRenderModuleSharedPtr&                 rRenderModule,
                            const ::canvas::ISurfaceProxyManagerSharedPtr&  rSurfaceProxy,
                            bool                                            bShowSpriteBounds );

        virtual void disposeThis() override;

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname           Base doing refcount          Base implementing the XComponent interface
        //                                          |                    |                         |
        //                                          V                    V                         V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( CanvasCustomSprite, CanvasCustomSpriteBase_Base, ::cppu::WeakComponentImplHelperBase )

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw( css::uno::RuntimeException ) override;

        // Sprite
        virtual void redraw() const override;

    private:
        /** MUST hold here, too, since BitmapCanvasHelper only contains a
            raw pointer (without refcounting)
        */
        SpriteCanvasRef          mpSpriteCanvas;
        DXSurfaceBitmapSharedPtr mpSurface;
    };
}

#endif // INCLUDED_CANVAS_SOURCE_DIRECTX_DX_CANVASCUSTOMSPRITE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
