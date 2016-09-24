/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/servicedecl.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>

#include "ogl_canvascustomsprite.hxx"
#include "ogl_spritecanvas.hxx"

#define SPRITECANVAS_SERVICE_NAME        "com.sun.star.rendering.SpriteCanvas.OGL"
#define SPRITECANVAS_IMPLEMENTATION_NAME "com.sun.star.comp.rendering.SpriteCanvas.OGL"


using namespace ::com::sun::star;

namespace sdecl = comphelper::service_decl;

namespace oglcanvas
{
    SpriteCanvas::SpriteCanvas( const uno::Sequence< uno::Any >&                aArguments,
                                const uno::Reference< uno::XComponentContext >& rxContext ) :
        maArguments(aArguments),
        mxComponentContext( rxContext )
    {
    }

    void SpriteCanvas::initialize()
    {
        // Only call initialize when not in probe mode
        if( maArguments.getLength() == 0 )
            return;

        SAL_INFO("canvas.ogl", "SpriteCanvas::initialize called" );

        /* aArguments:
           0: ptr to creating instance (Window or VirtualDevice)
           1: SystemEnvData as a streamed Any (or empty for VirtualDevice)
           2: current bounds of creating instance
           3: bool, denoting always on top state for Window (always false for VirtualDevice)
           4: XWindow for creating Window (or empty for VirtualDevice)
           5: SystemGraphicsData as a streamed Any
         */
        ENSURE_ARG_OR_THROW( maArguments.getLength() >= 5 &&
                             maArguments[4].getValueTypeClass() == uno::TypeClass_INTERFACE,
                             "OpenGL SpriteCanvas::initialize: wrong number of arguments, or wrong types" );

        uno::Reference< awt::XWindow > xParentWindow;
        maArguments[4] >>= xParentWindow;
        vcl::Window* pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
        if( !pParentWindow )
            throw lang::NoSupportException(
                "Parent window not VCL window, or canvas out-of-process!", nullptr);

        awt::Rectangle aRect;
        maArguments[2] >>= aRect;

        // setup helper
        maDeviceHelper.init( *pParentWindow,
                             *this,
                             aRect );
        maCanvasHelper.init( *this, maDeviceHelper );
        maArguments.realloc(0);
    }

    void SpriteCanvas::disposeThis()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mxComponentContext.clear();

        // forward to parent
        SpriteCanvasBaseT::disposeThis();
    }

    sal_Bool SAL_CALL SpriteCanvas::showBuffer( sal_Bool bUpdateAll ) throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return mbIsVisible && SpriteCanvasBaseT::showBuffer( bUpdateAll );
    }

    sal_Bool SAL_CALL SpriteCanvas::switchBuffer( sal_Bool bUpdateAll ) throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return mbIsVisible && SpriteCanvasBaseT::switchBuffer( bUpdateAll );
    }

    uno::Reference< rendering::XAnimatedSprite > SAL_CALL SpriteCanvas::createSpriteFromAnimation(
        const uno::Reference< rendering::XAnimation >& /*animation*/ ) throw (lang::IllegalArgumentException,
                                                                              uno::RuntimeException, std::exception)
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XAnimatedSprite > SAL_CALL SpriteCanvas::createSpriteFromBitmaps(
        const uno::Sequence< uno::Reference< rendering::XBitmap > >& /*animationBitmaps*/,
        ::sal_Int8 /*interpolationMode*/ ) throw (lang::IllegalArgumentException,
                                                  rendering::VolatileContentDestroyedException,
                                                  uno::RuntimeException, std::exception)
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XCustomSprite > SAL_CALL SpriteCanvas::createCustomSprite(
        const geometry::RealSize2D& spriteSize ) throw (lang::IllegalArgumentException,
                                                        uno::RuntimeException, std::exception)
    {
        return uno::Reference< rendering::XCustomSprite >(
            new CanvasCustomSprite(spriteSize, this, maDeviceHelper) );
    }

    uno::Reference< rendering::XSprite > SAL_CALL SpriteCanvas::createClonedSprite(
        const uno::Reference< rendering::XSprite >& /*original*/ ) throw (lang::IllegalArgumentException,
                                                                          uno::RuntimeException, std::exception)
    {
        return uno::Reference< rendering::XSprite >();
    }

    sal_Bool SAL_CALL SpriteCanvas::updateScreen(sal_Bool bUpdateAll)
        throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return maDeviceHelper.showBuffer(mbIsVisible, bUpdateAll);
    }

    ::rtl::OUString SAL_CALL SpriteCanvas::getServiceName(  ) throw (uno::RuntimeException, std::exception)
    {
        return ::rtl::OUString( SPRITECANVAS_SERVICE_NAME );
    }

    void SpriteCanvas::show( const ::rtl::Reference< CanvasCustomSprite >& xSprite )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        maDeviceHelper.show(xSprite);
    }

    void SpriteCanvas::hide( const ::rtl::Reference< CanvasCustomSprite >& xSprite )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        maDeviceHelper.hide(xSprite);
    }

    bool SpriteCanvas::renderRecordedActions() const
    {
        return maCanvasHelper.renderRecordedActions();
    }

    static uno::Reference<uno::XInterface> initCanvas( SpriteCanvas* pCanvas )
    {
        uno::Reference<uno::XInterface> xRet(static_cast<cppu::OWeakObject*>(pCanvas));
        pCanvas->initialize();
        return xRet;
    }

    sdecl::class_<SpriteCanvas, sdecl::with_args<true> > serviceImpl(&initCanvas);
    const sdecl::ServiceDecl oglSpriteCanvasDecl(
        serviceImpl,
        SPRITECANVAS_IMPLEMENTATION_NAME,
        SPRITECANVAS_SERVICE_NAME );
}

// The C shared lib entry points
extern "C"
SAL_DLLPUBLIC_EXPORT void* SAL_CALL oglcanvas_component_getFactory( sal_Char const* pImplName,
                                         void*, void* )
{
    return sdecl::component_getFactoryHelper( pImplName, {&oglcanvas::oglSpriteCanvasDecl} );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
