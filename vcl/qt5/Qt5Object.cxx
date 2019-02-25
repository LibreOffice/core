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

#include <Qt5Object.hxx>
#include <Qt5Object.moc>

#include <Qt5Frame.hxx>

#include <QtWidgets/QWidget>
#include <QtGui/QWindow>

Qt5Object::Qt5Object(Qt5Frame* pParent, bool bShow)
    : m_pParent(pParent)
    , m_pQWidget(nullptr)
    , m_pQWindow(nullptr)
{
    if (!m_pParent || !pParent->GetQWidget())
        return;

    m_pQWindow = new QWindow;
    m_pQWidget = QWidget::createWindowContainer(m_pQWindow, pParent->GetQWidget());

    if (bShow)
        m_pQWidget->show();

    m_aSystemData.nSize = sizeof(SystemEnvData);
    m_aSystemData.aWindow = pParent->GetQWidget()->winId();
    m_aSystemData.aShellWindow = reinterpret_cast<sal_IntPtr>(this);
    //m_aSystemData.pSalFrame = this;
    //m_aSystemData.pWidget = m_pQWidget;
    //m_aSystemData.nScreen = m_nXScreen.getXScreen();
    m_aSystemData.pToolkit = "qt5";
}

void Qt5Object::ResetClipRegion()
{
    if (m_pQWidget)
        m_pRegion = QRegion(m_pQWidget->geometry());
    else
        m_pRegion = QRegion();
}

void Qt5Object::BeginSetClipRegion(sal_uInt32) { m_pRegion = QRegion(); }

void Qt5Object::UnionClipRegion(long nX, long nY, long nWidth, long nHeight)
{
    m_pRegion += QRect(nX, nY, nWidth, nHeight);
}

void Qt5Object::EndSetClipRegion()
{
    if (m_pQWidget)
        m_pRegion = m_pRegion.intersected(m_pQWidget->geometry());
}

void Qt5Object::SetPosSize(long nX, long nY, long nWidth, long nHeight)
{
    if (m_pQWidget)
    {
        m_pQWidget->move(nX, nY);
        m_pQWidget->setFixedSize(nWidth, nHeight);
    }
}

void Qt5Object::Show(bool bVisible)
{
    if (m_pQWidget)
        m_pQWidget->setVisible(bVisible);
}

void Qt5Object::SetForwardKey(bool /*bEnable*/) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
