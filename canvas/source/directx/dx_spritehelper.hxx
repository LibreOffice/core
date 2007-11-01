/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_spritehelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 17:58:06 $
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

#ifndef _DXCANVAS_SPRITEHELPER_HXX
#define _DXCANVAS_SPRITEHELPER_HXX

#include <com/sun/star/rendering/XCustomSprite.hpp>

#include <canvas/base/canvascustomspritehelper.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include "dx_spritecanvas.hxx"
#include "dx_bitmap.hxx"

namespace dxcanvas
{
    /* Definition of SpriteHelper class */

    /** Helper class for canvas sprites.

        This class implements all sprite-related functionality, like
        that available on the XSprite interface.
     */
    class SpriteHelper : public ::canvas::CanvasCustomSpriteHelper
    {
    public:
        /** Create sprite helper
         */
        SpriteHelper();

        /** Late-init the sprite helper

            @param rSpriteSize
            Size of the sprite

            @param rSpriteCanvas
            Sprite canvas this sprite is part of. Object stores
            ref-counted reference to it, thus, don't forget to pass on
            disposing()!

            @param rRenderModule
            rendermodule to use

            @param rSpriteSurface
            The surface of the sprite (not the DX texture, but the
            persistent target of content rendering)

            @param bShowSpriteBounds
            When true, little debug bound rects for sprites are shown
         */
        void init( const ::com::sun::star::geometry::RealSize2D&    rSpriteSize,
                   const SpriteCanvasRef&                           rSpriteCanvas,
                   const IDXRenderModuleSharedPtr&                  rRenderModule,
                   const DXBitmapSharedPtr                          rBitmap,
                   bool                                             bShowSpriteBounds );

        void disposing();

        /** Repaint sprite content via hardware to associated sprite
            canvas

            @param io_bSurfaceDirty
            Input/output parameter, whether the sprite content is
            dirty or not. If texture was updated, set to false

         */
        void redraw( bool& io_bSurfaceDirty ) const;

    private:
        virtual ::basegfx::B2DPolyPolygon polyPolygonFromXPolyPolygon2D(
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >& xPoly ) const;

        /// Returns true, if the sprite _really_ needs redraw
        bool needRedraw() const;

        SpriteCanvasRef             mpSpriteCanvas;

        DXBitmapSharedPtr           mpBitmap;
        mutable bool                mbTextureDirty;  // when true, texture needs update
        bool                        mbShowSpriteBounds; // when true, debug bound rect for sprites is shown
    };
}

#endif /* _DXCANVAS_SPRITEHELPER_HXX */
