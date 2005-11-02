/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spritehelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:05:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        virtual void clearSurface();
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
