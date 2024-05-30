/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <mutex>

#include <QtCore/QObject>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QVideoFrame>
#include <QtMultimedia/QVideoSink>

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/media/XFrameGrabber.hpp>
#include <comphelper/compbase.hxx>

namespace avmedia::qt
{
class QtFrameGrabber : public QObject, public ::cppu::WeakImplHelper<css::media::XFrameGrabber>
{
    Q_OBJECT

private:
    std::unique_ptr<QVideoSink> m_xVideoSink;
    std::unique_ptr<QMediaPlayer> m_xMediaPlayer;

    std::recursive_mutex m_aMutex;
    bool m_bWaitingForFrame;
    css::uno::Reference<css::graphic::XGraphic> m_xGraphic;

public:
    QtFrameGrabber(const QUrl& rSourceUrl);

    virtual css::uno::Reference<css::graphic::XGraphic>
        SAL_CALL grabFrame(double fMediaTime) override;

private slots:
    void onErrorOccured(QMediaPlayer::Error eError, const QString& rErrorString);
    void onVideoFrameChanged(const QVideoFrame& rFrame);
};

} // namespace avmedia::qt

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
