/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "appearance.hxx"
#include <com/sun/star/uno/Sequence.hxx>
#include <dialmgr.hxx>
#include <helpids.h>
#include <officecfg/Office/Common.hxx>
#include <sfx2/objsh.hxx>
#include <strings.hrc>
#include <svtools/colorcfg.hxx>
#include <svtools/restartdialog.hxx>
#include <svx/itemwin.hxx>
#include <svx/svxids.hrc>
#include <tools/debug.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/themecolors.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertyvalue.hxx>
#include <map>

namespace
{
struct StringPair
{
    OUString aTranslatedString;
    OUString aRawString;
};

std::vector<StringPair> const& getBitmapList()
{
    static const std::vector<StringPair> aBitmapList = {
        { CuiResId(BMP_FUZZY_LIGHTGREY), "fuzzy-lightgrey.jpg" },
        { CuiResId(BMP_ICE_LIGHT), "ice-light.jpg" },
        { CuiResId(BMP_PAINTED_WHITE), "painted-white.jpg" },
        { CuiResId(BMP_TEXTURE_PAPER), "texture-paper.jpg" },
        { CuiResId(BMP_CRUMPLED_PAPER), "crumpled-paper.jpg" },
        { CuiResId(BMP_MARBLE), "marble.jpg" },
        { CuiResId(BMP_CONCRETE), "concrete.jpg" },
        { CuiResId(BMP_FUZZY_GREY), "fuzzy-grey.jpg" },
        { CuiResId(BMP_FUZZY_DARKGREY), "fuzzy-darkgrey.jpg" },
        { CuiResId(BMP_STONE), "stone.jpg" },
        { CuiResId(BMP_WHITE_DIFFUSION), "white-diffusion.jpg" },
        { CuiResId(BMP_SAND_LIGHT), "sand-light.jpg" },
        { CuiResId(BMP_SAND), "sand.jpg" },
        { CuiResId(BMP_SURFACE), "surface.jpg" },
        { CuiResId(BMP_STUDIO), "studio.jpg" },
        { CuiResId(BMP_INVOICE_PAPER), "invoice-paper.jpg" },
        { CuiResId(BMP_PARCHMENT_PAPER), "parchment-paper.jpg" },
        { CuiResId(BMP_CARDBOARD), "cardboard.jpg" },
        { CuiResId(BMP_FENCE), "fence.jpg" },
        { CuiResId(BMP_WOODEN_FENCE), "wooden-fence.jpg" },
        { CuiResId(BMP_WOOD), "wood.jpg" },
        { CuiResId(BMP_WOODEN_BOARD), "wooden-board.jpg" },
        { CuiResId(BMP_PAINTED_WOOD), "painted-wood.jpg" },
        { CuiResId(BMP_STONES), "stones.jpg" },
        { CuiResId(BMP_PEBBLE_LIGHT), "pebble-light.jpg" },
        { CuiResId(BMP_STONE_WALL), "stone-wall.jpg" },
        { CuiResId(BMP_STONE_GRAY), "stone-gray.jpg" },
        { CuiResId(BMP_ROCK_WALL), "rock-wall.jpg" },
        { CuiResId(BMP_SURFACE_BLACK), "surface-black.jpg" },
        { CuiResId(BMP_BRICK_WALL), "brick-wall.png" },
        { CuiResId(BMP_TILES), "tiles.jpg" },
        { CuiResId(BMP_GRAPH_PAPER), "graph-paper.png" },
        { CuiResId(BMP_CLOUD), "cloud.jpg" },
        { CuiResId(BMP_POOL), "pool.jpg" },
        { CuiResId(BMP_SKY), "sky.jpg" },
        { CuiResId(BMP_CIRCUIT_BOARD), "circuit-board.jpg" },
        { CuiResId(BMP_COFFEE), "coffee.jpg" },
        { CuiResId(BMP_COLOR_STRIPES), "color-stripes.png" },
        { CuiResId(BMP_FLORAL), "floral.png" },
        { CuiResId(BMP_LEAF), "leaf.jpg" },
        { CuiResId(BMP_MAPLE_LEAVES), "maple-leaves.jpg" },
        { CuiResId(BMP_SPACE), "space.png" },
        { CuiResId(BMP_GIRAFFE), "giraffe.png" },
        { CuiResId(BMP_TIGER), "tiger.jpg" },
        { CuiResId(BMP_ZEBRA), "zebra.png" },
    };
    return aBitmapList;
}

std::vector<StringPair> const& getBitmapDrawTypeList()
{
    static std::vector<StringPair> aBitmapDrawTypeList = {
        { CuiResId(BMP_DRAWTYPE_TILED), "Tiled" },
        { CuiResId(BMP_DRAWTYPE_STRETCHED), "Stretched" },
    };
    return aBitmapDrawTypeList;
}
}

static bool IsDarkModeEnabled()
{
    return MiscSettings::GetAppColorMode() == AppearanceMode::DARK
           || (MiscSettings::GetAppColorMode() == AppearanceMode::AUTO
               && MiscSettings::GetUseDarkMode());
}

SvxAppearanceTabPage::SvxAppearanceTabPage(weld::Container* pPage,
                                           weld::DialogController* pController,
                                           const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/appearance.ui"_ustr, u"AppearanceTabPage"_ustr, &rSet)
    , m_bRestartRequired(false)
    , pColorConfig(new EditableColorConfig)
    , m_xSchemeList(m_xBuilder->weld_combo_box(u"scheme"_ustr))
    , m_xMoreThemesBtn(m_xBuilder->weld_button(u"morethemesbtn"_ustr))
    , m_xEnableAppTheming(m_xBuilder->weld_check_button(u"enableapptheming"_ustr))
    , m_xUseOnlyWhiteDocBackground(m_xBuilder->weld_check_button(u"useonlywhitedocbackground"_ustr))
    , m_xColorEntryBtn(m_xBuilder->weld_combo_box(u"registrydropdown"_ustr))
    , m_xColorChangeBtn((new ColorListBox(m_xBuilder->weld_menu_button(u"colorsdropdownbtn"_ustr),
                                          [this] { return GetFrameWeld(); })))
    , m_xShowInDocumentChkBtn(m_xBuilder->weld_check_button(u"showindocumentchkbtn"_ustr))
    , m_xResetAllBtn(m_xBuilder->weld_button(u"resetallbtn"_ustr))
    , m_xUseBmpForAppBack(m_xBuilder->weld_check_button(u"usebmpforappback"_ustr))
    , m_xBitmapDropDown(m_xBuilder->weld_combo_box(u"bitmapdropdown"_ustr))
    , m_xBitmapDrawTypeDropDown(m_xBuilder->weld_combo_box(u"bitmapdrawtypedropdown"_ustr))
{
    InitThemes();
    InitCustomization();
}

void SvxAppearanceTabPage::LoadSchemeList()
{
    m_xSchemeList->clear();
    css::uno::Sequence<OUString> aSchemeNames = pColorConfig->GetSchemeNames();

    // insert auto, light, dark themes first
    m_xSchemeList->insert(0, CuiResId(RID_COLOR_SCHEME_LIBREOFFICE_AUTOMATIC),
                          &AUTOMATIC_COLOR_SCHEME, nullptr, nullptr);
    m_xSchemeList->insert(1, CuiResId(RID_COLOR_SCHEME_LIBREOFFICE_LIGHT), &LIGHT_COLOR_SCHEME,
                          nullptr, nullptr);
    m_xSchemeList->insert(2, CuiResId(RID_COLOR_SCHEME_LIBREOFFICE_DARK), &DARK_COLOR_SCHEME,
                          nullptr, nullptr);

    // insert all the custom color schemes
    for (size_t i = 0; i < aSchemeNames.size(); ++i)
        if (ThemeColors::IsCustomTheme(aSchemeNames[i]))
            m_xSchemeList->append_text(aSchemeNames[i]);

    // since this function just loads entries into the list, we don't call
    // MiscSettings::SetAppColorMode(...) here, and instead do so in the
    // SchemeChangeHdl callback.
    if (ThemeColors::IsAutomaticTheme(pColorConfig->GetCurrentSchemeName()))
        m_xSchemeList->set_active_id(AUTOMATIC_COLOR_SCHEME);
    else if (ThemeColors::IsLightTheme(pColorConfig->GetCurrentSchemeName()))
        m_xSchemeList->set_active_id(LIGHT_COLOR_SCHEME);
    else if (ThemeColors::IsDarkTheme(pColorConfig->GetCurrentSchemeName()))
        m_xSchemeList->set_active_id(DARK_COLOR_SCHEME);
    else
        m_xSchemeList->set_active_text(pColorConfig->GetCurrentSchemeName());
}

SvxAppearanceTabPage::~SvxAppearanceTabPage()
{
    if (m_bRestartRequired)
        ::svtools::executeRestartDialog(comphelper::getProcessComponentContext(), GetFrameWeld(),
                                        svtools::RESTART_REASON_THEME_CHANGE);
}

std::unique_ptr<SfxTabPage> SvxAppearanceTabPage::Create(weld::Container* pPage,
                                                         weld::DialogController* pController,
                                                         const SfxItemSet* rSet)
{
    return std::make_unique<SvxAppearanceTabPage>(pPage, pController, *rSet);
}

OUString SvxAppearanceTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"libreofficethemeslb"_ustr, u"optionslb"_ustr, u"appearancelb"_ustr,
                          u"itemslb"_ustr, u"colorlb"_ustr };

    for (const auto& label : labels)
    {
        if (const auto pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SvxAppearanceTabPage::FillItemSet(SfxItemSet* /* rSet */)
{
    // commit ColorConfig
    if (pColorConfig->IsModified())
        pColorConfig->Commit();

    return true;
}

void SvxAppearanceTabPage::Reset(const SfxItemSet* /* rSet */)
{
    // reset scheme list
    LoadSchemeList();

    m_xSchemeList->set_sensitive(
        !officecfg::Office::Common::Appearance::ApplicationAppearance::isReadOnly());
    m_xSchemeList->save_value();

    // reset ColorConfig
    pColorConfig->ClearModified();
    pColorConfig->DisableBroadcast();
    pColorConfig.reset(new EditableColorConfig);
}

IMPL_LINK_NOARG(SvxAppearanceTabPage, ShowInDocumentHdl, weld::Toggleable&, void)
{
    // get selected entry index and ColorConfigValue
    ColorConfigEntry nEntry = GetActiveEntry();
    if (nEntry == ColorConfigEntryCount)
        return;

    ColorConfigValue aCurrentEntryColor = pColorConfig->GetColorValue(nEntry);

    // set colorconfig value
    aCurrentEntryColor.bIsVisible = m_xShowInDocumentChkBtn->get_active();
    pColorConfig->SetColorValue(nEntry, aCurrentEntryColor);
}

IMPL_LINK_NOARG(SvxAppearanceTabPage, EnableAppThemingHdl, weld::Toggleable&, void)
{
    ThemeColors::SetThemeState(m_xEnableAppTheming->get_active() ? ThemeState::ENABLED
                                                                 : ThemeState::DISABLED);
    m_bRestartRequired = true;
}

IMPL_LINK_NOARG(SvxAppearanceTabPage, UseOnlyWhiteDocBackgroundHdl, weld::Toggleable&, void)
{
    ThemeColors::SetUseOnlyWhiteDocBackground(m_xUseOnlyWhiteDocBackground->get_active());
    m_bRestartRequired = true;
}

IMPL_LINK_NOARG(SvxAppearanceTabPage, ColorEntryChgHdl, weld::ComboBox&, void)
{
    // get selected entry index and ColorConfigValue
    ColorConfigEntry nEntry = GetActiveEntry();
    if (nEntry == ColorConfigEntryCount)
        return;

    const ColorConfigValue& rCurrentEntryColor = pColorConfig->GetColorValue(nEntry);

    // set automatic colors
    m_xColorChangeBtn->SetAutoDisplayColor(ColorConfig::GetDefaultColor(nEntry));
    // set values for the entry
    if (IsDarkModeEnabled())
        m_xColorChangeBtn->SelectEntry(rCurrentEntryColor.nDarkColor);
    else
        m_xColorChangeBtn->SelectEntry(rCurrentEntryColor.nLightColor);

    m_xShowInDocumentChkBtn->set_active(rCurrentEntryColor.bIsVisible);

    // show/hide show in document button
    if (!cNames[nEntry].bCanBeVisible)
        m_xShowInDocumentChkBtn->hide();
    else
        m_xShowInDocumentChkBtn->show();
}

IMPL_LINK_NOARG(SvxAppearanceTabPage, ColorValueChgHdl, ColorListBox&, void)
{
    // get the active entry
    ColorConfigEntry nEntry = GetActiveEntry();
    if (nEntry == ColorConfigEntryCount)
        return;

    ColorConfigValue aCurrentEntryColor = pColorConfig->GetColorValue(nEntry);

    // restart only for the UI colors
    if (nEntry >= WINDOWCOLOR)
        m_bRestartRequired = true;

    // set the color in pColorConfig
    if (IsDarkModeEnabled())
        aCurrentEntryColor.nDarkColor = m_xColorChangeBtn->GetSelectEntryColor();
    else
        aCurrentEntryColor.nLightColor = m_xColorChangeBtn->GetSelectEntryColor();

    // use nColor for caching the value of color in use. This avoids tedious refactoring which IMO
    // would use function calls to discriminate between colors. Those functions themself call some virtual functions
    // making the whole thing super slow (comparatively).
    aCurrentEntryColor.nColor = m_xColorChangeBtn->GetSelectEntryColor();

    pColorConfig->SetColorValue(nEntry, aCurrentEntryColor);
}

IMPL_LINK_NOARG(SvxAppearanceTabPage, SchemeChangeHdl, weld::ComboBox&, void)
{
    AppearanceMode eMode;
    OUString sNewScheme;

    if (ThemeColors::IsAutomaticTheme(m_xSchemeList->get_active_id()))
    {
        sNewScheme = AUTOMATIC_COLOR_SCHEME;
        eMode = AppearanceMode::AUTO;
    }
    else if (ThemeColors::IsLightTheme(m_xSchemeList->get_active_id()))
    {
        sNewScheme = LIGHT_COLOR_SCHEME;
        eMode = AppearanceMode::LIGHT;
    }
    else if (ThemeColors::IsDarkTheme(m_xSchemeList->get_active_id()))
    {
        sNewScheme = DARK_COLOR_SCHEME;
        eMode = AppearanceMode::DARK;
    }
    else
    {
        sNewScheme = m_xSchemeList->get_active_text();
        // for custom theme, keep appearance to AUTO, doesn't matter for the most part,
        // but if some colors like document colors are not specified, then they will be
        // set as per the OS appearance mode
        eMode = AppearanceMode::AUTO;
    }

    // NOTE: LoadScheme must be called after updating the AppearanceMode as LoadScheme calls Lode
    // which loads colors based on the current application appearance.
    MiscSettings::SetAppColorMode(eMode);
    pColorConfig->LoadScheme(sNewScheme);

    if (m_xSchemeList->get_value_changed_from_saved() && !ThemeColors::IsThemeDisabled())
        m_bRestartRequired = true;

    UpdateColorDropdown();
}

IMPL_LINK_NOARG(SvxAppearanceTabPage, SchemeListToggleHdl, weld::ComboBox&, void)
{
    LoadSchemeList();
}

IMPL_STATIC_LINK_NOARG(SvxAppearanceTabPage, MoreThemesHdl, weld::Button&, void)
{
    css::uno::Sequence<css::beans::PropertyValue> aArgs{ comphelper::makePropertyValue(
        u"AdditionsTag"_ustr, u"Themes"_ustr) };
    comphelper::dispatchCommand(u".uno:AdditionsDialog"_ustr, aArgs);
}

IMPL_LINK_NOARG(SvxAppearanceTabPage, ResetAllBtnHdl, weld::Button&, void)
{
    // load default document colors
    ColorConfigValue aValue;
    for (size_t i = 0; i < ColorConfigEntryCount; ++i)
    {
        aValue.nDarkColor = COL_AUTO;
        aValue.nLightColor = COL_AUTO;
        pColorConfig->SetColorValue(static_cast<ColorConfigEntry>(i), aValue);
    }
    pColorConfig->Commit();

    // RESET state for themes just prevents the theme colors from being used before
    // they are reloaded from the StyleSettings, please read the comment above
    // ColorConfig::SetupTheme()'s definition
    if (!ThemeColors::IsThemeDisabled())
    {
        ThemeColors::ResetTheme();
        m_bRestartRequired = true;
    }
}

IMPL_LINK_NOARG(SvxAppearanceTabPage, BitmapDropDownHdl, weld::ComboBox&, void)
{
    ThemeColors::SetAppBackBmpFileName(m_xBitmapDropDown->get_active_id());
}

IMPL_LINK_NOARG(SvxAppearanceTabPage, BitmapDrawTypeDropDownHdl, weld::ComboBox&, void)
{
    ThemeColors::SetAppBackBmpDrawType(m_xBitmapDrawTypeDropDown->get_active_id());
}

IMPL_LINK_NOARG(SvxAppearanceTabPage, UseBmpForAppBackHdl, weld::Toggleable&, void)
{
    ThemeColors::SetUseBmpForAppBack(m_xUseBmpForAppBack->get_active());
    UpdateBmpControlsState();
}

void SvxAppearanceTabPage::InitThemes()
{
    // init schemes combobox
    LoadSchemeList();

    m_xSchemeList->connect_changed(LINK(this, SvxAppearanceTabPage, SchemeChangeHdl));
    m_xSchemeList->connect_popup_toggled(LINK(this, SvxAppearanceTabPage, SchemeListToggleHdl));
    m_xMoreThemesBtn->connect_clicked(LINK(this, SvxAppearanceTabPage, MoreThemesHdl));

    m_xEnableAppTheming->connect_toggled(LINK(this, SvxAppearanceTabPage, EnableAppThemingHdl));
    m_xEnableAppTheming->set_active(ThemeColors::IsThemeEnabled());

    m_xUseOnlyWhiteDocBackground->connect_toggled(
        LINK(this, SvxAppearanceTabPage, UseOnlyWhiteDocBackgroundHdl));
    m_xUseOnlyWhiteDocBackground->set_active(ThemeColors::UseOnlyWhiteDocBackground());

    // connect callbacks for bitmap controls
    m_xUseBmpForAppBack->connect_toggled(LINK(this, SvxAppearanceTabPage, UseBmpForAppBackHdl));
    m_xBitmapDropDown->connect_changed(LINK(this, SvxAppearanceTabPage, BitmapDropDownHdl));
    m_xBitmapDrawTypeDropDown->connect_changed(
        LINK(this, SvxAppearanceTabPage, BitmapDrawTypeDropDownHdl));

    // initialize bitmap controls
    m_xUseBmpForAppBack->set_active(ThemeColors::UseBmpForAppBack());

    // insert bitmap entries
    for (size_t i = 0; i < getBitmapList().size(); ++i)
        m_xBitmapDropDown->append(getBitmapList()[i].aRawString,
                                  getBitmapList()[i].aTranslatedString);

    // check if the registry setting is valid or not
    bool bFound = false;
    for (size_t i = 0; i < getBitmapList().size(); ++i)
    {
        if (ThemeColors::GetAppBackBmpFileName() == getBitmapList()[i].aRawString)
        {
            bFound = true;
            m_xBitmapDropDown->set_active_id(ThemeColors::GetAppBackBmpFileName());
            break;
        }
    }
    if (!bFound)
        m_xBitmapDropDown->set_active(0);

    // insert bitmap draw type entries
    for (size_t i = 0; i < getBitmapDrawTypeList().size(); ++i)
        m_xBitmapDrawTypeDropDown->append(getBitmapDrawTypeList()[i].aRawString,
                                          getBitmapDrawTypeList()[i].aTranslatedString);

    // check if the registry setting is valid or not
    bFound = false;
    for (size_t i = 0; i < getBitmapList().size(); ++i)
    {
        if (ThemeColors::GetAppBackBmpDrawType() == getBitmapDrawTypeList()[i].aRawString)
        {
            bFound = true;
            m_xBitmapDrawTypeDropDown->set_active_id(ThemeColors::GetAppBackBmpDrawType());
            break;
        }
    }
    if (!bFound)
        m_xBitmapDrawTypeDropDown->set_active(0);

    UpdateBmpControlsState();
}

void SvxAppearanceTabPage::InitCustomization()
{
    m_xColorEntryBtn->connect_changed(LINK(this, SvxAppearanceTabPage, ColorEntryChgHdl));
    m_xColorChangeBtn->SetSelectHdl(LINK(this, SvxAppearanceTabPage, ColorValueChgHdl));
    m_xShowInDocumentChkBtn->connect_toggled(LINK(this, SvxAppearanceTabPage, ShowInDocumentHdl));
    m_xResetAllBtn->connect_clicked(LINK(this, SvxAppearanceTabPage, ResetAllBtnHdl));

    FillItemsList();

    m_xColorChangeBtn->SetSlotId(SID_ATTR_CHAR_COLOR);
    m_xColorEntryBtn->set_active(DOCCOLOR);
    UpdateColorDropdown();

    m_xShowInDocumentChkBtn->set_active(pColorConfig->GetColorValue(DOCCOLOR).bIsVisible);
    m_xShowInDocumentChkBtn->hide();
}

void SvxAppearanceTabPage::UpdateColorDropdown()
{
    // update color to light/dark
    ColorConfigEntry nEntry = GetActiveEntry();
    if (nEntry == ColorConfigEntryCount)
        return;

    const ColorConfigValue& rCurrentEntryColor = pColorConfig->GetColorValue(nEntry);

    // set automatic colors
    m_xColorChangeBtn->SetAutoDisplayColor(ColorConfig::GetDefaultColor(nEntry));
    // set values for the entry
    if (IsDarkModeEnabled())
        m_xColorChangeBtn->SelectEntry(rCurrentEntryColor.nDarkColor);
    else
        m_xColorChangeBtn->SelectEntry(rCurrentEntryColor.nLightColor);
}

void SvxAppearanceTabPage::UpdateBmpControlsState()
{
    bool bEnabled = m_xUseBmpForAppBack->get_active();
    m_xBitmapDropDown->set_sensitive(bEnabled);
    m_xBitmapDrawTypeDropDown->set_sensitive(bEnabled);
}

void SvxAppearanceTabPage::FillItemsList()
{
    static const std::map<ColorConfigEntry, OUString> aRegistryEntries
        = { { DOCCOLOR, CuiResId(REG_DOCCOLOR) },
            { DOCBOUNDARIES, CuiResId(REG_DOCBOUNDARIES) },
            { APPBACKGROUND, CuiResId(REG_APPBACKGROUND) },
            { TABLEBOUNDARIES, CuiResId(REG_TABLEBOUNDARIES) },
            { FONTCOLOR, CuiResId(REG_FONTCOLOR) },
            { LINKS, CuiResId(REG_LINKS) },
            { LINKSVISITED, CuiResId(REG_LINKSVISITED) },
            { SPELL, CuiResId(REG_SPELL) },
            { GRAMMAR, CuiResId(REG_GRAMMAR) },
            { SMARTTAGS, CuiResId(REG_SMARTTAGS) },
            { SHADOWCOLOR, CuiResId(REG_SHADOW) },
            { WRITERTEXTGRID, CuiResId(REG_WRITERTEXTGRID) },
            { WRITERFIELDSHADINGS, CuiResId(REG_WRITERFIELDSHADINGS) },
            { WRITERIDXSHADINGS, CuiResId(REG_WRITERIDXSHADINGS) },
            { WRITERDIRECTCURSOR, CuiResId(REG_WRITERDIRECTCURSOR) },
            { WRITERSCRIPTINDICATOR, CuiResId(REG_WRITERSCRIPTINDICATOR) },
            { WRITERSECTIONBOUNDARIES, CuiResId(REG_WRITERSECTIONBOUNDARIES) },
            { WRITERHEADERFOOTERMARK, CuiResId(REG_WRITERHEADERFOOTERMARK) },
            { WRITERPAGEBREAKS, CuiResId(REG_WRITERPAGEBREAKS) },
            { WRITERNONPRINTCHARS, CuiResId(REG_WRITERNONPRINTCHARS) },
            { HTMLSGML, CuiResId(REG_HTMLSGML) },
            { HTMLCOMMENT, CuiResId(REG_HTMLCOMMENT) },
            { HTMLKEYWORD, CuiResId(REG_HTMLKEYWORD) },
            { HTMLUNKNOWN, CuiResId(REG_HTMLUNKNOWN) },
            { CALCGRID, CuiResId(REG_CALCGRID) },
            { CALCCELLFOCUS, CuiResId(REG_CALCCELLFOCUS) },
            { CALCPAGEBREAK, CuiResId(REG_CALCPAGEBREAK) },
            { CALCPAGEBREAKMANUAL, CuiResId(REG_CALCPAGEBREAKMANUAL) },
            { CALCPAGEBREAKAUTOMATIC, CuiResId(REG_CALCPAGEBREAKAUTOMATIC) },
            { CALCHIDDENROWCOL, CuiResId(REG_CALCHIDDENCOLROW) },
            { CALCTEXTOVERFLOW, CuiResId(REG_CALCTEXTOVERFLOW) },
            { CALCCOMMENTS, CuiResId(REG_CALCCOMMENTS) },
            { CALCDETECTIVE, CuiResId(REG_CALCDETECTIVE) },
            { CALCDETECTIVEERROR, CuiResId(REG_CALCDETECTIVEERROR) },
            { CALCREFERENCE, CuiResId(REG_CALCREFERENCE) },
            { CALCNOTESBACKGROUND, CuiResId(REG_CALCNOTESBACKGROUND) },
            { CALCVALUE, CuiResId(REG_CALCVALUE) },
            { CALCFORMULA, CuiResId(REG_CALCFORMULA) },
            { CALCTEXT, CuiResId(REG_CALCTEXT) },
            { CALCPROTECTEDBACKGROUND, CuiResId(REG_CALCPROTECTEDBACKGROUND) },
            { DRAWGRID, CuiResId(REG_DRAWGRID) },
            { AUTHOR1, CuiResId(REG_AUTHOR1) },
            { AUTHOR2, CuiResId(REG_AUTHOR2) },
            { AUTHOR3, CuiResId(REG_AUTHOR3) },
            { AUTHOR4, CuiResId(REG_AUTHOR4) },
            { AUTHOR5, CuiResId(REG_AUTHOR5) },
            { AUTHOR6, CuiResId(REG_AUTHOR6) },
            { AUTHOR7, CuiResId(REG_AUTHOR7) },
            { AUTHOR8, CuiResId(REG_AUTHOR8) },
            { AUTHOR9, CuiResId(REG_AUTHOR9) },
            { BASICEDITOR, CuiResId(REG_BASICEDITOR) },
            { BASICIDENTIFIER, CuiResId(REG_BASICIDENTIFIER) },
            { BASICCOMMENT, CuiResId(REG_BASICCOMMENT) },
            { BASICNUMBER, CuiResId(REG_BASICNUMBER) },
            { BASICSTRING, CuiResId(REG_BASICSTRING) },
            { BASICOPERATOR, CuiResId(REG_BASICOPERATOR) },
            { BASICKEYWORD, CuiResId(REG_BASICKEYWORD) },
            { BASICERROR, CuiResId(REG_BASICERROR) },
            { SQLIDENTIFIER, CuiResId(REG_SQLIDENTIFIER) },
            { SQLNUMBER, CuiResId(REG_SQLNUMBER) },
            { SQLSTRING, CuiResId(REG_SQLSTRING) },
            { SQLOPERATOR, CuiResId(REG_SQLOPERATOR) },
            { SQLKEYWORD, CuiResId(REG_SQLKEYWORD) },
            { SQLPARAMETER, CuiResId(REG_SQLPARAMETER) },
            { SQLCOMMENT, CuiResId(REG_SQLCOMMENT) },
            { WINDOWCOLOR, CuiResId(REG_WINDOWCOLOR) },
            { WINDOWTEXTCOLOR, CuiResId(REG_WINDOWTEXTCOLOR) },
            { BASECOLOR, CuiResId(REG_BASECOLOR) },
            { BUTTONCOLOR, CuiResId(REG_BUTTONCOLOR) },
            { BUTTONTEXTCOLOR, CuiResId(REG_BUTTONTEXTCOLOR) },
            { ACCENTCOLOR, CuiResId(REG_ACCENTCOLOR) },
            { DISABLEDCOLOR, CuiResId(REG_DISABLEDCOLOR) },
            { DISABLEDTEXTCOLOR, CuiResId(REG_DISABLEDTEXTCOLOR) },
            { SHADECOLOR, CuiResId(REG_SHADOWCOLOR) },
            { SEPARATORCOLOR, CuiResId(REG_SEPARATORCOLOR) },
            { FACECOLOR, CuiResId(REG_FACECOLOR) },
            { ACTIVECOLOR, CuiResId(REG_ACTIVECOLOR) },
            { ACTIVETEXTCOLOR, CuiResId(REG_ACTIVETEXTCOLOR) },
            { ACTIVEBORDERCOLOR, CuiResId(REG_ACTIVEBORDERCOLOR) },
            { FIELDCOLOR, CuiResId(REG_FIELDCOLOR) },
            { MENUBARCOLOR, CuiResId(REG_MENUBARCOLOR) },
            { MENUBARTEXTCOLOR, CuiResId(REG_MENUBARTEXTCOLOR) },
            { MENUBARHIGHLIGHTCOLOR, CuiResId(REG_MENUBARHIGHLIGHTCOLOR) },
            { MENUBARHIGHLIGHTTEXTCOLOR, CuiResId(REG_MENUBARHIGHLIGHTTEXTCOLOR) },
            { MENUCOLOR, CuiResId(REG_MENUCOLOR) },
            { MENUTEXTCOLOR, CuiResId(REG_MENUTEXTCOLOR) },
            { MENUHIGHLIGHTCOLOR, CuiResId(REG_MENUHIGHLIGHTCOLOR) },
            { MENUHIGHLIGHTTEXTCOLOR, CuiResId(REG_MENUHIGHLIGHTTEXTCOLOR) },
            { MENUBORDERCOLOR, CuiResId(REG_MENUBORDERCOLOR) },
            { INACTIVECOLOR, CuiResId(REG_INACTIVECOLOR) },
            { INACTIVETEXTCOLOR, CuiResId(REG_INACTIVETEXTCOLOR) },
            { INACTIVEBORDERCOLOR, CuiResId(REG_INACTIVEBORDERCOLOR) } };

    for (size_t i = DOCCOLOR; i <= SHADOWCOLOR; ++i)
        m_xColorEntryBtn->append(OUString(cNames[i].cName),
                                 aRegistryEntries.at(static_cast<ColorConfigEntry>(i)));
    m_xColorEntryBtn->append_separator("SeparatorID");

    for (size_t i = WRITERTEXTGRID; i <= WRITERNONPRINTCHARS; ++i)
        m_xColorEntryBtn->append(OUString(cNames[i].cName),
                                 aRegistryEntries.at(static_cast<ColorConfigEntry>(i)));
    m_xColorEntryBtn->append_separator("SeparatorID");

    for (size_t i = HTMLSGML; i <= HTMLUNKNOWN; ++i)
        m_xColorEntryBtn->append(OUString(cNames[i].cName),
                                 aRegistryEntries.at(static_cast<ColorConfigEntry>(i)));
    m_xColorEntryBtn->append_separator("SeparatorID");

    for (size_t i = CALCGRID; i <= CALCPROTECTEDBACKGROUND; ++i)
        m_xColorEntryBtn->append(OUString(cNames[i].cName),
                                 aRegistryEntries.at(static_cast<ColorConfigEntry>(i)));
    m_xColorEntryBtn->append_separator("SeparatorID");

    m_xColorEntryBtn->append(OUString(cNames[DRAWGRID].cName), aRegistryEntries.at(DRAWGRID));
    m_xColorEntryBtn->append_separator("SeparatorID");

    for (size_t i = AUTHOR1; i <= AUTHOR9; ++i)
        m_xColorEntryBtn->append(OUString(cNames[i].cName),
                                 aRegistryEntries.at(static_cast<ColorConfigEntry>(i)));
    m_xColorEntryBtn->append_separator("SeparatorID");

    for (size_t i = BASICEDITOR; i <= BASICERROR; ++i)
        m_xColorEntryBtn->append(OUString(cNames[i].cName),
                                 aRegistryEntries.at(static_cast<ColorConfigEntry>(i)));
    m_xColorEntryBtn->append_separator("SeparatorID");

    for (size_t i = SQLIDENTIFIER; i <= SQLCOMMENT; ++i)
        m_xColorEntryBtn->append(OUString(cNames[i].cName),
                                 aRegistryEntries.at(static_cast<ColorConfigEntry>(i)));
    m_xColorEntryBtn->append_separator("SeparatorID");

    // don't show UI customization options if theme is disabled
    if (!ThemeColors::IsThemeDisabled())
        for (size_t i = WINDOWCOLOR; i <= INACTIVEBORDERCOLOR; ++i)
            m_xColorEntryBtn->append(OUString(cNames[i].cName),
                                     aRegistryEntries.at(static_cast<ColorConfigEntry>(i)));
}

ColorConfigEntry SvxAppearanceTabPage::GetActiveEntry()
{
    OUString sEntryId = m_xColorEntryBtn->get_active_id();
    for (int nEntry = 0; nEntry < ColorConfigEntryCount; ++nEntry)
    {
        if (sEntryId == cNames[nEntry].cName)
            return static_cast<ColorConfigEntry>(nEntry);
    }
    return ColorConfigEntryCount;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
