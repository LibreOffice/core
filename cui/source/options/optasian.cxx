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
#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <editeng/langitem.hxx>
#include <editeng/unolingu.hxx>
#include <o3tl/any.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <svl/asiancfg.hxx>
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

const sal_Char cIsKernAsianPunctuation[] = "IsKernAsianPunctuation";
const sal_Char cCharacterCompressionType[] = "CharacterCompressionType";

struct SvxForbiddenChars_Impl
{
    bool                                  bRemoved;
    std::unique_ptr<ForbiddenCharacters>  pCharacters;
};

struct SvxAsianLayoutPage_Impl
{
    SvxAsianConfig  aConfig;
    SvxAsianLayoutPage_Impl() {}

    Reference< XForbiddenCharacters >   xForbidden;
    Reference< XPropertySet >           xPrSet;
    Reference< XPropertySetInfo >       xPrSetInfo;
    std::map< LanguageType, std::unique_ptr<SvxForbiddenChars_Impl> >
                                        aChangedLanguagesMap;

    bool                hasForbiddenCharacters(LanguageType eLang);
    SvxForbiddenChars_Impl* getForbiddenCharacters(LanguageType eLang);
    void                    addForbiddenCharacters(LanguageType eLang, std::unique_ptr<ForbiddenCharacters> pForbidden);
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
        return it->second.get();
    return nullptr;
}

void SvxAsianLayoutPage_Impl::addForbiddenCharacters(
    LanguageType eLang, std::unique_ptr<ForbiddenCharacters> pForbidden)
{
    auto itOld = aChangedLanguagesMap.find( eLang );
    if( itOld == aChangedLanguagesMap.end() )
    {
        std::unique_ptr<SvxForbiddenChars_Impl> pChar(new SvxForbiddenChars_Impl);
        pChar->bRemoved = nullptr == pForbidden;
        pChar->pCharacters = std::move(pForbidden);
        aChangedLanguagesMap.emplace( eLang, std::move(pChar) );
    }
    else
    {
        itOld->second->bRemoved = nullptr == pForbidden;
        itOld->second->pCharacters = std::move(pForbidden);
    }
}

static LanguageType eLastUsedLanguageTypeForForbiddenCharacters(USHRT_MAX);

SvxAsianLayoutPage::SvxAsianLayoutPage( vcl::Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, "OptAsianPage", "cui/ui/optasianpage.ui", &rSet),
    pImpl(new SvxAsianLayoutPage_Impl)
{
    get(m_pCharKerningRB, "charkerning");
    get(m_pCharPunctKerningRB, "charpunctkerning");
    get(m_pNoCompressionRB, "nocompression");
    get(m_pPunctCompressionRB, "punctcompression");
    get(m_pPunctKanaCompressionRB, "punctkanacompression");
    get(m_pLanguageFT, "languageft");
    get(m_pLanguageLB, "language");
    get(m_pStandardCB, "standard");
    get(m_pStartFT, "startft");
    get(m_pStartED, "start");
    get(m_pEndFT, "endft");
    get(m_pEndED, "end");
    get(m_pHintFT, "hintft");

    LanguageHdl(*m_pLanguageLB);
    m_pLanguageLB->SetSelectHdl(LINK(this, SvxAsianLayoutPage, LanguageHdl));
    m_pStandardCB->SetClickHdl(LINK(this, SvxAsianLayoutPage, ChangeStandardHdl));
    Link<Edit&,void> aLk(LINK(this, SvxAsianLayoutPage, ModifyHdl));
    m_pStartED->SetModifyHdl(aLk);
    m_pEndED->SetModifyHdl(aLk);

    m_pLanguageLB->SetLanguageList( SvxLanguageListFlags::FBD_CHARS, false, false );
}

SvxAsianLayoutPage::~SvxAsianLayoutPage()
{
    disposeOnce();
}

void SvxAsianLayoutPage::dispose()
{
    pImpl.reset();
    m_pCharKerningRB.clear();
    m_pCharPunctKerningRB.clear();
    m_pNoCompressionRB.clear();
    m_pPunctCompressionRB.clear();
    m_pPunctKanaCompressionRB.clear();
    m_pLanguageFT.clear();
    m_pLanguageLB.clear();
    m_pStandardCB.clear();
    m_pStartFT.clear();
    m_pStartED.clear();
    m_pEndFT.clear();
    m_pEndED.clear();
    m_pHintFT.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxAsianLayoutPage::Create( TabPageParent pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxAsianLayoutPage>::Create(pParent.pParent, *rAttrSet);
}

bool SvxAsianLayoutPage::FillItemSet( SfxItemSet* )
{
    if(m_pCharKerningRB->IsValueChangedFromSaved())
    {
        pImpl->aConfig.SetKerningWesternTextOnly(m_pCharKerningRB->IsChecked());
        OUString sPunct(cIsKernAsianPunctuation);
        if(pImpl->xPrSetInfo.is() && pImpl->xPrSetInfo->hasPropertyByName(sPunct))
        {
            bool bVal = !m_pCharKerningRB->IsChecked();
            pImpl->xPrSet->setPropertyValue(sPunct, Any(bVal));
        }
    }

    if(m_pNoCompressionRB->IsValueChangedFromSaved() ||
       m_pPunctCompressionRB->IsValueChangedFromSaved())
    {
        CharCompressType nSet = m_pNoCompressionRB->IsChecked() ? CharCompressType::NONE :
                            m_pPunctCompressionRB->IsChecked() ? CharCompressType::PunctuationOnly :
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
                if(changedLanguage.second->bRemoved)
                    pImpl->xForbidden->removeForbiddenCharacters( aLocale );
                else if(changedLanguage.second->pCharacters)
                    pImpl->xForbidden->setForbiddenCharacters( aLocale, *( changedLanguage.second->pCharacters ) );
            }
        }
        catch (const Exception&)
        {
            OSL_FAIL("exception in XForbiddenCharacters");
        }
    }
    eLastUsedLanguageTypeForForbiddenCharacters = m_pLanguageLB->GetSelectedLanguage();

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
    OUString sForbidden("ForbiddenCharacters");
    bool bKernWesternText = pImpl->aConfig.IsKerningWesternTextOnly();
    CharCompressType nCompress = pImpl->aConfig.GetCharDistanceCompression();
    if(pImpl->xPrSetInfo.is())
    {
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
        m_pLanguageFT->Enable(false);
        m_pLanguageLB->Enable(false);
        m_pStandardCB->Enable(false);
        m_pStartFT->Enable(false);
        m_pStartED->Enable(false);
        m_pEndFT->Enable(false);
        m_pEndED->Enable(false);
        m_pHintFT->Enable(false);
    }
    if(bKernWesternText)
        m_pCharKerningRB->Check();
    else
        m_pCharPunctKerningRB->Check();
    switch(nCompress)
    {
        case CharCompressType::NONE : m_pNoCompressionRB->Check();        break;
        case CharCompressType::PunctuationOnly : m_pPunctCompressionRB->Check();     break;
        default: m_pPunctKanaCompressionRB->Check();
    }
    m_pCharKerningRB->SaveValue();
    m_pNoCompressionRB->SaveValue();
    m_pPunctCompressionRB->SaveValue();
    m_pPunctKanaCompressionRB->SaveValue();

    m_pLanguageLB->SelectEntryPos(0);
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
    m_pLanguageLB->SelectLanguage( eLastUsedLanguageTypeForForbiddenCharacters );
    LanguageHdl(*m_pLanguageLB);
}

IMPL_LINK_NOARG(SvxAsianLayoutPage, LanguageHdl, ListBox&, void)
{
    //set current value
    LanguageType eSelectLanguage = m_pLanguageLB->GetSelectedLanguage();
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
            if(pElement->bRemoved || !pElement->pCharacters)
            {
                bAvail = false;
            }
            else
            {
                sStart = pElement->pCharacters->beginLine;
                sEnd = pElement->pCharacters->endLine;
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
                OSL_FAIL("exception in XForbiddenCharacters");
            }
        }
    }
    else
    {
        bAvail = pImpl->aConfig.GetStartEndChars( aLocale, sStart, sEnd );
    }
    if(!bAvail)
    {
        LocaleDataWrapper aWrap( aLanguageTag );
        ForbiddenCharacters aForbidden = aWrap.getForbiddenCharacters();
        sStart = aForbidden.beginLine;
        sEnd = aForbidden.endLine;
    }
    m_pStandardCB->Check(!bAvail);
    m_pStartED->Enable(bAvail);
    m_pEndED->Enable(bAvail);
    m_pStartFT->Enable(bAvail);
    m_pEndFT->Enable(bAvail);
    m_pStartED->SetText(sStart);
    m_pEndED->SetText(sEnd);
}

IMPL_LINK(SvxAsianLayoutPage, ChangeStandardHdl, Button*, pBox, void)
{
    bool bCheck = static_cast<CheckBox*>(pBox)->IsChecked();
    m_pStartED->Enable(!bCheck);
    m_pEndED->Enable(!bCheck);
    m_pStartFT->Enable(!bCheck);
    m_pEndFT->Enable(!bCheck);

    ModifyHdl(*m_pStartED);
}

IMPL_LINK(SvxAsianLayoutPage, ModifyHdl, Edit&, rEdit, void)
{
    LanguageType eSelectLanguage = m_pLanguageLB->GetSelectedLanguage();
    Locale aLocale( LanguageTag::convertToLocale( eSelectLanguage ));
    OUString sStart = m_pStartED->GetText();
    OUString sEnd = m_pEndED->GetText();
    bool bEnable = rEdit.IsEnabled();
    if(pImpl->xForbidden.is())
    {
        try
        {
            if(bEnable)
            {
                std::unique_ptr<ForbiddenCharacters> pFCSet(new ForbiddenCharacters);
                pFCSet->beginLine = sStart;
                pFCSet->endLine = sEnd;
                pImpl->addForbiddenCharacters(eSelectLanguage, std::move(pFCSet));
            }
            else
                pImpl->addForbiddenCharacters(eSelectLanguage, nullptr);
        }
        catch (const Exception&)
        {
            OSL_FAIL("exception in XForbiddenCharacters");
        }
    }
    pImpl->aConfig.SetStartEndChars( aLocale, bEnable ? &sStart : nullptr, bEnable ? &sEnd : nullptr);
}

const sal_uInt16* SvxAsianLayoutPage::GetRanges()
{
    //no items are used
    static const sal_uInt16 pAsianLayoutRanges[] = { 0 };
    return pAsianLayoutRanges;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
