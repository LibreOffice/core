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

#include <comphelper/lok.hxx>
#include <o3tl/safeint.hxx>
#include <vcl/syswin.hxx>
#include <sal/log.hxx>

#include <headless/svpframe.hxx>
#include <headless/svpinst.hxx>
#ifndef IOS
#include <headless/svpgdi.hxx>
#endif

#include <basegfx/vector/b2ivector.hxx>

#ifndef IOS
#include <cairo.h>
#endif

SvpSalFrame* SvpSalFrame::s_pFocusFrame = nullptr;

#ifdef IOS
#define SvpSalGraphics AquaSalGraphics
#endif

SvpSalFrame::SvpSalFrame( SvpSalInstance* pInstance,
                          SalFrame* pParent,
                          SalFrameStyleFlags nSalFrameStyle ) :
    m_pInstance( pInstance ),
    m_pParent( static_cast<SvpSalFrame*>(pParent) ),
    m_nStyle( nSalFrameStyle ),
    m_bVisible( false ),
#ifndef IOS
    m_pSurface( nullptr ),
#endif
    m_nMinWidth( 0 ),
    m_nMinHeight( 0 ),
    m_nMaxWidth( 0 ),
    m_nMaxHeight( 0 )
{
    // SAL_DEBUG("SvpSalFrame::SvpSalFrame: " << this);
#ifdef IOS
    // Nothing
#elif defined ANDROID
    // Nothing
#else
    m_aSystemChildData.pSalFrame    = this;
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

    std::vector<SvpSalFrame*> Children = m_aChildren;
    for( auto& rChild : Children )
        rChild->SetParent( m_pParent );
    if( m_pParent )
        m_pParent->m_aChildren.erase(std::remove(m_pParent->m_aChildren.begin(), m_pParent->m_aChildren.end(), this), m_pParent->m_aChildren.end());

    if( s_pFocusFrame == this )
    {
        // SAL_DEBUG("SvpSalFrame::~SvpSalFrame: losing focus: " << this);
        s_pFocusFrame = nullptr;
        // call directly here, else an event for a destroyed frame would be dispatched
        CallCallback( SalEvent::LoseFocus, nullptr );
        // if the handler has not set a new focus frame
        // pass focus to another frame, preferably a document style window
        if( s_pFocusFrame == nullptr )
        {
            for (auto pSalFrame : m_pInstance->getFrames() )
            {
                SvpSalFrame* pFrame = static_cast<SvpSalFrame*>( pSalFrame );
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
#ifndef IOS
    if (m_pSurface)
        cairo_surface_destroy(m_pSurface);
#endif
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
        m_pInstance->PostEvent( this, nullptr, SalEvent::GetFocus );
    }
}

void SvpSalFrame::LoseFocus()
{
    if( s_pFocusFrame == this )
    {
        // SAL_DEBUG("SvpSalFrame::LoseFocus: " << this);
        m_pInstance->PostEvent( this, nullptr, SalEvent::LoseFocus );
        s_pFocusFrame = nullptr;
    }
}

SalGraphics* SvpSalFrame::AcquireGraphics()
{
    SvpSalGraphics* pGraphics = new SvpSalGraphics();
#ifndef IOS
    pGraphics->setSurface(m_pSurface, basegfx::B2IVector(maGeometry.nWidth, maGeometry.nHeight));
#endif
    m_aGraphics.push_back( pGraphics );
    return pGraphics;
}

void SvpSalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
    SvpSalGraphics* pSvpGraphics = dynamic_cast<SvpSalGraphics*>(pGraphics);
    m_aGraphics.erase(std::remove(m_aGraphics.begin(), m_aGraphics.end(), pSvpGraphics), m_aGraphics.end());
    delete pSvpGraphics;
}

bool SvpSalFrame::PostEvent(std::unique_ptr<ImplSVEvent> pData)
{
    m_pInstance->PostEvent( this, pData.release(), SalEvent::UserEvent );
    return true;
}

void SvpSalFrame::PostPaint() const
{
    if( m_bVisible )
    {
        SalPaintEvent aPEvt(0, 0, maGeometry.nWidth, maGeometry.nHeight);
        aPEvt.mbImmediateUpdate = false;
        CallCallback( SalEvent::Paint, &aPEvt );
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
        m_pInstance->PostEvent( this, nullptr, SalEvent::Resize );
        if( ! bNoActivate )
            GetFocus();
    }
    else if( ! bVisible && m_bVisible )
    {
        // SAL_DEBUG("SvpSalFrame::Show: hiding: " << this);
        m_bVisible = false;
        m_pInstance->PostEvent( this, nullptr, SalEvent::Resize );
        LoseFocus();
    }
    else
    {
        // SAL_DEBUG("SvpSalFrame::Show: nothing: " << this);
    }
}

void SvpSalFrame::SetMinClientSize( tools::Long nWidth, tools::Long nHeight )
{
    m_nMinWidth = nWidth;
    m_nMinHeight = nHeight;
}

void SvpSalFrame::SetMaxClientSize( tools::Long nWidth, tools::Long nHeight )
{
    m_nMaxWidth = nWidth;
    m_nMaxHeight = nHeight;
}

void SvpSalFrame::SetPosSize( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, sal_uInt16 nFlags )
{
    if( (nFlags & SAL_FRAME_POSSIZE_X) != 0 )
        maGeometry.nX = nX;
    if( (nFlags & SAL_FRAME_POSSIZE_Y) != 0 )
        maGeometry.nY = nY;
    if( (nFlags & SAL_FRAME_POSSIZE_WIDTH) != 0 )
    {
        maGeometry.nWidth = nWidth;
        if( m_nMaxWidth > 0 && maGeometry.nWidth > o3tl::make_unsigned(m_nMaxWidth) )
            maGeometry.nWidth = m_nMaxWidth;
        if( m_nMinWidth > 0 && maGeometry.nWidth < o3tl::make_unsigned(m_nMinWidth) )
            maGeometry.nWidth = m_nMinWidth;
    }
    if( (nFlags & SAL_FRAME_POSSIZE_HEIGHT) != 0 )
    {
        maGeometry.nHeight = nHeight;
        if( m_nMaxHeight > 0 && maGeometry.nHeight > o3tl::make_unsigned(m_nMaxHeight) )
            maGeometry.nHeight = m_nMaxHeight;
        if( m_nMinHeight > 0 && maGeometry.nHeight < o3tl::make_unsigned(m_nMinHeight) )
            maGeometry.nHeight = m_nMinHeight;
    }
#ifndef IOS
    basegfx::B2IVector aFrameSize( maGeometry.nWidth, maGeometry.nHeight );
    if (!m_pSurface || cairo_image_surface_get_width(m_pSurface) != aFrameSize.getX() ||
                       cairo_image_surface_get_height(m_pSurface) != aFrameSize.getY() )
    {
        if( aFrameSize.getX() == 0 )
            aFrameSize.setX( 1 );
        if( aFrameSize.getY() == 0 )
            aFrameSize.setY( 1 );

        if (m_pSurface)
            cairo_surface_destroy(m_pSurface);

        // Creating backing surfaces for invisible windows costs a big chunk of RAM.
        if (Application::IsHeadlessModeEnabled())
             aFrameSize = basegfx::B2IVector( 1, 1 );

        m_pSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                aFrameSize.getX(),
                                                aFrameSize.getY());

        // update device in existing graphics
        for (auto const& graphic : m_aGraphics)
        {
             graphic->setSurface(m_pSurface, aFrameSize);
        }
    }
    if( m_bVisible )
        m_pInstance->PostEvent( this, nullptr, SalEvent::Resize );
#endif
}

void SvpSalFrame::GetClientSize( tools::Long& rWidth, tools::Long& rHeight )
{
    rWidth = maGeometry.nWidth;
    rHeight = maGeometry.nHeight;
}

void SvpSalFrame::GetWorkArea( tools::Rectangle& rRect )
{
    rRect = tools::Rectangle( Point( 0, 0 ),
                       Size( VIRTUAL_DESKTOP_WIDTH, VIRTUAL_DESKTOP_HEIGHT ) );
}

SalFrame* SvpSalFrame::GetParent() const
{
    return m_pParent;
}

constexpr auto FRAMESTATE_MASK_GEOMETRY =
     WindowStateMask::X     | WindowStateMask::Y |
     WindowStateMask::Width | WindowStateMask::Height;

void SvpSalFrame::SetWindowState( const SalFrameState *pState )
{
    if (pState == nullptr)
        return;

    // Request for position or size change
    if (!(pState->mnMask & FRAMESTATE_MASK_GEOMETRY))
        return;

    tools::Long nX = maGeometry.nX;
    tools::Long nY = maGeometry.nY;
    tools::Long nWidth = maGeometry.nWidth;
    tools::Long nHeight = maGeometry.nHeight;

    // change requested properties
    if (pState->mnMask & WindowStateMask::X)
        nX = pState->mnX;
    if (pState->mnMask & WindowStateMask::Y)
        nY = pState->mnY;
    if (pState->mnMask & WindowStateMask::Width)
        nWidth = pState->mnWidth;
    if (pState->mnMask & WindowStateMask::Height)
        nHeight = pState->mnHeight;

    SetPosSize( nX, nY, nWidth, nHeight,
                SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y |
                SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
}

bool SvpSalFrame::GetWindowState( SalFrameState* pState )
{
    pState->mnState = WindowStateState::Normal;
    pState->mnX      = maGeometry.nX;
    pState->mnY      = maGeometry.nY;
    pState->mnWidth  = maGeometry.nWidth;
    pState->mnHeight = maGeometry.nHeight;
    pState->mnMask   = FRAMESTATE_MASK_GEOMETRY | WindowStateMask::State;

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

void SvpSalFrame::ToTop( SalFrameToTop )
{
    GetFocus();
}

void SvpSalFrame::SetPointer( PointerStyle )
{
}

void SvpSalFrame::CaptureMouse( bool )
{
}

void SvpSalFrame::SetPointerPos( tools::Long, tools::Long )
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

void SvpSalFrame::UpdateSettings( AllSettings& rSettings )
{
    StyleSettings aStyleSettings = rSettings.GetStyleSettings();

    Color aBackgroundColor( 0xef, 0xef, 0xef );
    aStyleSettings.BatchSetBackgrounds( aBackgroundColor, false );
    aStyleSettings.SetMenuColor( aBackgroundColor );
    aStyleSettings.SetMenuBarColor( aBackgroundColor );

    if (comphelper::LibreOfficeKit::isActive()) // TODO: remove this.
    {
        vcl::Font aStdFont( FAMILY_SWISS, Size( 0, 14 ) );
        aStdFont.SetCharSet( osl_getThreadTextEncoding() );
        aStdFont.SetWeight( WEIGHT_NORMAL );
        aStdFont.SetFamilyName( "Liberation Sans" );
        aStyleSettings.BatchSetFonts( aStdFont, aStdFont );

        aStdFont.SetFontSize(Size(0, 12));
        aStyleSettings.SetMenuFont(aStdFont);

        SvpSalGraphics* pGraphics = m_aGraphics.back();
        bool bFreeGraphics = false;
        if (!pGraphics)
        {
            pGraphics = dynamic_cast<SvpSalGraphics*>(AcquireGraphics());
            if (!pGraphics)
            {
                SAL_WARN("vcl.gtk3", "Could not get graphics - unable to update settings");
                return;
            }
            bFreeGraphics = true;
        }
        rSettings.SetStyleSettings(aStyleSettings);
#ifndef IOS // For now...
        pGraphics->UpdateSettings(rSettings);
#endif
        if (bFreeGraphics)
            ReleaseGraphics(pGraphics);
    }
    else
        rSettings.SetStyleSettings(aStyleSettings);
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
        m_pParent->m_aChildren.erase(std::remove(m_pParent->m_aChildren.begin(), m_pParent->m_aChildren.end(), this), m_pParent->m_aChildren.end());
    m_pParent = static_cast<SvpSalFrame*>(pNewParent);
}

bool SvpSalFrame::SetPluginParent( SystemParentData* )
{
    return true;
}

void SvpSalFrame::ResetClipRegion()
{
}

void SvpSalFrame::BeginSetClipRegion( sal_uInt32 )
{
}

void SvpSalFrame::UnionClipRegion( tools::Long, tools::Long, tools::Long, tools::Long )
{
}

void SvpSalFrame::EndSetClipRegion()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
