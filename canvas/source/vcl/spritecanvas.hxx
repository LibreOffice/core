/*************************************************************************
 *
 *  $RCSfile: spritecanvas.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2004-03-18 10:38:44 $
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

#ifndef _VCLCANVAS_SPRITECANVAS_HXX_
#define _VCLCANVAS_SPRITECANVAS_HXX_

#include <memory>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX
#include <comphelper/uno3.hxx>
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XSPRITECANVAS_HPP_
#include <drafts/com/sun/star/rendering/XSpriteCanvas.hpp>
#endif

#include <canvas/vclwrapper.hxx>

#include "redrawmanager.hxx"
#include "spritesurface.hxx"
#include "outdevprovider.hxx"
#include "canvasbase.hxx"
#include "impltools.hxx"

class OutputDevice;
class Point;

namespace com { namespace sun { namespace star { namespace uno
{
    class XComponentContext;
    class RuntimeException;
} } } }

class Window;

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper4< ::drafts::com::sun::star::rendering::XSpriteCanvas,
                         ::com::sun::star::lang::XInitialization,
                         ::com::sun::star::lang::XServiceInfo,
                         ::com::sun::star::lang::XServiceName >  SpriteCanvas_Base;

    class SpriteCanvas : public ::comphelper::OBaseMutex,
                         public SpriteSurface,
                         public OutDevProvider,
                         public SpriteCanvas_Base

    {
    public:
        typedef ::rtl::Reference< SpriteCanvas > ImplRef;

        SpriteCanvas( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

        // XInterface

        // Need to implement that, because OutDevProvide comes with an
        // unimplemented version of XInterface.

        // Forwarding the XInterface implementation to the
        // cppu::ImplHelper templated base, which does the refcounting and
        // queryInterface for us:  Classname     Base doing refcount and handling queryInterface
        //                             |                 |
        //                             V                 V
        DECLARE_UNO3_AGG_DEFAULTS( SpriteCanvas, SpriteCanvas_Base );

        // XCanvas
        virtual void SAL_CALL drawPoint( const ::drafts::com::sun::star::geometry::RealPoint2D&     aPoint,
                                         const ::drafts::com::sun::star::rendering::ViewState&      viewState,
                                         const ::drafts::com::sun::star::rendering::RenderState&    renderState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL drawLine( const ::drafts::com::sun::star::geometry::RealPoint2D&  aStartPoint,
                                        const ::drafts::com::sun::star::geometry::RealPoint2D&  aEndPoint,
                                        const ::drafts::com::sun::star::rendering::ViewState&   viewState,
                                        const ::drafts::com::sun::star::rendering::RenderState& renderState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL drawBezier( const ::drafts::com::sun::star::geometry::RealBezierSegment2D&    aBezierSegment,
                                          const ::drafts::com::sun::star::geometry::RealPoint2D&            aEndPoint,
                                          const ::drafts::com::sun::star::rendering::ViewState&             viewState,
                                          const ::drafts::com::sun::star::rendering::RenderState&           renderState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawPolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >& xPolyPolygon,
                             const ::drafts::com::sun::star::rendering::ViewState&                                          viewState,
                             const ::drafts::com::sun::star::rendering::RenderState&                                        renderState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            strokePolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const ::drafts::com::sun::star::rendering::ViewState&                                            viewState,
                               const ::drafts::com::sun::star::rendering::RenderState&                                          renderState,
                               const ::drafts::com::sun::star::rendering::StrokeAttributes&                                     strokeAttributes ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            strokeTexturedPolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                                       const ::drafts::com::sun::star::rendering::ViewState&                                            viewState,
                                       const ::drafts::com::sun::star::rendering::RenderState&                                          renderState,
                                       const ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::rendering::Texture >&           textures,
                                       const ::drafts::com::sun::star::rendering::StrokeAttributes&                                     strokeAttributes ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            strokeTextureMappedPolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >&  xPolyPolygon,
                                            const ::drafts::com::sun::star::rendering::ViewState&                                           viewState,
                                            const ::drafts::com::sun::star::rendering::RenderState&                                         renderState,
                                            const ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::rendering::Texture >&          textures,
                                            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::geometry::XMapping2D >&       xMapping,
                                            const ::drafts::com::sun::star::rendering::StrokeAttributes&                                    strokeAttributes ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >   SAL_CALL
            queryStrokeShapes( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const ::drafts::com::sun::star::rendering::ViewState&                                            viewState,
                               const ::drafts::com::sun::star::rendering::RenderState&                                          renderState,
                               const ::drafts::com::sun::star::rendering::StrokeAttributes&                                     strokeAttributes ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            fillPolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >& xPolyPolygon,
                             const ::drafts::com::sun::star::rendering::ViewState&                                          viewState,
                             const ::drafts::com::sun::star::rendering::RenderState&                                        renderState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            fillTexturedPolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >& xPolyPolygon,
                                     const ::drafts::com::sun::star::rendering::ViewState&                                          viewState,
                                     const ::drafts::com::sun::star::rendering::RenderState&                                        renderState,
                                     const ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::rendering::Texture >&         textures ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            fillTextureMappedPolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
                                          const ::drafts::com::sun::star::rendering::ViewState&                                             viewState,
                                          const ::drafts::com::sun::star::rendering::RenderState&                                           renderState,
                                          const ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::rendering::Texture >&            textures,
                                          const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::geometry::XMapping2D >&         xMapping ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCanvasFont >      SAL_CALL
            queryFont( const ::drafts::com::sun::star::rendering::FontRequest&  fontRequest ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawText( const ::drafts::com::sun::star::rendering::StringContext&                                     text,
                      const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCanvasFont >&   xFont,
                      const ::drafts::com::sun::star::rendering::ViewState&                                         viewState,
                      const ::drafts::com::sun::star::rendering::RenderState&                                       renderState,
                      sal_Int8                                                                                      textDirection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawOffsettedText( const ::drafts::com::sun::star::rendering::StringContext&                                    text,
                               const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCanvasFont >&  xFont,
                               const ::com::sun::star::uno::Sequence< double >&                                             offsets,
                               const ::drafts::com::sun::star::rendering::ViewState&                                        viewState,
                               const ::drafts::com::sun::star::rendering::RenderState&                                      renderState,
                               sal_Int8                                                                                     textDirection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawBitmap( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XBitmap >& xBitmap,
                        const ::drafts::com::sun::star::rendering::ViewState&                                   viewState,
                        const ::drafts::com::sun::star::rendering::RenderState&                                 renderState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XGraphicDevice >   SAL_CALL
            getDevice() throw (::com::sun::star::uno::RuntimeException);

        // XBitmapCanvas (only providing, not implementing the
        // interface. Also note subtle method parameter differences)
        virtual void SAL_CALL copyRect( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XBitmapCanvas >&   sourceCanvas,
                                        const ::drafts::com::sun::star::geometry::RealRectangle2D&                                      sourceRect,
                                        const ::drafts::com::sun::star::rendering::ViewState&                                           sourceViewState,
                                        const ::drafts::com::sun::star::rendering::RenderState&                                         sourceRenderState,
                                        const ::drafts::com::sun::star::geometry::RealRectangle2D&                                      destRect,
                                        const ::drafts::com::sun::star::rendering::ViewState&                                           destViewState,
                                        const ::drafts::com::sun::star::rendering::RenderState&                                         destRenderState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        // XSpriteCanvas
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XAnimatedSprite > SAL_CALL createSpriteFromAnimation( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XAnimation >& animation ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XAnimatedSprite > SAL_CALL createSpriteFromBitmaps( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XBitmap > >&   animationBitmaps,
                                                                                                                                           sal_Int16                                                                                                                    interpolationMode ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCustomSprite > SAL_CALL createCustomSprite( const ::drafts::com::sun::star::geometry::RealSize2D& spriteSize ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XSprite > SAL_CALL createClonedSprite( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XSprite >& original ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL updateScreen() throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw( ::com::sun::star::uno::Exception,
                                                                                                                                   ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw( ::com::sun::star::uno::RuntimeException );

        // XServiceName
        virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw (::com::sun::star::uno::RuntimeException);

        // factory
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext ) throw ( ::com::sun::star::uno::Exception );

        // SpriteSurface
        virtual void showSprite( const Sprite::ImplRef& sprite );
        virtual void hideSprite( const Sprite::ImplRef& sprite );
        virtual void moveSprite( const Sprite::ImplRef& sprite,
                                 const Point&           rOldPos,
                                 const Point&           rNewPos );
        virtual void updateSprite( const Sprite::ImplRef&   sprite,
                                   const Point&             rPos,
                                   const Rectangle&         rUpdateArea );

        // OutDevProvider
        virtual OutputDevice&       getOutDev();
        virtual const OutputDevice& getOutDev() const;

    protected:
        ~SpriteCanvas(); // we're a ref-counted UNO class. _We_ destroy ourselves.

    private:
        // default: disabled copy/assignment
        SpriteCanvas(const SpriteCanvas&);
        SpriteCanvas& operator=( const SpriteCanvas& );

        void checkOurState(); // throws
        OutDevProvider::ImplRef     getImplRef();

        // TODO: Lifetime issue. Cannot control pointer validity over
        // object lifetime, since we're a UNO component
        Window*                                         mpOutputWindow;     // for the screen output
        ::canvas::vcltools::VCLObject<VirtualDevice>    maVDev;             // for the back-buffer
        ::std::auto_ptr< RedrawManager >                mpRedrawManager;    // handles smooth screen updates for us
        CanvasBase                                      maCanvasHelper;     // for the basic canvas implementation
    };
}

#endif
