/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <optasian.hxx>
#include <editeng/langitem.hxx>
#include <editeng/unolingu.hxx>
#include <optasian.hrc>
#include <dialmgr.hxx>
#include <cuires.hrc>
#include <tools/table.hxx>
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
    sal_Bool                bRemoved;
    ForbiddenCharacters*    pCharacters;
};

DECLARE_TABLE( _SvxForbiddenCharacterTable_Impl, SvxForbiddenChars_Impl* )

class SvxForbiddenCharacterTable_Impl : public _SvxForbiddenCharacterTable_Impl
{
public:
    SvxForbiddenCharacterTable_Impl()
        : _SvxForbiddenCharacterTable_Impl( 4, 4 )
    {}
    ~SvxForbiddenCharacterTable_Impl();
};

struct SvxAsianLayoutPage_Impl
{
    SvxAsianConfig  aConfig;
        SvxAsianLayoutPage_Impl() :
            aConfig(sal_False){}

    Reference< XForbiddenCharacters >   xForbidden;
    Reference< XPropertySet >           xPrSet;
    Reference< XPropertySetInfo >       xPrSetInfo;
    SvxForbiddenCharacterTable_Impl     aChangedLanguagesTbl;

    sal_Bool                hasForbiddenCharacters(LanguageType eLang);
    SvxForbiddenChars_Impl* getForbiddenCharacters(LanguageType eLang);
    void                    addForbiddenCharacters(LanguageType eLang, ForbiddenCharacters* pForbidden);
};

SvxForbiddenCharacterTable_Impl::~SvxForbiddenCharacterTable_Impl()
{
    for( SvxForbiddenChars_Impl*  pDel = First(); pDel; pDel = Next() )
    {
        delete pDel->pCharacters;
        delete pDel;
    }
}

sal_Bool    SvxAsianLayoutPage_Impl::hasForbiddenCharacters(LanguageType eLang)
{
    return 0 != aChangedLanguagesTbl.Get(eLang);
}

SvxForbiddenChars_Impl* SvxAsianLayoutPage_Impl::getForbiddenCharacters(LanguageType eLang)
{
    SvxForbiddenChars_Impl* pImp = aChangedLanguagesTbl.Get(eLang);
    DBG_ASSERT(pImp, "language not available");
    if(pImp)
        return pImp;
    return 0;
}

void SvxAsianLayoutPage_Impl::addForbiddenCharacters(
    LanguageType eLang, ForbiddenCharacters* pForbidden)
{
    SvxForbiddenChars_Impl* pOld = aChangedLanguagesTbl.Get(eLang);
    if( !pOld )
    {
        pOld = new SvxForbiddenChars_Impl;
        pOld->bRemoved = 0 == pForbidden;
        pOld->pCharacters = pForbidden ? new ForbiddenCharacters(*pForbidden) : 0;
        aChangedLanguagesTbl.Insert( eLang, pOld );
    }
    else
    {
        pOld->bRemoved = 0 == pForbidden;
        delete pOld->pCharacters;
        pOld->pCharacters = pForbidden ? new ForbiddenCharacters(*pForbidden) : 0;
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

    aLanguageLB.SetLanguageList( LANG_LIST_FBD_CHARS, FALSE, FALSE );
}

SvxAsianLayoutPage::~SvxAsianLayoutPage()
{
    delete pImpl;
}

SfxTabPage* SvxAsianLayoutPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxAsianLayoutPage(pParent, rAttrSet);
}

BOOL SvxAsianLayoutPage::FillItemSet( SfxItemSet& )
{
    if(aCharKerningRB.IsChecked() != aCharKerningRB.GetSavedValue())
    {
        pImpl->aConfig.SetKerningWesternTextOnly(aCharKerningRB.IsChecked());
        OUString sPunct(C2U(cIsKernAsianPunctuation));
        if(pImpl->xPrSetInfo.is() && pImpl->xPrSetInfo->hasPropertyByName(sPunct))
        {
            Any aVal;
            BOOL bVal = !aCharKerningRB.IsChecked();
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
    if(pImpl->aConfig.IsModified())
        pImpl->aConfig.Commit();
    if(pImpl->xForbidden.is())
    {
        try
        {
            for( SvxForbiddenChars_Impl*  pElem = pImpl->aChangedLanguagesTbl.First();
                pElem; pElem = pImpl->aChangedLanguagesTbl.Next() )
            {
                ULONG nLang = pImpl->aChangedLanguagesTbl.GetKey( pElem );
                Locale aLocale;
                SvxLanguageToLocale(aLocale, (USHORT)nLang );
                if(pElem->bRemoved)
                    pImpl->xForbidden->removeForbiddenCharacters( aLocale );
                else if(pElem->pCharacters)
                    pImpl->xForbidden->setForbiddenCharacters( aLocale, *pElem->pCharacters );
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("exception in XForbiddenCharacters");
        }
    }
    eLastUsedLanguageTypeForForbiddenCharacters = aLanguageLB.GetSelectLanguage();

    return FALSE;
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
        aStartEndGB.Enable(FALSE);
        aLanguageFT.Enable(FALSE);
        aLanguageLB.Enable(FALSE);
        aStandardCB.Enable(FALSE);
        aStartFT.Enable(FALSE);
        aStartED.Enable(FALSE);
        aEndFT.Enable(FALSE);
        aEndED.Enable(FALSE);
        aHintFT.Enable(FALSE);
    }
    if(bKernWesternText)
        aCharKerningRB.Check(TRUE);
    else
        aCharPunctKerningRB.Check(TRUE);
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
        switch(eLastUsedLanguageTypeForForbiddenCharacters)
        {
            case  LANGUAGE_CHINESE            :
            case  LANGUAGE_CHINESE_SINGAPORE  :
                eLastUsedLanguageTypeForForbiddenCharacters = LANGUAGE_CHINESE_SIMPLIFIED;
            break;
            case  LANGUAGE_CHINESE_HONGKONG   :
            case  LANGUAGE_CHINESE_MACAU      :
                eLastUsedLanguageTypeForForbiddenCharacters = LANGUAGE_CHINESE_TRADITIONAL;
            break;
        }
    }
    aLanguageLB.SelectLanguage( eLastUsedLanguageTypeForForbiddenCharacters );
    LanguageHdl(&aLanguageLB);
}

IMPL_LINK(SvxAsianLayoutPage, LanguageHdl, SvxLanguageBox*, EMPTYARG )
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
                bAvail = FALSE;
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
            catch(Exception&)
            {
                DBG_ERROR("exception in XForbiddenCharacters");
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
        catch(Exception&)
        {
            DBG_ERROR("exception in XForbiddenCharacters");
        }
    }
    pImpl->aConfig.SetStartEndChars( aLocale, bEnable ? &sStart : 0, bEnable ? &sEnd : 0);
    return 0;
}

USHORT* SvxAsianLayoutPage::GetRanges()
{
    //no items are used
    static USHORT pAsianLayoutRanges[] = { 0 };
    return pAsianLayoutRanges;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
