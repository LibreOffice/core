/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <QtMainWindow.hxx>
#include <QtMainWindow.moc>
#include <QtAccessibleWidget.hxx>

#include <QtGui/QAccessible>
#include <QtGui/QCloseEvent>

QtMainWindow::QtMainWindow(QtFrame& rFrame, Qt::WindowFlags f)
    : QMainWindow(nullptr, f)
    , m_rFrame(rFrame)
{
#ifndef EMSCRIPTEN
    QAccessible::installFactory(QtAccessibleWidget::customFactory);
#endif
}

void QtMainWindow::closeEvent(QCloseEvent* pEvent)
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

void QtMainWindow::moveEvent(QMoveEvent*)
{
    const qreal fRatio = m_rFrame.devicePixelRatioF();
    m_rFrame.maGeometry.setPos(toPoint(pos() * fRatio));
    m_rFrame.CallCallback(SalEvent::Move, nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
