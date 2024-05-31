/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtCore/QBuffer>
#include <QtCore/QByteArray>

#include <sal/log.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/graph.hxx>
#include <vcl/image.hxx>
#include <vcl/scheduler.hxx>

#include "QtFrameGrabber.hxx"
#include <QtFrameGrabber.moc>

using namespace ::com::sun::star;

namespace
{
inline OUString toOUString(const QString& s)
{
    return OUString(reinterpret_cast<const sal_Unicode*>(s.data()), s.length());
}

uno::Reference<css::graphic::XGraphic> toXGraphic(const QImage& rImage)
{
    QByteArray aData;
    QBuffer aBuffer(&aData);
    rImage.save(&aBuffer, "PNG");

    SvMemoryStream aStream(aData.data(), aData.size(), StreamMode::READ);
    vcl::PngImageReader aReader(aStream);
    Graphic aGraphic;
    aReader.read(aGraphic);

    return aGraphic.GetXGraphic();
}
}

namespace avmedia::qt
{
QtFrameGrabber::QtFrameGrabber(const QUrl& rSourceUrl)
    : m_bWaitingForFrame(false)
{
    m_xMediaPlayer = std::make_unique<QMediaPlayer>();
    m_xMediaPlayer->setSource(rSourceUrl);

    m_xVideoSink = std::make_unique<QVideoSink>();
    m_xMediaPlayer->setVideoSink(m_xVideoSink.get());

    connect(m_xMediaPlayer.get(), &QMediaPlayer::errorOccurred, this,
            &QtFrameGrabber::onErrorOccured, Qt::BlockingQueuedConnection);
}

void QtFrameGrabber::onErrorOccured(QMediaPlayer::Error eError, const QString& rErrorString)
{
    std::lock_guard aLock(m_aMutex);

    SAL_WARN("avmedia", "Media playback error occurred when trying to grab frame: "
                            << toOUString(rErrorString) << ", code: " << eError);

    m_bWaitingForFrame = false;
}

void QtFrameGrabber::onVideoFrameChanged(const QVideoFrame& rFrame)
{
    std::lock_guard aLock(m_aMutex);

    disconnect(m_xVideoSink.get(), &QVideoSink::videoFrameChanged, this,
               &QtFrameGrabber::onVideoFrameChanged);

    const QImage aImage = rFrame.toImage();
    m_xGraphic = toXGraphic(aImage);
    m_bWaitingForFrame = false;
}

css::uno::Reference<css::graphic::XGraphic> SAL_CALL QtFrameGrabber::grabFrame(double fMediaTime)
{
    std::lock_guard aLock(m_aMutex);

    m_xMediaPlayer->setPosition(fMediaTime * 1000);

    // in order to get a video frame, connect to videoFrameChanged signal and start playing
    // until the first frame has been received
    m_bWaitingForFrame = true;
    connect(m_xVideoSink.get(), &QVideoSink::videoFrameChanged, this,
            &QtFrameGrabber::onVideoFrameChanged, Qt::BlockingQueuedConnection);
    m_xMediaPlayer->play();
    while (m_bWaitingForFrame)
        Scheduler::ProcessEventsToIdle();
    m_xMediaPlayer->stop();

    uno::Reference<css::graphic::XGraphic> xGraphic = m_xGraphic;
    m_xGraphic.clear();
    return xGraphic;
}

}; // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
