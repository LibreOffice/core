/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: spritehelper.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _VCLCANVAS_SPRITEHELPER_HXX
#define _VCLCANVAS_SPRITEHELPER_HXX

#include <com/sun/star/rendering/XCustomSprite.hpp>

#include <vcl/virdev.hxx>

#include <canvas/base/canvascustomspritehelper.hxx>
#include <canvas/vclwrapper.hxx>

#include "backbuffer.hxx"
#include "impltools.hxx"
#include "spritecanvas.hxx"


namespace vclcanvas
{
    /* Definition of SpriteHelper class */

    /** Helper class for canvas sprites.

        This class implements all sprite-related functionality, like
        that available on the XSprite interface.
     */
    class SpriteHelper : public ::canvas::CanvasCustomSpriteHelper
    {
    public:
        SpriteHelper();

        // make CanvasCustomSpriteHelper::init visible for name lookup
        using ::canvas::CanvasCustomSpriteHelper::init;

        /** Late-init the sprite helper

            @param rSpriteSize
            Size of the sprite

            @param rSpriteCanvas
            Sprite canvas this sprite is part of. Object stores
            ref-counted reference to it, thus, don't forget to pass on
            disposing()!

            @param rBackBuffer
            Buffer of the sprite content (non-alpha part)

            @param rBackBufferMask
            Buffer of the sprite content (alpha part)
         */
        void init( const ::com::sun::star::geometry::RealSize2D&    rSpriteSize,
                   const SpriteCanvasRef&                           rSpriteCanvas,
                   const BackBufferSharedPtr&                       rBackBuffer,
                   const BackBufferSharedPtr&                       rBackBufferMask,
                   bool                                             bShowSpriteBounds );

        void disposing();

        /** Repaint sprite content to associated sprite canvas

            @param rPos
            Output position (sprite's own position is disregarded)

            @param io_bSurfacesDirty
            When true, the referenced sprite surfaces (backBuffer and
            backBufferMask) have been modified since last call.

            @param bBufferedUpdate
            When true, the redraw does <em>not</em> happen directly on
            the front buffer, but within a VDev. Used to speed up
            drawing.
         */
        void redraw( OutputDevice&              rOutDev,
                     const ::basegfx::B2DPoint& rPos,
                     bool&                      bSurfacesDirty,
                     bool                       bBufferedUpdate ) const;

    private:
        virtual ::basegfx::B2DPolyPolygon polyPolygonFromXPolyPolygon2D(
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >& xPoly ) const;

        // for the redraw
        BackBufferSharedPtr                                 mpBackBuffer;
        BackBufferSharedPtr                                 mpBackBufferMask;

        // necessary to avoid downcasts later on (Sprite interface
        // only gets passed ptr to SpriteSurface interface, not
        // concrete implementations)
        SpriteCanvasRef                                     mpSpriteCanvas;

        /// Cached bitmap for the current sprite content
        mutable ::canvas::vcltools::VCLObject<BitmapEx>     maContent;

        /// When true, line sprite corners in red
        bool                                                mbShowSpriteBounds;

    };
}

#endif /* _VCLCANVAS_SPRITEHELPER_HXX */
