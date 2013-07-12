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
#include "vcl/syswin.hxx"
#include "headless/svpframe.hxx"
#include "headless/svpinst.hxx"
#include "headless/svpgdi.hxx"

#include <basebmp/bitmapdevice.hxx>
#include <basebmp/scanlineformats.hxx>
#include <basegfx/vector/b2ivector.hxx>

using namespace basebmp;
using namespace basegfx;

SvpSalFrame* SvpSalFrame::s_pFocusFrame = NULL;

namespace {
    /// Decouple SalFrame lifetime from damagetracker lifetime
    struct DamageTracker : public basebmp::IBitmapDeviceDamageTracker
    {
        DamageTracker( SvpSalFrame& rFrame ) : m_rFrame( rFrame ) {}
        virtual ~DamageTracker() {}
        virtual void damaged( const basegfx::B2IBox& rDamageRect ) const
        {
            m_rFrame.damaged( rDamageRect );
        }
        SvpSalFrame& m_rFrame;
    };
}

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
                basebmp::IBitmapDeviceDamageTrackerSharedPtr( new DamageTracker( *this ) ) );
    }
    m_bDamageTracking = bOn;
}

SvpSalFrame::SvpSalFrame( SvpSalInstance* pInstance,
                          SalFrame* pParent,
                          sal_uLong nSalFrameStyle,
                          bool      bTopDown,
                          basebmp::Format nScanlineFormat,
                          SystemParentData* ) :
    m_pInstance( pInstance ),
    m_pParent( static_cast<SvpSalFrame*>(pParent) ),
    m_nStyle( nSalFrameStyle ),
    m_bVisible( false ),
    m_bDamageTracking( false ),
    m_bTopDown( bTopDown ),
    m_nScanlineFormat( nScanlineFormat ),
    m_nMinWidth( 0 ),
    m_nMinHeight( 0 ),
    m_nMaxWidth( 0 ),
    m_nMaxHeight( 0 )
{
    // fast and easy cross-platform wiping of the data
    memset( (void *)&m_aSystemChildData, 0, sizeof( SystemChildData ) );
    m_aSystemChildData.nSize        = sizeof( SystemChildData );
#ifdef IOS
    // Nothing
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
        s_pFocusFrame = NULL;
        // call directly here, else an event for a destroyed frame would be dispatched
        CallCallback( SALEVENT_LOSEFOCUS, NULL );
        // if the handler has not set a new focus frame
        // pass focus to another frame, preferably a document style window
        if( s_pFocusFrame == NULL )
        {
            const std::list< SalFrame* >& rFrames( m_pInstance->getFrames() );
            for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
            {
                SvpSalFrame* pFrame = const_cast<SvpSalFrame*>(static_cast<const SvpSalFrame*>(*it));
                if( pFrame->m_bVisible        &&
                    pFrame->m_pParent == NULL &&
                    (pFrame->m_nStyle & (SAL_FRAME_STYLE_MOVEABLE |
                                         SAL_FRAME_STYLE_SIZEABLE |
                                         SAL_FRAME_STYLE_CLOSEABLE) ) != 0
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
    if( (m_nStyle & (SAL_FRAME_STYLE_OWNERDRAWDECORATION | SAL_FRAME_STYLE_FLOAT)) == 0 )
    {
        if( s_pFocusFrame )
            s_pFocusFrame->LoseFocus();
        s_pFocusFrame = this;
        m_pInstance->PostEvent( this, NULL, SALEVENT_GETFOCUS );
    }
}

void SvpSalFrame::LoseFocus()
{
    if( s_pFocusFrame == this )
    {
        m_pInstance->PostEvent( this, NULL, SALEVENT_LOSEFOCUS );
        s_pFocusFrame = NULL;
    }
}

SalGraphics* SvpSalFrame::GetGraphics()
{
    SvpSalGraphics* pGraphics = new SvpSalGraphics();
    pGraphics->setDevice( m_aFrame );
    m_aGraphics.push_back( pGraphics );
    return pGraphics;
}

void SvpSalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
    SvpSalGraphics* pSvpGraphics = dynamic_cast<SvpSalGraphics*>(pGraphics);
    m_aGraphics.remove( pSvpGraphics );
    delete pSvpGraphics;
}

sal_Bool SvpSalFrame::PostEvent( void* pData )
{
    m_pInstance->PostEvent( this, pData, SALEVENT_USEREVENT );
    return sal_True;
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

void SvpSalFrame::Show( sal_Bool bVisible, sal_Bool bNoActivate )
{
    if( bVisible && ! m_bVisible )
    {
        m_bVisible = true;
        m_pInstance->PostEvent( this, NULL, SALEVENT_RESIZE );
        if( ! bNoActivate )
            GetFocus();
    }
    else if( ! bVisible && m_bVisible )
    {
        m_bVisible = false;
        m_pInstance->PostEvent( this, NULL, SALEVENT_RESIZE );
        LoseFocus();
    }
}

void SvpSalFrame::Enable( sal_Bool )
{
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
    B2IVector aFrameSize( maGeometry.nWidth, maGeometry.nHeight );
    if( ! m_aFrame.get() || m_aFrame->getSize() != aFrameSize )
    {
        if( aFrameSize.getX() == 0 )
            aFrameSize.setX( 1 );
        if( aFrameSize.getY() == 0 )
            aFrameSize.setY( 1 );
        m_aFrame = createBitmapDevice( aFrameSize, m_bTopDown, m_nScanlineFormat );
        if (m_bDamageTracking)
            m_aFrame->setDamageTracker(
                basebmp::IBitmapDeviceDamageTrackerSharedPtr( new DamageTracker( *this ) ) );
        // update device in existing graphics
        for( std::list< SvpSalGraphics* >::iterator it = m_aGraphics.begin();
             it != m_aGraphics.end(); ++it )
             (*it)->setDevice( m_aFrame );
    }
    if( m_bVisible )
        m_pInstance->PostEvent( this, NULL, SALEVENT_RESIZE );
}

void SvpSalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    if( m_bVisible )
    {
        rWidth = maGeometry.nWidth;
        rHeight = maGeometry.nHeight;
    }
    else
        rWidth = rHeight = 0;
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
    if (pState == NULL)
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

sal_Bool SvpSalFrame::GetWindowState( SalFrameState* pState )
{
    pState->mnState = WINDOWSTATE_STATE_NORMAL;
    pState->mnX      = maGeometry.nX;
    pState->mnY      = maGeometry.nY;
    pState->mnWidth  = maGeometry.nWidth;
    pState->mnHeight = maGeometry.nHeight;
    pState->mnMask   = _FRAMESTATE_MASK_GEOMETRY | WINDOWSTATE_MASK_STATE;

    return sal_True;
}

void SvpSalFrame::ShowFullScreen( sal_Bool, sal_Int32 )
{
    SetPosSize( 0, 0, VIRTUAL_DESKTOP_WIDTH, VIRTUAL_DESKTOP_HEIGHT,
                SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
}

void SvpSalFrame::StartPresentation( sal_Bool )
{
}

void SvpSalFrame::SetAlwaysOnTop( sal_Bool )
{
}

void SvpSalFrame::ToTop( sal_uInt16 )
{
    GetFocus();
}

void SvpSalFrame::SetPointer( PointerStyle )
{
}

void SvpSalFrame::CaptureMouse( sal_Bool )
{
}

void SvpSalFrame::SetPointerPos( long, long )
{
}

void SvpSalFrame::Flush()
{
}

void SvpSalFrame::Sync()
{
}

void SvpSalFrame::SetInputContext( SalInputContext* )
{
}

void SvpSalFrame::EndExtTextInput( sal_uInt16 )
{
}

OUString SvpSalFrame::GetKeyName( sal_uInt16 )
{
    return OUString();
}

sal_Bool SvpSalFrame::MapUnicodeToKeyCode( sal_Unicode, LanguageType, KeyCode& )
{
    return sal_False;
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

SalFrame::SalIndicatorState SvpSalFrame::GetIndicatorState()
{
    SalIndicatorState aState;
    aState.mnState = 0;
    return aState;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
