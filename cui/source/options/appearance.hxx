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
#include <sfx2/app.hxx>
#include <vcl/graph.hxx>
#include <vcl/IconThemeInfo.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/svapp.hxx>

using namespace svtools;
class SvxAppearanceTabPage : public SfxTabPage
{
private:
    bool m_bRestartRequired;
    std::unique_ptr<EditableColorConfig> pColorConfig;

    std::unique_ptr<weld::ComboBox> m_xSchemeList;
    std::unique_ptr<weld::Button> m_xMoreThemesBtn;
    std::unique_ptr<weld::CheckButton> m_xEnableAppTheming;
    std::unique_ptr<weld::CheckButton> m_xUseOnlyWhiteDocBackground;
    std::unique_ptr<weld::ComboBox> m_xColorEntryBtn;
    std::unique_ptr<ColorListBox> m_xColorChangeBtn;
    std::unique_ptr<weld::CheckButton> m_xShowInDocumentChkBtn;
    std::unique_ptr<weld::Button> m_xResetAllBtn;

    std::unique_ptr<weld::CheckButton> m_xUseBmpForAppBack;
    std::unique_ptr<weld::ComboBox> m_xBitmapDropDown;
    std::unique_ptr<weld::ComboBox> m_xBitmapDrawTypeDropDown;

    std::vector<vcl::IconThemeInfo> mInstalledIconThemes;

    std::unique_ptr<weld::ComboBox> m_xIconsDropDown;
    std::unique_ptr<weld::Button> m_xMoreIconsBtn;
    std::unique_ptr<weld::ComboBox> m_xToolbarIconSize;
    std::unique_ptr<weld::ComboBox> m_xSidebarIconSize;
    std::unique_ptr<weld::ComboBox> m_xNotebookbarIconSize;
    OUString m_sAutoStr;

    sal_Int32 nInitialToolbarIconSizeSel;
    sal_Int32 nInitialSidebarIconSizeSel;
    sal_Int32 nInitialNotebookbarIconSizeSel;
    sal_Int32 nInitialIconThemeSel;

    DECL_LINK(ColorEntryChgHdl, weld::ComboBox&, void);
    DECL_LINK(ColorValueChgHdl, ColorListBox&, void);
    DECL_LINK(ShowInDocumentHdl, weld::Toggleable&, void);
    DECL_LINK(EnableAppThemingHdl, weld::Toggleable&, void);
    DECL_LINK(UseOnlyWhiteDocBackgroundHdl, weld::Toggleable&, void);
    DECL_LINK(SchemeChangeHdl, weld::ComboBox&, void);
    DECL_LINK(SchemeListToggleHdl, weld::ComboBox&, void);
    DECL_STATIC_LINK(SvxAppearanceTabPage, MoreThemesHdl, weld::Button&, void);
    DECL_LINK(ResetAllBtnHdl, weld::Button&, void);

    DECL_LINK(BitmapDropDownHdl, weld::ComboBox&, void);
    DECL_LINK(BitmapDrawTypeDropDownHdl, weld::ComboBox&, void);
    DECL_LINK(UseBmpForAppBackHdl, weld::Toggleable&, void);

    DECL_LINK(OnIconThemeChange, weld::ComboBox&, void);
    DECL_STATIC_LINK(SvxAppearanceTabPage, OnMoreIconsClick, weld::Button&, void);
    DECL_LINK(OnToolbarIconSizeChange, weld::ComboBox&, void);
    DECL_LINK(OnSidebarIconSizeChange, weld::ComboBox&, void);
    DECL_LINK(OnNotebookbarIconSizeChange, weld::ComboBox&, void);

    void InitThemes();
    void InitCustomization();
    void InitIcons();
    void LoadSchemeList();

    void UpdateBmpControlsState();
    void UpdateColorDropdown();
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
