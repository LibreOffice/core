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

#include "Kf5Frame.hxx"

#include "Kf5Tools.hxx"
#include "Kf5Instance.hxx"
#include "Kf5Graphics.hxx"
#include "Kf5Widget.hxx"

#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtGui/QIcon>
#include <QtGui/QWindow>

#include <saldatabasic.hxx>
#include <vcl/syswin.hxx>

Kf5Frame::Kf5Frame( Kf5Frame* pParent, SalFrameStyleFlags nStyle )
    : m_bGraphicsInUse( false )
{
    Kf5Instance *pInst = static_cast<Kf5Instance*>( GetSalData()->m_pInstance );
    pInst->insertFrame( this );

    if( nStyle & SalFrameStyleFlags::DEFAULT ) // ensure default style
    {
        nStyle |= SalFrameStyleFlags::MOVEABLE | SalFrameStyleFlags::SIZEABLE | SalFrameStyleFlags::CLOSEABLE;
        nStyle &= ~SalFrameStyleFlags::FLOAT;
    }

    m_nStyle = nStyle;
    m_pParent = pParent;

    Qt::WindowFlags aWinFlags;
    if ( !(nStyle & SalFrameStyleFlags::SYSTEMCHILD) )
    {
        if( nStyle & SalFrameStyleFlags::INTRO )
            aWinFlags |= Qt::SplashScreen;
        else if( nStyle & (SalFrameStyleFlags::FLOAT |
                           SalFrameStyleFlags::TOOLTIP) )
            aWinFlags |= Qt::ToolTip;
        else if( (nStyle & SalFrameStyleFlags::FLOAT) &&
                ! (nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION) )
            aWinFlags |= Qt::Popup;
        else if( nStyle & SalFrameStyleFlags::DIALOG && pParent )
            aWinFlags |= Qt::Dialog;
        else if( nStyle & SalFrameStyleFlags::TOOLWINDOW )
            aWinFlags |= Qt::Tool;
        else if( (nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION) )
            aWinFlags |= Qt::Window | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus;
        else
            aWinFlags |= Qt::Window;
    }

    m_pQWidget.reset( new Kf5Widget( *this, pParent ? pParent->GetQWidget() : nullptr, aWinFlags ) );

    if (pParent && !(pParent->m_nStyle & SalFrameStyleFlags::PLUG))
    {
        QWindow *pParentWindow = pParent->GetQWidget()->window()->windowHandle();
        QWindow *pChildWindow = m_pQWidget->window()->windowHandle();
        if ( pParentWindow != pChildWindow )
            pChildWindow->setTransientParent( pParentWindow );
    }
}

Kf5Frame::~Kf5Frame()
{
    Kf5Instance *pInst = static_cast<Kf5Instance*>( GetSalData()->m_pInstance );
    pInst->eraseFrame( this );
}

void Kf5Frame::TriggerPaintEvent()
{
    QSize aSize( m_pQWidget->size() );
    SalPaintEvent aPaintEvt(0, 0, aSize.width(), aSize.height(), true);
    CallCallback(SalEvent::Paint, &aPaintEvt);
}

SalGraphics* Kf5Frame::AcquireGraphics()
{
    if( m_bGraphicsInUse )
        return nullptr;

    if( !m_pGraphics.get() )
    {
        m_pGraphics.reset( new Kf5Graphics( this ) );
        m_pQImage.reset( new QImage( m_pQWidget->size(), QImage::Format_ARGB32 ) );
        m_pGraphics->ChangeQImage( m_pQImage.get() );
        TriggerPaintEvent();
    }
    m_bGraphicsInUse = true;

    return m_pGraphics.get();
}

void Kf5Frame::ReleaseGraphics( SalGraphics* pSalGraph )
{
    (void) pSalGraph;
    assert( pSalGraph == m_pGraphics.get() );
    m_bGraphicsInUse = false;
}

bool Kf5Frame::PostEvent( ImplSVEvent* pData )
{
    Kf5Instance *pInst = static_cast<Kf5Instance*>( GetSalData()->m_pInstance );
    pInst->PostEvent( this, pData, SalEvent::UserEvent );
    return true;
}

void Kf5Frame::SetTitle( const OUString& rTitle )
{
    m_pQWidget->window()->setWindowTitle( toQString( rTitle ) );
}

void Kf5Frame::SetIcon( sal_uInt16 nIcon )
{
    if( m_nStyle & (SalFrameStyleFlags::PLUG |
                    SalFrameStyleFlags::SYSTEMCHILD |
                    SalFrameStyleFlags::FLOAT |
                    SalFrameStyleFlags::INTRO |
                    SalFrameStyleFlags::OWNERDRAWDECORATION)
            || !m_pQWidget->isWindow() )
        return;

    const char * appicon;

    if (nIcon == SV_ICON_ID_TEXT)
        appicon = "libreoffice-writer";
    else if (nIcon == SV_ICON_ID_SPREADSHEET)
        appicon = "libreoffice-calc";
    else if (nIcon == SV_ICON_ID_DRAWING)
        appicon = "libreoffice-draw";
    else if (nIcon == SV_ICON_ID_PRESENTATION)
        appicon = "libreoffice-impress";
    else if (nIcon == SV_ICON_ID_DATABASE)
        appicon = "libreoffice-base";
    else if (nIcon == SV_ICON_ID_FORMULA)
        appicon = "libreoffice-math";
    else
        appicon = "libreoffice-startcenter";

    QIcon aIcon = QIcon::fromTheme( appicon );
    m_pQWidget->window()->setWindowIcon( aIcon );
}

void Kf5Frame::SetMenu( SalMenu* pMenu )
{
}

void Kf5Frame::DrawMenuBar()
{
}

void Kf5Frame::SetExtendedFrameStyle( SalExtStyle nExtStyle )
{
}

void Kf5Frame::Show( bool bVisible, bool bNoActivate )
{
    assert( m_pQWidget.get() );
    m_pQWidget->setVisible( bVisible );
}

void Kf5Frame::SetMinClientSize( long nWidth, long nHeight )
{
    if( ! isChild() )
        m_pQWidget->setMinimumSize( nWidth, nHeight );
}

void Kf5Frame::SetMaxClientSize( long nWidth, long nHeight )
{
    if( ! isChild() )
        m_pQWidget->setMaximumSize( nWidth, nHeight );
}

void Kf5Frame::SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags )
{
}

void Kf5Frame::GetClientSize( long& rWidth, long& rHeight )
{
    rWidth = m_pQWidget->width();
    rHeight = m_pQWidget->height();
}

void Kf5Frame::GetWorkArea( tools::Rectangle& rRect )
{
}

SalFrame* Kf5Frame::GetParent() const
{
    return m_pParent;
}

void Kf5Frame::SetWindowState( const SalFrameState* pState )
{
    if( !m_pQWidget->isWindow() || !pState || isChild( true, false ) )
        return;

    const WindowStateMask nMaxGeometryMask =
        WindowStateMask::X | WindowStateMask::Y |
        WindowStateMask::Width | WindowStateMask::Height |
        WindowStateMask::MaximizedX | WindowStateMask::MaximizedY |
        WindowStateMask::MaximizedWidth | WindowStateMask::MaximizedHeight;

    if( (pState->mnMask & WindowStateMask::State) &&
            (pState->mnState & WindowStateState::Maximized) &&
            (pState->mnMask & nMaxGeometryMask) == nMaxGeometryMask )
        m_pQWidget->showMaximized();
    else if( pState->mnMask & (WindowStateMask::X | WindowStateMask::Y |
                               WindowStateMask::Width | WindowStateMask::Height ) )
    {
        QRect rect = m_pQWidget->geometry();
        if ( pState->mnMask & WindowStateMask::X )
            rect.setX( pState->mnX );
        if ( pState->mnMask & WindowStateMask::Y )
            rect.setY( pState->mnY );
        if ( pState->mnMask & WindowStateMask::Width )
            rect.setWidth( pState->mnWidth );
        if ( pState->mnMask & WindowStateMask::Height )
            rect.setHeight( pState->mnHeight );
        m_pQWidget->setGeometry( rect );
    }
    else if( pState->mnMask & WindowStateMask::State && ! isChild() )
    {
        if( (pState->mnState & WindowStateState::Minimized) && m_pQWidget->isWindow() )
            m_pQWidget->showMinimized();
        else
            m_pQWidget->showNormal();
    }
}

bool Kf5Frame::GetWindowState( SalFrameState* pState )
{
    pState->mnState = WindowStateState::Normal;
    pState->mnMask  = WindowStateMask::State;
    if( m_pQWidget->isMinimized() || !m_pQWidget->windowHandle() )
        pState->mnState |= WindowStateState::Minimized;
    else if( m_pQWidget->isMaximized() )
    {
        pState->mnState |= WindowStateState::Maximized;
    }
    else
    {
        QRect rect = m_pQWidget->geometry();
        pState->mnX = rect.x();
        pState->mnY = rect.y();
        pState->mnWidth = rect.width();
        pState->mnHeight = rect.height();
        pState->mnMask  |= WindowStateMask::X            |
                           WindowStateMask::Y            |
                           WindowStateMask::Width        |
                           WindowStateMask::Height;
    }

    TriggerPaintEvent();
    return true;
}

void Kf5Frame::ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay )
{
}

void Kf5Frame::StartPresentation( bool bStart )
{
}

void Kf5Frame::SetAlwaysOnTop( bool bOnTop )
{
}

void Kf5Frame::ToTop( SalFrameToTop nFlags )
{
}

void Kf5Frame::SetPointer( PointerStyle ePointerStyle )
{
}

void Kf5Frame::CaptureMouse( bool bMouse )
{
}

void Kf5Frame::SetPointerPos( long nX, long nY )
{
}

void Kf5Frame::Flush()
{
}

void Kf5Frame::Flush( const tools::Rectangle& rRect )
{
}

void Kf5Frame::SetInputContext( SalInputContext* pContext )
{
}

void Kf5Frame::EndExtTextInput( EndExtTextInputFlags nFlags )
{
}

OUString Kf5Frame::GetKeyName( sal_uInt16 nKeyCode )
{
    return OUString();
}

bool Kf5Frame::MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode )
{
    return false;
}

LanguageType Kf5Frame::GetInputLanguage()
{
    return LANGUAGE_DONTKNOW;
}

void Kf5Frame::UpdateSettings( AllSettings& rSettings )
{
}

void Kf5Frame::Beep()
{
}

const SystemEnvData* Kf5Frame::GetSystemData() const
{
    return nullptr;
}

SalFrame::SalPointerState Kf5Frame::GetPointerState()
{
    return SalPointerState();
}

KeyIndicatorState Kf5Frame::GetIndicatorState()
{
    return KeyIndicatorState();
}

void Kf5Frame::SimulateKeyPress( sal_uInt16 nKeyCode )
{
}

void Kf5Frame::SetParent( SalFrame* pNewParent )
{
    m_pParent = static_cast< Kf5Frame* >( pNewParent );
}

bool Kf5Frame::SetPluginParent( SystemParentData* pNewParent )
{
    return false;
}

void Kf5Frame::ResetClipRegion()
{
}

void Kf5Frame::BeginSetClipRegion( sal_uLong nRects )
{
}

void Kf5Frame::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
}

void Kf5Frame::EndSetClipRegion()
{
}

void Kf5Frame::SetScreenNumber( unsigned int )
{
}

void Kf5Frame::SetApplicationID(const OUString &)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
