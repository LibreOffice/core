/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <svtools/colorcfg.hxx>

class VCL_DLLPUBLIC ThemeColors
{
    ThemeColors() {}
    static bool m_bIsThemeLoaded;
    static ThemeColors m_aThemeColors;

public:
    static ThemeColors& GetThemeColors() { return m_aThemeColors; }
    static void SetThemeColors(const ThemeColors& rThemeColors) { m_aThemeColors = rThemeColors; }

    static bool IsThemeLoaded() { return m_bIsThemeLoaded; }
    static void SetThemeLoaded(bool bLoaded) { m_bIsThemeLoaded = bLoaded; }
    static bool IsAutomaticTheme(std::u16string_view rThemeName)
    {
        return rThemeName == svtools::AUTOMATIC_COLOR_SCHEME;
    }

    static bool VclPluginCanUseThemeColors()
    {
        return IsThemeLoaded()
               && !ThemeColors::IsAutomaticTheme(ThemeColors::GetThemeColors().GetThemeName());
    };

    void SetWindowColor(const Color& rColor) { m_aWindowColor = rColor; }
    void SetWindowTextColor(const Color& rColor) { m_aWindowTextColor = rColor; }
    void SetBaseColor(const Color& rColor) { m_aBaseColor = rColor; }
    void SetButtonColor(const Color& rColor) { m_aButtonColor = rColor; }
    void SetButtonTextColor(const Color& rColor) { m_aButtonTextColor = rColor; }
    void SetAccentColor(const Color& rColor) { m_aAccentColor = rColor; }
    void SetDisabledColor(const Color& rColor) { m_aDisabledColor = rColor; }
    void SetDisabledTextColor(const Color& rColor) { m_aDisabledTextColor = rColor; }
    void SetShadeColor(const Color& rColor) { m_aShadeColor = rColor; }
    void SetSeparatorColor(const Color& rColor) { m_aSeparatorColor = rColor; }
    void SetFaceColor(const Color& rColor) { m_aFaceColor = rColor; }
    void SetActiveColor(const Color& rColor) { m_aActiveColor = rColor; }
    void SetActiveTextColor(const Color& rColor) { m_aActiveTextColor = rColor; }
    void SetActiveBorderColor(const Color& rColor) { m_aActiveBorderColor = rColor; }
    void SetFieldColor(const Color& rColor) { m_aFieldColor = rColor; }
    void SetMenuBarColor(const Color& rColor) { m_aMenuBarColor = rColor; }
    void SetMenuBarTextColor(const Color& rColor) { m_aMenuBarTextColor = rColor; }
    void SetMenuBarHighlightColor(const Color& rColor) { m_aMenuBarHighlightColor = rColor; }
    void SetMenuBarHighlightTextColor(const Color& rColor)
    {
        m_aMenuBarHighlightTextColor = rColor;
    }
    void SetMenuColor(const Color& rColor) { m_aMenuColor = rColor; }
    void SetMenuTextColor(const Color& rColor) { m_aMenuTextColor = rColor; }
    void SetMenuHighlightColor(const Color& rColor) { m_aMenuHighlightColor = rColor; }
    void SetMenuHighlightTextColor(const Color& rColor) { m_aMenuhighlightTextColor = rColor; }
    void SetMenuBorderColor(const Color& rColor) { m_aMenuBorderColor = rColor; }
    void SetInactiveColor(const Color& rColor) { m_aInactiveColor = rColor; }
    void SetInactiveTextColor(const Color& rColor) { m_aInactiveTextColor = rColor; }
    void SetInactiveBorderColor(const Color& rColor) { m_aInactiveBorderColor = rColor; }
    void SetThemeName(const OUString& rName) { m_sThemeName = rName; }

    void SetAppBackBitmapFileName(const OUString& rBitmapStr)
    {
        m_aAppBackBitmapFileName = rBitmapStr;
    }
    void SetAppBackBitmapStretched(bool bStretched) { m_bStretched = bStretched; }
    void SetAppBackUseBitmap(bool bDrawBitmap) { m_bUseBitmapForAppBack = bDrawBitmap; }

    const Color& GetWindowColor() const { return m_aWindowColor; }
    const Color& GetWindowTextColor() const { return m_aWindowTextColor; }
    const Color& GetBaseColor() const { return m_aBaseColor; }
    const Color& GetButtonColor() const { return m_aButtonColor; }
    const Color& GetButtonTextColor() const { return m_aButtonTextColor; }
    const Color& GetAccentColor() const { return m_aAccentColor; }
    const Color& GetDisabledColor() const { return m_aDisabledColor; }
    const Color& GetDisabledTextColor() const { return m_aDisabledTextColor; }
    const Color& GetShadeColor() const { return m_aShadeColor; }
    const Color& GetSeparatorColor() const { return m_aSeparatorColor; }
    const Color& GetFaceColor() const { return m_aFaceColor; }
    const Color& GetActiveColor() const { return m_aActiveColor; }
    const Color& GetActiveTextColor() const { return m_aActiveTextColor; }
    const Color& GetActiveBorderColor() const { return m_aActiveBorderColor; }
    const Color& GetFieldColor() const { return m_aFieldColor; }
    const Color& GetMenuBarColor() const { return m_aMenuBarColor; }
    const Color& GetMenuBarTextColor() const { return m_aMenuBarTextColor; }
    const Color& GetMenuBarHighlightColor() const { return m_aMenuBarHighlightColor; }
    const Color& GetMenuBarHighlightTextColor() const { return m_aMenuBarHighlightTextColor; }
    const Color& GetMenuColor() const { return m_aMenuColor; }
    const Color& GetMenuTextColor() const { return m_aMenuTextColor; }
    const Color& GetMenuHighlightColor() const { return m_aMenuHighlightColor; }
    const Color& GetMenuHighlightTextColor() const { return m_aMenuhighlightTextColor; }
    const Color& GetMenuBorderColor() const { return m_aMenuBorderColor; }
    const Color& GetInactiveColor() const { return m_aInactiveColor; }
    const Color& GetInactiveTextColor() const { return m_aInactiveTextColor; }
    const Color& GetInactiveBorderColor() const { return m_aInactiveBorderColor; }
    const OUString& GetThemeName() const { return m_sThemeName; }

    const OUString& GetAppBackBitmapFileName() const { return m_aAppBackBitmapFileName; }
    bool IsAppBackBitmapStretched() { return m_bStretched; }
    bool GetAppBackUseBitmap() { return m_bUseBitmapForAppBack; }

private:
    Color m_aWindowColor;
    Color m_aWindowTextColor;
    Color m_aBaseColor;
    Color m_aButtonColor;
    Color m_aButtonTextColor;
    Color m_aAccentColor;
    Color m_aDisabledColor;
    Color m_aDisabledTextColor;
    Color m_aShadeColor;
    Color m_aSeparatorColor;
    Color m_aFaceColor;
    Color m_aActiveColor;
    Color m_aActiveTextColor;
    Color m_aActiveBorderColor;
    Color m_aFieldColor;
    Color m_aMenuBarColor;
    Color m_aMenuBarTextColor;
    Color m_aMenuBarHighlightColor;
    Color m_aMenuBarHighlightTextColor;
    Color m_aMenuColor;
    Color m_aMenuTextColor;
    Color m_aMenuHighlightColor;
    Color m_aMenuhighlightTextColor;
    Color m_aMenuBorderColor;
    Color m_aInactiveColor;
    Color m_aInactiveTextColor;
    Color m_aInactiveBorderColor;

    OUString m_sThemeName;

    // using values for now as only one element is bitmap customizable. but as the number increases
    // we might want to replace these colors with structs similar to `struct ColorConfigValue`
    // The actual bitmap object is in StyleSettings because we cannot have non primitive objects in
    // static singleton classes as the order of initialization would be undefined.
    bool m_bStretched = false;
    bool m_bUseBitmapForAppBack = false;
    OUString m_aAppBackBitmapFileName = "";
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
