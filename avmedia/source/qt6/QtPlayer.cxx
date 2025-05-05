/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QUrl>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaMetaData>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>

#include <com/sun/star/media/XManager.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>
#include <rtl/string.hxx>
#include <tools/link.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/graph.hxx>
#include <vcl/qt/QtUtils.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/timer.hxx>

#include <gstwindow.hxx>
#include <mediamisc.hxx>
#include "QtFrameGrabber.hxx"
#include "QtPlayer.hxx"

#include <QtPlayer.moc>

using namespace ::com::sun::star;

namespace avmedia::qt
{
QtPlayer::QtPlayer()
    : QtPlayer_BASE(m_aMutex)
    , m_pMediaWidgetParent(nullptr)
{
}

bool QtPlayer::create(const OUString& rURL)
{
    const QUrl aQUrl(toQString(rURL));
    if (!aQUrl.isValid() || !aQUrl.isLocalFile())
        return false;

    m_xMediaPlayer = std::make_unique<QMediaPlayer>();
    m_xMediaPlayer->setSource(aQUrl);
    QAudioOutput* pAudioOutput = new QAudioOutput;
    pAudioOutput->setVolume(50);
    m_xMediaPlayer->setAudioOutput(pAudioOutput);

    return true;
}

void SAL_CALL QtPlayer::start()
{
    osl::MutexGuard aGuard(m_aMutex);

    assert(m_xMediaPlayer);
    m_xMediaPlayer->play();
}

void SAL_CALL QtPlayer::stop()
{
    osl::MutexGuard aGuard(m_aMutex);

    if (m_xMediaPlayer)
    {
        // don't use QMediaPlayer::stop because XPlayer::stop should leave the position unchanged
        m_xMediaPlayer->pause();
    }
}

sal_Bool SAL_CALL QtPlayer::isPlaying()
{
    osl::MutexGuard aGuard(m_aMutex);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    assert(m_xMediaPlayer);
    return m_xMediaPlayer->isPlaying();
#else
    return false;
#endif
}

double SAL_CALL QtPlayer::getDuration()
{
    osl::MutexGuard aGuard(m_aMutex);

    assert(m_xMediaPlayer);
    return m_xMediaPlayer->duration() / 1000.0;
}

void SAL_CALL QtPlayer::setMediaTime(double fTime)
{
    osl::MutexGuard aGuard(m_aMutex);

    assert(m_xMediaPlayer);
    m_xMediaPlayer->setPosition(fTime * 1000);
}

double SAL_CALL QtPlayer::getMediaTime()
{
    osl::MutexGuard aGuard(m_aMutex);

    assert(m_xMediaPlayer);
    return m_xMediaPlayer->position() / 1000.0;
}

void SAL_CALL QtPlayer::setPlaybackLoop(sal_Bool bSet)
{
    assert(m_xMediaPlayer);
    const int nLoops = bSet ? QMediaPlayer::Infinite : QMediaPlayer::Once;
    m_xMediaPlayer->setLoops(nLoops);
}

sal_Bool SAL_CALL QtPlayer::isPlaybackLoop()
{
    assert(m_xMediaPlayer);
    return m_xMediaPlayer->loops() == QMediaPlayer::Infinite;
}

void SAL_CALL QtPlayer::setVolumeDB(sal_Int16 nVolumeDB)
{
    osl::MutexGuard aGuard(m_aMutex);

    // range is -40 for silence to 0 for full volume
    const sal_Int16 nVolume = std::clamp<sal_Int16>(nVolumeDB, -40, 0);
    double fValue = (nVolume + 40) / 40.0;
    assert(m_xMediaPlayer);
    QAudioOutput* pAudioOutput = m_xMediaPlayer->audioOutput();
    assert(pAudioOutput);
    pAudioOutput->setVolume(fValue);
}

sal_Int16 SAL_CALL QtPlayer::getVolumeDB()
{
    osl::MutexGuard aGuard(m_aMutex);

    assert(m_xMediaPlayer);
    QAudioOutput* pAudioOutput = m_xMediaPlayer->audioOutput();
    assert(pAudioOutput);

    double fVolume = pAudioOutput->volume();
    return (fVolume * 40) - 40;
}

void SAL_CALL QtPlayer::setMute(sal_Bool bSet)
{
    osl::MutexGuard aGuard(m_aMutex);

    assert(m_xMediaPlayer);
    QAudioOutput* pAudioOutput = m_xMediaPlayer->audioOutput();
    assert(pAudioOutput);
    pAudioOutput->setMuted(bSet);
}

sal_Bool SAL_CALL QtPlayer::isMute()
{
    osl::MutexGuard aGuard(m_aMutex);

    assert(m_xMediaPlayer);
    QAudioOutput* pAudioOutput = m_xMediaPlayer->audioOutput();
    assert(pAudioOutput);
    return pAudioOutput->isMuted();
}

awt::Size SAL_CALL QtPlayer::getPreferredPlayerWindowSize()
{
    osl::MutexGuard aGuard(m_aMutex);

    assert(m_xMediaPlayer);

    // if media hasn't been loaded yet, ensure this happens, since
    // retrieving resolution doesn't work reliably otherwise
    if (m_xMediaPlayer->mediaStatus() == QMediaPlayer::LoadingMedia)
    {
        m_xMediaPlayer->play();

        while (m_xMediaPlayer->mediaStatus() == QMediaPlayer::LoadingMedia)
            QCoreApplication::processEvents();

        m_xMediaPlayer->stop();
    }

    const QMediaMetaData aMetaData = m_xMediaPlayer->metaData();
    const QVariant aResolutionVariant = aMetaData.value(QMediaMetaData::Resolution);
    if (aResolutionVariant.canConvert<QSize>())
    {
        const QSize aResolution = aResolutionVariant.value<QSize>();
        return awt::Size(aResolution.width(), aResolution.height());
    }

    return awt::Size(0, 0);
}

uno::Reference<::media::XPlayerWindow>
    SAL_CALL QtPlayer::createPlayerWindow(const uno::Sequence<uno::Any>& rArguments)
{
    osl::MutexGuard aGuard(m_aMutex);

    if (rArguments.getLength() > 1)
        rArguments[1] >>= m_aPlayerWidgetRect;

    if (rArguments.getLength() <= 2)
    {
        uno::Reference<::media::XPlayerWindow> xRet = new ::avmedia::gstreamer::Window;
        return xRet;
    }

    sal_IntPtr pIntPtr = 0;
    rArguments[2] >>= pIntPtr;
    SystemChildWindow* pParentWindow = reinterpret_cast<SystemChildWindow*>(pIntPtr);
    if (!pParentWindow)
        return nullptr;

    const SystemEnvData* pParentEnvData = pParentWindow->GetSystemData();
    if (!pParentEnvData)
        return nullptr;

    m_pMediaWidgetParent = static_cast<QWidget*>(pParentEnvData->pWidget);
    assert(m_pMediaWidgetParent);

    // while media is loading, QMediaPlayer::hasVideo doesn't yet return
    // whether media actually has video; defer creating audio/video widget
    if (m_xMediaPlayer->mediaStatus() == QMediaPlayer::LoadingMedia)
    {
        connect(m_xMediaPlayer.get(), &QMediaPlayer::mediaStatusChanged, this,
                &QtPlayer::createMediaPlayerWidget, Qt::SingleShotConnection);
    }
    else
    {
        createMediaPlayerWidget();
    }

    uno::Reference<::media::XPlayerWindow> xRet = new ::avmedia::gstreamer::Window;
    return xRet;
}

namespace
{
// see also MediaWindowImpl::createPlayer
uno::Reference<media::XFrameGrabber> createPlatformFrameGrabber(const OUString& rUrl)
{
    const uno::Reference<uno::XComponentContext>& xContext
        = comphelper::getProcessComponentContext();
    try
    {
        uno::Reference<css::media::XManager> xManager(
            xContext->getServiceManager()->createInstanceWithContext(AVMEDIA_MANAGER_SERVICE_NAME,
                                                                     xContext),
            uno::UNO_QUERY);
        if (!xManager.is())
            return nullptr;

        uno::Reference<media::XPlayer> xPlayer = xManager->createPlayer(rUrl);
        if (!xPlayer.is())
            return nullptr;

        return xPlayer->createFrameGrabber();
    }
    catch (const uno::Exception&)
    {
        SAL_WARN("avmedia", "Exception in createPlatformFrameGrabber");
    }

    return nullptr;
}
}

uno::Reference<media::XFrameGrabber> SAL_CALL QtPlayer::createFrameGrabber()
{
    osl::MutexGuard aGuard(m_aMutex);

    // use the default platform frame grabber (GStreamer on Linux) by default
    // instead of using QtFrameGrabber for now unless overriden by env variable,
    // as QtFrameGrabber has issues (see e.g. tdf#166055)
    static const bool bPreferQtFrameGrabber
        = (getenv("SAL_VCL_QT_USE_QT_FRAME_GRABBER") != nullptr);
    if (!bPreferQtFrameGrabber)
    {
        uno::Reference<media::XFrameGrabber> xFrameGrabber
            = createPlatformFrameGrabber(toOUString(m_xMediaPlayer->source().url()));
        if (xFrameGrabber.is())
            return xFrameGrabber;
    }

    return new QtFrameGrabber(m_xMediaPlayer->source());
}

OUString SAL_CALL QtPlayer::getImplementationName()
{
    return u"com.sun.star.comp.avmedia.Player_Qt"_ustr;
}

sal_Bool SAL_CALL QtPlayer::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL QtPlayer::getSupportedServiceNames()
{
    return { u"com.sun.star.media.Player_Qt"_ustr };
}

void SAL_CALL QtPlayer::disposing()
{
    osl::MutexGuard aGuard(m_aMutex);
    stop();
    QtPlayer_BASE::disposing();
}

QtPlayer::~QtPlayer()
{
    if (m_xMediaPlayer)
    {
        // ensure output objects get deleted as QMediaPlayer doesn't take ownership of them
        std::unique_ptr<QObject> xVideoWidget(m_xMediaPlayer->videoOutput());
        std::unique_ptr<QAudioOutput> xAudioOutput(m_xMediaPlayer->audioOutput());
    }

    m_xMediaPlayer.reset();
}

void QtPlayer::createMediaPlayerWidget()
{
    assert(m_xMediaPlayer);
    assert(m_xMediaPlayer->mediaStatus() != QMediaPlayer::LoadingMedia
           && "Media is still loading, detecting video availability not possible.");

    assert(m_pMediaWidgetParent && "Parent for media widget not set");

    // if media contains video, show the video output,
    // otherwise show an audio icon as a placeholder
    QWidget* pWidget;
    if (m_xMediaPlayer->hasVideo())
    {
        QVideoWidget* pVideoWidget = new QVideoWidget(m_pMediaWidgetParent);
        pVideoWidget->setAspectRatioMode(Qt::IgnoreAspectRatio);

        assert(!m_xMediaPlayer->videoOutput() && "Video output already set.");
        m_xMediaPlayer->setVideoOutput(pVideoWidget);

        pWidget = pVideoWidget;
    }
    else
    {
        QPixmap aAudioPixmap = loadQPixmapIcon(u"avmedia/res/avaudiologo.png"_ustr);
        aAudioPixmap
            = aAudioPixmap.scaled(QSize(m_aPlayerWidgetRect.Width, m_aPlayerWidgetRect.Height));

        QLabel* pLabel = new QLabel;
        pLabel->setPixmap(aAudioPixmap);
        pWidget = pLabel;
    }

    assert(pWidget);
    pWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // retrieve the layout (which is set in the QtObjectWidget ctor)
    QLayout* pLayout = m_pMediaWidgetParent->layout();
    assert(pLayout);
    assert(pLayout->count() == 0 && "Layout already has a widget set");
    pLayout->addWidget(pWidget);
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
