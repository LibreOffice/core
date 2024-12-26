/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/colorbox.hxx>
#include <svx/xflasit.hxx>
#include <svx/svxdlg.hxx>
#include <svtools/colorcfg.hxx>

/*
 * SYSTEM       ==> the application will use either light or dark colors from a theme based on
 *                  whether the system is in light mode or in dark mode
 *
 * LIGHT/DARK   ==> light/dark theme colors
 *
 * Note that Appearance settings have nothing to do with the themes, these just specify which one
 * of the theme colors (light/dark) should be used and whether the operating system should decide
 * that
 *
 * LibreOffice Themes will be enabled if some theme other than Automatic is selected.
 */

using namespace svtools;
class SvxAppearanceTabPage : public SfxTabPage
{
private:
    enum class Appearance
    {
        SYSTEM,
        LIGHT,
        DARK,
    };

    bool m_bRestartRequired;
    Appearance eCurrentAppearanceMode;
    std::unique_ptr<EditableColorConfig> pColorConfig;

    std::unique_ptr<weld::ComboBox> m_xSchemeList;
    std::unique_ptr<weld::Button> m_xMoreThemesBtn;
    std::unique_ptr<weld::Button> m_xAddSchemeBtn;
    std::unique_ptr<weld::Button> m_xRemoveSchemeBtn;
    std::unique_ptr<weld::RadioButton> m_xAppearanceSystem;
    std::unique_ptr<weld::RadioButton> m_xAppearanceLight;
    std::unique_ptr<weld::RadioButton> m_xAppearanceDark;
    std::unique_ptr<weld::ComboBox> m_xColorEntryBtn;
    std::unique_ptr<ColorListBox> m_xColorChangeBtn;
    std::unique_ptr<weld::CheckButton> m_xShowInDocumentChkBtn;

    std::unique_ptr<weld::RadioButton> m_xColorRadioBtn;
    std::unique_ptr<weld::RadioButton> m_xImageRadioBtn;
    std::unique_ptr<weld::RadioButton> m_xStretchedRadioBtn;
    std::unique_ptr<weld::RadioButton> m_xTiledRadioBtn;

    std::unique_ptr<weld::ComboBox> m_xBitmapDropDownBtn;

    DECL_LINK(AppearanceChangeHdl, weld::Toggleable&, void);
    DECL_LINK(ColorEntryChgHdl, weld::ComboBox&, void);
    DECL_LINK(ColorValueChgHdl, ColorListBox&, void);
    DECL_LINK(ShowInDocumentHdl, weld::Toggleable&, void);
    DECL_LINK(SchemeChangeHdl, weld::ComboBox&, void);
    DECL_LINK(SchemeListToggleHdl, weld::ComboBox&, void);
    DECL_STATIC_LINK(SvxAppearanceTabPage, MoreThemesHdl, weld::Button&, void);
    DECL_LINK(AddRemoveSchemeHdl, weld::Button&, void);
    DECL_LINK(CheckNameHdl_Impl, AbstractSvxNameDialog&, bool);
    DECL_LINK(ColorImageToggleHdl, weld::Toggleable&, void);
    DECL_LINK(StretchedTiledToggleHdl, weld::Toggleable&, void);
    DECL_LINK(BitmapChangeHdl, weld::ComboBox&, void);

    void InitThemes();
    void InitAppearance();
    void InitCustomization();
    void UpdateControlsState();
    void LoadSchemeList();

    void UpdateRemoveBtnState();
    void EnableImageControls(bool bEnabled);
    void UpdateColorDropdown();
    void UpdateOldAppearance();
    bool IsDarkModeEnabled();
    void FillItemsList();
    ColorConfigEntry GetActiveEntry();

public:
    SvxAppearanceTabPage(weld::Container* pPage, weld::DialogController* pController,
                         const SfxItemSet& rSet);

    virtual ~SvxAppearanceTabPage() override;

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet);

    virtual OUString GetAllStrings() override;
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
