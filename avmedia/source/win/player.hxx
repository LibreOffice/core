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
    virtual void SAL_CALL start(  ) override;
    virtual void SAL_CALL stop(  ) override;
    virtual sal_Bool SAL_CALL isPlaying(  ) override;
    virtual double SAL_CALL getDuration(  ) override;
    virtual void SAL_CALL setMediaTime( double fTime ) override;
    virtual double SAL_CALL getMediaTime(  ) override;
    virtual void SAL_CALL setPlaybackLoop( sal_Bool bSet ) override;
    virtual sal_Bool SAL_CALL isPlaybackLoop(  ) override;
    virtual void SAL_CALL setMute( sal_Bool bSet ) override;
    virtual sal_Bool SAL_CALL isMute(  ) override;
    virtual void SAL_CALL setVolumeDB( sal_Int16 nVolumeDB ) override;
    virtual sal_Int16 SAL_CALL getVolumeDB(  ) override;
    virtual css::awt::Size SAL_CALL getPreferredPlayerWindowSize(  ) override;
    virtual css::uno::Reference< css::media::XPlayerWindow > SAL_CALL createPlayerWindow( const css::uno::Sequence< css::uno::Any >& aArguments ) override;
    virtual css::uno::Reference< css::media::XFrameGrabber > SAL_CALL createFrameGrabber(  ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // ::cppu::OComponentHelper
    virtual void SAL_CALL disposing() override;

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
