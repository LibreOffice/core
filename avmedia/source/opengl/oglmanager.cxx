/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "oglmanager.hxx"
#include "oglframegrabber.hxx"
#include "oglplayer.hxx"

#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star;

namespace avmedia { namespace ogl {

OGLManager::OGLManager( const uno::Reference< lang::XMultiServiceFactory >& rMgr )
    : m_xMgr( rMgr )
{
    (void) m_xMgr;
}

OGLManager::~OGLManager()
{
}

uno::Reference< media::XPlayer > SAL_CALL OGLManager::createPlayer( const OUString& rURL )
    throw (uno::RuntimeException, std::exception)
{
    OGLPlayer* pPlayer( new OGLPlayer() );
    if( pPlayer->create(rURL) )
        return uno::Reference< media::XPlayer >(pPlayer);
    else
    {
        delete pPlayer;
        SAL_WARN("avmedia.opengl", "Can't create player for OpenGL model: " + rURL);
        return uno::Reference< media::XPlayer >();
    }
}

OUString SAL_CALL OGLManager::getImplementationName() throw ( uno::RuntimeException, std::exception )
{
    return OUString("com.sun.star.comp.avmedia.Manager_OpenGL");
}

sal_Bool SAL_CALL OGLManager::supportsService( const OUString& rServiceName )
        throw ( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL OGLManager::getSupportedServiceNames()
        throw ( uno::RuntimeException, std::exception )
{
    ::uno::Sequence< OUString > aRet { "com.sun.star.media.Manager_OpenGL" };
    return aRet;
}

} // namespace ogl
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
