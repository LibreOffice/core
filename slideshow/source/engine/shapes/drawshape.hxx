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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_DRAWSHAPE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_DRAWSHAPE_HXX

#include <osl/diagnose.hxx>
#include <com/sun/star/drawing/XShape.hpp>

#include "attributableshape.hxx"
#include "doctreenodesupplier.hxx"
#include "gdimtftools.hxx"
#include "viewshape.hxx"
#include "hyperlinkarea.hxx"

#include <boost/optional.hpp>
#include <set>
#include <vector>

class Graphic;

namespace slideshow
{
    namespace internal
    {
        class  Activity;
        struct SlideShowContext;
        class  DrawShapeSubsetting;
        class  DrawShape;
        typedef ::std::shared_ptr< DrawShape > DrawShapeSharedPtr;

        /** This class is the representation of a draw document's
            XShape, and implements the Shape, AnimatableShape, and
            AttributableShape interfaces.

            @attention this class is to be treated 'final', i.e. one
            should not derive from it.
         */
        class DrawShape : public AttributableShape,
                          public DocTreeNodeSupplier,
                          public HyperlinkArea,
                          public ::osl::DebugBase<DrawShape>
        {
        public:
            /** Create a shape for the given XShape

                @param xShape
                The XShape to represent.

                @param xContainingPage
                The page that contains this shape. Needed for proper
                import (currently, the UnoGraphicExporter needs this
                information).

                @param nPrio
                Externally-determined shape priority (used e.g. for
                paint ordering). This number _must be_ unique!

                @param bForeignSource
                When true, the source of the shape metafile might be a
                foreign application. The metafile is checked against
                unsupported content, and, if necessary, returned as a
                pre-rendererd bitmap.
             */
            static DrawShapeSharedPtr create(
                const css::uno::Reference< css::drawing::XShape >&    xShape,
                const css::uno::Reference< css::drawing::XDrawPage >& xContainingPage,
                double                                     nPrio,
                bool                                       bForeignSource,
                const SlideShowContext&                    rContext ); // throw ShapeLoadFailedException;

            /** Create a shape for the given XShape and graphic content

                @param xShape
                The XShape to represent.

                @param xContainingPage
                The page that contains this shape. Needed for proper
                import (currently, the UnoGraphicExporter needs this
                information).

                @param nPrio
                Externally-determined shape priority (used e.g. for
                paint ordering). This number _must be_ unique!

                @param rGraphic
                Graphic to display in the shape's bound rect. If this
                Graphic contains animatable content, the created
                DrawShape will register itself for intrinsic animation
                events.
             */
            static DrawShapeSharedPtr create(
                const css::uno::Reference< css::drawing::XShape >&    xShape,
                const css::uno::Reference< css::drawing::XDrawPage >& xContainingPage,
                double                                     nPrio,
                const Graphic&                             rGraphic,
                const SlideShowContext&                    rContext ); // throw ShapeLoadFailedException;

            virtual css::uno::Reference< css::drawing::XShape > getXShape() const override;

            virtual ~DrawShape();


            // View layer methods


            virtual void addViewLayer( const ViewLayerSharedPtr&    rNewLayer,
                                       bool                         bRedrawLayer ) override;
            virtual bool removeViewLayer( const ViewLayerSharedPtr& rNewLayer ) override;
            virtual void clearAllViewLayers() override;

            // attribute methods


            virtual ShapeAttributeLayerSharedPtr createAttributeLayer() override;
            virtual bool revokeAttributeLayer( const ShapeAttributeLayerSharedPtr& rLayer ) override;
            virtual ShapeAttributeLayerSharedPtr getTopmostAttributeLayer() const override;
            virtual void setVisibility( bool bVisible ) override;
            virtual ::basegfx::B2DRectangle getBounds() const override;
            virtual ::basegfx::B2DRectangle getDomBounds() const override;
            virtual ::basegfx::B2DRectangle getUpdateArea() const override;
            virtual bool isVisible() const override;
            virtual double getPriority() const override;


            // animation methods


            virtual void enterAnimationMode() override;
            virtual void leaveAnimationMode() override;
            virtual bool isBackgroundDetached() const override;

            // render methods


            virtual bool update() const override;
            virtual bool render() const override;
            virtual bool isContentChanged() const override;

            // Sub item specialities


            virtual const DocTreeNodeSupplier&  getTreeNodeSupplier() const override;
            virtual DocTreeNodeSupplier&        getTreeNodeSupplier() override;

            virtual DocTreeNode                 getSubsetNode() const override;
            virtual AttributableShapeSharedPtr  getSubset( const DocTreeNode& rTreeNode ) const override;
            virtual bool                        createSubset( AttributableShapeSharedPtr&   o_rSubset,
                                                              const DocTreeNode&            rTreeNode ) override;
            virtual bool                        revokeSubset( const AttributableShapeSharedPtr& rShape ) override;


            // DocTreeNodeSupplier methods


            virtual sal_Int32   getNumberOfTreeNodes        ( DocTreeNode::NodeType eNodeType ) const override; // throw ShapeLoadFailedException;
            virtual DocTreeNode getTreeNode                 ( sal_Int32             nNodeIndex,
                                                              DocTreeNode::NodeType eNodeType ) const override; // throw ShapeLoadFailedException;
            virtual sal_Int32   getNumberOfSubsetTreeNodes  ( const DocTreeNode&    rParentNode,
                                                              DocTreeNode::NodeType eNodeType ) const override; // throw ShapeLoadFailedException;
            virtual DocTreeNode getSubsetTreeNode           ( const DocTreeNode&    rParentNode,
                                                              sal_Int32             nNodeIndex,
                                                              DocTreeNode::NodeType eNodeType ) const override; // throw ShapeLoadFailedException;

            // HyperlinkArea methods


            virtual HyperlinkRegions getHyperlinkRegions() const override;
            virtual double getHyperlinkPriority() const override;


            // intrinsic animation methods


            /** Display next frame of an intrinsic animation.

                Used by IntrinsicAnimationActivity, to show the next
                animation frame.
             */
            void setIntrinsicAnimationFrame( ::std::size_t nCurrFrame );

            /** forces the drawshape to load and return a specially
                crafted metafile, usable to display drawing layer text
                animations.
            */
            GDIMetaFileSharedPtr const & forceScrollTextMetaFile();

        private:
            /** Create a shape for the given XShape

                @param xShape
                The XShape to represent.

                @param xContainingPage
                The page that contains this shape. Needed for proper
                import (currently, the UnoGraphicExporter needs this
                information).

                @param nPrio
                Externally-determined shape priority (used e.g. for
                paint ordering). This number _must be_ unique!

                @param bForeignSource
                When true, the source of the shape metafile might be a
                foreign application. The metafile is checked against
                unsupported content, and, if necessary, returned as a
                pre-rendererd bitmap.
             */
            DrawShape( const css::uno::Reference<
                            css::drawing::XShape >&    xShape,
                       const css::uno::Reference<
                            css::drawing::XDrawPage >& xContainingPage,
                       double                                       nPrio,
                       bool                                         bForeignSource,
                       const SlideShowContext&                      rContext ); // throw ShapeLoadFailedException;

            /** Create a shape for the given XShape and graphic content

                @param xShape
                The XShape to represent.

                @param xContainingPage
                The page that contains this shape. Needed for proper
                import (currently, the UnoGraphicExporter needs this
                information).

                @param nPrio
                Externally-determined shape priority (used e.g. for
                paint ordering). This number _must be_ unique!

                @param rGraphic
                Graphic to display in the shape's bound rect. If this
                Graphic contains animatable content, the created
                DrawShape will register itself for intrinsic animation
                events.
             */
            DrawShape( const css::uno::Reference< css::drawing::XShape >&    xShape,
                       const css::uno::Reference< css::drawing::XDrawPage >& xContainingPage,
                       double                                       nPrio,
                       const Graphic&                               rGraphic,
                       const SlideShowContext&                      rContext ); // throw ShapeLoadFailedException;

            /** Private copy constructor

                Used to create subsetted shapes
             */
            DrawShape( const DrawShape&, const DocTreeNode& rTreeNode, double nPrio );

            int  getUpdateFlags() const;
            bool implRender( int nUpdateFlags ) const;
            void updateStateIds() const;

            ViewShape::RenderArgs   getViewRenderArgs() const;
            ::basegfx::B2DRectangle getActualUnitShapeBounds() const;

            bool hasIntrinsicAnimation() const;
            bool hasHyperlinks() const;
            void prepareHyperlinkIndices() const;

            /// The associated XShape
            css::uno::Reference< css::drawing::XShape >                             mxShape;
            css::uno::Reference< css::drawing::XDrawPage >                          mxPage;

            /** A vector of metafiles actually representing the Shape.

                If this shape is not animated, only a single entry is
                available.
             */
            mutable VectorOfMtfAnimationFrames                                      maAnimationFrames;
            ::std::size_t                                                           mnCurrFrame;

            /// Metafile of currently active frame (static for shapes w/o intrinsic animation)
            mutable GDIMetaFileSharedPtr                                            mpCurrMtf;

            /// loadflags of current meta file
            mutable int                                                             mnCurrMtfLoadFlags;

            /// Contains the current shape bounds, in unit rect space
            mutable ::boost::optional<basegfx::B2DRectangle>                        maCurrentShapeUnitBounds;

            // The attributes of this Shape
            const double                                                            mnPriority;
            ::basegfx::B2DRectangle                                                 maBounds; // always needed for rendering.
                                                                                              // for subset shapes, this member
                                                                                              // might change when views are
                                                                                              // added, as minimal bounds are
                                                                                              // calculated

            // Pointer to modifiable shape attributes
            ShapeAttributeLayerSharedPtr                                            mpAttributeLayer; // only created lazily

            // held here, to signal our destruction
            std::weak_ptr<Activity>                                               mpIntrinsicAnimationActivity;

            // The attribute states, to detect attribute changes,
            // without buffering and querying each single attribute
            mutable State::StateId                                                  mnAttributeTransformationState;
            mutable State::StateId                                                  mnAttributeClipState;
            mutable State::StateId                                                  mnAttributeAlphaState;
            mutable State::StateId                                                  mnAttributePositionState;
            mutable State::StateId                                                  mnAttributeContentState;
            mutable State::StateId                                                  mnAttributeVisibilityState;

            /// the list of active view shapes (one for each registered view layer)
            typedef ::std::vector< ViewShapeSharedPtr > ViewShapeVector;
            ViewShapeVector                                                         maViewShapes;

            css::uno::Reference< css::uno::XComponentContext>                       mxComponentContext;

            /// hyperlink support
            typedef ::std::pair<sal_Int32 /* mtf start */,
                                sal_Int32 /* mtf end */> HyperlinkIndexPair;
            typedef ::std::vector<HyperlinkIndexPair> HyperlinkIndexPairVector;
            mutable HyperlinkIndexPairVector                                        maHyperlinkIndices;
            mutable HyperlinkRegions                                                maHyperlinkRegions;

            /// Delegated subset handling
            mutable DrawShapeSubsetting                                             maSubsetting;

            /// Whether this shape is currently in animation mode (value != 0)
            int                                                                     mnIsAnimatedCount;

            /// Number of times the bitmap animation shall loop
            ::std::size_t                                                           mnAnimationLoopCount;

            /// Cycle mode for bitmap animation
            CycleMode                                                               meCycleMode;

            /// Whether shape is visible (without attribute layers)
            bool                                                                    mbIsVisible;

            /// Whether redraw is necessary, regardless of state ids
            mutable bool                                                            mbForceUpdate;

            /// Whether attribute layer was revoked (making a redraw necessary)
            mutable bool                                                            mbAttributeLayerRevoked;

            /// whether a drawing layer animation has to be performed
            bool                                                                    mbDrawingLayerAnim;

        };
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_DRAWSHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
