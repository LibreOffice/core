/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <Qt5AccessibleEventListener.hxx>
#include <Qt5AccessibleWidget.hxx>
#include <Qt5MainWindow.hxx>
#include <Qt5MainWindow.moc>

#include <QtGui/QAccessible>
#include <QtGui/QCloseEvent>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>

using namespace css::accessibility;
using namespace css::uno;

Qt5MainWindow::Qt5MainWindow(Qt5Frame& rFrame, QWidget* parent, Qt::WindowFlags f)
    : QMainWindow(parent, f)
    , m_pFrame(&rFrame)
{
}

void Qt5MainWindow::closeEvent(QCloseEvent* pEvent)
{
    bool bRet = false;
    bRet = m_pFrame->CallCallback(SalEvent::Close, nullptr);

    if (bRet)
        pEvent->accept();
    // SalEvent::Close returning false may mean that user has vetoed
    // closing the frame ("you have unsaved changes" dialog for example)
    // We shouldn't process the event in such case
    else
        pEvent->ignore();
}
