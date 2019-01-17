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
#include <QtCore/QDebug>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtX11Extras/QX11Info>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

#undef Region

#include "KDE5SalFrame.hxx"
#include "KDE5SalGraphics.hxx"

#include <tools/color.hxx>

#include <vcl/font.hxx>
#include <vcl/settings.hxx>
#include <sal/log.hxx>

#include <unx/fontmanager.hxx>

#include <svdata.hxx>

#include <boost/optional.hpp>

KDE5SalFrame::KDE5SalFrame(KDE5SalFrame* pParent, SalFrameStyleFlags nState, bool bUseCairo)
    : Qt5Frame(pParent, nState, bUseCairo)
    , m_bGraphicsInUse(false)
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

    // set italic
    aInfo.m_eItalic = (qFontInfo.italic() ? ITALIC_NORMAL : ITALIC_NONE);

    // set weight
    int nWeight = qFontInfo.weight();
    if (nWeight <= QFont::Light)
        aInfo.m_eWeight = WEIGHT_LIGHT;
    else if (nWeight <= QFont::Normal)
        aInfo.m_eWeight = WEIGHT_NORMAL;
    else if (nWeight <= QFont::DemiBold)
        aInfo.m_eWeight = WEIGHT_SEMIBOLD;
    else if (nWeight <= QFont::Bold)
        aInfo.m_eWeight = WEIGHT_BOLD;
    else
        aInfo.m_eWeight = WEIGHT_ULTRABOLD;

    // set width
    int nStretch = rQFont.stretch();
    if (nStretch == 0) // QFont::AnyStretch since Qt 5.8
        aInfo.m_eWidth = WIDTH_DONTKNOW;
    else if (nStretch <= QFont::UltraCondensed)
        aInfo.m_eWidth = WIDTH_ULTRA_CONDENSED;
    else if (nStretch <= QFont::ExtraCondensed)
        aInfo.m_eWidth = WIDTH_EXTRA_CONDENSED;
    else if (nStretch <= QFont::Condensed)
        aInfo.m_eWidth = WIDTH_CONDENSED;
    else if (nStretch <= QFont::SemiCondensed)
        aInfo.m_eWidth = WIDTH_SEMI_CONDENSED;
    else if (nStretch <= QFont::Unstretched)
        aInfo.m_eWidth = WIDTH_NORMAL;
    else if (nStretch <= QFont::SemiExpanded)
        aInfo.m_eWidth = WIDTH_SEMI_EXPANDED;
    else if (nStretch <= QFont::Expanded)
        aInfo.m_eWidth = WIDTH_EXPANDED;
    else if (nStretch <= QFont::ExtraExpanded)
        aInfo.m_eWidth = WIDTH_EXTRA_EXPANDED;
    else
        aInfo.m_eWidth = WIDTH_ULTRA_EXPANDED;

    SAL_INFO("vcl.kde5", "font name BEFORE system match: \"" << aInfo.m_aFamilyName << "\"");

    // match font to e.g. resolve "Sans"
    psp::PrintFontManager::get().matchFont(aInfo, rLocale);

    SAL_INFO("vcl.kde5", "font match " << (aInfo.m_nID != 0 ? "succeeded" : "failed")
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
void KDE5SalFrame::UpdateSettings(AllSettings& rSettings)
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
    std::unique_ptr<QMenuBar> pMenuBar = std::unique_ptr<QMenuBar>(new QMenuBar());
    aFont = toFont(pMenuBar->font(), rSettings.GetUILanguageTag().getLocale());
    style.SetMenuFont(aFont);

    rSettings.SetStyleSettings(style);
}

SalGraphics* KDE5SalFrame::AcquireGraphics()
{
    if (m_bGraphicsInUse)
        return nullptr;

    m_bGraphicsInUse = true;

    if (!m_pKDE5Graphics.get())
    {
        m_pKDE5Graphics.reset(new KDE5SalGraphics(this));
        Qt5Frame::InitSvpSalGraphics(m_pKDE5Graphics.get());
    }

    return m_pKDE5Graphics.get();
}

void KDE5SalFrame::ReleaseGraphics(SalGraphics* pSalGraph)
{
    (void)pSalGraph;
    assert(pSalGraph == m_pKDE5Graphics.get());
    m_bGraphicsInUse = false;
}

void KDE5SalFrame::StartPresentation(bool bStart)
{
    // disable screensaver for running preso
    boost::optional<unsigned int> aWindow;
    boost::optional<Display*> aDisplay;
    if (QX11Info::isPlatformX11())
    {
        aWindow = QX11Info::appRootWindow();
        aDisplay = QX11Info::display();
    }

    m_ScreenSaverInhibitor.inhibit(bStart, "presentation", QX11Info::isPlatformX11(), aWindow,
                                   aDisplay);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
