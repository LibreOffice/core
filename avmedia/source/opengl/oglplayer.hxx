/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_AVMEDIA_SOURCE_OPENGL_OGLPLAYER_HXX
#define INCLUDED_AVMEDIA_SOURCE_OPENGL_OGLPLAYER_HXX

#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/media/XPlayer.hpp>

#include <libgltf.h>

#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/timer.hxx>

#include <vector>

namespace avmedia { namespace ogl {

class OGLWindow;

typedef ::cppu::WeakComponentImplHelper2< com::sun::star::media::XPlayer,
                                          com::sun::star::lang::XServiceInfo > Player_BASE;

class OGLPlayer : public cppu::BaseMutex,
                  public Player_BASE
{
public:

    OGLPlayer();
    virtual ~OGLPlayer();

    bool create( const OUString& rURL );
    void releaseInputFiles();

    // XPlayer
    virtual void SAL_CALL start() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL stop() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isPlaying() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getDuration() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setMediaTime( double fTime ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getMediaTime() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPlaybackLoop( sal_Bool bSet ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isPlaybackLoop() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setVolumeDB( sal_Int16 nVolumeDB ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getVolumeDB() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setMute( sal_Bool bSet ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isMute() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual com::sun::star::awt::Size SAL_CALL getPreferredPlayerWindowSize() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual com::sun::star::uno::Reference< com::sun::star::media::XPlayerWindow > SAL_CALL createPlayerWindow( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& rArguments ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual com::sun::star::uno::Reference< com::sun::star::media::XFrameGrabber > SAL_CALL createFrameGrabber() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    DECL_LINK_TYPED( TimerHandler, Timer*, void );

    OUString m_sURL;

    libgltf::glTFHandle* m_pHandle;
    std::vector<libgltf::glTFFile> m_vInputFiles;

    rtl::Reference<OpenGLContext> m_xContext;
    AutoTimer m_aTimer;
    OGLWindow* m_pOGLWindow;
    bool m_bIsRendering;
};

} // namespace ogl
} // namespace avmedia

#endif // INCLUDED_AVMEDIA_SOURCE_OPENGL_OGLPLAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
