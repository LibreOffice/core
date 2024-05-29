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

#include <com/sun/star/i18n/ScriptType.hpp>
#include <hintids.hxx>
#include <hints.hxx>
#include <ndtxt.hxx>
#include <swtypes.hxx>
#include <init.hxx>
#include <svl/languageoptions.hxx>
#include <utility>
#include <vcl/outdev.hxx>
#include <osl/diagnose.h>

SwFormatChg::SwFormatChg( SwFormat* pFormat )
    : SwMsgPoolItem( RES_FMT_CHG ), pChangedFormat( pFormat )
{
}



namespace sw {

InsertText::InsertText(const sal_Int32 nP, const sal_Int32 nL, const bool isInFMCommand, const bool isInFMResult)
    : SfxHint( SfxHintId::SwInsertText )
    , nPos( nP ), nLen( nL )
    , isInsideFieldmarkCommand(isInFMCommand)
    , isInsideFieldmarkResult(isInFMResult)
{
}

DeleteText::DeleteText( const sal_Int32 nS, const sal_Int32 nL )
    : SfxHint( SfxHintId::SwDeleteText ), nStart( nS ), nLen( nL )
{
}

DeleteChar::DeleteChar( const sal_Int32 nPos )
    : SfxHint( SfxHintId::SwDeleteChar ), m_nPos( nPos )
{
}

VirtPageNumHint::VirtPageNumHint(const SwPageFrame* pPg):
    SfxHint(SfxHintId::SwVirtPageNumHint),
    m_pPage(nullptr),
    m_pOrigPage(pPg),
    m_pFrame(nullptr),
    m_bFound(false)
{
}

void AutoFormatUsedHint::CheckNode(const SwNode* pNode) const
{
    if(pNode && &pNode->GetNodes() == &m_rNodes)
        SetUsed();
}
} // namespace sw

SwUpdateAttr::SwUpdateAttr( sal_Int32 nS, sal_Int32 nE, sal_uInt16 nW )
    : SwMsgPoolItem( RES_UPDATE_ATTR ), m_nStart( nS ), m_nEnd( nE ), m_nWhichAttr( nW )
{
}

SwUpdateAttr::SwUpdateAttr( sal_Int32 nS, sal_Int32 nE, sal_uInt16 nW, std::vector<sal_uInt16> aW )
    : SwMsgPoolItem( RES_UPDATE_ATTR ), m_nStart( nS ), m_nEnd( nE ), m_nWhichAttr( nW ), m_aWhichFmtAttrs(std::move( aW ))
{
}

SwTableFormulaUpdate::SwTableFormulaUpdate(const SwTable* pNewTable)
    : m_pTable(pNewTable)
    , m_nSplitLine(USHRT_MAX)
    , m_eFlags(TBL_CALC)
{
    m_aData.pDelTable = nullptr;
    m_bModified = m_bBehindSplitLine = false;
    OSL_ENSURE( m_pTable, "No Table pointer" );
}

SwAttrSetChg::SwAttrSetChg( const SwAttrSet& rTheSet, SwAttrSet& rSet )
    : SwMsgPoolItem( RES_ATTRSET_CHG ),
    m_bDelSet( false ),
    m_pChgSet( &rSet ),
    m_pTheChgdSet( &rTheSet )
{
}

SwAttrSetChg::SwAttrSetChg( const SwAttrSetChg& rChgSet )
    : SwMsgPoolItem( RES_ATTRSET_CHG ),
    m_bDelSet( true ),
    m_pTheChgdSet( rChgSet.m_pTheChgdSet )
{
    m_pChgSet = new SwAttrSet( *rChgSet.m_pChgSet );
}

SwAttrSetChg::~SwAttrSetChg()
{
    if( m_bDelSet )
        delete m_pChgSet;
}

#ifdef DBG_UTIL
void SwAttrSetChg::ClearItem( sal_uInt16 nWhch )
{
    OSL_ENSURE( m_bDelSet, "The Set may not be changed!" );
    m_pChgSet->ClearItem( nWhch );
}
#endif

SwMsgPoolItem::SwMsgPoolItem( sal_uInt16 nWhch )
    : SfxPoolItem( nWhch )
{
}

bool SwMsgPoolItem::operator==( const SfxPoolItem& rItem ) const
{
    assert( SfxPoolItem::operator==(rItem)); (void)rItem;
    // SwMsgPoolItem now knows operator== due to evtl. deeper
    // ItemCompares using SfxPoolItem::areSame. No members,
    // so always equal
    return true;
}

SwMsgPoolItem* SwMsgPoolItem::Clone( SfxItemPool* ) const
{
    OSL_FAIL( "SwMsgPoolItem knows no Clone" );
    return nullptr;
}

const SfxPoolItem* GetDfltAttr(sal_uInt16 nWhich)
{
#ifdef DBG_UTIL
    OSL_ASSERT(nWhich < POOLATTR_END && nWhich >= POOLATTR_BEGIN);
    const SfxPoolItem* pRetval(getItemInfoPackageSwAttributes().getExistingItemInfo(nWhich - POOLATTR_BEGIN).getItem());
    OSL_ENSURE(pRetval, "GetDfltFormatAttr(): Dflt == 0");
    return pRetval;
#else
    return getItemInfoPackageSwAttributes().getExistingItemInfo(nWhich - POOLATTR_BEGIN).getItem();
#endif
}

SwFindNearestNode::SwFindNearestNode( const SwNode& rNd )
    : SwMsgPoolItem( RES_FINDNEARESTNODE ), m_pNode( &rNd ), m_pFound( nullptr )
{
}

void SwFindNearestNode::CheckNode( const SwNode& rNd )
{
    if( &m_pNode->GetNodes() == &rNd.GetNodes() )
    {
        SwNodeOffset nIdx = rNd.GetIndex();
        if( nIdx < m_pNode->GetIndex() &&
            ( !m_pFound || nIdx > m_pFound->GetIndex() ) &&
            nIdx > rNd.GetNodes().GetEndOfExtras().GetIndex() )
            m_pFound = &rNd;
    }
}

sal_uInt16 GetWhichOfScript( sal_uInt16 nWhich, sal_uInt16 nScript )
{
    static const sal_uInt16 aLangMap[3] =
        { RES_CHRATR_LANGUAGE, RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CTL_LANGUAGE };
    static const sal_uInt16 aFontMap[3] =
        { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,  RES_CHRATR_CTL_FONT};
    static const sal_uInt16 aFontSizeMap[3] =
        { RES_CHRATR_FONTSIZE, RES_CHRATR_CJK_FONTSIZE,  RES_CHRATR_CTL_FONTSIZE };
    static const sal_uInt16 aWeightMap[3] =
        { RES_CHRATR_WEIGHT, RES_CHRATR_CJK_WEIGHT,  RES_CHRATR_CTL_WEIGHT};
    static const sal_uInt16 aPostureMap[3] =
        { RES_CHRATR_POSTURE, RES_CHRATR_CJK_POSTURE,  RES_CHRATR_CTL_POSTURE};

    const sal_uInt16* pM;
    switch( nWhich )
    {
    case RES_CHRATR_LANGUAGE:
    case RES_CHRATR_CJK_LANGUAGE:
    case RES_CHRATR_CTL_LANGUAGE:
        pM = aLangMap;
        break;

    case RES_CHRATR_FONT:
    case RES_CHRATR_CJK_FONT:
    case RES_CHRATR_CTL_FONT:
        pM = aFontMap;
        break;

    case RES_CHRATR_FONTSIZE:
    case RES_CHRATR_CJK_FONTSIZE:
    case RES_CHRATR_CTL_FONTSIZE:
        pM = aFontSizeMap;
        break;

    case  RES_CHRATR_WEIGHT:
    case  RES_CHRATR_CJK_WEIGHT:
    case  RES_CHRATR_CTL_WEIGHT:
        pM = aWeightMap;
        break;

    case RES_CHRATR_POSTURE:
    case RES_CHRATR_CJK_POSTURE:
    case RES_CHRATR_CTL_POSTURE:
        pM = aPostureMap;
        break;

    default:
        pM = nullptr;
    }

    sal_uInt16 nRet;
    if( pM )
    {
        using namespace ::com::sun::star;
        {
            if( i18n::ScriptType::WEAK == nScript )
                nScript = SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() );
            switch( nScript)
            {
            case i18n::ScriptType::COMPLEX:
                ++pM;
                [[fallthrough]];
            case i18n::ScriptType::ASIAN:
                ++pM;
                [[fallthrough]];
            default:
                nRet = *pM;
            }
        }
    }
    else
        nRet = nWhich;
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
