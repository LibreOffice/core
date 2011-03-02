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

#ifndef INCLUDED_SLIDESHOW_DRAWSHAPE_HXX
#define INCLUDED_SLIDESHOW_DRAWSHAPE_HXX

#include <osl/diagnose.hxx>
#include <com/sun/star/drawing/XShape.hpp>

#include "attributableshape.hxx"
#include "doctreenodesupplier.hxx"
#include "gdimtftools.hxx"
#include "viewshape.hxx"
#include "hyperlinkarea.hxx"

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
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
        typedef ::boost::shared_ptr< DrawShape > DrawShapeSharedPtr;

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
                const ::com::sun::star::uno::Reference<
                   ::com::sun::star::drawing::XShape >&    xShape,
                const ::com::sun::star::uno::Reference<
                   ::com::sun::star::drawing::XDrawPage >& xContainingPage,
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
                const ::com::sun::star::uno::Reference<
                   ::com::sun::star::drawing::XShape >&    xShape,
                const ::com::sun::star::uno::Reference<
                   ::com::sun::star::drawing::XDrawPage >& xContainingPage,
                double                                     nPrio,
                const Graphic&                             rGraphic,
                const SlideShowContext&                    rContext ); // throw ShapeLoadFailedException;

            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShape > getXShape() const;

            virtual ~DrawShape();


            // View layer methods
            //------------------------------------------------------------------

            virtual void addViewLayer( const ViewLayerSharedPtr&    rNewLayer,
                                       bool                         bRedrawLayer );
            virtual bool removeViewLayer( const ViewLayerSharedPtr& rNewLayer );
            virtual bool clearAllViewLayers();

            // attribute methods
            //------------------------------------------------------------------

            virtual ShapeAttributeLayerSharedPtr createAttributeLayer();
            virtual bool revokeAttributeLayer( const ShapeAttributeLayerSharedPtr& rLayer );
            virtual ShapeAttributeLayerSharedPtr getTopmostAttributeLayer() const;
            virtual void setVisibility( bool bVisible );
            virtual ::basegfx::B2DRectangle getBounds() const;
            virtual ::basegfx::B2DRectangle getDomBounds() const;
            virtual ::basegfx::B2DRectangle getUpdateArea() const;
            virtual bool isVisible() const;
            virtual double getPriority() const;


            // animation methods
            //------------------------------------------------------------------

            virtual void enterAnimationMode();
            virtual void leaveAnimationMode();
            virtual bool isBackgroundDetached() const;

            // render methods
            //------------------------------------------------------------------

            virtual bool update() const;
            virtual bool render() const;
            virtual bool isContentChanged() const;

            // Sub item specialities
            //------------------------------------------------------------------

            virtual const DocTreeNodeSupplier&  getTreeNodeSupplier() const;
            virtual DocTreeNodeSupplier&        getTreeNodeSupplier();

            virtual DocTreeNode                 getSubsetNode() const;
            virtual AttributableShapeSharedPtr  getSubset( const DocTreeNode& rTreeNode ) const;
            virtual bool                        createSubset( AttributableShapeSharedPtr&   o_rSubset,
                                                              const DocTreeNode&            rTreeNode );
            virtual bool                        revokeSubset( const AttributableShapeSharedPtr& rShape );


            // DocTreeNodeSupplier methods
            //------------------------------------------------------------------

            virtual sal_Int32   getNumberOfTreeNodes        ( DocTreeNode::NodeType eNodeType ) const; // throw ShapeLoadFailedException;
            virtual DocTreeNode getTreeNode                 ( sal_Int32             nNodeIndex,
                                                              DocTreeNode::NodeType eNodeType ) const; // throw ShapeLoadFailedException;
            virtual sal_Int32   getNumberOfSubsetTreeNodes  ( const DocTreeNode&    rParentNode,
                                                              DocTreeNode::NodeType eNodeType ) const; // throw ShapeLoadFailedException;
            virtual DocTreeNode getSubsetTreeNode           ( const DocTreeNode&    rParentNode,
                                                              sal_Int32             nNodeIndex,
                                                              DocTreeNode::NodeType eNodeType ) const; // throw ShapeLoadFailedException;

            // HyperlinkArea methods
            //------------------------------------------------------------------

            virtual HyperlinkRegions getHyperlinkRegions() const;
            virtual double getHyperlinkPriority() const;


            // intrinsic animation methods
            //------------------------------------------------------------------

            /** Display next frame of an intrinsic animation.

                Used by IntrinsicAnimationActivity, to show the next
                animation frame.
             */
            bool setIntrinsicAnimationFrame( ::std::size_t nCurrFrame );

            /** forces the drawshape to load and return a specially
                crafted metafile, usable to display drawing layer text
                animations.
            */
            GDIMetaFileSharedPtr forceScrollTextMetaFile();

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
            DrawShape( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::drawing::XShape >&    xShape,
                       const ::com::sun::star::uno::Reference<
                            ::com::sun::star::drawing::XDrawPage >& xContainingPage,
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
            DrawShape( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::drawing::XShape >&    xShape,
                       const ::com::sun::star::uno::Reference<
                            ::com::sun::star::drawing::XDrawPage >& xContainingPage,
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

            void ensureVerboseMtfComments() const;
            bool hasIntrinsicAnimation() const;
            bool hasHyperlinks() const;
            void prepareHyperlinkIndices() const;

            /// The associated XShape
            ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShape >                             mxShape;
            ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XDrawPage >                          mxPage;

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
            boost::weak_ptr<Activity>                                               mpIntrinsicAnimationActivity;

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

            ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext>                           mxComponentContext;

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

#endif /* INCLUDED_SLIDESHOW_DRAWSHAPE_HXX */
