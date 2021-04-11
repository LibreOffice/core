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

#include <memory>
#include <QtGui/QColor>
#include <QtWidgets/QStyle>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

#include <Qt5FontFace.hxx>
#include "KF5SalFrame.hxx"

#include <tools/color.hxx>

#include <vcl/font.hxx>
#include <vcl/settings.hxx>
#include <sal/log.hxx>

#include <unx/fontmanager.hxx>

#include <svdata.hxx>

#include <optional>

KF5SalFrame::KF5SalFrame(KF5SalFrame* pParent, SalFrameStyleFlags nState, bool bUseCairo)
    : Qt5Frame(pParent, nState, bUseCairo)
{
}

/** Helper function to add information to Font from QFont.

    Mostly grabbed from the Gtk+ vclplug (salnativewidgets-gtk.cxx).
*/
static vcl::Font toFont(const QFont& rQFont, const css::lang::Locale& rLocale)
{
    psp::FastPrintFontInfo aInfo;
    QFontInfo qFontInfo(rQFont);

    // set family name
    aInfo.m_aFamilyName = OUString(static_cast<const char*>(rQFont.family().toUtf8()),
                                   strlen(static_cast<const char*>(rQFont.family().toUtf8())),
                                   RTL_TEXTENCODING_UTF8);

    aInfo.m_eItalic = Qt5FontFace::toFontItalic(qFontInfo.style());
    aInfo.m_eWeight = Qt5FontFace::toFontWeight(qFontInfo.weight());
    aInfo.m_eWidth = Qt5FontFace::toFontWidth(rQFont.stretch());

    SAL_INFO("vcl.kf5", "font name BEFORE system match: \"" << aInfo.m_aFamilyName << "\"");

    // match font to e.g. resolve "Sans"
    psp::PrintFontManager::get().matchFont(aInfo, rLocale);

    SAL_INFO("vcl.kf5", "font match " << (aInfo.m_nID != 0 ? "succeeded" : "failed")
                                      << ", name AFTER: \"" << aInfo.m_aFamilyName << "\"");

    // font height
    int nPointHeight = qFontInfo.pointSize();
    if (nPointHeight <= 0)
        nPointHeight = rQFont.pointSize();

    // Create the font
    vcl::Font aFont(aInfo.m_aFamilyName, Size(0, nPointHeight));
    if (aInfo.m_eWeight != WEIGHT_DONTKNOW)
        aFont.SetWeight(aInfo.m_eWeight);
    if (aInfo.m_eWidth != WIDTH_DONTKNOW)
        aFont.SetWidthType(aInfo.m_eWidth);
    if (aInfo.m_eItalic != ITALIC_DONTKNOW)
        aFont.SetItalic(aInfo.m_eItalic);
    if (aInfo.m_ePitch != PITCH_DONTKNOW)
        aFont.SetPitch(aInfo.m_ePitch);

    return aFont;
}

/** Implementation of KDE integration's main method.
*/
void KF5SalFrame::UpdateSettings(AllSettings& rSettings)
{
    Qt5Frame::UpdateSettings(rSettings);

    StyleSettings style(rSettings.GetStyleSettings());
    bool bSetTitleFont = false;

    // WM settings
    /*KConfig *pConfig = KGlobal::config().data();
    if ( pConfig )
    {
        const char *pKey;

        {
            KConfigGroup aWMGroup = pConfig->group( "WM" );

            pKey = "titleFont";
            if (aWMGroup.hasKey(pKey))
            {
                vcl::Font aFont = toFont(aWMGroup.readEntry(pKey, QFont()),
                                         rSettings.GetUILanguageTag().getLocale());
                style.SetTitleFont( aFont );
                bSetTitleFont = true;
            }
        }

        KConfigGroup aIconsGroup = pConfig->group("Icons");

        pKey = "Theme";
        if (aIconsGroup.hasKey(pKey))
            style.SetPreferredIconTheme( readEntryUntranslated(&aIconsGroup, pKey));

        //toolbar
        pKey = "toolbarFont";
        if (aIconsGroup.hasKey(pKey))
        {
            vcl::Font aFont = toFont(aIconsGroup.readEntry(pKey, QFont()),
                                     rSettings.GetUILanguageTag().getLocale());
            style.SetToolFont( aFont );
        }
    }*/

    // Font
    vcl::Font aFont = toFont(QApplication::font(), rSettings.GetUILanguageTag().getLocale());

    style.BatchSetFonts(aFont, aFont);

    aFont.SetWeight(WEIGHT_BOLD);
    if (!bSetTitleFont)
    {
        style.SetTitleFont(aFont);
    }
    style.SetFloatTitleFont(aFont);
    style.SetHelpFont(toFont(QToolTip::font(), rSettings.GetUILanguageTag().getLocale()));

    int flash_time = QApplication::cursorFlashTime();
    style.SetCursorBlinkTime(flash_time != 0 ? flash_time / 2 : STYLE_CURSOR_NOBLINKTIME);

    // Menu
    std::unique_ptr<QMenuBar> pMenuBar = std::make_unique<QMenuBar>();
    aFont = toFont(pMenuBar->font(), rSettings.GetUILanguageTag().getLocale());
    style.SetMenuFont(aFont);

    rSettings.SetStyleSettings(style);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
