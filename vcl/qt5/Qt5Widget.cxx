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

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>

#include <cairo.h>
#include <headless/svpgdi.hxx>

Qt5Widget::Qt5Widget( Qt5Frame &rFrame, QWidget *parent, Qt::WindowFlags f )
    : QWidget( parent, f )
    , m_pFrame( &rFrame )
{
    create();
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
        p.drawImage( QPoint( 0, 0 ), aImage );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
