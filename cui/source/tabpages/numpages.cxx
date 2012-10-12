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

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>

#include <numpages.hxx>
#include <numpages.hrc>
#include <dialmgr.hxx>
#include <tools/shl.hxx>
#include <i18npool/mslangid.hxx>
#include <helpid.hrc>
#include <editeng/numitem.hxx>
#include <svl/eitem.hxx>
#include <vcl/svapp.hxx>
#include <svx/gallery.hxx>
#include <svl/urihelper.hxx>
#include <editeng/brshitem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/graph.hxx>
#include <vcl/msgbox.hxx>
#include "cuicharmap.hxx"
#include <editeng/flstitem.hxx>
#include <svx/dlgutil.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <svx/numvset.hxx>
#include <svx/htmlmode.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/ctrltool.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <comphelper/componentcontext.hxx>

#include <algorithm>
#include <vector>
#include "sfx2/opengrf.hxx"

#include <cuires.hrc>
#include <sfx2/request.hxx>
#include <svl/aeitem.hxx>
#include <svl/stritem.hxx>
#include <svl/slstitm.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;
using namespace com::sun::star::text;
using namespace com::sun::star::container;
using namespace com::sun::star::style;
using rtl::OUString;

#define NUM_PAGETYPE_BULLET         0
#define NUM_PAGETYPE_SINGLENUM      1
#define NUM_PAGETYPE_NUM            2
#define NUM_PAGETYPE_BMP            3
#define PAGETYPE_USER_START         10

#define SHOW_NUMBERING              0
#define SHOW_BULLET                 1
#define SHOW_BITMAP                 2

#define MAX_BMP_WIDTH               16
#define MAX_BMP_HEIGHT              16

static sal_Bool bLastRelative =         sal_False;
static const sal_Char cNumberingType[] = "NumberingType";
static const sal_Char cParentNumbering[] = "ParentNumbering";
static const sal_Char cPrefix[] = "Prefix";
static const sal_Char cSuffix[] = "Suffix";
static const sal_Char cBulletChar[] = "BulletChar";
static const sal_Char cBulletFontName[] = "BulletFontName";

static Reference<XDefaultNumberingProvider> lcl_GetNumberingProvider()
{
    Reference<XComponentContext>         xContext( ::comphelper::getProcessComponentContext() );
    Reference<XDefaultNumberingProvider> xRet = text::DefaultNumberingProvider::create(xContext);
    return xRet;
}

static SvxNumSettings_Impl* lcl_CreateNumSettingsPtr(const Sequence<PropertyValue>& rLevelProps)
{
    const PropertyValue* pValues = rLevelProps.getConstArray();
    SvxNumSettings_Impl* pNew = new SvxNumSettings_Impl;
    for(sal_Int32 j = 0; j < rLevelProps.getLength(); j++)
    {
        if ( pValues[j].Name == cNumberingType )
            pValues[j].Value >>= pNew->nNumberType;
        else if ( pValues[j].Name == cPrefix )
            pValues[j].Value >>= pNew->sPrefix;
        else if ( pValues[j].Name == cSuffix )
            pValues[j].Value >>= pNew->sSuffix;
        else if ( pValues[j].Name == cParentNumbering )
            pValues[j].Value >>= pNew->nParentNumbering;
        else if ( pValues[j].Name == cBulletChar )
            pValues[j].Value >>= pNew->sBulletChar;
        else if ( pValues[j].Name == cBulletFontName )
            pValues[j].Value >>= pNew->sBulletFont;
    }
    return pNew;
}

// the selection of bullets from the StarSymbol
static const sal_Unicode aBulletTypes[] =
{
    0x2022,
    0x25cf,
    0xe00c,
    0xe00a,
    0x2794,
    0x27a2,
    0x2717,
    0x2714
};

static sal_Char const aNumChar[] =
{
    'A', //CHARS_UPPER_LETTER
    'a', //CHARS_LOWER_LETTER
    'I', //ROMAN_UPPER
    'i', //ROMAN_LOWER
    '1', //ARABIC
    ' '
};

// Is one of the masked formats set?
static sal_Bool lcl_IsNumFmtSet(SvxNumRule* pNum, sal_uInt16 nLevelMask)
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < SVX_MAX_NUM && !bRet; i++ )
    {
        if(nLevelMask & nMask)
            bRet |= 0 != pNum->Get( i );
        nMask <<= 1 ;
    }
    return bRet;
}

static Font& lcl_GetDefaultBulletFont()
{
    static sal_Bool bInit = 0;
    static Font aDefBulletFont( rtl::OUString("StarSymbol"),
                                String(), Size( 0, 14 ) );
    if(!bInit)
    {
        aDefBulletFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
        aDefBulletFont.SetFamily( FAMILY_DONTKNOW );
        aDefBulletFont.SetPitch( PITCH_DONTKNOW );
        aDefBulletFont.SetWeight( WEIGHT_DONTKNOW );
        aDefBulletFont.SetTransparent( sal_True );
        bInit = sal_True;
    }
    return aDefBulletFont;
}

SvxSingleNumPickTabPage::SvxSingleNumPickTabPage(Window* pParent,
                               const SfxItemSet& rSet)  :
    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_PICK_SINGLE_NUM ), rSet ),
    aValuesFL(      this, CUI_RES(FL_VALUES) ),
    pExamplesVS(    new SvxNumValueSet(this, CUI_RES(VS_VALUES), NUM_PAGETYPE_SINGLENUM )),
    pActNum(0),
    pSaveNum(0),
    nActNumLvl( USHRT_MAX ),
    bModified(sal_False),
    bPreset(sal_False),
    nNumItemId(SID_ATTR_NUMBERING_RULE)
{
    FreeResource();
    SetExchangeSupport();
    pExamplesVS->SetSelectHdl(LINK(this, SvxSingleNumPickTabPage, NumSelectHdl_Impl));
    pExamplesVS->SetDoubleClickHdl(LINK(this, SvxSingleNumPickTabPage, DoubleClickHdl_Impl));
    pExamplesVS->SetHelpId(HID_VALUESET_SINGLENUM );

    Reference<XDefaultNumberingProvider> xDefNum = lcl_GetNumberingProvider();
    if(xDefNum.is())
    {
        Sequence< Sequence< PropertyValue > > aNumberings;
        LanguageType eLang = Application::GetSettings().GetLanguage();
        Locale aLocale = SvxCreateLocale(eLang);
        try
        {
            aNumberings =
                xDefNum->getDefaultContinuousNumberingLevels( aLocale );


            sal_Int32 nLength = aNumberings.getLength() > NUM_VALUSET_COUNT ? NUM_VALUSET_COUNT :aNumberings.getLength();

            const Sequence<PropertyValue>* pValuesArr = aNumberings.getConstArray();
            for(sal_Int32 i = 0; i < nLength; i++)
            {
                SvxNumSettings_Impl* pNew = lcl_CreateNumSettingsPtr(pValuesArr[i]);
                aNumSettingsArr.push_back(pNew);
            }
        }
        catch(Exception&)
        {
        }
        Reference<XNumberingFormatter> xFormat(xDefNum, UNO_QUERY);
        pExamplesVS->SetNumberingSettings(aNumberings, xFormat, aLocale);
    }
}

 SvxSingleNumPickTabPage::~SvxSingleNumPickTabPage()
{
    delete pActNum;
    delete pExamplesVS;
    delete pSaveNum;
}

SfxTabPage*  SvxSingleNumPickTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SvxSingleNumPickTabPage(pParent, rAttrSet);
}

sal_Bool  SvxSingleNumPickTabPage::FillItemSet( SfxItemSet& rSet )
{
    if( (bPreset || bModified) && pSaveNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SvxNumBulletItem( *pSaveNum ), nNumItemId);
        rSet.Put(SfxBoolItem(SID_PARAM_NUM_PRESET, bPreset));
    }

    return bModified;
}

void  SvxSingleNumPickTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    bPreset = sal_False;
    sal_Bool bIsPreset = sal_False;
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    if(pExampleSet)
    {
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_NUM_PRESET, sal_False, &pItem))
            bIsPreset = ((const SfxBoolItem*)pItem)->GetValue();
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_CUR_NUM_LEVEL, sal_False, &pItem))
            nActNumLvl = ((const SfxUInt16Item*)pItem)->GetValue();
    }
    if(SFX_ITEM_SET == rSet.GetItemState(nNumItemId, sal_False, &pItem))
    {
        delete pSaveNum;
        pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());
    }
    if(*pSaveNum != *pActNum)
    {
        *pActNum = *pSaveNum;
        pExamplesVS->SetNoSelection();
    }

    if(pActNum && (!lcl_IsNumFmtSet(pActNum, nActNumLvl) || bIsPreset))
    {
        pExamplesVS->SelectItem(1);
        NumSelectHdl_Impl(pExamplesVS);
        bPreset = sal_True;
    }
    bPreset |= bIsPreset;

    bModified = sal_False;
}

int  SvxSingleNumPickTabPage::DeactivatePage(SfxItemSet *_pSet)
{
    if(_pSet)
        FillItemSet(*_pSet);
    return sal_True;
}

void  SvxSingleNumPickTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;

    // in Draw the item exists as WhichId, in Writer only as SlotId
    SfxItemState eState = rSet.GetItemState(SID_ATTR_NUMBERING_RULE, sal_False, &pItem);
    if(eState != SFX_ITEM_SET)
    {
        nNumItemId = rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        eState = rSet.GetItemState(nNumItemId, sal_False, &pItem);

        if( eState != SFX_ITEM_SET )
        {
            pItem = &static_cast< const SvxNumBulletItem& >( rSet.Get( nNumItemId, sal_True ) );
            eState = SFX_ITEM_SET;
        }
    }
    DBG_ASSERT(eState == SFX_ITEM_SET, "kein Item gefunden!");
    delete pSaveNum;
    pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());

    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
}

IMPL_LINK_NOARG(SvxSingleNumPickTabPage, NumSelectHdl_Impl)
{
    if(pActNum)
    {
        bPreset = sal_False;
        bModified = sal_True;
        sal_uInt16 nIdx = pExamplesVS->GetSelectItemId() - 1;
        DBG_ASSERT(aNumSettingsArr.size() > nIdx, "wrong index");
        if(aNumSettingsArr.size() <= nIdx)
            return 0;
        SvxNumSettings_Impl* _pSet = &aNumSettingsArr[nIdx];
        sal_Int16 eNewType = _pSet->nNumberType;
        const sal_Unicode cLocalPrefix = !_pSet->sPrefix.isEmpty() ? _pSet->sPrefix.getStr()[0] : 0;
        const sal_Unicode cLocalSuffix = !_pSet->sSuffix.isEmpty() ? _pSet->sSuffix.getStr()[0] : 0;

        sal_uInt16 nMask = 1;
        for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                SvxNumberFormat aFmt(pActNum->GetLevel(i));
                aFmt.SetNumberingType(eNewType);
                String aEmptyStr;
                if(cLocalPrefix == ' ')
                    aFmt.SetPrefix( aEmptyStr );
                else
                    aFmt.SetPrefix(_pSet->sPrefix);
                if(cLocalSuffix == ' ')
                    aFmt.SetSuffix( aEmptyStr );
                else
                    aFmt.SetSuffix(_pSet->sSuffix);
                aFmt.SetCharFmtName(sNumCharFmtName);
                // #62069# // #92724#
                aFmt.SetBulletRelSize(100);
                pActNum->SetLevel(i, aFmt);
            }
            nMask <<= 1 ;
        }
    }
    return 0;
}

IMPL_LINK_NOARG(SvxSingleNumPickTabPage, DoubleClickHdl_Impl)
{
    NumSelectHdl_Impl(pExamplesVS);
    OKButton& rOk = GetTabDialog()->GetOKButton();
    rOk.GetClickHdl().Call(&rOk);
    return 0;
}


SvxBulletPickTabPage::SvxBulletPickTabPage(Window* pParent,
                               const SfxItemSet& rSet)  :
    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_PICK_BULLET ), rSet ),
    aValuesFL(      this, CUI_RES(FL_VALUES) ),
    pExamplesVS(    new SvxNumValueSet(this, CUI_RES(VS_VALUES), NUM_PAGETYPE_BULLET )),
    pActNum(0),
    pSaveNum(0),
    nActNumLvl( USHRT_MAX ),
    bModified(sal_False),
    bPreset(sal_False),
    nNumItemId(SID_ATTR_NUMBERING_RULE)
{
    FreeResource();
    SetExchangeSupport();
    pExamplesVS->SetSelectHdl(LINK(this, SvxBulletPickTabPage, NumSelectHdl_Impl));
    pExamplesVS->SetDoubleClickHdl(LINK(this, SvxBulletPickTabPage, DoubleClickHdl_Impl));
    pExamplesVS->SetHelpId(HID_VALUESET_BULLET    );

}

 SvxBulletPickTabPage::~SvxBulletPickTabPage()
{
    delete pActNum;
    delete pExamplesVS;
    delete pSaveNum;
}

SfxTabPage*  SvxBulletPickTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SvxBulletPickTabPage(pParent, rAttrSet);
}

sal_Bool  SvxBulletPickTabPage::FillItemSet( SfxItemSet& rSet )
{
    if( (bPreset || bModified) && pActNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SvxNumBulletItem( *pSaveNum ), nNumItemId);
        rSet.Put(SfxBoolItem(SID_PARAM_NUM_PRESET, bPreset));
    }
    return bModified;
}

void  SvxBulletPickTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    bPreset = sal_False;
    sal_Bool bIsPreset = sal_False;
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    if(pExampleSet)
    {
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_NUM_PRESET, sal_False, &pItem))
            bIsPreset = ((const SfxBoolItem*)pItem)->GetValue();
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_CUR_NUM_LEVEL, sal_False, &pItem))
            nActNumLvl = ((const SfxUInt16Item*)pItem)->GetValue();
    }
    if(SFX_ITEM_SET == rSet.GetItemState(nNumItemId, sal_False, &pItem))
    {
        delete pSaveNum;
        pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());
    }
    if(*pSaveNum != *pActNum)
    {
        *pActNum = *pSaveNum;
        pExamplesVS->SetNoSelection();
    }

    if(pActNum && (!lcl_IsNumFmtSet(pActNum, nActNumLvl) || bIsPreset))
    {
        pExamplesVS->SelectItem(1);
        NumSelectHdl_Impl(pExamplesVS);
        bPreset = sal_True;
    }
    bPreset |= bIsPreset;
    bModified = sal_False;
}

int  SvxBulletPickTabPage::DeactivatePage(SfxItemSet *_pSet)
{
    if(_pSet)
        FillItemSet(*_pSet);
    return sal_True;
}

void  SvxBulletPickTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    // in Draw the item exists as WhichId, in Writer only as SlotId
    SfxItemState eState = rSet.GetItemState(SID_ATTR_NUMBERING_RULE, sal_False, &pItem);
    if(eState != SFX_ITEM_SET)
    {
        nNumItemId = rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        eState = rSet.GetItemState(nNumItemId, sal_False, &pItem);

        if( eState != SFX_ITEM_SET )
        {
            pItem = &static_cast< const SvxNumBulletItem& >( rSet.Get( nNumItemId, sal_True ) );
            eState = SFX_ITEM_SET;
        }

    }
    DBG_ASSERT(eState == SFX_ITEM_SET, "kein Item gefunden!");
    delete pSaveNum;
    pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());

    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
}

IMPL_LINK_NOARG(SvxBulletPickTabPage, NumSelectHdl_Impl)
{
    if(pActNum)
    {
        bPreset = sal_False;
        bModified = sal_True;
        sal_Unicode cChar = aBulletTypes[pExamplesVS->GetSelectItemId() - 1];
        Font& rActBulletFont = lcl_GetDefaultBulletFont();

        sal_uInt16 nMask = 1;
        for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                SvxNumberFormat aFmt(pActNum->GetLevel(i));
                aFmt.SetNumberingType( SVX_NUM_CHAR_SPECIAL );
                // #i93908# clear suffix for bullet lists
                aFmt.SetPrefix(::rtl::OUString());
                aFmt.SetSuffix(::rtl::OUString());
                aFmt.SetBulletFont(&rActBulletFont);
                aFmt.SetBulletChar(cChar );
                aFmt.SetCharFmtName(sBulletCharFmtName);
                // #62069# // #92724#
                aFmt.SetBulletRelSize(45);
                pActNum->SetLevel(i, aFmt);
            }
            nMask <<= 1;
        }
    }

    return 0;
}


IMPL_LINK_NOARG(SvxBulletPickTabPage, DoubleClickHdl_Impl)
{
    NumSelectHdl_Impl(pExamplesVS);
    OKButton& rOk = GetTabDialog()->GetOKButton();
    rOk.GetClickHdl().Call(&rOk);
    return 0;
}


void SvxBulletPickTabPage::PageCreated(SfxAllItemSet aSet)
{

    SFX_ITEMSET_ARG (&aSet,pBulletCharFmt,SfxStringItem,SID_BULLET_CHAR_FMT,sal_False);

    if (pBulletCharFmt)
        SetCharFmtName( pBulletCharFmt->GetValue());


}


SvxNumPickTabPage::SvxNumPickTabPage(Window* pParent,
                               const SfxItemSet& rSet) :
    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_PICK_NUM ), rSet ),
    aValuesFL(      this, CUI_RES(FL_VALUES) ),
    pExamplesVS(    new SvxNumValueSet(this, CUI_RES(VS_VALUES), NUM_PAGETYPE_NUM )),
    pActNum(0),
    pSaveNum(0),
    nActNumLvl( USHRT_MAX ),
    nNumItemId(SID_ATTR_NUMBERING_RULE),
    bModified(sal_False),
    bPreset(sal_False)
{

    FreeResource();

    SetExchangeSupport();

    pExamplesVS->SetSelectHdl(LINK(this, SvxNumPickTabPage, NumSelectHdl_Impl));
    pExamplesVS->SetDoubleClickHdl(LINK(this, SvxNumPickTabPage, DoubleClickHdl_Impl));
    pExamplesVS->SetHelpId(HID_VALUESET_NUM       );

    Reference<XDefaultNumberingProvider> xDefNum = lcl_GetNumberingProvider();
    if(xDefNum.is())
    {
        Sequence<Reference<XIndexAccess> > aOutlineAccess;
        LanguageType eLang = Application::GetSettings().GetLanguage();
        Locale aLocale = SvxCreateLocale(eLang);
        try
        {
            aOutlineAccess = xDefNum->getDefaultOutlineNumberings( aLocale );

            for(sal_Int32 nItem = 0;
                nItem < aOutlineAccess.getLength() && nItem < NUM_VALUSET_COUNT;
                nItem++ )
            {
                SvxNumSettingsArr_Impl& rItemArr = aNumSettingsArrays[ nItem ];

                Reference<XIndexAccess> xLevel = aOutlineAccess.getConstArray()[nItem];
                for(sal_Int32 nLevel = 0; nLevel < xLevel->getCount() && nLevel < 5; nLevel++)
                {
                    Any aValueAny = xLevel->getByIndex(nLevel);
                    Sequence<PropertyValue> aLevelProps;
                    aValueAny >>= aLevelProps;
                    SvxNumSettings_Impl* pNew = lcl_CreateNumSettingsPtr(aLevelProps);
                    rItemArr.push_back( pNew );
                }
            }
        }
        catch(Exception&)
        {
        }
        Reference<XNumberingFormatter> xFormat(xDefNum, UNO_QUERY);
        pExamplesVS->SetOutlineNumberingSettings(aOutlineAccess, xFormat, aLocale);
    }
}

 SvxNumPickTabPage::~SvxNumPickTabPage()
{
    delete pActNum;
    delete pExamplesVS;
    delete pSaveNum;
}

SfxTabPage*  SvxNumPickTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SvxNumPickTabPage(pParent, rAttrSet);
}

sal_Bool  SvxNumPickTabPage::FillItemSet( SfxItemSet& rSet )
{
    if( (bPreset || bModified) && pActNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SvxNumBulletItem( *pSaveNum ), nNumItemId);
        rSet.Put(SfxBoolItem(SID_PARAM_NUM_PRESET, bPreset));
    }
    return bModified;
}

void  SvxNumPickTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    bPreset = sal_False;
    sal_Bool bIsPreset = sal_False;
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    if(pExampleSet)
    {
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_NUM_PRESET, sal_False, &pItem))
            bIsPreset = ((const SfxBoolItem*)pItem)->GetValue();
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_CUR_NUM_LEVEL, sal_False, &pItem))
            nActNumLvl = ((const SfxUInt16Item*)pItem)->GetValue();
    }
    if(SFX_ITEM_SET == rSet.GetItemState(nNumItemId, sal_False, &pItem))
    {
        delete pSaveNum;
        pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());
    }
    if(*pSaveNum != *pActNum)
    {
        *pActNum = *pSaveNum;
        pExamplesVS->SetNoSelection();
    }

    if(pActNum && (!lcl_IsNumFmtSet(pActNum, nActNumLvl) || bIsPreset))
    {
        pExamplesVS->SelectItem(1);
        NumSelectHdl_Impl(pExamplesVS);
        bPreset = sal_True;
    }
    bPreset |= bIsPreset;
    bModified = sal_False;
}

int  SvxNumPickTabPage::DeactivatePage(SfxItemSet *_pSet)
{
    if(_pSet)
        FillItemSet(*_pSet);
    return sal_True;
}

void  SvxNumPickTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    // in Draw the item exists as WhichId, in Writer only as SlotId
    SfxItemState eState = rSet.GetItemState(SID_ATTR_NUMBERING_RULE, sal_False, &pItem);
    if(eState != SFX_ITEM_SET)
    {
        nNumItemId = rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        eState = rSet.GetItemState(nNumItemId, sal_False, &pItem);

        if( eState != SFX_ITEM_SET )
        {
            pItem = &static_cast< const SvxNumBulletItem& >( rSet.Get( nNumItemId, sal_True ) );
            eState = SFX_ITEM_SET;
        }

    }
    DBG_ASSERT(eState == SFX_ITEM_SET, "kein Item gefunden!");
    delete pSaveNum;
    pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());

    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;

}

// all levels are changed here
IMPL_LINK_NOARG(SvxNumPickTabPage, NumSelectHdl_Impl)
{
    if(pActNum)
    {
        bPreset = sal_False;
        bModified = sal_True;

        const FontList*  pList = 0;

        SvxNumSettingsArr_Impl& rItemArr = aNumSettingsArrays[pExamplesVS->GetSelectItemId() - 1];

        Font& rActBulletFont = lcl_GetDefaultBulletFont();
        SvxNumSettings_Impl* pLevelSettings = 0;
        for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(rItemArr.size() > i)
                pLevelSettings = &rItemArr[i];
            if(!pLevelSettings)
                break;
            SvxNumberFormat aFmt(pActNum->GetLevel(i));
            aFmt.SetNumberingType( pLevelSettings->nNumberType );
            sal_uInt16 nUpperLevelOrChar = (sal_uInt16)pLevelSettings->nParentNumbering;
            if(aFmt.GetNumberingType() == SVX_NUM_CHAR_SPECIAL)
            {
                // #i93908# clear suffix for bullet lists
                aFmt.SetPrefix(::rtl::OUString());
                aFmt.SetSuffix(::rtl::OUString());
                if( !pLevelSettings->sBulletFont.isEmpty() &&
                    pLevelSettings->sBulletFont.compareTo(
                            rActBulletFont.GetName()))
                {
                    //search for the font
                    if(!pList)
                    {
                        SfxObjectShell* pCurDocShell = SfxObjectShell::Current();
                        const SvxFontListItem* pFontListItem =
                                (const SvxFontListItem* )pCurDocShell
                                                    ->GetItem( SID_ATTR_CHAR_FONTLIST );
                        pList = pFontListItem ? pFontListItem->GetFontList() : 0;
                    }
                    if(pList && pList->IsAvailable( pLevelSettings->sBulletFont ) )
                    {
                        FontInfo aInfo = pList->Get(
                            pLevelSettings->sBulletFont,WEIGHT_NORMAL, ITALIC_NONE);
                        Font aFont(aInfo);
                        aFmt.SetBulletFont(&aFont);
                    }
                    else
                    {
                        //if it cannot be found then create a new one
                        Font aCreateFont( pLevelSettings->sBulletFont,
                                                String(), Size( 0, 14 ) );
                        aCreateFont.SetCharSet( RTL_TEXTENCODING_DONTKNOW );
                        aCreateFont.SetFamily( FAMILY_DONTKNOW );
                        aCreateFont.SetPitch( PITCH_DONTKNOW );
                        aCreateFont.SetWeight( WEIGHT_DONTKNOW );
                        aCreateFont.SetTransparent( sal_True );
                        aFmt.SetBulletFont( &aCreateFont );
                    }
                }
                else
                    aFmt.SetBulletFont( &rActBulletFont );

                aFmt.SetBulletChar( !pLevelSettings->sBulletChar.isEmpty()
                                        ? pLevelSettings->sBulletChar.getStr()[0]
                                        : 0 );
                aFmt.SetCharFmtName( sBulletCharFmtName );
                // #62069# // #92724#
                aFmt.SetBulletRelSize(45);
            }
            else
            {
                aFmt.SetIncludeUpperLevels(sal::static_int_cast< sal_uInt8 >(0 != nUpperLevelOrChar ? pActNum->GetLevelCount() : 0));
                aFmt.SetCharFmtName(sNumCharFmtName);
                // #62069# // #92724#
                aFmt.SetBulletRelSize(100);
                // #i93908#
                aFmt.SetPrefix(pLevelSettings->sPrefix);
                aFmt.SetSuffix(pLevelSettings->sSuffix);
            }
            pActNum->SetLevel(i, aFmt);
        }
    }
    return 0;
}

IMPL_LINK_NOARG(SvxNumPickTabPage, DoubleClickHdl_Impl)
{
    NumSelectHdl_Impl(pExamplesVS);
    OKButton& rOk = GetTabDialog()->GetOKButton();
    rOk.GetClickHdl().Call(&rOk);
    return 0;
}

void SvxNumPickTabPage::PageCreated(SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pNumCharFmt,SfxStringItem,SID_NUM_CHAR_FMT,sal_False);
    SFX_ITEMSET_ARG (&aSet,pBulletCharFmt,SfxStringItem,SID_BULLET_CHAR_FMT,sal_False);


    if (pNumCharFmt &&pBulletCharFmt)
        SetCharFmtNames( pNumCharFmt->GetValue(),pBulletCharFmt->GetValue());
}


SvxBitmapPickTabPage::SvxBitmapPickTabPage(Window* pParent,
                               const SfxItemSet& rSet) :
    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_PICK_BMP ), rSet ),
    aValuesFL(      this, CUI_RES(FL_VALUES) ),
    pExamplesVS(    new SvxBmpNumValueSet(this, CUI_RES(VS_VALUES)/*, aGrfNames*/ )),
    aErrorText(     this, CUI_RES(FT_ERROR)),
    aLinkedCB(      this, CUI_RES(CB_LINKED)),
    pActNum(0),
    pSaveNum(0),
    nActNumLvl( USHRT_MAX ),
    nNumItemId(SID_ATTR_NUMBERING_RULE),
    bModified(sal_False),
    bPreset(sal_False)
{
    FreeResource();
    SetExchangeSupport();
    eCoreUnit = rSet.GetPool()->GetMetric(rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE));
    pExamplesVS->SetSelectHdl(LINK(this, SvxBitmapPickTabPage, NumSelectHdl_Impl));
    pExamplesVS->SetDoubleClickHdl(LINK(this, SvxBitmapPickTabPage, DoubleClickHdl_Impl));
    aLinkedCB.SetClickHdl(LINK(this, SvxBitmapPickTabPage, LinkBmpHdl_Impl));

    // determine graphic name
    GalleryExplorer::FillObjList(GALLERY_THEME_BULLETS, aGrfNames);
    pExamplesVS->SetHelpId(HID_VALUESET_NUMBMP    );

    sal_uInt16 i = 0;
    for(std::vector<String>::iterator it = aGrfNames.begin(); it != aGrfNames.end(); ++it, ++i)
    {
        pExamplesVS->InsertItem( i + 1, i);

        INetURLObject aObj(*it);
        if(aObj.GetProtocol() == INET_PROT_FILE)
            *it = aObj.PathToFileName();

        pExamplesVS->SetItemText( i + 1, *it );
    }

    if(aGrfNames.empty())
    {
        aErrorText.Show();
    }
    else
    {
        pExamplesVS->Show();
        pExamplesVS->Format();
    }

    pExamplesVS->SetAccessibleRelationMemberOf( &aValuesFL );
}

SvxBitmapPickTabPage::~SvxBitmapPickTabPage()
{
    delete pExamplesVS;
    delete pActNum;
    delete pSaveNum;
}

SfxTabPage*  SvxBitmapPickTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SvxBitmapPickTabPage(pParent, rAttrSet);
}

void  SvxBitmapPickTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    bPreset = sal_False;
    sal_Bool bIsPreset = sal_False;
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    if(pExampleSet)
    {
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_NUM_PRESET, sal_False, &pItem))
            bIsPreset = ((const SfxBoolItem*)pItem)->GetValue();
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_CUR_NUM_LEVEL, sal_False, &pItem))
            nActNumLvl = ((const SfxUInt16Item*)pItem)->GetValue();
    }
    if(SFX_ITEM_SET == rSet.GetItemState(nNumItemId, sal_False, &pItem))
    {
        delete pSaveNum;
        pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());
    }
    if(*pSaveNum != *pActNum)
    {
        *pActNum = *pSaveNum;
        pExamplesVS->SetNoSelection();
    }

    if(!aGrfNames.empty() &&
        (pActNum && (!lcl_IsNumFmtSet(pActNum, nActNumLvl) || bIsPreset)))
    {
        pExamplesVS->SelectItem(1);
        NumSelectHdl_Impl(pExamplesVS);
        bPreset = sal_True;
    }
    bPreset |= bIsPreset;
    bModified = sal_False;
}

int  SvxBitmapPickTabPage::DeactivatePage(SfxItemSet *_pSet)
{
    if(_pSet)
        FillItemSet(*_pSet);
    return sal_True;
}

sal_Bool  SvxBitmapPickTabPage::FillItemSet( SfxItemSet& rSet )
{
    if ( aGrfNames.empty() )
    {
        return sal_False;
    }
    if( (bPreset || bModified) && pActNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SvxNumBulletItem( *pSaveNum ), nNumItemId);
        rSet.Put(SfxBoolItem(SID_PARAM_NUM_PRESET, bPreset));
    }

    return bModified;
}

void  SvxBitmapPickTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    // in Draw the item exists as WhichId, in Writer only as SlotId
    SfxItemState eState = rSet.GetItemState(SID_ATTR_NUMBERING_RULE, sal_False, &pItem);
    if(eState != SFX_ITEM_SET)
    {
        nNumItemId = rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        eState = rSet.GetItemState(nNumItemId, sal_False, &pItem);

        if( eState != SFX_ITEM_SET )
        {
            pItem = &static_cast< const SvxNumBulletItem& >( rSet.Get( nNumItemId, sal_True ) );
            eState = SFX_ITEM_SET;
        }

    }
    DBG_ASSERT(eState == SFX_ITEM_SET, "kein Item gefunden!");
    delete pSaveNum;
    pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());

    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
    if(!pActNum->IsFeatureSupported(NUM_ENABLE_LINKED_BMP))
    {
        aLinkedCB.Check(sal_False);
        aLinkedCB.Enable(sal_False);
    }
    else if(!pActNum->IsFeatureSupported(NUM_ENABLE_EMBEDDED_BMP))
    {
        aLinkedCB.Check(sal_True);
        aLinkedCB.Enable(sal_False);
    }
}

IMPL_LINK_NOARG(SvxBitmapPickTabPage, NumSelectHdl_Impl)
{
    if(pActNum)
    {
        bPreset = sal_False;
        bModified = sal_True;
        sal_uInt16 nIdx = pExamplesVS->GetSelectItemId() - 1;

        sal_uInt16 nMask = 1;
        String aEmptyStr;
        sal_uInt16 nSetNumberingType = SVX_NUM_BITMAP;
        if(aLinkedCB.IsChecked())
            nSetNumberingType |= LINK_TOKEN;
        for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                SvxNumberFormat aFmt(pActNum->GetLevel(i));
                aFmt.SetNumberingType(nSetNumberingType);
                aFmt.SetPrefix( aEmptyStr );
                aFmt.SetSuffix( aEmptyStr );
                aFmt.SetCharFmtName( sNumCharFmtName );

                Graphic aGraphic;
                if(GalleryExplorer::GetGraphicObj( GALLERY_THEME_BULLETS, nIdx, &aGraphic))
                {
                    Size aSize = SvxNumberFormat::GetGraphicSizeMM100(&aGraphic);
                    sal_Int16 eOrient = text::VertOrientation::LINE_CENTER;
                    aSize = OutputDevice::LogicToLogic(aSize, MAP_100TH_MM, (MapUnit)eCoreUnit);
                    SvxBrushItem aBrush(aGraphic, GPOS_AREA, SID_ATTR_BRUSH );
                    aFmt.SetGraphicBrush( &aBrush, &aSize, &eOrient );
                }
                else if(aGrfNames.size() > nIdx)
                    aFmt.SetGraphic( aGrfNames[nIdx] );
                pActNum->SetLevel(i, aFmt);
            }
            nMask <<= 1 ;
        }
    }

    return 0;
}

IMPL_LINK_NOARG(SvxBitmapPickTabPage, DoubleClickHdl_Impl)
{
    NumSelectHdl_Impl(pExamplesVS);
    OKButton& rOk = GetTabDialog()->GetOKButton();
    rOk.GetClickHdl().Call(&rOk);
    return 0;
}

IMPL_LINK_NOARG(SvxBitmapPickTabPage, LinkBmpHdl_Impl)
{
    if(!pExamplesVS->IsNoSelection())
    {
        NumSelectHdl_Impl(pExamplesVS);
    }
    return 0;
}

// static
void SvxNumOptionsTabPage::GetI18nNumbering( ListBox& rFmtLB, sal_uInt16 nDoNotRemove )
{

    Reference<XDefaultNumberingProvider> xDefNum = lcl_GetNumberingProvider();
    Reference<XNumberingTypeInfo> xInfo(xDefNum, UNO_QUERY);

    // Extended numbering schemes present in the resource but not offered by
    // the i18n framework per configuration must be removed from the listbox.
    // Do not remove a special entry matching nDoNotRemove.
    const sal_uInt16 nDontRemove = 0xffff;
    ::std::vector< sal_uInt16> aRemove( rFmtLB.GetEntryCount(), nDontRemove);
    for (size_t i=0; i<aRemove.size(); ++i)
    {
        sal_uInt16 nEntryData = (sal_uInt16)(sal_uLong)rFmtLB.GetEntryData(
                sal::static_int_cast< sal_uInt16 >(i));
        if (nEntryData > NumberingType::CHARS_LOWER_LETTER_N && nEntryData != nDoNotRemove)
            aRemove[i] = nEntryData;
    }
    if(xInfo.is())
    {
        Sequence<sal_Int16> aTypes = xInfo->getSupportedNumberingTypes(  );
        const sal_Int16* pTypes = aTypes.getConstArray();
        for(sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
        {
            sal_Int16 nCurrent = pTypes[nType];
            if(nCurrent > NumberingType::CHARS_LOWER_LETTER_N)
            {
                sal_Bool bInsert = sal_True;
                for(sal_uInt16 nEntry = 0; nEntry < rFmtLB.GetEntryCount(); nEntry++)
                {
                    sal_uInt16 nEntryData = (sal_uInt16)(sal_uLong)rFmtLB.GetEntryData(nEntry);
                    if(nEntryData == (sal_uInt16) nCurrent)
                    {
                        bInsert = sal_False;
                        aRemove[nEntry] = nDontRemove;
                        break;
                    }
                }
                if(bInsert)
                {
                    OUString aIdent = xInfo->getNumberingIdentifier( nCurrent );
                    sal_uInt16 nPos = rFmtLB.InsertEntry(aIdent);
                    rFmtLB.SetEntryData(nPos,(void*)(sal_uLong)nCurrent);
                }
            }
        }
    }
    for (size_t i=0; i<aRemove.size(); ++i)
    {
        if (aRemove[i] != nDontRemove)
        {
            sal_uInt16 nPos = rFmtLB.GetEntryPos( (void*)(sal_uLong)aRemove[i]);
            rFmtLB.RemoveEntry( nPos);
        }
    }
}

// tabpage numeration options
#define NUM_NO_GRAPHIC 1000
SvxNumOptionsTabPage::SvxNumOptionsTabPage(Window* pParent,
                               const SfxItemSet& rSet) :
    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_NUM_OPTIONS ), rSet ),

    aFormatFL(      this, CUI_RES(FL_FORMAT   )),
    aLevelFT(       this, CUI_RES(FT_LEVEL    )),
    aLevelLB(       this, CUI_RES(LB_LEVEL  )),
    aFmtFT(         this, CUI_RES(FT_FMT        )),
    aFmtLB(         this, CUI_RES(LB_FMT        )),
    aPrefixFT(      this, CUI_RES(FT_PREFIX )),
    aPrefixED(      this, CUI_RES(ED_PREFIX )),
    aSuffixFT(      this, CUI_RES(FT_SUFFIX )),
    aSuffixED(      this, CUI_RES(ED_SUFFIX )),
    aCharFmtFT(     this, CUI_RES(FT_CHARFMT    )),
    aCharFmtLB(     this, CUI_RES(LB_CHARFMT    )),
    aBulColorFT(    this, CUI_RES(FT_BUL_COLOR)),
    aBulColLB(      this, CUI_RES(LB_BUL_COLOR)),
    aBulRelSizeFT(  this, CUI_RES(FT_BUL_REL_SIZE)),
    aBulRelSizeMF(  this, CUI_RES(MF_BUL_REL_SIZE)),
    aAllLevelFT(    this, CUI_RES(FT_ALL_LEVEL)),
    aAllLevelNF(    this, CUI_RES(NF_ALL_LEVEL)),
    aStartFT(       this, CUI_RES(FT_START  )),
    aStartED(       this, CUI_RES(ED_START  )),
    aBulletPB(      this, CUI_RES(PB_BULLET )),
    aAlignFT(       this, CUI_RES(FT_ALIGN  )),
    aAlignLB(       this, CUI_RES(LB_ALIGN  )),
    aBitmapFT(      this, CUI_RES(FT_BITMAP )),
    aBitmapMB(      this, CUI_RES(MB_BITMAP )),
    aSizeFT(        this, CUI_RES(FT_SIZE       )),
    aWidthMF(       this, CUI_RES(MF_WIDTH  )),
    aMultFT(        this, CUI_RES(FT_MULT       )),
    aHeightMF(      this, CUI_RES(MF_HEIGHT )),
    aRatioCB(       this, CUI_RES(CB_RATIO  )),
    aOrientFT(      this, CUI_RES(FT_ORIENT )),
    aOrientLB(      this, CUI_RES(LB_ORIENT )),
    aSameLevelFL(   this, CUI_RES(FL_SAME_LEVEL)),
    aSameLevelCB(   this, CUI_RES(CB_SAME_LEVEL)),
    pPreviewWIN(    new SvxNumberingPreview(this, CUI_RES(WIN_PREVIEW   ))),
    pActNum(0),
    pSaveNum(0),
    bLastWidthModified(sal_False),
    bModified(sal_False),
    bPreset(sal_False),
    bAutomaticCharStyles(sal_True),
    bHTMLMode(sal_False),
    bMenuButtonInitialized(sal_False),
    sBullet(CUI_RES(STR_BULLET)),
    nBullet(0xff),
    nActNumLvl(USHRT_MAX),
    nNumItemId(SID_ATTR_NUMBERING_RULE)
{
    sStartWith = aStartFT.GetText();
    pPreviewWIN->SetBackground(Wallpaper(Color(COL_TRANSPARENT)));
    SetExchangeSupport();
    aActBulletFont = lcl_GetDefaultBulletFont();

    aBulletPB.SetClickHdl(LINK(this, SvxNumOptionsTabPage, BulletHdl_Impl));
    aFmtLB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, NumberTypeSelectHdl_Impl));
    aBitmapMB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, GraphicHdl_Impl));
    aLevelLB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, LevelHdl_Impl));
    aCharFmtLB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, CharFmtHdl_Impl));
    aWidthMF.SetModifyHdl(LINK(this, SvxNumOptionsTabPage, SizeHdl_Impl));
    aHeightMF.SetModifyHdl(LINK(this, SvxNumOptionsTabPage, SizeHdl_Impl));
    aRatioCB.SetClickHdl(LINK(this, SvxNumOptionsTabPage, RatioHdl_Impl));
    aStartED.SetModifyHdl(LINK(this, SvxNumOptionsTabPage, EditModifyHdl_Impl));
    aPrefixED.SetModifyHdl(LINK(this, SvxNumOptionsTabPage, EditModifyHdl_Impl));
    aSuffixED.SetModifyHdl(LINK(this, SvxNumOptionsTabPage, EditModifyHdl_Impl));
    aAllLevelNF.SetModifyHdl(LINK(this,SvxNumOptionsTabPage, AllLevelHdl_Impl));
    aOrientLB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, OrientHdl_Impl));
    aSameLevelCB.SetClickHdl(LINK(this, SvxNumOptionsTabPage, SameLevelHdl_Impl));
    aBulRelSizeMF.SetModifyHdl(LINK(this,SvxNumOptionsTabPage, BulRelSizeHdl_Impl));
    aBulColLB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, BulColorHdl_Impl));
    aInvalidateTimer.SetTimeoutHdl(LINK(this, SvxNumOptionsTabPage, PreviewInvalidateHdl_Impl));
    aInvalidateTimer.SetTimeout(50);

    aBitmapMB.GetPopupMenu()->SetHighlightHdl(LINK(this, SvxNumOptionsTabPage, PopupActivateHdl_Impl));
    PopupMenu* pPopup = new PopupMenu;
    aBitmapMB.GetPopupMenu()->SetPopupMenu( MN_GALLERY, pPopup );

    pPopup->InsertItem( NUM_NO_GRAPHIC, String(CUI_RES(ST_POPUP_EMPTY_ENTRY)) );
    pPopup->EnableItem( NUM_NO_GRAPHIC, sal_False );

    eCoreUnit = rSet.GetPool()->GetMetric(rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE));

    aBitmapMB.SetAccessibleRelationLabeledBy( &aBitmapFT );

    FreeResource();

    // Get advanced numbering types from the component.
    // Watch out for the ugly 0x88/*SVX_NUM_BITMAP|0x80*/ to not remove that.
    GetI18nNumbering( aFmtLB, (SVX_NUM_BITMAP | 0x80));

    aBulletPB.SetAccessibleRelationMemberOf(&aFormatFL);
    aBulletPB.SetAccessibleRelationLabeledBy(&aStartFT);
    aBulletPB.SetAccessibleName(aStartFT.GetText());
}

SvxNumOptionsTabPage::~SvxNumOptionsTabPage()
{
    delete aBitmapMB.GetPopupMenu()->GetPopupMenu( MN_GALLERY );
    delete pActNum;
    delete pPreviewWIN;
    delete pSaveNum;
}

void SvxNumOptionsTabPage::SetMetric(FieldUnit eMetric)
{
    if(eMetric == FUNIT_MM)
    {
        aWidthMF     .SetDecimalDigits(1);
        aHeightMF     .SetDecimalDigits(1);
    }
    aWidthMF .SetUnit( eMetric );
    aHeightMF .SetUnit( eMetric );
}

SfxTabPage* SvxNumOptionsTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SvxNumOptionsTabPage(pParent, rAttrSet);
};

void    SvxNumOptionsTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    sal_uInt16 nTmpNumLvl = USHRT_MAX;
    if(pExampleSet)
    {
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_NUM_PRESET, sal_False, &pItem))
            bPreset = ((const SfxBoolItem*)pItem)->GetValue();
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_CUR_NUM_LEVEL, sal_False, &pItem))
            nTmpNumLvl = ((const SfxUInt16Item*)pItem)->GetValue();
    }
    if(SFX_ITEM_SET == rSet.GetItemState(nNumItemId, sal_False, &pItem))
    {
        delete pSaveNum;
        pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());
    }
    //
    bModified = (!pActNum->Get( 0 ) || bPreset);
    if(*pActNum != *pSaveNum ||
        nActNumLvl != nTmpNumLvl)
    {
        nActNumLvl = nTmpNumLvl;
        sal_uInt16 nMask = 1;
        aLevelLB.SetUpdateMode(sal_False);
        aLevelLB.SetNoSelection();
        aLevelLB.SelectEntryPos( pActNum->GetLevelCount(), nActNumLvl == USHRT_MAX);
        if(nActNumLvl != USHRT_MAX)
            for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
            {
                if(nActNumLvl & nMask)
                    aLevelLB.SelectEntryPos( i, sal_True);
                nMask <<= 1 ;
            }
        aLevelLB.SetUpdateMode(sal_True);
        *pActNum = *pSaveNum;
        InitControls();
    }

}

int     SvxNumOptionsTabPage::DeactivatePage(SfxItemSet * _pSet)
{
    if(_pSet)
        FillItemSet(*_pSet);
    return sal_True;
}

sal_Bool    SvxNumOptionsTabPage::FillItemSet( SfxItemSet& rSet )
{
    rSet.Put(SfxUInt16Item(SID_PARAM_CUR_NUM_LEVEL, nActNumLvl));
    if(bModified && pActNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SvxNumBulletItem( *pSaveNum ), nNumItemId);
        rSet.Put(SfxBoolItem(SID_PARAM_NUM_PRESET, sal_False));
    }
    return bModified;
};

void    SvxNumOptionsTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    // in Draw the item exists as WhichId, in Writer only as SlotId
    SfxItemState eState = rSet.GetItemState(SID_ATTR_NUMBERING_RULE, sal_False, &pItem);
    if(eState != SFX_ITEM_SET)
    {
        nNumItemId = rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        eState = rSet.GetItemState(nNumItemId, sal_False, &pItem);

        if( eState != SFX_ITEM_SET )
        {
            pItem = &static_cast< const SvxNumBulletItem& >( rSet.Get( nNumItemId, sal_True ) );
            eState = SFX_ITEM_SET;
        }

    }
    DBG_ASSERT(eState == SFX_ITEM_SET, "kein Item gefunden!");
    delete pSaveNum;
    pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());

    // insert levels
    if(!aLevelLB.GetEntryCount())
    {
        for(sal_uInt16 i = 1; i <= pSaveNum->GetLevelCount(); i++)
            aLevelLB.InsertEntry( UniString::CreateFromInt32(i));
        if(pSaveNum->GetLevelCount() > 1)
        {
            String sEntry(rtl::OUString("1 - "));
            sEntry += UniString::CreateFromInt32( pSaveNum->GetLevelCount() );
            aLevelLB.InsertEntry(sEntry);
            aLevelLB.SelectEntry(sEntry);
        }
        else
            aLevelLB.SelectEntryPos(0);
    }
    else
        aLevelLB.SelectEntryPos(aLevelLB.GetEntryCount() - 1);

    sal_uInt16 nMask = 1;
    aLevelLB.SetUpdateMode(sal_False);
    aLevelLB.SetNoSelection();
    if(nActNumLvl == USHRT_MAX)
    {
        aLevelLB.SelectEntryPos( pSaveNum->GetLevelCount(), sal_True);
    }
    else
        for(sal_uInt16 i = 0; i < pSaveNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
                aLevelLB.SelectEntryPos( i, sal_True);
            nMask <<= 1 ;
        }
    aLevelLB.SetUpdateMode(sal_True);

    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
    pPreviewWIN->SetNumRule(pActNum);
    aSameLevelCB.Check(pActNum->IsContinuousNumbering());

    // fill ColorListBox as needed
    if ( pActNum->IsFeatureSupported( NUM_BULLET_COLOR ) )
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        DBG_ASSERT( pDocSh, "DocShell not found!" );
        XColorListRef pColorTable;
        if ( pDocSh )
        {
            pItem = pDocSh->GetItem( SID_COLOR_TABLE );
            if ( pItem )
                pColorTable = ( (SvxColorListItem*)pItem )->GetColorList();
        }

        if ( !pColorTable.is() )
            pColorTable = XColorList::CreateStdColorList();

        aBulColLB.InsertEntry( Color( COL_AUTO ), SVX_RESSTR( RID_SVXSTR_AUTOMATIC ));

        for ( long i = 0; i < pColorTable->Count(); i++ )
        {
            XColorEntry* pEntry = pColorTable->GetColor(i);
            aBulColLB.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        }
    }

    SfxObjectShell* pShell;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_HTML_MODE, sal_False, &pItem )
         || ( 0 != ( pShell = SfxObjectShell::Current()) &&
              0 != ( pItem = pShell->GetItem( SID_HTML_MODE ) ) ) )
    {
        sal_uInt16 nHtmlMode = ((SfxUInt16Item*)pItem)->GetValue();
        bHTMLMode = 0 != (nHtmlMode&HTMLMODE_ON);
    }

    sal_Bool bCharFmt = pActNum->IsFeatureSupported(NUM_CHAR_STYLE);
    aCharFmtFT.Show(bCharFmt);
    aCharFmtLB.Show(bCharFmt);

    sal_Bool bContinuous = pActNum->IsFeatureSupported(NUM_CONTINUOUS);

    sal_Bool bAllLevel = bContinuous && !bHTMLMode;
    aAllLevelFT.Show(bAllLevel);
    aAllLevelNF.Show(bAllLevel);

    aSameLevelFL.Show(bContinuous);
    aSameLevelCB.Show(bContinuous);
    // again misusage: in Draw there is numeration only until the bitmap
    // without SVX_NUM_NUMBER_NONE
    //remove types that are unsupported by Draw/Impress
    if(!bContinuous)
    {
        sal_uInt16 nFmtCount = aFmtLB.GetEntryCount();
        for(sal_uInt16 i = nFmtCount; i; i--)
        {
            sal_uInt16 nEntryData = (sal_uInt16)(sal_uLong)aFmtLB.GetEntryData(i - 1);
            if(/*SVX_NUM_NUMBER_NONE == nEntryData ||*/
                ((SVX_NUM_BITMAP|LINK_TOKEN) ==  nEntryData))
                aFmtLB.RemoveEntry(i - 1);
        }
    }
    //one must be enabled
    if(!pActNum->IsFeatureSupported(NUM_ENABLE_LINKED_BMP))
    {
        long nData = SVX_NUM_BITMAP|LINK_TOKEN;
        sal_uInt16 nPos = aFmtLB.GetEntryPos((void*)nData);
        if(LISTBOX_ENTRY_NOTFOUND != nPos)
            aFmtLB.RemoveEntry(nPos);
    }
    else if(!pActNum->IsFeatureSupported(NUM_ENABLE_EMBEDDED_BMP))
    {
        long nData = SVX_NUM_BITMAP;
        sal_uInt16 nPos = aFmtLB.GetEntryPos((void*)nData);
        if(LISTBOX_ENTRY_NOTFOUND != nPos)
            aFmtLB.RemoveEntry(nPos);
    }
    if(pActNum->IsFeatureSupported(NUM_SYMBOL_ALIGNMENT))
    {
        aAlignFT.Show();
        aAlignLB.Show();
        Size aSz(aFormatFL.GetSizePixel());
        aSz.Height() = aLevelFT.GetSizePixel().Height();
        aFormatFL.SetSizePixel(aSz);
        aAlignLB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, EditModifyHdl_Impl));
    }

    // MegaHack: because of a not-fixable 'design mistake/error' in Impress
    // delete all kinds of numeric enumerations
    if(pActNum->IsFeatureSupported(NUM_NO_NUMBERS))
    {
        sal_uInt16 nFmtCount = aFmtLB.GetEntryCount();
        for(sal_uInt16 i = nFmtCount; i; i--)
        {
            sal_uInt16 nEntryData = (sal_uInt16)(sal_uLong)aFmtLB.GetEntryData(i - 1);
            if( /*nEntryData >= SVX_NUM_CHARS_UPPER_LETTER &&*/  nEntryData <= SVX_NUM_NUMBER_NONE)
                aFmtLB.RemoveEntry(i - 1);
        }
    }

    InitControls();
    bModified = sal_False;

}

void SvxNumOptionsTabPage::InitControls()
{
    sal_Bool bShowBullet    = sal_True;
    sal_Bool bShowBitmap    = sal_True;
    sal_Bool bSameType      = sal_True;
    sal_Bool bSameStart     = sal_True;
    sal_Bool bSamePrefix    = sal_True;
    sal_Bool bSameSuffix    = sal_True;
    sal_Bool bAllLevel      = sal_True;
    sal_Bool bSameCharFmt   = sal_True;
    sal_Bool bSameVOrient   = sal_True;
    sal_Bool bSameSize      = sal_True;
    sal_Bool bSameBulColor  = sal_True;
    sal_Bool bSameBulRelSize= sal_True;
    sal_Bool bSameAdjust    = sal_True;

    const SvxNumberFormat* aNumFmtArr[SVX_MAX_NUM];
    String sFirstCharFmt;
    sal_Int16 eFirstOrient = text::VertOrientation::NONE;
    Size aFirstSize(0,0);
    sal_uInt16 nMask = 1;
    sal_uInt16 nLvl = USHRT_MAX;
    sal_uInt16 nHighestLevel = 0;
    String aEmptyStr;

    sal_Bool bBullColor = pActNum->IsFeatureSupported(NUM_BULLET_COLOR);
    sal_Bool bBullRelSize = pActNum->IsFeatureSupported(NUM_BULLET_REL_SIZE);
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            aNumFmtArr[i] = &pActNum->GetLevel(i);
            bShowBullet &= aNumFmtArr[i]->GetNumberingType() == SVX_NUM_CHAR_SPECIAL;
            bShowBitmap &= (aNumFmtArr[i]->GetNumberingType()&(~LINK_TOKEN)) == SVX_NUM_BITMAP;
            if(USHRT_MAX == nLvl)
            {
                nLvl = i;
                sFirstCharFmt = aNumFmtArr[i]->GetCharFmtName();
                eFirstOrient = aNumFmtArr[i]->GetVertOrient();
                if(bShowBitmap)
                    aFirstSize = aNumFmtArr[i]->GetGraphicSize();
            }
            if( i > nLvl)
            {
                bSameType &=   aNumFmtArr[i]->GetNumberingType() == aNumFmtArr[nLvl]->GetNumberingType();
                bSameStart = aNumFmtArr[i]->GetStart() == aNumFmtArr[nLvl]->GetStart();

                bSamePrefix = aNumFmtArr[i]->GetPrefix() == aNumFmtArr[nLvl]->GetPrefix();
                bSameSuffix = aNumFmtArr[i]->GetSuffix() == aNumFmtArr[nLvl]->GetSuffix();
                bAllLevel &= aNumFmtArr[i]->GetIncludeUpperLevels() == aNumFmtArr[nLvl]->GetIncludeUpperLevels();
                bSameCharFmt    &=  sFirstCharFmt == aNumFmtArr[i]->GetCharFmtName();
                bSameVOrient    &= eFirstOrient == aNumFmtArr[i]->GetVertOrient();
                if(bShowBitmap && bSameSize)
                    bSameSize &= aNumFmtArr[i]->GetGraphicSize() == aFirstSize;
                bSameBulColor &= aNumFmtArr[i]->GetBulletColor() == aNumFmtArr[nLvl]->GetBulletColor();
                bSameBulRelSize &= aNumFmtArr[i]->GetBulletRelSize() == aNumFmtArr[nLvl]->GetBulletRelSize();
                bSameAdjust     &= aNumFmtArr[i]->GetNumAdjust() == aNumFmtArr[nLvl]->GetNumAdjust();
            }
            nHighestLevel = i;
        }
        else
            aNumFmtArr[i] = 0;

        nMask <<= 1 ;

    }
    SwitchNumberType(bShowBullet ? 1 : bShowBitmap ? 2 : 0);
    CheckForStartValue_Impl(aNumFmtArr[nLvl]->GetNumberingType());
    if(bShowBitmap)
    {
        if(!bSameVOrient || eFirstOrient == text::VertOrientation::NONE)
            aOrientLB.SetNoSelection();
        else
            aOrientLB.SelectEntryPos(
                sal::static_int_cast< sal_uInt16 >(eFirstOrient - 1));
                // no text::VertOrientation::NONE

        if(bSameSize)
        {
            SetMetricValue(aHeightMF, aFirstSize.Height(), eCoreUnit);
            SetMetricValue(aWidthMF, aFirstSize.Width(), eCoreUnit);
        }
        else
        {
            aHeightMF.SetText(aEmptyStr);
            aWidthMF.SetText(aEmptyStr);
        }
    }

    if(bSameType)
    {
        sal_uInt16 nLBData = (sal_uInt16) aNumFmtArr[nLvl]->GetNumberingType();
        aFmtLB.SelectEntryPos(aFmtLB.GetEntryPos( (void*)sal::static_int_cast<sal_uIntPtr>( nLBData ) ));
    }
    else
        aFmtLB.SetNoSelection();

    aAllLevelNF.Enable(nHighestLevel > 0 && !aSameLevelCB.IsChecked());
    aAllLevelNF.SetMax(nHighestLevel + 1);
    if(bAllLevel)
    {
        aAllLevelNF.SetValue(aNumFmtArr[nLvl]->GetIncludeUpperLevels());
    }
    else
    {
        aAllLevelNF.SetText(aEmptyStr);
    }
    if(bSameAdjust)
    {
        sal_uInt16 nPos = 1; // centered
        if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_LEFT)
            nPos = 0;
        else if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_RIGHT)
            nPos = 2;
        aAlignLB.SelectEntryPos(nPos);
    }
    else
    {
        aAlignLB.SetNoSelection();
    }

    if(bBullRelSize)
    {
        if(bSameBulRelSize)
            aBulRelSizeMF.SetValue(aNumFmtArr[nLvl]->GetBulletRelSize());
        else
            aBulRelSizeMF.SetText(aEmptyStr);
    }
    if(bBullColor)
    {
        if(bSameBulColor)
            aBulColLB.SelectEntry(aNumFmtArr[nLvl]->GetBulletColor());
        else
            aBulColLB.SetNoSelection();
    }
    switch(nBullet)
    {
        case SHOW_NUMBERING:
            if(bSameStart)
            {
                aStartED.SetValue(aNumFmtArr[nLvl]->GetStart());
            }
            else
                aStartED.SetText(aEmptyStr);
        break;
        case SHOW_BULLET:
        break;
        case SHOW_BITMAP:
        break;
    }

    if(bSamePrefix)
        aPrefixED.SetText(aNumFmtArr[nLvl]->GetPrefix());
    else
        aPrefixED.SetText(aEmptyStr);
    if(bSameSuffix)
        aSuffixED.SetText(aNumFmtArr[nLvl]->GetSuffix());
    else
        aSuffixED.SetText(aEmptyStr);

    if(bSameCharFmt)
    {
        if(sFirstCharFmt.Len())
                aCharFmtLB.SelectEntry(sFirstCharFmt);
        else
            aCharFmtLB.SelectEntryPos( 0 );
    }
    else
        aCharFmtLB.SetNoSelection();

    pPreviewWIN->SetLevel(nActNumLvl);
    pPreviewWIN->Invalidate();
}

// 0 - Number; 1 - Bullet; 2 - Bitmap
void SvxNumOptionsTabPage::SwitchNumberType( sal_uInt8 nType, sal_Bool )
{
    if(nBullet == nType)
        return;
    nBullet = nType;
    sal_Bool bBitmap = sal_False;
    sal_Bool bBullet = sal_False;
    sal_Bool bEnableBitmap = sal_False;
    if(nType == SHOW_NUMBERING)
    {
        // switch label, memorize old text
        aStartFT.SetText(sStartWith);

    }
    else if(nType == SHOW_BULLET)
    {
        // switch label, memorize old text
        aStartFT.SetText(sBullet);
        bBullet = sal_True;
    }
    else
    {
        bBitmap = sal_True;
        bEnableBitmap = sal_True;
    }
    sal_Bool bNumeric = !(bBitmap||bBullet);
    aPrefixFT.Show(bNumeric);
    aPrefixED.Show(bNumeric);
    aSuffixFT.Show(bNumeric);
    aSuffixED.Show(bNumeric);

    sal_Bool bCharFmt = pActNum->IsFeatureSupported(NUM_CHAR_STYLE);
    aCharFmtFT.Show(!bBitmap && bCharFmt);
    aCharFmtLB.Show(!bBitmap && bCharFmt);

    // this is rather misusage, as there is no own flag
    // for complete numeration
    sal_Bool bAllLevelFeature = pActNum->IsFeatureSupported(NUM_CONTINUOUS);
    sal_Bool bAllLevel = bNumeric && bAllLevelFeature && !bHTMLMode;
    aAllLevelFT.Show(bAllLevel);
    aAllLevelNF.Show(bAllLevel);

    aStartFT.Show(!bBitmap);
    aStartED.Show(!(bBullet||bBitmap));

    aBulletPB.Show(bBullet);
    sal_Bool bBullColor = pActNum->IsFeatureSupported(NUM_BULLET_COLOR);
    aBulColorFT.Show(   !bBitmap && bBullColor );
    aBulColLB.Show(     !bBitmap && bBullColor );
    sal_Bool bBullResSize = pActNum->IsFeatureSupported(NUM_BULLET_REL_SIZE);
    aBulRelSizeFT.Show( !bBitmap && bBullResSize );
    aBulRelSizeMF.Show( !bBitmap && bBullResSize );

    aBitmapFT   .Show(bBitmap);
    aBitmapMB   .Show(bBitmap);

    aSizeFT     .Show(bBitmap);
    aWidthMF    .Show(bBitmap);
    aMultFT     .Show(bBitmap);
    aHeightMF   .Show(bBitmap);
    aRatioCB    .Show(bBitmap);

    aOrientFT   .Show(bBitmap &&  bAllLevelFeature);
    aOrientLB   .Show(bBitmap &&  bAllLevelFeature);

    aSizeFT     .Enable(bEnableBitmap);
    aWidthMF    .Enable(bEnableBitmap);
    aMultFT     .Enable(bEnableBitmap);
    aHeightMF   .Enable(bEnableBitmap);
    aRatioCB    .Enable(bEnableBitmap);
    aOrientFT   .Enable(bEnableBitmap);
    aOrientLB   .Enable(bEnableBitmap);

}

IMPL_LINK( SvxNumOptionsTabPage, LevelHdl_Impl, ListBox *, pBox )
{
    sal_uInt16 nSaveNumLvl = nActNumLvl;
    nActNumLvl = 0;
    if(pBox->IsEntryPosSelected( pActNum->GetLevelCount() ) &&
        (pBox->GetSelectEntryCount() == 1 || nSaveNumLvl != 0xffff))
    {
        nActNumLvl = 0xFFFF;
        pBox->SetUpdateMode(sal_False);
        for( sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++ )
            pBox->SelectEntryPos( i, sal_False );
        pBox->SetUpdateMode(sal_True);
    }
    else if(pBox->GetSelectEntryCount())
    {
        sal_uInt16 nMask = 1;
        for( sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++ )
        {
            if(pBox->IsEntryPosSelected( i ))
                nActNumLvl |= nMask;
            nMask <<= 1;
        }
        pBox->SelectEntryPos( pActNum->GetLevelCount(), sal_False );
    }
    else
    {
        nActNumLvl = nSaveNumLvl;
        sal_uInt16 nMask = 1;
        for( sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++ )
        {
            if(nActNumLvl & nMask)
            {
                pBox->SelectEntryPos(i);
                break;
            }
            nMask <<=1;
        }
    }
    InitControls();
    return 0;
}

IMPL_LINK_NOARG(SvxNumOptionsTabPage, PreviewInvalidateHdl_Impl)
{
    pPreviewWIN->Invalidate();
    return 0;
}

IMPL_LINK( SvxNumOptionsTabPage, AllLevelHdl_Impl, NumericField*, pBox )
{
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        sal_uInt16 nMask = 1;
        for(sal_uInt16 e = 0; e < pActNum->GetLevelCount(); e++)
        {
            if(nActNumLvl & nMask)
            {
                SvxNumberFormat aNumFmt(pActNum->GetLevel(e));
                aNumFmt.SetIncludeUpperLevels((sal_uInt8) std::min(pBox->GetValue(), sal_Int64(e + 1)) );
                pActNum->SetLevel(e, aNumFmt);
            }
            nMask <<= 1;
        }
    }
    SetModified();
    return 0;
}

IMPL_LINK( SvxNumOptionsTabPage, NumberTypeSelectHdl_Impl, ListBox *, pBox )
{
    String sSelectStyle;
    sal_Bool bShowOrient = sal_False;
    sal_Bool bBmp = sal_False;
    String aEmptyStr;
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            // PAGEDESC does not exist
            sal_uInt16 nNumType = (sal_uInt16)(sal_uLong)pBox->GetEntryData(pBox->GetSelectEntryPos());
            aNumFmt.SetNumberingType((sal_Int16)nNumType);
            sal_uInt16 nNumberingType = aNumFmt.GetNumberingType();
            if(SVX_NUM_BITMAP == (nNumberingType&(~LINK_TOKEN)))
            {
                bBmp |= 0 != aNumFmt.GetBrush();
                aNumFmt.SetIncludeUpperLevels( sal_False );
                aNumFmt.SetSuffix( aEmptyStr );
                aNumFmt.SetPrefix( aEmptyStr );
                if(!bBmp)
                    aNumFmt.SetGraphic(aEmptyStr);
                pActNum->SetLevel(i, aNumFmt);
                SwitchNumberType(SHOW_BITMAP, bBmp );
                bShowOrient = sal_True;
            }
            else if( SVX_NUM_CHAR_SPECIAL == nNumberingType )
            {
                aNumFmt.SetIncludeUpperLevels( sal_False );
                aNumFmt.SetSuffix( aEmptyStr );
                aNumFmt.SetPrefix( aEmptyStr );
                if( !aNumFmt.GetBulletFont() )
                    aNumFmt.SetBulletFont(&aActBulletFont);
                if( !aNumFmt.GetBulletChar() )
                    aNumFmt.SetBulletChar( SVX_DEF_BULLET );
                pActNum->SetLevel(i, aNumFmt);
                SwitchNumberType(SHOW_BULLET);
                // allocation of the drawing pattern is automatic
                if(bAutomaticCharStyles)
                {
                    sSelectStyle = sBulletCharFmtName;
                }
            }
            else
            {
                aNumFmt.SetPrefix( aPrefixED.GetText() );
                aNumFmt.SetSuffix( aSuffixED.GetText() );
                SwitchNumberType(SHOW_NUMBERING);
                pActNum->SetLevel(i, aNumFmt);
                CheckForStartValue_Impl(nNumberingType);

                // allocation of the drawing pattern is automatic
                if(bAutomaticCharStyles)
                {
                    sSelectStyle = sNumCharFmtName;
                }
            }
        }
        nMask <<= 1;
    }
    sal_Bool bAllLevelFeature = pActNum->IsFeatureSupported(NUM_CONTINUOUS);
    if(bShowOrient && bAllLevelFeature)
    {
        aOrientFT.Show();
        aOrientLB.Show();
    }
    else
    {
        aOrientFT.Hide();
        aOrientLB.Hide();
    }
    SetModified();
    if(sSelectStyle.Len())
    {
        aCharFmtLB.SelectEntry(sSelectStyle);
        CharFmtHdl_Impl(&aCharFmtLB);
        bAutomaticCharStyles = sal_True;
    }
    return 0;
}

void SvxNumOptionsTabPage::CheckForStartValue_Impl(sal_uInt16 nNumberingType)
{
    sal_Bool bIsNull = aStartED.GetValue() == 0;
    sal_Bool bNoZeroAllowed = nNumberingType < SVX_NUM_ARABIC ||
                        SVX_NUM_CHARS_UPPER_LETTER_N == nNumberingType ||
                        SVX_NUM_CHARS_LOWER_LETTER_N == nNumberingType;
    aStartED.SetMin(bNoZeroAllowed ? 1 : 0);
    if(bIsNull && bNoZeroAllowed)
        aStartED.GetModifyHdl().Call(&aStartED);
}

IMPL_LINK( SvxNumOptionsTabPage, OrientHdl_Impl, ListBox *, pBox )
{
    sal_uInt16 nPos = pBox->GetSelectEntryPos();
    nPos ++; // no VERT_NONE

    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            if(SVX_NUM_BITMAP == (aNumFmt.GetNumberingType()&(~LINK_TOKEN)))
            {
                const SvxBrushItem* pBrushItem =  aNumFmt.GetBrush();
                const Size& rSize = aNumFmt.GetGraphicSize();
                sal_Int16 eOrient = (sal_Int16)nPos;
                aNumFmt.SetGraphicBrush( pBrushItem, &rSize, &eOrient );
                pActNum->SetLevel(i, aNumFmt);
            }
        }
        nMask <<= 1;
    }
    SetModified(sal_False);
    return 0;

}

IMPL_LINK( SvxNumOptionsTabPage, SameLevelHdl_Impl, CheckBox *, pBox )
{
    sal_Bool bSet = pBox->IsChecked();
    pActNum->SetContinuousNumbering(bSet);
    sal_Bool bRepaint = sal_False;
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
        if(aNumFmt.GetNumberingType() != SVX_NUM_NUMBER_NONE)
        {
            bRepaint = sal_True;
            break;
        }
    }
    SetModified(bRepaint);
    InitControls();
    return 0;
}

IMPL_LINK( SvxNumOptionsTabPage, BulColorHdl_Impl, ColorListBox*, pBox )
{
    Color nSetColor = pBox->GetSelectEntryColor();

    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            aNumFmt.SetBulletColor(nSetColor);
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK( SvxNumOptionsTabPage, BulRelSizeHdl_Impl, MetricField *, pField)
{
    sal_uInt16 nRelSize = (sal_uInt16)pField->GetValue();

    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            aNumFmt.SetBulletRelSize(nRelSize);
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK( SvxNumOptionsTabPage, GraphicHdl_Impl, MenuButton *, pButton )
{
    sal_uInt16                  nItemId = pButton->GetCurItemId();
    String                  aGrfName;
    Size                    aSize;
    sal_Bool                bSucc(sal_False);
    SvxOpenGraphicDialog    aGrfDlg( CUI_RES(RID_STR_EDIT_GRAPHIC) );

    if(MN_GALLERY_ENTRY <= nItemId )
    {
        sal_uInt16 idx = nItemId - MN_GALLERY_ENTRY;
        if (idx < aGrfNames.size())
        {
            aGrfName = aGrfNames[idx];
            Graphic aGraphic;
            if(GalleryExplorer::GetGraphicObj( GALLERY_THEME_BULLETS, idx, &aGraphic))
            {
                aSize = SvxNumberFormat::GetGraphicSizeMM100(&aGraphic);
                bSucc = sal_True;
            }
        }
    }
    else
    {
        aGrfDlg.EnableLink( sal_False );
        aGrfDlg.AsLink( sal_False );
        if ( !aGrfDlg.Execute() )
        {
            // memorize selected filter
            aGrfName = aGrfDlg.GetPath();

            Graphic aGraphic;
            if( !aGrfDlg.GetGraphic(aGraphic) )
            {
                aSize = SvxNumberFormat::GetGraphicSizeMM100(&aGraphic);
                bSucc = sal_True;
            }
        }
    }
    if(bSucc)
    {
        aSize = OutputDevice::LogicToLogic(aSize, MAP_100TH_MM, (MapUnit)eCoreUnit);

        sal_uInt16 nMask = 1;
        for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
                aNumFmt.SetCharFmtName(sNumCharFmtName);
                aNumFmt.SetGraphic(aGrfName);

                // set size for a later comparison
                const SvxBrushItem* pBrushItem = aNumFmt.GetBrush();
                // initiate asynchronous loading
                sal_Int16 eOrient = aNumFmt.GetVertOrient();
                aNumFmt.SetGraphicBrush( pBrushItem, &aSize, &eOrient );
                aInitSize[i] = aNumFmt.GetGraphicSize();

                pActNum->SetLevel(i, aNumFmt);
            }
            nMask <<= 1;
        }
        aRatioCB .Enable();
        aSizeFT .Enable();
        aMultFT.Enable();
        aWidthMF .Enable();
        aHeightMF.Enable();
        SetMetricValue(aWidthMF, aSize.Width(), eCoreUnit);
        SetMetricValue(aHeightMF, aSize.Height(), eCoreUnit);
        aOrientFT.Enable();
        aOrientLB.Enable();
        SetModified();
        //needed due to asynchronous loading of graphics in the SvxBrushItem
        aInvalidateTimer.Start();
    }
    return 0;
}

IMPL_LINK_NOARG(SvxNumOptionsTabPage, PopupActivateHdl_Impl)
{
    if(!bMenuButtonInitialized)
    {
        bMenuButtonInitialized = sal_True;
        EnterWait();
        PopupMenu* pPopup = aBitmapMB.GetPopupMenu()->GetPopupMenu( MN_GALLERY );

        if(GalleryExplorer::FillObjList(GALLERY_THEME_BULLETS, aGrfNames))
        {
            pPopup->RemoveItem( pPopup->GetItemPos( NUM_NO_GRAPHIC ));
            GalleryExplorer::BeginLocking(GALLERY_THEME_BULLETS);

            Graphic aGraphic;
            String sGrfName;
            std::vector<String>::const_iterator it = aGrfNames.begin();
            for(sal_uInt16 i = 0; it != aGrfNames.end(); ++it, ++i)
            {
                sGrfName = *it;
                INetURLObject aObj(sGrfName);
                if(aObj.GetProtocol() == INET_PROT_FILE)
                    sGrfName = aObj.PathToFileName();

                if(GalleryExplorer::GetGraphicObj( GALLERY_THEME_BULLETS, i, &aGraphic))
                {
                    Bitmap aBitmap(aGraphic.GetBitmap());
                    Size aSize(aBitmap.GetSizePixel());
                    if(aSize.Width() > MAX_BMP_WIDTH ||
                        aSize.Height() > MAX_BMP_HEIGHT)
                    {
                        sal_Bool bWidth = aSize.Width() > aSize.Height();
                        double nScale = bWidth ?
                                            (double)MAX_BMP_WIDTH / (double)aSize.Width():
                                                (double)MAX_BMP_HEIGHT / (double)aSize.Height();
                        aBitmap.Scale(nScale, nScale);
                    }
                    Image aImage(aBitmap);

                    pPopup->InsertItem(MN_GALLERY_ENTRY + i, sGrfName, aImage );
                }
                else
                {
                    Image aImage;
                    pPopup->InsertItem(
                        MN_GALLERY_ENTRY + i, sGrfName, aImage );
                }
            }
            GalleryExplorer::EndLocking(GALLERY_THEME_BULLETS);
        }
        LeaveWait();
    }
    return 0;
}

IMPL_LINK_NOARG(SvxNumOptionsTabPage, BulletHdl_Impl)
{
    SvxCharacterMap* pMap = new SvxCharacterMap( this, sal_True );

    sal_uInt16 nMask = 1;
    const Font* pFmtFont = 0;
    sal_Bool bSameBullet = sal_True;
    sal_Unicode cBullet = 0;
    sal_Bool bFirst = sal_True;
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            const SvxNumberFormat&  rCurFmt = pActNum->GetLevel(i);
            if(bFirst)
            {
                 cBullet = rCurFmt.GetBulletChar();
            }
            else if(rCurFmt.GetBulletChar() != cBullet )
            {
                bSameBullet = sal_False;
                break;
            }
            if(!pFmtFont)
                pFmtFont = rCurFmt.GetBulletFont();
            bFirst = sal_False;
        }
        nMask <<= 1;

    }

    if(pFmtFont)
        pMap->SetCharFont(*pFmtFont);
    else
        pMap->SetCharFont(aActBulletFont);
    if(bSameBullet)
        pMap->SetChar( cBullet );
    if(pMap->Execute() == RET_OK)
    {
        // change Font Numrules
        aActBulletFont = pMap->GetCharFont();

        sal_uInt16 _nMask = 1;
        for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & _nMask)
            {
                SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
                aNumFmt.SetBulletFont(&aActBulletFont); ;
                aNumFmt.SetBulletChar( (sal_Unicode) pMap->GetChar() );
                pActNum->SetLevel(i, aNumFmt);
            }
            _nMask <<= 1;
        }

        SetModified();
    }
    delete pMap;
    return 0;
}

IMPL_LINK( SvxNumOptionsTabPage, SizeHdl_Impl, MetricField *, pField)
{
    sal_Bool bWidth = pField == &aWidthMF;
    bLastWidthModified = bWidth;
    sal_Bool bRatio = aRatioCB.IsChecked();
    long nWidthVal = static_cast<long>(aWidthMF.Denormalize(aWidthMF.GetValue(FUNIT_100TH_MM)));
    long nHeightVal = static_cast<long>(aHeightMF.Denormalize(aHeightMF.GetValue(FUNIT_100TH_MM)));
    nWidthVal = OutputDevice::LogicToLogic( nWidthVal ,
                                                MAP_100TH_MM, (MapUnit)eCoreUnit );
    nHeightVal = OutputDevice::LogicToLogic( nHeightVal,
                                                MAP_100TH_MM, (MapUnit)eCoreUnit);
    double  fSizeRatio;

    sal_Bool bRepaint = sal_False;
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            if(SVX_NUM_BITMAP == (aNumFmt.GetNumberingType()&(~LINK_TOKEN)))
            {
                Size aSize(aNumFmt.GetGraphicSize() );
                Size aSaveSize(aSize);

                if (aInitSize[i].Height())
                    fSizeRatio = (double)aInitSize[i].Width() / (double)aInitSize[i].Height();
                else
                    fSizeRatio = (double)1;

                if(bWidth)
                {
                    long nDelta = nWidthVal - aInitSize[i].Width();
                    aSize.Width() = nWidthVal;
                    if (bRatio)
                    {
                        aSize.Height() = aInitSize[i].Height() + (long)((double)nDelta / fSizeRatio);
                        aHeightMF.SetUserValue(aHeightMF.Normalize(
                            OutputDevice::LogicToLogic( aSize.Height(), (MapUnit)eCoreUnit, MAP_100TH_MM )),
                                FUNIT_100TH_MM);
                    }
                }
                else
                {
                    long nDelta = nHeightVal - aInitSize[i].Height();
                    aSize.Height() = nHeightVal;
                    if (bRatio)
                    {
                        aSize.Width() = aInitSize[i].Width() + (long)((double)nDelta * fSizeRatio);
                        aWidthMF.SetUserValue(aWidthMF.Normalize(
                            OutputDevice::LogicToLogic( aSize.Width(), (MapUnit)eCoreUnit, MAP_100TH_MM )),
                                FUNIT_100TH_MM);
                    }
                }
                const SvxBrushItem* pBrushItem =  aNumFmt.GetBrush();
                sal_Int16 eOrient = aNumFmt.GetVertOrient();
                if(aSize != aSaveSize)
                    bRepaint = sal_True;
                aNumFmt.SetGraphicBrush( pBrushItem, &aSize, &eOrient );
                pActNum->SetLevel(i, aNumFmt);
            }
        }
        nMask <<= 1;
    }
    SetModified(bRepaint);
    return 0;
}

IMPL_LINK( SvxNumOptionsTabPage, RatioHdl_Impl, CheckBox *, pBox )
{
    if (pBox->IsChecked())
    {
        if (bLastWidthModified)
            SizeHdl_Impl(&aWidthMF);
        else
            SizeHdl_Impl(&aHeightMF);
    }
    return 0;
}

IMPL_LINK_NOARG(SvxNumOptionsTabPage, CharFmtHdl_Impl)
{
    bAutomaticCharStyles = sal_False;
    sal_uInt16 nEntryPos = aCharFmtLB.GetSelectEntryPos();
    String sEntry = aCharFmtLB.GetSelectEntry();
    sal_uInt16 nMask = 1;
    String aEmptyStr;
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            if( 0 == nEntryPos )
                aNumFmt.SetCharFmtName(aEmptyStr);
            else
            {
                if(SVX_NUM_BITMAP != (aNumFmt.GetNumberingType()&(~LINK_TOKEN)))
                    aNumFmt.SetCharFmtName(sEntry);
            }
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified(sal_False);
    return 0;

};

IMPL_LINK( SvxNumOptionsTabPage, EditModifyHdl_Impl, Edit *, pEdit )
{
    sal_Bool bPrefix = pEdit == &aPrefixED;
    sal_Bool bSuffix = pEdit == &aSuffixED;
    sal_Bool bStart = pEdit == &aStartED;
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            if(bPrefix)
                aNumFmt.SetPrefix( aPrefixED.GetText() );
            else if(bSuffix)
                aNumFmt.SetSuffix( aSuffixED.GetText() );
            else if(bStart)
                aNumFmt.SetStart( (sal_uInt16)aStartED.GetValue() );
            else //align
            {
                sal_uInt16 nPos = aAlignLB.GetSelectEntryPos();
                SvxAdjust eAdjust = SVX_ADJUST_CENTER;
                if(nPos == 0)
                    eAdjust = SVX_ADJUST_LEFT;
                else if(nPos == 2)
                    eAdjust = SVX_ADJUST_RIGHT;
                aNumFmt.SetNumAdjust( eAdjust );
            }
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();

    return 0;
}

static sal_uInt16 lcl_DrawGraphic(VirtualDevice* pVDev, const SvxNumberFormat &rFmt, sal_uInt16 nXStart,
                        sal_uInt16 nYStart, sal_uInt16 nDivision)
{
    const SvxBrushItem* pBrushItem = rFmt.GetBrush();
    sal_uInt16 nRet = 0;
    if(pBrushItem)
    {
        const Graphic* pGrf = pBrushItem->GetGraphic();
        if(pGrf)
        {
            Size aGSize( rFmt.GetGraphicSize() );
            aGSize.Width() /= nDivision;
            nRet = (sal_uInt16)aGSize.Width();
            aGSize.Height() /= nDivision;
            pGrf->Draw( pVDev, Point(nXStart,nYStart),
                    pVDev->PixelToLogic( aGSize ) );
        }
    }
    return nRet;

}

static sal_uInt16 lcl_DrawBullet(VirtualDevice* pVDev,
            const SvxNumberFormat& rFmt, sal_uInt16 nXStart,
            sal_uInt16 nYStart, const Size& rSize)
{
    Font aTmpFont(pVDev->GetFont());

    // via Uno it's possible that no font has been set!
    Font aFont(rFmt.GetBulletFont() ? *rFmt.GetBulletFont() : aTmpFont);
    Size aTmpSize(rSize);
    aTmpSize.Width() *= rFmt.GetBulletRelSize();
    aTmpSize.Width() /= 100 ;
    aTmpSize.Height() *= rFmt.GetBulletRelSize();
    aTmpSize.Height() /= 100 ;
    // in case of a height of zero it is drawed in original height
    if(!aTmpSize.Height())
        aTmpSize.Height() = 1;
    aFont.SetSize(aTmpSize);
    aFont.SetTransparent(sal_True);
    Color aBulletColor = rFmt.GetBulletColor();
    if(aBulletColor.GetColor() == COL_AUTO)
        aBulletColor = Color(pVDev->GetFillColor().IsDark() ? COL_WHITE : COL_BLACK);
    else if(aBulletColor == pVDev->GetFillColor())
        aBulletColor.Invert();
    aFont.SetColor(aBulletColor);
    pVDev->SetFont( aFont );
    rtl::OUString aText(rFmt.GetBulletChar());
    long nY = nYStart;
    nY -= ((aTmpSize.Height() - rSize.Height())/ 2);
    pVDev->DrawText( Point(nXStart, nY), aText );
    sal_uInt16 nRet = (sal_uInt16)pVDev->GetTextWidth(aText);

    pVDev->SetFont(aTmpFont);
    return nRet;
}

// paint preview of numeration
void    SvxNumberingPreview::Paint( const Rectangle& /*rRect*/ )
{
    Size aSize(PixelToLogic(GetOutputSizePixel()));
    Rectangle aRect(Point(0,0), aSize);

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const Color aBackColor = rStyleSettings.GetFieldColor();
    const Color aTextColor = rStyleSettings.GetFieldTextColor();

    VirtualDevice* pVDev = new VirtualDevice(*this);
    pVDev->EnableRTL( IsRTLEnabled() );
    pVDev->SetMapMode(GetMapMode());
    pVDev->SetOutputSize( aSize );

    Color aLineColor(COL_LIGHTGRAY);
    if(aLineColor == aBackColor)
        aLineColor.Invert();
    pVDev->SetLineColor(aLineColor);
    pVDev->SetFillColor( aBackColor );
    pVDev->DrawRect(aRect);

    if(pActNum)
    {
        sal_uInt16 nWidthRelation;
        if(nPageWidth)
        {
            nWidthRelation = sal_uInt16 (nPageWidth / aSize.Width());
            if(bPosition)
                nWidthRelation = nWidthRelation * 2 / 3;
            else
                nWidthRelation = nWidthRelation / 4;
        }
        else
            nWidthRelation = 30; // chapter dialog

        // height per level
        sal_uInt16 nXStep = sal::static_int_cast< sal_uInt16 >(aSize.Width() / (3 * pActNum->GetLevelCount()));
        if(pActNum->GetLevelCount() < 10)
            nXStep /= 2;
        sal_uInt16 nYStart = 4;
        // the whole height mustn't be used for a single level
        sal_uInt16 nYStep = sal::static_int_cast< sal_uInt16 >((aSize.Height() - 6)/ (pActNum->GetLevelCount() > 1 ? pActNum->GetLevelCount() : 5));
        aStdFont = OutputDevice::GetDefaultFont(
                DEFAULTFONT_UI_SANS, MsLangId::getSystemLanguage(), DEFAULTFONT_FLAGS_ONLYONE);
        aStdFont.SetColor(aTextColor);
        aStdFont.SetFillColor(aBackColor);

        sal_uInt16 nFontHeight = nYStep * 6 / 10;
        if(bPosition)
            nFontHeight = nYStep * 15 / 10;
        aStdFont.SetSize(Size( 0, nFontHeight ));

        SvxNodeNum aNum( (sal_uInt8)0 );
        sal_uInt16 nPreNum = pActNum->GetLevel(0).GetStart();

        if(bPosition)
        {
            sal_uInt16 nLineHeight = nFontHeight * 8 / 7;
            sal_uInt8 nStart = 0;
            while( !(nActLevel & (1<<nStart)) )
            {
                nStart++;
            }
            if(nStart)
                nStart--;
            sal_uInt8 nEnd = std::min( (sal_uInt8)(nStart + 3), (sal_uInt8)pActNum->GetLevelCount() );
            for( sal_uInt8 nLevel = nStart; nLevel < nEnd; ++nLevel )
            {
                const SvxNumberFormat &rFmt = pActNum->GetLevel(nLevel);
                aNum.GetLevelVal()[ nLevel ] = rFmt.GetStart();

                sal_uInt16 nXStart( 0 );
                short nTextOffset( 0 );
                sal_uInt16 nNumberXPos( 0 );
                if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    nXStart = rFmt.GetAbsLSpace() / nWidthRelation;
                    nTextOffset = rFmt.GetCharTextDistance() / nWidthRelation;
                    nNumberXPos = nXStart;
                    sal_uInt16 nFirstLineOffset = (-rFmt.GetFirstLineOffset()) / nWidthRelation;

                    if(nFirstLineOffset <= nNumberXPos)
                        nNumberXPos = nNumberXPos - nFirstLineOffset;
                    else
                        nNumberXPos = 0;
                    // in draw this is valid
                    if(nTextOffset < 0)
                        nNumberXPos = nNumberXPos + nTextOffset;
                }
                else if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    const long nTmpNumberXPos( ( rFmt.GetIndentAt() +
                                                 rFmt.GetFirstLineIndent() ) /
                                               nWidthRelation );
                    if ( nTmpNumberXPos < 0 )
                    {
                        nNumberXPos = 0;
                    }
                    else
                    {
                        nNumberXPos = static_cast<sal_uInt16>( nTmpNumberXPos );
                    }
                }

                sal_uInt16 nBulletWidth = 0;
                if( SVX_NUM_BITMAP == (rFmt.GetNumberingType() &(~LINK_TOKEN)))
                {
                    nBulletWidth = rFmt.IsShowSymbol() ? lcl_DrawGraphic(pVDev, rFmt,
                                        nNumberXPos,
                                            nYStart, nWidthRelation) : 0;
                }
                else if( SVX_NUM_CHAR_SPECIAL == rFmt.GetNumberingType() )
                {
                    nBulletWidth =  rFmt.IsShowSymbol() ?
                     lcl_DrawBullet(pVDev, rFmt, nNumberXPos, nYStart, aStdFont.GetSize()) : 0;
                }
                else
                {
                    pVDev->SetFont(aStdFont);
                    aNum.SetLevel( nLevel );
                    if(pActNum->IsContinuousNumbering())
                        aNum.GetLevelVal()[nLevel] = nPreNum;
                    String aText(pActNum->MakeNumString( aNum ));
                    Font aSaveFont = pVDev->GetFont();
                    Font aColorFont(aSaveFont);
                    Color aTmpBulletColor = rFmt.GetBulletColor();
                    if(aTmpBulletColor.GetColor() == COL_AUTO)
                        aTmpBulletColor = Color(aBackColor.IsDark() ? COL_WHITE : COL_BLACK);
                    else if(aTmpBulletColor == aBackColor)
                        aTmpBulletColor.Invert();
                    aColorFont.SetColor(aTmpBulletColor);
                    pVDev->SetFont(aColorFont);
                    pVDev->DrawText( Point(nNumberXPos, nYStart), aText );
                    pVDev->SetFont(aSaveFont);
                    nBulletWidth = (sal_uInt16)pVDev->GetTextWidth(aText);
                    nPreNum++;
                }
                if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT &&
                     rFmt.GetLabelFollowedBy() == SvxNumberFormat::SPACE )
                {
                    pVDev->SetFont(aStdFont);
                    rtl::OUString aText(' ');
                    pVDev->DrawText( Point(nNumberXPos, nYStart), aText );
                    nBulletWidth = nBulletWidth + (sal_uInt16)pVDev->GetTextWidth(aText);
                }

                sal_uInt16 nTextXPos( 0 );
                if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    nTextXPos = nXStart;
                    if(nTextOffset < 0)
                         nTextXPos = nTextXPos + nTextOffset;
                    if(nNumberXPos + nBulletWidth + nTextOffset > nTextXPos )
                        nTextXPos = nNumberXPos + nBulletWidth + nTextOffset;
                }
                else if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    switch ( rFmt.GetLabelFollowedBy() )
                    {
                        case SvxNumberFormat::LISTTAB:
                        {
                            nTextXPos = static_cast<sal_uInt16>(
                                            rFmt.GetListtabPos() / nWidthRelation );
                            if ( nTextXPos < nNumberXPos + nBulletWidth )
                            {
                                nTextXPos = nNumberXPos + nBulletWidth;
                            }
                        }
                        break;
                        case SvxNumberFormat::SPACE:
                        case SvxNumberFormat::NOTHING:
                        {
                            nTextXPos = nNumberXPos + nBulletWidth;
                        }
                        break;
                    }

                    nXStart = static_cast<sal_uInt16>( rFmt.GetIndentAt() / nWidthRelation );
                }

                Rectangle aRect1(Point(nTextXPos, nYStart + nFontHeight / 2), Size(aSize.Width() / 2, 2));
                pVDev->SetFillColor( aBackColor );
                pVDev->DrawRect( aRect1 );

                Rectangle aRect2(Point(nXStart, nYStart + nLineHeight + nFontHeight / 2 ), Size(aSize.Width() / 2, 2));
                pVDev->DrawRect( aRect2 );
                nYStart += 2 * nLineHeight;
            }
        }
        else
        {
            //#i5153# painting gray or black rectangles as 'normal' numbering text
            String sMsg( RTL_CONSTASCII_USTRINGPARAM( "Preview") );
            long nWidth = pVDev->GetTextWidth(sMsg);
            long nTextHeight = pVDev->GetTextHeight();
            long nRectHeight = nTextHeight * 2 / 3;
            long nTopOffset = nTextHeight - nRectHeight;
            Color aBlackColor(COL_BLACK);
            if(aBlackColor == aBackColor)
                aBlackColor.Invert();

            for( sal_uInt8 nLevel = 0; nLevel < pActNum->GetLevelCount();
                            ++nLevel, nYStart = nYStart + nYStep )
            {
                const SvxNumberFormat &rFmt = pActNum->GetLevel(nLevel);
                aNum.GetLevelVal()[ nLevel ] = rFmt.GetStart();
                sal_uInt16 nXStart( 0 );
                if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    nXStart = rFmt.GetAbsLSpace() / nWidthRelation;
                }
                else if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    const long nTmpXStart( ( rFmt.GetIndentAt() +
                                             rFmt.GetFirstLineIndent() ) /
                                           nWidthRelation );
                    if ( nTmpXStart < 0 )
                    {
                        nXStart = 0;
                    }
                    else
                    {
                        nXStart = static_cast<sal_uInt16>(nTmpXStart);
                    }
                }
                nXStart /= 2;
                nXStart += 2;
                sal_uInt16 nTextOffset = 2 * nXStep;
                if( SVX_NUM_BITMAP == (rFmt.GetNumberingType()&(~LINK_TOKEN)) )
                {
                    if(rFmt.IsShowSymbol())
                    {
                        nTextOffset = lcl_DrawGraphic(pVDev, rFmt, nXStart, nYStart, nWidthRelation);
                        nTextOffset = nTextOffset + nXStep;
                    }
                }
                else if( SVX_NUM_CHAR_SPECIAL == rFmt.GetNumberingType() )
                {
                    if(rFmt.IsShowSymbol())
                    {
                        nTextOffset =  lcl_DrawBullet(pVDev, rFmt, nXStart, nYStart, aStdFont.GetSize());
                        nTextOffset = nTextOffset + nXStep;
                    }
                }
                else
                {
                    Font aColorFont(aStdFont);
                    Color aTmpBulletColor = rFmt.GetBulletColor();
                    if(aTmpBulletColor.GetColor() == COL_AUTO)
                        aTmpBulletColor = Color(aBackColor.IsDark() ? COL_WHITE : COL_BLACK);
                    else if(aTmpBulletColor == aBackColor)
                        aTmpBulletColor.Invert();
                    aColorFont.SetColor(aTmpBulletColor);
                    pVDev->SetFont(aColorFont);
                    aNum.SetLevel( nLevel );
                    if(pActNum->IsContinuousNumbering())
                        aNum.GetLevelVal()[nLevel] = nPreNum;
                    String aText(pActNum->MakeNumString( aNum ));
                    pVDev->DrawText( Point(nXStart, nYStart), aText );
                    pVDev->SetFont(aStdFont);
                    nTextOffset = (sal_uInt16)pVDev->GetTextWidth(aText);
                    nTextOffset = nTextOffset + nXStep;
                    nPreNum++;
                }
                if(pOutlineNames)
                {
                    //#i5153# outline numberings still use the style names as text
                    pVDev->SetFont(aStdFont);
                    sMsg = pOutlineNames[nLevel];
                    pVDev->DrawText( Point(nXStart + nTextOffset, nYStart), sMsg );
                }
                else
                {
                    //#i5153# the selected rectangle(s) should be black
                    if( 0 != (nActLevel & (1<<nLevel)))
                    {
                        pVDev->SetFillColor( aBlackColor );
                        pVDev->SetLineColor( aBlackColor );
                    }
                    else
                    {
                        //#i5153# unselected levels are gray
                        pVDev->SetFillColor( aLineColor );
                        pVDev->SetLineColor( aLineColor );
                    }
                    Rectangle aRect1(Point(nXStart + nTextOffset, nYStart + nTopOffset), Size(nWidth, nRectHeight));
                    pVDev->DrawRect(aRect1);
                }
            }
        }
    }
    DrawOutDev( Point(0,0), aSize,
                Point(0,0), aSize,
                        *pVDev );
    delete pVDev;

}

SvxNumPositionTabPage::SvxNumPositionTabPage(Window* pParent,
                               const SfxItemSet& rSet) :
    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_NUM_POSITION ), rSet ),
    aPositionFL(    this, CUI_RES(FL_POSITION )),
    aLevelFT(       this, CUI_RES(FT_LEVEL    )),
    aLevelLB(       this, CUI_RES(LB_LEVEL  )),
    aDistBorderFT(  this, CUI_RES(FT_BORDERDIST )),
    aDistBorderMF(  this, CUI_RES(MF_BORDERDIST )),
    aRelativeCB(    this, CUI_RES(CB_RELATIVE       )),
    aIndentFT(      this, CUI_RES(FT_INDENT     )),
    aIndentMF(      this, CUI_RES(MF_INDENT     )),
    aDistNumFT(     this, CUI_RES(FT_NUMDIST        )),
    aDistNumMF(     this, CUI_RES(MF_NUMDIST        )),
    aAlignFT(       this, CUI_RES(FT_ALIGN  )),
    aAlignLB(       this, CUI_RES(LB_ALIGN  )),
    aLabelFollowedByFT( this, CUI_RES(FT_LABEL_FOLLOWED_BY) ),
    aLabelFollowedByLB( this, CUI_RES(LB_LABEL_FOLLOWED_BY) ),
    aListtabFT( this, CUI_RES(FT_LISTTAB) ),
    aListtabMF( this, CUI_RES(MF_LISTTAB) ),
    aAlign2FT( this, CUI_RES(FT_ALIGN_2) ),
    aAlign2LB( this, CUI_RES(LB_ALIGN_2) ),
    aAlignedAtFT( this, CUI_RES(FT_ALIGNED_AT) ),
    aAlignedAtMF( this, CUI_RES(MF_ALIGNED_AT) ),
    aIndentAtFT( this, CUI_RES(FT_INDENT_AT) ),
    aIndentAtMF( this, CUI_RES(MF_INDENT_AT) ),
    aStandardPB(    this, CUI_RES(PB_STANDARD       )),
    pPreviewWIN(    new SvxNumberingPreview(this, CUI_RES(WIN_PREVIEW   ))),
    pActNum(0),
    pSaveNum(0),
    nActNumLvl( USHRT_MAX ),
    nNumItemId(SID_ATTR_NUMBERING_RULE),
        bModified(false),
        bPreset(false),
    bInInintControl(sal_False),
    bLabelAlignmentPosAndSpaceModeActive( false )
{
    FreeResource();
    SetExchangeSupport();
    pPreviewWIN->SetBackground(Wallpaper(Color(COL_TRANSPARENT)));

    aRelativeCB.Check();
    aAlignLB.SetSelectHdl(LINK(this, SvxNumPositionTabPage, EditModifyHdl_Impl));
    aAlign2LB.SetSelectHdl(LINK(this, SvxNumPositionTabPage, EditModifyHdl_Impl));
    for ( sal_uInt16 i = 0; i < aAlignLB.GetEntryCount(); ++i )
    {
        aAlign2LB.InsertEntry( aAlignLB.GetEntry( i ) );
    }
    aAlign2LB.SetDropDownLineCount( aAlign2LB.GetEntryCount() );
    aAlign2FT.SetText( aAlignFT.GetText() );

    Link aLk = LINK(this, SvxNumPositionTabPage, DistanceHdl_Impl);

    aDistBorderMF.SetUpHdl(aLk);
    aDistBorderMF.SetDownHdl(aLk);
    aDistBorderMF.SetLoseFocusHdl(aLk);

    aDistNumMF.SetUpHdl(aLk);
    aDistNumMF.SetDownHdl(aLk);
    aDistNumMF.SetLoseFocusHdl(aLk);

    aIndentMF.SetUpHdl(aLk);
    aIndentMF.SetDownHdl(aLk);
    aIndentMF.SetLoseFocusHdl(aLk);

    aLabelFollowedByLB.SetDropDownLineCount( aLabelFollowedByLB.GetEntryCount() );
    aLabelFollowedByLB.SetSelectHdl( LINK(this, SvxNumPositionTabPage, LabelFollowedByHdl_Impl) );

    aLk = LINK(this, SvxNumPositionTabPage, ListtabPosHdl_Impl);
    aListtabMF.SetUpHdl(aLk);
    aListtabMF.SetDownHdl(aLk);
    aListtabMF.SetLoseFocusHdl(aLk);

    aLk = LINK(this, SvxNumPositionTabPage, AlignAtHdl_Impl);
    aAlignedAtMF.SetUpHdl(aLk);
    aAlignedAtMF.SetDownHdl(aLk);
    aAlignedAtMF.SetLoseFocusHdl(aLk);

    aLk = LINK(this, SvxNumPositionTabPage, IndentAtHdl_Impl);
    aIndentAtMF.SetUpHdl(aLk);
    aIndentAtMF.SetDownHdl(aLk);
    aIndentAtMF.SetLoseFocusHdl(aLk);

    aLevelLB.SetSelectHdl(LINK(this, SvxNumPositionTabPage, LevelHdl_Impl));
    aRelativeCB.SetClickHdl(LINK(this, SvxNumPositionTabPage, RelativeHdl_Impl));
    aStandardPB.SetClickHdl(LINK(this, SvxNumPositionTabPage, StandardHdl_Impl));


    aRelativeCB.Check(bLastRelative);
    pPreviewWIN->SetPositionMode();
    eCoreUnit = rSet.GetPool()->GetMetric(rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE));

#if OSL_DEBUG_LEVEL > 1
    pDebugFixedText = new FixedText(this, 0);
    pDebugFixedText->Show();
    Size aSize(200, 20);
    Point aPos(250,0);

    pDebugFixedText->SetPosSizePixel(aPos, aSize);
    pDebugFixedText->SetText(rtl::OUString("Das ist ein Debug-Text"));
#endif

    aStandardPB.SetAccessibleRelationMemberOf(&aPositionFL);
}

SvxNumPositionTabPage::~SvxNumPositionTabPage()
{
    delete pActNum;
    delete pPreviewWIN;
    delete pSaveNum;
#if OSL_DEBUG_LEVEL > 1
    delete pDebugFixedText;
#endif
}
/*-------------------------------------------------------*/

#if OSL_DEBUG_LEVEL > 1
void lcl_PrintDebugOutput(FixedText& rFixed, const SvxNumberFormat& rNumFmt)
{
#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))

    sal_Char const sHash[] = " # ";
    if ( rNumFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        String sDebugText( UniString::CreateFromInt32( TWIP_TO_MM100(rNumFmt.GetAbsLSpace() ) ) );
        sDebugText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( sHash ) );
        sDebugText += UniString::CreateFromInt32( TWIP_TO_MM100(rNumFmt.GetCharTextDistance() ) );
        sDebugText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( sHash ) );
        sDebugText += UniString::CreateFromInt32( TWIP_TO_MM100(rNumFmt.GetFirstLineOffset() ) );
        rFixed.SetText(sDebugText);
    }
    else if ( rNumFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        String sDebugText( UniString::CreateFromInt32( TWIP_TO_MM100(rNumFmt.GetListtabPos() ) ) );
        sDebugText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( sHash ) );
        sDebugText += UniString::CreateFromInt32( TWIP_TO_MM100(rNumFmt.GetFirstLineIndent() ) );
        sDebugText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( sHash ) );
        sDebugText += UniString::CreateFromInt32( TWIP_TO_MM100(rNumFmt.GetIndentAt() ) );
        rFixed.SetText(sDebugText);
    }

}
#endif

void SvxNumPositionTabPage::InitControls()
{
    bInInintControl = sal_True;
    const bool bRelative = !bLabelAlignmentPosAndSpaceModeActive &&
                     aRelativeCB.IsEnabled() && aRelativeCB.IsChecked();
    const bool bSingleSelection = aLevelLB.GetSelectEntryCount() == 1 &&
                            USHRT_MAX != nActNumLvl;

    aDistBorderMF.Enable( !bLabelAlignmentPosAndSpaceModeActive &&
                          ( bSingleSelection || bRelative ) );
    aDistBorderFT.Enable( !bLabelAlignmentPosAndSpaceModeActive &&
                          ( bSingleSelection || bRelative ) );

    bool bSetDistEmpty = false;
    bool bSameDistBorderNum = !bLabelAlignmentPosAndSpaceModeActive;
    bool bSameDist      = !bLabelAlignmentPosAndSpaceModeActive;
    bool bSameIndent    = !bLabelAlignmentPosAndSpaceModeActive;
    bool bSameAdjust    = true;

    bool bSameLabelFollowedBy = bLabelAlignmentPosAndSpaceModeActive;
    bool bSameListtab = bLabelAlignmentPosAndSpaceModeActive;
    bool bSameAlignAt = bLabelAlignmentPosAndSpaceModeActive;
    bool bSameIndentAt = bLabelAlignmentPosAndSpaceModeActive;

    const SvxNumberFormat* aNumFmtArr[SVX_MAX_NUM];
    sal_uInt16 nMask = 1;
    sal_uInt16 nLvl = USHRT_MAX;
    long nFirstBorderTextRelative = -1;
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        aNumFmtArr[i] = &pActNum->GetLevel(i);
        if(nActNumLvl & nMask)
        {
            if(USHRT_MAX == nLvl)
                nLvl = i;

            if( i > nLvl)
            {
                bSameAdjust &= aNumFmtArr[i]->GetNumAdjust() == aNumFmtArr[nLvl]->GetNumAdjust();
                if ( !bLabelAlignmentPosAndSpaceModeActive )
                {
                    if(bRelative)
                    {
                        if(nFirstBorderTextRelative == -1)
                            nFirstBorderTextRelative =
                            (aNumFmtArr[i]->GetAbsLSpace() + aNumFmtArr[i]->GetFirstLineOffset() -
                            aNumFmtArr[i - 1]->GetAbsLSpace() + aNumFmtArr[i - 1]->GetFirstLineOffset());
                        else
                            bSameDistBorderNum &= nFirstBorderTextRelative ==
                            (aNumFmtArr[i]->GetAbsLSpace() + aNumFmtArr[i]->GetFirstLineOffset() -
                            aNumFmtArr[i - 1]->GetAbsLSpace() + aNumFmtArr[i - 1]->GetFirstLineOffset());
                    }
                    else
                        bSameDistBorderNum &=
                        aNumFmtArr[i]->GetAbsLSpace() - aNumFmtArr[i]->GetFirstLineOffset() ==
                        aNumFmtArr[i - 1]->GetAbsLSpace() - aNumFmtArr[i - 1]->GetFirstLineOffset();

                    bSameDist       &= aNumFmtArr[i]->GetCharTextDistance() == aNumFmtArr[nLvl]->GetCharTextDistance();
                    bSameIndent     &= aNumFmtArr[i]->GetFirstLineOffset() == aNumFmtArr[nLvl]->GetFirstLineOffset();
                }
                else
                {
                    bSameLabelFollowedBy &=
                        aNumFmtArr[i]->GetLabelFollowedBy() == aNumFmtArr[nLvl]->GetLabelFollowedBy();
                    bSameListtab &=
                        aNumFmtArr[i]->GetListtabPos() == aNumFmtArr[nLvl]->GetListtabPos();
                    bSameAlignAt &=
                        ( ( aNumFmtArr[i]->GetIndentAt() + aNumFmtArr[i]->GetFirstLineIndent() )
                            == ( aNumFmtArr[nLvl]->GetIndentAt() + aNumFmtArr[nLvl]->GetFirstLineIndent() ) );
                    bSameIndentAt &=
                        aNumFmtArr[i]->GetIndentAt() == aNumFmtArr[nLvl]->GetIndentAt();
                }
            }
        }
        nMask <<= 1;

    }
    if (SVX_MAX_NUM <= nLvl)
    {
        OSL_ENSURE(false, "cannot happen.");
        return;
    }

    if(bSameDistBorderNum)
    {
        long nDistBorderNum;
        if(bRelative)
        {
            nDistBorderNum = (long)aNumFmtArr[nLvl]->GetAbsLSpace()+ aNumFmtArr[nLvl]->GetFirstLineOffset();
            if(nLvl)
                nDistBorderNum -= (long)aNumFmtArr[nLvl - 1]->GetAbsLSpace()+ aNumFmtArr[nLvl - 1]->GetFirstLineOffset();
        }
        else
        {
            nDistBorderNum = (long)aNumFmtArr[nLvl]->GetAbsLSpace()+ aNumFmtArr[nLvl]->GetFirstLineOffset();
        }
        SetMetricValue(aDistBorderMF, nDistBorderNum, eCoreUnit);
    }
    else
        bSetDistEmpty = true;

#if OSL_DEBUG_LEVEL > 1
    lcl_PrintDebugOutput(*pDebugFixedText, *aNumFmtArr[nLvl]);
#endif

    const String aEmptyStr;
    if(bSameDist)
        SetMetricValue(aDistNumMF, aNumFmtArr[nLvl]->GetCharTextDistance(), eCoreUnit);
    else
        aDistNumMF.SetText(aEmptyStr);
    if(bSameIndent)
        SetMetricValue(aIndentMF, - aNumFmtArr[nLvl]->GetFirstLineOffset(), eCoreUnit);
    else
        aIndentMF.SetText(aEmptyStr);

    if(bSameAdjust)
    {
        sal_uInt16 nPos = 1; // centered
        if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_LEFT)
            nPos = 0;
        else if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_RIGHT)
            nPos = 2;
        aAlignLB.SelectEntryPos(nPos);
        aAlign2LB.SelectEntryPos( nPos );
    }
    else
    {
        aAlignLB.SetNoSelection();
        aAlign2LB.SetNoSelection();
    }

    if ( bSameLabelFollowedBy )
    {
        sal_uInt16 nPos = 0; // LISTTAB
        if ( aNumFmtArr[nLvl]->GetLabelFollowedBy() == SvxNumberFormat::SPACE )
        {
            nPos = 1;
        }
        else if ( aNumFmtArr[nLvl]->GetLabelFollowedBy() == SvxNumberFormat::NOTHING )
        {
            nPos = 2;
        }
        aLabelFollowedByLB.SelectEntryPos( nPos );
    }
    else
    {
        aLabelFollowedByLB.SetNoSelection();
    }

    if ( aNumFmtArr[nLvl]->GetLabelFollowedBy() == SvxNumberFormat::LISTTAB )
    {
        aListtabFT.Enable( true );
        aListtabMF.Enable( true );
        if ( bSameListtab )
        {
            SetMetricValue( aListtabMF, aNumFmtArr[nLvl]->GetListtabPos(), eCoreUnit );
        }
        else
        {
            aListtabMF.SetText(aEmptyStr);
        }
    }
    else
    {
        aListtabFT.Enable( false );
        aListtabMF.Enable( false );
        aListtabMF.SetText(aEmptyStr);
    }

    if ( bSameAlignAt )
    {
        SetMetricValue( aAlignedAtMF,
                        aNumFmtArr[nLvl]->GetIndentAt() + aNumFmtArr[nLvl]->GetFirstLineIndent(),
                        eCoreUnit );
    }
    else
    {
        aAlignedAtMF.SetText(aEmptyStr);
    }

    if ( bSameIndentAt )
    {
        SetMetricValue( aIndentAtMF, aNumFmtArr[nLvl]->GetIndentAt(), eCoreUnit );
    }
    else
    {
        aIndentAtMF.SetText(aEmptyStr);
    }

    if ( bSetDistEmpty )
        aDistBorderMF.SetText(aEmptyStr);

    bInInintControl = sal_False;
}

void SvxNumPositionTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    sal_uInt16 nTmpNumLvl = USHRT_MAX;
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    if(pExampleSet)
    {
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_NUM_PRESET, sal_False, &pItem))
            bPreset = ((const SfxBoolItem*)pItem)->GetValue();
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_CUR_NUM_LEVEL, sal_False, &pItem))
            nTmpNumLvl = ((const SfxUInt16Item*)pItem)->GetValue();
    }
    if(SFX_ITEM_SET == rSet.GetItemState(nNumItemId, sal_False, &pItem))
    {
        delete pSaveNum;
        pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());
    }
    bModified = (!pActNum->Get( 0 ) || bPreset);
    if(*pSaveNum != *pActNum ||
        nActNumLvl != nTmpNumLvl )
    {
        *pActNum = *pSaveNum;
        nActNumLvl = nTmpNumLvl;
        sal_uInt16 nMask = 1;
        aLevelLB.SetUpdateMode(sal_False);
        aLevelLB.SetNoSelection();
        aLevelLB.SelectEntryPos( pActNum->GetLevelCount(), nActNumLvl == USHRT_MAX);
        if(nActNumLvl != USHRT_MAX)
            for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
            {
                if(nActNumLvl & nMask)
                    aLevelLB.SelectEntryPos( i, sal_True);
                nMask <<= 1 ;
            }
        aRelativeCB.Enable(nActNumLvl != 1);
        aLevelLB.SetUpdateMode(sal_True);

        InitPosAndSpaceMode();
        ShowControlsDependingOnPosAndSpaceMode();

        InitControls();
    }
    pPreviewWIN->SetLevel(nActNumLvl);
    pPreviewWIN->Invalidate();
}

int  SvxNumPositionTabPage::DeactivatePage(SfxItemSet *_pSet)
{
    if(_pSet)
    {
        if(aDistBorderMF.IsEnabled())
            DistanceHdl_Impl(&aDistBorderMF);
        DistanceHdl_Impl(&aIndentMF);
        FillItemSet(*_pSet);
    }
    return sal_True;
}

sal_Bool SvxNumPositionTabPage::FillItemSet( SfxItemSet& rSet )
{
    rSet.Put(SfxUInt16Item(SID_PARAM_CUR_NUM_LEVEL, nActNumLvl));

    if(bModified && pActNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SvxNumBulletItem( *pSaveNum ), nNumItemId);
        rSet.Put(SfxBoolItem(SID_PARAM_NUM_PRESET, sal_False));
    }
    return bModified;
}

void SvxNumPositionTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    // in Draw the item exists as WhichId, in Writer only as SlotId
    SfxItemState eState = rSet.GetItemState(SID_ATTR_NUMBERING_RULE, sal_False, &pItem);
    if(eState != SFX_ITEM_SET)
    {
        nNumItemId = rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        eState = rSet.GetItemState(nNumItemId, sal_False, &pItem);

        if( eState != SFX_ITEM_SET )
        {
            pItem = &static_cast< const SvxNumBulletItem& >( rSet.Get( nNumItemId, sal_True ) );
            eState = SFX_ITEM_SET;
        }

    }
    DBG_ASSERT(eState == SFX_ITEM_SET, "kein Item gefunden!");
    delete pSaveNum;
    pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());

    // insert levels
    if(!aLevelLB.GetEntryCount())
    {
        for(sal_uInt16 i = 1; i <= pSaveNum->GetLevelCount(); i++)
            aLevelLB.InsertEntry(UniString::CreateFromInt32(i));
        if(pSaveNum->GetLevelCount() > 1)
        {
            String sEntry(rtl::OUString("1 - "));
            sEntry.Append( UniString::CreateFromInt32( pSaveNum->GetLevelCount() ) );
            aLevelLB.InsertEntry(sEntry);
            aLevelLB.SelectEntry(sEntry);
        }
        else
            aLevelLB.SelectEntryPos(0);
    }
    else
        aLevelLB.SelectEntryPos(aLevelLB.GetEntryCount() - 1);
    sal_uInt16 nMask = 1;
    aLevelLB.SetUpdateMode(sal_False);
    aLevelLB.SetNoSelection();
    if(nActNumLvl == USHRT_MAX)
    {
        aLevelLB.SelectEntryPos( pSaveNum->GetLevelCount(), sal_True);
    }
    else
        for(sal_uInt16 i = 0; i < pSaveNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
                aLevelLB.SelectEntryPos( i, sal_True);
            nMask <<= 1;
        }
    aLevelLB.SetUpdateMode(sal_True);

    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
    pPreviewWIN->SetNumRule(pActNum);

    InitPosAndSpaceMode();
    ShowControlsDependingOnPosAndSpaceMode();

    InitControls();
    bModified = sal_False;
}

void SvxNumPositionTabPage::InitPosAndSpaceMode()
{
    if ( pActNum == 0 )
    {
        DBG_ASSERT( false,
                "<SvxNumPositionTabPage::InitPosAndSpaceMode()> - misusage of method -> <pAktNum> has to be already set!" );
        return;
    }

    SvxNumberFormat::SvxNumPositionAndSpaceMode ePosAndSpaceMode =
                                            SvxNumberFormat::LABEL_ALIGNMENT;
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < pActNum->GetLevelCount(); ++i )
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt( pActNum->GetLevel(i) );
            ePosAndSpaceMode = aNumFmt.GetPositionAndSpaceMode();
            if ( ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                break;
            }
        }
        nMask <<= 1;
    }

    bLabelAlignmentPosAndSpaceModeActive =
                    ePosAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT;
}

void SvxNumPositionTabPage::ShowControlsDependingOnPosAndSpaceMode()
{
    aDistBorderFT.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aDistBorderMF.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aRelativeCB.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aIndentFT.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aIndentMF.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aDistNumFT.Show( !bLabelAlignmentPosAndSpaceModeActive &&
                     pActNum->IsFeatureSupported(NUM_CONTINUOUS) );
    aDistNumMF.Show( !bLabelAlignmentPosAndSpaceModeActive &&
                     pActNum->IsFeatureSupported(NUM_CONTINUOUS));
    aAlignFT.Show( !bLabelAlignmentPosAndSpaceModeActive );
    aAlignLB.Show( !bLabelAlignmentPosAndSpaceModeActive );

    aLabelFollowedByFT.Show( bLabelAlignmentPosAndSpaceModeActive );
    aLabelFollowedByLB.Show( bLabelAlignmentPosAndSpaceModeActive );
    aListtabFT.Show( bLabelAlignmentPosAndSpaceModeActive );
    aListtabMF.Show( bLabelAlignmentPosAndSpaceModeActive );
    aAlign2FT.Show( bLabelAlignmentPosAndSpaceModeActive );
    aAlign2LB.Show( bLabelAlignmentPosAndSpaceModeActive );
    aAlignedAtFT.Show( bLabelAlignmentPosAndSpaceModeActive );
    aAlignedAtMF.Show( bLabelAlignmentPosAndSpaceModeActive );
    aIndentAtFT.Show( bLabelAlignmentPosAndSpaceModeActive );
    aIndentAtMF.Show( bLabelAlignmentPosAndSpaceModeActive );
}

SfxTabPage* SvxNumPositionTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SvxNumPositionTabPage(pParent, rAttrSet);
}

void    SvxNumPositionTabPage::SetMetric(FieldUnit eMetric)
{
    if(eMetric == FUNIT_MM)
    {
        aDistBorderMF .SetDecimalDigits(1);
        aDistNumMF    .SetDecimalDigits(1);
        aIndentMF     .SetDecimalDigits(1);
        aListtabMF.SetDecimalDigits(1);
        aAlignedAtMF.SetDecimalDigits(1);
        aIndentAtMF.SetDecimalDigits(1);
    }
    aDistBorderMF .SetUnit( eMetric );
    aDistNumMF    .SetUnit( eMetric );
    aIndentMF     .SetUnit( eMetric );
    aListtabMF.SetUnit( eMetric );
    aAlignedAtMF.SetUnit( eMetric );
    aIndentAtMF.SetUnit( eMetric );
}

IMPL_LINK_NOARG(SvxNumPositionTabPage, EditModifyHdl_Impl)
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));

            const sal_uInt16 nPos = aAlignLB.IsVisible()
                                ? aAlignLB.GetSelectEntryPos()
                                : aAlign2LB.GetSelectEntryPos();
            SvxAdjust eAdjust = SVX_ADJUST_CENTER;
            if(nPos == 0)
                eAdjust = SVX_ADJUST_LEFT;
            else if(nPos == 2)
                eAdjust = SVX_ADJUST_RIGHT;
            aNumFmt.SetNumAdjust( eAdjust );
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

IMPL_LINK( SvxNumPositionTabPage, LevelHdl_Impl, ListBox *, pBox )
{
    sal_uInt16 nSaveNumLvl = nActNumLvl;
    nActNumLvl = 0;
    if(pBox->IsEntryPosSelected( pActNum->GetLevelCount() ) &&
            (pBox->GetSelectEntryCount() == 1 || nSaveNumLvl != 0xffff))
    {
        nActNumLvl = 0xFFFF;
        pBox->SetUpdateMode(sal_False);
        for( sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++ )
            pBox->SelectEntryPos( i, sal_False );
        pBox->SetUpdateMode(sal_True);
    }
    else if(pBox->GetSelectEntryCount())
    {
        sal_uInt16 nMask = 1;
        for( sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++ )
        {
            if(pBox->IsEntryPosSelected( i ))
                nActNumLvl |= nMask;
            nMask <<= 1;
        }
        pBox->SelectEntryPos( pActNum->GetLevelCount(), sal_False );
    }
    else
    {
        nActNumLvl = nSaveNumLvl;
        sal_uInt16 nMask = 1;
        for( sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++ )
        {
            if(nActNumLvl & nMask)
            {
                pBox->SelectEntryPos(i);
                break;
            }
            nMask <<=1;
        }
    }
    aRelativeCB.Enable(nActNumLvl != 1);
    SetModified();
    InitPosAndSpaceMode();
    ShowControlsDependingOnPosAndSpaceMode();
    InitControls();
    return 0;
}

IMPL_LINK( SvxNumPositionTabPage, DistanceHdl_Impl, MetricField *, pFld )
{
    if(bInInintControl)
        return 0;
    long nValue = GetCoreValue(*pFld, eCoreUnit);
    sal_uInt16 nMask = 1;
#if OSL_DEBUG_LEVEL > 1
    sal_Bool bFirst = sal_True;
#endif
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt( pActNum->GetLevel( i ) );
            if(pFld == &aDistBorderMF)
            {

                if(aRelativeCB.IsChecked())
                {
                    if(0 == i)
                    {
                        long nTmp = aNumFmt.GetFirstLineOffset();
                        aNumFmt.SetAbsLSpace( sal_uInt16(nValue - nTmp));
                    }
                    else
                    {
                        long nTmp = pActNum->GetLevel( i - 1 ).GetAbsLSpace() +
                                    pActNum->GetLevel( i - 1 ).GetFirstLineOffset() -
                                    pActNum->GetLevel( i ).GetFirstLineOffset();

                        aNumFmt.SetAbsLSpace( sal_uInt16(nValue + nTmp));
                    }
                }
                else
                {
                    aNumFmt.SetAbsLSpace( (short)nValue - aNumFmt.GetFirstLineOffset());
                }
            }
            else if(pFld == &aDistNumMF)
            {
                aNumFmt.SetCharTextDistance( (short)nValue );
            }
            else if(pFld == &aIndentMF)
            {
                // together with the FirstLineOffset the AbsLSpace must be changed, too
                long nDiff = nValue + aNumFmt.GetFirstLineOffset();
                long nAbsLSpace = aNumFmt.GetAbsLSpace();
                aNumFmt.SetAbsLSpace(sal_uInt16(nAbsLSpace + nDiff));
                aNumFmt.SetFirstLineOffset( -(short)nValue );
            }

#if OSL_DEBUG_LEVEL > 1
            if(bFirst)
                lcl_PrintDebugOutput(*pDebugFixedText, aNumFmt);
            bFirst = sal_False;
#endif
            pActNum->SetLevel( i, aNumFmt );
        }
        nMask <<= 1;
    }

    SetModified();
    if(!aDistBorderMF.IsEnabled())
    {
        String aEmptyStr;
        aDistBorderMF.SetText(aEmptyStr);
    }

    return 0;
}

IMPL_LINK( SvxNumPositionTabPage, RelativeHdl_Impl, CheckBox *, pBox )
{
    sal_Bool bOn = pBox->IsChecked();
    sal_Bool bSingleSelection = aLevelLB.GetSelectEntryCount() == 1 && USHRT_MAX != nActNumLvl;
    sal_Bool bSetValue = sal_False;
    long nValue = 0;
    if(bOn || bSingleSelection)
    {
        sal_uInt16 nMask = 1;
        sal_Bool bFirst = sal_True;
        bSetValue = sal_True;
        for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                const SvxNumberFormat &rNumFmt = pActNum->GetLevel(i);
                if(bFirst)
                {
                    nValue = rNumFmt.GetAbsLSpace() + rNumFmt.GetFirstLineOffset();
                    if(bOn && i)
                        nValue -= (pActNum->GetLevel(i - 1).GetAbsLSpace() + pActNum->GetLevel(i - 1).GetFirstLineOffset());
                }
                else
                    bSetValue = nValue ==
                        (rNumFmt.GetAbsLSpace() + rNumFmt.GetFirstLineOffset()) -
                            (pActNum->GetLevel(i - 1).GetAbsLSpace() + pActNum->GetLevel(i - 1).GetFirstLineOffset());
                bFirst = sal_False;
            }
            nMask <<= 1;
        }

    }
    String aEmptyStr;
    if(bSetValue)
        SetMetricValue(aDistBorderMF, nValue,   eCoreUnit);
    else
        aDistBorderMF.SetText(aEmptyStr);
    aDistBorderMF.Enable(bOn || bSingleSelection);
    aDistBorderFT.Enable(bOn || bSingleSelection);
    bLastRelative = bOn;
    return 0;
}

IMPL_LINK_NOARG(SvxNumPositionTabPage, LabelFollowedByHdl_Impl)
{
    // determine value to be set at the chosen list levels
    SvxNumberFormat::LabelFollowedBy eLabelFollowedBy = SvxNumberFormat::LISTTAB;
    {
        const sal_uInt16 nPos = aLabelFollowedByLB.GetSelectEntryPos();
        if ( nPos == 1 )
        {
            eLabelFollowedBy = SvxNumberFormat::SPACE;
        }
        else if ( nPos == 2 )
        {
            eLabelFollowedBy = SvxNumberFormat::NOTHING;
        }
    }

    // set value at the chosen list levels
    bool bSameListtabPos = true;
    sal_uInt16 nFirstLvl = USHRT_MAX;
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < pActNum->GetLevelCount(); ++i )
    {
        if ( nActNumLvl & nMask )
        {
            SvxNumberFormat aNumFmt( pActNum->GetLevel(i) );
            aNumFmt.SetLabelFollowedBy( eLabelFollowedBy );
            pActNum->SetLevel( i, aNumFmt );

            if ( nFirstLvl == USHRT_MAX )
            {
                nFirstLvl = i;
            }
            else
            {
                bSameListtabPos &= aNumFmt.GetListtabPos() ==
                        pActNum->GetLevel( nFirstLvl ).GetListtabPos();
            }
        }
        nMask <<= 1;
    }

    // enable/disable metric field for list tab stop position depending on
    // selected item following the list label.
    aListtabFT.Enable( eLabelFollowedBy == SvxNumberFormat::LISTTAB );
    aListtabMF.Enable( eLabelFollowedBy == SvxNumberFormat::LISTTAB );
    if ( bSameListtabPos && eLabelFollowedBy == SvxNumberFormat::LISTTAB )
    {
        SetMetricValue( aListtabMF, pActNum->GetLevel( nFirstLvl ).GetListtabPos(), eCoreUnit );
    }
    else
    {
        aListtabMF.SetText( String() );
    }

    SetModified();

    return 0;
}

IMPL_LINK( SvxNumPositionTabPage, ListtabPosHdl_Impl, MetricField*, pFld )
{
    // determine value to be set at the chosen list levels
    const long nValue = GetCoreValue( *pFld, eCoreUnit );

    // set value at the chosen list levels
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < pActNum->GetLevelCount(); ++i )
    {
        if ( nActNumLvl & nMask )
        {
            SvxNumberFormat aNumFmt( pActNum->GetLevel(i) );
            aNumFmt.SetListtabPos( nValue );
            pActNum->SetLevel( i, aNumFmt );
#if OSL_DEBUG_LEVEL > 1
            lcl_PrintDebugOutput(*pDebugFixedText, aNumFmt);
#endif
        }
        nMask <<= 1;
    }

    SetModified();

    return 0;
}

IMPL_LINK( SvxNumPositionTabPage, AlignAtHdl_Impl, MetricField*, pFld )
{
    // determine value to be set at the chosen list levels
    const long nValue = GetCoreValue( *pFld, eCoreUnit );

    // set value at the chosen list levels
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < pActNum->GetLevelCount(); ++i )
    {
        if ( nActNumLvl & nMask )
        {
            SvxNumberFormat aNumFmt( pActNum->GetLevel(i) );
            const long nFirstLineIndent = nValue - aNumFmt.GetIndentAt();
            aNumFmt.SetFirstLineIndent( nFirstLineIndent );
            pActNum->SetLevel( i, aNumFmt );
#if OSL_DEBUG_LEVEL > 1
            lcl_PrintDebugOutput(*pDebugFixedText, aNumFmt);
#endif
        }
        nMask <<= 1;
    }

    SetModified();

    return 0;
}

IMPL_LINK( SvxNumPositionTabPage, IndentAtHdl_Impl, MetricField*, pFld )
{
    // determine value to be set at the chosen list levels
    const long nValue = GetCoreValue( *pFld, eCoreUnit );

    // set value at the chosen list levels
    sal_uInt16 nMask = 1;
    for( sal_uInt16 i = 0; i < pActNum->GetLevelCount(); ++i )
    {
        if ( nActNumLvl & nMask )
        {
            SvxNumberFormat aNumFmt( pActNum->GetLevel(i) );
            const long nAlignedAt = aNumFmt.GetIndentAt() +
                                    aNumFmt.GetFirstLineIndent();
            aNumFmt.SetIndentAt( nValue );
            const long nNewFirstLineIndent = nAlignedAt - nValue;
            aNumFmt.SetFirstLineIndent( nNewFirstLineIndent );
            pActNum->SetLevel( i, aNumFmt );
#if OSL_DEBUG_LEVEL > 1
            lcl_PrintDebugOutput(*pDebugFixedText, aNumFmt);
#endif
        }
        nMask <<= 1;
    }

    SetModified();

    return 0;
}

IMPL_LINK_NOARG(SvxNumPositionTabPage, StandardHdl_Impl)
{
    sal_uInt16 nMask = 1;
    SvxNumRule aTmpNumRule( pActNum->GetFeatureFlags(),
                            pActNum->GetLevelCount(),
                            pActNum->IsContinuousNumbering(),
                            SVX_RULETYPE_NUMBERING,
                            pActNum->GetLevel( 0 ).GetPositionAndSpaceMode() );
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt( pActNum->GetLevel( i ) );
            SvxNumberFormat aTempFmt(aTmpNumRule.GetLevel( i ));
            aNumFmt.SetPositionAndSpaceMode( aTempFmt.GetPositionAndSpaceMode() );
            if ( aTempFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aNumFmt.SetAbsLSpace( aTempFmt.GetAbsLSpace() );
                aNumFmt.SetCharTextDistance( aTempFmt.GetCharTextDistance() );
                aNumFmt.SetFirstLineOffset( aTempFmt.GetFirstLineOffset() );
            }
            else if ( aTempFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aNumFmt.SetNumAdjust( aTempFmt.GetNumAdjust() );
                aNumFmt.SetLabelFollowedBy( aTempFmt.GetLabelFollowedBy() );
                aNumFmt.SetListtabPos( aTempFmt.GetListtabPos() );
                aNumFmt.SetFirstLineIndent( aTempFmt.GetFirstLineIndent() );
                aNumFmt.SetIndentAt( aTempFmt.GetIndentAt() );
            }

            pActNum->SetLevel( i, aNumFmt );
        }
        nMask <<= 1;
    }

    InitControls();
    SetModified();
    return 0;
}

void SvxNumPositionTabPage::SetModified(sal_Bool bRepaint)
{
    bModified = sal_True;
    if(bRepaint)
    {
        pPreviewWIN->SetLevel(nActNumLvl);
        pPreviewWIN->Invalidate();
    }
}

void SvxNumOptionsTabPage::SetModified(sal_Bool bRepaint)
{
    bModified = sal_True;
    if(bRepaint)
    {
        pPreviewWIN->SetLevel(nActNumLvl);
        pPreviewWIN->Invalidate();
    }
}


void SvxNumOptionsTabPage::PageCreated(SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pListItem,SfxStringListItem,SID_CHAR_FMT_LIST_BOX,sal_False);
    SFX_ITEMSET_ARG (&aSet,pNumCharFmt,SfxStringItem,SID_NUM_CHAR_FMT,sal_False);
    SFX_ITEMSET_ARG (&aSet,pBulletCharFmt,SfxStringItem,SID_BULLET_CHAR_FMT,sal_False);
    SFX_ITEMSET_ARG (&aSet,pMetricItem,SfxAllEnumItem,SID_METRIC_ITEM,sal_False);

    if (pNumCharFmt &&pBulletCharFmt)
        SetCharFmts( pNumCharFmt->GetValue(),pBulletCharFmt->GetValue());

    if (pListItem)
    {
        ListBox& myCharFmtLB = GetCharFmtListBox();
        const std::vector<String> &aList = (pListItem)->GetList();
        sal_uInt32 nCount = aList.size();;
        for(sal_uInt32 i = 0; i < nCount; i++)
            myCharFmtLB.InsertEntry(aList[i]);
    }
    if (pMetricItem)
        SetMetric(static_cast<FieldUnit>(pMetricItem->GetValue()));
}

void SvxNumPositionTabPage::PageCreated(SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pMetricItem,SfxAllEnumItem,SID_METRIC_ITEM,sal_False);

    if (pMetricItem)
        SetMetric(static_cast<FieldUnit>(pMetricItem->GetValue()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
