/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvascustomspritehelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:42:39 $
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

#ifndef INCLUDED_CANVAS_CANVASCUSTOMSPRITEHELPER_HXX
#define INCLUDED_CANVAS_CANVASCUSTOMSPRITEHELPER_HXX

#ifndef _COM_SUN_STAR_RENDERING_XCUSTOMSPRITE_HPP_
#include <com/sun/star/rendering/XCustomSprite.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP_
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif
#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif
#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef INCLUDED_CANVAS_SPRITESURFACE_HXX
#include <canvas/base/spritesurface.hxx>
#endif


namespace canvas
{
    /* Definition of CanvasCustomSpriteHelper class */

    /** Base class for an XSprite helper implementation - to be used
        in concert with CanvasCustomSpriteBase
     */
    class CanvasCustomSpriteHelper
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
        void init( const ::com::sun::star::geometry::RealSize2D&    rSpriteSize,
                   const SpriteSurface::Reference&                  rOwningSpriteCanvas );

        /** Object is being disposed, release all internal references

            @derive when overriding this method in derived classes,
            <em>always</em> call the base class' method!
         */
        void disposing();

        // XCanvas
        /// need to call this method for XCanvas::drawBitmap(), for opacity tracking
        void checkDrawBitmap( const Sprite::Reference&                                                          rSprite,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >&   xBitmap,
                              const ::com::sun::star::rendering::ViewState&                                     viewState,
                              const ::com::sun::star::rendering::RenderState&                                   renderState );

        // XSprite
        void setAlpha( const Sprite::Reference& rSprite,
                       double                   alpha );
        void move( const Sprite::Reference&                         rSprite,
                   const ::com::sun::star::geometry::RealPoint2D&   aNewPos,
                   const ::com::sun::star::rendering::ViewState&    viewState,
                   const ::com::sun::star::rendering::RenderState&  renderState );
        void transform( const Sprite::Reference&                            rSprite,
                       const ::com::sun::star::geometry::AffineMatrix2D&    aTransformation );
        void clip( const Sprite::Reference&                                                                 rSprite,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >&   aClip );
        void setPriority( const Sprite::Reference&  rSprite,
                          double                    nPriority );
        void show( const Sprite::Reference& rSprite );
        void hide( const Sprite::Reference& rSprite );

        // XCustomSprite
        /// Need to call this method for XCustomSprite::getContentCanvas(), for surface preparations
        void prepareContentCanvas( const Sprite::Reference& rSprite );

        // Sprite
        bool isAreaUpdateOpaque( const ::basegfx::B2DRange& rUpdateArea ) const;
        ::basegfx::B2DPoint  getPosPixel() const;
        ::basegfx::B2DVector getSizePixel() const;
        ::basegfx::B2DRange  getUpdateArea() const;
        double               getPriority() const;

        // redraw must be implemented by derived - non sensible default implementation
        // void redraw( const Sprite::Reference& rSprite,
        //              const ::basegfx::B2DPoint& rPos ) const;


        // Helper methods for derived classes
        // ----------------------------------

        /// Calc sprite update area from given raw sprite bounds
        ::basegfx::B2DRange getUpdateArea( const ::basegfx::B2DRange& rUntransformedSpriteBounds ) const;

        /// Calc update area for unclipped sprite content
        ::basegfx::B2DRange getFullSpriteRect() const;

        /** Returns true, if sprite content bitmap is fully opaque.

            This does not take clipping or transformation into
            account, but only denotes that the sprite bitmap's alpha
            channel is all 1.0
         */
        bool isContentFullyOpaque() const { return mbIsContentFullyOpaque; }

        /// Returns true, if transformation has changed since last transformUpdated() call
        bool hasAlphaChanged() const { return mbAlphaDirty; }

        /// Returns true, if transformation has changed since last transformUpdated() call
        bool hasPositionChanged() const { return mbPositionDirty; }

        /// Returns true, if transformation has changed since last transformUpdated() call
        bool hasTransformChanged() const { return mbTransformDirty; }

        /// Returns true, if transformation has changed since last transformUpdated() call
        bool hasClipChanged() const { return mbClipDirty; }

        /// Returns true, if transformation has changed since last transformUpdated() call
        bool hasPrioChanged() const { return mbPrioDirty; }

        /// Returns true, if transformation has changed since last transformUpdated() call
        bool hasVisibilityChanged() const { return mbVisibilityDirty; }

        /// Retrieve current alpha value
        double getAlpha() const { return mfAlpha; }

        /// Retrieve current clip
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XPolyPolygon2D >& getClip() const { return mxClipPoly; }

        const ::basegfx::B2DHomMatrix& getTransformation() const { return maTransform; }

        /// Retrieve current activation state
        bool isActive() const { return mbActive; }

    protected:
        /** Notifies that caller is again in sync with current alph

            const, but modifies state visible to derived
            classes. beware of passing this information to the
            outside!
         */
        void alphaUpdated() const { mbAlphaDirty=false; }

        /** Notifies that caller is again in sync with current position

            const, but modifies state visible to derived
            classes. beware of passing this information to the
            outside!
         */
        void positionUpdated() const { mbPositionDirty=false; }

        /** Notifies that caller is again in sync with current transformation

            const, but modifies state visible to derived
            classes. beware of passing this information to the
            outside!
         */
        void transformUpdated() const { mbTransformDirty=false; }

        /** Notifies that caller is again in sync with current clip

            const, but modifies state visible to derived
            classes. beware of passing this information to the
            outside!
         */
        void clipUpdated() const { mbClipDirty=false; }

        /** Notifies that caller is again in sync with current priority

            const, but modifies state visible to derived
            classes. beware of passing this information to the
            outside!
         */
        void prioUpdated() const { mbPrioDirty=false; }

        /** Notifies that caller is again in sync with current visibility

            const, but modifies state visible to derived
            classes. beware of passing this information to the
            outside!
         */
        void visibilityUpdated() const { mbVisibilityDirty=false; }

    private:
        CanvasCustomSpriteHelper( const CanvasCustomSpriteHelper& );
        CanvasCustomSpriteHelper& operator=( const CanvasCustomSpriteHelper& );

        /** Called to clear the sprite surface to fully transparent

            @derive must be overridden by derived classes, and
            implemented as to fully clear the sprite content.
         */
        virtual void clearSurface() = 0;

        /** Called to convert an API polygon to a basegfx polygon

            @derive Needs to be provided by backend-specific code
         */
        virtual ::basegfx::B2DPolyPolygon polyPolygonFromXPolyPolygon2D(
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >& xPoly ) const = 0;

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

        // --------------------------------------------------------------------

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
        ::com::sun::star::uno::Reference<
              ::com::sun::star::rendering::XPolyPolygon2D > mxClipPoly;
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

        /// True, iff mfAlpha has changed
        mutable bool                                        mbAlphaDirty;

        /// True, iff maPosition has changed
        mutable bool                                        mbPositionDirty;

        /// True, iff maTransform has changed
        mutable bool                                        mbTransformDirty;

        /// True, iff mxClipPoly has changed
        mutable bool                                        mbClipDirty;

        /// True, iff mnPriority has changed
        mutable bool                                        mbPrioDirty;

        /// True, iff mbActive has changed
        mutable bool                                        mbVisibilityDirty;
    };
}

#endif /* INCLUDED_CANVAS_CANVASCUSTOMSPRITEHELPER_HXX */
