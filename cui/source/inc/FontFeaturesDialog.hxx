/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/font/Feature.hxx>
#include <svx/fntctrl.hxx>
#include <memory>

namespace cui
{
struct FontFeatureItem
{
    FontFeatureItem(weld::Widget* pParent)
        : m_aFeatureCode(0)
        , m_nDefault(-1)
        , m_xBuilder(Application::CreateBuilder(pParent, u"cui/ui/fontfragment.ui"_ustr))
        , m_xContainer(m_xBuilder->weld_widget(u"fontentry"_ustr))
        , m_xText(m_xBuilder->weld_label(u"label"_ustr))
        , m_xCombo(m_xBuilder->weld_combo_box(u"combo"_ustr))
        , m_xCheck(m_xBuilder->weld_check_button(u"check"_ustr))
    {
        m_xCheck->connect_toggled(LINK(this, FontFeatureItem, CheckBoxToggledHdl));
    }

    sal_uInt32 m_aFeatureCode;
    sal_Int32 m_nDefault;
    weld::TriStateEnabled m_aTriStateEnabled;
    Link<weld::Toggleable&, void> m_aToggleHdl;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Widget> m_xContainer;
    std::unique_ptr<weld::Label> m_xText;
    std::unique_ptr<weld::ComboBox> m_xCombo;
    std::unique_ptr<weld::CheckButton> m_xCheck;

private:
    DECL_LINK(CheckBoxToggledHdl, weld::Toggleable&, void);
};

class FontFeaturesDialog : public weld::GenericDialogController
{
private:
    std::vector<std::unique_ptr<FontFeatureItem>> m_aFeatureItems;
    OUString m_sFontName;
    OUString m_sResultFontName;

    SvxFontPrevWindow m_aPreviewWindow;
    std::unique_ptr<weld::ScrolledWindow> m_xContentWindow;
    std::unique_ptr<weld::Container> m_xContentBox;
    std::unique_ptr<weld::Container> m_xContentGrid;
    std::unique_ptr<weld::Container> m_xStylisticSetsBox;
    std::unique_ptr<weld::Container> m_xStylisticSetsGrid;
    std::unique_ptr<weld::Container> m_xCharacterVariantsBox;
    std::unique_ptr<weld::Container> m_xCharacterVariantsGrid;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWindow;

    void initialize();
    OUString createFontNameWithFeatures();

    // returns the max height of a row
    int fillGrid(std::vector<vcl::font::Feature> const& rFontFeatures);

    DECL_LINK(ComboBoxSelectedHdl, weld::ComboBox&, void);
    DECL_LINK(CheckBoxToggledHdl, weld::Toggleable&, void);

public:
    FontFeaturesDialog(weld::Window* pParent, OUString aFontName);
    ~FontFeaturesDialog() override;
    virtual short run() override;

    OUString const& getResultFontName() const { return m_sResultFontName; }

    void updateFontPreview();
};

} // end svx namespaces

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
