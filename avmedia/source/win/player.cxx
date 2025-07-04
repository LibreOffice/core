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

#include <objbase.h>
#include <uuids.h>
#include <evcode.h>
#include <propvarutil.h>
#include <propkey.h>
// Media Foundation headers
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#include <avmedia/mediaitem.hxx>
#include <avmedia/mediawindow.hxx>

#include "player.hxx"
#include "framegrabber.hxx"
#include "window.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <osl/file.hxx>
#include <rtl/ref.hxx>

constexpr OUStringLiteral AVMEDIA_WIN_PLAYER_IMPLEMENTATIONNAME = u"com.sun.star.comp.avmedia.Player_DirectX";
constexpr OUString AVMEDIA_WIN_PLAYER_SERVICENAME = u"com.sun.star.media.Player_DirectX"_ustr;
constexpr float AVMEDIA_DB_RANGE = 40.0;

using namespace ::com::sun::star;

namespace avmedia::win {

static LRESULT CALLBACK MediaPlayerWndProc_2( HWND /*hWnd*/, UINT /*nMsg*/, WPARAM /*nPar1*/, LPARAM /*nPar2*/ )
{
    return 0;
}

Player::Player() :
    Player_BASE(m_aMutex),
    sal::systools::CoInitializeGuard(COINIT_APARTMENTTHREADED, false,
                                     sal::systools::CoInitializeGuard::WhenFailed::NoThrow),
    m_cRef( 1 ),
    mnUnmutedVolume( 1 ),
    mnFrameWnd( nullptr ),
    mbMuted( false ),
    mbLooping( false ),
    mbAutoPlayBack( false ),
    mnFrameWidth( 0 ),
    mnFrameHeight( 0 ),
    g_pPlayer( nullptr ),
    g_bHasVideo( false ),
    g_bHasAudio( false ),
    m_state( Closed )
{
}

Player::~Player()
{
    if( mnFrameWnd )
        ::DestroyWindow( mnFrameWnd );

    if (g_pPlayer)
        g_pPlayer->Shutdown();
    SafeRelease(&g_pPlayer);
}

//***************************** IUnknown methods *****************************//


//------------------------------------------------------------------------------
//  AddRef
//------------------------------------------------------------------------------

ULONG Player::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

//------------------------------------------------------------------------------
//  Release
//------------------------------------------------------------------------------

ULONG Player::Release()
{
    ULONG uCount = InterlockedDecrement(&m_cRef);
    if (uCount == 0)
    {
        delete this;
    }
    return uCount;
}

//------------------------------------------------------------------------------
//  QueryInterface
//------------------------------------------------------------------------------

STDMETHODIMP Player::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(Player, IMFPMediaPlayerCallback),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

//********************* IMFPMediaPlayerCallback methods **********************//

//-----------------------------------------------------------------------------
// OnMediaPlayerEvent
//
// Notifies the object of an MFPlay event.
//-----------------------------------------------------------------------------

void Player::OnMediaPlayerEvent(MFP_EVENT_HEADER* pEventHeader)
{
    if (FAILED(pEventHeader->hrEvent))
    {
        SAL_WARN("avmedia.win",
            "Player::OnMediaPlayerEvent failed with error code: " << pEventHeader->hrEvent);

        ::avmedia::MediaWindow::executeFormatErrorBox(nullptr);

        return;
    }

    switch (pEventHeader->eEventType)
    {
    case MFP_EVENT_TYPE_MEDIAITEM_CREATED:
        OnMediaItemCreated(MFP_GET_MEDIAITEM_CREATED_EVENT(pEventHeader));
        break;

    case MFP_EVENT_TYPE_MEDIAITEM_SET:
        OnMediaItemSet(MFP_GET_MEDIAITEM_SET_EVENT(pEventHeader));
        break;

    case MFP_EVENT_TYPE_POSITION_SET:
        OnMediaPosSet(MFP_GET_POSITION_SET_EVENT(pEventHeader));
        break;

    case MFP_EVENT_TYPE_PLAYBACK_ENDED:
        OnMediaItemEnded(MFP_GET_PLAYBACK_ENDED_EVENT(pEventHeader));
        break;
    }
}

void SAL_CALL Player::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    OnClose(mnFrameWnd);
}

bool Player::create( const OUString& rURL )
{
    bool bRet = !rURL.isEmpty();
    if (bRet)
    {
        maURL = rURL;
        // Initialize the Media Foundation platform.
        HRESULT hr = MFStartup(MF_VERSION);
        if (SUCCEEDED(hr))
        {
            // Create the source reader.
            IMFSourceReader* pReader;
            hr = MFCreateSourceReaderFromURL(o3tl::toW(maURL.getStr()), nullptr, &pReader);
            if (SUCCEEDED(hr))
            {
                IMFMediaType* pType = nullptr;
                DWORD dwMediaTypeIndex = 0;
                if (SUCCEEDED(pReader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, dwMediaTypeIndex, &pType)))
                {
                    MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &mnFrameWidth, &mnFrameHeight);
                    SafeRelease(&pType);
                }
                SafeRelease(&pReader);
            }
            // Shut down Media Foundation.
            MFShutdown();
        }
        bRet = SUCCEEDED(hr);
    }
    else
        maURL.clear();

    return bRet;
}

void Player::setNotifyWnd( HWND nNotifyWnd )
{
    mnFrameWnd = nNotifyWnd;
}

//-------------------------------------------------------------------
// OnClose
//
// Handles the WM_CLOSE message.
//-------------------------------------------------------------------
void Player::OnClose(HWND /*hwnd*/)
{
    if (g_pPlayer)
        g_pPlayer->Shutdown();
    SafeRelease(&g_pPlayer);

    PostQuitMessage(0);
}

//-------------------------------------------------------------------
// OnPaint
//
// Handles the WM_PAINT message.
//-------------------------------------------------------------------
void Player::OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = 0;

    hdc = BeginPaint(hwnd, &ps);

    if (g_pPlayer && g_bHasVideo)
    {
        // Playback has started and there is video.

        // Do not draw the window background, because the video
        // frame fills the entire client area.

        g_pPlayer->UpdateVideo();
    }
    else
    {
        // There is no video stream, or playback has not started.
        // Paint the entire client area.

        FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
    }

    EndPaint(hwnd, &ps);
}

//-------------------------------------------------------------------
// OnSize
//
// Handles the WM_SIZE message.
//-------------------------------------------------------------------
void Player::OnSize(HWND /*hwnd*/, UINT state, int /*cx*/, int /*cy*/)
{
    if (state == SIZE_RESTORED)
    {
        if (g_pPlayer)
        {
            // Resize the video.
            g_pPlayer->UpdateVideo();
        }
    }
}

//-------------------------------------------------------------------
// OnMediaItemCreated
//
// Called when the IMFPMediaPlayer::CreateMediaItemFromURL method
// completes.
//-------------------------------------------------------------------
void Player::OnMediaItemCreated(MFP_MEDIAITEM_CREATED_EVENT* pEvent)
{
    if (g_pPlayer)
    {
        BOOL bHasVideoOrAudio = FALSE, bIsSelected = FALSE;

        // Check if the media item contains video or audio.
        HRESULT hr = pEvent->pMediaItem->HasVideo(&bHasVideoOrAudio, &bIsSelected);
        if (SUCCEEDED(hr))
            g_bHasVideo = bHasVideoOrAudio && bIsSelected;

        hr = pEvent->pMediaItem->HasAudio(&bHasVideoOrAudio, &bIsSelected);
        if (SUCCEEDED(hr))
            g_bHasAudio = bHasVideoOrAudio && bIsSelected;

        if (SUCCEEDED(hr) && (g_bHasVideo || g_bHasAudio))
        {
            // Set the media item on the player. This method completes asynchronously.
            hr = g_pPlayer->SetMediaItem(pEvent->pMediaItem);
        }

        if (FAILED(hr) || (!g_bHasVideo && !g_bHasAudio))
        {
            SAL_WARN("avmedia.win",
                "Player::OnMediaItemCreated failed with error code: " << hr);

            ::avmedia::MediaWindow::executeFormatErrorBox(nullptr);
            m_state = Closed;
        }
    }
}

//-------------------------------------------------------------------
// OnMediaItemSet
//
// Called when the IMFPMediaPlayer::SetMediaItem method completes.
//-------------------------------------------------------------------
void Player::OnMediaItemSet(MFP_MEDIAITEM_SET_EVENT* /*pEvent*/)
{
    HRESULT hr = S_OK;

    if (mbAutoPlayBack)
    {
        hr = g_pPlayer->Play();
        if (SUCCEEDED(hr))
            m_state = Started;
    }

    if (FAILED(hr))
    {
        SAL_WARN("avmedia.win",
            "Player::OnMediaItemSet failed with error code: " << hr);

        ::avmedia::MediaWindow::executeFormatErrorBox(nullptr);
    }
}

void Player::OnMediaPosSet(MFP_POSITION_SET_EVENT* /*pEvent*/)
{
    // TODO: Handle position set event if needed with update of MediaPlayer UI.
    // void MediaWindowControl::update() should be called
}

void Player::OnMediaItemEnded(MFP_PLAYBACK_ENDED_EVENT* /*pEvent*/)
{
    if( mbLooping )
        start();
    else
        m_state = Stopped;
}

HRESULT Player::InitializeWindow(bool bAddSoundWindow)
{
    HRESULT hr = S_OK;
    SafeRelease(&g_pPlayer);

    if (bAddSoundWindow)
    {
        static WNDCLASSW* mpWndClass = nullptr;
        if ( !mpWndClass )
        {
            mpWndClass = new WNDCLASSW;

            memset( mpWndClass, 0, sizeof( *mpWndClass ) );
            mpWndClass->hInstance = GetModuleHandleW( nullptr );
            mpWndClass->cbWndExtra = sizeof( DWORD );
            mpWndClass->lpfnWndProc = MediaPlayerWndProc_2;
            mpWndClass->lpszClassName = L"com_sun_star_media_Sound_Player";
            mpWndClass->hbrBackground = static_cast<HBRUSH>(::GetStockObject( BLACK_BRUSH ));
            mpWndClass->hCursor = ::LoadCursor( nullptr, IDC_ARROW );

            RegisterClassW( mpWndClass );
        }
        if ( !mnFrameWnd )
        {
            mnFrameWnd = CreateWindowW( mpWndClass->lpszClassName, nullptr,
                                        0,
                                        0, 0, 0, 0,
                                        nullptr, nullptr, mpWndClass->hInstance, nullptr);
            if ( mnFrameWnd )
            {
                ::ShowWindow(mnFrameWnd, SW_HIDE);
                SetWindowLongPtrW( mnFrameWnd, 0, reinterpret_cast<LONG_PTR>(this) );
            }
        }
    }

    // Create the MFPlayer object.
    hr = MFPCreateMediaPlayer(
        nullptr,
        FALSE,          // Start playback automatically?
        0,              // Flags
        this,           // Callback pointer
        mnFrameWnd,     // Video window
        &g_pPlayer
    );

    // Create a new media item for this URL.
    // The CreateMediaItemFromURL method completes asynchronously. When it does,
    // MFPlay sends an MFP_EVENT_TYPE_MEDIAITEM_CREATED event.
    if (SUCCEEDED(hr))
        hr = g_pPlayer->CreateMediaItemFromURL(o3tl::toW(maURL.getStr()), FALSE, 0, nullptr);

    if (SUCCEEDED(hr))
        m_state = Stopped;

    return hr;
}

void SAL_CALL Player::start(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if( g_pPlayer )
    {
        HRESULT hr = S_OK;
        if (g_bHasVideo || g_bHasAudio)
        {
            MFP_MEDIAPLAYER_STATE state = MFP_MEDIAPLAYER_STATE_EMPTY;
            hr = g_pPlayer->GetState(&state);
            if (SUCCEEDED(hr))
            {
                if (state == MFP_MEDIAPLAYER_STATE_PAUSED || state == MFP_MEDIAPLAYER_STATE_STOPPED)
                {
                    hr = g_pPlayer->Play();
                    if (SUCCEEDED(hr))
                        m_state = Started;
                }
            }
        }

        if (FAILED(hr))
        {
            SAL_WARN("avmedia.win",
                "Player::start failed with error code: " << hr);

            ::avmedia::MediaWindow::executeFormatErrorBox(nullptr);
        }
    }
}

void SAL_CALL Player::stop(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (g_pPlayer && (g_bHasVideo || g_bHasAudio))
    {
        HRESULT hr = S_OK;
        if (getMediaTime())
        {
            hr = g_pPlayer->Pause();
            if (SUCCEEDED(hr))
                m_state = Paused;
        }
        else
        {
            hr = g_pPlayer->Stop();
            if (SUCCEEDED(hr))
                m_state = Stopped;
        }
    }
}

sal_Bool SAL_CALL Player::isPlaying()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    bool bRet = false;
    if (g_pPlayer && (g_bHasVideo || g_bHasAudio))
    {
        MFP_MEDIAPLAYER_STATE state = MFP_MEDIAPLAYER_STATE_EMPTY;
        HRESULT hr = g_pPlayer->GetState(&state);
        if (SUCCEEDED(hr))
        {
            bRet = (state == MFP_MEDIAPLAYER_STATE_PLAYING && m_state == Started);
        }
        else
        {
            SAL_WARN("avmedia.win",
                "Player::isPlaying failed with error code: " << hr);
        }
    }

    return bRet;
}

double SAL_CALL Player::getDuration(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    REFTIME aRefTime( 0.0 );
    if (g_pPlayer && (g_bHasVideo || g_bHasAudio))
    {
        PROPVARIANT var;
        HRESULT hr = g_pPlayer->GetDuration(MFP_POSITIONTYPE_100NS, &var);
        if (SUCCEEDED(hr))
        {
            hr = PropVariantToDouble(var, &aRefTime);
            aRefTime = aRefTime / (1000.0 * 10.0) / 1000.0; // Convert from 100-nanosecond units to seconds
            PropVariantClear(&var);
        }
    }
    return aRefTime;
}

void SAL_CALL Player::setMediaTime( double fTime )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if (g_pPlayer && (g_bHasVideo || g_bHasAudio))
    {
        PROPVARIANT var;
        PropVariantInit(&var);
        var.vt = VT_I8; // PROPVARIANT type has to be VT_I8.
        var.hVal.QuadPart = static_cast<MFTIME>(fTime * 1000.0 * (1000.0 * 10.0)); // Convert from seconds to 100-nanosecond units
        HRESULT hr = g_pPlayer->SetPosition(MFP_POSITIONTYPE_100NS, &var);
        if (FAILED(hr))
        {
            SAL_WARN("avmedia.win",
                     "Player::setMediaTime: setMediaTime failed with error code: " << hr);
        }
        PropVariantClear(&var);
        // on resetting back to zero the reported timestamp doesn't seem to get
        // updated in a reasonable time, so on zero just force an update of timestamp to 0.
        // Same as in /core/avmedia/source/gtk/gtkplayer.cxx:GtkPlayer::setMediaTime(double fTime)
        // With UpdateVideo() it can help on video streams, but doesn't help on audio streams. TODO!
        if (fTime == 0.0)
            g_pPlayer->UpdateVideo();
    }
}

double SAL_CALL Player::getMediaTime(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    REFTIME aRefTime( 0.0 );
    if (g_pPlayer && (g_bHasVideo || g_bHasAudio))
    {
        PROPVARIANT var;
        HRESULT hr = g_pPlayer->GetPosition(MFP_POSITIONTYPE_100NS, &var);
        if (SUCCEEDED(hr))
        {
            hr = PropVariantToDouble(var, &aRefTime);
            aRefTime = aRefTime / (1000.0 * 10.0) / 1000.0; // Convert from 100-nanosecond units to seconds
            PropVariantClear(&var);
        }
    }

    return aRefTime;
}

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    mbLooping = bSet;
}

sal_Bool SAL_CALL Player::isPlaybackLoop(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    return mbLooping;
}

void SAL_CALL Player::setMute( sal_Bool bSet )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if (g_pPlayer && (g_bHasVideo || g_bHasAudio) &&
        (mbMuted != static_cast<BOOL>(bSet)))
    {
        mbMuted = bSet;
        HRESULT hr = g_pPlayer->SetMute(bSet);
        if (FAILED(hr))
        {
            SAL_WARN("avmedia.win",
                     "Player::setMute: setMute failed with error code: " << hr);
        }
    }
}

sal_Bool SAL_CALL Player::isMute(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if (g_pPlayer && (g_bHasVideo || g_bHasAudio))
    {
        HRESULT hr = g_pPlayer->GetMute(&mbMuted);
        if (FAILED(hr))
        {
            SAL_WARN("avmedia.win",
                     "Player::isMute: getMute failed with error code: " << hr);
            mbMuted = false; // Reset to default if error occurs
        }
    }
    return mbMuted;
}

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    mnUnmutedVolume = static_cast< float >( (nVolumeDB / AVMEDIA_DB_RANGE) + 1.0 );

    if ( g_pPlayer && (g_bHasVideo || g_bHasAudio))
    {
        HRESULT hr = g_pPlayer->SetVolume(mnUnmutedVolume);
        if (FAILED(hr))
        {
            SAL_WARN("avmedia.win",
                     "Player::setVolumeDB: setVolumeDB failed with error code: " << hr);
        }
    }
}

sal_Int16 SAL_CALL Player::getVolumeDB(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if (g_pPlayer && (g_bHasVideo || g_bHasAudio))
    {
        HRESULT hr = g_pPlayer->GetVolume(&mnUnmutedVolume);
        if (FAILED(hr))
        {
            SAL_WARN("avmedia.win",
                "Player::getVolumeDB: getVolumeDB failed with error code: " << hr);
            mnUnmutedVolume = 1; // Reset to default if error occurs
        }
    }
    return static_cast< sal_Int16 >( (mnUnmutedVolume - 1.0) * AVMEDIA_DB_RANGE);
}

awt::Size SAL_CALL Player::getPreferredPlayerWindowSize(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    return awt::Size(mnFrameWidth, mnFrameHeight);
}

uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    uno::Reference< ::media::XPlayerWindow >    xRet;
    awt::Size aSize(getPreferredPlayerWindowSize());

    if( aSize.Width > 0 && aSize.Height > 0 )
    {
        rtl::Reference<::avmedia::win::Window> pWindow = new ::avmedia::win::Window( *this );

        xRet = pWindow;

        if (!pWindow->create( aArguments ) )
            xRet.clear();
    }
    else
    {
        if (SUCCEEDED(InitializeWindow(true)))
        {
            sal_IntPtr pIntPtr = 0;
            if ((aArguments.getLength() >= 4) && (aArguments[3] >>= pIntPtr) && pIntPtr)
            {
                auto pItem = reinterpret_cast<const avmedia::MediaItem*>(pIntPtr);
                if (pItem->getState() == avmedia::MediaState::Play)
                {
                    setAutoPlayBack(true);
                }
            }
        }
        else
        {
            ::avmedia::MediaWindow::executeFormatErrorBox(nullptr);
        }
    }

    return xRet;
}

uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber(  )
{
    uno::Reference< media::XFrameGrabber > xRet;

    if( !maURL.isEmpty() )
    {
        rtl::Reference<FrameGrabber> pGrabber = new FrameGrabber(maURL, mnFrameWidth, mnFrameHeight);
        xRet = pGrabber;
    }

    return xRet;
}

OUString SAL_CALL Player::getImplementationName(  )
{
    return AVMEDIA_WIN_PLAYER_IMPLEMENTATIONNAME;
}

sal_Bool SAL_CALL Player::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL Player::getSupportedServiceNames(  )
{
    return { AVMEDIA_WIN_PLAYER_SERVICENAME };
}

} // namespace avmedia::win


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
