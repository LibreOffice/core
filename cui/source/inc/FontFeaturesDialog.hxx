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
        , m_nDefault(0)
        , m_xBuilder(Application::CreateBuilder(pParent, "cui/ui/fontfragment.ui"))
        , m_xContainer(m_xBuilder->weld_widget("fontentry"))
        , m_xText(m_xBuilder->weld_label("label"))
        , m_xCombo(m_xBuilder->weld_combo_box("combo"))
        , m_xCheck(m_xBuilder->weld_check_button("check"))
    {
    }

    sal_uInt32 m_aFeatureCode;
    sal_uInt32 m_nDefault;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Widget> m_xContainer;
    std::unique_ptr<weld::Label> m_xText;
    std::unique_ptr<weld::ComboBox> m_xCombo;
    std::unique_ptr<weld::CheckButton> m_xCheck;
};

class FontFeaturesDialog : public weld::GenericDialogController
{
private:
    std::vector<FontFeatureItem> m_aFeatureItems;
    OUString m_sFontName;
    OUString m_sResultFontName;

    SvxFontPrevWindow m_aPreviewWindow;
    std::unique_ptr<weld::ScrolledWindow> m_xContentWindow;
    std::unique_ptr<weld::Container> m_xContentGrid;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWindow;

    void initialize();
    OUString createFontNameWithFeatures();

    void fillGrid(std::vector<vcl::font::Feature> const& rFontFeatures);

    DECL_LINK(ComboBoxSelectedHdl, weld::ComboBox&, void);
    DECL_LINK(CheckBoxToggledHdl, weld::ToggleButton&, void);

public:
    FontFeaturesDialog(weld::Window* pParent, OUString const& rFontName);
    ~FontFeaturesDialog() override;
    virtual short run() override;

    OUString const& getResultFontName() const { return m_sResultFontName; }

    void updateFontPreview();
};

} // end svx namespaces

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
