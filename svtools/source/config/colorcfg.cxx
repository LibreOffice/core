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

#include <sal/config.h>

#include <string_view>

#include <svtools/colorcfg.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/configitem.hxx>
#include <unotools/confignode.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configpaths.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svl/poolitem.hxx>
#include <mutex>
#include <vcl/window.hxx>

#include "itemholder2.hxx"

#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/themecolors.hxx>
#include <officecfg/Office/UI.hxx>
#include <officecfg/Office/Common.hxx>

using namespace utl;
using namespace com::sun::star;

const char g_sIsVisible[] = "/IsVisible";
const char g_sBackgroundType[] = "/BackgroundType";


namespace svtools
{

static sal_Int32            nColorRefCount_Impl = 0;
namespace
{
    std::mutex& ColorMutex_Impl()
    {
        static std::mutex SINGLETON;
        return SINGLETON;
    }
}

ColorConfig_Impl*    ColorConfig::m_pImpl = nullptr;

class ColorConfig_Impl : public utl::ConfigItem
{
    ColorConfigValue m_aConfigValues[ColorConfigEntryCount];
    OUString         m_sLoadedScheme;

    virtual void                    ImplCommit() override;

public:
    explicit ColorConfig_Impl();
    virtual ~ColorConfig_Impl() override;

    void                            Load(const OUString& rScheme);
    void                            CommitCurrentSchemeName();
    //changes the name of the current scheme but doesn't load it!
    void                            SetCurrentSchemeName(const OUString& rSchemeName) {m_sLoadedScheme = rSchemeName;}
    virtual void                    Notify( const uno::Sequence<OUString>& aPropertyNames) override;

    const ColorConfigValue&         GetColorConfigValue(ColorConfigEntry eValue) const
                                                            {return m_aConfigValues[eValue];}
    void                            SetColorConfigValue(ColorConfigEntry eValue,
                                                            const ColorConfigValue& rValue );

    const OUString&            GetLoadedScheme() const {return m_sLoadedScheme;}

    uno::Sequence< OUString> GetSchemeNames();

    void                            AddScheme(const OUString& rNode);
    void                            RemoveScheme(const OUString& rNode);
    using ConfigItem::SetModified;
    using ConfigItem::ClearModified;
    void                            SettingsChanged();

    DECL_LINK( DataChangedEventListener, VclSimpleEvent&, void );
};

namespace {

uno::Sequence< OUString> GetPropertyNames(std::u16string_view rScheme)
{
    // this assumes that all the entries will have at max 3 properties. this
    // might not be the case as more and more UI elements support the bitmap
    // background property. If that happens we might want to increase it to
    // 5 * ColorConfigEntryCount; shouldn't be a problem now
    uno::Sequence<OUString> aNames(3 * ColorConfigEntryCount);
    OUString* pNames = aNames.getArray();
    int nIndex = 0;
    OUString sBase = "ColorSchemes/"
                   + utl::wrapConfigurationElementName(rScheme);
    for(sal_Int32 i = 0; i < ColorConfigEntryCount; ++i)
    {
        // every property has two entries, one for light color and one
        // for dark color. and an optional visibility entry based on
        // cNames[nIndex].bCanBeVisible
        OUString sBaseName = sBase + "/" + cNames[i].cName;
        pNames[nIndex++] = sBaseName + "/Light";
        pNames[nIndex++] = sBaseName + "/Dark";

        if (cNames[i].bCanHaveBitmap)
        {
            pNames[nIndex++] = sBaseName + "/BackgroundType";
            pNames[nIndex++] = sBaseName + "/Bitmap";
        }

        if(cNames[i].bCanBeVisible)
            pNames[nIndex++] = sBaseName + g_sIsVisible;
    }
    aNames.realloc(nIndex);
    return aNames;
}

}

ColorConfig_Impl::ColorConfig_Impl() :
    ConfigItem(u"Office.UI/ColorScheme"_ustr)
{
    //try to register on the root node - if possible
    uno::Sequence < OUString > aNames(1);
    EnableNotification( aNames );

    if (!comphelper::IsFuzzing())
        Load(OUString());

    ::Application::AddEventListener( LINK(this, ColorConfig_Impl, DataChangedEventListener) );

}

ColorConfig_Impl::~ColorConfig_Impl()
{
    ::Application::RemoveEventListener( LINK(this, ColorConfig_Impl, DataChangedEventListener) );
}

void ColorConfig_Impl::Load(const OUString& rScheme)
{
    OUString sScheme(rScheme);
    if(sScheme.isEmpty())
    {
        //detect current scheme name
        uno::Sequence < OUString > aCurrent { u"CurrentColorScheme"_ustr };
        uno::Sequence< uno::Any > aCurrentVal = GetProperties( aCurrent );
        aCurrentVal.getConstArray()[0] >>= sScheme;
    }
    m_sLoadedScheme = sScheme;

    // in cases like theme not found or extension removal, use AUTOMATIC_COLOR_SCHEME as fallback.
    if (!ThemeColors::IsAutomaticTheme(sScheme))
    {
        uno::Sequence<OUString> aSchemes = GetSchemeNames();
        bool bFound = std::any_of(aSchemes.begin(), aSchemes.end(),
            [&sScheme](const OUString& rSchemeName) { return sScheme == rSchemeName; });

        if (!bFound)
            sScheme = AUTOMATIC_COLOR_SCHEME;
    }

    uno::Sequence < OUString > aColorNames = GetPropertyNames(sScheme);
    uno::Sequence< uno::Any > aColors = GetProperties( aColorNames );
    const uno::Any* pColors = aColors.getConstArray();
    const OUString* pColorNames = aColorNames.getConstArray();
    sal_Int32 nIndex = 0;
    for(int i = 0; i < ColorConfigEntryCount && aColors.getLength() > nIndex; ++i)
    {
        // light color value
        Color nTmp;
        pColors[nIndex] >>= nTmp;
        m_aConfigValues[i].nLightColor = nTmp;

        if (!pColors[nIndex].hasValue())
            m_aConfigValues[i].nLightColor = COL_AUTO;
        ++nIndex;

        // dark color value
        pColors[nIndex] >>= nTmp;
        m_aConfigValues[i].nDarkColor = nTmp;

        if (!pColors[nIndex].hasValue())
            m_aConfigValues[i].nDarkColor = COL_AUTO;
        ++nIndex;

        bool bIsDarkMode
            = MiscSettings::GetAppColorMode() == 2
              || (MiscSettings::GetAppColorMode() == 0 && MiscSettings::GetUseDarkMode());

        // based on the appearance (light/dark) cache the value of the appropriate color in nColor.
        // this way we don't have to add hundreds of function calls in the codebase and it will be fast.
        if (bIsDarkMode)
            m_aConfigValues[i].nColor = m_aConfigValues[i].nDarkColor;
        else
            m_aConfigValues[i].nColor = m_aConfigValues[i].nLightColor;

        if(nIndex >= aColors.getLength())
            break;

        m_aConfigValues[i].bUseBitmapBackground = false;

        // for entries that support bitmap background customization
        if (pColorNames[nIndex].endsWith(g_sBackgroundType))
        {
            // use bitmap for background
            bool bUseBitmapBackground = false;
            pColors[nIndex++] >>= bUseBitmapBackground;
            m_aConfigValues[i].bUseBitmapBackground = bUseBitmapBackground;

            OUString aBitmapStr = "";
            pColors[nIndex++] >>= aBitmapStr;

            // stretched or tiled
            bool bIsBitmapStretched = aBitmapStr.endsWith("stretched");
            m_aConfigValues[i].bIsBitmapStretched = bIsBitmapStretched;

            // bitmap file name
            int nNameEnding = aBitmapStr.indexOf(";");
            std::u16string_view aBitmapFileName;
            if (aBitmapStr.isEmpty())
                aBitmapFileName = u"";
            else
                aBitmapFileName = aBitmapStr.subView(0, nNameEnding);

            m_aConfigValues[i].sBitmapFileName = aBitmapFileName;
        }

        // we check if the property ends with "/IsVisible" because not all entries are visible
        // see cNames[nIndex].bCanBeVisible
        if(pColorNames[nIndex].endsWith(g_sIsVisible))
             m_aConfigValues[i].bIsVisible = Any2Bool(pColors[nIndex++]);
    }
}

void ColorConfig_Impl::Notify(const uno::Sequence<OUString>& rProperties)
{
    const OUString sOldLoadedScheme = m_sLoadedScheme;

    ColorConfigValue aOldConfigValues[ColorConfigEntryCount];
    std::copy( m_aConfigValues, m_aConfigValues + ColorConfigEntryCount, aOldConfigValues );

    // loading via notification always uses the default setting
    Load(OUString());

    const bool bNoColorSchemeChange = sOldLoadedScheme == m_sLoadedScheme;

    // If the name of the scheme hasn't changed, then there is no change to the
    // global color scheme name, but Kit deliberately only changed the then
    // current document when it last changed, so there are typically a mixture
    // of documents with the original 'light' color scheme and the last changed
    // color scheme 'dark'. Kit then tries to set the color scheme again to the
    // last changed color scheme 'dark' to try and update a 'light' document
    // that had opted out of the last change to 'dark'...
    const bool bEmptyColorSchemeNotify =
        rProperties.getLength() == 1
        && rProperties[0] == "CurrentColorScheme"
        && bNoColorSchemeChange;

    // ...We can get into a similar situation with inverted backgrounds, for
    // similar reasons, so even if we are only changing the current color scheme
    // we need to make sure that something actually changed...
    bool bNoConfigChange = true;
    for (int i = 0; i < ColorConfigEntryCount; ++i) {
        if (aOldConfigValues[i] != m_aConfigValues[i]) {
            bNoConfigChange = false;
            break;
        }
    }

    // ...and if something from a different color scheme changes, our config
    // values wouldn't change anyway, so we need to make sure that if something
    // changed it was this color scheme...
    const OUString sCurrentSchemePropertyPrefix = "ColorSchemes/org.openoffice.Office.UI:ColorScheme['" + m_sLoadedScheme + "']/";
    bool bOnlyCurrentSchemeChanges = true;
    for (int i = 0; i < rProperties.getLength(); ++i) {
        if (!rProperties[i].startsWith(sCurrentSchemePropertyPrefix)) {
            bOnlyCurrentSchemeChanges = false;
            break;
        }
    }

    bool bEmptyCurrentSchemeNotify = bNoColorSchemeChange && bNoConfigChange && bOnlyCurrentSchemeChanges;

    // ...We can tag apparent null change attempts with
    // 'OnlyCurrentDocumentColorScheme' to allow them to go through, but
    // identify what that change is for, so the other color config listeners for
    // whom it doesn't matter, can ignore it as an optimization.
    const bool bOnlyCurrentDocumentColorScheme = (bEmptyColorSchemeNotify || bEmptyCurrentSchemeNotify) && comphelper::LibreOfficeKit::isActive();
    NotifyListeners(bOnlyCurrentDocumentColorScheme ? ConfigurationHints::OnlyCurrentDocumentColorScheme : ConfigurationHints::NONE);
}

void ColorConfig_Impl::ImplCommit()
{
    uno::Sequence < OUString > aColorNames = GetPropertyNames(m_sLoadedScheme);
    uno::Sequence < beans::PropertyValue > aPropValues(aColorNames.getLength());
    beans::PropertyValue* pPropValues = aPropValues.getArray();
    const OUString* pColorNames = aColorNames.getConstArray();
    sal_Int32 nIndex = 0;
    for(int i = 0; i < ColorConfigEntryCount && nIndex < aColorNames.getLength(); ++i)
    {
        // light color value
        pPropValues[nIndex].Name = pColorNames[nIndex];
        if(m_aConfigValues[i].nLightColor != COL_AUTO) //save automatic colors as void value
            pPropValues[nIndex].Value <<= m_aConfigValues[i].nLightColor;
        nIndex++;

        // dark color value
        pPropValues[nIndex].Name = pColorNames[nIndex];
        if(m_aConfigValues[i].nDarkColor != COL_AUTO) //save automatic colors as void value
            pPropValues[nIndex].Value <<= m_aConfigValues[i].nDarkColor;
        nIndex++;

        if(nIndex >= aColorNames.getLength())
            break;

        if (pColorNames[nIndex].endsWith(g_sBackgroundType))
        {
            pPropValues[nIndex].Name = pColorNames[nIndex];
            pPropValues[nIndex].Value <<= m_aConfigValues[i].bUseBitmapBackground;

            ++nIndex; // Bitmap
            OUString aBitmapStr = m_aConfigValues[i].sBitmapFileName + ";";
            aBitmapStr += m_aConfigValues[i].bIsBitmapStretched ? std::u16string_view(u"stretched")
                                                                : std::u16string_view(u"tiled");

            pPropValues[nIndex].Name = pColorNames[nIndex];
            pPropValues[nIndex].Value <<= aBitmapStr;
            ++nIndex;
        }

        // we check if the property ends with "/IsVisible" because not all entries are visible
        // see cNames[nIndex].bCanBeVisible
        if(pColorNames[nIndex].endsWith(g_sIsVisible))
        {
             pPropValues[nIndex].Name = pColorNames[nIndex];
             pPropValues[nIndex].Value <<= m_aConfigValues[i].bIsVisible;
             nIndex++;
        }
    }
    SetSetProperties(u"ColorSchemes"_ustr, aPropValues);

    CommitCurrentSchemeName();
}

void ColorConfig_Impl::CommitCurrentSchemeName()
{
    //save current scheme name
    uno::Sequence < OUString > aCurrent { u"CurrentColorScheme"_ustr };
    uno::Sequence< uno::Any > aCurrentVal(1);
    aCurrentVal.getArray()[0] <<= m_sLoadedScheme;
    PutProperties(aCurrent, aCurrentVal);
    ThemeColors::GetThemeColors().SetThemeName(m_sLoadedScheme);
}

void ColorConfig_Impl::SetColorConfigValue(ColorConfigEntry eValue, const ColorConfigValue& rValue )
{
    if(rValue != m_aConfigValues[eValue])
    {
        m_aConfigValues[eValue] = rValue;
        SetModified();
    }
}

uno::Sequence< OUString> ColorConfig_Impl::GetSchemeNames()
{
    return GetNodeNames(u"ColorSchemes"_ustr);
}

void ColorConfig_Impl::AddScheme(const OUString& rScheme)
{
    if(ConfigItem::AddNode(u"ColorSchemes"_ustr, rScheme))
    {
        m_sLoadedScheme = rScheme;
        Commit();
    }
}

void ColorConfig_Impl::RemoveScheme(const OUString& rScheme)
{
    uno::Sequence< OUString > aElements { rScheme };
    ClearNodeElements(u"ColorSchemes"_ustr, aElements);
}

void ColorConfig_Impl::SettingsChanged()
{
    SolarMutexGuard aVclGuard;

    NotifyListeners(ConfigurationHints::NONE);
}

IMPL_LINK( ColorConfig_Impl, DataChangedEventListener, VclSimpleEvent&, rEvent, void )
{
    if ( rEvent.GetId() == VclEventId::ApplicationDataChanged )
    {
        DataChangedEvent* pData = static_cast<DataChangedEvent*>(static_cast<VclWindowEvent&>(rEvent).GetData());
        if ( (pData->GetType() == DataChangedEventType::SETTINGS) &&
             (pData->GetFlags() & AllSettingsFlags::STYLE) )
        {
            SettingsChanged();
        }
    }
}

// caches registry colors into the static ThemeColors::m_aThemeColors object. if the color
// value is set to COL_AUTO, the ColorConfig::GetColorValue function calls ColorConfig::GetDefaultColor()
// which returns some hard coded colors for the document, and StyleSettings colors for the UI (lcl_GetDefaultUIColor).
void ColorConfig::LoadThemeColorsFromRegistry()
{
    ThemeColors& rThemeColors = ThemeColors::GetThemeColors();

    rThemeColors.SetWindowColor(GetColorValue(svtools::WINDOWCOLOR).nColor);
    rThemeColors.SetWindowTextColor(GetColorValue(svtools::WINDOWTEXTCOLOR).nColor);
    rThemeColors.SetBaseColor(GetColorValue(svtools::BASECOLOR).nColor);
    rThemeColors.SetButtonColor(GetColorValue(svtools::BUTTONCOLOR).nColor);
    rThemeColors.SetButtonTextColor(GetColorValue(svtools::BUTTONTEXTCOLOR).nColor);
    rThemeColors.SetAccentColor(GetColorValue(svtools::ACCENTCOLOR).nColor);
    rThemeColors.SetDisabledColor(GetColorValue(svtools::DISABLEDCOLOR).nColor);
    rThemeColors.SetDisabledTextColor(GetColorValue(svtools::DISABLEDTEXTCOLOR).nColor);
    rThemeColors.SetShadeColor(GetColorValue(svtools::SHADECOLOR).nColor);
    rThemeColors.SetSeparatorColor(GetColorValue(svtools::SEPARATORCOLOR).nColor);
    rThemeColors.SetFaceColor(GetColorValue(svtools::FACECOLOR).nColor);
    rThemeColors.SetActiveColor(GetColorValue(svtools::ACTIVECOLOR).nColor);
    rThemeColors.SetActiveTextColor(GetColorValue(svtools::ACTIVETEXTCOLOR).nColor);
    rThemeColors.SetActiveBorderColor(GetColorValue(svtools::ACTIVEBORDERCOLOR).nColor);
    rThemeColors.SetFieldColor(GetColorValue(svtools::FIELDCOLOR).nColor);
    rThemeColors.SetMenuBarColor(GetColorValue(svtools::MENUBARCOLOR).nColor);
    rThemeColors.SetMenuBarTextColor(GetColorValue(svtools::MENUBARTEXTCOLOR).nColor);
    rThemeColors.SetMenuBarHighlightColor(GetColorValue(svtools::MENUBARHIGHLIGHTCOLOR).nColor);
    rThemeColors.SetMenuBarHighlightTextColor(
        GetColorValue(svtools::MENUBARHIGHLIGHTTEXTCOLOR).nColor);
    rThemeColors.SetMenuColor(GetColorValue(svtools::MENUCOLOR).nColor);
    rThemeColors.SetMenuTextColor(GetColorValue(svtools::MENUTEXTCOLOR).nColor);
    rThemeColors.SetMenuHighlightColor(GetColorValue(svtools::MENUHIGHLIGHTCOLOR).nColor);
    rThemeColors.SetMenuHighlightTextColor(GetColorValue(svtools::MENUHIGHLIGHTTEXTCOLOR).nColor);
    rThemeColors.SetMenuBorderColor(GetColorValue(svtools::MENUBORDERCOLOR).nColor);
    rThemeColors.SetInactiveColor(GetColorValue(svtools::INACTIVECOLOR).nColor);
    rThemeColors.SetInactiveTextColor(GetColorValue(svtools::INACTIVETEXTCOLOR).nColor);
    rThemeColors.SetInactiveBorderColor(GetColorValue(svtools::INACTIVEBORDERCOLOR).nColor);
    rThemeColors.SetThemeName(GetCurrentSchemeName());

    // as more controls support it, we might want to have ColorConfigValue entries in ThemeColors
    // instead of just colors. for now that seems overkill for just one control.
    rThemeColors.SetAppBackBitmapFileName(
        m_pImpl->GetColorConfigValue(svtools::APPBACKGROUND).sBitmapFileName);
    rThemeColors.SetAppBackUseBitmap(
        m_pImpl->GetColorConfigValue(svtools::APPBACKGROUND).bUseBitmapBackground);
    rThemeColors.SetAppBackBitmapStretched(
        m_pImpl->GetColorConfigValue(svtools::APPBACKGROUND).bIsBitmapStretched);

    ThemeColors::SetThemeCached(true);
}

void ColorConfig::SetupTheme()
{
    if (ThemeColors::IsThemeDisabled())
    {
        ThemeColors::SetThemeCached(false);
        return;
    }

    // When the theme is set to RESET, the IsThemeReset conditional doesn't let the theme to be loaded
    // as explained above, and returns if the StyleSettings doesn't have system colors loaded. IsThemeReset
    // is also used in VclPluginCanUseThemeColors where it prevents the VCL_PLUGINs from using theme colors.
    if (ThemeColors::IsThemeReset())
    {
        if (!Application::GetSettings().GetStyleSettings().GetSystemColorsLoaded())
            return;
        ThemeColors::SetThemeState(ThemeState::ENABLED);
    }

    // When the application is started for the first time, themes is set to ENABLED.
    // that would skip the first two checks for IsThemeDisabled and IsThemeReset in the
    // ColorConfig::SetupTheme function and call LoadThemeColorsFromRegistry();
    if (!ThemeColors::IsThemeCached())
    {
        // registry to ColorConfig::m_pImpl
        m_pImpl->Load(GetCurrentSchemeName());
        m_pImpl->CommitCurrentSchemeName();

        // ColorConfig::m_pImpl to static ThemeColors::m_aThemeColors
        LoadThemeColorsFromRegistry();
    }
}

ColorConfig::ColorConfig()
{
    if (comphelper::IsFuzzing())
        return;
    std::unique_lock aGuard( ColorMutex_Impl() );
    if ( !m_pImpl )
    {
        m_pImpl = new ColorConfig_Impl;
        aGuard.unlock(); // because holdConfigItem will call this constructor
        svtools::ItemHolder2::holdConfigItem(EItem::ColorConfig);
    }
    ++nColorRefCount_Impl;
    m_pImpl->AddListener(this);
    SetupTheme();

    ::Application::AddEventListener( LINK(this, ColorConfig, DataChangedHdl) );
}

ColorConfig::~ColorConfig()
{
    ::Application::RemoveEventListener( LINK(this, ColorConfig, DataChangedHdl) );

    if (comphelper::IsFuzzing())
        return;
    std::unique_lock aGuard( ColorMutex_Impl() );
    m_pImpl->RemoveListener(this);
    if(!--nColorRefCount_Impl)
    {
        delete m_pImpl;
        m_pImpl = nullptr;
    }
}

static Color lcl_GetDefaultUIColor(ColorConfigEntry eEntry)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    switch (eEntry)
    {
        case WINDOWCOLOR:
            return rStyleSettings.GetWindowColor();
        case WINDOWTEXTCOLOR:
            return rStyleSettings.GetWindowTextColor();
        case BASECOLOR:
            return rStyleSettings.GetFieldColor();
        case BUTTONCOLOR:
            return rStyleSettings.GetDialogColor();
        case BUTTONTEXTCOLOR:
            return rStyleSettings.GetButtonTextColor();
        case ACCENTCOLOR:
            return rStyleSettings.GetAccentColor();
        case DISABLEDCOLOR:
            return rStyleSettings.GetDisableColor();
        case DISABLEDTEXTCOLOR:
        case SHADECOLOR:
            return rStyleSettings.GetShadowColor();
        case SEPARATORCOLOR:
            return rStyleSettings.GetSeparatorColor();
        case FACECOLOR:
            return rStyleSettings.GetFaceColor();
        case ACTIVECOLOR:
            return rStyleSettings.GetActiveColor();
        case ACTIVETEXTCOLOR:
            return rStyleSettings.GetActiveTextColor();
        case ACTIVEBORDERCOLOR:
            return rStyleSettings.GetActiveBorderColor();
        case FIELDCOLOR:
            return rStyleSettings.GetFieldColor();
        case MENUBARCOLOR:
            return rStyleSettings.GetMenuBarColor();
        case MENUBARTEXTCOLOR:
            return rStyleSettings.GetMenuBarTextColor();
        case MENUBARHIGHLIGHTCOLOR:
            return rStyleSettings.GetAccentColor();
        case MENUBARHIGHLIGHTTEXTCOLOR:
            return rStyleSettings.GetMenuBarHighlightTextColor();
        case MENUCOLOR:
            return rStyleSettings.GetMenuColor();
        case MENUTEXTCOLOR:
            return rStyleSettings.GetMenuTextColor();
        case MENUHIGHLIGHTCOLOR:
            return rStyleSettings.GetMenuHighlightColor();
        case MENUHIGHLIGHTTEXTCOLOR:
            return rStyleSettings.GetMenuHighlightTextColor();
        case MENUBORDERCOLOR:
            return rStyleSettings.GetMenuBorderColor();
        case INACTIVECOLOR:
        case INACTIVETEXTCOLOR:
        case INACTIVEBORDERCOLOR:
            return rStyleSettings.GetShadowColor();
        default:
            return COL_AUTO;
    }
}

Color ColorConfig::GetDefaultColor(ColorConfigEntry eEntry, int nMod)
{
    // the actual value of default color doesn't matter for colors in Group_Application
    // and this is just to prevent index out of bound error.
    if (eEntry >= WINDOWCOLOR)
        return lcl_GetDefaultUIColor(eEntry);

    enum ColorType { clLight = 0,
                     clDark,
                     nColorTypes };

    static const Color cAutoColors[][nColorTypes] =
    {
        { COL_WHITE,        COL_WHITE       }, // DOCCOLOR
        { COL_LIGHTGRAY,    Color(0x808080) }, // DOCBOUNDARIES
        { Color(0xDFDFDE),  Color(0x333333) }, // APPBACKGROUND
        { COL_LIGHTGRAY,    Color(0x808080) }, // TABLEBOUNDARIES
        { COL_BLACK,        COL_BLACK       }, // FONTCOLOR
        { COL_BLUE,         Color(0x1D99F3) }, // LINKS
        { Color(0x0000cc),  Color(0x9B59B6) }, // LINKSVISITED
        { COL_LIGHTRED,     Color(0xC9211E) }, // SPELL
        { COL_LIGHTBLUE,    Color(0x729FCF) }, // GRAMMAR
        { COL_LIGHTMAGENTA, Color(0x780373) }, // SMARTTAGS
        { COL_GRAY,         Color(0x1C1C1C) }, // SHADOWCOLOR
        { COL_LIGHTGRAY,    Color(0x808080) }, // WRITERTEXTGRID
        { COL_LIGHTGRAY,    COL_LIGHTGRAY   }, // WRITERFIELDSHADING
        { COL_LIGHTGRAY,    Color(0x1C1C1C) }, // WRITERIDXSHADINGS
        { COL_BLACK,        COL_BLACK       }, // WRITERDIRECTCURSOR
        { COL_GREEN,        Color(0x1E6A39) }, // WRITERSCRIPTINDICATOR
        { COL_LIGHTGRAY,    Color(0x808080) }, // WRITERSECTIONBOUNDARIES
        { Color(0x0369a3),  Color(0xB4C7DC) }, // WRITERHEADERFOOTERMARK
        { COL_BLUE,         Color(0x729FCF) }, // WRITERPAGEBREAKS
        { Color(0x268BD2),  Color(0x268BD2) }, // WRITERNONPRINTCHARS
        { COL_LIGHTBLUE,    COL_LIGHTBLUE   }, // HTMLSGML
        { COL_LIGHTGREEN,   COL_LIGHTGREEN  }, // HTMLCOMMENT
        { COL_LIGHTRED,     COL_LIGHTRED    }, // HTMLKEYWORD
        { COL_GRAY,         COL_GRAY        }, // HTMLUNKNOWN
        { COL_GRAY3,        COL_GRAY3       }, // CALCGRID
        { COL_LIGHTBLUE,    COL_LIGHTBLUE   }, // CALCCELLFOCUS
        { COL_BLUE,         COL_BLUE        }, // CALCPAGEBREAK
        { Color(0x2300dc),  Color(0x2300DC) }, // CALCPAGEBREAKMANUAL
        { COL_GRAY7,        COL_GRAY7       }, // CALCPAGEBREAKAUTOMATIC
        { Color(0x2300dc),  Color(0x2300DC) }, // CALCHIDDENCOLROW
        { COL_LIGHTRED,     COL_LIGHTRED    }, // CALCTEXTOVERFLOW
        { Color(0xbf819e),  Color(0xbf819e) }, // CALCCOMMENT
        { COL_LIGHTBLUE,    Color(0x355269) }, // CALCDETECTIVE
        { COL_LIGHTRED,     Color(0xC9211E) }, // CALCDETECTIVEERROR
        { Color(0xef0fff),  Color(0x0D23D5) }, // CALCREFERENCE
        { Color(0xffffc0),  Color(0xE8A202) }, // CALCNOTESBACKGROUND
        { COL_LIGHTBLUE,    Color(0x729FCF) }, // CALCVALUE
        { COL_GREEN,        Color(0x77BC65) }, // CALCFORMULA
        { COL_BLACK,        Color(0xEEEEEE) }, // CALCTEXT
        { COL_LIGHTGRAY,    Color(0x1C1C1C) }, // CALCPROTECTEDBACKGROUND
        { COL_GRAY7,        COL_GRAY7       }, // DRAWGRID
        { Color(0xC69200),  Color(0xffffa6) }, // AUTHOR1
        { Color(0x0646A2),  Color(0xb4c7dc) }, // AUTHOR2
        { Color(0x579D1C),  Color(0xffa6a6) }, // AUTHOR3
        { Color(0x692B9D),  Color(0xafd095) }, // AUTHOR4
        { Color(0xC5000B),  Color(0xffb66c) }, // AUTHOR5
        { Color(0x008080),  Color(0xbf819e) }, // AUTHOR6
        { Color(0x8C8400),  Color(0xd4ea6b) }, // AUTHOR7
        { Color(0x35556B),  Color(0xe8a202) }, // AUTHOR8
        { Color(0xD17600),  Color(0x5983b0) }, // AUTHOR9
        { COL_WHITE,        Color(0x1C1C1C) }, // BASICEDITOR
        { COL_GREEN,        Color(0xDDE8CB) }, // BASICIDENTIFIER
        { COL_GRAY,         Color(0xEEEEEE) }, // BASICCOMMENT
        { COL_LIGHTRED,     Color(0xFFA6A6) }, // BASICNUMBER
        { COL_LIGHTRED,     Color(0xFFA6A6) }, // BASICSTRING
        { COL_BLUE,         Color(0xB4C7DC) }, // BASICOPERATOR
        { COL_BLUE,         Color(0xB4C7DC) }, // BASICKEYWORD
        { COL_RED,          Color(0xFF3838) }, // BASICERROR
        { Color(0x009900),  Color(0x009900) }, // SQLIDENTIFIER
        { COL_BLACK,        COL_BLACK       }, // SQLNUMBER
        { Color(0xCE7B00),  Color(0xCE7B00) }, // SQLSTRING
        { COL_BLACK,        COL_BLACK       }, // SQLOPERATOR
        { Color(0x0000E6),  Color(0x0000E6) }, // SQLKEYWORD
        { Color(0x259D9D),  Color(0x259D9D) }, // SQLPARAMETER
        { COL_GRAY,         COL_GRAY        }, // SQLCOMMENT
    };
    Color aRet;
    switch(eEntry)
    {
        case APPBACKGROUND :
            aRet = Application::GetSettings().GetStyleSettings().GetWorkspaceColor();
            break;

        case LINKS :
            aRet = Application::GetSettings().GetStyleSettings().GetLinkColor();
            break;

        case LINKSVISITED :
            aRet = Application::GetSettings().GetStyleSettings().GetVisitedLinkColor();
            break;

        case CALCCELLFOCUS:
            aRet = Application::GetSettings().GetStyleSettings().GetAccentColor();
            break;

        default:
            int nAppMod;

            if(nMod == 0)
                nAppMod = clLight;
            else if(nMod == 1)
                nAppMod = clDark;
            else
            {
                switch (MiscSettings::GetAppColorMode()) {
                    default:
                        if (MiscSettings::GetUseDarkMode())
                            nAppMod = clDark;
                        else
                            nAppMod = clLight;
                        break;
                    case 1: nAppMod = clLight; break;
                    case 2: nAppMod = clDark; break;
                }
            }
            aRet = cAutoColors[eEntry][nAppMod];
    }
    // fdo#71511: if in a11y HC mode, do pull background color from theme
    if (Application::GetSettings().GetStyleSettings().GetHighContrastMode() && nMod == -1)
    {
        switch(eEntry)
        {
            case DOCCOLOR :
                aRet = Application::GetSettings().GetStyleSettings().GetWindowColor();
                break;
            case FONTCOLOR :
                aRet = Application::GetSettings().GetStyleSettings().GetWindowTextColor();
                break;
            default:
                break;
        }
    }
    return aRet;
}

ColorConfigValue ColorConfig::GetColorValue(ColorConfigEntry eEntry, bool bSmart) const
{
    ColorConfigValue aRet;

    if (m_pImpl)
        aRet = m_pImpl->GetColorConfigValue(eEntry);

    if (bSmart && aRet.nColor == COL_AUTO)
        aRet.nColor = ColorConfig::GetDefaultColor(eEntry);

    return aRet;
}

const OUString& ColorConfig::GetCurrentSchemeName()
{
    uno::Sequence<OUString> aNames = m_pImpl->GetSchemeNames();
    OUString aCurrentSchemeName = officecfg::Office::UI::ColorScheme::CurrentColorScheme::get().value();

    for (const OUString& rSchemeName : aNames)
        if (rSchemeName == aCurrentSchemeName)
            return m_pImpl->GetLoadedScheme();

    // Use "Automatic" as fallback
    auto pChange(comphelper::ConfigurationChanges::create());
    officecfg::Office::UI::ColorScheme::CurrentColorScheme::set(AUTOMATIC_COLOR_SCHEME, pChange);
    pChange->commit();

    m_pImpl->SetCurrentSchemeName(AUTOMATIC_COLOR_SCHEME);
    return m_pImpl->GetLoadedScheme();
}

IMPL_LINK( ColorConfig, DataChangedHdl, VclSimpleEvent&, rEvent, void )
{
    if (rEvent.GetId() == VclEventId::ApplicationDataChanged)
    {
        DataChangedEvent* pData = static_cast<DataChangedEvent*>(static_cast<VclWindowEvent&>(rEvent).GetData());
        if (pData->GetType() == DataChangedEventType::SETTINGS &&
            pData->GetFlags() & AllSettingsFlags::STYLE)
        {
            ThemeColors::SetThemeCached(false);
            SetupTheme();
        }
    }
}

EditableColorConfig::EditableColorConfig() :
    m_pImpl(new ColorConfig_Impl),
    m_bModified(false)
{
    m_pImpl->BlockBroadcasts(true);
}

EditableColorConfig::~EditableColorConfig()
{
    m_pImpl->BlockBroadcasts(false);
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
}

uno::Sequence< OUString >  EditableColorConfig::GetSchemeNames() const
{
    return m_pImpl->GetSchemeNames();
}

void EditableColorConfig::DeleteScheme(const OUString& rScheme )
{
    m_pImpl->RemoveScheme(rScheme);
}

void EditableColorConfig::AddScheme(const OUString& rScheme )
{
    m_pImpl->AddScheme(rScheme);
}

void EditableColorConfig::LoadScheme(const OUString& rScheme )
{
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    m_bModified = false;
    m_pImpl->Load(rScheme);
    //the name of the loaded scheme has to be committed separately
    m_pImpl->CommitCurrentSchemeName();
}

const OUString& EditableColorConfig::GetCurrentSchemeName()const
{
    return m_pImpl->GetLoadedScheme();
}

// Changes the name of the current scheme but doesn't load it!
void EditableColorConfig::SetCurrentSchemeName(const OUString& rScheme)
{
    m_pImpl->SetCurrentSchemeName(rScheme);
    m_pImpl->CommitCurrentSchemeName();
}

const ColorConfigValue& EditableColorConfig::GetColorValue(
    ColorConfigEntry eEntry)const
{
    return m_pImpl->GetColorConfigValue(eEntry);
}

void EditableColorConfig::SetColorValue(
    ColorConfigEntry eEntry, const ColorConfigValue& rValue)
{
    m_pImpl->SetColorConfigValue(eEntry, rValue);
    m_pImpl->ClearModified();
    m_bModified = true;
}

void EditableColorConfig::SetModified()
{
    m_bModified = true;
}

void EditableColorConfig::Commit()
{
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    m_bModified = false;
}

void EditableColorConfig::DisableBroadcast()
{
    m_pImpl->BlockBroadcasts(true);
}

void EditableColorConfig::EnableBroadcast()
{
    m_pImpl->BlockBroadcasts(false);
}


}//namespace svtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
