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

#include <map>
#include <optasian.hxx>
#include <editeng/langitem.hxx>
#include <editeng/unolingu.hxx>
#include <optasian.hrc>
#include <dialmgr.hxx>
#include <cuires.hrc>
#include <i18npool/mslangid.hxx>
#include <tools/shl.hxx>
#include <svl/asiancfg.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localedatawrapper.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using rtl::OUString;

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)

const sal_Char cIsKernAsianPunctuation[] = "IsKernAsianPunctuation";
const sal_Char cCharacterCompressionType[] = "CharacterCompressionType";

struct SvxForbiddenChars_Impl
{
    ~SvxForbiddenChars_Impl();

    sal_Bool                bRemoved;
    ForbiddenCharacters*    pCharacters;
};

SvxForbiddenChars_Impl::~SvxForbiddenChars_Impl()
{
    delete pCharacters;
}

typedef ::std::map< LanguageType, SvxForbiddenChars_Impl* > SvxForbiddenCharacterMap_Impl;

struct SvxAsianLayoutPage_Impl
{
    SvxAsianConfig  aConfig;
        SvxAsianLayoutPage_Impl() {}

    ~SvxAsianLayoutPage_Impl();

    Reference< XForbiddenCharacters >   xForbidden;
    Reference< XPropertySet >           xPrSet;
    Reference< XPropertySetInfo >       xPrSetInfo;
    SvxForbiddenCharacterMap_Impl       aChangedLanguagesMap;

    sal_Bool                hasForbiddenCharacters(LanguageType eLang);
    SvxForbiddenChars_Impl* getForbiddenCharacters(LanguageType eLang);
    void                    addForbiddenCharacters(LanguageType eLang, ForbiddenCharacters* pForbidden);
};

SvxAsianLayoutPage_Impl::~SvxAsianLayoutPage_Impl()
{
    SvxForbiddenCharacterMap_Impl::iterator it;
    for( it = aChangedLanguagesMap.begin(); it != aChangedLanguagesMap.end(); ++it )
    {
        delete it->second;
    }
}

sal_Bool    SvxAsianLayoutPage_Impl::hasForbiddenCharacters(LanguageType eLang)
{
    return aChangedLanguagesMap.count( eLang );
}

SvxForbiddenChars_Impl* SvxAsianLayoutPage_Impl::getForbiddenCharacters(LanguageType eLang)
{
    SvxForbiddenCharacterMap_Impl::iterator it = aChangedLanguagesMap.find( eLang );
    DBG_ASSERT( ( it != aChangedLanguagesMap.end() ), "language not available");
    if( it != aChangedLanguagesMap.end() )
        return it->second;
    return 0;
}

void SvxAsianLayoutPage_Impl::addForbiddenCharacters(
    LanguageType eLang, ForbiddenCharacters* pForbidden)
{
    SvxForbiddenCharacterMap_Impl::iterator itOld = aChangedLanguagesMap.find( eLang );
    if( itOld == aChangedLanguagesMap.end() )
    {
        SvxForbiddenChars_Impl* pChar = new SvxForbiddenChars_Impl;
        pChar->bRemoved = 0 == pForbidden;
        pChar->pCharacters = pForbidden ? new ForbiddenCharacters(*pForbidden) : 0;
        aChangedLanguagesMap.insert( ::std::make_pair( eLang, pChar ) );
    }
    else
    {
        itOld->second->bRemoved = 0 == pForbidden;
        delete itOld->second->pCharacters;
        itOld->second->pCharacters = pForbidden ? new ForbiddenCharacters(*pForbidden) : 0;
    }
}

static LanguageType eLastUsedLanguageTypeForForbiddenCharacters = USHRT_MAX;

SvxAsianLayoutPage::SvxAsianLayoutPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, CUI_RES( RID_SVXPAGE_ASIAN_LAYOUT ), rSet),
    aKerningGB(             this, CUI_RES(GB_KERNING            )),
    aCharKerningRB(         this, CUI_RES(RB_CHAR_KERNING       )),
    aCharPunctKerningRB(    this, CUI_RES(RB_CHAR_PUNCT     )),
    aCharDistGB(            this, CUI_RES(GB_CHAR_DIST      )),
    aNoCompressionRB(       this, CUI_RES(RB_NO_COMP            )),
    aPunctCompressionRB(    this, CUI_RES(RB_PUNCT_COMP     )),
     aPunctKanaCompressionRB(this, CUI_RES(RB_PUNCT_KANA_COMP   )),
    aStartEndGB(            this, CUI_RES(GB_START_END      )),
    aLanguageFT(            this, CUI_RES(FT_LANGUAGE           )),
    aLanguageLB(            this, CUI_RES(LB_LANGUAGE           )),
    aStandardCB(            this, CUI_RES(CB_STANDARD           )),
    aStartFT(               this, CUI_RES(FT_START          )),
    aStartED(               this, CUI_RES(ED_START          )),
    aEndFT(                 this, CUI_RES(FT_END                )),
    aEndED(                 this, CUI_RES(ED_END                )),
    aHintFT(                this, CUI_RES(FT_HINT               )),
    pImpl(new SvxAsianLayoutPage_Impl)
{
    FreeResource();
    LanguageHdl(&aLanguageLB);
    aLanguageLB.SetSelectHdl(LINK(this, SvxAsianLayoutPage, LanguageHdl));
    aStandardCB.SetClickHdl(LINK(this, SvxAsianLayoutPage, ChangeStandardHdl));
    Link aLk(LINK(this, SvxAsianLayoutPage, ModifyHdl));
    aStartED.SetModifyHdl(aLk);
    aEndED.SetModifyHdl(aLk);

    aLanguageLB.SetLanguageList( LANG_LIST_FBD_CHARS, sal_False, sal_False );
}

SvxAsianLayoutPage::~SvxAsianLayoutPage()
{
    delete pImpl;
}

SfxTabPage* SvxAsianLayoutPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxAsianLayoutPage(pParent, rAttrSet);
}

sal_Bool SvxAsianLayoutPage::FillItemSet( SfxItemSet& )
{
    if(aCharKerningRB.IsChecked() != aCharKerningRB.GetSavedValue())
    {
        pImpl->aConfig.SetKerningWesternTextOnly(aCharKerningRB.IsChecked());
        OUString sPunct(C2U(cIsKernAsianPunctuation));
        if(pImpl->xPrSetInfo.is() && pImpl->xPrSetInfo->hasPropertyByName(sPunct))
        {
            Any aVal;
            sal_Bool bVal = !aCharKerningRB.IsChecked();
            aVal.setValue(&bVal, ::getBooleanCppuType());
            pImpl->xPrSet->setPropertyValue(sPunct, aVal);
        }
    }

    if(aNoCompressionRB.IsChecked() != aNoCompressionRB.GetSavedValue() ||
            aPunctCompressionRB.IsChecked() != aPunctCompressionRB.GetSavedValue())
    {
        sal_Int16 nSet = aNoCompressionRB.IsChecked() ? 0 :
                            aPunctCompressionRB.IsChecked() ? 1 : 2;
        pImpl->aConfig.SetCharDistanceCompression(nSet);
        OUString sCompress(C2U(cCharacterCompressionType));
        if(pImpl->xPrSetInfo.is() && pImpl->xPrSetInfo->hasPropertyByName(sCompress))
        {
            Any aVal;
            aVal <<= nSet;
            pImpl->xPrSet->setPropertyValue(sCompress, aVal);
        }
    }
    pImpl->aConfig.Commit();
    if(pImpl->xForbidden.is())
    {
        try
        {
            SvxForbiddenCharacterMap_Impl::iterator itElem;
            for( itElem = pImpl->aChangedLanguagesMap.begin();
                itElem != pImpl->aChangedLanguagesMap.end(); ++itElem )
            {
                Locale aLocale;
                SvxLanguageToLocale( aLocale, itElem->first );
                if(itElem->second->bRemoved)
                    pImpl->xForbidden->removeForbiddenCharacters( aLocale );
                else if(itElem->second->pCharacters)
                    pImpl->xForbidden->setForbiddenCharacters( aLocale, *( itElem->second->pCharacters ) );
            }
        }
        catch (const Exception&)
        {
            OSL_FAIL("exception in XForbiddenCharacters");
        }
    }
    eLastUsedLanguageTypeForForbiddenCharacters = aLanguageLB.GetSelectLanguage();

    return sal_False;
}

void SvxAsianLayoutPage::Reset( const SfxItemSet& )
{
    SfxViewFrame* pCurFrm = SfxViewFrame::Current();
    SfxObjectShell* pDocSh = pCurFrm ? pCurFrm->GetObjectShell() : 0;
    Reference< XModel > xModel;
    if(pDocSh)
        xModel = pDocSh->GetModel();
    Reference<XMultiServiceFactory> xFact(xModel, UNO_QUERY);
    if(xFact.is())
    {
        pImpl->xPrSet = Reference<XPropertySet>(
            xFact->createInstance(C2U("com.sun.star.document.Settings")), UNO_QUERY);
    }
    if( pImpl->xPrSet.is() )
        pImpl->xPrSetInfo = pImpl->xPrSet->getPropertySetInfo();
    OUString sForbidden(C2U("ForbiddenCharacters"));
    sal_Bool bKernWesternText = pImpl->aConfig.IsKerningWesternTextOnly();
    sal_Int16 nCompress = pImpl->aConfig.GetCharDistanceCompression();
    if(pImpl->xPrSetInfo.is())
    {
        if(pImpl->xPrSetInfo->hasPropertyByName(sForbidden))
        {
            Any aForbidden = pImpl->xPrSet->getPropertyValue(sForbidden);
            aForbidden >>= pImpl->xForbidden;
        }
        OUString sCompress(C2U(cCharacterCompressionType));
        if(pImpl->xPrSetInfo->hasPropertyByName(sCompress))
        {
            Any aVal = pImpl->xPrSet->getPropertyValue(sCompress);
            aVal >>= nCompress;
        }
        OUString sPunct(C2U(cIsKernAsianPunctuation));
        if(pImpl->xPrSetInfo->hasPropertyByName(sPunct))
        {
            Any aVal = pImpl->xPrSet->getPropertyValue(sPunct);
            bKernWesternText = !*(sal_Bool*)aVal.getValue();
        }
    }
    else
    {
        aStartEndGB.Enable(sal_False);
        aLanguageFT.Enable(sal_False);
        aLanguageLB.Enable(sal_False);
        aStandardCB.Enable(sal_False);
        aStartFT.Enable(sal_False);
        aStartED.Enable(sal_False);
        aEndFT.Enable(sal_False);
        aEndED.Enable(sal_False);
        aHintFT.Enable(sal_False);
    }
    if(bKernWesternText)
        aCharKerningRB.Check(sal_True);
    else
        aCharPunctKerningRB.Check(sal_True);
    switch(nCompress)
    {
        case 0 : aNoCompressionRB.Check();        break;
        case 1 : aPunctCompressionRB.Check();     break;
        default: aPunctKanaCompressionRB.Check();
    }
    aCharKerningRB.SaveValue();
    aNoCompressionRB.SaveValue();
    aPunctCompressionRB.SaveValue();
    aPunctKanaCompressionRB.SaveValue();

    aLanguageLB.SelectEntryPos(0);
    //preselect the system language in the box - if available
    if(USHRT_MAX == eLastUsedLanguageTypeForForbiddenCharacters)
    {
        eLastUsedLanguageTypeForForbiddenCharacters = SvxLocaleToLanguage(
            Application::GetSettings().GetLocale() );
        if (MsLangId::isSimplifiedChinese(eLastUsedLanguageTypeForForbiddenCharacters))
            eLastUsedLanguageTypeForForbiddenCharacters = LANGUAGE_CHINESE_SIMPLIFIED;
        else if (MsLangId::isTraditionalChinese(eLastUsedLanguageTypeForForbiddenCharacters))
            eLastUsedLanguageTypeForForbiddenCharacters = LANGUAGE_CHINESE_TRADITIONAL;
    }
    aLanguageLB.SelectLanguage( eLastUsedLanguageTypeForForbiddenCharacters );
    LanguageHdl(&aLanguageLB);
}

IMPL_LINK_NOARG(SvxAsianLayoutPage, LanguageHdl)
{
    //set current value
    Locale aLocale;
    LanguageType eSelectLanguage = aLanguageLB.GetSelectLanguage();
    SvxLanguageToLocale(aLocale, eSelectLanguage );

    OUString sStart, sEnd;
    sal_Bool bAvail;
    if(pImpl->xForbidden.is())
    {
        bAvail = pImpl->hasForbiddenCharacters(eSelectLanguage);
        if(bAvail)
        {
            SvxForbiddenChars_Impl* pElement = pImpl->getForbiddenCharacters(eSelectLanguage);
            if(pElement->bRemoved || !pElement->pCharacters)
            {
                bAvail = sal_False;
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
        Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        LocaleDataWrapper aWrap( xMSF, aLocale );
        ForbiddenCharacters aForbidden = aWrap.getForbiddenCharacters();
        sStart = aForbidden.beginLine;
        sEnd = aForbidden.endLine;
    }
    aStandardCB.Check(!bAvail);
    aStartED.Enable(bAvail);
    aEndED.Enable(bAvail);
    aStartFT.Enable(bAvail);
    aEndFT.Enable(bAvail);
    aStartED.SetText(sStart);
    aEndED.SetText(sEnd);

    return 0;
}

IMPL_LINK(SvxAsianLayoutPage, ChangeStandardHdl, CheckBox*, pBox)
{
    sal_Bool bCheck = pBox->IsChecked();
    aStartED.Enable(!bCheck);
    aEndED.Enable(!bCheck);
    aStartFT.Enable(!bCheck);
    aEndFT.Enable(!bCheck);

    ModifyHdl(&aStartED);
    return 0;
}

IMPL_LINK(SvxAsianLayoutPage, ModifyHdl, Edit*, pEdit)
{
    Locale aLocale;
    LanguageType eSelectLanguage = aLanguageLB.GetSelectLanguage();
    SvxLanguageToLocale(aLocale, eSelectLanguage );
    OUString sStart = aStartED.GetText();
    OUString sEnd = aEndED.GetText();
    sal_Bool bEnable = pEdit->IsEnabled();
    if(pImpl->xForbidden.is())
    {
        try
        {
            if(bEnable)
            {
                ForbiddenCharacters aSet;
                aSet.beginLine = sStart;
                aSet.endLine = sEnd;
                pImpl->addForbiddenCharacters(eSelectLanguage, &aSet);
            }
            else
                pImpl->addForbiddenCharacters(eSelectLanguage, 0);
        }
        catch (const Exception&)
        {
            OSL_FAIL("exception in XForbiddenCharacters");
        }
    }
    pImpl->aConfig.SetStartEndChars( aLocale, bEnable ? &sStart : 0, bEnable ? &sEnd : 0);
    return 0;
}

sal_uInt16* SvxAsianLayoutPage::GetRanges()
{
    //no items are used
    static sal_uInt16 pAsianLayoutRanges[] = { 0 };
    return pAsianLayoutRanges;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
