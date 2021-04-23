/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <Qt5MainWindow.hxx>
#include <Qt5MainWindow.moc>
#include <Qt5AccessibleWidget.hxx>

#include <QtGui/QAccessible>
#include <QtGui/QCloseEvent>

Qt5MainWindow::Qt5MainWindow(Qt5Frame& rFrame, Qt::WindowFlags f)
    : QMainWindow(nullptr, f)
    , m_rFrame(rFrame)
{
#ifndef EMSCRIPTEN
    QAccessible::installFactory(Qt5AccessibleWidget::customFactory);
#endif
}

void Qt5MainWindow::closeEvent(QCloseEvent* pEvent)
{
    bool bRet = false;
    bRet = m_rFrame.CallCallback(SalEvent::Close, nullptr);

    if (bRet)
        pEvent->accept();
    // SalEvent::Close returning false may mean that user has vetoed
    // closing the frame ("you have unsaved changes" dialog for example)
    // We shouldn't process the event in such case
    else
        pEvent->ignore();
}

void Qt5MainWindow::moveEvent(QMoveEvent* pEvent)
{
    const qreal fRatio = m_rFrame.devicePixelRatioF();
    m_rFrame.maGeometry.nX = round(pEvent->pos().x() * fRatio);
    m_rFrame.maGeometry.nY = round(pEvent->pos().y() * fRatio);
    m_rFrame.CallCallback(SalEvent::Move, nullptr);
}
