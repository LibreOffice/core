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

#include <string.h>
#include <vcl/svpforlokit.hxx>
#include <vcl/syswin.hxx>

#include "headless/svpframe.hxx"
#include "headless/svpinst.hxx"
#include "headless/svpgdi.hxx"

#include <basebmp/bitmapdevice.hxx>
#include <basebmp/scanlineformats.hxx>
#include <basegfx/vector/b2ivector.hxx>

using namespace basebmp;
using namespace basegfx;

SvpSalFrame* SvpSalFrame::s_pFocusFrame = nullptr;

#ifdef IOS
#define SvpSalGraphics AquaSalGraphics
#endif

#ifndef IOS
namespace {
    /// Decouple SalFrame lifetime from damagetracker lifetime
    struct DamageTracker : public basebmp::IBitmapDeviceDamageTracker
    {
        virtual ~DamageTracker() {}
        virtual void damaged( const basegfx::B2IBox& ) const override {}
    };
}
#endif

#ifdef ANDROID
void SvpSalFrame::enableDamageTracker( bool bOn )
{
    if( m_bDamageTracking == bOn )
        return;
    if( m_aFrame.get() )
    {
        if( m_bDamageTracking )
            m_aFrame->setDamageTracker( basebmp::IBitmapDeviceDamageTrackerSharedPtr() );
        else
            m_aFrame->setDamageTracker(
                basebmp::IBitmapDeviceDamageTrackerSharedPtr( new DamageTracker ) );
    }
    m_bDamageTracking = bOn;
}
#endif


SvpSalFrame::SvpSalFrame( SvpSalInstance* pInstance,
                          SalFrame* pParent,
                          SalFrameStyleFlags nSalFrameStyle,
                          basebmp::Format nScanlineFormat,
                          SystemParentData* ) :
    m_pInstance( pInstance ),
    m_pParent( static_cast<SvpSalFrame*>(pParent) ),
    m_nStyle( nSalFrameStyle ),
    m_bVisible( false ),
#ifndef IOS
    m_bDamageTracking( false ),
    m_nScanlineFormat( nScanlineFormat ),
#endif
    m_nMinWidth( 0 ),
    m_nMinHeight( 0 ),
    m_nMaxWidth( 0 ),
    m_nMaxHeight( 0 )
{
    // SAL_DEBUG("SvpSalFrame::SvpSalFrame: " << this);
    // fast and easy cross-platform wiping of the data
    memset( static_cast<void *>(&m_aSystemChildData), 0, sizeof( SystemEnvData ) );
    m_aSystemChildData.nSize        = sizeof( SystemEnvData );
#ifdef IOS
    (void) nScanlineFormat;
#elif defined ANDROID
    // Nothing
#else
    m_aSystemChildData.pSalFrame    = this;
    m_aSystemChildData.nDepth       = 24;
#endif

    if( m_pParent )
        m_pParent->m_aChildren.push_back( this );

    if( m_pInstance )
        m_pInstance->registerFrame( this );

    SetPosSize( 0, 0, 800, 600, SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
}

SvpSalFrame::~SvpSalFrame()
{
    if( m_pInstance )
        m_pInstance->deregisterFrame( this );

    std::list<SvpSalFrame*> Children = m_aChildren;
    for( std::list<SvpSalFrame*>::iterator it = Children.begin();
         it != Children.end(); ++it )
         (*it)->SetParent( m_pParent );
    if( m_pParent )
        m_pParent->m_aChildren.remove( this );

    if( s_pFocusFrame == this )
    {
        // SAL_DEBUG("SvpSalFrame::~SvpSalFrame: losing focus: " << this);
        s_pFocusFrame = nullptr;
        // call directly here, else an event for a destroyed frame would be dispatched
        CallCallback( SALEVENT_LOSEFOCUS, nullptr );
        // if the handler has not set a new focus frame
        // pass focus to another frame, preferably a document style window
        if( s_pFocusFrame == nullptr )
        {
            const std::list< SalFrame* >& rFrames( m_pInstance->getFrames() );
            for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
            {
                SvpSalFrame* pFrame = const_cast<SvpSalFrame*>(static_cast<const SvpSalFrame*>(*it));
                if( pFrame->m_bVisible        &&
                    pFrame->m_pParent == nullptr &&
                    (pFrame->m_nStyle & (SalFrameStyleFlags::MOVEABLE |
                                         SalFrameStyleFlags::SIZEABLE |
                                         SalFrameStyleFlags::CLOSEABLE) )
                    )
                {
                    pFrame->GetFocus();
                    break;
                }
            }
        }
    }
}

void SvpSalFrame::GetFocus()
{
    if( s_pFocusFrame == this )
        return;

    if( (m_nStyle & (SalFrameStyleFlags::OWNERDRAWDECORATION | SalFrameStyleFlags::FLOAT)) == SalFrameStyleFlags::NONE )
    {
        if( s_pFocusFrame )
            s_pFocusFrame->LoseFocus();
        // SAL_DEBUG("SvpSalFrame::GetFocus(): " << this);
        s_pFocusFrame = this;
        m_pInstance->PostEvent( this, nullptr, SALEVENT_GETFOCUS );
    }
}

void SvpSalFrame::LoseFocus()
{
    if( s_pFocusFrame == this )
    {
        // SAL_DEBUG("SvpSalFrame::LoseFocus: " << this);
        m_pInstance->PostEvent( this, nullptr, SALEVENT_LOSEFOCUS );
        s_pFocusFrame = nullptr;
    }
}

SalGraphics* SvpSalFrame::AcquireGraphics()
{
    SvpSalGraphics* pGraphics = new SvpSalGraphics();
#ifndef IOS
    pGraphics->setDevice( m_aFrame );
#endif
    m_aGraphics.push_back( pGraphics );
    return pGraphics;
}

void SvpSalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
    SvpSalGraphics* pSvpGraphics = dynamic_cast<SvpSalGraphics*>(pGraphics);
    m_aGraphics.remove( pSvpGraphics );
    delete pSvpGraphics;
}

bool SvpSalFrame::PostEvent(ImplSVEvent* pData)
{
    m_pInstance->PostEvent( this, pData, SALEVENT_USEREVENT );
    return true;
}

void SvpSalFrame::PostPaint(bool bImmediate) const
{
    if( m_bVisible )
    {
        SalPaintEvent aPEvt(0, 0, maGeometry.nWidth, maGeometry.nHeight);
        aPEvt.mbImmediateUpdate = bImmediate;
        CallCallback( SALEVENT_PAINT, &aPEvt );
    }
}

void SvpSalFrame::SetTitle( const OUString& )
{
}

void SvpSalFrame::SetIcon( sal_uInt16 )
{
}

void SvpSalFrame::SetMenu( SalMenu* )
{
}

void SvpSalFrame::DrawMenuBar()
{
}

void SvpSalFrame::SetExtendedFrameStyle( SalExtStyle )
{
}

void SvpSalFrame::Show( bool bVisible, bool bNoActivate )
{
    if( bVisible && ! m_bVisible )
    {
        // SAL_DEBUG("SvpSalFrame::Show: showing: " << this);
        m_bVisible = true;
        m_pInstance->PostEvent( this, nullptr, SALEVENT_RESIZE );
        if( ! bNoActivate )
            GetFocus();
    }
    else if( ! bVisible && m_bVisible )
    {
        // SAL_DEBUG("SvpSalFrame::Show: hiding: " << this);
        m_bVisible = false;
        m_pInstance->PostEvent( this, nullptr, SALEVENT_RESIZE );
        LoseFocus();
    }
    else
    {
        // SAL_DEBUG("SvpSalFrame::Show: nothing: " << this);
    }
}

void SvpSalFrame::SetMinClientSize( long nWidth, long nHeight )
{
    m_nMinWidth = nWidth;
    m_nMinHeight = nHeight;
}

void SvpSalFrame::SetMaxClientSize( long nWidth, long nHeight )
{
    m_nMaxWidth = nWidth;
    m_nMaxHeight = nHeight;
}

void SvpSalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags )
{
    if( (nFlags & SAL_FRAME_POSSIZE_X) != 0 )
        maGeometry.nX = nX;
    if( (nFlags & SAL_FRAME_POSSIZE_Y) != 0 )
        maGeometry.nY = nY;
    if( (nFlags & SAL_FRAME_POSSIZE_WIDTH) != 0 )
    {
        maGeometry.nWidth = nWidth;
        if( m_nMaxWidth > 0 && maGeometry.nWidth > (unsigned int)m_nMaxWidth )
            maGeometry.nWidth = m_nMaxWidth;
        if( m_nMinWidth > 0 && maGeometry.nWidth < (unsigned int)m_nMinWidth )
            maGeometry.nWidth = m_nMinWidth;
    }
    if( (nFlags & SAL_FRAME_POSSIZE_HEIGHT) != 0 )
    {
        maGeometry.nHeight = nHeight;
        if( m_nMaxHeight > 0 && maGeometry.nHeight > (unsigned int)m_nMaxHeight )
            maGeometry.nHeight = m_nMaxHeight;
        if( m_nMinHeight > 0 && maGeometry.nHeight < (unsigned int)m_nMinHeight )
            maGeometry.nHeight = m_nMinHeight;
    }
#ifndef IOS
    B2IVector aFrameSize( maGeometry.nWidth, maGeometry.nHeight );
    if( ! m_aFrame.get() || m_aFrame->getSize() != aFrameSize )
    {
        if( aFrameSize.getX() == 0 )
            aFrameSize.setX( 1 );
        if( aFrameSize.getY() == 0 )
            aFrameSize.setY( 1 );
        m_aFrame = createBitmapDevice( aFrameSize, true, m_nScanlineFormat );
        if (m_bDamageTracking)
            m_aFrame->setDamageTracker(
                basebmp::IBitmapDeviceDamageTrackerSharedPtr( new DamageTracker ) );
        // update device in existing graphics
        for( std::list< SvpSalGraphics* >::iterator it = m_aGraphics.begin();
             it != m_aGraphics.end(); ++it )
        {
             (*it)->setDevice( m_aFrame );
        }
    }
    if( m_bVisible )
        m_pInstance->PostEvent( this, nullptr, SALEVENT_RESIZE );
#endif
}

void SvpSalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    rWidth = maGeometry.nWidth;
    rHeight = maGeometry.nHeight;
}

void SvpSalFrame::GetWorkArea( Rectangle& rRect )
{
    rRect = Rectangle( Point( 0, 0 ),
                       Size( VIRTUAL_DESKTOP_WIDTH, VIRTUAL_DESKTOP_HEIGHT ) );
}

SalFrame* SvpSalFrame::GetParent() const
{
    return m_pParent;
}

#define _FRAMESTATE_MASK_GEOMETRY \
     (WINDOWSTATE_MASK_X     | WINDOWSTATE_MASK_Y |   \
      WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT)

void SvpSalFrame::SetWindowState( const SalFrameState *pState )
{
    if (pState == nullptr)
        return;

    // Request for position or size change
    if (pState->mnMask & _FRAMESTATE_MASK_GEOMETRY)
    {
        long nX = maGeometry.nX;
        long nY = maGeometry.nY;
        long nWidth = maGeometry.nWidth;
        long nHeight = maGeometry.nHeight;

        // change requested properties
        if (pState->mnMask & WINDOWSTATE_MASK_X)
            nX = pState->mnX;
        if (pState->mnMask & WINDOWSTATE_MASK_Y)
            nY = pState->mnY;
        if (pState->mnMask & WINDOWSTATE_MASK_WIDTH)
            nWidth = pState->mnWidth;
        if (pState->mnMask & WINDOWSTATE_MASK_HEIGHT)
            nHeight = pState->mnHeight;

        SetPosSize( nX, nY, nWidth, nHeight,
                    SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y |
                    SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
    }
}

bool SvpSalFrame::GetWindowState( SalFrameState* pState )
{
    pState->mnState = WINDOWSTATE_STATE_NORMAL;
    pState->mnX      = maGeometry.nX;
    pState->mnY      = maGeometry.nY;
    pState->mnWidth  = maGeometry.nWidth;
    pState->mnHeight = maGeometry.nHeight;
    pState->mnMask   = _FRAMESTATE_MASK_GEOMETRY | WINDOWSTATE_MASK_STATE;

    return true;
}

void SvpSalFrame::ShowFullScreen( bool, sal_Int32 )
{
    SetPosSize( 0, 0, VIRTUAL_DESKTOP_WIDTH, VIRTUAL_DESKTOP_HEIGHT,
                SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
}

void SvpSalFrame::StartPresentation( bool )
{
}

void SvpSalFrame::SetAlwaysOnTop( bool )
{
}

void SvpSalFrame::ToTop( sal_uInt16 )
{
    GetFocus();
}

void SvpSalFrame::SetPointer( PointerStyle )
{
}

void SvpSalFrame::CaptureMouse( bool )
{
}

void SvpSalFrame::SetPointerPos( long, long )
{
}

void SvpSalFrame::Flush()
{
}

void SvpSalFrame::SetInputContext( SalInputContext* )
{
}

void SvpSalFrame::EndExtTextInput( EndExtTextInputFlags )
{
}

OUString SvpSalFrame::GetKeyName( sal_uInt16 )
{
    return OUString();
}

bool SvpSalFrame::MapUnicodeToKeyCode( sal_Unicode, LanguageType, vcl::KeyCode& )
{
    return false;
}

LanguageType SvpSalFrame::GetInputLanguage()
{
    return LANGUAGE_DONTKNOW;
}

void SvpSalFrame::UpdateSettings( AllSettings& )
{
}

void SvpSalFrame::Beep()
{
}

const SystemEnvData* SvpSalFrame::GetSystemData() const
{
    return &m_aSystemChildData;
}

SalFrame::SalPointerState SvpSalFrame::GetPointerState()
{
    SalPointerState aState;
    aState.mnState = 0;
    return aState;
}

KeyIndicatorState SvpSalFrame::GetIndicatorState()
{
    return KeyIndicatorState::NONE;
}

void SvpSalFrame::SimulateKeyPress( sal_uInt16 /*nKeyCode*/ )
{
}

void SvpSalFrame::SetParent( SalFrame* pNewParent )
{
    if( m_pParent )
        m_pParent->m_aChildren.remove( this );
    m_pParent = static_cast<SvpSalFrame*>(pNewParent);
}

bool SvpSalFrame::SetPluginParent( SystemParentData* )
{
    return true;
}

void SvpSalFrame::ResetClipRegion()
{
}

void SvpSalFrame::BeginSetClipRegion( sal_uLong )
{
}

void SvpSalFrame::UnionClipRegion( long, long, long, long )
{
}

void SvpSalFrame::EndSetClipRegion()
{
}

SalFrame* GetSvpFocusFrameForLibreOfficeKit()
{
    return SvpSalFrame::GetFocusFrame();
}

vcl::Window* GetSalFrameWindowForLibreOfficeKit(SalFrame *pSF)
{
    return pSF->GetWindow();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
