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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_VIEWSHAPE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_VIEWSHAPE_HXX

#include <cppcanvas/renderer.hxx>
#include <cppcanvas/bitmap.hxx>

#include <basegfx/range/b2drectangle.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <tools.hxx>
#include <shapeattributelayer.hxx>
#include <animatedsprite.hxx>
#include <viewlayer.hxx>
#include <doctreenode.hxx>

#include <vector>
#include <memory>

enum class UpdateFlags
{
    NONE           = 0x00,
    Transformation = 0x01,
    Clip           = 0x02,
    Alpha          = 0x04,
    Position       = 0x08,
    Content        = 0x10,
    Force          = 0x20,
};
namespace o3tl {
    template<> struct typed_flags<UpdateFlags> : is_typed_flags<UpdateFlags, 0x3f> {};
}



namespace slideshow::internal
    {
        /** This class is the viewable representation of a draw
            document's XShape, associated to a specific View

            The class is able to render the associated XShape on
            View implementations.
         */
        class ViewShape
        {
        public:
            /** Create a ViewShape for the given View

                @param rView
                The associated View object.
             */
            explicit ViewShape( const ViewLayerSharedPtr& rViewLayer );

            ///Forbid copy construction
            ViewShape(const ViewShape&) = delete;
            /// Forbid copy assignment
            ViewShape& operator=(const ViewShape&) = delete;

            /** Query the associated view layer of this shape
             */
            const ViewLayerSharedPtr& getViewLayer() const;

            /** Query dimension of a safety border around the shape for AA

                If the view performs antialiasing, this method
                calculates a safety border around the shape, in the
                shape coordinate system, which is guaranteed to
                include every pixel touched when rendering the shape.
             */
            ::basegfx::B2DSize getAntialiasingBorder() const;


            // animation methods


            /** Notify the ViewShape that an animation starts now

                This method enters animation mode on the associate
                target view. The shape can be animated in parallel on
                different views.
             */
            void enterAnimationMode();

            /** Notify the ViewShape that it is no longer animated

                This methods ends animation mode on the associate
                target view
             */
            void leaveAnimationMode();


            // render methods


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
                         UpdateFlags                    nUpdateFlags,
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

                const ::cppcanvas::CanvasSharedPtr& getDestinationCanvas() const
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
            static bool prefetch( RendererCacheEntry&                  io_rCacheEntry,
                           const ::cppcanvas::CanvasSharedPtr&  rDestinationCanvas,
                           const GDIMetaFileSharedPtr&          rMtf,
                           const ShapeAttributeLayerSharedPtr&  rAttr );

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
                               UpdateFlags                          nUpdateFlags,
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
                         UpdateFlags                            nUpdateFlags,
                         const ShapeAttributeLayerSharedPtr&    pAttr,
                         const VectorOfDocTreeNodes&            rSubsets,
                         bool                                   bIsVisible ) const;

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

        typedef ::std::shared_ptr< ViewShape > ViewShapeSharedPtr;

}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_VIEWSHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
