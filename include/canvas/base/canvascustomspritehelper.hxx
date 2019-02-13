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

#ifndef INCLUDED_CANVAS_BASE_CANVASCUSTOMSPRITEHELPER_HXX
#define INCLUDED_CANVAS_BASE_CANVASCUSTOMSPRITEHELPER_HXX

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <canvas/base/spritesurface.hxx>
#include <canvas/canvastoolsdllapi.h>

namespace com::sun::star::geometry { struct AffineMatrix2D; }
namespace com::sun::star::geometry { struct RealPoint2D; }
namespace com::sun::star::geometry { struct RealSize2D; }
namespace com::sun::star::rendering { class XBitmap; }
namespace com::sun::star::rendering { class XPolyPolygon2D; }
namespace com::sun::star::rendering { struct RenderState; }
namespace com::sun::star::rendering { struct ViewState; }


namespace canvas
{
    /* Definition of CanvasCustomSpriteHelper class */

    /** Base class for an XSprite helper implementation - to be used
        in concert with CanvasCustomSpriteBase
     */
    class CANVASTOOLS_DLLPUBLIC CanvasCustomSpriteHelper
    {
    public:
        CanvasCustomSpriteHelper();
        virtual ~CanvasCustomSpriteHelper() {}

        /** Init helper

            @param rSpriteSize
            Requested size of the sprite, as passed to the
            XSpriteCanvas::createCustomSprite() method

            @param rOwningSpriteCanvas
            The XSpriteCanvas this sprite is displayed on
         */
        void init( const css::geometry::RealSize2D&                 rSpriteSize,
                   const SpriteSurface::Reference&                  rOwningSpriteCanvas );

        /** Object is being disposed, release all internal references

            @derive when overriding this method in derived classes,
            <em>always</em> call the base class' method!
         */
        void disposing();

        // XCanvas
        /// need  to call this method for XCanvas::clear(), for opacity tracking
        void clearingContent( const Sprite::Reference& rSprite );

        /// need to call this method for XCanvas::drawBitmap(), for opacity tracking
        void checkDrawBitmap( const Sprite::Reference&                                rSprite,
                              const css::uno::Reference< css::rendering::XBitmap >&   xBitmap,
                              const css::rendering::ViewState&                        viewState,
                              const css::rendering::RenderState&                      renderState );

        // XSprite
        void setAlpha( const Sprite::Reference& rSprite,
                       double                   alpha );
        void move( const Sprite::Reference&            rSprite,
                   const css::geometry::RealPoint2D&   aNewPos,
                   const css::rendering::ViewState&    viewState,
                   const css::rendering::RenderState&  renderState );
        void transform( const Sprite::Reference&               rSprite,
                       const css::geometry::AffineMatrix2D&    aTransformation );
        void clip( const Sprite::Reference&                                       rSprite,
                   const css::uno::Reference< css::rendering::XPolyPolygon2D >&   aClip );
        void setPriority( const Sprite::Reference&  rSprite,
                          double                    nPriority );
        void show( const Sprite::Reference& rSprite );
        void hide( const Sprite::Reference& rSprite );

        // Sprite
        bool isAreaUpdateOpaque( const ::basegfx::B2DRange& rUpdateArea ) const;
        const ::basegfx::B2DPoint& getPosPixel() const { return maPosition; }
        const ::basegfx::B2DVector& getSizePixel() const { return maSize; }
        ::basegfx::B2DRange  getUpdateArea() const;
        double               getPriority() const { return mfPriority; }

        // redraw must be implemented by derived - non sensible default implementation
        // void redraw( const Sprite::Reference& rSprite,
        //              const ::basegfx::B2DPoint& rPos ) const;


        // Helper methods for derived classes


        /// Calc sprite update area from given raw sprite bounds
        ::basegfx::B2DRange getUpdateArea( const ::basegfx::B2DRange& rUntransformedSpriteBounds ) const;

        /** Returns true, if sprite content bitmap is fully opaque.

            This does not take clipping or transformation into
            account, but only denotes that the sprite bitmap's alpha
            channel is all 1.0
         */
        bool isContentFullyOpaque() const { return mbIsContentFullyOpaque; }

        /// Returns true, if transformation has changed since last transformUpdated() call
        bool hasTransformChanged() const { return mbTransformDirty; }

        /// Retrieve current alpha value
        double getAlpha() const { return mfAlpha; }

        /// Retrieve current clip
        const css::uno::Reference<
            css::rendering::XPolyPolygon2D >& getClip() const { return mxClipPoly; }

        const ::basegfx::B2DHomMatrix& getTransformation() const { return maTransform; }

        /// Retrieve current activation state
        bool isActive() const { return mbActive; }

    protected:
        /** Notifies that caller is again in sync with current transformation

            const, but modifies state visible to derived
            classes. beware of passing this information to the
            outside!
         */
        void transformUpdated() const { mbTransformDirty=false; }

    private:
        CanvasCustomSpriteHelper( const CanvasCustomSpriteHelper& ) = delete;
        CanvasCustomSpriteHelper& operator=( const CanvasCustomSpriteHelper& ) = delete;

        /** Called to convert an API polygon to a basegfx polygon

            @derive Needs to be provided by backend-specific code
         */
        virtual ::basegfx::B2DPolyPolygon polyPolygonFromXPolyPolygon2D(
            css::uno::Reference< css::rendering::XPolyPolygon2D >& xPoly ) const = 0;

        /** Update clip information from current state

            This method recomputes the maCurrClipBounds and
            mbIsCurrClipRectangle members from the current clip and
            transformation. IFF the clip changed from rectangular to
            rectangular again, this method issues a sequence of
            optimized SpriteSurface::updateSprite() calls.

            @return true, if SpriteSurface::updateSprite() was already
            called within this method.
         */
        bool updateClipState( const Sprite::Reference& rSprite );


        /// Owning sprite canvas
        SpriteSurface::Reference                            mpSpriteCanvas;

        /** Currently active clip area.

            This member is either empty, denoting that the current
            clip shows the full sprite content, or contains a
            rectangular subarea of the sprite, outside of which
            the sprite content is fully clipped.

            @see mbIsCurrClipRectangle
         */
        ::basegfx::B2DRange                                 maCurrClipBounds;

        // sprite state
        ::basegfx::B2DPoint                                 maPosition;
        ::basegfx::B2DVector                                maSize;
        ::basegfx::B2DHomMatrix                             maTransform;
        css::uno::Reference< css::rendering::XPolyPolygon2D > mxClipPoly;
        double                                              mfPriority;
        double                                              mfAlpha;
        bool                                                mbActive; // true, if not hidden

        /** If true, denotes that the current sprite clip is a true
            rectangle, i.e. maCurrClipBounds <em>exactly</em>
            describes the visible area of the sprite.

            @see maCurrClipBounds
         */
        bool                                                mbIsCurrClipRectangle;

        /** Redraw speedup.

            When true, this flag denotes that the current sprite
            content is fully opaque, thus, that blits to the screen do
            neither have to take alpha into account, nor prepare any
            background for the sprite area.
         */
        mutable bool                                        mbIsContentFullyOpaque;

        /// True, iff maTransform has changed
        mutable bool                                        mbTransformDirty;
    };
}

#endif // INCLUDED_CANVAS_BASE_CANVASCUSTOMSPRITEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
