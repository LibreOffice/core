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
#include <utility>
#include <vcl/font/FeatureParser.hxx>
#include <FontFeatures.hxx>
#include <unordered_set>

using namespace css;

namespace cui
{
FontFeaturesDialog::FontFeaturesDialog(weld::Window* pParent, OUString aFontName)
    : GenericDialogController(pParent, u"cui/ui/fontfeaturesdialog.ui"_ustr,
                              u"FontFeaturesDialog"_ustr)
    , m_sFontName(std::move(aFontName))
    , m_xContentWindow(m_xBuilder->weld_scrolled_window(u"contentWindow"_ustr))
    , m_xContentBox(m_xBuilder->weld_container(u"contentBox"_ustr))
    , m_xContentGrid(m_xBuilder->weld_container(u"contentGrid"_ustr))
    , m_xStylisticSetsBox(m_xBuilder->weld_container(u"stylisticSetsBox"_ustr))
    , m_xStylisticSetsGrid(m_xBuilder->weld_container(u"stylisticSetsGrid"_ustr))
    , m_xCharacterVariantsBox(m_xBuilder->weld_container(u"characterVariantsBox"_ustr))
    , m_xCharacterVariantsGrid(m_xBuilder->weld_container(u"characterVariantsGrid"_ustr))
    , m_xPreviewWindow(new weld::CustomWeld(*m_xBuilder, u"preview"_ustr, m_aPreviewWindow))
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
                                              DeviceFormat::WITH_ALPHA);
    std::vector<vcl::font::Feature> rFontFeatures = getFontFeatureList(m_sFontName, *aVDev);

    std::unordered_set<sal_uInt32> aDoneFeatures;
    std::vector<vcl::font::Feature> rFilteredFontFeatures;

    for (vcl::font::Feature const& rFontFeature : rFontFeatures)
    {
        sal_uInt32 nFontFeatureCode = rFontFeature.m_nCode;
        if (!aDoneFeatures.insert(nFontFeatureCode).second)
            continue;
        rFilteredFontFeatures.push_back(rFontFeature);
    }

    int nRowHeight = fillGrid(rFilteredFontFeatures);

    auto nFeaturesHeight = m_xContentBox->get_preferred_size().Height()
                           + m_xStylisticSetsBox->get_preferred_size().Height()
                           + m_xCharacterVariantsBox->get_preferred_size().Height();
    m_xContentWindow->set_size_request(
        -1, std::min(std::max(m_xContentWindow->get_preferred_size().Height(), nFeaturesHeight),
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

    sal_Int32 nIdx, nStylisticSets(0), nCharacterVariants(0), nOtherFeatures(0);
    for (vcl::font::Feature const& rFontFeature : rFontFeatures)
    {
        sal_uInt32 nFontFeatureCode = rFontFeature.m_nCode;

        vcl::font::FeatureDefinition aDefinition;
        if (rFontFeature.m_aDefinition)
            aDefinition = rFontFeature.m_aDefinition;
        if (!aDefinition)
            aDefinition = { nFontFeatureCode, u""_ustr };

        if (rFontFeature.isStylisticSet())
        {
            nIdx = nStylisticSets++;
            m_xStylisticSetsBox->set_visible(true);
            m_aFeatureItems.emplace_back(
                std::make_unique<FontFeatureItem>(m_xStylisticSetsGrid.get()));
        }
        else if (rFontFeature.isCharacterVariant())
        {
            nIdx = nCharacterVariants++;
            m_xCharacterVariantsBox->set_visible(true);
            m_aFeatureItems.emplace_back(
                std::make_unique<FontFeatureItem>(m_xCharacterVariantsGrid.get()));
        }
        else
        {
            nIdx = nOtherFeatures++;
            m_xContentBox->set_visible(true);
            m_aFeatureItems.emplace_back(std::make_unique<FontFeatureItem>(m_xContentGrid.get()));
        }

        int32_t nValue = 0;
        if (aExistingFeatures.find(nFontFeatureCode) != aExistingFeatures.end())
            nValue = aExistingFeatures.at(nFontFeatureCode);
        else
            nValue = aDefinition.getDefault();

        FontFeatureItem& aCurrentItem = *m_aFeatureItems.back();
        aCurrentItem.m_aFeatureCode = nFontFeatureCode;
        aCurrentItem.m_nDefault = aDefinition.getDefault();

        sal_Int32 nGridPositionX = (nIdx % 2) * 2;
        sal_Int32 nGridPositionY = nIdx / 2;
        aCurrentItem.m_xContainer->set_grid_left_attach(nGridPositionX);
        aCurrentItem.m_xContainer->set_grid_top_attach(nGridPositionY);

        Link<weld::ComboBox&, void> aComboBoxSelectHandler
            = LINK(this, FontFeaturesDialog, ComboBoxSelectedHdl);
        Link<weld::Toggleable&, void> aCheckBoxToggleHandler
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
            if (nValue < 0)
            {
                aCurrentItem.m_xCheck->set_state(TRISTATE_INDET);
                aCurrentItem.m_aTriStateEnabled.bTriStateEnabled = true;
                aCurrentItem.m_aTriStateEnabled.eState = TRISTATE_INDET;
            }
            else
            {
                aCurrentItem.m_xCheck->set_state(nValue > 0 ? TRISTATE_TRUE : TRISTATE_FALSE);
                aCurrentItem.m_aTriStateEnabled.bTriStateEnabled = false;
                aCurrentItem.m_aTriStateEnabled.eState = aCurrentItem.m_xCheck->get_state();
            }
            aCurrentItem.m_xCheck->set_label(aDefinition.getDescription());
            aCurrentItem.m_aToggleHdl = aCheckBoxToggleHandler;
            aCurrentItem.m_xCheck->show();
        }

        nRowHeight
            = std::max<int>(nRowHeight, aCurrentItem.m_xContainer->get_preferred_size().Height());
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

IMPL_LINK(FontFeatureItem, CheckBoxToggledHdl, weld::Toggleable&, rToggle, void)
{
    m_aTriStateEnabled.ButtonToggled(rToggle);
    m_aTriStateEnabled.bTriStateEnabled = false;
    m_aToggleHdl.Call(rToggle);
}

IMPL_LINK_NOARG(FontFeaturesDialog, CheckBoxToggledHdl, weld::Toggleable&, void)
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

    for (const auto& rEntry : m_aFeatureItems)
    {
        const FontFeatureItem& rItem(*rEntry);
        if (rItem.m_xCheck->get_visible())
        {
            if (rItem.m_xCheck->get_state() != TRISTATE_INDET)
            {
                if (!bFirst)
                    sNameSuffix.append(vcl::font::FeatureSeparator);
                else
                    bFirst = false;

                sNameSuffix.append(vcl::font::featureCodeAsString(rItem.m_aFeatureCode));
                if (rItem.m_xCheck->get_state() == TRISTATE_FALSE)
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

                sNameSuffix.append(vcl::font::featureCodeAsString(rItem.m_aFeatureCode) + "="
                                   + OUString::number(nSelection));
            }
        }
    }
    sResultFontName = vcl::font::trimFontNameFeatures(m_sFontName);
    if (!sNameSuffix.isEmpty())
        sResultFontName += OUStringChar(vcl::font::FeaturePrefix) + sNameSuffix;
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
