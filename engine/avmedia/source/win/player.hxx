/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <sal/config.h>

#include <WinDef.h>

// Media Foundation headers
#include <mfplay.h>
#include <mferror.h>

#include "wincommon.hxx"

#include <com/sun/star/media/XPlayer.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <systools/win32/comtools.hxx>


namespace avmedia::win {

enum PlayerState
{
    Closed = 0,     // No session.
    Started,        // Session is playing a file.
    Paused,         // Session is paused.
    Stopped         // Session is stopped (ready to play).
};

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

typedef ::cppu::WeakComponentImplHelper< css::media::XPlayer,
                                         css::lang::XServiceInfo > Player_BASE;

class Player : public IMFPMediaPlayerCallback,
               public cppu::BaseMutex,
               public Player_BASE,
               public sal::systools::CoInitializeGuard
{
public:

    explicit Player();
    ~Player() override;

    bool    create( const OUString& rURL );
    HRESULT InitializeWindow( bool bAddSoundWindow );
    void    setNotifyWnd( HWND nNotifyWnd );
    HWND*   getNotifyWnd() { return &mnFrameWnd; }
    void    setAutoPlayBack(bool bVal) { mbAutoPlayBack = bVal; }
    const UINT32  GetVideoWidth() const { return mnFrameWidth; }
    const UINT32  GetVideoHeight() const { return mnFrameHeight; }

    // Error handling
    void ShowErrorMessage(HRESULT hr, bool bPopUpWin);

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFPMediaPlayerCallback methods
    void STDMETHODCALLTYPE OnMediaPlayerEvent(MFP_EVENT_HEADER* pEventHeader);

    // Window message handlers
    void    OnClose(HWND hwnd);
    void    OnPaint(HWND hwnd);
    void    OnSize(HWND hwnd, UINT state, int cx, int cy);

    // MFPlay event handler functions.
    void    OnMediaItemCreated(MFP_MEDIAITEM_CREATED_EVENT* pEvent);
    void    OnMediaItemSet(MFP_MEDIAITEM_SET_EVENT* pEvent);
    void    OnMediaPosSet(MFP_POSITION_SET_EVENT* pEvent);
    void    OnMediaItemEnded(MFP_PLAYBACK_ENDED_EVENT* pEvent);

    // XPlayer
    virtual void start(  ) override;
    virtual void stop(  ) override;
    virtual bool isPlaying(  ) override;
    virtual double getDuration(  ) override;
    virtual void setMediaTime( double fTime ) override;
    virtual double getMediaTime(  ) override;
    virtual void setPlaybackLoop( bool bSet ) override;
    virtual bool isPlaybackLoop(  ) override;
    virtual void setMute( bool bSet ) override;
    virtual bool isMute(  ) override;
    virtual void setVolumeDB( sal_Int16 nVolumeDB ) override;
    virtual sal_Int16 getVolumeDB(  ) override;
    virtual css::awt::Size getPreferredPlayerWindowSize(  ) override;
    virtual css::uno::Reference< css::media::XPlayerWindow > createPlayerWindow( const css::uno::Sequence< cpo::uno::Any >& aArguments ) override;
    virtual css::uno::Reference< css::media::XFrameGrabber > createFrameGrabber(  ) override;

    // XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // ::cppu::OComponentHelper
    virtual void disposing() override;

private:

    long                    m_cRef;          // Reference count.
    OUString                maURL;
    float                   mnUnmutedVolume;
    HWND                    mnFrameWnd;
    BOOL                    mbMuted;
    bool                    mbLooping;
    bool                    mbAutoPlayBack;
    UINT32                  mnFrameWidth;
    UINT32                  mnFrameHeight;
    IMFPMediaPlayer*        g_pPlayer;      // The MFPlay player object.
    BOOL                    g_bHasVideo;
    BOOL                    g_bHasAudio;
    PlayerState             m_state;
};

} // namespace avmedia::win


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
