/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include <tools/gen.hxx>
#include <QtSystem.hxx>
#include <QtTools.hxx>

unsigned int QtSystem::GetDisplayScreenCount() { return QGuiApplication::screens().size(); }

AbsoluteScreenPixelRectangle QtSystem::GetDisplayScreenPosSizePixel(unsigned int nScreen)
{
    QRect qRect = QGuiApplication::screens().at(nScreen)->geometry();
    return AbsoluteScreenPixelRectangle(toRectangle(scaledQRect(qRect, qApp->devicePixelRatio())));
}

int QtSystem::ShowNativeDialog(const OUString&, const OUString&, const std::vector<OUString>&)
{
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
