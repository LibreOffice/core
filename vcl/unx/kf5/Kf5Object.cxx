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

#include "Kf5Object.hxx"

#include "Kf5Frame.hxx"
#include "Kf5Widget.hxx"

#include <QtWidgets/QWidget>

Kf5Object::Kf5Object( Kf5Frame* pParent, bool bShow )
    : m_pParent( pParent )
{
    if ( !m_pParent || !pParent->GetQWidget() )
        return;
    m_pQWidget.reset( new QWidget( pParent->GetQWidget() ) );
    if ( bShow )
        m_pQWidget->show();
}

Kf5Object::~Kf5Object()
{
}

void Kf5Object::ResetClipRegion()
{
    if ( m_pQWidget.get() )
        m_pRegion = QRegion( m_pQWidget->geometry() );
    else
        m_pRegion = QRegion();
}

void Kf5Object::BeginSetClipRegion( sal_uLong )
{
    m_pRegion = QRegion();
}

void Kf5Object::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    m_pRegion += QRect( nX, nY, nWidth, nHeight );
}

void Kf5Object::EndSetClipRegion()
{
    if ( m_pQWidget.get() )
        m_pRegion = m_pRegion.intersected( m_pQWidget->geometry() );
}

void Kf5Object::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{

}

void Kf5Object::Show( bool bVisible )
{
    if( m_pQWidget )
        m_pQWidget->setVisible( bVisible );
}

void Kf5Object::SetForwardKey( bool bEnable )
{
}

const SystemEnvData* Kf5Object::GetSystemData() const
{
    return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
