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

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/media/XPlayer.hpp>

#include <libgltf.h>

#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/timer.hxx>

#include <vector>

namespace avmedia { namespace ogl {

class OGLWindow;

typedef ::cppu::WeakComponentImplHelper< css::media::XPlayer, css::lang::XServiceInfo > Player_BASE;

class OGLPlayer : public cppu::BaseMutex,
                  public Player_BASE
{
public:

    OGLPlayer();
    virtual ~OGLPlayer() override;

    bool create( const OUString& rURL );
    void releaseInputFiles();

    // XPlayer
    virtual void SAL_CALL start() override;
    virtual void SAL_CALL stop() override;
    virtual sal_Bool SAL_CALL isPlaying() override;
    virtual double SAL_CALL getDuration() override;
    virtual void SAL_CALL setMediaTime( double fTime ) override;
    virtual double SAL_CALL getMediaTime() override;
    virtual void SAL_CALL setPlaybackLoop( sal_Bool bSet ) override;
    virtual sal_Bool SAL_CALL isPlaybackLoop() override;
    virtual void SAL_CALL setVolumeDB( sal_Int16 nVolumeDB ) override;
    virtual sal_Int16 SAL_CALL getVolumeDB() override;
    virtual void SAL_CALL setMute( sal_Bool bSet ) override;
    virtual sal_Bool SAL_CALL isMute() override;
    virtual css::awt::Size SAL_CALL getPreferredPlayerWindowSize() override;
    virtual css::uno::Reference< css::media::XPlayerWindow > SAL_CALL createPlayerWindow( const css::uno::Sequence< css::uno::Any >& rArguments ) override;
    virtual css::uno::Reference< css::media::XFrameGrabber > SAL_CALL createFrameGrabber() override;
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    DECL_LINK( TimerHandler, Timer*, void );

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
