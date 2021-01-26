/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <map>
#include <set>
#include <vector>
#include <math.h>

#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>
#include <rtl/string.hxx>
#include <salhelper/thread.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#include "gstplayer.hxx"
#include "gstframegrabber.hxx"
#include "gstwindow.hxx"

#include <gst/video/videooverlay.h>
#define AVMEDIA_GST_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_GStreamer"
#define AVMEDIA_GST_PLAYER_SERVICENAME        "com.sun.star.media.Player_GStreamer"

#include <gst/pbutils/missing-plugins.h>
#include <gst/pbutils/pbutils.h>

#define AVVERSION "gst 1.0: "

using namespace ::com::sun::star;

namespace avmedia::gstreamer {

namespace {

class FlagGuard {
public:
    explicit FlagGuard(bool & flag): flag_(flag) { flag_ = true; }

    ~FlagGuard() { flag_ = false; }

private:
    bool & flag_;
};

class MissingPluginInstallerThread: public salhelper::Thread {
public:
    MissingPluginInstallerThread(): Thread("MissingPluginInstaller") {}

private:
    void execute() override;
};


class MissingPluginInstaller {
    friend class MissingPluginInstallerThread;

public:
    MissingPluginInstaller(): launchNewThread_(true), inCleanUp_(false) {}

    ~MissingPluginInstaller();

    void report(rtl::Reference<Player> const & source, GstMessage * message);

    // Player::~Player calls Player::disposing calls
    // MissingPluginInstaller::detach, so do not take Player by rtl::Reference
    // here (which would bump its refcount back from 0 to 1):
    void detach(Player const * source);

private:
    void processQueue();

    DECL_STATIC_LINK(MissingPluginInstaller, launchUi, void*, void);

    osl::Mutex mutex_;
    std::set<OString> reported_;
    std::map<OString, std::set<rtl::Reference<Player>>> queued_;
    rtl::Reference<MissingPluginInstallerThread> currentThread_;
    std::vector<OString> currentDetails_;
    std::set<rtl::Reference<Player>> currentSources_;
    bool launchNewThread_;
    bool inCleanUp_;
};


MissingPluginInstaller::~MissingPluginInstaller() {
    osl::MutexGuard g(mutex_);
    SAL_WARN_IF(currentThread_.is(), "avmedia.gstreamer", "unjoined thread");
    inCleanUp_ = true;
}


void MissingPluginInstaller::report(
    rtl::Reference<Player> const & source, GstMessage * message)
{
    // assert(gst_is_missing_plugin_message(message));
    gchar * det = gst_missing_plugin_message_get_installer_detail(message);
    if (det == nullptr) {
        SAL_WARN(
            "avmedia.gstreamer",
            "gst_missing_plugin_message_get_installer_detail failed");
        return;
    }
    std::size_t len = std::strlen(det);
    if (len > SAL_MAX_INT32) {
        SAL_WARN("avmedia.gstreamer", "detail string too long");
        g_free(det);
        return;
    }
    OString detStr(det, len);
    g_free(det);
    rtl::Reference<MissingPluginInstallerThread> join;
    rtl::Reference<MissingPluginInstallerThread> launch;
    {
        osl::MutexGuard g(mutex_);
        if (reported_.find(detStr) != reported_.end()) {
            return;
        }
        auto & i = queued_[detStr];
        bool fresh = i.empty();
        i.insert(source);
        if (!(fresh && launchNewThread_)) {
            return;
        }
        join = currentThread_;
        currentThread_ = new MissingPluginInstallerThread;
        {
            FlagGuard f(inCleanUp_);
            currentSources_.clear();
        }
        processQueue();
        launchNewThread_ = false;
        launch = currentThread_;
    }
    if (join.is()) {
        join->join();
    }
    launch->acquire();
    Application::PostUserEvent(
        LINK(this, MissingPluginInstaller, launchUi), launch.get());
}


void eraseSource(std::set<rtl::Reference<Player>> & set, Player const * source)
{
    auto i = std::find_if(
        set.begin(), set.end(),
        [source](rtl::Reference<Player> const & el) {
            return el.get() == source;
        });
    if (i != set.end()) {
        set.erase(i);
    }
}


void MissingPluginInstaller::detach(Player const * source) {
    rtl::Reference<MissingPluginInstallerThread> join;
    {
        osl::MutexGuard g(mutex_);
        if (inCleanUp_) {
            // Guard against ~MissingPluginInstaller with erroneously un-joined
            // currentThread_ (thus non-empty currentSources_) calling
            // destructor of currentSources_, calling ~Player, calling here,
            // which would use currentSources_ while it is already being
            // destroyed:
            return;
        }
        for (auto i = queued_.begin(); i != queued_.end();) {
            eraseSource(i->second, source);
            if (i->second.empty()) {
                i = queued_.erase(i);
            } else {
                ++i;
            }
        }
        if (currentThread_.is()) {
            assert(!currentSources_.empty());
            eraseSource(currentSources_, source);
            if (currentSources_.empty()) {
                join = currentThread_;
                currentThread_.clear();
                launchNewThread_ = true;
            }
        }
    }
    if (join.is()) {
        // missing cancellability of gst_install_plugins_sync
        join->join();
    }
}


void MissingPluginInstaller::processQueue() {
    assert(!queued_.empty());
    assert(currentDetails_.empty());
    for (const auto& rEntry : queued_) {
        reported_.insert(rEntry.first);
        currentDetails_.push_back(rEntry.first);
        currentSources_.insert(rEntry.second.begin(), rEntry.second.end());
    }
    queued_.clear();
}


IMPL_STATIC_LINK(MissingPluginInstaller, launchUi, void *, p, void)
{
    MissingPluginInstallerThread* thread = static_cast<MissingPluginInstallerThread*>(p);
    rtl::Reference<MissingPluginInstallerThread> ref(thread, SAL_NO_ACQUIRE);
    gst_pb_utils_init();
        // not thread safe; hopefully fine to consistently call from our event
        // loop (which is the only reason to have this
        // Application::PostUserEvent diversion, in case
        // MissingPluginInstaller::report might be called from outside our event
        // loop), and hopefully fine to call gst_is_missing_plugin_message and
        // gst_missing_plugin_message_get_installer_detail before calling
        // gst_pb_utils_init
    ref->launch();
}


struct TheMissingPluginInstaller:
    public rtl::Static<MissingPluginInstaller, TheMissingPluginInstaller>
{};


void MissingPluginInstallerThread::execute() {
    MissingPluginInstaller & inst = TheMissingPluginInstaller::get();
    for (;;) {
        std::vector<OString> details;
        {
            osl::MutexGuard g(inst.mutex_);
            assert(!inst.currentDetails_.empty());
            details.swap(inst.currentDetails_);
        }
        std::vector<char *> args;
        args.reserve(details.size());
        for (auto const& i : details)
        {
            args.push_back(const_cast<char *>(i.getStr()));
        }
        args.push_back(nullptr);
        gst_install_plugins_sync(args.data(), nullptr);
        {
            osl::MutexGuard g(inst.mutex_);
            if (inst.queued_.empty() || inst.launchNewThread_) {
                inst.launchNewThread_ = true;
                break;
            }
            inst.processQueue();
        }
    }
}

} // end anonymous namespace


Player::Player() :
    GstPlayer_BASE( m_aMutex ),
    mpPlaybin( nullptr ),
    mpVolumeControl( nullptr ),
    mbUseGtkSink( false ),
    mbFakeVideo (false ),
    mnUnmutedVolume( 0 ),
    mbPlayPending ( false ),
    mbMuted( false ),
    mbLooping( false ),
    mbInitialized( false ),
    mpDisplay( nullptr ),
    mnWindowID( 0 ),
    mpXOverlay( nullptr ),
    mnDuration( 0 ),
    mnWidth( 0 ),
    mnHeight( 0 ),
    mnWatchID( 0 ),
    mbWatchID( false )
{
    // Initialize GStreamer library
    int argc = 1;
    char name[] = "libreoffice";
    char *arguments[] = { name };
    char** argv = arguments;
    GError* pError = nullptr;

    mbInitialized = gst_init_check( &argc, &argv, &pError );

    SAL_INFO( "avmedia.gstreamer", AVVERSION << this << " Player::Player" );

    if (pError != nullptr)
    {
        // TODO: throw an exception?
        SAL_INFO( "avmedia.gstreamer", AVVERSION << this << " Player::Player error '" << pError->message << "'" );
        g_error_free (pError);
    }
}


Player::~Player()
{
    SAL_INFO( "avmedia.gstreamer", AVVERSION << this << " Player::~Player" );
    if( mbInitialized )
        disposing();
}


void SAL_CALL Player::disposing()
{
    TheMissingPluginInstaller::get().detach(this);

    ::osl::MutexGuard aGuard(m_aMutex);

    stop();

    SAL_INFO( "avmedia.gstreamer", AVVERSION << this << " Player::disposing" );

    // Release the elements and pipeline
    if( mbInitialized )
    {
        if( mpPlaybin )
        {
            gst_element_set_state( mpPlaybin, GST_STATE_NULL );
            g_object_unref( G_OBJECT( mpPlaybin ) );

            mpPlaybin = nullptr;
            mpVolumeControl = nullptr;
        }

        if( mpXOverlay ) {
            g_object_unref( G_OBJECT ( mpXOverlay ) );
            mpXOverlay = nullptr;
        }

    }
    if (mbWatchID)
    {
        g_source_remove(mnWatchID);
        mbWatchID = false;
    }
}


static gboolean pipeline_bus_callback( GstBus *, GstMessage *message, gpointer data )
{
    Player* pPlayer = static_cast<Player*>(data);

    pPlayer->processMessage( message );

    return true;
}


static GstBusSyncReply pipeline_bus_sync_handler( GstBus *, GstMessage * message, gpointer data )
{
    Player* pPlayer = static_cast<Player*>(data);

    return pPlayer->processSyncMessage( message );
}


void Player::processMessage( GstMessage *message )
{
    switch( GST_MESSAGE_TYPE( message ) ) {
    case GST_MESSAGE_EOS:
        gst_element_set_state( mpPlaybin, GST_STATE_READY );
        mbPlayPending = false;
        if (mbLooping)
            start();
        break;
    case GST_MESSAGE_STATE_CHANGED:
        if (message->src == GST_OBJECT(mpPlaybin))
        {
            GstState newstate, pendingstate;

            gst_message_parse_state_changed (message, nullptr, &newstate, &pendingstate);

            if (!mbUseGtkSink && newstate == GST_STATE_PAUSED &&
                pendingstate == GST_STATE_VOID_PENDING && mpXOverlay)
            {
                gst_video_overlay_expose(mpXOverlay);
            }

            if (mbPlayPending)
                mbPlayPending = ((newstate == GST_STATE_READY) || (newstate == GST_STATE_PAUSED));
        }
        break;
    default:
        break;
    }
}

#define LCL_WAYLAND_DISPLAY_HANDLE_CONTEXT_TYPE "GstWaylandDisplayHandleContextType"

static bool lcl_is_wayland_display_handle_need_context_message(GstMessage* msg)
{
    g_return_val_if_fail(GST_IS_MESSAGE(msg), false);

    if (GST_MESSAGE_TYPE(msg) != GST_MESSAGE_NEED_CONTEXT)
        return false;
    const gchar *type = nullptr;
    if (!gst_message_parse_context_type(msg, &type))
        return false;
    return !g_strcmp0(type, LCL_WAYLAND_DISPLAY_HANDLE_CONTEXT_TYPE);
}

static GstContext* lcl_wayland_display_handle_context_new(void* display)
{
    GstContext *context = gst_context_new(LCL_WAYLAND_DISPLAY_HANDLE_CONTEXT_TYPE, true);
    gst_structure_set (gst_context_writable_structure (context),
                       "handle", G_TYPE_POINTER, display, nullptr);
    return context;
}

GstBusSyncReply Player::processSyncMessage( GstMessage *message )
{
#if OSL_DEBUG_LEVEL > 0
    if ( GST_MESSAGE_TYPE( message ) == GST_MESSAGE_ERROR )
    {
        GError* error;
        gchar* error_debug;

        gst_message_parse_error( message, &error, &error_debug );
        SAL_WARN(
            "avmedia.gstreamer",
            "error: '" << error->message << "' debug: '"
                << error_debug << "'");
    }
#endif

    if (!mbUseGtkSink)
    {
        if (gst_is_video_overlay_prepare_window_handle_message (message) )
        {
            SAL_INFO( "avmedia.gstreamer", AVVERSION << this << " processSyncMessage prepare window id: " <<
                      GST_MESSAGE_TYPE_NAME( message ) << " " << static_cast<int>(mnWindowID) );
            if( mpXOverlay )
                g_object_unref( G_OBJECT ( mpXOverlay ) );
            g_object_set( GST_MESSAGE_SRC( message ), "force-aspect-ratio", FALSE, nullptr );
            mpXOverlay = GST_VIDEO_OVERLAY( GST_MESSAGE_SRC( message ) );
            g_object_ref( G_OBJECT ( mpXOverlay ) );
            if ( mnWindowID != 0 )
            {
                gst_video_overlay_set_window_handle( mpXOverlay, mnWindowID );
                gst_video_overlay_handle_events(mpXOverlay, 0); // Let the parent window handle events.
                if (maArea.Width > 0 && maArea.Height > 0)
                    gst_video_overlay_set_render_rectangle(mpXOverlay, maArea.X, maArea.Y, maArea.Width, maArea.Height);
            }

            return GST_BUS_DROP;
        }
        else if (lcl_is_wayland_display_handle_need_context_message(message))
        {
            GstContext *context = lcl_wayland_display_handle_context_new(mpDisplay);
            gst_element_set_context(GST_ELEMENT(GST_MESSAGE_SRC(message)), context);

            return GST_BUS_DROP;
        }
    }

    if( GST_MESSAGE_TYPE( message ) == GST_MESSAGE_ASYNC_DONE ) {
        if( mnDuration == 0) {
            gint64 gst_duration = 0;
            if( gst_element_query_duration( mpPlaybin, GST_FORMAT_TIME, &gst_duration) )
                mnDuration = gst_duration;
        }
        if( mnWidth == 0 ) {
            GstPad *pad = nullptr;

            g_signal_emit_by_name( mpPlaybin, "get-video-pad", 0, &pad );

            if( pad ) {
                int w = 0, h = 0;

                GstCaps *caps = gst_pad_get_current_caps( pad );

                if( gst_structure_get( gst_caps_get_structure( caps, 0 ),
                                       "width", G_TYPE_INT, &w,
                                       "height", G_TYPE_INT, &h,
                                       nullptr ) ) {
                    mnWidth = w;
                    mnHeight = h;

                    SAL_INFO( "avmedia.gstreamer", AVVERSION "queried size: " << mnWidth << "x" << mnHeight );

                }
                gst_caps_unref( caps );
                g_object_unref( pad );
            }

            maSizeCondition.set();
        }
    } else if (gst_is_missing_plugin_message(message)) {
        TheMissingPluginInstaller::get().report(this, message);
        if( mnWidth == 0 ) {
            // an error occurred, set condition so that OOo thread doesn't wait for us
            maSizeCondition.set();
        }
    } else if( GST_MESSAGE_TYPE( message ) == GST_MESSAGE_ERROR ) {
        if( mnWidth == 0 ) {
            // an error occurred, set condition so that OOo thread doesn't wait for us
            maSizeCondition.set();
        }
    }

    return GST_BUS_PASS;
}

void Player::preparePlaybin( std::u16string_view rURL, GstElement *pSink )
{
    if (mpPlaybin != nullptr)
    {
        gst_element_set_state( mpPlaybin, GST_STATE_NULL );
        mbPlayPending = false;
        g_object_unref( mpPlaybin );
    }

    mpPlaybin = gst_element_factory_make( "playbin", nullptr );

    //tdf#96989 on systems with flat-volumes setting the volume directly on the
    //playbin to 100% results in setting the global volume to 100% of the
    //maximum. We expect to set as % of the current volume.
    mpVolumeControl = gst_element_factory_make( "volume", nullptr );
    GstElement *pAudioSink = gst_element_factory_make( "autoaudiosink", nullptr );
    GstElement* pAudioOutput = gst_bin_new("audio-output-bin");
    assert(pAudioOutput);
    if (pAudioSink)
        gst_bin_add(GST_BIN(pAudioOutput), pAudioSink);
    if (mpVolumeControl)
    {
        gst_bin_add(GST_BIN(pAudioOutput), mpVolumeControl);
        if (pAudioSink)
            gst_element_link(mpVolumeControl, pAudioSink);
        GstPad *pPad = gst_element_get_static_pad(mpVolumeControl, "sink");
        gst_element_add_pad(GST_ELEMENT(pAudioOutput), gst_ghost_pad_new("sink", pPad));
        gst_object_unref(GST_OBJECT(pPad));
    }
    g_object_set(G_OBJECT(mpPlaybin), "audio-sink", pAudioOutput, nullptr);

    if( pSink != nullptr ) // used for getting preferred size etc.
    {
        g_object_set( G_OBJECT( mpPlaybin ), "video-sink", pSink, nullptr );
        mbFakeVideo = true;
    }
    else
        mbFakeVideo = false;

    OString ascURL = OUStringToOString( rURL, RTL_TEXTENCODING_UTF8 );
    g_object_set( G_OBJECT( mpPlaybin ), "uri", ascURL.getStr() , nullptr );

    GstBus *pBus = gst_element_get_bus( mpPlaybin );
    if (mbWatchID)
    {
        g_source_remove(mnWatchID);
        mbWatchID = false;
    }
    mnWatchID = gst_bus_add_watch( pBus, pipeline_bus_callback, this );
    mbWatchID = true;
    SAL_INFO( "avmedia.gstreamer", AVVERSION << this << " set sync handler" );
    gst_bus_set_sync_handler( pBus, pipeline_bus_sync_handler, this, nullptr );
    g_object_unref( pBus );
}


bool Player::create( const OUString& rURL )
{
    bool    bRet = false;

    // create all the elements and link them

    SAL_INFO( "avmedia.gstreamer", "create player, URL: '" << rURL << "'" );

    if( mbInitialized && !rURL.isEmpty() )
    {
        // fakesink for pre-roll & sizing ...
        preparePlaybin( rURL, gst_element_factory_make( "fakesink", nullptr ) );

        gst_element_set_state( mpPlaybin, GST_STATE_PAUSED );
        mbPlayPending = false;

        bRet = true;
    }

    if( bRet )
        maURL = rURL;
    else
        maURL.clear();

    return bRet;
}


void SAL_CALL Player::start()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // set the pipeline state to READY and run the loop
    if( mbInitialized && mpPlaybin != nullptr )
    {
        gst_element_set_state( mpPlaybin, GST_STATE_PLAYING );
        mbPlayPending = true;
    }
}


void SAL_CALL Player::stop()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // set the pipeline in PAUSED STATE
    if( mpPlaybin )
        gst_element_set_state( mpPlaybin, GST_STATE_PAUSED );

    mbPlayPending = false;
    SAL_INFO( "avmedia.gstreamer", AVVERSION "stop " << mpPlaybin );
}


sal_Bool SAL_CALL Player::isPlaying()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    bool            bRet = mbPlayPending;

    // return whether the pipeline is in PLAYING STATE or not
    if( !mbPlayPending && mbInitialized && mpPlaybin )
    {
        bRet = GST_STATE( mpPlaybin ) == GST_STATE_PLAYING;
    }

    SAL_INFO( "avmedia.gstreamer", AVVERSION "isPlaying " << bRet );

    return bRet;
}


double SAL_CALL Player::getDuration()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // slideshow checks for non-zero duration, so cheat here
    double duration = 0.3;

    if( mpPlaybin && mnDuration > 0 ) {
        duration = mnDuration / GST_SECOND;
    }

    return duration;
}


void SAL_CALL Player::setMediaTime( double fTime )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if( !mpPlaybin )
        return;

    gint64 gst_position = llround (fTime * GST_SECOND);

    gst_element_seek( mpPlaybin, 1.0,
                      GST_FORMAT_TIME,
                      GST_SEEK_FLAG_FLUSH,
                      GST_SEEK_TYPE_SET, gst_position,
                      GST_SEEK_TYPE_NONE, 0 );

    SAL_INFO( "avmedia.gstreamer", AVVERSION "seek to: " << gst_position << " ns original: " << fTime << " s" );
}


double SAL_CALL Player::getMediaTime()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    double position = 0.0;

    if( mpPlaybin ) {
        // get current position in the stream
        gint64 gst_position;
        if( gst_element_query_position( mpPlaybin, GST_FORMAT_TIME, &gst_position ) )
            position = gst_position / GST_SECOND;
    }

    return position;
}


void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    // TODO check how to do with GST
    mbLooping = bSet;
}


sal_Bool SAL_CALL Player::isPlaybackLoop()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    // TODO check how to do with GST
    return mbLooping;
}


void SAL_CALL Player::setMute( sal_Bool bSet )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    SAL_INFO( "avmedia.gstreamer", AVVERSION "set mute: " << bSet << " muted: " << mbMuted << " unmuted volume: " << mnUnmutedVolume );

    // change the volume to 0 or the unmuted volume
    if (mpVolumeControl && mbMuted != bool(bSet))
    {
        double nVolume = mnUnmutedVolume;
        if( bSet )
        {
            nVolume = 0.0;
        }

        g_object_set( G_OBJECT( mpVolumeControl ), "volume", nVolume, nullptr );

        mbMuted = bSet;
    }
}


sal_Bool SAL_CALL Player::isMute()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    return mbMuted;
}


void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    mnUnmutedVolume = pow( 10.0, nVolumeDB / 20.0 );

    SAL_INFO( "avmedia.gstreamer", AVVERSION "set volume: " << nVolumeDB << " gst volume: " << mnUnmutedVolume );

    // change volume
    if (mpVolumeControl && !mbMuted)
    {
        g_object_set( G_OBJECT( mpVolumeControl ), "volume", mnUnmutedVolume, nullptr );
    }
}


sal_Int16 SAL_CALL Player::getVolumeDB()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    sal_Int16 nVolumeDB(0);

    if (mpVolumeControl)
    {
        double nGstVolume = 0.0;

        g_object_get( G_OBJECT( mpVolumeControl ), "volume", &nGstVolume, nullptr );

        nVolumeDB = static_cast<sal_Int16>( 20.0*log10 ( nGstVolume ) );
    }

    return nVolumeDB;
}


awt::Size SAL_CALL Player::getPreferredPlayerWindowSize()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    awt::Size aSize( 0, 0 );

    if( maURL.isEmpty() )
    {
        SAL_INFO( "avmedia.gstreamer", AVVERSION << this << " Player::getPreferredPlayerWindowSize - empty URL => 0x0" );
        return aSize;
    }

    SAL_INFO( "avmedia.gstreamer", AVVERSION << this << " pre-Player::getPreferredPlayerWindowSize, member " << mnWidth << "x" << mnHeight );

    osl::Condition::Result aResult = maSizeCondition.wait( std::chrono::seconds(10) );

    SAL_INFO( "avmedia.gstreamer", AVVERSION << this << " Player::getPreferredPlayerWindowSize after waitCondition " << aResult << ", member " << mnWidth << "x" << mnHeight );

    if( mnWidth != 0 && mnHeight != 0 ) {
        aSize.Width = mnWidth;
        aSize.Height = mnHeight;
    }

    return aSize;
}

uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow( const uno::Sequence< uno::Any >& rArguments )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    uno::Reference< ::media::XPlayerWindow >    xRet;

    if (rArguments.getLength() > 1)
       rArguments[1] >>= maArea;

    awt::Size aSize = getPreferredPlayerWindowSize();

    if( mbFakeVideo )
        preparePlaybin( maURL, nullptr );

    SAL_INFO( "avmedia.gstreamer", AVVERSION "Player::createPlayerWindow " << aSize.Width << "x" << aSize.Height << " length: " << rArguments.getLength() );

    if( aSize.Width > 0 && aSize.Height > 0 )
    {
        if (rArguments.getLength() <= 2)
        {
            xRet = new ::avmedia::gstreamer::Window;
            return xRet;
        }

        sal_IntPtr pIntPtr = 0;
        rArguments[ 2 ] >>= pIntPtr;
        SystemChildWindow *pParentWindow = reinterpret_cast< SystemChildWindow* >( pIntPtr );
        if (!pParentWindow)
            return nullptr;

        const SystemEnvData* pEnvData = pParentWindow->GetSystemData();
        if (!pEnvData)
            return nullptr;

        // tdf#124027: the position of embedded window is identical w/ the position
        // of media object in all other vclplugs (kf5, gen), in gtk3 w/o gtksink it
        // needs to be translated
        if (pEnvData->toolkit == SystemEnvData::Toolkit::Gtk)
        {
            Point aPoint = pParentWindow->GetPosPixel();
            maArea.X = aPoint.getX();
            maArea.Y = aPoint.getY();
        }

        mbUseGtkSink = false;

        GstElement *pVideosink = static_cast<GstElement*>(pParentWindow->CreateGStreamerSink());
        if (pVideosink)
        {
            if (pEnvData->toolkit == SystemEnvData::Toolkit::Gtk)
                mbUseGtkSink = true;
        }
        else
        {
            if (pEnvData->platform == SystemEnvData::Platform::Wayland)
                pVideosink = gst_element_factory_make("waylandsink", "video-output");
            else
                pVideosink = gst_element_factory_make("autovideosink", "video-output");
            if (!pVideosink)
                return nullptr;
        }

        xRet = new ::avmedia::gstreamer::Window;

        g_object_set(G_OBJECT(mpPlaybin), "video-sink", pVideosink, nullptr);
        g_object_set(G_OBJECT(mpPlaybin), "force-aspect-ratio", FALSE, nullptr);

        if (!mbUseGtkSink)
        {
            mnWindowID = pEnvData->GetWindowHandle(pParentWindow->ImplGetFrame());
            mpDisplay = pEnvData->pDisplay;
            SAL_INFO( "avmedia.gstreamer", AVVERSION "set window id to " << static_cast<int>(mnWindowID) << " XOverlay " << mpXOverlay);
        }
        gst_element_set_state( mpPlaybin, GST_STATE_PAUSED );
        if (!mbUseGtkSink && mpXOverlay)
            gst_video_overlay_set_window_handle( mpXOverlay, mnWindowID );
    }

    return xRet;
}

uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    FrameGrabber* pFrameGrabber = nullptr;
    const awt::Size aPrefSize( getPreferredPlayerWindowSize() );

    if( ( aPrefSize.Width > 0 ) && ( aPrefSize.Height > 0 ) )
        pFrameGrabber = FrameGrabber::create( maURL );
    SAL_INFO( "avmedia.gstreamer", AVVERSION "created FrameGrabber " << pFrameGrabber );

    return pFrameGrabber;
}


OUString SAL_CALL Player::getImplementationName()
{
    return AVMEDIA_GST_PLAYER_IMPLEMENTATIONNAME;
}


sal_Bool SAL_CALL Player::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}


uno::Sequence< OUString > SAL_CALL Player::getSupportedServiceNames()
{
    return { AVMEDIA_GST_PLAYER_SERVICENAME };
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
