/*************************************************************************
 *
 *  $RCSfile: spritecanvas.cxx,v $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
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
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XSPRITECANVAS_HPP_
#include <drafts/com/sun/star/rendering/XSpriteCanvas.hpp>
#endif

#ifndef BOOST_SCOPED_ARRAY_HPP_INCLUDED
#include <external/boost/scoped_array.hpp>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#include <algorithm>

#include <canvas/verbosetrace.hxx>

#include <canvas/canvastools.hxx>

#include "impltools.hxx"
#include "canvasfont.hxx"
#include "spritecanvas.hxx"
#include "graphicdevice.hxx"
#include "outdevprovider.hxx"
#include "canvascustomsprite.hxx"
#include "bitmapcanvas.hxx"

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

#define IMPLEMENTATION_NAME "VCLCanvas::SpriteCanvas"
#define SERVICE_NAME "drafts.com.sun.star.rendering.Canvas"

namespace
{
    static ::rtl::OUString SAL_CALL getImplementationName_SpriteCanvas()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    static uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_SpriteCanvas()
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

}

namespace vclcanvas
{
    SpriteCanvas::SpriteCanvas( const uno::Reference< uno::XComponentContext >& rxContext ) :
        SpriteCanvas_Base( m_aMutex )
    {
    }

    SpriteCanvas::~SpriteCanvas()
    {
    }

    OutDevProvider::ImplRef SpriteCanvas::getImplRef()
    {
        return OutDevProvider::ImplRef::createFromQuery( this );
    }

    void SAL_CALL SpriteCanvas::drawPoint( const geometry::RealPoint2D&         aPoint,
                                           const rendering::ViewState&      viewState,
                                           const rendering::RenderState&    renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        mpRedrawManager->backgroundDirty();
        maCanvasHelper.drawPoint( aPoint, viewState, renderState, getImplRef() );
    }

    void SAL_CALL SpriteCanvas::drawLine( const geometry::RealPoint2D& aStartPoint, const geometry::RealPoint2D& aEndPoint, const rendering::ViewState& viewState, const rendering::RenderState& renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        mpRedrawManager->backgroundDirty();
        maCanvasHelper.drawLine(aStartPoint, aEndPoint, viewState, renderState, getImplRef());
    }

    void SAL_CALL SpriteCanvas::drawBezier( const geometry::RealBezierSegment2D&    aBezierSegment,
                                            const geometry::RealPoint2D&            aEndPoint,
                                            const rendering::ViewState&             viewState,
                                            const rendering::RenderState&           renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        mpRedrawManager->backgroundDirty();
        maCanvasHelper.drawBezier(aBezierSegment, aEndPoint, viewState, renderState, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL SpriteCanvas::drawPolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        // TODO: Remember to handle backgroundDirty also for XCachedPrimitive redraw!
        mpRedrawManager->backgroundDirty();
        return maCanvasHelper.drawPolyPolygon(xPolyPolygon, viewState, renderState, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL SpriteCanvas::strokePolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState, const rendering::StrokeAttributes& strokeAttributes ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        mpRedrawManager->backgroundDirty();
        return maCanvasHelper.strokePolyPolygon(xPolyPolygon, viewState, renderState, strokeAttributes, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL SpriteCanvas::strokeTexturedPolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState, const uno::Sequence< rendering::Texture >& textures, const rendering::StrokeAttributes& strokeAttributes ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        mpRedrawManager->backgroundDirty();
        return maCanvasHelper.strokeTexturedPolyPolygon(xPolyPolygon, viewState, renderState, textures, strokeAttributes, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL SpriteCanvas::strokeTextureMappedPolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState, const uno::Sequence< rendering::Texture >& textures, const uno::Reference< geometry::XMapping2D >& xMapping, const rendering::StrokeAttributes& strokeAttributes ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        mpRedrawManager->backgroundDirty();
        return maCanvasHelper.strokeTextureMappedPolyPolygon(xPolyPolygon, viewState, renderState, textures, xMapping, strokeAttributes, getImplRef());
    }

    uno::Reference< rendering::XPolyPolygon2D > SAL_CALL SpriteCanvas::queryStrokeShapes( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState, const rendering::StrokeAttributes& strokeAttributes ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        return maCanvasHelper.queryStrokeShapes(xPolyPolygon, viewState, renderState, strokeAttributes, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL SpriteCanvas::fillPolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        mpRedrawManager->backgroundDirty();
        return maCanvasHelper.fillPolyPolygon(xPolyPolygon, viewState, renderState, getImplRef() );
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL SpriteCanvas::fillTexturedPolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState, const uno::Sequence< rendering::Texture >& textures ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        mpRedrawManager->backgroundDirty();
        return maCanvasHelper.fillTexturedPolyPolygon(xPolyPolygon, viewState, renderState, textures, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL SpriteCanvas::fillTextureMappedPolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState, const uno::Sequence< rendering::Texture >& textures, const uno::Reference< geometry::XMapping2D >& xMapping ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        mpRedrawManager->backgroundDirty();
        return maCanvasHelper.fillTextureMappedPolyPolygon(xPolyPolygon, viewState, renderState, textures, xMapping, getImplRef());
    }

    uno::Reference< rendering::XCanvasFont > SAL_CALL SpriteCanvas::queryFont( const rendering::FontRequest& fontRequest ) throw (uno::RuntimeException)
    {
        return maCanvasHelper.queryFont( fontRequest, getImplRef() );
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL SpriteCanvas::drawText( const rendering::StringContext&                  text,
                                                                                   const uno::Reference< rendering::XCanvasFont >&  xFont,
                                                                                   const rendering::ViewState&                      viewState,
                                                                                   const rendering::RenderState&                    renderState,
                                                                                   sal_Int8                                         textDirection ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        mpRedrawManager->backgroundDirty();
        return maCanvasHelper.drawText(text, xFont, viewState, renderState, textDirection, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL SpriteCanvas::drawOffsettedText( const rendering::StringContext& text, const uno::Reference< rendering::XCanvasFont >& xFont, const uno::Sequence< double >& offsets, const rendering::ViewState& viewState, const rendering::RenderState& renderState, sal_Int8 textDirection ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        mpRedrawManager->backgroundDirty();
        return maCanvasHelper.drawOffsettedText(text, xFont, offsets, viewState, renderState, textDirection, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL SpriteCanvas::drawBitmap( const uno::Reference< rendering::XBitmap >& xBitmap, const rendering::ViewState& viewState, const rendering::RenderState& renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        mpRedrawManager->backgroundDirty();
        return maCanvasHelper.drawBitmap(xBitmap, viewState, renderState, getImplRef());
    }

    uno::Reference< rendering::XGraphicDevice > SAL_CALL SpriteCanvas::getDevice() throw (uno::RuntimeException)
    {
        return maCanvasHelper.getDevice( getImplRef() );
    }

    void SAL_CALL SpriteCanvas::copyRect( const uno::Reference< rendering::XBitmapCanvas >& sourceCanvas,
                                          const geometry::RealRectangle2D&                  sourceRect,
                                          const rendering::ViewState&                       sourceViewState,
                                          const rendering::RenderState&                     sourceRenderState,
                                          const geometry::RealRectangle2D&                  destRect,
                                          const rendering::ViewState&                       destViewState,
                                          const rendering::RenderState&                     destRenderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        checkOurState();

        tools::LocalGuard aGuard;

        mpRedrawManager->backgroundDirty();
    }

    uno::Reference< rendering::XAnimatedSprite > SAL_CALL SpriteCanvas::createSpriteFromAnimation( const uno::Reference< rendering::XAnimation >& animation ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return uno::Reference< rendering::XAnimatedSprite >(NULL);
    }

    uno::Reference< rendering::XAnimatedSprite > SAL_CALL SpriteCanvas::createSpriteFromBitmaps( const uno::Sequence< uno::Reference< rendering::XBitmap > >&   animationBitmaps,
                                                                                                 sal_Int16                                                      interpolationMode ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return uno::Reference< rendering::XAnimatedSprite >(NULL);
    }

    uno::Reference< rendering::XCustomSprite > SAL_CALL SpriteCanvas::createCustomSprite( const geometry::RealSize2D& spriteSize ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return uno::Reference< rendering::XCustomSprite >(
            new CanvasCustomSprite( spriteSize,
                                    ImplRef(this)) );
    }

    uno::Reference< rendering::XSprite > SAL_CALL SpriteCanvas::createClonedSprite( const uno::Reference< rendering::XSprite >& original ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return uno::Reference< rendering::XSprite >(NULL);
    }

    sal_Bool SAL_CALL SpriteCanvas::updateScreen() throw (uno::RuntimeException)
    {
        checkOurState();

        tools::LocalGuard aGuard;

        mpRedrawManager->updateScreen();

        // commit to screen
        maCanvasHelper.flush();

        return sal_True;
    }

    void SAL_CALL SpriteCanvas::initialize( const uno::Sequence< uno::Any >& aArguments ) throw( uno::Exception,
                                                                                                 uno::RuntimeException)
    {
        VERBOSE_TRACE( "SpriteCanvas::initialize called" );

        OSL_ENSURE( aArguments.getLength() >= 1,
                    "SpriteCanvas::initialize: wrong number of arguments" );

        // We expect a single Any here, containing a pointer to a valid
        // VCL window, on which to output
        if( aArguments.getLength() >= 1 &&
            aArguments[0].getValueTypeClass() == uno::TypeClass_HYPER )
        {
            mpOutputWindow = *(Window**)(aArguments[0].getValue());
            OSL_ENSURE( mpOutputWindow != NULL,
                        "SpriteCanvas::initialize: invalid Window pointer" );

            // setup back buffer
            maVDev->SetOutputSizePixel( mpOutputWindow->GetOutputSizePixel() );

            // always render into back buffer
            maCanvasHelper.setOutDev( *maVDev );

            mpRedrawManager = ::std::auto_ptr< RedrawManager >( new RedrawManager( *mpOutputWindow,
                                                                                   *maVDev ) );
        }
    }

    ::rtl::OUString SAL_CALL SpriteCanvas::getImplementationName() throw( uno::RuntimeException )
    {
        return getImplementationName_SpriteCanvas();
    }

    sal_Bool SAL_CALL SpriteCanvas::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL SpriteCanvas::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        return getSupportedServiceNames_SpriteCanvas();
    }

    ::rtl::OUString SAL_CALL SpriteCanvas::getServiceName(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
    }

    uno::Reference< uno::XInterface > SAL_CALL SpriteCanvas::createInstance( const uno::Reference< uno::XComponentContext >& xContext ) throw ( uno::Exception )
    {
        return uno::Reference< uno::XInterface >( static_cast<cppu::OWeakObject*>(new SpriteCanvas( xContext )) );
    }

    // SpriteSurface
    void SpriteCanvas::showSprite( const Sprite::ImplRef& sprite )
    {
        checkOurState();

        tools::LocalGuard aGuard;
        mpRedrawManager->showSprite( sprite );
    }

    void SpriteCanvas::hideSprite( const Sprite::ImplRef& sprite )
    {
        checkOurState();

        // strictly speaking, the solar mutex here is overkill, and a
        // object mutex would suffice. But on the other hand, nearly
        // every other method needs the solar mutex anyway, so it's no
        // big loss (and much simpler) here.
        tools::LocalGuard aGuard;
        mpRedrawManager->hideSprite( sprite );
    }

    void SpriteCanvas::moveSprite( const Sprite::ImplRef&   sprite,
                                   const Point&             rOldPos,
                                   const Point&             rNewPos )
    {
        checkOurState();

        tools::LocalGuard aGuard;
        mpRedrawManager->moveSprite( sprite, rOldPos, rNewPos );
    }

    void SpriteCanvas::updateSprite( const Sprite::ImplRef& sprite,
                                     const Point&           rPos,
                                     const Rectangle&       rUpdateArea )
    {
        checkOurState();

        tools::LocalGuard aGuard;
        mpRedrawManager->updateSprite( sprite, rPos, rUpdateArea );
    }

    // OutDevProvider
    ::OutputDevice& SpriteCanvas::getOutDev()
    {
        return maCanvasHelper.getOutDev();
    }

    const ::OutputDevice& SpriteCanvas::getOutDev() const
    {
        return maCanvasHelper.getOutDev();
    }

    void SpriteCanvas::checkOurState()
    {
        if( mpOutputWindow == NULL ||
            mpRedrawManager.get() == NULL )
            throw uno::RuntimeException();
    }

}

namespace
{
    /* shared lib exports implemented with helpers */
    static struct ::cppu::ImplementationEntry s_component_entries [] =
    {
        {
            vclcanvas::SpriteCanvas::createInstance, getImplementationName_SpriteCanvas,
            getSupportedServiceNames_SpriteCanvas, ::cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };
}


/* Exported UNO methods for registration and object creation.
   ==========================================================
 */
extern "C"
{
    void SAL_CALL component_getImplementationEnvironment( const sal_Char**  ppEnvTypeName,
                                                          uno_Environment** ppEnv )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo( lang::XMultiServiceFactory*  xMgr,
                                           registry::XRegistryKey*      xRegistry )
    {
        return ::cppu::component_writeInfoHelper(
            xMgr, xRegistry, s_component_entries );
    }

    void * SAL_CALL component_getFactory( sal_Char const*               implName,
                                          lang::XMultiServiceFactory*   xMgr,
                                          registry::XRegistryKey*       xRegistry )
    {
        return ::cppu::component_getFactoryHelper(
            implName, xMgr, xRegistry, s_component_entries );
    }
}
