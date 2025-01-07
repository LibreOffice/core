/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <IconThemeSelector.hxx>
#include <QtCustomStyle.hxx>
#include <QtFrame.hxx>
#include <QtWidgets/qdrawutil.h>
#include <QtWidgets/QApplication>
#include <vcl/themecolors.hxx>
#include <vcl/qt/QtUtils.hxx>

void QtCustomStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                                  QPainter* painter, const QWidget* widget) const
{
    if (!ThemeColors::IsThemeCached() || IsSystemThemeChanged())
    {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        return;
    }

    const ThemeColors& aThemeColors = ThemeColors::GetThemeColors();
    switch (element)
    {
        case PE_FrameTabWidget:
        {
            painter->save();
            QBrush aFillBrush(toQColor(aThemeColors.GetWindowColor()));
            QStyleOption aOpt = *option;
            qDrawWinPanel(painter, option->rect, option->palette, false, &aFillBrush);
            painter->restore();
            break;
        }
        case PE_FrameFocusRect:
            break;
        default:
            QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}

QPalette QtCustomStyle::customPalette()
{
    if (!ThemeColors::IsThemeCached())
        return QApplication::palette();

    const ThemeColors& aThemeColors = ThemeColors::GetThemeColors();
    QPalette aPal;

    aPal.setColor(QPalette::Base, toQColor(aThemeColors.GetBaseColor()));
    aPal.setColor(QPalette::Window, toQColor(aThemeColors.GetWindowColor()));

    aPal.setColor(QPalette::WindowText, toQColor(aThemeColors.GetWindowTextColor()));
    aPal.setColor(QPalette::Disabled, QPalette::WindowText,
                  toQColor(aThemeColors.GetSeparatorColor()));
    aPal.setColor(QPalette::Text, toQColor(aThemeColors.GetButtonTextColor()));
    aPal.setColor(QPalette::ButtonText, toQColor(aThemeColors.GetButtonTextColor()));
    aPal.setColor(QPalette::Disabled, QPalette::ButtonText,
                  toQColor(aThemeColors.GetDisabledTextColor()));
    aPal.setColor(QPalette::PlaceholderText, toQColor(aThemeColors.GetWindowTextColor()));
    aPal.setColor(QPalette::Button, toQColor(aThemeColors.GetButtonColor()));
    aPal.setColor(QPalette::Highlight, toQColor(aThemeColors.GetAccentColor()));

    aPal.setColor(QPalette::Dark, toQColor(aThemeColors.GetShadeColor()));
    aPal.setColor(QPalette::Midlight, toQColor(aThemeColors.GetShadeColor()));
    aPal.setColor(QPalette::Light, toQColor(aThemeColors.GetWindowColor()));
    aPal.setColor(QPalette::Shadow, toQColor(aThemeColors.GetWindowColor()));

    return aPal;
}

QPalette QtCustomStyle::GetMenuBarPalette()
{
    if (!ThemeColors::IsThemeCached() || IsSystemThemeChanged())
        return QApplication::palette();

    QPalette aPal;
    const ThemeColors& aThemeColors = ThemeColors::GetThemeColors();

    aPal.setColor(QPalette::Text, toQColor(aThemeColors.GetMenuBarTextColor()));
    aPal.setColor(QPalette::ButtonText, toQColor(aThemeColors.GetMenuBarTextColor()));
    aPal.setColor(QPalette::Window, toQColor(aThemeColors.GetMenuBarColor()));
    aPal.setColor(QPalette::Highlight, toQColor(aThemeColors.GetMenuBarHighlightColor()));
    aPal.setColor(QPalette::HighlightedText, toQColor(aThemeColors.GetMenuBarHighlightTextColor()));

    return aPal;
}

QPalette QtCustomStyle::GetMenuPalette()
{
    if (!ThemeColors::IsThemeCached() || IsSystemThemeChanged())
        return QApplication::palette();

    QPalette aPal;
    const ThemeColors& aThemeColors = ThemeColors::GetThemeColors();

    aPal.setColor(QPalette::Base, toQColor(aThemeColors.GetMenuColor()));
    aPal.setColor(QPalette::Highlight, toQColor(aThemeColors.GetMenuHighlightColor()));
    aPal.setColor(QPalette::HighlightedText, toQColor(aThemeColors.GetMenuHighlightTextColor()));
    aPal.setColor(QPalette::Disabled, QPalette::WindowText,
                  toQColor(aThemeColors.GetDisabledTextColor()));
    aPal.setColor(QPalette::Window, toQColor(aThemeColors.GetMenuColor()));

    aPal.setColor(QPalette::Text, toQColor(aThemeColors.GetMenuTextColor()));
    aPal.setColor(QPalette::Disabled, QPalette::Text,
                  toQColor(aThemeColors.GetDisabledTextColor()));
    aPal.setColor(QPalette::ButtonText, toQColor(aThemeColors.GetMenuTextColor()));
    aPal.setColor(QPalette::WindowText, toQColor(aThemeColors.GetMenuTextColor()));
    aPal.setColor(QPalette::Button, toQColor(aThemeColors.GetButtonColor()));
    aPal.setColor(QPalette::Disabled, QPalette::ButtonText,
                  toQColor(aThemeColors.GetDisabledTextColor()));

    aPal.setColor(QPalette::Dark, toQColor(aThemeColors.GetMenuColor()));
    aPal.setColor(QPalette::Midlight, toQColor(aThemeColors.GetMenuColor()));
    aPal.setColor(QPalette::Light, toQColor(aThemeColors.GetMenuColor()));
    aPal.setColor(QPalette::Shadow, toQColor(aThemeColors.GetMenuColor()));

    return aPal;
}

bool QtCustomStyle::IsSystemThemeChanged()
{
    return QApplication::palette() != QtCustomStyle::customPalette();
}

void QtCustomStyle::LoadCustomStyle(bool bDarkMode)
{
    if (!ThemeColors::VclPluginCanUseThemeColors())
        return;

    // don't set custom palette in case the system theme has been changed.
    if (!(m_bIsCustomStyleSet && IsSystemThemeChanged()))
        QApplication::setPalette(QtCustomStyle::customPalette());

    QIcon::setThemeName(toQString(vcl::IconThemeSelector::GetIconThemeForDesktopEnvironment(
        Application::GetDesktopEnvironment(), bDarkMode)));

    if (m_bIsCustomStyleSet)
        return;

    QApplication::setStyle(new QtCustomStyle);
    m_bIsCustomStyleSet = true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
