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

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>

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
    p.drawImage( pEvent->rect().topLeft(), *m_pFrame->m_pQImage, pEvent->rect() );
}

void Qt5Widget::resizeEvent( QResizeEvent* )
{
    QImage *pImage = new QImage( m_pFrame->m_pQWidget->size(), QImage::Format_ARGB32 );
    m_pFrame->m_pGraphics->ChangeQImage( pImage );
    m_pFrame->m_pQImage.reset( pImage );
    m_pFrame->CallCallback( SalEvent::Resize, nullptr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
