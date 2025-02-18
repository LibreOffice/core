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
#include <tools/link.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/timer.hxx>

#include <gstwindow.hxx>
#include "gtkplayer.hxx"

#include <gtk/gtk.h>

constexpr OUStringLiteral AVMEDIA_GTK_PLAYER_IMPLEMENTATIONNAME
    = u"com.sun.star.comp.avmedia.Player_Gtk";
constexpr OUString AVMEDIA_GTK_PLAYER_SERVICENAME = u"com.sun.star.media.Player_Gtk"_ustr;

using namespace ::com::sun::star;

namespace avmedia::gtk
{
GtkPlayer::GtkPlayer()
    : GtkPlayer_BASE(m_aMutex)
    , m_lListener(m_aMutex)
    , m_pStream(nullptr)
    , m_pVideo(nullptr)
    , m_nNotifySignalId(0)
    , m_nInvalidateSizeSignalId(0)
    , m_nTimeoutId(0)
    , m_nUnmutedVolume(0)
{
}

GtkPlayer::~GtkPlayer() { disposing(); }

static gboolean gtk_media_stream_unref(gpointer user_data)
{
    g_object_unref(user_data);
    return FALSE;
}

void GtkPlayer::cleanup()
{
    if (m_pVideo)
    {
        gtk_widget_unparent(m_pVideo);
        m_pVideo = nullptr;
    }

    if (m_pStream)
    {
        uninstallNotify();

        // shouldn't have to attempt this unref on idle, but with gtk4-4.4.1 I get
        // intermittent "instance of invalid non-instantiable type '(null)'"
        // on some mysterious gst dbus callback
        if (g_main_context_default())
            g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, gtk_media_stream_unref, m_pStream, nullptr);
        else
            g_object_unref(m_pStream);
        m_pStream = nullptr;
    }
}

void SAL_CALL GtkPlayer::disposing()
{
    osl::MutexGuard aGuard(m_aMutex);

    stop();

    cleanup();
}

static void do_notify(GtkPlayer* pThis)
{
    rtl::Reference<GtkPlayer> xThis(pThis);
    xThis->notifyListeners();
    xThis->uninstallNotify();
}

static void invalidate_size_cb(GdkPaintable* /*pPaintable*/, GtkPlayer* pThis) { do_notify(pThis); }

static void notify_cb(GtkMediaStream* /*pStream*/, GParamSpec* pspec, GtkPlayer* pThis)
{
    if (g_str_equal(pspec->name, "prepared") || g_str_equal(pspec->name, "error"))
        do_notify(pThis);
}

static bool timeout_cb(GtkPlayer* pThis)
{
    do_notify(pThis);
    return false;
}

void GtkPlayer::installNotify()
{
    if (m_nNotifySignalId)
        return;
    m_nNotifySignalId = g_signal_connect(m_pStream, "notify", G_CALLBACK(notify_cb), this);
    // notify should be enough, but there is an upstream bug so also try "invalidate-size" and add a timeout for
    // audio-only case where that won't happen, see: https://gitlab.gnome.org/GNOME/gtk/-/merge_requests/4513
    m_nInvalidateSizeSignalId
        = g_signal_connect(m_pStream, "invalidate-size", G_CALLBACK(invalidate_size_cb), this);
    m_nTimeoutId = g_timeout_add_seconds(10, G_SOURCE_FUNC(timeout_cb), this);
}

void GtkPlayer::uninstallNotify()
{
    if (!m_nNotifySignalId)
        return;
    g_signal_handler_disconnect(m_pStream, m_nNotifySignalId);
    m_nNotifySignalId = 0;
    g_signal_handler_disconnect(m_pStream, m_nInvalidateSizeSignalId);
    m_nInvalidateSizeSignalId = 0;
    g_source_remove(m_nTimeoutId);
    m_nTimeoutId = 0;
}

bool GtkPlayer::create(const OUString& rURL)
{
    bool bRet = false;

    cleanup();

    if (!rURL.isEmpty())
    {
        GFile* pFile = g_file_new_for_uri(OUStringToOString(rURL, RTL_TEXTENCODING_UTF8).getStr());
        m_pStream = gtk_media_file_new_for_file(pFile);
        g_object_unref(pFile);

        bRet = gtk_media_stream_get_error(m_pStream) == nullptr;
    }

    if (bRet)
        m_aURL = rURL;
    else
        m_aURL.clear();

    return bRet;
}

void GtkPlayer::notifyListeners()
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
        duration = gtk_media_stream_get_duration(m_pStream) / 1000000.0;

    return duration;
}

void SAL_CALL GtkPlayer::setMediaTime(double fTime)
{
    osl::MutexGuard aGuard(m_aMutex);

    if (!m_pStream)
        return;

    gint64 gst_position = llround(fTime * 1000000);

    gtk_media_stream_seek(m_pStream, gst_position);

    // on resetting back to zero the reported timestamp doesn't seem to get
    // updated in a reasonable time, so on zero just force an update of
    // timestamp to 0
    if (gst_position == 0 && gtk_media_stream_is_prepared(m_pStream))
        gtk_media_stream_update(m_pStream, gst_position);
}

double SAL_CALL GtkPlayer::getMediaTime()
{
    osl::MutexGuard aGuard(m_aMutex);

    double position = 0.0;

    if (m_pStream)
        position = gtk_media_stream_get_timestamp(m_pStream) / 1000000.0;

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

// gtk4-4.4.1 docs state: "Muting a stream will cause no audio to be played, but
// it does not modify the volume. This means that muting and then unmuting the
// stream will restore the volume settings." but that doesn't seem to be my
// experience at all
void SAL_CALL GtkPlayer::setMute(sal_Bool bSet)
{
    osl::MutexGuard aGuard(m_aMutex);
    bool bMuted = gtk_media_stream_get_muted(m_pStream);
    if (bMuted == static_cast<bool>(bSet))
        return;
    gtk_media_stream_set_muted(m_pStream, bSet);
    if (!bSet)
        setVolumeDB(m_nUnmutedVolume);
}

sal_Bool SAL_CALL GtkPlayer::isMute()
{
    osl::MutexGuard aGuard(m_aMutex);
    return gtk_media_stream_get_muted(m_pStream);
}

void SAL_CALL GtkPlayer::setVolumeDB(sal_Int16 nVolumeDB)
{
    osl::MutexGuard aGuard(m_aMutex);

    // range is -40 for silence to 0 for full volume
    m_nUnmutedVolume = std::clamp<sal_Int16>(nVolumeDB, -40, 0);
    double fValue = (m_nUnmutedVolume + 40) / 40.0;
    gtk_media_stream_set_volume(m_pStream, fValue);
}

sal_Int16 SAL_CALL GtkPlayer::getVolumeDB()
{
    osl::MutexGuard aGuard(m_aMutex);

    if (gtk_media_stream_get_muted(m_pStream))
        return m_nUnmutedVolume;

    double fVolume = gtk_media_stream_get_volume(m_pStream);

    m_nUnmutedVolume = (fVolume * 40) - 40;

    return m_nUnmutedVolume;
}

awt::Size SAL_CALL GtkPlayer::getPreferredPlayerWindowSize()
{
    osl::MutexGuard aGuard(m_aMutex);

    awt::Size aSize(0, 0);

    if (m_pStream)
    {
        aSize.Width = gdk_paintable_get_intrinsic_width(GDK_PAINTABLE(m_pStream));
        aSize.Height = gdk_paintable_get_intrinsic_height(GDK_PAINTABLE(m_pStream));
    }

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

    m_pVideo = gtk_picture_new_for_paintable(GDK_PAINTABLE(m_pStream));
    gtk_picture_set_content_fit(GTK_PICTURE(m_pVideo), GTK_CONTENT_FIT_FILL);
    gtk_widget_set_can_target(m_pVideo, false);
    gtk_widget_set_vexpand(m_pVideo, true);
    gtk_widget_set_hexpand(m_pVideo, true);

    GtkWidget* pParent = static_cast<GtkWidget*>(pEnvData->pWidget);
    gtk_widget_set_can_target(pParent, false);
    gtk_grid_attach(GTK_GRID(pParent), m_pVideo, 0, 0, 1, 1);
    gtk_widget_set_visible(m_pVideo, true);
    gtk_widget_set_visible(pParent, true);

    xRet = new ::avmedia::gstreamer::Window;

    return xRet;
}

void SAL_CALL
GtkPlayer::addPlayerListener(const css::uno::Reference<css::media::XPlayerListener>& rListener)
{
    m_lListener.addInterface(cppu::UnoType<css::media::XPlayerListener>::get(), rListener);
    if (gtk_media_stream_is_prepared(m_pStream))
    {
        css::lang::EventObject aEvent;
        aEvent.Source = getXWeak();
        rListener->preferredPlayerWindowSizeAvailable(aEvent);
    }
    else
        installNotify();
}

void SAL_CALL
GtkPlayer::removePlayerListener(const css::uno::Reference<css::media::XPlayerListener>& rListener)
{
    m_lListener.removeInterface(cppu::UnoType<css::media::XPlayerListener>::get(), rListener);
}

namespace
{
class GtkFrameGrabber : public ::cppu::WeakImplHelper<css::media::XFrameGrabber>
{
private:
    awt::Size m_aSize;
    GtkMediaStream* m_pStream;

public:
    GtkFrameGrabber(GtkMediaStream* pStream, const awt::Size& rSize)
        : m_aSize(rSize)
        , m_pStream(pStream)
    {
        g_object_ref(m_pStream);
    }

    virtual ~GtkFrameGrabber() override { g_object_unref(m_pStream); }

    // XFrameGrabber
    virtual css::uno::Reference<css::graphic::XGraphic>
        SAL_CALL grabFrame(double fMediaTime) override
    {
        gint64 gst_position = llround(fMediaTime * 1000000);
        gtk_media_stream_seek(m_pStream, gst_position);

        cairo_surface_t* surface
            = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, m_aSize.Width, m_aSize.Height);

        GtkSnapshot* snapshot = gtk_snapshot_new();
        gdk_paintable_snapshot(GDK_PAINTABLE(m_pStream), snapshot, m_aSize.Width, m_aSize.Height);
        GskRenderNode* node = gtk_snapshot_free_to_node(snapshot);

        cairo_t* cr = cairo_create(surface);
        gsk_render_node_draw(node, cr);
        cairo_destroy(cr);

        gsk_render_node_unref(node);

        std::unique_ptr<BitmapEx> xBitmap(
            vcl::bitmap::CreateFromCairoSurface(Size(m_aSize.Width, m_aSize.Height), surface));

        cairo_surface_destroy(surface);

        return Graphic(*xBitmap).GetXGraphic();
    }
};
}

uno::Reference<media::XFrameGrabber> SAL_CALL GtkPlayer::createFrameGrabber()
{
    osl::MutexGuard aGuard(m_aMutex);

    rtl::Reference<GtkFrameGrabber> xFrameGrabber;

    const awt::Size aPrefSize(getPreferredPlayerWindowSize());

    if (aPrefSize.Width > 0 && aPrefSize.Height > 0)
        xFrameGrabber.set(new GtkFrameGrabber(m_pStream, aPrefSize));

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
