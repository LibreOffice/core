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

#ifndef _NULLCANVAS_SPRITECANVASHELPER_HXX_
#define _NULLCANVAS_SPRITECANVASHELPER_HXX_

#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <canvas/spriteredrawmanager.hxx>

#include "null_canvashelper.hxx"

namespace basegfx
{
    class B2IRange;
}

namespace nullcanvas
{
    class SpriteCanvas;

    class SpriteCanvasHelper : public CanvasHelper
    {
    public:
        SpriteCanvasHelper();

        void init( ::canvas::SpriteRedrawManager& rManager,
                   SpriteCanvas&                  rDevice,
                   const ::basegfx::B2ISize&      rSize,
                   bool                           bHasAlpha );

        /// Dispose all internal references
        void disposing();

        // XSpriteCanvas
        ::com::sun::star::uno::Reference<
              ::com::sun::star::rendering::XAnimatedSprite >        createSpriteFromAnimation(
                const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimation >& animation );

        ::com::sun::star::uno::Reference<
              ::com::sun::star::rendering::XAnimatedSprite >        createSpriteFromBitmaps(
                const ::com::sun::star::uno::Sequence<
                      ::com::sun::star::uno::Reference<
                            ::com::sun::star::rendering::XBitmap > >& animationBitmaps,
                sal_Int8                                              interpolationMode );

        ::com::sun::star::uno::Reference<
              ::com::sun::star::rendering::XCustomSprite >      createCustomSprite(
                const ::com::sun::star::geometry::RealSize2D& spriteSize );

        ::com::sun::star::uno::Reference<
              ::com::sun::star::rendering::XSprite >                createClonedSprite(
                const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSprite >& original );

        /** Actually perform the screen update

            @param rCurrArea
            Current window area in absolute screen coordinates

            @param bUpdateAll
            sal_True, if everything must be updated, not only changed
            sprites

            @param io_bSurfaceDirty
            In/out parameter, whether backbuffer surface is dirty (if
            yes, we're performing a full update, anyway)
         */
        sal_Bool updateScreen( const ::basegfx::B2IRange& rCurrArea,
                               sal_Bool                   bUpdateAll,
                               bool&                      io_bSurfaceDirty );

    private:
        /// Set from the SpriteCanvas: instance coordinating sprite redraw
        ::canvas::SpriteRedrawManager*  mpRedrawManager;
    };
}

#endif /* _NULLCANVAS_SPRITECANVASHELPER_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
