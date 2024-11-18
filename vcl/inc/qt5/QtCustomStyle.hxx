/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QtGui/QPainter>
#include <QtWidgets/QProxyStyle>
#include <QtWidgets/QStyleOption>

class QtCustomStyle : public QProxyStyle
{
    inline static bool m_bDefaultPaletteLoaded = false;
    inline static bool m_bIsCustomStyleSet = false;

public:
    QtCustomStyle(){};

    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter,
                       const QWidget* widget = nullptr) const override;

    static QPalette customPalette();
    static QPalette GetMenuBarPalette();
    static QPalette GetMenuPalette();
    static bool IsSystemThemeChanged();

    static void LoadCustomStyle(bool bDarkMode);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
