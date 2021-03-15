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
#include <Qt5Widget.hxx>

#include <QtGui/QGuiApplication>

Qt5Object::Qt5Object(Qt5Frame* pParent, bool bShow)
    : m_pParent(pParent)
    , m_pQWidget(nullptr)
    , m_pQWindow(nullptr)
{
    if (!m_pParent || !pParent->GetQWidget())
        return;

    m_pQWindow = new Qt5ObjectWindow(*this);
    m_pQWidget = QWidget::createWindowContainer(m_pQWindow, pParent->GetQWidget());
    m_pQWidget->setAttribute(Qt::WA_NoSystemBackground);
    connect(m_pQWidget, &QObject::destroyed, this, [this]() { m_pQWidget = nullptr; });

    if (bShow)
        m_pQWidget->show();

    m_aSystemData.aShellWindow = reinterpret_cast<sal_IntPtr>(this);
    //m_aSystemData.pSalFrame = this;
    m_aSystemData.pWidget = m_pQWidget;
    //m_aSystemData.nScreen = m_nXScreen.getXScreen();
    m_aSystemData.toolkit = SystemEnvData::Toolkit::Qt5;
    m_aSystemData.platform = SystemEnvData::Platform::Xcb;
    const bool bWayland = QGuiApplication::platformName() == "wayland";
    if (!bWayland)
    {
        m_aSystemData.platform = SystemEnvData::Platform::Xcb;
        m_aSystemData.SetWindowHandle(m_pQWindow->winId()); // ID of the embedded window
    }
    else
    {
        m_aSystemData.platform = SystemEnvData::Platform::Wayland;
        // TODO implement as needed for Wayland,
        // s.a. commit c0d4f3ad3307c which did this for gtk3
        // QPlatformNativeInterface* native = QGuiApplication::platformNativeInterface();
        // m_aSystemData.pDisplay = native->nativeResourceForWindow("display", nullptr);
        // m_aSystemData.aWindow = reinterpret_cast<unsigned long>(
        //     native->nativeResourceForWindow("surface", m_pQWidget->windowHandle()));
    }
}

Qt5Object::~Qt5Object()
{
    if (m_pQWidget)
    {
        m_pQWidget->setParent(nullptr);
        delete m_pQWidget;
    }
}

void Qt5Object::ResetClipRegion()
{
    if (m_pQWidget)
        m_pRegion = QRegion(m_pQWidget->geometry());
    else
        m_pRegion = QRegion();
}

void Qt5Object::BeginSetClipRegion(sal_uInt32) { m_pRegion = QRegion(); }

void Qt5Object::UnionClipRegion(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                tools::Long nHeight)
{
    m_pRegion += QRect(nX, nY, nWidth, nHeight);
}

void Qt5Object::EndSetClipRegion()
{
    if (m_pQWidget)
        m_pRegion = m_pRegion.intersected(m_pQWidget->geometry());
}

void Qt5Object::SetPosSize(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight)
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

Qt5ObjectWindow::Qt5ObjectWindow(Qt5Object& rParent)
    : m_rParent(rParent)
{
    assert(m_rParent.frame() && m_rParent.frame()->GetQWidget());
}

void Qt5ObjectWindow::focusInEvent(QFocusEvent* pEvent)
{
    m_rParent.CallCallback(SalObjEvent::GetFocus);
    QWindow::focusInEvent(pEvent);
}

void Qt5ObjectWindow::focusOutEvent(QFocusEvent* pEvent)
{
    m_rParent.CallCallback(SalObjEvent::LoseFocus);
    QWindow::focusOutEvent(pEvent);
}

void Qt5ObjectWindow::mousePressEvent(QMouseEvent* pEvent)
{
    m_rParent.CallCallback(SalObjEvent::ToTop);
    Qt5Widget::handleMousePressEvent(*m_rParent.frame(), pEvent);
}

void Qt5ObjectWindow::mouseReleaseEvent(QMouseEvent* pEvent)
{
    Qt5Widget::handleMouseReleaseEvent(*m_rParent.frame(), pEvent);
}

bool Qt5ObjectWindow::event(QEvent* pEvent)
{
    return Qt5Widget::handleEvent(*m_rParent.frame(), *m_rParent.widget(), pEvent)
           || QWindow::event(pEvent);
}

void Qt5ObjectWindow::keyReleaseEvent(QKeyEvent* pEvent)
{
    if (!Qt5Widget::handleKeyReleaseEvent(*m_rParent.frame(), *m_rParent.widget(), pEvent))
        QWindow::keyReleaseEvent(pEvent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
