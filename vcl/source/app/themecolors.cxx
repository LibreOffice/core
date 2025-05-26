/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/themecolors.hxx>
#include <officecfg/Office/Common.hxx>

ThemeColors ThemeColors::m_aThemeColors;
bool ThemeColors::m_bIsThemeCached = false;

void ThemeColors::SetThemeState(ThemeState eState)
{
    auto pChange(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Appearance::LibreOfficeTheme::set(static_cast<int>(eState), pChange);
    pChange->commit();
}

ThemeState ThemeColors::GetThemeState()
{
    return static_cast<ThemeState>(officecfg::Office::Common::Appearance::LibreOfficeTheme::get());
}

bool ThemeColors::UseOnlyWhiteDocBackground()
{
    return officecfg::Office::Common::Appearance::UseOnlyWhiteDocBackground::get();
}

void ThemeColors::SetUseOnlyWhiteDocBackground(bool bFlag)
{
    auto pChange(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Appearance::UseOnlyWhiteDocBackground::set(bFlag, pChange);
    pChange->commit();
}

bool ThemeColors::UseBmpForAppBack()
{
    return officecfg::Office::Common::Appearance::UseBmpForAppBack::get();
}

OUString ThemeColors::GetAppBackBmpFileName()
{
    return officecfg::Office::Common::Appearance::AppBackBmpFileName::get();
}

OUString ThemeColors::GetAppBackBmpDrawType()
{
    return officecfg::Office::Common::Appearance::AppBackBmpDrawType::get();
}

void ThemeColors::SetAppBackBmpFileName(const OUString& rFileName)
{
    auto pChange(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Appearance::AppBackBmpFileName::set(rFileName, pChange);
    pChange->commit();
}

void ThemeColors::SetAppBackBmpDrawType(const OUString& rDrawType)
{
    auto pChange(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Appearance::AppBackBmpDrawType::set(rDrawType, pChange);
    pChange->commit();
}

void ThemeColors::SetUseBmpForAppBack(bool bUseBmp)
{
    auto pChange(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Appearance::UseBmpForAppBack::set(bUseBmp, pChange);
    pChange->commit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
