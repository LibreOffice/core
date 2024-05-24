/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <QtCore/QUrl>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaMetaData>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QtWidgets/QLayout>

#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>
#include <rtl/string.hxx>
#include <tools/link.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/timer.hxx>

#include <gstwindow.hxx>
#include "QtPlayer.hxx"

#include <QtPlayer.moc>

using namespace ::com::sun::star;

namespace
{
inline QString toQString(const OUString& rStr)
{
    return QString::fromUtf16(rStr.getStr(), rStr.getLength());
}
}

namespace avmedia::qt
{
QtPlayer::QtPlayer()
    : QtPlayer_BASE(m_aMutex)
    , m_lListener(m_aMutex)
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

    assert(m_xMediaPlayer);
    // don't use QMediaPlayer::stop because XPlayer::stop should leave the position unchanged
    m_xMediaPlayer->pause();
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

    QWidget* pParent = static_cast<QWidget*>(pParentEnvData->pWidget);
    QVideoWidget* pVideoWidget = new QVideoWidget(pParent);
    pVideoWidget->setAspectRatioMode(Qt::IgnoreAspectRatio);
    pVideoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    assert(!m_xMediaPlayer->videoOutput() && "Video widget already set.");
    m_xMediaPlayer->setVideoOutput(pVideoWidget);

    // retrieve the layout (which is set in the QtObjectWidget ctor)
    QLayout* pLayout = pParent->layout();
    assert(pLayout);
    pLayout->addWidget(pVideoWidget);

    uno::Reference<::media::XPlayerWindow> xRet = new ::avmedia::gstreamer::Window;
    return xRet;
}

uno::Reference<media::XFrameGrabber> SAL_CALL QtPlayer::createFrameGrabber() { return nullptr; }

void SAL_CALL
QtPlayer::addPlayerListener(const css::uno::Reference<css::media::XPlayerListener>& rListener)
{
    m_lListener.addInterface(cppu::UnoType<css::media::XPlayerListener>::get(), rListener);
    if (isReadyToPlay())
    {
        css::lang::EventObject aEvent;
        aEvent.Source = getXWeak();
        rListener->preferredPlayerWindowSizeAvailable(aEvent);
    }
    else
    {
        installNotify();
    }
}

void SAL_CALL
QtPlayer::removePlayerListener(const css::uno::Reference<css::media::XPlayerListener>& rListener)
{
    m_lListener.removeInterface(cppu::UnoType<css::media::XPlayerListener>::get(), rListener);
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
    // ensure output objects get deleted as QMediaPlayer doesn't take ownership of them
    std::unique_ptr<QObject> xVideoWidget(m_xMediaPlayer->videoOutput());
    std::unique_ptr<QAudioOutput> xAudioOutput(m_xMediaPlayer->audioOutput());
    m_xMediaPlayer.reset();
}

bool QtPlayer::isReadyToPlay()
{
    assert(m_xMediaPlayer);
    QMediaPlayer::MediaStatus eStatus = m_xMediaPlayer->mediaStatus();
    return eStatus == QMediaPlayer::BufferingMedia || eStatus == QMediaPlayer::BufferedMedia
           || eStatus == QMediaPlayer::LoadedMedia || eStatus == QMediaPlayer::EndOfMedia;
}

void QtPlayer::installNotify()
{
    connect(m_xMediaPlayer.get(), &QMediaPlayer::mediaStatusChanged, this,
            &QtPlayer::notifyIfReady);
}

void QtPlayer::uninstallNotify()
{
    disconnect(m_xMediaPlayer.get(), &QMediaPlayer::mediaStatusChanged, this,
               &QtPlayer::notifyIfReady);
}

void QtPlayer::notifyIfReady(QMediaPlayer::MediaStatus)
{
    if (isReadyToPlay())
    {
        rtl::Reference<QtPlayer> xThis(this);
        xThis->notifyListeners();
        xThis->uninstallNotify();
    }
}

void QtPlayer::notifyListeners()
{
    comphelper::OInterfaceContainerHelper2* pContainer
        = m_lListener.getContainer(cppu::UnoType<css::media::XPlayerListener>::get());
    if (!pContainer)
        return;

    css::lang::EventObject aEvent;
    aEvent.Source = getXWeak();

    comphelper::OInterfaceIteratorHelper2 pIterator(*pContainer);
    while (pIterator.hasMoreElements())
    {
        css::uno::Reference<css::media::XPlayerListener> xListener(
            static_cast<css::media::XPlayerListener*>(pIterator.next()));
        xListener->preferredPlayerWindowSizeAvailable(aEvent);
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
