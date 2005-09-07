/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvascustomsprite.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:18:42 $
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

#ifndef _VCLCANVAS_CANVASCUSTOMSPRITE_HXX
#define _VCLCANVAS_CANVASCUSTOMSPRITE_HXX

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif

#ifndef _COMPHELPER_UNO3_HXX
#include <comphelper/uno3.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XCUSTOMSPRITE_HPP_
#include <com/sun/star/rendering/XCustomSprite.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP_
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#include <canvas/vclwrapper.hxx>
#include <canvas/bitmapcanvasbase.hxx>

#include "canvashelper.hxx"
#include "backbuffer.hxx"
#include "impltools.hxx"
#include "spritecanvas.hxx"
#include "sprite.hxx"
#include "repainttarget.hxx"


#define CANVASCUSTOMSPRITE_IMPLEMENTATION_NAME "VCLCanvas::CanvasCustomSprite"

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper3< ::com::sun::star::rendering::XCustomSprite,
                                               ::com::sun::star::rendering::XBitmapCanvas,
                                                ::com::sun::star::lang::XServiceInfo >                                  CanvasCustomSpriteBase_Base;
    typedef ::canvas::internal::BitmapCanvasBase< CanvasCustomSpriteBase_Base, CanvasHelper, tools::LocalGuard >    CanvasCustomSprite_Base;

    /* Definition of CanvasCustomSprite class */

    class CanvasCustomSprite : public Sprite,
                               public CanvasCustomSprite_Base,
                               public RepaintTarget
    {
    public:
        CanvasCustomSprite( const ::com::sun::star::geometry::RealSize2D&   rSpriteSize,
                            const WindowGraphicDevice::ImplRef&                     rDevice,
                            const SpriteCanvas::ImplRef&                            rSpriteCanvas );

        // XInterface

        // Need to employ this macro, because Sprite comes with an
        // unimplemented version of XInterface.

        // Forwarding the XInterface implementation to the
        // cppu::ImplHelper templated base, which does the refcounting and
        // queryInterface for us:  Classname     Base doing refcount and handling queryInterface
        //                             |                 |
        //                             V                 V
        DECLARE_UNO3_AGG_DEFAULTS( CanvasCustomSprite, CanvasCustomSpriteBase_Base );

        virtual void SAL_CALL disposing();

        // XCanvas: selectively override base's method here, for opacity tracking
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >& xBitmap,
                        const ::com::sun::star::rendering::ViewState&                                   viewState,
                        const ::com::sun::star::rendering::RenderState&                                 renderState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        // XSprite
        virtual void SAL_CALL setAlpha( double alpha ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL move( const ::com::sun::star::geometry::RealPoint2D& aNewPos, const ::com::sun::star::rendering::ViewState& viewState, const ::com::sun::star::rendering::RenderState& renderState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL transform( const ::com::sun::star::geometry::AffineMatrix2D& aTransformation ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clip( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >& aClip ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPriority( double nPriority ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL show(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL hide(  ) throw (::com::sun::star::uno::RuntimeException);

        // XCustomSprite
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas > SAL_CALL
            getContentCanvas(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw( ::com::sun::star::uno::RuntimeException );

        // Sprite
        virtual void redraw( OutputDevice& rTargetSurface ) const;
        virtual void redraw( OutputDevice&  rTargetSurface,
                             const Point&   rOutputPos ) const;
        virtual bool isAreaUpdateOpaque( const Rectangle& rUpdateArea ) const;
        virtual ::basegfx::B2DPoint getSpritePos() const;
        virtual ::basegfx::B2DSize  getSpriteSize() const;

        // RepaintTarget
        virtual bool repaint( const GraphicObjectSharedPtr& rGrf,
                              const ::Point&                rPt,
                              const ::Size&                 rSz,
                              const GraphicAttr&            rAttr ) const;

    protected:
        ~CanvasCustomSprite(); // we're a ref-counted UNO class. _We_ destroy ourselves.

    private:
        // default: disabled copy/assignment
        CanvasCustomSprite(const CanvasCustomSprite&);
        CanvasCustomSprite& operator=( const CanvasCustomSprite& );

        Rectangle getSpriteRect( const ::basegfx::B2DRectangle& rBounds ) const;
        Rectangle getFullSpriteRect() const;
        Rectangle getSpriteRect() const;

        // for the integrated bitmap canvas implementation
        BackBufferSharedPtr     mpBackBuffer;
        BackBufferSharedPtr     mpBackBufferMask;

        SpriteCanvas::ImplRef   mpSpriteCanvas;

        mutable ::canvas::vcltools::VCLObject<BitmapEx>     maContent;

        /** Currently active clip area.

            This member is either empty, denoting that the current
            clip shows the full sprite content, or contains a
            rectangular subarea of the sprite, outside of which
            the sprite content is fully clipped.

            @see mbIsCurrClipRectangle
         */
        ::basegfx::B2DRectangle                             maCurrClipBounds;

        // sprite state
        ::basegfx::B2DPoint                                 maPosition;
        Size                                                maSize;
        ::basegfx::B2DHomMatrix                             maTransform;
        ::com::sun::star::uno::Reference<
              ::com::sun::star::rendering::XPolyPolygon2D > mxClipPoly;
        double                                              mfAlpha;
        bool                                                mbActive;

        /** If true, denotes that the current sprite clip is a true
            rectangle, i.e. maCurrClipBounds <em>exactly</em>
            describes the visible area of the sprite.

            @see maCurrClipBounds
         */
        bool                                                mbIsCurrClipRectangle;

        /** OutDev render speedup.

            When true, this flag denotes that the current
            mpBackBufferMask content is fully opaque, thus, that blits
            to the screen can use a plain Bitmap instead of the
            BitmapEx.
         */
        mutable bool                                        mbIsContentFullyOpaque;

        /// True, iff maTransform has changed
        mutable bool                                        mbTransformDirty;

    };
}

#endif /* _VCLCANVAS_CANVASCUSTOMSPRITE_HXX */
