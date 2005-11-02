/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spritecanvashelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:04:50 $
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

#ifndef _VCLCANVAS_SPRITECANVASHELPER_HXX_
#define _VCLCANVAS_SPRITECANVASHELPER_HXX_

#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <vcl/virdev.hxx>

#include <canvas/spriteredrawmanager.hxx>
#include <canvas/elapsedtime.hxx>
#include <canvas/vclwrapper.hxx>
#include "canvashelper.hxx"
#include "impltools.hxx"


namespace vclcanvas
{
    class RedrawManager;
    class SpriteCanvas;

    class SpriteCanvasHelper : public CanvasHelper
    {
    public:
        SpriteCanvasHelper();

        void setRedrawManager( ::canvas::SpriteRedrawManager& rManager );

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

            @param bUpdateAll
            sal_True, if everything must be updated, not only changed
            sprites

            @param io_bSurfaceDirty
            In/out parameter, whether backbuffer surface is dirty (if
            yes, we're performing a full update, anyway)
         */
        sal_Bool updateScreen( sal_Bool bUpdateAll,
                               bool&    io_bSurfaceDirty );

        // SpriteRedrawManager functor calls
        // -------------------------------------------------

        /** Gets called for simple background repaints
         */
        void backgroundPaint( const ::basegfx::B2DRange& rUpdateRect );

        /** Gets called when area can be handled by scrolling.

            Called method must copy screen content from rMoveStart to
            rMoveEnd, and restore the background in the uncovered
            areas.

            @param rMoveStart
            Source rect of the scroll

            @param rMoveEnd
            Dest rect of the scroll

            @param rUpdateArea
            All info necessary, should rMoveStart be partially or
            fully outside the outdev
         */
        void scrollUpdate( const ::basegfx::B2DRange&                           rMoveStart,
                           const ::basegfx::B2DRange&                           rMoveEnd,
                           const ::canvas::SpriteRedrawManager::UpdateArea&     rUpdateArea );

        void opaqueUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                           const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites );

        void genericUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                            const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites );

        ::com::sun::star::uno::Any isUnsafeScrolling() const
        {
            return ::com::sun::star::uno::makeAny(mbIsUnsafeScrolling);
        }
        void enableUnsafeScrolling( const ::com::sun::star::uno::Any& rAny )
        {
            mbIsUnsafeScrolling = rAny.get<bool>();
        }

        ::com::sun::star::uno::Any isSpriteBounds() const
        {
            return ::com::sun::star::uno::makeAny(mbShowSpriteBounds);
        }
        void enableSpriteBounds( const ::com::sun::star::uno::Any& rAny )
        {
            mbShowSpriteBounds = rAny.get<bool>();
        }

    private:
        void renderFrameCounter( OutputDevice& rOutDev );
        void renderSpriteCount( OutputDevice& rOutDev );
        void renderMemUsage( OutputDevice& rOutDev );

        /// Set from the SpriteCanvas: instance coordinating sprite redraw
        ::canvas::SpriteRedrawManager*                  mpRedrawManager;

        /** Background compositing surface.

            Typically, sprites will be composited in the background,
            before pushing them to screen. This happens here.
         */
        ::canvas::vcltools::VCLObject< VirtualDevice >  maVDev;

        /// For the frame counter timings
        ::canvas::tools::ElapsedTime                    maLastUpdate;

        /// When true, canvas displays debug info on each frame
        bool                                            mbShowFrameInfo;

        /// When true, canvas creates all new sprites with red lines in the corners
        bool                                            mbShowSpriteBounds;

        /// When true, canvas uses the scroll optimization (direct scrolls in front buffer)
        bool                                            mbIsUnsafeScrolling;
    };
}

#endif /* _VCLCANVAS_SPRITECANVASHELPER_HXX_ */

