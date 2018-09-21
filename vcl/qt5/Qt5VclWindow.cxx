/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Qt5VclWindow.hxx>
#include <Qt5VclWindow.moc>

#include <Qt5Frame.hxx>
#include <Qt5Widget.hxx>
#include <Qt5Tools.hxx>

#include <sal/log.hxx>

using namespace css::accessibility;

Qt5VclWindow::Qt5VclWindow(vcl::Window* pWindow)
    : m_pWindow(pWindow)
{
    qDebug("Qt5VclWindow::Qt5VclWindow");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
