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
#include <vcl/graphic.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/bitmapaccess.hxx>

#include <vcl/opengl/OpenGLHelper.hxx>

#include <memory>

using namespace com::sun::star;
using namespace libgltf;

namespace avmedia { namespace ogl {

OGLFrameGrabber::OGLFrameGrabber(  glTFHandle& rHandle )
    : FrameGrabber_BASE()
    , m_rHandle( rHandle )
{
}

OGLFrameGrabber::~OGLFrameGrabber()
{
}

uno::Reference< css::graphic::XGraphic > SAL_CALL OGLFrameGrabber::grabFrame( double /*fMediaTime*/ )
        throw ( uno::RuntimeException, std::exception )
{
    std::unique_ptr<sal_uInt8[]> pBuffer(new sal_uInt8[m_rHandle.viewport.width * m_rHandle.viewport.height * 4]);
    glTFHandle* pHandle = &m_rHandle;
    int nRet = gltf_renderer_get_bitmap(&pHandle, 1, reinterpret_cast<char*>(pBuffer.get()), GL_BGRA);
    if( nRet != 0 )
    {
        SAL_WARN("avmedia.opengl", "Error occurred while rendering to bitmap! Error code: " << nRet);
        return uno::Reference< css::graphic::XGraphic >();
    }
    BitmapEx aBitmap = OpenGLHelper::ConvertBGRABufferToBitmapEx(pBuffer.get(), m_rHandle.viewport.width, m_rHandle.viewport.height);
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
    uno::Sequence< OUString > aRet { "com.sun.star.media.FrameGrabber_OpenGL" };
    return aRet;
}

} // namespace ogl
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
