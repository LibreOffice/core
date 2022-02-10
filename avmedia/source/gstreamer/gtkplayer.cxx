/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <mutex>

#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>
#include <rtl/string.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#include "gstplayer.hxx"
#include "gstframegrabber.hxx"
#include "gstwindow.hxx"

#include <gtk/gtk.h>

constexpr OUStringLiteral AVMEDIA_GTK_PLAYER_IMPLEMENTATIONNAME
    = u"com.sun.star.comp.avmedia.Player_GtkVideo";
constexpr OUStringLiteral AVMEDIA_GTK_PLAYER_SERVICENAME = u"com.sun.star.media.Player_GtkVideo";

using namespace ::com::sun::star;

namespace avmedia::gstreamer
{
GtkPlayer::GtkPlayer()
    : GstPlayer_BASE(m_aMutex)
    , m_pStream(nullptr)
{
}

GtkPlayer::~GtkPlayer() { disposing(); }

void SAL_CALL GtkPlayer::disposing()
{
    osl::MutexGuard aGuard(m_aMutex);

    stop();

    if (m_pStream)
    {
        g_object_unref(m_pStream);
        m_pStream = nullptr;
    }
}

bool GtkPlayer::create(const OUString& rURL)
{
    bool bRet = false;

    if (m_pStream)
    {
        g_object_unref(m_pStream);
        m_pStream = nullptr;
    }

    if (!rURL.isEmpty())
    {
        GFile* pFile = g_file_new_for_uri(OUStringToOString(rURL, RTL_TEXTENCODING_UTF8).getStr());
        m_pStream = gtk_media_file_new_for_file(pFile);
        g_object_unref(pFile);

        gtk_media_stream_pause(m_pStream);

        bRet = gtk_media_stream_get_error(m_pStream) == nullptr;
    }

    if (bRet)
        m_aURL = rURL;
    else
        m_aURL.clear();

    return bRet;
}

void SAL_CALL GtkPlayer::start()
{
    osl::MutexGuard aGuard(m_aMutex);

    if (m_pStream)
        gtk_media_stream_play(m_pStream);
}

void SAL_CALL GtkPlayer::stop()
{
    osl::MutexGuard aGuard(m_aMutex);

    if (m_pStream)
        gtk_media_stream_pause(m_pStream);
}

sal_Bool SAL_CALL GtkPlayer::isPlaying()
{
    osl::MutexGuard aGuard(m_aMutex);

    bool bRet = false;

    if (m_pStream)
        bRet = gtk_media_stream_get_playing(m_pStream);

    return bRet;
}

double SAL_CALL GtkPlayer::getDuration()
{
    osl::MutexGuard aGuard(m_aMutex);

    double duration = 0.0;

    if (m_pStream)
        duration = gtk_media_stream_get_duration(m_pStream);

    return duration;
}

void SAL_CALL GtkPlayer::setMediaTime(double fTime)
{
    osl::MutexGuard aGuard(m_aMutex);

    if (!m_pStream)
        return;

    gint64 gst_position = llround(fTime * GST_SECOND);

    gtk_media_stream_seek(m_pStream, gst_position);
}

double SAL_CALL GtkPlayer::getMediaTime()
{
    osl::MutexGuard aGuard(m_aMutex);

    double position = 0.0;

    if (m_pStream)
    {
        // get current position in the stream
        gint64 gst_position = gtk_media_stream_get_timestamp(m_pStream);
        position = gst_position / GST_SECOND;
    }

    return position;
}

void SAL_CALL GtkPlayer::setPlaybackLoop(sal_Bool bSet)
{
    osl::MutexGuard aGuard(m_aMutex);
    gtk_media_stream_set_loop(m_pStream, bSet);
}

sal_Bool SAL_CALL GtkPlayer::isPlaybackLoop()
{
    osl::MutexGuard aGuard(m_aMutex);
    return gtk_media_stream_get_loop(m_pStream);
}

void SAL_CALL GtkPlayer::setMute(sal_Bool bSet)
{
    osl::MutexGuard aGuard(m_aMutex);
    gtk_media_stream_set_muted(m_pStream, bSet);
}

sal_Bool SAL_CALL GtkPlayer::isMute()
{
    osl::MutexGuard aGuard(m_aMutex);
    return gtk_media_stream_get_muted(m_pStream);
}

void SAL_CALL GtkPlayer::setVolumeDB(sal_Int16 nVolumeDB)
{
    osl::MutexGuard aGuard(m_aMutex);

    double nVolume = pow(10.0, nVolumeDB / 20.0);
    gtk_media_stream_set_volume(m_pStream, nVolume);
}

sal_Int16 SAL_CALL GtkPlayer::getVolumeDB()
{
    osl::MutexGuard aGuard(m_aMutex);

    double nGstVolume = gtk_media_stream_get_volume(m_pStream);
    return static_cast<sal_Int16>(20.0 * log10(nGstVolume));
}

awt::Size SAL_CALL GtkPlayer::getPreferredPlayerWindowSize()
{
    osl::MutexGuard aGuard(m_aMutex);

    awt::Size aSize(0, 0);

    SAL_WARN("avmedia.gstreamer", "TODO: getPreferredPlayerWindowSize");

    return aSize;
}

uno::Reference<::media::XPlayerWindow>
    SAL_CALL GtkPlayer::createPlayerWindow(const uno::Sequence<uno::Any>& rArguments)
{
    osl::MutexGuard aGuard(m_aMutex);

    uno::Reference<::media::XPlayerWindow> xRet;

    if (rArguments.getLength() > 1)
        rArguments[1] >>= m_aArea;

    if (rArguments.getLength() <= 2)
    {
        xRet = new ::avmedia::gstreamer::Window;
        return xRet;
    }

    sal_IntPtr pIntPtr = 0;
    rArguments[2] >>= pIntPtr;
    SystemChildWindow* pParentWindow = reinterpret_cast<SystemChildWindow*>(pIntPtr);
    if (!pParentWindow)
        return nullptr;

    const SystemEnvData* pEnvData = pParentWindow->GetSystemData();
    if (!pEnvData)
        return nullptr;

    GtkWidget* pVideo = gtk_video_new_for_media_stream(m_pStream);
    gtk_widget_set_vexpand(pVideo, true);
    gtk_widget_set_hexpand(pVideo, true);

    GtkWidget* pParent = static_cast<GtkWidget*>(pEnvData->pWidget);
    gtk_grid_attach(GTK_GRID(pParent), pVideo, 0, 0, 1, 1);
    gtk_widget_show(pVideo);
    gtk_widget_show(pParent);

    xRet = new ::avmedia::gstreamer::Window;

    gtk_media_stream_pause(m_pStream);

    return xRet;
}

uno::Reference<media::XFrameGrabber> SAL_CALL GtkPlayer::createFrameGrabber()
{
    osl::MutexGuard aGuard(m_aMutex);
    rtl::Reference<FrameGrabber> xFrameGrabber;
    SAL_WARN("avmedia.gstreamer", "TODO: createFrameGrabber");
    return xFrameGrabber;
}

OUString SAL_CALL GtkPlayer::getImplementationName()
{
    return AVMEDIA_GTK_PLAYER_IMPLEMENTATIONNAME;
}

sal_Bool SAL_CALL GtkPlayer::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence<OUString> SAL_CALL GtkPlayer::getSupportedServiceNames()
{
    return { AVMEDIA_GTK_PLAYER_SERVICENAME };
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
