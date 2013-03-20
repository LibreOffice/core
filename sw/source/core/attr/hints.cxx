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

SwFmtChg::SwFmtChg( SwFmt* pFmt )
    : SwMsgPoolItem( RES_FMT_CHG ), pChangedFmt( pFmt )
{
}

SwInsTxt::SwInsTxt( xub_StrLen nP, xub_StrLen nL )
    : SwMsgPoolItem( RES_INS_TXT ), nPos( nP ), nLen( nL )
{
}

SwDelChr::SwDelChr( xub_StrLen nP )
    : SwMsgPoolItem( RES_DEL_CHR ), nPos( nP )
{
}

SwDelTxt::SwDelTxt( xub_StrLen nS, xub_StrLen nL )
    : SwMsgPoolItem( RES_DEL_TXT ), nStart( nS ), nLen( nL )
{
}

SwUpdateAttr::SwUpdateAttr( xub_StrLen nS, xub_StrLen nE, sal_uInt16 nW )
    : SwMsgPoolItem( RES_UPDATE_ATTR ), nStart( nS ), nEnd( nE ), nWhichAttr( nW )
{
}

/** Is sent if reference marks should be updated.

    To get the page/chapter number, the frame has to be asked. For that we need
    the current OutputDevice.
*/
SwRefMarkFldUpdate::SwRefMarkFldUpdate( const OutputDevice* pOutput )
    : SwMsgPoolItem( RES_REFMARKFLD_UPDATE ),
    pOut( pOutput )
{
    OSL_ENSURE( pOut, "No OutputDevice pointer" );
}

SwDocPosUpdate::SwDocPosUpdate( const SwTwips nDcPos )
    : SwMsgPoolItem( RES_DOCPOS_UPDATE ), nDocPos(nDcPos)
{
}

/** Is sent if a table should be recalculated */
SwTableFmlUpdate::SwTableFmlUpdate( const SwTable* pNewTbl )
    : SwMsgPoolItem( RES_TABLEFML_UPDATE ),
    pTbl( pNewTbl ), pHistory( 0 ), nSplitLine( USHRT_MAX ),
    eFlags( TBL_CALC )
{
    DATA.pDelTbl = 0;
    bModified = bBehindSplitLine = sal_False;
    OSL_ENSURE( pTbl, "No Table pointer" );
}

SwAutoFmtGetDocNode::SwAutoFmtGetDocNode( const SwNodes* pNds )
    : SwMsgPoolItem( RES_AUTOFMT_DOCNODE ), pCntntNode( 0 ), pNodes( pNds )
{
}

SwAttrSetChg::SwAttrSetChg( const SwAttrSet& rTheSet, SwAttrSet& rSet )
    : SwMsgPoolItem( RES_ATTRSET_CHG ),
    bDelSet( sal_False ),
    pChgSet( &rSet ),
    pTheChgdSet( &rTheSet )
{
}

SwAttrSetChg::SwAttrSetChg( const SwAttrSetChg& rChgSet )
    : SwMsgPoolItem( RES_ATTRSET_CHG ),
    bDelSet( sal_True ),
    pTheChgdSet( rChgSet.pTheChgdSet )
{
    pChgSet = new SwAttrSet( *rChgSet.pChgSet );
}

SwAttrSetChg::~SwAttrSetChg()
{
    if( bDelSet )
        delete pChgSet;
}

#ifdef DBG_UTIL
void SwAttrSetChg::ClearItem( sal_uInt16 nWhch )
{
    OSL_ENSURE( bDelSet, "The Set may not be changed!" );
    pChgSet->ClearItem( nWhch );
}
#endif

SwMsgPoolItem::SwMsgPoolItem( sal_uInt16 nWhch )
    : SfxPoolItem( nWhch )
{
}

// "Overhead" of SfxPoolItem
int SwMsgPoolItem::operator==( const SfxPoolItem& ) const
{
    OSL_FAIL( "SwMsgPoolItem knows no ==" );
    return 0;
}

SfxPoolItem* SwMsgPoolItem::Clone( SfxItemPool* ) const
{
    OSL_FAIL( "SwMsgPoolItem knows no Clone" );
    return 0;
}

#if OSL_DEBUG_LEVEL > 0
/** Get the default attribute from corresponding default attribute table.

    @param[in] nWhich Position in table
    @return Attribute if found, null pointer otherwise
*/
const SfxPoolItem* GetDfltAttr( sal_uInt16 nWhich )
{
    OSL_ASSERT( nWhich < POOLATTR_END && nWhich >= POOLATTR_BEGIN );

    SfxPoolItem *pHt = aAttrTab[ nWhich - POOLATTR_BEGIN ];
    OSL_ENSURE( pHt, "GetDfltFmtAttr(): Dflt == 0" );
    return pHt;
}
#else
/** Get the default attribute from corresponding default attribute table.

    @param[in] nWhich Position in table
*/
const SfxPoolItem* GetDfltAttr( sal_uInt16 nWhich )
{
    return aAttrTab[ nWhich - POOLATTR_BEGIN ];
}
#endif

SwCondCollCondChg::SwCondCollCondChg( SwFmt *pFmt )
    : SwMsgPoolItem( RES_CONDCOLL_CONDCHG ), pChangedFmt( pFmt )
{
}

SwVirtPageNumInfo::SwVirtPageNumInfo( const SwPageFrm *pPg ) :
    SwMsgPoolItem( RES_VIRTPAGENUM_INFO ), pPage( 0 ), pOrigPage( pPg ), pFrm( 0 )
{
}


SwFindNearestNode::SwFindNearestNode( const SwNode& rNd )
    : SwMsgPoolItem( RES_FINDNEARESTNODE ), pNd( &rNd ), pFnd( 0 )
{
}

void SwFindNearestNode::CheckNode( const SwNode& rNd )
{
    if( &pNd->GetNodes() == &rNd.GetNodes() )
    {
        sal_uLong nIdx = rNd.GetIndex();
        if( nIdx < pNd->GetIndex() &&
            ( !pFnd || nIdx > pFnd->GetIndex() ) &&
            nIdx > rNd.GetNodes().GetEndOfExtras().GetIndex() )
            pFnd = &rNd;
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
        pM = 0;
    }

    sal_uInt16 nRet;
    if( pM )
    {
        using namespace ::com::sun::star;
        {
            if( i18n::ScriptType::WEAK == nScript )
                nScript = GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage() );
            switch( nScript)
            {
            case i18n::ScriptType::COMPLEX:
                ++pM;  // no break;
            case i18n::ScriptType::ASIAN:
                ++pM;  // no break;
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
