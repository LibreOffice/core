/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <memory>
#include <map>
#include <optasian.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/any.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <svl/asiancfg.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unotools/localedatawrapper.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;

constexpr OUString cIsKernAsianPunctuation = u"IsKernAsianPunctuation"_ustr;
constexpr OUString cCharacterCompressionType = u"CharacterCompressionType"_ustr;

namespace {

struct SvxForbiddenChars_Impl
{
    bool                                  bRemoved;
    std::optional<ForbiddenCharacters>    oCharacters;
};

}

struct SvxAsianLayoutPage_Impl
{
    SvxAsianConfig  aConfig;
    SvxAsianLayoutPage_Impl() {}

    Reference< XForbiddenCharacters >   xForbidden;
    Reference< XPropertySet >           xPrSet;
    Reference< XPropertySetInfo >       xPrSetInfo;
    std::map< LanguageType, SvxForbiddenChars_Impl >
                                        aChangedLanguagesMap;

    bool                hasForbiddenCharacters(LanguageType eLang);
    SvxForbiddenChars_Impl* getForbiddenCharacters(LanguageType eLang);
    void                    addForbiddenCharacters(LanguageType eLang, std::optional<ForbiddenCharacters> oForbidden);
};

bool    SvxAsianLayoutPage_Impl::hasForbiddenCharacters(LanguageType eLang)
{
    return aChangedLanguagesMap.count( eLang );
}

SvxForbiddenChars_Impl* SvxAsianLayoutPage_Impl::getForbiddenCharacters(LanguageType eLang)
{
    auto it = aChangedLanguagesMap.find( eLang );
    DBG_ASSERT( ( it != aChangedLanguagesMap.end() ), "language not available");
    if( it != aChangedLanguagesMap.end() )
        return &it->second;
    return nullptr;
}

void SvxAsianLayoutPage_Impl::addForbiddenCharacters(
    LanguageType eLang, std::optional<ForbiddenCharacters> oForbidden)
{
    auto itOld = aChangedLanguagesMap.find( eLang );
    if( itOld == aChangedLanguagesMap.end() )
    {
        SvxForbiddenChars_Impl aChar;
        aChar.bRemoved = !oForbidden.has_value();
        aChar.oCharacters = std::move(oForbidden);
        aChangedLanguagesMap.emplace( eLang, std::move(aChar) );
    }
    else
    {
        itOld->second.bRemoved = !oForbidden.has_value();
        itOld->second.oCharacters = std::move(oForbidden);
    }
}

static LanguageType eLastUsedLanguageTypeForForbiddenCharacters(USHRT_MAX);

SvxAsianLayoutPage::SvxAsianLayoutPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/optasianpage.ui", "OptAsianPage", &rSet)
    , pImpl(new SvxAsianLayoutPage_Impl)
    , m_xCharKerningRB(m_xBuilder->weld_radio_button("charkerning"))
    , m_xCharPunctKerningRB(m_xBuilder->weld_radio_button("charpunctkerning"))
    , m_xNoCompressionRB(m_xBuilder->weld_radio_button("nocompression"))
    , m_xPunctCompressionRB(m_xBuilder->weld_radio_button("punctcompression"))
    , m_xPunctKanaCompressionRB(m_xBuilder->weld_radio_button("punctkanacompression"))
    , m_xLanguageFT(m_xBuilder->weld_label("languageft"))
    , m_xLanguageLB(new SvxLanguageBox(m_xBuilder->weld_combo_box("language")))
    , m_xStandardCB(m_xBuilder->weld_check_button("standard"))
    , m_xStartFT(m_xBuilder->weld_label("startft"))
    , m_xStartED(m_xBuilder->weld_entry("start"))
    , m_xEndFT(m_xBuilder->weld_label("endft"))
    , m_xEndED(m_xBuilder->weld_entry("end"))
    , m_xHintFT(m_xBuilder->weld_label("hintft"))
{
    LanguageHdl(*m_xLanguageLB->get_widget());
    m_xLanguageLB->connect_changed(LINK(this, SvxAsianLayoutPage, LanguageHdl));
    m_xStandardCB->connect_toggled(LINK(this, SvxAsianLayoutPage, ChangeStandardHdl));
    Link<weld::Entry&,void> aLk(LINK(this, SvxAsianLayoutPage, ModifyHdl));
    m_xStartED->connect_changed(aLk);
    m_xEndED->connect_changed(aLk);

    m_xLanguageLB->SetLanguageList( SvxLanguageListFlags::FBD_CHARS, false, false );
}

SvxAsianLayoutPage::~SvxAsianLayoutPage()
{
}

std::unique_ptr<SfxTabPage> SvxAsianLayoutPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SvxAsianLayoutPage>(pPage, pController, *rAttrSet);
}

OUString SvxAsianLayoutPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[]
        = { "label1", "label2", "label3", "languageft", "startft", "endft", "hintft" };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString radioButton[] = { "charkerning", "charpunctkerning", "nocompression",
                               "punctcompression", "punctkanacompression" };

    for (const auto& radio : radioButton)
    {
        if (const auto& pString = m_xBuilder->weld_radio_button(radio))
            sAllStrings += pString->get_label() + " ";
    }

    sAllStrings += m_xStandardCB->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

bool SvxAsianLayoutPage::FillItemSet( SfxItemSet* )
{
    if(m_xCharKerningRB->get_state_changed_from_saved())
    {
        pImpl->aConfig.SetKerningWesternTextOnly(m_xCharKerningRB->get_active());
        OUString sPunct(cIsKernAsianPunctuation);
        if(pImpl->xPrSetInfo.is() && pImpl->xPrSetInfo->hasPropertyByName(sPunct))
        {
            bool bVal = !m_xCharKerningRB->get_active();
            pImpl->xPrSet->setPropertyValue(sPunct, Any(bVal));
        }
    }

    if(m_xNoCompressionRB->get_state_changed_from_saved() ||
       m_xPunctCompressionRB->get_state_changed_from_saved())
    {
        CharCompressType nSet = m_xNoCompressionRB->get_active() ? CharCompressType::NONE :
                            m_xPunctCompressionRB->get_active() ? CharCompressType::PunctuationOnly :
                            CharCompressType::PunctuationAndKana;
        pImpl->aConfig.SetCharDistanceCompression(nSet);
        OUString sCompress(cCharacterCompressionType);
        if(pImpl->xPrSetInfo.is() && pImpl->xPrSetInfo->hasPropertyByName(sCompress))
        {
            pImpl->xPrSet->setPropertyValue(sCompress, Any(static_cast<sal_uInt16>(nSet)));
        }
    }
    pImpl->aConfig.Commit();
    if(pImpl->xForbidden.is())
    {
        try
        {
            for (auto const& changedLanguage : pImpl->aChangedLanguagesMap)
            {
                Locale aLocale( LanguageTag::convertToLocale(changedLanguage.first));
                if(changedLanguage.second.bRemoved)
                    pImpl->xForbidden->removeForbiddenCharacters( aLocale );
                else if(changedLanguage.second.oCharacters)
                    pImpl->xForbidden->setForbiddenCharacters( aLocale, *( changedLanguage.second.oCharacters ) );
            }
        }
        catch (const Exception&)
        {
            TOOLS_WARN_EXCEPTION( "cui.options", "in XForbiddenCharacters");
        }
    }
    eLastUsedLanguageTypeForForbiddenCharacters = m_xLanguageLB->get_active_id();

    return false;
}

void SvxAsianLayoutPage::Reset( const SfxItemSet* )
{
    SfxViewFrame* pCurFrm = SfxViewFrame::Current();
    SfxObjectShell* pDocSh = pCurFrm ? pCurFrm->GetObjectShell() : nullptr;
    Reference< XModel > xModel;
    if(pDocSh)
        xModel = pDocSh->GetModel();
    Reference<XMultiServiceFactory> xFact(xModel, UNO_QUERY);
    if(xFact.is())
    {
        pImpl->xPrSet.set(xFact->createInstance("com.sun.star.document.Settings"), UNO_QUERY);
    }
    if( pImpl->xPrSet.is() )
        pImpl->xPrSetInfo = pImpl->xPrSet->getPropertySetInfo();
    bool bKernWesternText = SvxAsianConfig::IsKerningWesternTextOnly();
    CharCompressType nCompress = SvxAsianConfig::GetCharDistanceCompression();
    if(pImpl->xPrSetInfo.is())
    {
        OUString sForbidden("ForbiddenCharacters");
        if(pImpl->xPrSetInfo->hasPropertyByName(sForbidden))
        {
            Any aForbidden = pImpl->xPrSet->getPropertyValue(sForbidden);
            aForbidden >>= pImpl->xForbidden;
        }
        OUString sCompress(cCharacterCompressionType);
        if(pImpl->xPrSetInfo->hasPropertyByName(sCompress))
        {
            Any aVal = pImpl->xPrSet->getPropertyValue(sCompress);
            sal_uInt16 nTmp;
            if (aVal >>= nTmp)
                nCompress = static_cast<CharCompressType>(nTmp);
        }
        OUString sPunct(cIsKernAsianPunctuation);
        if(pImpl->xPrSetInfo->hasPropertyByName(sPunct))
        {
            Any aVal = pImpl->xPrSet->getPropertyValue(sPunct);
            bKernWesternText = !*o3tl::doAccess<bool>(aVal);
        }
    }
    else
    {
        m_xLanguageFT->set_sensitive(false);
        m_xLanguageLB->set_sensitive(false);
        m_xStandardCB->set_sensitive(false);
        m_xStartFT->set_sensitive(false);
        m_xStartED->set_sensitive(false);
        m_xEndFT->set_sensitive(false);
        m_xEndED->set_sensitive(false);
        m_xHintFT->set_sensitive(false);
    }
    if(bKernWesternText)
        m_xCharKerningRB->set_active(true);
    else
        m_xCharPunctKerningRB->set_active(true);
    switch(nCompress)
    {
        case CharCompressType::NONE : m_xNoCompressionRB->set_active(true);        break;
        case CharCompressType::PunctuationOnly : m_xPunctCompressionRB->set_active(true);     break;
        default: m_xPunctKanaCompressionRB->set_active(true);
    }
    m_xCharKerningRB->save_state();
    m_xNoCompressionRB->save_state();
    m_xPunctCompressionRB->save_state();
    m_xPunctKanaCompressionRB->save_state();

    m_xLanguageLB->set_active(0);
    //preselect the system language in the box - if available
    if(LanguageType(USHRT_MAX) == eLastUsedLanguageTypeForForbiddenCharacters)
    {
        eLastUsedLanguageTypeForForbiddenCharacters =
            Application::GetSettings().GetLanguageTag().getLanguageType();
        if (MsLangId::isSimplifiedChinese(eLastUsedLanguageTypeForForbiddenCharacters))
            eLastUsedLanguageTypeForForbiddenCharacters = LANGUAGE_CHINESE_SIMPLIFIED;
        else if (MsLangId::isTraditionalChinese(eLastUsedLanguageTypeForForbiddenCharacters))
            eLastUsedLanguageTypeForForbiddenCharacters = LANGUAGE_CHINESE_TRADITIONAL;
    }
    m_xLanguageLB->set_active_id(eLastUsedLanguageTypeForForbiddenCharacters);
    LanguageHdl(*m_xLanguageLB->get_widget());
}

IMPL_LINK_NOARG(SvxAsianLayoutPage, LanguageHdl, weld::ComboBox&, void)
{
    //set current value
    LanguageType eSelectLanguage = m_xLanguageLB->get_active_id();
    LanguageTag aLanguageTag( eSelectLanguage);
    const Locale& aLocale( aLanguageTag.getLocale());

    OUString sStart, sEnd;
    bool bAvail;
    if(pImpl->xForbidden.is())
    {
        bAvail = pImpl->hasForbiddenCharacters(eSelectLanguage);
        if(bAvail)
        {
            SvxForbiddenChars_Impl* pElement = pImpl->getForbiddenCharacters(eSelectLanguage);
            if(pElement->bRemoved || !pElement->oCharacters)
            {
                bAvail = false;
            }
            else
            {
                sStart = pElement->oCharacters->beginLine;
                sEnd = pElement->oCharacters->endLine;
            }
        }
        else
        {
            try
            {
                bAvail = pImpl->xForbidden->hasForbiddenCharacters(aLocale);
                if(bAvail)
                {
                    ForbiddenCharacters aForbidden = pImpl->xForbidden->getForbiddenCharacters( aLocale );
                    sStart = aForbidden.beginLine;
                    sEnd = aForbidden.endLine;
                }
            }
            catch (const Exception&)
            {
                TOOLS_WARN_EXCEPTION( "cui.options", "in XForbiddenCharacters");
            }
        }
    }
    else
    {
        bAvail = SvxAsianConfig::GetStartEndChars( aLocale, sStart, sEnd );
    }
    if(!bAvail)
    {
        LocaleDataWrapper aWrap( std::move(aLanguageTag) );
        ForbiddenCharacters aForbidden = aWrap.getForbiddenCharacters();
        sStart = aForbidden.beginLine;
        sEnd = aForbidden.endLine;
    }
    m_xStandardCB->set_active(!bAvail);
    m_xStartED->set_sensitive(bAvail);
    m_xEndED->set_sensitive(bAvail);
    m_xStartFT->set_sensitive(bAvail);
    m_xEndFT->set_sensitive(bAvail);
    m_xStartED->set_text(sStart);
    m_xEndED->set_text(sEnd);
}

IMPL_LINK(SvxAsianLayoutPage, ChangeStandardHdl, weld::Toggleable&, rBox, void)
{
    bool bCheck = rBox.get_active();
    m_xStartED->set_sensitive(!bCheck);
    m_xEndED->set_sensitive(!bCheck);
    m_xStartFT->set_sensitive(!bCheck);
    m_xEndFT->set_sensitive(!bCheck);

    ModifyHdl(*m_xStartED);
}

IMPL_LINK(SvxAsianLayoutPage, ModifyHdl, weld::Entry&, rEdit, void)
{
    LanguageType eSelectLanguage = m_xLanguageLB->get_active_id();
    Locale aLocale( LanguageTag::convertToLocale( eSelectLanguage ));
    OUString sStart = m_xStartED->get_text();
    OUString sEnd = m_xEndED->get_text();
    bool bEnable = rEdit.get_sensitive();
    if(pImpl->xForbidden.is())
    {
        try
        {
            if(bEnable)
            {
                ForbiddenCharacters aFCSet;
                aFCSet.beginLine = sStart;
                aFCSet.endLine = sEnd;
                pImpl->addForbiddenCharacters(eSelectLanguage, std::move(aFCSet));
            }
            else
                pImpl->addForbiddenCharacters(eSelectLanguage, std::nullopt);
        }
        catch (const Exception&)
        {
            TOOLS_WARN_EXCEPTION( "cui.options", "in XForbiddenCharacters");
        }
    }
    pImpl->aConfig.SetStartEndChars( aLocale, bEnable ? &sStart : nullptr, bEnable ? &sEnd : nullptr);
}

WhichRangesContainer SvxAsianLayoutPage::GetRanges()
{
    //no items are used
    return WhichRangesContainer();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
