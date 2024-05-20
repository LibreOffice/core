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

#include <QtMultimedia/QMediaPlayer>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/media/XPlayer.hpp>
#include <com/sun/star/media/XPlayerNotifier.hpp>
#include <comphelper/multicontainer2.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

namespace avmedia::qt
{
typedef cppu::WeakComponentImplHelper<css::media::XPlayer, css::media::XPlayerNotifier,
                                      css::lang::XServiceInfo>
    QtPlayer_BASE;

class QtPlayer final : public QObject, public cppu::BaseMutex, public QtPlayer_BASE
{
    Q_OBJECT

public:
    explicit QtPlayer();
    ~QtPlayer() override;

    bool create(const OUString& rURL);

    // XPlayer
    virtual void SAL_CALL start() override;
    virtual void SAL_CALL stop() override;
    virtual sal_Bool SAL_CALL isPlaying() override;
    virtual double SAL_CALL getDuration() override;
    virtual void SAL_CALL setMediaTime(double fTime) override;
    virtual double SAL_CALL getMediaTime() override;
    virtual void SAL_CALL setPlaybackLoop(sal_Bool bSet) override;
    virtual sal_Bool SAL_CALL isPlaybackLoop() override;
    virtual void SAL_CALL setVolumeDB(sal_Int16 nVolumeDB) override;
    virtual sal_Int16 SAL_CALL getVolumeDB() override;
    virtual void SAL_CALL setMute(sal_Bool bSet) override;
    virtual sal_Bool SAL_CALL isMute() override;
    virtual css::awt::Size SAL_CALL getPreferredPlayerWindowSize() override;
    virtual css::uno::Reference<css::media::XPlayerWindow>
        SAL_CALL createPlayerWindow(const css::uno::Sequence<css::uno::Any>& rArgs) override;
    virtual css::uno::Reference<css::media::XFrameGrabber> SAL_CALL createFrameGrabber() override;

    // XPlayerNotifier
    virtual void SAL_CALL
    addPlayerListener(const css::uno::Reference<css::media::XPlayerListener>& rListener) override;
    virtual void SAL_CALL removePlayerListener(
        const css::uno::Reference<css::media::XPlayerListener>& rListener) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    virtual void SAL_CALL disposing() final override;

private:
    std::unique_ptr<QMediaPlayer> m_xMediaPlayer;
    comphelper::OMultiTypeInterfaceContainerHelper2 m_lListener;

    bool isReadyToPlay();

    void installNotify();
    void uninstallNotify();
    void notifyListeners();
    void notifyIfReady(QMediaPlayer::MediaStatus eStatus);
};

} // namespace avmedia::qt

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
