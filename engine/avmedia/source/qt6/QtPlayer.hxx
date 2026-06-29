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

#include <QtMultimedia/QMediaPlayer>
#include <QtWidgets/QWidget>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/media/XPlayer.hpp>
#include <com/sun/star/media/XPlayerNotifier.hpp>
#include <comphelper/multicontainer2.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

namespace avmedia::qt
{
typedef cppu::WeakComponentImplHelper<css::media::XPlayer, css::lang::XServiceInfo> QtPlayer_BASE;

class QtPlayer final : public QObject, public cppu::BaseMutex, public QtPlayer_BASE
{
    Q_OBJECT

public:
    explicit QtPlayer();
    ~QtPlayer() override;

    bool create(const OUString& rURL);

    // XPlayer
    virtual void start() override;
    virtual void stop() override;
    virtual bool isPlaying() override;
    virtual double getDuration() override;
    virtual void setMediaTime(double fTime) override;
    virtual double getMediaTime() override;
    virtual void setPlaybackLoop(bool bSet) override;
    virtual bool isPlaybackLoop() override;
    virtual void setVolumeDB(sal_Int16 nVolumeDB) override;
    virtual sal_Int16 getVolumeDB() override;
    virtual void setMute(bool bSet) override;
    virtual bool isMute() override;
    virtual css::awt::Size getPreferredPlayerWindowSize() override;
    virtual css::uno::Reference<css::media::XPlayerWindow>
    createPlayerWindow(const css::uno::Sequence<cpo::uno::Any>& rArgs) override;
    virtual css::uno::Reference<css::media::XFrameGrabber> createFrameGrabber() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> getSupportedServiceNames() override;

    virtual void disposing() final override;

private:
    std::unique_ptr<QMediaPlayer> m_xMediaPlayer;

    // area to use for the player widget
    css::awt::Rectangle m_aPlayerWidgetRect;

    QWidget* m_pMediaWidgetParent;

    void createMediaPlayerWidget();
};

} // namespace avmedia::qt

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
