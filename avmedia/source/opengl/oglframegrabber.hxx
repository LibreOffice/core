/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_AVMEDIA_SOURCE_OPENGL_OGLFRAMEGRABBER_HXX
#define INCLUDED_AVMEDIA_SOURCE_OPENGL_OGLFRAMEGRABBER_HXX

#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/media/XFrameGrabber.hpp>

#include <libgltf.h>

namespace avmedia { namespace ogl {

typedef ::cppu::WeakImplHelper2< com::sun::star::media::XFrameGrabber,
                                 com::sun::star::lang::XServiceInfo > FrameGrabber_BASE;

class OGLFrameGrabber : public FrameGrabber_BASE
{
public:

    explicit OGLFrameGrabber( libgltf::glTFHandle& rHandle );
    virtual ~OGLFrameGrabber();

    // XFrameGrabber
    virtual com::sun::star::uno::Reference< com::sun::star::graphic::XGraphic > SAL_CALL grabFrame( double fMediaTime ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    libgltf::glTFHandle& m_rHandle;
};

}
}

#endif // INCLUDED_AVMEDIA_SOURCE_OPENGL_OGLFRAMEGRABBER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
