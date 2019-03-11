/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <FontFeaturesDialog.hxx>
#include <vcl/font/FeatureParser.hxx>
#include <vcl/virdev.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/dialmgr.hxx>
#include <unordered_set>

using namespace css;

namespace cui
{
FontFeaturesDialog::FontFeaturesDialog(weld::Window* pParent, OUString const& rFontName)
    : GenericDialogController(pParent, "cui/ui/fontfeaturesdialog.ui", "FontFeaturesDialog")
    , m_sFontName(rFontName)
    , m_xContentWindow(m_xBuilder->weld_scrolled_window("contentWindow"))
    , m_xContentGrid(m_xBuilder->weld_container("contentGrid"))
    , m_xPreviewWindow(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreviewWindow))
{
    svtools::ColorConfig aColorConfig;
    Color aFillColor(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);
    m_aPreviewWindow.SetBackColor(aFillColor);
    initialize();
}

FontFeaturesDialog::~FontFeaturesDialog() {}

static void makeEnumComboBox(weld::ComboBox& rNameBox,
                             vcl::font::FeatureDefinition const& rFeatureDefinition)
{
    for (vcl::font::FeatureParameter const& rParameter : rFeatureDefinition.getEnumParameters())
        rNameBox.append_text(rParameter.getDescription());
}

void FontFeaturesDialog::initialize()
{
    ScopedVclPtrInstance<VirtualDevice> aVDev(*Application::GetDefaultDevice(),
                                              DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
    aVDev->SetOutputSizePixel(Size(10, 10));

    vcl::Font aFont = aVDev->GetFont();
    aFont.SetFamilyName(m_sFontName);
    aVDev->SetFont(aFont);

    std::vector<vcl::font::Feature> rFontFeatures;

    if (!aVDev->GetFontFeatures(rFontFeatures))
        return;

    std::unordered_set<sal_uInt32> aDoneFeatures;
    std::vector<vcl::font::Feature> rFilteredFontFeatures;

    for (vcl::font::Feature const& rFontFeature : rFontFeatures)
    {
        sal_uInt32 nFontFeatureCode = rFontFeature.m_aID.m_aFeatureCode;
        if (aDoneFeatures.find(nFontFeatureCode) != aDoneFeatures.end())
            continue;
        aDoneFeatures.insert(nFontFeatureCode);
        rFilteredFontFeatures.push_back(rFontFeature);
    }

    fillGrid(rFilteredFontFeatures);

    updateFontPreview();
}

void FontFeaturesDialog::fillGrid(std::vector<vcl::font::Feature> const& rFontFeatures)
{
    vcl::font::FeatureParser aParser(m_sFontName);
    auto aExistingFeatures = aParser.getFeaturesMap();

    sal_Int32 i = 0;
    for (vcl::font::Feature const& rFontFeature : rFontFeatures)
    {
        sal_uInt32 nFontFeatureCode = rFontFeature.m_aID.m_aFeatureCode;

        vcl::font::FeatureDefinition aDefinition;
        if (rFontFeature.m_aDefinition)
            aDefinition = rFontFeature.m_aDefinition;
        if (!aDefinition)
            aDefinition = { nFontFeatureCode, nullptr };

        m_aFeatureItems.emplace_back(m_xContentGrid.get());

        uint32_t nValue = 0;
        if (aExistingFeatures.find(nFontFeatureCode) != aExistingFeatures.end())
            nValue = aExistingFeatures.at(nFontFeatureCode);

        FontFeatureItem& aCurrentItem = m_aFeatureItems.back();
        aCurrentItem.m_aFeatureCode = nFontFeatureCode;

        sal_Int32 nGridPositionX = (i % 2) * 2;
        sal_Int32 nGridPositionY = i / 2;
        aCurrentItem.m_xContainer->set_grid_left_attach(nGridPositionX);
        aCurrentItem.m_xContainer->set_grid_top_attach(nGridPositionY);

        Link<weld::ComboBox&, void> aComboBoxSelectHandler
            = LINK(this, FontFeaturesDialog, ComboBoxSelectedHdl);
        Link<weld::ToggleButton&, void> aCheckBoxToggleHandler
            = LINK(this, FontFeaturesDialog, CheckBoxToggledHdl);

        if (aDefinition.getType() == vcl::font::FeatureParameterType::ENUM)
        {
            aCurrentItem.m_xText->set_label(aDefinition.getDescription());
            aCurrentItem.m_xText->show();

            makeEnumComboBox(*aCurrentItem.m_xCombo, aDefinition);

            aCurrentItem.m_xCombo->set_active(nValue);
            aCurrentItem.m_xCombo->connect_changed(aComboBoxSelectHandler);
            aCurrentItem.m_xCombo->show();
        }
        else
        {
            aCurrentItem.m_xCheck->set_active(nValue > 0);
            aCurrentItem.m_xCheck->set_label(aDefinition.getDescription());
            aCurrentItem.m_xCheck->connect_toggled(aCheckBoxToggleHandler);
            aCurrentItem.m_xCheck->show();
        }

        i++;
    }
}

void FontFeaturesDialog::updateFontPreview()
{
    vcl::Font rPreviewFont = m_aPreviewWindow.GetFont();
    vcl::Font rPreviewFontCJK = m_aPreviewWindow.GetCJKFont();
    vcl::Font rPreviewFontCTL = m_aPreviewWindow.GetCTLFont();

    OUString sNewFontName = createFontNameWithFeatures();

    rPreviewFont.SetFamilyName(sNewFontName);
    rPreviewFontCJK.SetFamilyName(sNewFontName);
    rPreviewFontCTL.SetFamilyName(sNewFontName);

    m_aPreviewWindow.SetFont(rPreviewFont, rPreviewFontCJK, rPreviewFontCTL);
}

IMPL_LINK_NOARG(FontFeaturesDialog, CheckBoxToggledHdl, weld::ToggleButton&, void)
{
    updateFontPreview();
}

IMPL_LINK_NOARG(FontFeaturesDialog, ComboBoxSelectedHdl, weld::ComboBox&, void)
{
    updateFontPreview();
}

OUString FontFeaturesDialog::createFontNameWithFeatures()
{
    OUString sResultFontName;
    OUStringBuffer sNameSuffix;
    bool bFirst = true;

    for (FontFeatureItem& rItem : m_aFeatureItems)
    {
        if (rItem.m_xCheck->get_visible())
        {
            if (rItem.m_xCheck->get_active())
            {
                if (!bFirst)
                    sNameSuffix.append(OUString(vcl::font::FeatureSeparator));
                else
                    bFirst = false;

                sNameSuffix.append(vcl::font::featureCodeAsString(rItem.m_aFeatureCode));
            }
        }
        else if (rItem.m_xCombo->get_visible() && rItem.m_xText->get_visible())
        {
            int nSelection = rItem.m_xCombo->get_active();
            if (nSelection > 0)
            {
                if (!bFirst)
                    sNameSuffix.append(OUString(vcl::font::FeatureSeparator));
                else
                    bFirst = false;

                sNameSuffix.append(vcl::font::featureCodeAsString(rItem.m_aFeatureCode));
                sNameSuffix.append("=");
                sNameSuffix.append(OUString::number(nSelection));
            }
        }
    }
    sResultFontName = vcl::font::trimFontNameFeatures(m_sFontName);
    if (!sNameSuffix.isEmpty())
        sResultFontName += OUString(vcl::font::FeaturePrefix) + sNameSuffix.makeStringAndClear();
    return sResultFontName;
}

short FontFeaturesDialog::run()
{
    short nResult = GenericDialogController::run();
    if (nResult == RET_OK)
    {
        m_sResultFontName = createFontNameWithFeatures();
    }
    return nResult;
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
