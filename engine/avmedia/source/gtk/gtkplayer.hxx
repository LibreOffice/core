/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/media/XPlayer.hpp>
#include <com/sun/star/media/XPlayerNotifier.hpp>
#include <comphelper/multicontainer2.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

typedef struct _GtkMediaStream GtkMediaStream;
typedef struct _GtkWidget GtkWidget;

namespace avmedia::gtk
{
typedef cppu::WeakComponentImplHelper<css::media::XPlayer, css::media::XPlayerNotifier,
                                      css::lang::XServiceInfo>
    GtkPlayer_BASE;

class GtkPlayer final : public cppu::BaseMutex, public GtkPlayer_BASE
{
public:
    explicit GtkPlayer();
    virtual ~GtkPlayer() override;

    bool create(const OUString& rURL);

    virtual void start() override;
    virtual void stop() override;
    virtual bool isPlaying() override;
    virtual double getDuration() override;
    virtual void setMediaTime(double fTime) override;
    virtual double getMediaTime() override;
    virtual void setPlaybackLoop(bool bSet) override;
    virtual bool isPlaybackLoop() override;
    virtual void setMute(bool bSet) override;
    virtual bool isMute() override;
    virtual void setVolumeDB(sal_Int16 nVolumeDB) override;
    virtual sal_Int16 getVolumeDB() override;
    virtual css::awt::Size getPreferredPlayerWindowSize() override;
    virtual css::uno::Reference<css::media::XPlayerWindow>
        createPlayerWindow(const cpo::uno::Sequence<cpo::uno::Any>& rArgs) override;
    virtual css::uno::Reference<css::media::XFrameGrabber> createFrameGrabber() override;

    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    virtual void
    addPlayerListener(const css::uno::Reference<css::media::XPlayerListener>& rListener) override;
    virtual void removePlayerListener(
        const css::uno::Reference<css::media::XPlayerListener>& rListener) override;

    virtual void disposing() final override;

    void notifyListeners();
    void installNotify();
    void uninstallNotify();

private:
    void cleanup();

    comphelper::OMultiTypeInterfaceContainerHelper2 m_lListener;

    OUString m_aURL;
    css::awt::Rectangle m_aArea; // Area of the player window.
    GtkMediaStream* m_pStream;
    GtkWidget* m_pVideo;
    unsigned long m_nNotifySignalId;
    unsigned long m_nInvalidateSizeSignalId;
    unsigned long m_nTimeoutId;
    sal_Int16 m_nUnmutedVolume;
};

} // namespace avmedia::gtk

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
