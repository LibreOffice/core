/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "oglframegrabber.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/graph.hxx>

using namespace com::sun::star;

namespace avmedia { namespace ogl {

OGLFrameGrabber::OGLFrameGrabber( const OUString& /*rUrl*/ )
    : FrameGrabber_BASE()
{
}

OGLFrameGrabber::~OGLFrameGrabber()
{
}

uno::Reference< css::graphic::XGraphic > SAL_CALL OGLFrameGrabber::grabFrame( double /*fMediaTime*/ )
        throw ( uno::RuntimeException, std::exception )
{
    // TODO: Here we need a bitmap of the model at the point specified by fMediaTime
    // See com::sun::star::media::XFrameGrabber
    BitmapEx aBitmap;
    return Graphic( aBitmap ).GetXGraphic();
}

OUString SAL_CALL OGLFrameGrabber::getImplementationName() throw ( uno::RuntimeException, std::exception )
{
    return OUString("com.sun.star.comp.avmedia.FrameGrabber_OpenGL");
}

sal_Bool SAL_CALL OGLFrameGrabber::supportsService( const OUString& rServiceName )
        throw ( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL OGLFrameGrabber::getSupportedServiceNames()
        throw ( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(1);
    aRet[0] = OUString("com.sun.star.media.FrameGrabber_OpenGL");
    return aRet;
}

} // namespace ogl
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
