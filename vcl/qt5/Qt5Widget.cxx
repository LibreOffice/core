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

#include "Qt5Widget.hxx"
#include <Qt5Widget.moc>

#include "Qt5Frame.hxx"
#include "Qt5Graphics.hxx"
#include "Qt5Tools.hxx"

#include <QtGui/QFocusEvent>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QShowEvent>
#include <QtGui/QWheelEvent>

#include <cairo.h>
#include <headless/svpgdi.hxx>

Qt5Widget::Qt5Widget( Qt5Frame &rFrame, QWidget *parent, Qt::WindowFlags f )
    : QWidget( parent, f )
    , m_pFrame( &rFrame )
{
    create();
    setMouseTracking( true );
    setFocusPolicy( Qt::StrongFocus );
}

Qt5Widget::~Qt5Widget()
{
}

void Qt5Widget::paintEvent( QPaintEvent *pEvent )
{
    QPainter p( this );
    if( m_pFrame->m_bUseCairo )
    {
        cairo_surface_t *pSurface = m_pFrame->m_pSurface.get();
        cairo_surface_flush( pSurface );

        QImage aImage( cairo_image_surface_get_data( pSurface ),
            size().width(), size().height(), Qt5_DefaultFormat32 );
        p.drawImage( pEvent->rect().topLeft(), aImage, pEvent->rect() );
    }
    else
        p.drawImage( pEvent->rect().topLeft(),
                     *m_pFrame->m_pQImage, pEvent->rect() );
}

void Qt5Widget::resizeEvent( QResizeEvent* )
{
    if( m_pFrame->m_bUseCairo )
    {
        int width = size().width();
        int height = size().height();
        cairo_surface_t *pSurface = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, width, height );
        cairo_surface_set_user_data( pSurface, SvpSalGraphics::getDamageKey(),
                                     &m_pFrame->m_aDamageHandler, nullptr );
        m_pFrame->m_pSvpGraphics->setSurface( pSurface, basegfx::B2IVector(width, height) );
        m_pFrame->m_pSurface.reset( pSurface );
    }
    else
    {
        QImage *pImage = new QImage( size(), Qt5_DefaultFormat32 );
        m_pFrame->m_pQt5Graphics->ChangeQImage( pImage );
        m_pFrame->m_pQImage.reset( pImage );
    }

    m_pFrame->CallCallback( SalEvent::Resize, nullptr );
}

void Qt5Widget::handleMouseButtonEvent( QMouseEvent *pEvent, bool bReleased )
{
    SalMouseEvent aEvent;
    switch( pEvent->button() )
    {
    case Qt::LeftButton: aEvent.mnButton = MOUSE_LEFT; break;
    case Qt::MidButton: aEvent.mnButton = MOUSE_MIDDLE; break;
    case Qt::RightButton: aEvent.mnButton = MOUSE_RIGHT; break;
    default: return;
    }

    aEvent.mnTime   = pEvent->timestamp();
    aEvent.mnX      = (long) pEvent->pos().x();
    aEvent.mnY      = (long) pEvent->pos().y();
    aEvent.mnCode   = GetKeyModCode( pEvent->modifiers() ) |
                      GetMouseModCode( pEvent->buttons() );

    SalEvent nEventType;
    if ( bReleased )
        nEventType = SalEvent::MouseButtonUp;
    else
        nEventType = SalEvent::MouseButtonDown;
    m_pFrame->CallCallback( nEventType, &aEvent );
}

void Qt5Widget::mousePressEvent( QMouseEvent *pEvent )
{
    handleMouseButtonEvent( pEvent, false );
}

void Qt5Widget::mouseReleaseEvent( QMouseEvent *pEvent )
{
    handleMouseButtonEvent( pEvent, true );
}

void Qt5Widget::mouseMoveEvent( QMouseEvent *pEvent )
{
    SalMouseEvent aEvent;
    aEvent.mnTime = pEvent->timestamp();
    aEvent.mnX    = pEvent->pos().x();
    aEvent.mnY    = pEvent->pos().y();
    aEvent.mnCode = GetKeyModCode( pEvent->modifiers() ) |
                    GetMouseModCode( pEvent->buttons() );
    aEvent.mnButton = 0;

    m_pFrame->CallCallback( SalEvent::MouseMove, &aEvent );
    pEvent->accept();
}

void Qt5Widget::wheelEvent( QWheelEvent *pEvent )
{
    SalWheelMouseEvent aEvent;

    aEvent.mnTime = pEvent->timestamp();
    aEvent.mnX    = pEvent->pos().x();
    aEvent.mnY    = pEvent->pos().y();
    aEvent.mnCode = GetKeyModCode( pEvent->modifiers() ) |
                    GetMouseModCode( pEvent->buttons() );

    int nDelta = pEvent->angleDelta().x();
    aEvent.mbHorz = true;
    if ( !nDelta )
    {
        nDelta = pEvent->angleDelta().y();
        aEvent.mbHorz = false;
    }
    if ( !nDelta )
        return;
    nDelta /= 8;

    aEvent.mnDelta = nDelta;
    aEvent.mnNotchDelta = nDelta > 0 ? 1 : -1;
    aEvent.mnScrollLines = 3;

    m_pFrame->CallCallback( SalEvent::WheelMouse, &aEvent );
    pEvent->accept();
}

void Qt5Widget::moveEvent( QMoveEvent* )
{
    m_pFrame->CallCallback( SalEvent::Move, nullptr );
}

void Qt5Widget::showEvent( QShowEvent* )
{
    QSize aSize( m_pFrame->m_pQWidget->size() );
    SalPaintEvent aPaintEvt( 0, 0, aSize.width(), aSize.height(), true );
    m_pFrame->CallCallback( SalEvent::Paint, &aPaintEvt );
}

static sal_uInt16 GetKeyCode( int keyval )
{
    sal_uInt16 nCode = 0;
    if( keyval >= Qt::Key_0 && keyval <= Qt::Key_9 )
        nCode = KEY_0 + ( keyval - Qt::Key_0 );
    else if( keyval >= Qt::Key_A && keyval <= Qt::Key_Z )
        nCode = KEY_A + ( keyval - Qt::Key_A );
    else if( keyval >= Qt::Key_F1 && keyval <= Qt::Key_F26 )
        nCode = KEY_F1 + (keyval - Qt::Key_F1);
    else
    {
        switch( keyval )
        {
            case Qt::Key_Down:          nCode = KEY_DOWN;         break;
            case Qt::Key_Up:            nCode = KEY_UP;           break;
            case Qt::Key_Left:          nCode = KEY_LEFT;         break;
            case Qt::Key_Right:         nCode = KEY_RIGHT;        break;
            case Qt::Key_Home:          nCode = KEY_HOME;         break;
            case Qt::Key_End:           nCode = KEY_END;          break;
            case Qt::Key_PageUp:        nCode = KEY_PAGEUP;       break;
            case Qt::Key_PageDown:      nCode = KEY_PAGEDOWN;     break;
            case Qt::Key_Return:        nCode = KEY_RETURN;       break;
            case Qt::Key_Escape:        nCode = KEY_ESCAPE;       break;
            case Qt::Key_Tab:           nCode = KEY_TAB;          break;
            case Qt::Key_Backspace:     nCode = KEY_BACKSPACE;    break;
            case Qt::Key_Space:         nCode = KEY_SPACE;        break;
            case Qt::Key_Insert:        nCode = KEY_INSERT;       break;
            case Qt::Key_Delete:        nCode = KEY_DELETE;       break;
            case Qt::Key_Plus:          nCode = KEY_ADD;          break;
            case Qt::Key_Minus:         nCode = KEY_SUBTRACT;     break;
            case Qt::Key_Asterisk:      nCode = KEY_MULTIPLY;     break;
            case Qt::Key_Slash:         nCode = KEY_DIVIDE;       break;
            case Qt::Key_Period:        nCode = KEY_POINT;        break;
            case Qt::Key_Comma:         nCode = KEY_COMMA;        break;
            case Qt::Key_Less:          nCode = KEY_LESS;         break;
            case Qt::Key_Greater:       nCode = KEY_GREATER;      break;
            case Qt::Key_Equal:         nCode = KEY_EQUAL;        break;
            case Qt::Key_Find:          nCode = KEY_FIND;         break;
            case Qt::Key_Menu:          nCode = KEY_CONTEXTMENU;  break;
            case Qt::Key_Help:          nCode = KEY_HELP;         break;
            case Qt::Key_Undo:          nCode = KEY_UNDO;         break;
            case Qt::Key_Redo:          nCode = KEY_REPEAT;       break;
            case Qt::Key_Cancel:        nCode = KEY_F11;          break;
            case Qt::Key_AsciiTilde:    nCode = KEY_TILDE;        break;
            case Qt::Key_QuoteLeft:     nCode = KEY_QUOTELEFT;    break;
            case Qt::Key_BracketLeft:   nCode = KEY_BRACKETLEFT;  break;
            case Qt::Key_BracketRight:  nCode = KEY_BRACKETRIGHT; break;
            case Qt::Key_Semicolon:     nCode = KEY_SEMICOLON;    break;
            case Qt::Key_Copy:          nCode = KEY_COPY;         break;
            case Qt::Key_Cut:           nCode = KEY_CUT;          break;
            case Qt::Key_Open:          nCode = KEY_OPEN;         break;
            case Qt::Key_Paste:         nCode = KEY_PASTE;        break;
        }
    }

    return nCode;
}

bool Qt5Widget::handleKeyEvent( QKeyEvent *pEvent, bool bDown )
{
    SalKeyEvent aEvent;

    aEvent.mnCharCode   = (pEvent->text().isEmpty() ? 0 : pEvent->text().at( 0 ).unicode());
    aEvent.mnRepeat     = 0;
    aEvent.mnCode       = GetKeyCode( pEvent->key() );
    aEvent.mnCode      |= GetKeyModCode( pEvent->modifiers() );

    bool bStopProcessingKey;
    if ( bDown )
        bStopProcessingKey = m_pFrame->CallCallback( SalEvent::KeyInput, &aEvent );
    else
        bStopProcessingKey = m_pFrame->CallCallback( SalEvent::KeyUp, &aEvent );
    return bStopProcessingKey;
}

void Qt5Widget::keyPressEvent( QKeyEvent *pEvent )
{
    if ( handleKeyEvent( pEvent, true ) )
        pEvent->accept();
}

void Qt5Widget::keyReleaseEvent( QKeyEvent *pEvent )
{
    if ( handleKeyEvent( pEvent, false ) )
        pEvent->accept();
}

void Qt5Widget::focusInEvent( QFocusEvent* )
{
     m_pFrame->CallCallback( SalEvent::GetFocus, nullptr );
}

void Qt5Widget::focusOutEvent( QFocusEvent* )
{
     m_pFrame->CallCallback( SalEvent::LoseFocus, nullptr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
