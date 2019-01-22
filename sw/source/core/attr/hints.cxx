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
#include <editeng/scripttypeitem.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <ndtxt.hxx>
#include <swtypes.hxx>
#include <vcl/outdev.hxx>
#include <osl/diagnose.h>

SwFormatChg::SwFormatChg( SwFormat* pFormat )
    : SwMsgPoolItem( RES_FMT_CHG ), pChangedFormat( pFormat )
{
}

SwInsText::SwInsText( sal_Int32 nP, sal_Int32 nL )
    : SwMsgPoolItem( RES_INS_TXT ), nPos( nP ), nLen( nL )
{
}

SwDelChr::SwDelChr( sal_Int32 nP )
    : SwMsgPoolItem( RES_DEL_CHR ), nPos( nP )
{
}

SwDelText::SwDelText( sal_Int32 nS, sal_Int32 nL )
    : SwMsgPoolItem( RES_DEL_TXT ), nStart( nS ), nLen( nL )
{
}

namespace sw {

MoveText::MoveText(SwTextNode *const pD, sal_Int32 const nD, sal_Int32 const nS, sal_Int32 const nL)
    : pDestNode(pD), nDestStart(nD), nSourceStart(nS), nLen(nL)
{
}

RedlineDelText::RedlineDelText(sal_Int32 const nS, sal_Int32 const nL)
    : nStart(nS), nLen(nL)
{
}

RedlineUnDelText::RedlineUnDelText(sal_Int32 const nS, sal_Int32 const nL)
    : nStart(nS), nLen(nL)
{
}

} // namespace sw

SwUpdateAttr::SwUpdateAttr( sal_Int32 nS, sal_Int32 nE, sal_uInt16 nW )
    : SwMsgPoolItem( RES_UPDATE_ATTR ), m_nStart( nS ), m_nEnd( nE ), m_nWhichAttr( nW )
{
}

SwUpdateAttr::SwUpdateAttr( sal_Int32 nS, sal_Int32 nE, sal_uInt16 nW, std::vector<sal_uInt16> aW )
    : SwMsgPoolItem( RES_UPDATE_ATTR ), m_nStart( nS ), m_nEnd( nE ), m_nWhichAttr( nW ), m_aWhichFmtAttrs( aW )
{
}

SwRefMarkFieldUpdate::SwRefMarkFieldUpdate( OutputDevice* pOutput )
    : SwMsgPoolItem( RES_REFMARKFLD_UPDATE ),
    pOut( pOutput )
{
    OSL_ENSURE( pOut, "No OutputDevice pointer" );
}

SwDocPosUpdate::SwDocPosUpdate( const SwTwips nDcPos )
    : SwMsgPoolItem( RES_DOCPOS_UPDATE ), nDocPos(nDcPos)
{
}

SwTableFormulaUpdate::SwTableFormulaUpdate( const SwTable* pNewTable )
    : SwMsgPoolItem( RES_TABLEFML_UPDATE ),
    m_pTable( pNewTable ), m_pHistory( nullptr ), m_nSplitLine( USHRT_MAX ),
    m_eFlags( TBL_CALC )
{
    m_aData.pDelTable = nullptr;
    m_bModified = m_bBehindSplitLine = false;
    OSL_ENSURE( m_pTable, "No Table pointer" );
}

SwAutoFormatGetDocNode::SwAutoFormatGetDocNode( const SwNodes* pNds )
    : SwMsgPoolItem( RES_AUTOFMT_DOCNODE ), pNodes( pNds )
{
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

bool SwMsgPoolItem::operator==( const SfxPoolItem& ) const
{
    OSL_FAIL( "SwMsgPoolItem knows no ==" );
    return false;
}

SfxPoolItem* SwMsgPoolItem::Clone( SfxItemPool* ) const
{
    OSL_FAIL( "SwMsgPoolItem knows no Clone" );
    return nullptr;
}

#if OSL_DEBUG_LEVEL > 0
const SfxPoolItem* GetDfltAttr( sal_uInt16 nWhich )
{
    OSL_ASSERT( nWhich < POOLATTR_END && nWhich >= POOLATTR_BEGIN );

    SfxPoolItem *pHt = aAttrTab[ nWhich - POOLATTR_BEGIN ];
    OSL_ENSURE( pHt, "GetDfltFormatAttr(): Dflt == 0" );
    return pHt;
}
#else
const SfxPoolItem* GetDfltAttr( sal_uInt16 nWhich )
{
    return aAttrTab[ nWhich - POOLATTR_BEGIN ];
}
#endif

SwCondCollCondChg::SwCondCollCondChg( SwFormat *pFormat )
    : SwMsgPoolItem( RES_CONDCOLL_CONDCHG ), pChangedFormat( pFormat )
{
}

SwVirtPageNumInfo::SwVirtPageNumInfo( const SwPageFrame *pPg ) :
    SwMsgPoolItem( RES_VIRTPAGENUM_INFO ), m_pPage( nullptr ), m_pOrigPage( pPg ), m_pFrame( nullptr )
{
}

SwFindNearestNode::SwFindNearestNode( const SwNode& rNd )
    : SwMsgPoolItem( RES_FINDNEARESTNODE ), m_pNode( &rNd ), m_pFound( nullptr )
{
}

void SwFindNearestNode::CheckNode( const SwNode& rNd )
{
    if( &m_pNode->GetNodes() == &rNd.GetNodes() )
    {
        sal_uLong nIdx = rNd.GetIndex();
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
