/*************************************************************************
 *
 *  $RCSfile: canvascustomsprite.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 17:11:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XCUSTOMSPRITE_HPP_
#include <drafts/com/sun/star/rendering/XCustomSprite.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP_
#include <drafts/com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
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


#define CANVASCUSTOMSPRITE_IMPLEMENTATION_NAME "VCLCanvas::CanvasCustomSprite"

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper3< ::drafts::com::sun::star::rendering::XCustomSprite,
                                               ::drafts::com::sun::star::rendering::XBitmapCanvas,
                                                ::com::sun::star::lang::XServiceInfo >                                  CanvasCustomSpriteBase_Base;
    typedef ::canvas::internal::BitmapCanvasBase< CanvasCustomSpriteBase_Base, CanvasHelper, tools::LocalGuard >    CanvasCustomSprite_Base;

    /* Definition of CanvasCustomSprite class */

    class CanvasCustomSprite : public Sprite,
                               public CanvasCustomSprite_Base
    {
    public:
        CanvasCustomSprite( const ::drafts::com::sun::star::geometry::RealSize2D&   rSpriteSize,
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

        // XSprite
        virtual void SAL_CALL setAlpha( double alpha ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL move( const ::drafts::com::sun::star::geometry::RealPoint2D& aNewPos, const ::drafts::com::sun::star::rendering::ViewState& viewState, const ::drafts::com::sun::star::rendering::RenderState& renderState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL transform( const ::drafts::com::sun::star::geometry::AffineMatrix2D& aTransformation ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clip( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >& aClip ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPriority( double nPriority ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL show(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL hide(  ) throw (::com::sun::star::uno::RuntimeException);

        // XCustomSprite
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCanvas > SAL_CALL
            getContentCanvas(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw( ::com::sun::star::uno::RuntimeException );

        // Sprite
        virtual void redraw( OutputDevice& rTargetSurface ) const;
        virtual void redraw( OutputDevice& rTargetSurface, const Point& rOutputPosition ) const;
        virtual ::basegfx::B2DPoint getPos() const;
        virtual ::basegfx::B2DSize  getSize() const;

    protected:
        ~CanvasCustomSprite(); // we're a ref-counted UNO class. _We_ destroy ourselves.

    private:
        // default: disabled copy/assignment
        CanvasCustomSprite(const CanvasCustomSprite&);
        CanvasCustomSprite& operator=( const CanvasCustomSprite& );

        // for the integrated bitmap canvas implementation
        BackBufferSharedPtr     mpBackBuffer;
        BackBufferSharedPtr     mpBackBufferMask;

        SpriteCanvas::ImplRef   mpSpriteCanvas;

        mutable ::canvas::vcltools::VCLObject<BitmapEx>             maContent;

        // sprite state
        ::basegfx::B2DPoint                                         maPosition;
        Size                                                        maSize;
        ::com::sun::star::uno::Reference<
              ::drafts::com::sun::star::rendering::XPolyPolygon2D > mxClipPoly;
        double                                                      mfAlpha;
        bool                                                        mbActive;
    };
}

#endif /* _VCLCANVAS_CANVASCUSTOMSPRITE_HXX */
