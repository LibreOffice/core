/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <smediteng.hxx>
#include <smmod.hxx>
#include <cfgitem.hxx>

#include <vcl/settings.hxx>
#include <editeng/editview.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>

SmEditEngine::SmEditEngine(SfxItemPool* pItemPool)
    : EditEngine(pItemPool)
    , m_nOldZoom(100)
    , m_nNewZoom(100)
    , m_nDefaultFontSize(0)
    , m_aAllSelection(0, 0, 0, 0)
{
    SetText(u""_ustr);

    // Add external text leading
    SetAddExtLeading(true);

    // Allow to undo changes ( Ctrl + z )
    EnableUndo(true);

    // Length in pixel of a tabulation
    SetDefTab(sal_uInt16(Application::GetDefaultDevice()->GetTextWidth(u"XXXX"_ustr)));

    // Set default background color by theme
    SetBackgroundColor(
        Application::GetDefaultDevice()->GetSettings().GetStyleSettings().GetFieldColor());

    // Control words
    SetControlWord((GetControlWord() | EEControlBits::AUTOINDENTING)
                   & EEControlBits(~EEControlBits::UNDOATTRIBS)
                   & EEControlBits(~EEControlBits::PASTESPECIAL));

    // Word delimiters for auto word selection by double click
    SetWordDelimiters(u" .=+-*/(){}[];\""_ustr);

    // Default mapping mode
    SetRefMapMode(MapMode(MapUnit::MapPixel));

    // Default size of the box
    SetPaperSize(Size(1000, 0));
}

bool SmEditEngine::checkZoom()
{
    return m_nOldZoom != (m_nNewZoom = SM_MOD()->GetConfig()->GetSmEditWindowZoomFactor());
}

void SmEditEngine::executeZoom(EditView* pEditView)
{
    if (checkZoom())
    {
        updateZoom();
        if (pEditView)
        {
            FormatAndLayout(pEditView);
            pEditView->SetSelection(pEditView->GetSelection());
        }
    }
}

void SmEditEngine::updateZoom()
{
    // In first run get font size as base to scale
    if (m_nDefaultFontSize == 0)
    {
        SfxItemSet sfxatts = GetAttribs(0, 0, 0, GetAttribsFlags::CHARATTRIBS);
        const SvxFontHeightItem* sfxattsh = sfxatts.GetItem(EE_CHAR_FONTHEIGHT);
        m_nDefaultFontSize = sfxattsh->GetHeight();
    }

    // Now we calculate the new font size
    sal_Int32 nNewFontSize = m_nDefaultFontSize * m_nNewZoom / 100;

    // We apply the new font size to all the text
    updateAllESelection(); // Update length of the text
    SfxItemSet aSet = GetEmptyItemSet();
    aSet.Put(SvxFontHeightItem(nNewFontSize, 100, EE_CHAR_FONTHEIGHT));
    QuickSetAttribs(aSet, m_aAllSelection);

    // We don't forget to equalize the zoomvalues
    m_nOldZoom = m_nNewZoom;
}

void SmEditEngine::updateAllESelection()
{
    sal_Int32 paracount = GetParagraphCount();
    m_aAllSelection.nEndPara = paracount > 0 ? paracount - 1 : 0;
    sal_Int32 textlength = GetTextLen(m_aAllSelection.nEndPara);
    m_aAllSelection.nEndPos = textlength > 0 ? textlength : 0;
}

void SmEditEngine::setSmItemPool(SfxItemPool* mpItemPool, const SvtLinguOptions& maLangOptions)
{
    // Set fonts to be used
    struct FontData
    {
        LanguageType nFallbackLang;
        LanguageType nLang;
        DefaultFontType nFontType;
        sal_uInt16 nFontInfoId;
    };

    FontData aFontDataTable[3]
        = { // info to get western font to be used
            { LANGUAGE_ENGLISH_US, maLangOptions.nDefaultLanguage, DefaultFontType::FIXED,
              EE_CHAR_FONTINFO },
            // info to get CJK font to be used
            { LANGUAGE_JAPANESE, maLangOptions.nDefaultLanguage_CJK, DefaultFontType::CJK_TEXT,
              EE_CHAR_FONTINFO_CJK },
            // info to get CTL font to be used
            { LANGUAGE_ARABIC_SAUDI_ARABIA, maLangOptions.nDefaultLanguage_CTL,
              DefaultFontType::CTL_TEXT, EE_CHAR_FONTINFO_CTL }
          };

    // Text color
    auto aDefaultDevice = Application::GetDefaultDevice();
    Color aTextColor = aDefaultDevice->GetSettings().GetStyleSettings().GetFieldTextColor();
    for (const FontData& aFontData : aFontDataTable)
    {
        LanguageType nLang
            = (LANGUAGE_NONE == aFontData.nLang) ? aFontData.nFallbackLang : aFontData.nLang;
        vcl::Font aFont = OutputDevice::GetDefaultFont(aFontData.nFontType, nLang,
                                                       GetDefaultFontFlags::OnlyOne);
        aFont.SetColor(aTextColor);
        mpItemPool->SetUserDefaultItem(SvxFontItem(aFont.GetFamilyType(), aFont.GetFamilyName(),
                                                   aFont.GetStyleName(), aFont.GetPitch(),
                                                   aFont.GetCharSet(), aFontData.nFontInfoId));
    }

    // Set font heights
    SvxFontHeightItem aFontHeight(
        aDefaultDevice->LogicToPixel(Size(0, 11), MapMode(MapUnit::MapPoint)).Height(), 100,
        EE_CHAR_FONTHEIGHT);
    mpItemPool->SetUserDefaultItem(aFontHeight);
    aFontHeight.SetWhich(EE_CHAR_FONTHEIGHT_CJK);
    mpItemPool->SetUserDefaultItem(aFontHeight);
    aFontHeight.SetWhich(EE_CHAR_FONTHEIGHT_CTL);
    mpItemPool->SetUserDefaultItem(aFontHeight);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
