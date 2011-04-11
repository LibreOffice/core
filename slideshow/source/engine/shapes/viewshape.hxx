/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_SLIDESHOW_VIEWSHAPE_HXX
#define INCLUDED_SLIDESHOW_VIEWSHAPE_HXX

#include <cppcanvas/renderer.hxx>
#include <cppcanvas/bitmap.hxx>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include "tools.hxx"
#include "shapeattributelayer.hxx"
#include "animatedsprite.hxx"
#include "viewlayer.hxx"
#include "doctreenode.hxx"

#include <vector>


namespace slideshow
{
    namespace internal
    {
        /** This class is the viewable representation of a draw
            document's XShape, associated to a specific View

            The class is able to render the associated XShape on
            View implementations.
         */
        class ViewShape : private boost::noncopyable
        {
        public:
            /** Create a ViewShape for the given View

                @param rView
                The associated View object.
             */
            explicit ViewShape( const ViewLayerSharedPtr& rViewLayer );

            /** Query the associated view layer of this shape
             */
            ViewLayerSharedPtr getViewLayer() const;

            /** Query dimension of a safety border around the shape for AA

                If the view performs antialiasing, this method
                calculates a safety border around the shape, in the
                shape coordinate system, which is guaranteed to
                include every pixel touched when rendering the shape.
             */
            ::basegfx::B2DSize getAntialiasingBorder() const;


            // animation methods
            //------------------------------------------------------------------

            /** Notify the ViewShape that an animation starts now

                This method enters animation mode on the associate
                target view. The shape can be animated in parallel on
                different views.

                @return whether the mode change finished successfully.
             */
            bool enterAnimationMode();

            /** Notify the ViewShape that it is no longer animated

                This methods ends animation mode on the associate
                target view
             */
            void leaveAnimationMode();

            /** Query whether the ViewShape is currently animated

                This method checks whether the ViewShape is currently in
                animation mode.
             */
            bool isBackgroundDetached() const { return mbAnimationMode; }


            // render methods
            //------------------------------------------------------------------

            enum UpdateFlags
            {
                NONE=           0,
                TRANSFORMATION= 1,
                CLIP=           2,
                ALPHA=          4,
                POSITION=       8,
                CONTENT=        16,
                FORCE=          32
            };

            struct RenderArgs
            {
                /** Create render argument struct

                    @param rOrigBounds
                    The initial shape bounds

                    @param rUpdateBounds
                    The area covered by the shape

                    @param rBounds
                    The current shape bounds

                    @param rAttr
                    The current shape attribute set. Can be NULL, for
                    default attributes. Attention: stored as a reference,
                    thus, parameter object must stay valid!

                    @param rSubsets
                    Vector of subset rendering ranges. Attention:
                    stored as a reference, thus, parameter object must
                    stay valid!

                    @param nPrio
                    Shape priority
                */
                RenderArgs( const ::basegfx::B2DRectangle&      rOrigBounds,
                            const ::basegfx::B2DRectangle&      rUpdateBounds,
                            const ::basegfx::B2DRectangle&      rBounds,
                            const ::basegfx::B2DRectangle&      rUnitBounds,
                            const ShapeAttributeLayerSharedPtr& rAttr,
                            const VectorOfDocTreeNodes&         rSubsets,
                            double                              nPrio ) :
                    maOrigBounds( rOrigBounds ),
                    maUpdateBounds( rUpdateBounds ),
                    maBounds( rBounds ),
                    maUnitBounds( rUnitBounds ),
                    mrAttr( rAttr ),
                    mrSubsets( rSubsets ),
                    mnShapePriority( nPrio )
                {
                }

                const ::basegfx::B2DRectangle       maOrigBounds;
                const ::basegfx::B2DRectangle       maUpdateBounds;
                const ::basegfx::B2DRectangle       maBounds;
                const ::basegfx::B2DRectangle       maUnitBounds;
                const ShapeAttributeLayerSharedPtr& mrAttr;
                const VectorOfDocTreeNodes&         mrSubsets;
                const double                        mnShapePriority;
            };

            /** Update the ViewShape

                This method updates the ViewShape on the associated
                view. If the shape is currently animated, the render
                target is the sprite, otherwise the view's
                canvas. This method does not render anything, if the
                update flags are 0.

                @param rMtf
                The metafile representation of the shape

                @param rArgs
                Parameter structure, containing all necessary arguments

                @param nUpdateFlags
                Bitmask of things to update. Use FORCE to force a repaint.

                @param bIsVisible
                When false, the shape is fully invisible (and possibly
                don't need to be painted)

                @return whether the rendering finished successfully.
            */
            bool update( const GDIMetaFileSharedPtr&    rMtf,
                         const RenderArgs&              rArgs,
                         int                            nUpdateFlags,
                         bool                           bIsVisible ) const;

            /** Retrieve renderer for given canvas and metafile.

                If necessary, the renderer is created or updated for
                the metafile and attribute layer.

                @return a renderer that renders to the given
                destination canvas
             */
            ::cppcanvas::RendererSharedPtr getRenderer( const ::cppcanvas::CanvasSharedPtr& rDestinationCanvas,
                                                        const GDIMetaFileSharedPtr&         rMtf,
                                                        const ShapeAttributeLayerSharedPtr& rAttr ) const;


        private:
            struct RendererCacheEntry
            {
                RendererCacheEntry() :
                    mpDestinationCanvas(),
                    mpRenderer(),
                    mpMtf(),
                    mpLastBitmap(),
                    mpLastBitmapCanvas()
                {
                }

                ::cppcanvas::CanvasSharedPtr getDestinationCanvas() const
                {
                    return mpDestinationCanvas;
                }

                ::cppcanvas::CanvasSharedPtr        mpDestinationCanvas;
                ::cppcanvas::RendererSharedPtr      mpRenderer;
                GDIMetaFileSharedPtr                mpMtf;
                ::cppcanvas::BitmapSharedPtr        mpLastBitmap;
                ::cppcanvas::BitmapCanvasSharedPtr  mpLastBitmapCanvas;
            };

            typedef ::std::vector< RendererCacheEntry > RendererCacheVector;


            /** Prefetch Renderer for given canvas
             */
            bool prefetch( RendererCacheEntry&                  io_rCacheEntry,
                           const ::cppcanvas::CanvasSharedPtr&  rDestinationCanvas,
                           const GDIMetaFileSharedPtr&          rMtf,
                           const ShapeAttributeLayerSharedPtr&  rAttr ) const;

            /** Draw with prefetched Renderer to stored canvas

                This method draws prefetched Renderer to its
                associated canvas (which happens to be mpLastCanvas).
             */
            bool draw( const ::cppcanvas::CanvasSharedPtr&  rDestinationCanvas,
                       const GDIMetaFileSharedPtr&          rMtf,
                       const ShapeAttributeLayerSharedPtr&  rAttr,
                       const ::basegfx::B2DHomMatrix&       rTransform,
                       const ::basegfx::B2DPolyPolygon*     pClip,
                       const VectorOfDocTreeNodes&          rSubsets ) const;

            /** Render shape to an active sprite
             */
            bool renderSprite( const ViewLayerSharedPtr&            rViewLayer,
                               const GDIMetaFileSharedPtr&          rMtf,
                               const ::basegfx::B2DRectangle&       rOrigBounds,
                               const ::basegfx::B2DRectangle&       rBounds,
                               const ::basegfx::B2DRectangle&       rUnitBounds,
                               int                                  nUpdateFlags,
                               const ShapeAttributeLayerSharedPtr&  pAttr,
                               const VectorOfDocTreeNodes&          rSubsets,
                               double                               nPrio,
                               bool                                 bIsVisible ) const;

            /** Render shape to given canvas
             */
            bool render( const ::cppcanvas::CanvasSharedPtr&    rDestinationCanvas,
                         const GDIMetaFileSharedPtr&            rMtf,
                         const ::basegfx::B2DRectangle&         rBounds,
                         const ::basegfx::B2DRectangle&         rUpdateBounds,
                         int                                    nUpdateFlags,
                         const ShapeAttributeLayerSharedPtr&    pAttr,
                         const VectorOfDocTreeNodes&            rSubsets,
                         bool                                   bIsVisible ) const;

            /** Calc sprite size in pixel

                Converts user coordinate system to device pixel, and
                adds antialiasing border.

                @param rUserSize
                Size of the sprite in user coordinate system (doc coordinates)
             */
            ::basegfx::B2DSize calcSpriteSizePixel( const ::basegfx::B2DSize& rUserSize ) const;

            enum{ MAX_RENDER_CACHE_ENTRIES=2 };

            /** Retrieve a valid iterator to renderer cache entry

                This method ensures that an internal limit of
                MAX_RENDER_CACHE_ENTRIES is not exceeded.

                @param rDestinationCanvas
                Destination canvas to retrieve cache entry for

                @return a valid iterator to a renderer cache entry for
                the given canvas. The entry might be
                default-constructed (if newly added)
             */
            RendererCacheVector::iterator getCacheEntry( const ::cppcanvas::CanvasSharedPtr& rDestinationCanvas ) const;

            void invalidateRenderer() const;

            /** The view layer this object is part of.

                Needed for sprite creation
            */
            ViewLayerSharedPtr                          mpViewLayer;

            /// A set of cached mtf/canvas combinations
            mutable RendererCacheVector                 maRenderers;

            /// The sprite object
            mutable AnimatedSpriteSharedPtr             mpSprite;

            /// If true, render() calls go to the sprite
            mutable bool                                mbAnimationMode;

            /// If true, shape needs full repaint (and the sprite a setup, if any)
            mutable bool                                mbForceUpdate;
        };

        typedef ::boost::shared_ptr< ViewShape > ViewShapeSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_VIEWSHAPE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
