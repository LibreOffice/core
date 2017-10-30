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

#include "Qt5Frame.hxx"

#include "Qt5Tools.hxx"
#include "Qt5Instance.hxx"
#include "Qt5Graphics.hxx"
#include "Qt5Widget.hxx"

#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtGui/QIcon>
#include <QtGui/QWindow>

#include <saldatabasic.hxx>
#include <vcl/syswin.hxx>

Qt5Frame::Qt5Frame( Qt5Frame* pParent, SalFrameStyleFlags nStyle )
    : m_bGraphicsInUse( false )
{
    Qt5Instance *pInst = static_cast<Qt5Instance*>( GetSalData()->m_pInstance );
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

    m_pQWidget.reset( new Qt5Widget( *this, pParent ? pParent->GetQWidget() : nullptr, aWinFlags ) );

    if (pParent && !(pParent->m_nStyle & SalFrameStyleFlags::PLUG))
    {
        QWindow *pParentWindow = pParent->GetQWidget()->window()->windowHandle();
        QWindow *pChildWindow = m_pQWidget->window()->windowHandle();
        if ( pParentWindow != pChildWindow )
            pChildWindow->setTransientParent( pParentWindow );
    }
}

Qt5Frame::~Qt5Frame()
{
    Qt5Instance *pInst = static_cast<Qt5Instance*>( GetSalData()->m_pInstance );
    pInst->eraseFrame( this );
}

void Qt5Frame::TriggerPaintEvent()
{
    QSize aSize( m_pQWidget->size() );
    SalPaintEvent aPaintEvt(0, 0, aSize.width(), aSize.height(), true);
    CallCallback(SalEvent::Paint, &aPaintEvt);
}

SalGraphics* Qt5Frame::AcquireGraphics()
{
    if( m_bGraphicsInUse )
        return nullptr;

    if( !m_pGraphics.get() )
    {
        m_pGraphics.reset( new Qt5Graphics( this ) );
        m_pQImage.reset( new QImage( m_pQWidget->size(), QImage::Format_ARGB32 ) );
        m_pGraphics->ChangeQImage( m_pQImage.get() );
        TriggerPaintEvent();
    }
    m_bGraphicsInUse = true;

    return m_pGraphics.get();
}

void Qt5Frame::ReleaseGraphics( SalGraphics* pSalGraph )
{
    (void) pSalGraph;
    assert( pSalGraph == m_pGraphics.get() );
    m_bGraphicsInUse = false;
}

bool Qt5Frame::PostEvent( ImplSVEvent* pData )
{
    Qt5Instance *pInst = static_cast<Qt5Instance*>( GetSalData()->m_pInstance );
    pInst->PostEvent( this, pData, SalEvent::UserEvent );
    return true;
}

void Qt5Frame::SetTitle( const OUString& rTitle )
{
    m_pQWidget->window()->setWindowTitle( toQString( rTitle ) );
}

void Qt5Frame::SetIcon( sal_uInt16 nIcon )
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

void Qt5Frame::SetMenu( SalMenu* pMenu )
{
}

void Qt5Frame::DrawMenuBar()
{
}

void Qt5Frame::SetExtendedFrameStyle( SalExtStyle nExtStyle )
{
}

void Qt5Frame::Show( bool bVisible, bool bNoActivate )
{
    assert( m_pQWidget.get() );
    m_pQWidget->setVisible( bVisible );
}

void Qt5Frame::SetMinClientSize( long nWidth, long nHeight )
{
    if( ! isChild() )
        m_pQWidget->setMinimumSize( nWidth, nHeight );
}

void Qt5Frame::SetMaxClientSize( long nWidth, long nHeight )
{
    if( ! isChild() )
        m_pQWidget->setMaximumSize( nWidth, nHeight );
}

void Qt5Frame::SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags )
{
}

void Qt5Frame::GetClientSize( long& rWidth, long& rHeight )
{
    rWidth = m_pQWidget->width();
    rHeight = m_pQWidget->height();
}

void Qt5Frame::GetWorkArea( tools::Rectangle& rRect )
{
}

SalFrame* Qt5Frame::GetParent() const
{
    return m_pParent;
}

void Qt5Frame::SetWindowState( const SalFrameState* pState )
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

bool Qt5Frame::GetWindowState( SalFrameState* pState )
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

void Qt5Frame::ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay )
{
}

void Qt5Frame::StartPresentation( bool bStart )
{
}

void Qt5Frame::SetAlwaysOnTop( bool bOnTop )
{
}

void Qt5Frame::ToTop( SalFrameToTop nFlags )
{
}

void Qt5Frame::SetPointer( PointerStyle ePointerStyle )
{
}

void Qt5Frame::CaptureMouse( bool bMouse )
{
}

void Qt5Frame::SetPointerPos( long nX, long nY )
{
}

void Qt5Frame::Flush()
{
}

void Qt5Frame::Flush( const tools::Rectangle& rRect )
{
}

void Qt5Frame::SetInputContext( SalInputContext* pContext )
{
}

void Qt5Frame::EndExtTextInput( EndExtTextInputFlags nFlags )
{
}

OUString Qt5Frame::GetKeyName( sal_uInt16 nKeyCode )
{
    return OUString();
}

bool Qt5Frame::MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode )
{
    return false;
}

LanguageType Qt5Frame::GetInputLanguage()
{
    return LANGUAGE_DONTKNOW;
}

void Qt5Frame::UpdateSettings( AllSettings& rSettings )
{
}

void Qt5Frame::Beep()
{
}

const SystemEnvData* Qt5Frame::GetSystemData() const
{
    return nullptr;
}

SalFrame::SalPointerState Qt5Frame::GetPointerState()
{
    return SalPointerState();
}

KeyIndicatorState Qt5Frame::GetIndicatorState()
{
    return KeyIndicatorState();
}

void Qt5Frame::SimulateKeyPress( sal_uInt16 nKeyCode )
{
}

void Qt5Frame::SetParent( SalFrame* pNewParent )
{
    m_pParent = static_cast< Qt5Frame* >( pNewParent );
}

bool Qt5Frame::SetPluginParent( SystemParentData* pNewParent )
{
    return false;
}

void Qt5Frame::ResetClipRegion()
{
}

void Qt5Frame::BeginSetClipRegion( sal_uLong nRects )
{
}

void Qt5Frame::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
}

void Qt5Frame::EndSetClipRegion()
{
}

void Qt5Frame::SetScreenNumber( unsigned int )
{
}

void Qt5Frame::SetApplicationID(const OUString &)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
