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
#include <svx/dialmgr.hxx>

using namespace css;

namespace cui
{
FontFeaturesDialog::FontFeaturesDialog(vcl::Window* pParent, OUString const& rFontName)
    : ModalDialog(pParent, "FontFeaturesDialog", "cui/ui/fontfeaturesdialog.ui")
    , m_sFontName(rFontName)
{
    get(m_pContentGrid, "contentGrid");
    get(m_pPreviewWindow, "preview");
    initialize();
}

FontFeaturesDialog::~FontFeaturesDialog() { disposeOnce(); }

VclPtr<ComboBox> makeEnumComboBox(vcl::Window* pParent,
                                  vcl::font::FeatureDefinition const& rFeatureDefinition)
{
    VclPtr<ComboBox> aNameBox(
        VclPtr<ComboBox>::Create(pParent, WB_TABSTOP | WB_DROPDOWN | WB_AUTOHSCROLL));
    for (vcl::font::FeatureParameter const& rParameter : rFeatureDefinition.getEnumParameters())
    {
        aNameBox->InsertEntry(rParameter.getDescription());
    }
    aNameBox->EnableAutoSize(true);
    return aNameBox;
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
    std::unordered_map<sal_uInt32, sal_uInt32> aExistingFeatures = aParser.getFeaturesMap();

    sal_Int32 i = 0;
    for (vcl::font::Feature const& rFontFeature : rFontFeatures)
    {
        sal_uInt32 nFontFeatureCode = rFontFeature.m_aID.m_aFeatureCode;

        vcl::font::FeatureDefinition aDefinition;
        if (rFontFeature.m_aDefinition)
            aDefinition = rFontFeature.m_aDefinition;
        if (!aDefinition)
            aDefinition = { nFontFeatureCode, nullptr };

        m_aFeatureItems.emplace_back();

        sal_uInt32 nValue = 0;
        if (aExistingFeatures.find(nFontFeatureCode) != aExistingFeatures.end())
            nValue = aExistingFeatures.at(nFontFeatureCode);

        FontFeatureItem& aCurrentItem = m_aFeatureItems.back();
        aCurrentItem.m_aFeatureCode = nFontFeatureCode;

        sal_Int32 nGridPositionX = (i % 2) * 2;
        sal_Int32 nGridPositionY = i / 2;

        Link<ComboBox&, void> aComboBoxSelectHandler
            = LINK(this, FontFeaturesDialog, ComboBoxSelectedHdl);
        Link<CheckBox&, void> aCheckBoxToggleHandler
            = LINK(this, FontFeaturesDialog, CheckBoxToggledHdl);

        if (aDefinition.getType() == vcl::font::FeatureParameterType::ENUM)
        {
            aCurrentItem.m_pText
                = VclPtr<FixedText>::Create(m_pContentGrid, WB_LEFT | WB_VCENTER | WB_3DLOOK);
            aCurrentItem.m_pText->set_grid_left_attach(nGridPositionX);
            aCurrentItem.m_pText->set_grid_top_attach(nGridPositionY);
            aCurrentItem.m_pText->set_margin_left(6);
            aCurrentItem.m_pText->set_margin_right(6);
            aCurrentItem.m_pText->set_margin_top(3);
            aCurrentItem.m_pText->set_margin_bottom(3);
            aCurrentItem.m_pText->SetText(aDefinition.getDescription());
            aCurrentItem.m_pText->Show();

            aCurrentItem.m_pCombo = makeEnumComboBox(m_pContentGrid, aDefinition);

            aCurrentItem.m_pCombo->SelectEntryPos(nValue);
            aCurrentItem.m_pCombo->set_grid_left_attach(nGridPositionX + 1);
            aCurrentItem.m_pCombo->set_grid_top_attach(nGridPositionY);
            aCurrentItem.m_pCombo->set_margin_left(6);
            aCurrentItem.m_pCombo->set_margin_right(6);
            aCurrentItem.m_pCombo->set_margin_top(3);
            aCurrentItem.m_pCombo->set_margin_bottom(3);
            aCurrentItem.m_pCombo->SetSelectHdl(aComboBoxSelectHandler);
            aCurrentItem.m_pCombo->Show();
        }
        else
        {
            aCurrentItem.m_pCheck = VclPtr<CheckBox>::Create(
                m_pContentGrid, WB_CLIPCHILDREN | WB_LEFT | WB_VCENTER | WB_3DLOOK);
            aCurrentItem.m_pCheck->set_grid_left_attach(nGridPositionX);
            aCurrentItem.m_pCheck->set_grid_top_attach(nGridPositionY);
            aCurrentItem.m_pCheck->set_grid_width(2);
            aCurrentItem.m_pCheck->set_margin_left(6);
            aCurrentItem.m_pCheck->set_margin_right(6);
            aCurrentItem.m_pCheck->set_margin_top(3);
            aCurrentItem.m_pCheck->set_margin_bottom(3);
            aCurrentItem.m_pCheck->Check(nValue > 0);
            aCurrentItem.m_pCheck->SetText(aDefinition.getDescription());
            aCurrentItem.m_pCheck->SetToggleHdl(aCheckBoxToggleHandler);
            aCurrentItem.m_pCheck->Show();
        }

        i++;
    }
}

void FontFeaturesDialog::updateFontPreview()
{
    vcl::Font rPreviewFont = m_pPreviewWindow->GetFont();
    vcl::Font rPreviewFontCJK = m_pPreviewWindow->GetCJKFont();
    vcl::Font rPreviewFontCTL = m_pPreviewWindow->GetCTLFont();

    OUString sNewFontName = createFontNameWithFeatures();

    rPreviewFont.SetFamilyName(sNewFontName);
    rPreviewFontCJK.SetFamilyName(sNewFontName);
    rPreviewFontCTL.SetFamilyName(sNewFontName);

    m_pPreviewWindow->SetFont(rPreviewFont, rPreviewFontCJK, rPreviewFontCTL);
}

void FontFeaturesDialog::dispose()
{
    m_pContentGrid.clear();
    m_pPreviewWindow.clear();
    for (FontFeatureItem& rItem : m_aFeatureItems)
    {
        rItem.m_pText.disposeAndClear();
        rItem.m_pCombo.disposeAndClear();
        rItem.m_pCheck.disposeAndClear();
    }
    ModalDialog::dispose();
}

IMPL_LINK_NOARG(FontFeaturesDialog, CheckBoxToggledHdl, CheckBox&, void) { updateFontPreview(); }

IMPL_LINK_NOARG(FontFeaturesDialog, ComboBoxSelectedHdl, ComboBox&, void) { updateFontPreview(); }

OUString FontFeaturesDialog::createFontNameWithFeatures()
{
    OUString sResultFontName;
    OUStringBuffer sNameSuffix;
    bool bFirst = true;

    for (FontFeatureItem& rItem : m_aFeatureItems)
    {
        if (rItem.m_pCheck)
        {
            if (rItem.m_pCheck->IsChecked())
            {
                if (!bFirst)
                    sNameSuffix.append(OUString(vcl::font::FeatureSeparator));
                else
                    bFirst = false;

                sNameSuffix.append(vcl::font::featureCodeAsString(rItem.m_aFeatureCode));
            }
        }
        else if (rItem.m_pCombo && rItem.m_pText)
        {
            sal_uInt32 nSelection = rItem.m_pCombo->GetSelectedEntryPos();
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

short FontFeaturesDialog::Execute()
{
    short nResult = ModalDialog::Execute();
    if (nResult == RET_OK)
    {
        m_sResultFontName = createFontNameWithFeatures();
    }
    return nResult;
}

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
