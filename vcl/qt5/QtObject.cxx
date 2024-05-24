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

#include <QtObject.hxx>
#include <QtObject.moc>

#include <QtFrame.hxx>
#include <QtWidget.hxx>

#include <QtGui/QGuiApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QVBoxLayout>

QtObject::QtObject(QtFrame* pParent, bool bShow)
    : m_pParent(pParent)
    , m_pQWidget(nullptr)
    , m_bForwardKey(false)
{
    if (!m_pParent || !pParent->GetQWidget())
        return;

    m_pQWidget = new QtObjectWidget(*this);
    if (bShow)
        m_pQWidget->show();

    QtFrame::FillSystemEnvData(m_aSystemData, reinterpret_cast<sal_IntPtr>(this), m_pQWidget);
}

QtObject::~QtObject()
{
    if (m_pQWidget)
    {
        m_pQWidget->setParent(nullptr);
        delete m_pQWidget;
    }
}

QWindow* QtObject::windowHandle() const
{
    return m_pQWidget ? m_pQWidget->windowHandle() : nullptr;
}

void QtObject::ResetClipRegion()
{
    if (m_pQWidget)
        m_pRegion = QRegion(m_pQWidget->geometry());
    else
        m_pRegion = QRegion();
}

void QtObject::BeginSetClipRegion(sal_uInt32) { m_pRegion = QRegion(); }

void QtObject::UnionClipRegion(tools::Long nX, tools::Long nY, tools::Long nWidth,
                               tools::Long nHeight)
{
    m_pRegion += QRect(nX, nY, nWidth, nHeight);
}

void QtObject::EndSetClipRegion()
{
    if (m_pQWidget)
        m_pRegion = m_pRegion.intersected(m_pQWidget->geometry());
}

void QtObject::SetPosSize(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight)
{
    if (m_pQWidget)
    {
        m_pQWidget->move(nX, nY);
        m_pQWidget->setFixedSize(nWidth, nHeight);
    }
}

void QtObject::Show(bool bVisible)
{
    if (m_pQWidget)
        m_pQWidget->setVisible(bVisible);
}

void QtObject::SetForwardKey(bool bEnable) { m_bForwardKey = bEnable; }

void QtObject::Reparent(SalFrame* pFrame)
{
    QtFrame* pNewParent = static_cast<QtFrame*>(pFrame);
    if (m_pParent == pNewParent)
        return;
    m_pParent = pNewParent;
    m_pQWidget->setParent(m_pParent->GetQWidget());
}

QtObjectWidget::QtObjectWidget(QtObject& rParent)
    : QWidget(rParent.frame()->GetQWidget())
    , m_rParent(rParent)
{
    assert(m_rParent.frame() && m_rParent.frame()->GetQWidget());
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    // set layout, used for video playback, see QtPlayer::createPlayerWindow
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

void QtObjectWidget::focusInEvent(QFocusEvent*)
{
    SolarMutexGuard aGuard;
    m_rParent.CallCallback(SalObjEvent::GetFocus);
}

void QtObjectWidget::focusOutEvent(QFocusEvent*)
{
    SolarMutexGuard aGuard;
    m_rParent.CallCallback(SalObjEvent::LoseFocus);
}

void QtObjectWidget::mousePressEvent(QMouseEvent* pEvent)
{
    SolarMutexGuard aGuard;
    m_rParent.CallCallback(SalObjEvent::ToTop);

    if (m_rParent.forwardKey())
        pEvent->ignore();
}

void QtObjectWidget::mouseReleaseEvent(QMouseEvent* pEvent)
{
    if (m_rParent.forwardKey())
        pEvent->ignore();
}

void QtObjectWidget::keyReleaseEvent(QKeyEvent* pEvent)
{
    if (m_rParent.forwardKey())
        pEvent->ignore();
}

void QtObjectWidget::keyPressEvent(QKeyEvent* pEvent)
{
    if (m_rParent.forwardKey())
        pEvent->ignore();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
