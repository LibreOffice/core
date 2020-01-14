/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

#include <string.h>
#include <tools/gen.hxx>
#include <Qt5System.hxx>
#include <Qt5Tools.hxx>

unsigned int Qt5System::GetDisplayScreenCount()
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    return QApplication::desktop()->screenCount();
    SAL_WNODEPRECATED_DECLARATIONS_POP
}

tools::Rectangle Qt5System::GetDisplayScreenPosSizePixel(unsigned int nScreen)
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    QRect qRect = QApplication::desktop()->screenGeometry(nScreen);
    SAL_WNODEPRECATED_DECLARATIONS_POP
    return toRectangle(qRect);
}

int Qt5System::ShowNativeDialog(const OUString&, const OUString&, const std::vector<OUString>&)
{
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
