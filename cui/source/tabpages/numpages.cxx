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
#include <i18nlangtag/mslangid.hxx>
#include <helpid.hrc>
#include <editeng/numitem.hxx>
#include <svl/eitem.hxx>
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <svx/gallery.hxx>
#include <svl/urihelper.hxx>
#include <editeng/brushitem.hxx>
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
#include <sfx2/htmlmode.hxx>
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

#define NUM_PAGETYPE_BULLET         0
#define NUM_PAGETYPE_SINGLENUM      1
#define NUM_PAGETYPE_NUM            2

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
    static Font aDefBulletFont( OUString("StarSymbol"),
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
                               const SfxItemSet& rSet)
    : SfxTabPage(pParent, "PickNumberingPage", "cui/ui/picknumberingpage.ui", rSet)
    , pActNum(0)
    , pSaveNum(0)
    , nActNumLvl(USHRT_MAX)
    , bModified(false)
    , bPreset(false)
    , nNumItemId(SID_ATTR_NUMBERING_RULE)
{
    SetExchangeSupport();
    get(m_pExamplesVS, "valueset");
    m_pExamplesVS->init(NUM_PAGETYPE_SINGLENUM);
    m_pExamplesVS->SetSelectHdl(LINK(this, SvxSingleNumPickTabPage, NumSelectHdl_Impl));
    m_pExamplesVS->SetDoubleClickHdl(LINK(this, SvxSingleNumPickTabPage, DoubleClickHdl_Impl));

    Reference<XDefaultNumberingProvider> xDefNum = lcl_GetNumberingProvider();
    if(xDefNum.is())
    {
        Sequence< Sequence< PropertyValue > > aNumberings;
        const Locale& rLocale = Application::GetSettings().GetLanguageTag().getLocale();
        try
        {
            aNumberings =
                xDefNum->getDefaultContinuousNumberingLevels( rLocale );


            sal_Int32 nLength = aNumberings.getLength() > NUM_VALUSET_COUNT ? NUM_VALUSET_COUNT :aNumberings.getLength();

            const Sequence<PropertyValue>* pValuesArr = aNumberings.getConstArray();
            for(sal_Int32 i = 0; i < nLength; i++)
            {
                SvxNumSettings_Impl* pNew = lcl_CreateNumSettingsPtr(pValuesArr[i]);
                aNumSettingsArr.push_back(pNew);
            }
        }
        catch(const Exception&)
        {
        }
        Reference<XNumberingFormatter> xFormat(xDefNum, UNO_QUERY);
        m_pExamplesVS->SetNumberingSettings(aNumberings, xFormat, rLocale);
    }
}

SvxSingleNumPickTabPage::~SvxSingleNumPickTabPage()
{
    delete pActNum;
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
        m_pExamplesVS->SetNoSelection();
    }

    if(pActNum && (!lcl_IsNumFmtSet(pActNum, nActNumLvl) || bIsPreset))
    {
        m_pExamplesVS->SelectItem(1);
        NumSelectHdl_Impl(m_pExamplesVS);
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
        sal_uInt16 nIdx = m_pExamplesVS->GetSelectItemId() - 1;
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
    NumSelectHdl_Impl(m_pExamplesVS);
    PushButton& rOk = GetTabDialog()->GetOKButton();
    rOk.GetClickHdl().Call(&rOk);
    return 0;
}


SvxBulletPickTabPage::SvxBulletPickTabPage(Window* pParent,
                               const SfxItemSet& rSet)
    : SfxTabPage(pParent, "PickBulletPage", "cui/ui/pickbulletpage.ui", rSet)
    , pActNum(0)
    , pSaveNum(0)
    , nActNumLvl(USHRT_MAX)
    , bModified(false)
    , bPreset(false)
    , nNumItemId(SID_ATTR_NUMBERING_RULE)
{
    SetExchangeSupport();
    get(m_pExamplesVS, "valueset");
    m_pExamplesVS->init(NUM_PAGETYPE_BULLET),
    m_pExamplesVS->SetSelectHdl(LINK(this, SvxBulletPickTabPage, NumSelectHdl_Impl));
    m_pExamplesVS->SetDoubleClickHdl(LINK(this, SvxBulletPickTabPage, DoubleClickHdl_Impl));
}

SvxBulletPickTabPage::~SvxBulletPickTabPage()
{
    delete pActNum;
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
        m_pExamplesVS->SetNoSelection();
    }

    if(pActNum && (!lcl_IsNumFmtSet(pActNum, nActNumLvl) || bIsPreset))
    {
        m_pExamplesVS->SelectItem(1);
        NumSelectHdl_Impl(m_pExamplesVS);
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
        sal_Unicode cChar = aBulletTypes[m_pExamplesVS->GetSelectItemId() - 1];
        Font& rActBulletFont = lcl_GetDefaultBulletFont();

        sal_uInt16 nMask = 1;
        for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                SvxNumberFormat aFmt(pActNum->GetLevel(i));
                aFmt.SetNumberingType( SVX_NUM_CHAR_SPECIAL );
                // #i93908# clear suffix for bullet lists
                aFmt.SetPrefix( OUString() );
                aFmt.SetSuffix( OUString() );
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
    NumSelectHdl_Impl(m_pExamplesVS);
    PushButton& rOk = GetTabDialog()->GetOKButton();
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
                               const SfxItemSet& rSet)
    : SfxTabPage(pParent, "PickOutlinePage", "cui/ui/pickoutlinepage.ui", rSet)
    , pActNum(0)
    , pSaveNum(0)
    , nActNumLvl(USHRT_MAX)
    , nNumItemId(SID_ATTR_NUMBERING_RULE)
    , bModified(false)
    , bPreset(false)
{
    SetExchangeSupport();

    get(m_pExamplesVS, "valueset");
    m_pExamplesVS->init(NUM_PAGETYPE_NUM);
    m_pExamplesVS->SetSelectHdl(LINK(this, SvxNumPickTabPage, NumSelectHdl_Impl));
    m_pExamplesVS->SetDoubleClickHdl(LINK(this, SvxNumPickTabPage, DoubleClickHdl_Impl));

    Reference<XDefaultNumberingProvider> xDefNum = lcl_GetNumberingProvider();
    if(xDefNum.is())
    {
        Sequence<Reference<XIndexAccess> > aOutlineAccess;
        const Locale& rLocale = Application::GetSettings().GetLanguageTag().getLocale();
        try
        {
            aOutlineAccess = xDefNum->getDefaultOutlineNumberings( rLocale );

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
        catch(const Exception&)
        {
        }
        Reference<XNumberingFormatter> xFormat(xDefNum, UNO_QUERY);
        m_pExamplesVS->SetOutlineNumberingSettings(aOutlineAccess, xFormat, rLocale);
    }
}

SvxNumPickTabPage::~SvxNumPickTabPage()
{
    delete pActNum;
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
        m_pExamplesVS->SetNoSelection();
    }

    if(pActNum && (!lcl_IsNumFmtSet(pActNum, nActNumLvl) || bIsPreset))
    {
        m_pExamplesVS->SelectItem(1);
        NumSelectHdl_Impl(m_pExamplesVS);
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

        SvxNumSettingsArr_Impl& rItemArr = aNumSettingsArrays[m_pExamplesVS->GetSelectItemId() - 1];

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
                aFmt.SetPrefix(OUString());
                aFmt.SetSuffix(OUString());
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
    NumSelectHdl_Impl(m_pExamplesVS);
    PushButton& rOk = GetTabDialog()->GetOKButton();
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
                               const SfxItemSet& rSet)
    : SfxTabPage(pParent, "PickGraphicPage", "cui/ui/pickgraphicpage.ui", rSet)
    , pActNum(0)
    , pSaveNum(0)
    , nActNumLvl(USHRT_MAX)
    , nNumItemId(SID_ATTR_NUMBERING_RULE)
    , bModified(false)
    , bPreset(false)
{
    SetExchangeSupport();
    get(m_pErrorText, "errorft");
    get(m_pExamplesVS, "valueset");
    m_pExamplesVS->SetSelectHdl(LINK(this, SvxBitmapPickTabPage, NumSelectHdl_Impl));
    m_pExamplesVS->SetDoubleClickHdl(LINK(this, SvxBitmapPickTabPage, DoubleClickHdl_Impl));

    eCoreUnit = rSet.GetPool()->GetMetric(rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE));

    // determine graphic name
    GalleryExplorer::FillObjList(GALLERY_THEME_BULLETS, aGrfNames);

    sal_uInt16 i = 0;
    for(std::vector<String>::iterator it = aGrfNames.begin(); it != aGrfNames.end(); ++it, ++i)
    {
        m_pExamplesVS->InsertItem( i + 1, i);

        INetURLObject aObj(*it);
        if(aObj.GetProtocol() == INET_PROT_FILE)
            *it = aObj.PathToFileName();

        m_pExamplesVS->SetItemText( i + 1, *it );
    }

    if(aGrfNames.empty())
    {
        m_pErrorText->Show();
    }
    else
    {
        m_pExamplesVS->Show();
        m_pExamplesVS->Format();
    }
}

SvxBitmapPickTabPage::~SvxBitmapPickTabPage()
{
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
        m_pExamplesVS->SetNoSelection();
    }

    if(!aGrfNames.empty() &&
        (pActNum && (!lcl_IsNumFmtSet(pActNum, nActNumLvl) || bIsPreset)))
    {
        m_pExamplesVS->SelectItem(1);
        NumSelectHdl_Impl(m_pExamplesVS);
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
}

IMPL_LINK_NOARG(SvxBitmapPickTabPage, NumSelectHdl_Impl)
{
    if(pActNum)
    {
        bPreset = sal_False;
        bModified = sal_True;
        sal_uInt16 nIdx = m_pExamplesVS->GetSelectItemId() - 1;

        sal_uInt16 nMask = 1;
        String aEmptyStr;
        sal_uInt16 nSetNumberingType = SVX_NUM_BITMAP;
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
    NumSelectHdl_Impl(m_pExamplesVS);
    PushButton& rOk = GetTabDialog()->GetOKButton();
    rOk.GetClickHdl().Call(&rOk);
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

// tabpage numbering options
SvxNumOptionsTabPage::SvxNumOptionsTabPage(Window* pParent,
                               const SfxItemSet& rSet)
    : SfxTabPage(pParent, "NumberingOptionsPage", "cui/ui/numberingoptionspage.ui", rSet)
    , pActNum(0)
    , pSaveNum(0)
    , bLastWidthModified(sal_False)
    , bModified(sal_False)
    , bPreset(sal_False)
    , bAutomaticCharStyles(sal_True)
    , bHTMLMode(sal_False)
    , bMenuButtonInitialized(sal_False)
    , nBullet(0xff)
    , nActNumLvl(USHRT_MAX)
    , nNumItemId(SID_ATTR_NUMBERING_RULE)
{
    get(m_pLevelLB, "levellb");
    get(m_pFmtLB, "numfmtlb");

    get(m_pSeparatorFT, "separator");
    get(m_pPrefixFT, "prefixft");
    get(m_pPrefixED, "prefix");
    get(m_pSuffixFT, "suffixft");
    get(m_pSuffixED, "suffix");

    get(m_pCharFmtFT, "charstyleft");
    get(m_pCharFmtLB, "charstyle");

    get(m_pBulColorFT, "colorft");
    get(m_pBulColLB, "color");

    get(m_pBulRelSizeFT, "relsizeft");
    get(m_pBulRelSizeMF, "relsize");

    get(m_pAllLevelFT, "sublevelsft");
    get(m_pAllLevelNF, "sublevels");

    get(m_pStartFT, "startatft");
    get(m_pStartED, "startat");

    get(m_pBulletFT, "bulletft");
    get(m_pBulletPB, "bullet");

    get(m_pAlignFT, "numalignft");
    get(m_pAlignLB, "numalign");

    get(m_pBitmapFT, "bitmapft");
    get(m_pBitmapMB, "bitmap");

    get(m_pWidthFT, "widthft");
    get(m_pWidthMF, "widthmf");
    get(m_pHeightFT, "heightft");
    get(m_pHeightMF, "heightmf");

    get(m_pRatioCB, "keepratio");

    get(m_pOrientFT, "orientft");
    get(m_pOrientLB, "orientlb");

    get(m_pAllLevelsFrame, "levelsframe");
    get(m_pSameLevelCB, "allsame");

    get(m_pPreviewWIN, "preview");

    m_pPreviewWIN->SetBackground(Wallpaper(Color(COL_TRANSPARENT)));
    SetExchangeSupport();
    aActBulletFont = lcl_GetDefaultBulletFont();

    m_pBulletPB->SetClickHdl(LINK(this, SvxNumOptionsTabPage, BulletHdl_Impl));
    m_pFmtLB->SetSelectHdl(LINK(this, SvxNumOptionsTabPage, NumberTypeSelectHdl_Impl));
    m_pBitmapMB->SetSelectHdl(LINK(this, SvxNumOptionsTabPage, GraphicHdl_Impl));
    m_pLevelLB->EnableMultiSelection(true);
    m_pLevelLB->SetSelectHdl(LINK(this, SvxNumOptionsTabPage, LevelHdl_Impl));
    m_pCharFmtLB->SetSelectHdl(LINK(this, SvxNumOptionsTabPage, CharFmtHdl_Impl));
    m_pWidthMF->SetModifyHdl(LINK(this, SvxNumOptionsTabPage, SizeHdl_Impl));
    m_pHeightMF->SetModifyHdl(LINK(this, SvxNumOptionsTabPage, SizeHdl_Impl));
    m_pRatioCB->SetClickHdl(LINK(this, SvxNumOptionsTabPage, RatioHdl_Impl));
    m_pStartED->SetModifyHdl(LINK(this, SvxNumOptionsTabPage, EditModifyHdl_Impl));
    m_pPrefixED->SetModifyHdl(LINK(this, SvxNumOptionsTabPage, EditModifyHdl_Impl));
    m_pSuffixED->SetModifyHdl(LINK(this, SvxNumOptionsTabPage, EditModifyHdl_Impl));
    m_pAllLevelNF->SetModifyHdl(LINK(this,SvxNumOptionsTabPage, AllLevelHdl_Impl));
    m_pOrientLB->SetSelectHdl(LINK(this, SvxNumOptionsTabPage, OrientHdl_Impl));
    m_pSameLevelCB->SetClickHdl(LINK(this, SvxNumOptionsTabPage, SameLevelHdl_Impl));
    m_pBulRelSizeMF->SetModifyHdl(LINK(this,SvxNumOptionsTabPage, BulRelSizeHdl_Impl));
    m_pBulColLB->SetSelectHdl(LINK(this, SvxNumOptionsTabPage, BulColorHdl_Impl));
    aInvalidateTimer.SetTimeoutHdl(LINK(this, SvxNumOptionsTabPage, PreviewInvalidateHdl_Impl));
    aInvalidateTimer.SetTimeout(50);

    Menu *pBitmapMenu = m_pBitmapMB->GetPopupMenu();

    pBitmapMenu->SetHighlightHdl(LINK(this, SvxNumOptionsTabPage, PopupActivateHdl_Impl));
    m_nGalleryId = pBitmapMenu->GetItemId("gallery");
    assert(m_nGalleryId != MENU_ITEM_NOTFOUND);
    PopupMenu* pPopup = new PopupMenu;
    pBitmapMenu->SetPopupMenu(m_nGalleryId, pPopup);

    eCoreUnit = rSet.GetPool()->GetMetric(rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE));

    // Get advanced numbering types from the component.
    // Watch out for the ugly 0x88/*SVX_NUM_BITMAP|0x80*/ to not remove that.
    GetI18nNumbering( *m_pFmtLB, (SVX_NUM_BITMAP | 0x80));
}

SvxNumOptionsTabPage::~SvxNumOptionsTabPage()
{
    delete m_pBitmapMB->GetPopupMenu()->GetPopupMenu(m_nGalleryId);
    delete pActNum;
    delete pSaveNum;
}

void SvxNumOptionsTabPage::SetMetric(FieldUnit eMetric)
{
    if(eMetric == FUNIT_MM)
    {
        m_pWidthMF->SetDecimalDigits(1);
        m_pHeightMF->SetDecimalDigits(1);
    }
    m_pWidthMF->SetUnit( eMetric );
    m_pHeightMF->SetUnit( eMetric );
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
        m_pLevelLB->SetUpdateMode(sal_False);
        m_pLevelLB->SetNoSelection();
        m_pLevelLB->SelectEntryPos( pActNum->GetLevelCount(), nActNumLvl == USHRT_MAX);
        if(nActNumLvl != USHRT_MAX)
            for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
            {
                if(nActNumLvl & nMask)
                    m_pLevelLB->SelectEntryPos( i, sal_True);
                nMask <<= 1 ;
            }
        m_pLevelLB->SetUpdateMode(sal_True);
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
    if(!m_pLevelLB->GetEntryCount())
    {
        for(sal_uInt16 i = 1; i <= pSaveNum->GetLevelCount(); i++)
            m_pLevelLB->InsertEntry( OUString::number(i));
        if(pSaveNum->GetLevelCount() > 1)
        {
            OUString sEntry( "1 - " );
            sEntry += OUString::number( pSaveNum->GetLevelCount() );
            m_pLevelLB->InsertEntry(sEntry);
            m_pLevelLB->SelectEntry(sEntry);
        }
        else
            m_pLevelLB->SelectEntryPos(0);
    }
    else
        m_pLevelLB->SelectEntryPos(m_pLevelLB->GetEntryCount() - 1);

    sal_uInt16 nMask = 1;
    m_pLevelLB->SetUpdateMode(sal_False);
    m_pLevelLB->SetNoSelection();
    if(nActNumLvl == USHRT_MAX)
    {
        m_pLevelLB->SelectEntryPos( pSaveNum->GetLevelCount(), sal_True);
    }
    else
        for(sal_uInt16 i = 0; i < pSaveNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
                m_pLevelLB->SelectEntryPos( i, sal_True);
            nMask <<= 1 ;
        }
    m_pLevelLB->SetUpdateMode(sal_True);

    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
    m_pPreviewWIN->SetNumRule(pActNum);
    m_pSameLevelCB->Check(pActNum->IsContinuousNumbering());

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

        m_pBulColLB->InsertEntry( Color( COL_AUTO ), SVX_RESSTR( RID_SVXSTR_AUTOMATIC ));

        for ( long i = 0; i < pColorTable->Count(); i++ )
        {
            XColorEntry* pEntry = pColorTable->GetColor(i);
            m_pBulColLB->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
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
    m_pCharFmtFT->Show(bCharFmt);
    m_pCharFmtLB->Show(bCharFmt);

    sal_Bool bContinuous = pActNum->IsFeatureSupported(NUM_CONTINUOUS);

    sal_Bool bAllLevel = bContinuous && !bHTMLMode;
    m_pAllLevelFT->Show(bAllLevel);
    m_pAllLevelNF->Show(bAllLevel);

    m_pAllLevelsFrame->Show(bContinuous);

    // again misusage: in Draw there is numeration only until the bitmap
    // without SVX_NUM_NUMBER_NONE
    //remove types that are unsupported by Draw/Impress
    if(!bContinuous)
    {
        sal_uInt16 nFmtCount = m_pFmtLB->GetEntryCount();
        for(sal_uInt16 i = nFmtCount; i; i--)
        {
            sal_uInt16 nEntryData = (sal_uInt16)(sal_uLong)m_pFmtLB->GetEntryData(i - 1);
            if(/*SVX_NUM_NUMBER_NONE == nEntryData ||*/
                ((SVX_NUM_BITMAP|LINK_TOKEN) ==  nEntryData))
                m_pFmtLB->RemoveEntry(i - 1);
        }
    }
    //one must be enabled
    if(!pActNum->IsFeatureSupported(NUM_ENABLE_LINKED_BMP))
    {
        sal_IntPtr nData = SVX_NUM_BITMAP|LINK_TOKEN;
        sal_uInt16 nPos = m_pFmtLB->GetEntryPos((void*)nData);
        if(LISTBOX_ENTRY_NOTFOUND != nPos)
            m_pFmtLB->RemoveEntry(nPos);
    }
    else if(!pActNum->IsFeatureSupported(NUM_ENABLE_EMBEDDED_BMP))
    {
        sal_IntPtr nData = SVX_NUM_BITMAP;
        sal_uInt16 nPos = m_pFmtLB->GetEntryPos((void*)nData);
        if(LISTBOX_ENTRY_NOTFOUND != nPos)
            m_pFmtLB->RemoveEntry(nPos);
    }
    if(pActNum->IsFeatureSupported(NUM_SYMBOL_ALIGNMENT))
    {
        m_pAlignFT->Show();
        m_pAlignLB->Show();
        m_pAlignLB->SetSelectHdl(LINK(this, SvxNumOptionsTabPage, EditModifyHdl_Impl));
    }
    else
    {
        m_pAlignFT->Hide();
        m_pAlignLB->Hide();
    }

    // MegaHack: because of a not-fixable 'design mistake/error' in Impress
    // delete all kinds of numeric enumerations
    if(pActNum->IsFeatureSupported(NUM_NO_NUMBERS))
    {
        sal_uInt16 nFmtCount = m_pFmtLB->GetEntryCount();
        for(sal_uInt16 i = nFmtCount; i; i--)
        {
            sal_uInt16 nEntryData = (sal_uInt16)(sal_uLong)m_pFmtLB->GetEntryData(i - 1);
            if( /*nEntryData >= SVX_NUM_CHARS_UPPER_LETTER &&*/  nEntryData <= SVX_NUM_NUMBER_NONE)
                m_pFmtLB->RemoveEntry(i - 1);
        }
    }

    InitControls();
    bModified = sal_False;

}

void SvxNumOptionsTabPage::InitControls()
{
    bool bShowBullet    = true;
    bool bShowBitmap    = true;
    bool bSameType      = true;
    bool bSameStart     = true;
    bool bSamePrefix    = true;
    bool bSameSuffix    = true;
    bool bAllLevel      = true;
    sal_Bool bSameCharFmt   = true;
    sal_Bool bSameVOrient   = true;
    bool bSameSize      = true;
    bool bSameBulColor  = true;
    bool bSameBulRelSize= true;
    bool bSameAdjust    = true;

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
                bSameCharFmt    &=  (sal_Bool) (sFirstCharFmt == aNumFmtArr[i]->GetCharFmtName());
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
            m_pOrientLB->SetNoSelection();
        else
            m_pOrientLB->SelectEntryPos(
                sal::static_int_cast< sal_uInt16 >(eFirstOrient - 1));
                // no text::VertOrientation::NONE

        if(bSameSize)
        {
            SetMetricValue(*m_pHeightMF, aFirstSize.Height(), eCoreUnit);
            SetMetricValue(*m_pWidthMF, aFirstSize.Width(), eCoreUnit);
        }
        else
        {
            m_pHeightMF->SetText(aEmptyStr);
            m_pWidthMF->SetText(aEmptyStr);
        }
    }

    if(bSameType)
    {
        sal_uInt16 nLBData = (sal_uInt16) aNumFmtArr[nLvl]->GetNumberingType();
        m_pFmtLB->SelectEntryPos(m_pFmtLB->GetEntryPos( (void*)sal::static_int_cast<sal_uIntPtr>( nLBData ) ));
    }
    else
        m_pFmtLB->SetNoSelection();

    m_pAllLevelNF->Enable(nHighestLevel > 0 && !m_pSameLevelCB->IsChecked());
    m_pAllLevelNF->SetMax(nHighestLevel + 1);
    if(bAllLevel)
    {
        m_pAllLevelNF->SetValue(aNumFmtArr[nLvl]->GetIncludeUpperLevels());
    }
    else
    {
        m_pAllLevelNF->SetText(aEmptyStr);
    }
    if(bSameAdjust)
    {
        sal_uInt16 nPos = 1; // centered
        if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_LEFT)
            nPos = 0;
        else if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_RIGHT)
            nPos = 2;
        m_pAlignLB->SelectEntryPos(nPos);
    }
    else
    {
        m_pAlignLB->SetNoSelection();
    }

    if(bBullRelSize)
    {
        if(bSameBulRelSize)
            m_pBulRelSizeMF->SetValue(aNumFmtArr[nLvl]->GetBulletRelSize());
        else
            m_pBulRelSizeMF->SetText(aEmptyStr);
    }
    if(bBullColor)
    {
        if(bSameBulColor)
            m_pBulColLB->SelectEntry(aNumFmtArr[nLvl]->GetBulletColor());
        else
            m_pBulColLB->SetNoSelection();
    }
    switch(nBullet)
    {
        case SHOW_NUMBERING:
            if(bSameStart)
            {
                m_pStartED->SetValue(aNumFmtArr[nLvl]->GetStart());
            }
            else
                m_pStartED->SetText(aEmptyStr);
        break;
        case SHOW_BULLET:
        break;
        case SHOW_BITMAP:
        break;
    }

    if(bSamePrefix)
        m_pPrefixED->SetText(aNumFmtArr[nLvl]->GetPrefix());
    else
        m_pPrefixED->SetText(aEmptyStr);
    if(bSameSuffix)
        m_pSuffixED->SetText(aNumFmtArr[nLvl]->GetSuffix());
    else
        m_pSuffixED->SetText(aEmptyStr);

    if(bSameCharFmt)
    {
        if(sFirstCharFmt.Len())
                m_pCharFmtLB->SelectEntry(sFirstCharFmt);
        else
            m_pCharFmtLB->SelectEntryPos( 0 );
    }
    else
        m_pCharFmtLB->SetNoSelection();

    m_pPreviewWIN->SetLevel(nActNumLvl);
    m_pPreviewWIN->Invalidate();
}

// 0 - Number; 1 - Bullet; 2 - Bitmap
void SvxNumOptionsTabPage::SwitchNumberType( sal_uInt8 nType, sal_Bool )
{
    if(nBullet == nType)
        return;
    nBullet = nType;
    sal_Bool bBullet = (nType == SHOW_BULLET);
    sal_Bool bBitmap = (nType == SHOW_BITMAP);
    sal_Bool bEnableBitmap = (nType == SHOW_BITMAP);
    sal_Bool bNumeric = !(bBitmap||bBullet);
    m_pSeparatorFT->Show(bNumeric);
    m_pPrefixFT->Show(bNumeric);
    m_pPrefixED->Show(bNumeric);
    m_pSuffixFT->Show(bNumeric);
    m_pSuffixED->Show(bNumeric);

    sal_Bool bCharFmt = pActNum->IsFeatureSupported(NUM_CHAR_STYLE);
    m_pCharFmtFT->Show(!bBitmap && bCharFmt);
    m_pCharFmtLB->Show(!bBitmap && bCharFmt);

    // this is rather misusage, as there is no own flag
    // for complete numeration
    sal_Bool bAllLevelFeature = pActNum->IsFeatureSupported(NUM_CONTINUOUS);
    sal_Bool bAllLevel = bNumeric && bAllLevelFeature && !bHTMLMode;
    m_pAllLevelFT->Show(bAllLevel);
    m_pAllLevelNF->Show(bAllLevel);

    m_pStartFT->Show(!(bBullet||bBitmap));
    m_pStartED->Show(!(bBullet||bBitmap));

    m_pBulletFT->Show(bBullet);
    m_pBulletPB->Show(bBullet);
    sal_Bool bBullColor = pActNum->IsFeatureSupported(NUM_BULLET_COLOR);
    m_pBulColorFT->Show(!bBitmap && bBullColor);
    m_pBulColLB->Show(!bBitmap && bBullColor);
    sal_Bool bBullResSize = pActNum->IsFeatureSupported(NUM_BULLET_REL_SIZE);
    m_pBulRelSizeFT->Show(!bBitmap && bBullResSize);
    m_pBulRelSizeMF->Show(!bBitmap && bBullResSize);

    m_pBitmapFT->Show(bBitmap);
    m_pBitmapMB->Show(bBitmap);

    m_pWidthFT->Show(bBitmap);
    m_pWidthMF->Show(bBitmap);
    m_pHeightFT->Show(bBitmap);
    m_pHeightMF->Show(bBitmap);
    m_pRatioCB->Show(bBitmap);

    m_pOrientFT->Show(bBitmap &&  bAllLevelFeature);
    m_pOrientLB->Show(bBitmap &&  bAllLevelFeature);

    m_pWidthFT->Enable(bEnableBitmap);
    m_pWidthMF->Enable(bEnableBitmap);
    m_pHeightFT->Enable(bEnableBitmap);
    m_pHeightMF->Enable(bEnableBitmap);
    m_pRatioCB->Enable(bEnableBitmap);
    m_pOrientFT->Enable(bEnableBitmap);
    m_pOrientLB->Enable(bEnableBitmap);
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
    m_pPreviewWIN->Invalidate();
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
                    sSelectStyle = m_sBulletCharFmtName;
                }
            }
            else
            {
                aNumFmt.SetPrefix( m_pPrefixED->GetText() );
                aNumFmt.SetSuffix( m_pSuffixED->GetText() );
                SwitchNumberType(SHOW_NUMBERING);
                pActNum->SetLevel(i, aNumFmt);
                CheckForStartValue_Impl(nNumberingType);

                // allocation of the drawing pattern is automatic
                if(bAutomaticCharStyles)
                {
                    sSelectStyle = m_sNumCharFmtName;
                }
            }
        }
        nMask <<= 1;
    }
    sal_Bool bAllLevelFeature = pActNum->IsFeatureSupported(NUM_CONTINUOUS);
    if(bShowOrient && bAllLevelFeature)
    {
        m_pOrientFT->Show();
        m_pOrientLB->Show();
    }
    else
    {
        m_pOrientFT->Hide();
        m_pOrientLB->Hide();
    }
    SetModified();
    if(sSelectStyle.Len())
    {
        m_pCharFmtLB->SelectEntry(sSelectStyle);
        CharFmtHdl_Impl(m_pCharFmtLB);
        bAutomaticCharStyles = sal_True;
    }
    return 0;
}

void SvxNumOptionsTabPage::CheckForStartValue_Impl(sal_uInt16 nNumberingType)
{
    sal_Bool bIsNull = m_pStartED->GetValue() == 0;
    sal_Bool bNoZeroAllowed = nNumberingType < SVX_NUM_ARABIC ||
                        SVX_NUM_CHARS_UPPER_LETTER_N == nNumberingType ||
                        SVX_NUM_CHARS_LOWER_LETTER_N == nNumberingType;
    m_pStartED->SetMin(bNoZeroAllowed ? 1 : 0);
    if(bIsNull && bNoZeroAllowed)
        m_pStartED->GetModifyHdl().Call(m_pStartED);
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
    SvxOpenGraphicDialog    aGrfDlg( CUI_RES(RID_SVXSTR_EDIT_GRAPHIC) );

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
                aNumFmt.SetCharFmtName(m_sNumCharFmtName);
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
        m_pRatioCB->Enable();
        m_pWidthFT->Enable();
        m_pHeightFT->Enable();
        m_pWidthMF->Enable();
        m_pHeightMF->Enable();
        SetMetricValue(*m_pWidthMF, aSize.Width(), eCoreUnit);
        SetMetricValue(*m_pHeightMF, aSize.Height(), eCoreUnit);
        m_pOrientFT->Enable();
        m_pOrientLB->Enable();
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
        PopupMenu* pMenu = m_pBitmapMB->GetPopupMenu();
        PopupMenu* pPopup = pMenu->GetPopupMenu(m_nGalleryId);

        if (GalleryExplorer::FillObjList(GALLERY_THEME_BULLETS, aGrfNames))
        {
            pPopup->Clear();
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
    sal_Bool bWidth = pField == m_pWidthMF;
    bLastWidthModified = bWidth;
    sal_Bool bRatio = m_pRatioCB->IsChecked();
    long nWidthVal = static_cast<long>(m_pWidthMF->Denormalize(m_pWidthMF->GetValue(FUNIT_100TH_MM)));
    long nHeightVal = static_cast<long>(m_pHeightMF->Denormalize(m_pHeightMF->GetValue(FUNIT_100TH_MM)));
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
                        m_pHeightMF->SetUserValue(m_pHeightMF->Normalize(
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
                        m_pWidthMF->SetUserValue(m_pWidthMF->Normalize(
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
            SizeHdl_Impl(m_pWidthMF);
        else
            SizeHdl_Impl(m_pHeightMF);
    }
    return 0;
}

IMPL_LINK_NOARG(SvxNumOptionsTabPage, CharFmtHdl_Impl)
{
    bAutomaticCharStyles = sal_False;
    sal_uInt16 nEntryPos = m_pCharFmtLB->GetSelectEntryPos();
    String sEntry = m_pCharFmtLB->GetSelectEntry();
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
    sal_Bool bPrefix = pEdit == m_pPrefixED;
    sal_Bool bSuffix = pEdit == m_pSuffixED;
    sal_Bool bStart = pEdit == m_pStartED;
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            if(bPrefix)
                aNumFmt.SetPrefix( m_pPrefixED->GetText() );
            else if(bSuffix)
                aNumFmt.SetSuffix( m_pSuffixED->GetText() );
            else if(bStart)
                aNumFmt.SetStart( (sal_uInt16)m_pStartED->GetValue() );
            else //align
            {
                sal_uInt16 nPos = m_pAlignLB->GetSelectEntryPos();
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
    OUString aText(rFmt.GetBulletChar());
    long nY = nYStart;
    nY -= ((aTmpSize.Height() - rSize.Height())/ 2);
    pVDev->DrawText( Point(nXStart, nY), aText );
    sal_uInt16 nRet = (sal_uInt16)pVDev->GetTextWidth(aText);

    pVDev->SetFont(aTmpFont);
    return nRet;
}

SvxNumberingPreview::SvxNumberingPreview(Window* pParent, WinBits nWinBits)
    : Window(pParent, nWinBits)
    , pActNum(0)
    , nPageWidth(0)
    , pOutlineNames(0)
    , bPosition(false)
    , nActLevel(USHRT_MAX)
{
    SetBorderStyle(WINDOW_BORDER_MONO);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxNumberingPreview(Window *pParent, VclBuilder::stringmap &)
{
    return new SvxNumberingPreview(pParent, WB_BORDER);
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
                    OUString aText(' ');
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
            OUString sMsg( "Preview" );
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

//See uiconfig/swriter/ui/outlinepositionpage.ui for effectively a duplicate
//dialog to this one, except with a different preview window impl.
//TODO, determine if SwNumPositionTabPage and SvxNumPositionTabPage can be
//merged
SvxNumPositionTabPage::SvxNumPositionTabPage(Window* pParent,
                               const SfxItemSet& rSet)
    : SfxTabPage(pParent, "NumberingPositionPage", "cui/ui/numberingpositionpage.ui", rSet)
    , pActNum(0)
    , pSaveNum(0)
    , nActNumLvl(USHRT_MAX)
    , nNumItemId(SID_ATTR_NUMBERING_RULE)
    , bModified(false)
    , bPreset(false)
    , bInInintControl(false)
    , bLabelAlignmentPosAndSpaceModeActive(false)
{
    SetExchangeSupport();

    get(m_pLevelLB, "levellb");
    get(m_pDistBorderFT, "indent");
    get(m_pDistBorderMF, "indentmf");
    get(m_pRelativeCB, "relative");
    get(m_pIndentFT, "numberingwidth");
    get(m_pIndentMF, "numberingwidthmf");
    get(m_pDistNumFT, "numdist");
    get(m_pDistNumMF, "numdistmf");
    get(m_pAlignFT, "numalign");
    get(m_pAlignLB, "numalignlb");

    get(m_pLabelFollowedByFT, "numfollowedby");
    get(m_pLabelFollowedByLB, "numfollowedbylb");
    get(m_pListtabFT, "at");
    get(m_pListtabMF, "atmf");
    get(m_pAlign2FT, "num2align");
    get(m_pAlign2LB, "num2alignlb");
    get(m_pAlignedAtFT, "alignedat");
    get(m_pAlignedAtMF, "alignedatmf");
    get(m_pIndentAtFT, "indentat");
    get(m_pIndentAtMF, "indentatmf");

    get(m_pStandardPB, "standard");
    get(m_pPreviewWIN, "preview");

    m_pPreviewWIN->SetBackground(Wallpaper(Color(COL_TRANSPARENT)));

    m_pRelativeCB->Check();
    m_pAlignLB->SetSelectHdl(LINK(this, SvxNumPositionTabPage, EditModifyHdl_Impl));
    m_pAlign2LB->SetSelectHdl(LINK(this, SvxNumPositionTabPage, EditModifyHdl_Impl));
    for ( sal_uInt16 i = 0; i < m_pAlignLB->GetEntryCount(); ++i )
    {
        m_pAlign2LB->InsertEntry( m_pAlignLB->GetEntry( i ) );
    }
    m_pAlign2LB->SetDropDownLineCount( m_pAlign2LB->GetEntryCount() );

    Link aLk = LINK(this, SvxNumPositionTabPage, DistanceHdl_Impl);

    m_pDistBorderMF->SetUpHdl(aLk);
    m_pDistBorderMF->SetDownHdl(aLk);
    m_pDistBorderMF->SetLoseFocusHdl(aLk);

    m_pDistNumMF->SetUpHdl(aLk);
    m_pDistNumMF->SetDownHdl(aLk);
    m_pDistNumMF->SetLoseFocusHdl(aLk);

    m_pIndentMF->SetUpHdl(aLk);
    m_pIndentMF->SetDownHdl(aLk);
    m_pIndentMF->SetLoseFocusHdl(aLk);

    m_pLabelFollowedByLB->SetDropDownLineCount( m_pLabelFollowedByLB->GetEntryCount() );
    m_pLabelFollowedByLB->SetSelectHdl( LINK(this, SvxNumPositionTabPage, LabelFollowedByHdl_Impl) );

    aLk = LINK(this, SvxNumPositionTabPage, ListtabPosHdl_Impl);
    m_pListtabMF->SetUpHdl(aLk);
    m_pListtabMF->SetDownHdl(aLk);
    m_pListtabMF->SetLoseFocusHdl(aLk);

    aLk = LINK(this, SvxNumPositionTabPage, AlignAtHdl_Impl);
    m_pAlignedAtMF->SetUpHdl(aLk);
    m_pAlignedAtMF->SetDownHdl(aLk);
    m_pAlignedAtMF->SetLoseFocusHdl(aLk);

    aLk = LINK(this, SvxNumPositionTabPage, IndentAtHdl_Impl);
    m_pIndentAtMF->SetUpHdl(aLk);
    m_pIndentAtMF->SetDownHdl(aLk);
    m_pIndentAtMF->SetLoseFocusHdl(aLk);

    m_pLevelLB->EnableMultiSelection(true);
    m_pLevelLB->SetSelectHdl(LINK(this, SvxNumPositionTabPage, LevelHdl_Impl));
    m_pRelativeCB->SetClickHdl(LINK(this, SvxNumPositionTabPage, RelativeHdl_Impl));
    m_pStandardPB->SetClickHdl(LINK(this, SvxNumPositionTabPage, StandardHdl_Impl));


    m_pRelativeCB->Check(bLastRelative);
    m_pPreviewWIN->SetPositionMode();
    eCoreUnit = rSet.GetPool()->GetMetric(rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE));
}

SvxNumPositionTabPage::~SvxNumPositionTabPage()
{
    delete pActNum;
    delete pSaveNum;
}
/*-------------------------------------------------------*/

#if OSL_DEBUG_LEVEL > 1
void lcl_PrintDebugOutput(FixedText& rFixed, const SvxNumberFormat& rNumFmt)
{
#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))

    OUString const sHash( " # " );
    if ( rNumFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        OUString sDebugText( OUString::number( TWIP_TO_MM100(rNumFmt.GetAbsLSpace() ) ) );
        sDebugText += sHash;
        sDebugText += OUString::number( TWIP_TO_MM100(rNumFmt.GetCharTextDistance() ) );
        sDebugText += sHash;
        sDebugText += OUString::number( TWIP_TO_MM100(rNumFmt.GetFirstLineOffset() ) );
        rFixed.SetText(sDebugText);
    }
    else if ( rNumFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        String sDebugText( OUString::number( TWIP_TO_MM100(rNumFmt.GetListtabPos() ) ) );
        sDebugText += sHash;
        sDebugText += OUString::number( TWIP_TO_MM100(rNumFmt.GetFirstLineIndent() ) );
        sDebugText += sHash;
        sDebugText += OUString::number( TWIP_TO_MM100(rNumFmt.GetIndentAt() ) );
        rFixed.SetText(sDebugText);
    }

}
#endif

void SvxNumPositionTabPage::InitControls()
{
    bInInintControl = sal_True;
    const bool bRelative = !bLabelAlignmentPosAndSpaceModeActive &&
                     m_pRelativeCB->IsEnabled() && m_pRelativeCB->IsChecked();
    const bool bSingleSelection = m_pLevelLB->GetSelectEntryCount() == 1 &&
                            USHRT_MAX != nActNumLvl;

    m_pDistBorderMF->Enable( !bLabelAlignmentPosAndSpaceModeActive &&
                          ( bSingleSelection || bRelative ) );
    m_pDistBorderFT->Enable( !bLabelAlignmentPosAndSpaceModeActive &&
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
        SetMetricValue(*m_pDistBorderMF, nDistBorderNum, eCoreUnit);
    }
    else
        bSetDistEmpty = true;

    const String aEmptyStr;
    if(bSameDist)
        SetMetricValue(*m_pDistNumMF, aNumFmtArr[nLvl]->GetCharTextDistance(), eCoreUnit);
    else
        m_pDistNumMF->SetText(aEmptyStr);
    if(bSameIndent)
        SetMetricValue(*m_pIndentMF, - aNumFmtArr[nLvl]->GetFirstLineOffset(), eCoreUnit);
    else
        m_pIndentMF->SetText(aEmptyStr);

    if(bSameAdjust)
    {
        sal_uInt16 nPos = 1; // centered
        if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_LEFT)
            nPos = 0;
        else if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_RIGHT)
            nPos = 2;
        m_pAlignLB->SelectEntryPos(nPos);
        m_pAlign2LB->SelectEntryPos( nPos );
    }
    else
    {
        m_pAlignLB->SetNoSelection();
        m_pAlign2LB->SetNoSelection();
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
        m_pLabelFollowedByLB->SelectEntryPos( nPos );
    }
    else
    {
        m_pLabelFollowedByLB->SetNoSelection();
    }

    if ( aNumFmtArr[nLvl]->GetLabelFollowedBy() == SvxNumberFormat::LISTTAB )
    {
        m_pListtabFT->Enable( true );
        m_pListtabMF->Enable( true );
        if ( bSameListtab )
        {
            SetMetricValue(*m_pListtabMF, aNumFmtArr[nLvl]->GetListtabPos(), eCoreUnit);
        }
        else
        {
            m_pListtabMF->SetText(aEmptyStr);
        }
    }
    else
    {
        m_pListtabFT->Enable( false );
        m_pListtabMF->Enable( false );
        m_pListtabMF->SetText(aEmptyStr);
    }

    if ( bSameAlignAt )
    {
        SetMetricValue(*m_pAlignedAtMF,
                        aNumFmtArr[nLvl]->GetIndentAt() + aNumFmtArr[nLvl]->GetFirstLineIndent(),
                        eCoreUnit);
    }
    else
    {
        m_pAlignedAtMF->SetText(aEmptyStr);
    }

    if ( bSameIndentAt )
    {
        SetMetricValue(*m_pIndentAtMF, aNumFmtArr[nLvl]->GetIndentAt(), eCoreUnit);
    }
    else
    {
        m_pIndentAtMF->SetText(aEmptyStr);
    }

    if ( bSetDistEmpty )
        m_pDistBorderMF->SetText(aEmptyStr);

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
        m_pLevelLB->SetUpdateMode(false);
        m_pLevelLB->SetNoSelection();
        m_pLevelLB->SelectEntryPos( pActNum->GetLevelCount(), nActNumLvl == USHRT_MAX);
        if(nActNumLvl != USHRT_MAX)
            for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
            {
                if(nActNumLvl & nMask)
                    m_pLevelLB->SelectEntryPos( i, sal_True);
                nMask <<= 1 ;
            }
        m_pRelativeCB->Enable(nActNumLvl != 1);
        m_pLevelLB->SetUpdateMode(sal_True);

        InitPosAndSpaceMode();
        ShowControlsDependingOnPosAndSpaceMode();

        InitControls();
    }
    m_pPreviewWIN->SetLevel(nActNumLvl);
    m_pPreviewWIN->Invalidate();
}

int  SvxNumPositionTabPage::DeactivatePage(SfxItemSet *_pSet)
{
    if(_pSet)
    {
        if(m_pDistBorderMF->IsEnabled())
            DistanceHdl_Impl(m_pDistBorderMF);
        DistanceHdl_Impl(m_pIndentMF);
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
    if(!m_pLevelLB->GetEntryCount())
    {
        for(sal_uInt16 i = 1; i <= pSaveNum->GetLevelCount(); i++)
            m_pLevelLB->InsertEntry( OUString::number(i) );
        if(pSaveNum->GetLevelCount() > 1)
        {
            OUString sEntry( "1 - " );
            sEntry += OUString::number( pSaveNum->GetLevelCount() );
            m_pLevelLB->InsertEntry(sEntry);
            m_pLevelLB->SelectEntry(sEntry);
        }
        else
            m_pLevelLB->SelectEntryPos(0);
    }
    else
        m_pLevelLB->SelectEntryPos(m_pLevelLB->GetEntryCount() - 1);
    sal_uInt16 nMask = 1;
    m_pLevelLB->SetUpdateMode(sal_False);
    m_pLevelLB->SetNoSelection();
    if(nActNumLvl == USHRT_MAX)
    {
        m_pLevelLB->SelectEntryPos( pSaveNum->GetLevelCount(), sal_True);
    }
    else
    {
        for(sal_uInt16 i = 0; i < pSaveNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
                m_pLevelLB->SelectEntryPos( i, sal_True);
            nMask <<= 1;
        }
    }
    m_pLevelLB->SetUpdateMode(sal_True);

    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
    m_pPreviewWIN->SetNumRule(pActNum);

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
    m_pDistBorderFT->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pDistBorderMF->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pRelativeCB->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pIndentFT->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pIndentMF->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pDistNumFT->Show( !bLabelAlignmentPosAndSpaceModeActive &&
                     pActNum->IsFeatureSupported(NUM_CONTINUOUS) );
    m_pDistNumMF->Show( !bLabelAlignmentPosAndSpaceModeActive &&
                     pActNum->IsFeatureSupported(NUM_CONTINUOUS));
    m_pAlignFT->Show( !bLabelAlignmentPosAndSpaceModeActive );
    m_pAlignLB->Show( !bLabelAlignmentPosAndSpaceModeActive );

    m_pLabelFollowedByFT->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pLabelFollowedByLB->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pListtabFT->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pListtabMF->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pAlign2FT->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pAlign2LB->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pAlignedAtFT->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pAlignedAtMF->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pIndentAtFT->Show( bLabelAlignmentPosAndSpaceModeActive );
    m_pIndentAtMF->Show( bLabelAlignmentPosAndSpaceModeActive );
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
        m_pDistBorderMF->SetDecimalDigits(1);
        m_pDistNumMF->SetDecimalDigits(1);
        m_pIndentMF->SetDecimalDigits(1);
        m_pListtabMF->SetDecimalDigits(1);
        m_pAlignedAtMF->SetDecimalDigits(1);
        m_pIndentAtMF->SetDecimalDigits(1);
    }
    m_pDistBorderMF->SetUnit( eMetric );
    m_pDistNumMF->SetUnit( eMetric );
    m_pIndentMF->SetUnit( eMetric );
    m_pListtabMF->SetUnit( eMetric );
    m_pAlignedAtMF->SetUnit( eMetric );
    m_pIndentAtMF->SetUnit( eMetric );
}

IMPL_LINK_NOARG(SvxNumPositionTabPage, EditModifyHdl_Impl)
{
    sal_uInt16 nMask = 1;
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));

            const sal_uInt16 nPos = m_pAlignLB->IsVisible()
                                ? m_pAlignLB->GetSelectEntryPos()
                                : m_pAlign2LB->GetSelectEntryPos();
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
    m_pRelativeCB->Enable(nActNumLvl != 1);
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
    for(sal_uInt16 i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt( pActNum->GetLevel( i ) );
            if (pFld == m_pDistBorderMF)
            {

                if(m_pRelativeCB->IsChecked())
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
            else if (pFld == m_pDistNumMF)
            {
                aNumFmt.SetCharTextDistance( (short)nValue );
            }
            else if (pFld == m_pIndentMF)
            {
                // together with the FirstLineOffset the AbsLSpace must be changed, too
                long nDiff = nValue + aNumFmt.GetFirstLineOffset();
                long nAbsLSpace = aNumFmt.GetAbsLSpace();
                aNumFmt.SetAbsLSpace(sal_uInt16(nAbsLSpace + nDiff));
                aNumFmt.SetFirstLineOffset( -(short)nValue );
            }

            pActNum->SetLevel( i, aNumFmt );
        }
        nMask <<= 1;
    }

    SetModified();
    if(!m_pDistBorderMF->IsEnabled())
    {
        String aEmptyStr;
        m_pDistBorderMF->SetText(aEmptyStr);
    }

    return 0;
}

IMPL_LINK( SvxNumPositionTabPage, RelativeHdl_Impl, CheckBox *, pBox )
{
    sal_Bool bOn = pBox->IsChecked();
    sal_Bool bSingleSelection = m_pLevelLB->GetSelectEntryCount() == 1 && USHRT_MAX != nActNumLvl;
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
        SetMetricValue(*m_pDistBorderMF, nValue,   eCoreUnit);
    else
        m_pDistBorderMF->SetText(aEmptyStr);
    m_pDistBorderMF->Enable(bOn || bSingleSelection);
    m_pDistBorderFT->Enable(bOn || bSingleSelection);
    bLastRelative = bOn;
    return 0;
}

IMPL_LINK_NOARG(SvxNumPositionTabPage, LabelFollowedByHdl_Impl)
{
    // determine value to be set at the chosen list levels
    SvxNumberFormat::LabelFollowedBy eLabelFollowedBy = SvxNumberFormat::LISTTAB;
    {
        const sal_uInt16 nPos = m_pLabelFollowedByLB->GetSelectEntryPos();
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
    m_pListtabFT->Enable( eLabelFollowedBy == SvxNumberFormat::LISTTAB );
    m_pListtabMF->Enable( eLabelFollowedBy == SvxNumberFormat::LISTTAB );
    if ( bSameListtabPos && eLabelFollowedBy == SvxNumberFormat::LISTTAB )
    {
        SetMetricValue(*m_pListtabMF, pActNum->GetLevel( nFirstLvl ).GetListtabPos(), eCoreUnit);
    }
    else
    {
        m_pListtabMF->SetText( String() );
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
        m_pPreviewWIN->SetLevel(nActNumLvl);
        m_pPreviewWIN->Invalidate();
    }
}

void SvxNumOptionsTabPage::SetModified(sal_Bool bRepaint)
{
    bModified = sal_True;
    if(bRepaint)
    {
        m_pPreviewWIN->SetLevel(nActNumLvl);
        m_pPreviewWIN->Invalidate();
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
        const std::vector<OUString> &aList = pListItem->GetList();
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
