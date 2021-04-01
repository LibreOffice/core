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
#include <rtl/ustrbuf.hxx>
#include <vcl/font/FeatureParser.hxx>
#include <vcl/virdev.hxx>
#include <svtools/colorcfg.hxx>
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
    initialize();
}

FontFeaturesDialog::~FontFeaturesDialog() {}

static sal_Int32 makeEnumComboBox(weld::ComboBox& rNameBox,
                                  vcl::font::FeatureDefinition const& rFeatureDefinition,
                                  uint32_t nDefault)
{
    sal_Int32 nRes = 0;
    int count = 0;
    for (vcl::font::FeatureParameter const& rParameter : rFeatureDefinition.getEnumParameters())
    {
        rNameBox.append(OUString::number(rParameter.getCode()), rParameter.getDescription());
        if (rParameter.getCode() == nDefault)
            nRes = count;
        ++count;
    }
    return nRes;
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
        if (!aDoneFeatures.insert(nFontFeatureCode).second)
            continue;
        rFilteredFontFeatures.push_back(rFontFeature);
    }

    int nRowHeight = fillGrid(rFilteredFontFeatures);

    m_xContentWindow->set_size_request(
        -1, std::min(std::max(m_xContentWindow->get_preferred_size().Height(),
                              m_xContentGrid->get_preferred_size().Height()),
                     static_cast<tools::Long>(300L)));

    if (nRowHeight)
    {
        // tdf#141333 use row height + the 6 px spacing of contentGrid
        m_xContentWindow->vadjustment_set_step_increment(nRowHeight + 6);
    }

    updateFontPreview();
}

int FontFeaturesDialog::fillGrid(std::vector<vcl::font::Feature> const& rFontFeatures)
{
    int nRowHeight(0);

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
        else
            nValue = aDefinition.getDefault();

        FontFeatureItem& aCurrentItem = m_aFeatureItems.back();
        aCurrentItem.m_aFeatureCode = nFontFeatureCode;
        aCurrentItem.m_nDefault = aDefinition.getDefault();

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

            sal_Int32 nInit = makeEnumComboBox(*aCurrentItem.m_xCombo, aDefinition, nValue);

            aCurrentItem.m_xCombo->set_active(nInit);
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

        nRowHeight
            = std::max<int>(nRowHeight, aCurrentItem.m_xContainer->get_preferred_size().Height());

        i++;
    }

    return nRowHeight;
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

    for (const FontFeatureItem& rItem : m_aFeatureItems)
    {
        if (rItem.m_xCheck->get_visible())
        {
            if (sal_uInt32(rItem.m_xCheck->get_active()) != rItem.m_nDefault)
            {
                if (!bFirst)
                    sNameSuffix.append(vcl::font::FeatureSeparator);
                else
                    bFirst = false;

                sNameSuffix.append(vcl::font::featureCodeAsString(rItem.m_aFeatureCode));
                if (!rItem.m_xCheck->get_active())
                    sNameSuffix.append("=0");
            }
        }
        else if (rItem.m_xCombo->get_visible() && rItem.m_xText->get_visible())
        {
            sal_Int32 nSelection = rItem.m_xCombo->get_active_id().toInt32();
            if (nSelection != int(rItem.m_nDefault))
            {
                if (!bFirst)
                    sNameSuffix.append(vcl::font::FeatureSeparator);
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
        sResultFontName
            += OUStringChar(vcl::font::FeaturePrefix) + sNameSuffix.makeStringAndClear();
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
