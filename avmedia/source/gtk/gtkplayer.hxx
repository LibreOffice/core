/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
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

    virtual void SAL_CALL start() override;
    virtual void SAL_CALL stop() override;
    virtual sal_Bool SAL_CALL isPlaying() override;
    virtual double SAL_CALL getDuration() override;
    virtual void SAL_CALL setMediaTime(double fTime) override;
    virtual double SAL_CALL getMediaTime() override;
    virtual void SAL_CALL setPlaybackLoop(sal_Bool bSet) override;
    virtual sal_Bool SAL_CALL isPlaybackLoop() override;
    virtual void SAL_CALL setMute(sal_Bool bSet) override;
    virtual sal_Bool SAL_CALL isMute() override;
    virtual void SAL_CALL setVolumeDB(sal_Int16 nVolumeDB) override;
    virtual sal_Int16 SAL_CALL getVolumeDB() override;
    virtual css::awt::Size SAL_CALL getPreferredPlayerWindowSize() override;
    virtual css::uno::Reference<css::media::XPlayerWindow>
        SAL_CALL createPlayerWindow(const css::uno::Sequence<css::uno::Any>& rArgs) override;
    virtual css::uno::Reference<css::media::XFrameGrabber> SAL_CALL createFrameGrabber() override;

    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    virtual void SAL_CALL
    addPlayerListener(const css::uno::Reference<css::media::XPlayerListener>& rListener) override;
    virtual void SAL_CALL removePlayerListener(
        const css::uno::Reference<css::media::XPlayerListener>& rListener) override;

    virtual void SAL_CALL disposing() final override;

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
