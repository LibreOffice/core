/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/log.hxx>

#include <com/sun/star/lang/NoSupportException.hpp>
#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/window.hxx>

#include "ogl_canvascustomsprite.hxx"
#include "ogl_spritecanvas.hxx"

using namespace ::com::sun::star;

namespace oglcanvas
{
    SpriteCanvas::SpriteCanvas( const uno::Sequence< uno::Any >&                aArguments,
                                const uno::Reference< uno::XComponentContext >& /*rxContext*/ ) :
        maArguments(aArguments)
    {
    }

    void SpriteCanvas::initialize()
    {
        // Only call initialize when not in probe mode
        if( !maArguments.hasElements() )
            return;

        SAL_INFO("canvas.ogl", "SpriteCanvas::initialize called" );

        /* aArguments:
           0: ptr to creating instance (Window or VirtualDevice)
           1: current bounds of creating instance
           2: bool, denoting always on top state for Window (always false for VirtualDevice)
           3: XWindow for creating Window (or empty for VirtualDevice)
           4: SystemGraphicsData as a streamed Any
         */
        ENSURE_ARG_OR_THROW( maArguments.getLength() >= 4 &&
                             maArguments[3].getValueTypeClass() == uno::TypeClass_INTERFACE,
                             "OpenGL SpriteCanvas::initialize: wrong number of arguments, or wrong types" );

        uno::Reference< awt::XWindow > xParentWindow;
        maArguments[3] >>= xParentWindow;
        VclPtr<vcl::Window> pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
        if( !pParentWindow )
            throw lang::NoSupportException(
                u"Parent window not VCL window, or canvas out-of-process!"_ustr, nullptr);

        awt::Rectangle aRect;
        maArguments[1] >>= aRect;

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

        // forward to parent
        SpriteCanvasBaseT::disposeThis();
    }

    sal_Bool SAL_CALL SpriteCanvas::showBuffer( sal_Bool bUpdateAll )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return mbIsVisible && SpriteCanvasBaseT::showBuffer( bUpdateAll );
    }

    sal_Bool SAL_CALL SpriteCanvas::switchBuffer( sal_Bool bUpdateAll )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return mbIsVisible && SpriteCanvasBaseT::switchBuffer( bUpdateAll );
    }

    uno::Reference< rendering::XAnimatedSprite > SAL_CALL SpriteCanvas::createSpriteFromAnimation(
        const uno::Reference< rendering::XAnimation >& /*animation*/ )
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XAnimatedSprite > SAL_CALL SpriteCanvas::createSpriteFromBitmaps(
        const uno::Sequence< uno::Reference< rendering::XBitmap > >& /*animationBitmaps*/,
        ::sal_Int8 /*interpolationMode*/ )
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XCustomSprite > SAL_CALL SpriteCanvas::createCustomSprite(
        const geometry::RealSize2D& spriteSize )
    {
        return uno::Reference< rendering::XCustomSprite >(
            new CanvasCustomSprite(spriteSize, this, maDeviceHelper) );
    }

    uno::Reference< rendering::XSprite > SAL_CALL SpriteCanvas::createClonedSprite(
        const uno::Reference< rendering::XSprite >& /*original*/ )
    {
        return uno::Reference< rendering::XSprite >();
    }

    sal_Bool SAL_CALL SpriteCanvas::updateScreen(sal_Bool bUpdateAll)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return maDeviceHelper.showBuffer(mbIsVisible, bUpdateAll);
    }

    OUString SAL_CALL SpriteCanvas::getServiceName(  )
    {
        return u"com.sun.star.rendering.SpriteCanvas.OGL"_ustr;
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

    void SpriteCanvas::renderRecordedActions() const
    {
        maCanvasHelper.renderRecordedActions();
    }

}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_rendering_SpriteCanvas_OGL_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& args)
{
    if( !OpenGLHelper::supportsOpenGL())
        return nullptr;
    rtl::Reference<oglcanvas::SpriteCanvas> p = new oglcanvas::SpriteCanvas(args, context);
    p->initialize();
    return cppu::acquire(p.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
