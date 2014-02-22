/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

/*
 * This file contains methods for the WW8 output
 * (nodes, attributes, formats und chars).
 */

#include <hintids.hxx>

#include <vcl/svapp.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/settings.hxx>

#include <svl/zformat.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svl/grabbagitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/paravertalignitem.hxx>
#include <editeng/pgrditem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/paperinf.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include <fmtfld.hxx>
#include <fchrfmt.hxx>
#include <fmtfsize.hxx>
#include <fmtpdsc.hxx>
#include <fmtornt.hxx>
#include <fmtanchr.hxx>
#include <fmtclds.hxx>
#include <fmtsrnd.hxx>
#include <fmtftn.hxx>
#include <fmtflcnt.hxx>
#include <frmatr.hxx>
#include <swtable.hxx>
#include <fmtinfmt.hxx>
#include <txtfld.hxx>
#include <txtftn.hxx>
#include <poolfmt.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <paratr.hxx>
#include <fldbas.hxx>
#include <docufld.hxx>
#include <expfld.hxx>
#include <pagedesc.hxx>
#include <flddat.hxx>
#include <ndtxt.hxx>
#include <swrect.hxx>
#include <reffld.hxx>
#include <ftninfo.hxx>
#include <charfmt.hxx>
#include <section.hxx>
#include <lineinfo.hxx>
#include <fmtline.hxx>
#include <tox.hxx>
#include <fmtftntx.hxx>
#include <breakit.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <unotools/localedatawrapper.hxx>
#include <tgrditem.hxx>
#include <flddropdown.hxx>
#include <chpfld.hxx>
#include <fmthdft.hxx>

#include <filter/msfilter/sprmids.hxx>

#include <fmtcntnt.hxx>
#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include "wrtww8.hxx"
#include "ww8par.hxx"
#include "ww8attributeoutput.hxx"
#include "fields.hxx"
#include <vcl/outdev.hxx>
#include <i18nlangtag/languagetag.hxx>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;
using namespace nsFieldFlags;
using namespace nsSwDocInfoSubType;
using namespace sw::util;
using namespace sw::types;

bool WW8Export::CollapseScriptsforWordOk( sal_uInt16 nScript, sal_uInt16 nWhich )
{
    bool bRet = true;
    if ( nScript == i18n::ScriptType::ASIAN )
    {
        
        
        
        
        switch ( nWhich )
        {
            case RES_CHRATR_FONTSIZE:
            case RES_CHRATR_POSTURE:
            case RES_CHRATR_WEIGHT:
                bRet = false;
                break;
            case RES_CHRATR_LANGUAGE:
            case RES_CHRATR_CTL_FONT:
            case RES_CHRATR_CTL_FONTSIZE:
            case RES_CHRATR_CTL_LANGUAGE:
            case RES_CHRATR_CTL_POSTURE:
            case RES_CHRATR_CTL_WEIGHT:
                if (bWrtWW8 == 0)
                    bRet = false;
            default:
                break;
        }
    }
    else if ( nScript == i18n::ScriptType::COMPLEX )
    {
        
        
        
        if ( bWrtWW8 == 0 )
        {
            switch ( nWhich )
            {
                case RES_CHRATR_CJK_FONT:
                case RES_CHRATR_CJK_FONTSIZE:
                case RES_CHRATR_CJK_POSTURE:
                case RES_CHRATR_CJK_WEIGHT:
                case RES_CHRATR_CJK_LANGUAGE:
                case RES_CHRATR_FONT:
                case RES_CHRATR_FONTSIZE:
                case RES_CHRATR_POSTURE:
                case RES_CHRATR_WEIGHT:
                case RES_CHRATR_LANGUAGE:
                    bRet = false;
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        
        
        
        
        switch ( nWhich )
        {
            case RES_CHRATR_CJK_FONTSIZE:
            case RES_CHRATR_CJK_POSTURE:
            case RES_CHRATR_CJK_WEIGHT:
                bRet = false;
                break;
            case RES_CHRATR_CJK_LANGUAGE:
            case RES_CHRATR_CTL_FONT:
            case RES_CHRATR_CTL_FONTSIZE:
            case RES_CHRATR_CTL_LANGUAGE:
            case RES_CHRATR_CTL_POSTURE:
            case RES_CHRATR_CTL_WEIGHT:
                if ( bWrtWW8 == 0 )
                    bRet = false;
            default:
                break;
        }
    }
    return bRet;
}





void MSWordExportBase::ExportPoolItemsToCHP( sw::PoolItems &rItems, sal_uInt16 nScript )
{
    sw::cPoolItemIter aEnd = rItems.end();
    for ( sw::cPoolItemIter aI = rItems.begin(); aI != aEnd; ++aI )
    {
        const SfxPoolItem *pItem = aI->second;
        sal_uInt16 nWhich = pItem->Which();
        if ( ( isCHRATR( nWhich ) || isTXTATR( nWhich ) ) && CollapseScriptsforWordOk( nScript, nWhich ) )
        {
             
             
             
             
             
             if ( nWhich == RES_TXTATR_INETFMT && ( rItems.begin()->second->Which() == RES_TXTATR_CHARFMT ) )
                 continue;
            AttrOutput().OutputItem( *pItem );
        }
    }
}

/*
 * Format wie folgt ausgeben:
 *      - gebe die Attribute aus; ohne Parents!
 */

void MSWordExportBase::OutputItemSet( const SfxItemSet& rSet, bool bPapFmt, bool bChpFmt, sal_uInt16 nScript,
                                      bool bExportParentItemSet )
{
    if( bExportParentItemSet || rSet.Count() )
    {
        const SfxPoolItem* pItem;
        pISet = &rSet;                  

        
        if ( bPapFmt && SFX_ITEM_SET == rSet.GetItemState( RES_FRAMEDIR, bExportParentItemSet ) )
        {
            
            if ( SFX_ITEM_SET != rSet.GetItemState( RES_PARATR_ADJUST, bExportParentItemSet ) )
            {
                if ( 0 != ( pItem = rSet.GetItem( RES_PARATR_ADJUST, bExportParentItemSet ) ) )
                {
                    
                    AttrOutput().OutputItem( *pItem );
                }
            }
        }

        if ( bPapFmt && SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_NUMRULE, bExportParentItemSet, &pItem ) )
        {
            AttrOutput().OutputItem( *pItem );

            
            if ( ( (SwNumRuleItem*)pItem )->GetValue().isEmpty() &&
                 SFX_ITEM_SET != rSet.GetItemState( RES_LR_SPACE, false) &&
                 SFX_ITEM_SET == rSet.GetItemState( RES_LR_SPACE, true, &pItem ) )
            {
                
                AttrOutput().OutputItem( *pItem );
            }
        }

        sw::PoolItems aItems;
        GetPoolItems( rSet, aItems, bExportParentItemSet );
        if ( bChpFmt )
            ExportPoolItemsToCHP(aItems, nScript);
        if ( bPapFmt )
        {
            sw::cPoolItemIter aEnd = aItems.end();
            for ( sw::cPoolItemIter aI = aItems.begin(); aI != aEnd; ++aI )
            {
                pItem = aI->second;
                sal_uInt16 nWhich = pItem->Which();
                if ( nWhich >= RES_PARATR_BEGIN && nWhich < RES_FRMATR_END && nWhich != RES_PARATR_NUMRULE)
                    AttrOutput().OutputItem( *pItem );
            }
        }
        pISet = 0;                      
    }
}

#include "switerator.hxx"

void MSWordExportBase::GatherChapterFields()
{
    
    SwFieldType* pType = pDoc->GetSysFldType( RES_CHAPTERFLD );
    SwIterator<SwFmtFld,SwFieldType> aFmtFlds( *pType );
    for ( SwFmtFld* pFld = aFmtFlds.First(); pFld; pFld = aFmtFlds.Next() )
    {
        if (const SwTxtFld *pTxtFld = pFld->GetTxtFld())
        {
            const SwTxtNode &rTxtNode = pTxtFld->GetTxtNode();
            maChapterFieldLocs.push_back(rTxtNode.GetIndex());
        }
    }
}

bool MSWordExportBase::CntntContainsChapterField(const SwFmtCntnt &rCntnt) const
{
    bool bRet = false;
    if ( const SwNodeIndex* pSttIdx = rCntnt.GetCntntIdx() )
    {
        SwNodeIndex aIdx( *pSttIdx, 1 );
        SwNodeIndex aEnd( *pSttIdx->GetNode().EndOfSectionNode() );
        sal_uLong nStart = aIdx.GetIndex();
        sal_uLong nEnd = aEnd.GetIndex();
        
        mycCFIter aIEnd = maChapterFieldLocs.end();
        for ( mycCFIter aI = maChapterFieldLocs.begin(); aI != aIEnd; ++aI )
        {
            if ( ( nStart <= *aI ) && ( *aI <= nEnd ) )
            {
                bRet = true;
                break;
            }
        }
    }
    return bRet;
}

bool MSWordExportBase::FmtHdFtContainsChapterField(const SwFrmFmt &rFmt) const
{
    if ( maChapterFieldLocs.empty() )
        return false;

    const SwFrmFmt *pFmt = 0;

    pFmt = rFmt.GetHeader().GetHeaderFmt();
    if ( pFmt && CntntContainsChapterField( pFmt->GetCntnt() ) )
        return true;

    pFmt = rFmt.GetFooter().GetFooterFmt();
    if ( pFmt && CntntContainsChapterField( pFmt->GetCntnt() ) )
        return true;

    return false;
}

bool MSWordExportBase::SetAktPageDescFromNode(const SwNode &rNd)
{
    bool bNewPageDesc = false;
    const SwPageDesc* pCurrent = SwPageDesc::GetPageDescOfNode(rNd);
    OSL_ENSURE(pCurrent && pAktPageDesc, "Not possible surely");
    if (pAktPageDesc && pCurrent)
    {
        if (pCurrent != pAktPageDesc)
        {
            if (pAktPageDesc->GetFollow() != pCurrent)
                bNewPageDesc = true;
            else
            {
                const SwFrmFmt& rTitleFmt = pAktPageDesc->GetFirstMaster();
                const SwFrmFmt& rFollowFmt = pCurrent->GetMaster();

                bNewPageDesc = !IsPlausableSingleWordSection(rTitleFmt,
                    rFollowFmt);
            }
            pAktPageDesc = pCurrent;
        }
        else
        {
            const SwFrmFmt &rFmt = pCurrent->GetMaster();
            bNewPageDesc = FmtHdFtContainsChapterField(rFmt);
        }
    }
    return bNewPageDesc;
}








void MSWordExportBase::OutputSectionBreaks( const SfxItemSet *pSet, const SwNode& rNd, bool isCellOpen)
{
    if ( bStyDef || bOutKF || bInWriteEscher || bOutPageDescs )
        return;

    bBreakBefore = true;
    bool bNewPageDesc = false;
    const SfxPoolItem* pItem=0;
    const SwFmtPageDesc *pPgDesc=0;

    
    
    
    
    bool bBreakSet = false;

    const SwPageDesc * pPageDesc = rNd.FindPageDesc(sal_False);

    
    if (pAktPageDesc != pPageDesc)
    {
        if (isCellOpen && (pAktPageDesc->GetName() != pPageDesc->GetName()))
        {
           
        }
        else
        {
            bBreakSet = true;
            bNewPageDesc = true;
            pAktPageDesc = pPageDesc;
        }
    }

    if ( pSet && pSet->Count() )
    {
        if ( SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, false, &pItem ) &&
             static_cast<const SwFmtPageDesc*>(pItem)->GetRegisteredIn() != NULL)
        {
            bBreakSet = true;
            bNewPageDesc = true;
            pPgDesc = static_cast<const SwFmtPageDesc*>(pItem);
            pAktPageDesc = pPgDesc->GetPageDesc();
        }
        else if ( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, false, &pItem ) )
        {
            
            bool bRemoveHardBreakInsideTable = false;
            if ( bOutTable )
            {
                const SwTableNode* pTableNode = rNd.FindTableNode();
                if ( pTableNode )
                {
                    const SwTableBox* pBox = rNd.GetTblBox();
                    const SwTableLine* pLine = pBox ? pBox->GetUpper() : 0;
                    
                    if ( pLine && !pLine->GetUpper() )
                    {
                        
                        if ( 0 < pLine->GetTabBoxes().GetPos( pBox ) && pBox->GetSttNd() )
                        {
                            bRemoveHardBreakInsideTable = true;
                        }
                    }
                }
            }

            bBreakSet = true;

            if ( !bRemoveHardBreakInsideTable )
            {
                OSL_ENSURE(pAktPageDesc, "should not be possible");
                /*
                 If because of this pagebreak the page desc following the page
                 break is the follow style of the current page desc then output a
                 section break using that style instead.  At least in those cases
                 we end up with the same style in word and writer, nothing can be
                 done when it happens when we get a new pagedesc because we
                 overflow from the first page style.
                */
                if ( pAktPageDesc )
                {
                    
                    const SvxFmtBreakItem* pBreak = dynamic_cast<const SvxFmtBreakItem*>(pItem);
                    if ( pBreak &&
                         pBreak->GetBreak() == SVX_BREAK_PAGE_BEFORE )
                    {
                        bNewPageDesc |= SetAktPageDescFromNode( rNd );
                    }
                }
                if ( !bNewPageDesc )
                    AttrOutput().OutputItem( *pItem );
            }
        }
    }

    /*
    #i9301#
    No explicit page break, lets see if the style had one and we've moved to a
    new page style because of it, if we have to then we take the opportunity to
    set the equivalent word section here. We *could* do it for every paragraph
    that moves onto a new page because of layout, but that would be insane.
    */
    bool bHackInBreak = false;
    if ( !bBreakSet )
    {
        if ( const SwCntntNode *pNd = rNd.GetCntntNode() )
        {
            const SvxFmtBreakItem &rBreak =
                ItemGet<SvxFmtBreakItem>( *pNd, RES_BREAK );
            if ( rBreak.GetBreak() == SVX_BREAK_PAGE_BEFORE )
                bHackInBreak = true;
            else
            {   
                
                const SwFmtPageDesc &rPageDesc =
                    ItemGet<SwFmtPageDesc>( *pNd, RES_PAGEDESC );
                if ( rPageDesc.KnowsPageDesc() )
                    bHackInBreak = true;
            }
        }
    }

    if ( bHackInBreak )
    {
        OSL_ENSURE( pAktPageDesc, "should not be possible" );
        if ( pAktPageDesc )
            bNewPageDesc = SetAktPageDescFromNode( rNd );
    }

    if ( bNewPageDesc && pAktPageDesc )
    {
        PrepareNewPageDesc( pSet, rNd, pPgDesc, pAktPageDesc );
    }
    bBreakBefore = false;
}


bool MSWordExportBase::OutputFollowPageDesc( const SfxItemSet* pSet, const SwTxtNode* pNd )
{
    bool bRet = false;

    if ( pNd &&
         pAktPageDesc &&
         pAktPageDesc != pAktPageDesc->GetFollow() )
    {
        PrepareNewPageDesc( pSet, *pNd, 0, pAktPageDesc->GetFollow() );
        bRet = true;
    }

    return bRet;
}

const SwSectionFmt* MSWordExportBase::GetSectionFormat( const SwNode& rNd ) const
{
    const SwSectionFmt* pFmt = NULL;
    const SwSectionNode* pSect = rNd.FindSectionNode();
    if ( pSect &&
         CONTENT_SECTION == pSect->GetSection().GetType() )
    {
        pFmt = pSect->GetSection().GetFmt();
    }

    return pFmt;
}

sal_uLong MSWordExportBase::GetSectionLineNo( const SfxItemSet* pSet, const SwNode& rNd ) const
{
    const SwFmtLineNumber* pNItem = 0;
    if ( pSet )
    {
        pNItem = &( ItemGet<SwFmtLineNumber>( *pSet, RES_LINENUMBER ) );
    }
    else if ( const SwCntntNode *pNd = rNd.GetCntntNode() )
    {
        pNItem = &( ItemGet<SwFmtLineNumber>( *pNd, RES_LINENUMBER ) );
    }

    return pNItem? pNItem->GetStartValue() : 0;
}

void WW8Export::PrepareNewPageDesc( const SfxItemSet*pSet,
                                      const SwNode& rNd,
                                      const SwFmtPageDesc* pNewPgDescFmt,
                                      const SwPageDesc* pNewPgDesc )
{
    
    
    
    

    sal_uLong nFcPos = ReplaceCr( msword::PageBreak ); 

    
    
    if ( !nFcPos )
        return;

    const SwSectionFmt* pFmt = GetSectionFormat( rNd );
    const sal_uLong nLnNm = GetSectionLineNo( pSet, rNd );

    OSL_ENSURE( pNewPgDescFmt || pNewPgDesc, "Neither page desc format nor page desc provided." );

    if ( pNewPgDescFmt )
    {
        pSepx->AppendSep( Fc2Cp( nFcPos ), *pNewPgDescFmt, rNd, pFmt, nLnNm );
    }
    else if ( pNewPgDesc )
    {
        pSepx->AppendSep( Fc2Cp( nFcPos ), pNewPgDesc, rNd, pFmt, nLnNm );
    }
}

void MSWordExportBase::CorrectTabStopInSet( SfxItemSet& rSet, sal_uInt16 nAbsLeft )
{
    const SvxTabStopItem *pItem =
        sw::util::HasItem<SvxTabStopItem>( rSet, RES_PARATR_TABSTOP );

    if ( pItem )
    {
        
        SvxTabStopItem aTStop(*pItem);
        for ( sal_uInt16 nCnt = 0; nCnt < aTStop.Count(); ++nCnt )
        {
            SvxTabStop& rTab = (SvxTabStop&)aTStop[ nCnt ];
            if ( SVX_TAB_ADJUST_DEFAULT != rTab.GetAdjustment() &&
                rTab.GetTabPos() >= nAbsLeft )
            {
                rTab.GetTabPos() -= nAbsLeft;
            }
            else
            {
                aTStop.Remove( nCnt );
                --nCnt;
            }
        }
        rSet.Put( aTStop );
    }
}

sal_uInt8 WW8Export::GetNumId( sal_uInt16 eNumType )
{
    sal_uInt8 nRet = 0;
    switch( eNumType )
    {
    case SVX_NUM_CHARS_UPPER_LETTER:
    case SVX_NUM_CHARS_UPPER_LETTER_N:  nRet = 3;       break;
    case SVX_NUM_CHARS_LOWER_LETTER:
    case SVX_NUM_CHARS_LOWER_LETTER_N:  nRet = 4;       break;
    case SVX_NUM_ROMAN_UPPER:           nRet = 1;       break;
    case SVX_NUM_ROMAN_LOWER:           nRet = 2;       break;

    case SVX_NUM_BITMAP:
    case SVX_NUM_CHAR_SPECIAL:          nRet = 23;      break;

    
    case SVX_NUM_NUMBER_NONE:           nRet = 0xff;    break;
    }
    return nRet;
}

void WW8AttributeOutput::OutlineNumbering( sal_uInt8 nLvl, const SwNumFmt &rNFmt, const SwFmt &rFmt )
{
    if ( nLvl >= WW8ListManager::nMaxLevel )
        nLvl = WW8ListManager::nMaxLevel-1;

    if ( m_rWW8Export.bWrtWW8 )
    {
        
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_POutLvl );
        m_rWW8Export.pO->push_back( nLvl );
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_PIlvl );
        m_rWW8Export.pO->push_back( nLvl );
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_PIlfo );
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO,
                1 + m_rWW8Export.GetId( *m_rWW8Export.pDoc->GetOutlineNumRule() ) );
    }
    else
    {
        m_rWW8Export.Out_SwNumLvl( nLvl );
        if ( rNFmt.GetPositionAndSpaceMode() ==
                                   SvxNumberFormat::LABEL_WIDTH_AND_POSITION  &&
             rNFmt.GetAbsLSpace() )
        {
            SwNumFmt aNumFmt( rNFmt );
            const SvxLRSpaceItem& rLR =
                ItemGet<SvxLRSpaceItem>( rFmt, RES_LR_SPACE );

            aNumFmt.SetAbsLSpace( writer_cast<short>(
                    aNumFmt.GetAbsLSpace() + rLR.GetLeft() ) );
            m_rWW8Export.Out_NumRuleAnld(
                    *m_rWW8Export.pDoc->GetOutlineNumRule(),
                    aNumFmt, nLvl );
        }
        else
            m_rWW8Export.Out_NumRuleAnld(
                    *m_rWW8Export.pDoc->GetOutlineNumRule(),
                    rNFmt, nLvl );
    }
}


bool WW8Export::DisallowInheritingOutlineNumbering(const SwFmt &rFmt)
{
    bool bRet( false );

    
    
    
    
    if (SFX_ITEM_SET != rFmt.GetItemState(RES_PARATR_NUMRULE, false))
    {
        if (const SwFmt *pParent = rFmt.DerivedFrom())
        {
            if (((const SwTxtFmtColl*)pParent)->IsAssignedToListLevelOfOutlineStyle())
            {
                if (bWrtWW8)
                {
                    SwWW8Writer::InsUInt16(*pO, NS_sprm::LN_POutLvl);
                    pO->push_back(sal_uInt8(9));
                    SwWW8Writer::InsUInt16(*pO, NS_sprm::LN_PIlfo);
                    SwWW8Writer::InsUInt16(*pO, 0);

                    bRet = true;
                }
                /*whats the winword 6 way to do this ?*/
            }
        }
    }

    return bRet;
}

void MSWordExportBase::OutputFormat( const SwFmt& rFmt, bool bPapFmt, bool bChpFmt, bool bFlyFmt )
{
    bool bCallOutSet = true;
    const SwModify* pOldMod = pOutFmtNode;
    pOutFmtNode = &rFmt;

    switch( rFmt.Which() )
    {
    case RES_CONDTXTFMTCOLL:
    case RES_TXTFMTCOLL:
        if( bPapFmt )
        {
            if (((const SwTxtFmtColl&)rFmt).IsAssignedToListLevelOfOutlineStyle())
            {
                int nLvl = ((const SwTxtFmtColl&)rFmt).GetAssignedOutlineStyleLevel();

                
                
                const SwNumFmt& rNFmt = pDoc->GetOutlineNumRule()->Get( static_cast<sal_uInt16>( nLvl ) );
                if ( bStyDef )
                    AttrOutput().OutlineNumbering( static_cast< sal_uInt8 >( nLvl ), rNFmt, rFmt );

                if ( rNFmt.GetPositionAndSpaceMode() ==
                                           SvxNumberFormat::LABEL_WIDTH_AND_POSITION  &&
                     rNFmt.GetAbsLSpace() )
                {
                    SfxItemSet aSet( rFmt.GetAttrSet() );
                    SvxLRSpaceItem aLR(
                        ItemGet<SvxLRSpaceItem>(aSet, RES_LR_SPACE));

                    aLR.SetTxtLeft( aLR.GetTxtLeft() + rNFmt.GetAbsLSpace() );
                    aLR.SetTxtFirstLineOfst( GetWordFirstLineOffset(rNFmt));

                    aSet.Put( aLR );
                    CorrectTabStopInSet( aSet, rNFmt.GetAbsLSpace() );
                    OutputItemSet( aSet, bPapFmt, bChpFmt,
                        i18n::ScriptType::LATIN, mbExportModeRTF);
                    bCallOutSet = false;
                }
            }
            else
            {
                
                
                
                
                
                if ( bStyDef && DisallowInheritingOutlineNumbering(rFmt) )
                {
                    SfxItemSet aSet( rFmt.GetAttrSet() );
                    SvxLRSpaceItem aLR(
                        ItemGet<SvxLRSpaceItem>(aSet, RES_LR_SPACE));
                    aSet.Put( aLR );
                    OutputItemSet( aSet, bPapFmt, bChpFmt,
                        com::sun::star::i18n::ScriptType::LATIN, mbExportModeRTF);
                    bCallOutSet = false;
                }
            }
        }
        break;

    case RES_CHRFMT:
        break;
    case RES_FLYFRMFMT:
        if (bFlyFmt)
        {
            OSL_ENSURE(mpParentFrame, "No parent frame, all broken");

            if (mpParentFrame)
            {
                const SwFrmFmt &rFrmFmt = mpParentFrame->GetFrmFmt();

                SfxItemSet aSet(pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                    RES_FRMATR_END-1);
                aSet.Set(rFrmFmt.GetAttrSet());

                
                
                if (pFlyOffset)
                {
                    aSet.Put(SwFmtHoriOrient(pFlyOffset->X()));
                    aSet.Put(SwFmtVertOrient(pFlyOffset->Y()));
                    SwFmtAnchor aAnchor(rFrmFmt.GetAnchor());
                    aAnchor.SetType(eNewAnchorType);
                    aSet.Put(aAnchor);
                }

                if (SFX_ITEM_SET != aSet.GetItemState(RES_SURROUND))
                    aSet.Put(SwFmtSurround(SURROUND_NONE));

                bOutFlyFrmAttrs = true;
                
                OutputItemSet(aSet, true, false,
                    i18n::ScriptType::LATIN, mbExportModeRTF);
                bOutFlyFrmAttrs = false;

                bCallOutSet = false;
            }
        }
        break;
    case RES_FRMFMT:
        break;
    default:
        OSL_ENSURE( !this, "Which format is exported here?" );
        break;
    }

    if( bCallOutSet )
        OutputItemSet( rFmt.GetAttrSet(), bPapFmt, bChpFmt,
            i18n::ScriptType::LATIN, mbExportModeRTF);
    pOutFmtNode = pOldMod;
}

bool MSWordExportBase::HasRefToObject( sal_uInt16 nTyp, const OUString* pName, sal_uInt16 nSeqNo )
{
    const SwTxtNode* pNd;

    SwFieldType* pType = pDoc->GetSysFldType( RES_GETREFFLD );
    SwIterator<SwFmtFld, SwFieldType> aFmtFlds( *pType );
    for ( SwFmtFld* pFmtFld = aFmtFlds.First(); pFmtFld; pFmtFld = aFmtFlds.Next() )
    {
        if ( pFmtFld->GetTxtFld() && nTyp == pFmtFld->GetField()->GetSubType() &&
             0 != ( pNd = pFmtFld->GetTxtFld()->GetpTxtNode() ) &&
             pNd->GetNodes().IsDocNodes() )
        {
            const SwGetRefField& rRFld = *static_cast< SwGetRefField* >( pFmtFld->GetField() );
            switch ( nTyp )
            {
                case REF_BOOKMARK:
                case REF_SETREFATTR:
                    if ( pName && *pName == rRFld.GetSetRefName() )
                        return true;
                    break;
                case REF_FOOTNOTE:
                case REF_ENDNOTE:
                    if ( nSeqNo == rRFld.GetSeqNo() )
                        return true;
                    break;
                case REF_SEQUENCEFLD:
                    break;      
                case REF_OUTLINE:
                    break;      
            }
        }
    }

    return false;
}

OUString MSWordExportBase::GetBookmarkName( sal_uInt16 nTyp, const OUString* pName, sal_uInt16 nSeqNo )
{
    OUString sRet;
    switch ( nTyp )
    {
        case REF_SETREFATTR:
            if ( pName )
            {
                sRet += "Ref_";
                sRet += *pName;
            }
            break;
        case REF_SEQUENCEFLD:
            break;      
        case REF_BOOKMARK:
            if ( pName )
                sRet = *pName;
            break;
        case REF_OUTLINE:
            break;      
        case REF_FOOTNOTE:
            sRet += "_RefF";
            sRet += OUString::number( nSeqNo );
            break;
        case REF_ENDNOTE:
            sRet += "_RefE";
            sRet += OUString::number( nSeqNo );
            break;
    }
    return BookmarkToWord( sRet ); 
}


/* File CHRATR.HXX: */
void WW8AttributeOutput::RTLAndCJKState( bool bIsRTL, sal_uInt16 nScript )
{
    if ( m_rWW8Export.bWrtWW8 && bIsRTL )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_CFBiDi );
        m_rWW8Export.pO->push_back( (sal_uInt8)1 );
    }

    
    if ( m_rWW8Export.bWrtWW8 && nScript == i18n::ScriptType::COMPLEX && !bIsRTL )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_CComplexScript );
        m_rWW8Export.pO->push_back( (sal_uInt8)0x81 );
        m_rWW8Export.pDop->bUseThaiLineBreakingRules = true;
    }
}

void WW8AttributeOutput::EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner )
{
    m_rWW8Export.pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell() - (mbOnTOXEnding?2:0), m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );
    mbOnTOXEnding = false;
    m_rWW8Export.pO->clear();

    if ( pTextNodeInfoInner.get() != NULL )
    {
        if ( pTextNodeInfoInner->isEndOfLine() )
        {
            TableRowEnd( pTextNodeInfoInner->getDepth() );

            SVBT16 nSty;
            ShortToSVBT16( 0, nSty );
            m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), (sal_uInt8*)&nSty, (sal_uInt8*)&nSty+2 );     
            TableInfoRow( pTextNodeInfoInner );
            m_rWW8Export.pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data());
            m_rWW8Export.pO->clear();
            
            m_rWW8Export.pChpPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data());
        }
    }
}

void WW8AttributeOutput::StartRunProperties()
{
    WW8_WrPlcFld* pCurrentFields = m_rWW8Export.CurrentFieldPlc();
    m_nFieldResults = pCurrentFields ? pCurrentFields->ResultCount() : 0;
}


void WW8AttributeOutput::StartRun( const SwRedlineData* pRedlineData, bool /*bSingleEmptyRun*/ )
{
    if (pRedlineData)
    {
        const OUString &rComment = pRedlineData->GetComment();
        
        if (!rComment.isEmpty() && (m_rWW8Export.nTxtTyp == TXT_MAINTEXT))
        {
            if (m_rWW8Export.pAtn->IsNewRedlineComment(pRedlineData))
            {
                m_rWW8Export.pAtn->Append( m_rWW8Export.Fc2Cp( m_rWW8Export.Strm().Tell() ), pRedlineData );
                m_rWW8Export.WritePostItBegin( m_rWW8Export.pO );
            }
        }
    }
}

void WW8AttributeOutput::OnTOXEnding()
{
    mbOnTOXEnding = true;
}

void WW8AttributeOutput::EndRunProperties( const SwRedlineData* pRedlineData )
{
    Redline( pRedlineData );

    WW8_WrPlcFld* pCurrentFields = m_rWW8Export.CurrentFieldPlc();
    sal_uInt16 nNewFieldResults = pCurrentFields ? pCurrentFields->ResultCount() : 0;

    bool bExportedFieldResult = ( m_nFieldResults != nNewFieldResults );

    
    
    
    
    if ( !bExportedFieldResult )
    {
        m_rWW8Export.pChpPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(),
                m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );
    }
    m_rWW8Export.pO->clear();
}

void WW8AttributeOutput::RunText( const OUString& rText, rtl_TextEncoding eCharSet )
{
    RawText( rText, m_rWW8Export.bWrtWW8, eCharSet );
}

void WW8AttributeOutput::RawText( const OUString& rText, bool bForceUnicode, rtl_TextEncoding eCharSet )
{
    m_rWW8Export.OutSwString( rText, 0, rText.getLength(), bForceUnicode, eCharSet );
}

void WW8AttributeOutput::OutputFKP()
{
    if ( !m_rWW8Export.pO->empty() )
    {
        m_rWW8Export.pChpPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(),
                m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );
        m_rWW8Export.pO->clear();
    }
}

void WW8AttributeOutput::ParagraphStyle( sal_uInt16 nStyle )
{
    OSL_ENSURE( m_rWW8Export.pO->empty(), " pO ist am ZeilenEnde nicht leer" );

    SVBT16 nSty;
    ShortToSVBT16( nStyle, nSty );
    m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), (sal_uInt8*)&nSty, (sal_uInt8*)&nSty+2 );     
}

void WW8AttributeOutput::OutputWW8Attribute( sal_uInt8 nId, bool bVal )
{
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( 8 == nId ? NS_sprm::LN_CFDStrike : NS_sprm::LN_CFBold + nId );
    else if (8 == nId )
        return; 
    else
        m_rWW8Export.pO->push_back( 85 + nId );

    m_rWW8Export.pO->push_back( bVal ? 1 : 0 );
}

void WW8AttributeOutput::OutputWW8AttributeCTL( sal_uInt8 nId, bool bVal )
{
    OSL_ENSURE( nId <= 1, "out of range" );
    if ( !m_rWW8Export.bWrtWW8 || nId > 1 )
        return;

    m_rWW8Export.InsUInt16( NS_sprm::LN_CFBoldBi + nId );
    m_rWW8Export.pO->push_back( bVal ? 1 : 0 );
}

void WW8AttributeOutput::CharFont( const SvxFontItem& rFont )
{
    sal_uInt16 nFontID = m_rWW8Export.GetId( rFont );

    if ( m_rWW8Export.bWrtWW8 )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_CRgFtc0 );
        m_rWW8Export.InsUInt16( nFontID );
        m_rWW8Export.InsUInt16( NS_sprm::LN_CRgFtc2 );
    }
    else
        m_rWW8Export.pO->push_back( 93 );

    m_rWW8Export.InsUInt16( nFontID );
}

void WW8AttributeOutput::CharFontCTL( const SvxFontItem& rFont )
{
    
    
    sal_uInt16 nFontID = m_rWW8Export.GetId( rFont );
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_CFtcBi );
    else
        m_rWW8Export.pO->push_back( 93 );
    m_rWW8Export.InsUInt16( nFontID );
}

void WW8AttributeOutput::CharFontCJK( const SvxFontItem& rFont )
{
    
    
    sal_uInt16 nFontID = m_rWW8Export.GetId( rFont );
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_CRgFtc1 );
    else
        m_rWW8Export.pO->push_back( 93 );
    m_rWW8Export.InsUInt16( nFontID );
}

void WW8AttributeOutput::CharWeightCTL( const SvxWeightItem& rWeight )
{
    
    
    if (m_rWW8Export.bWrtWW8)
    {
        OutputWW8AttributeCTL( 0, WEIGHT_BOLD == rWeight.GetWeight());
    }
    else
    {
        OutputWW8Attribute( 0, WEIGHT_BOLD == rWeight.GetWeight());
    }
}

void WW8AttributeOutput::CharPostureCTL( const SvxPostureItem& rPosture )
{
    
    
    if (m_rWW8Export.bWrtWW8)
    {
        OutputWW8AttributeCTL( 1, ITALIC_NONE != rPosture.GetPosture() );
    }
    else
    {
        OutputWW8Attribute( 1, ITALIC_NONE != rPosture.GetPosture() );
    }
}

void WW8AttributeOutput::CharPosture( const SvxPostureItem& rPosture )
{
    OutputWW8Attribute( 1, ITALIC_NONE != rPosture.GetPosture() );
}

void WW8AttributeOutput::CharWeight( const SvxWeightItem& rWeight )
{
    OutputWW8Attribute( 0, WEIGHT_BOLD == rWeight.GetWeight() );
}


void WW8AttributeOutput::CharContour( const SvxContourItem& rContour )
{
    OutputWW8Attribute( 3, rContour.GetValue() ? true : false);
}

void WW8AttributeOutput::CharShadow( const SvxShadowedItem& rShadow )
{
    OutputWW8Attribute( 4, rShadow.GetValue() ? true : false);
}

void WW8AttributeOutput::CharKerning( const SvxKerningItem& rKerning )
{
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_CDxaSpace );
    else
        m_rWW8Export.pO->push_back( 96 );

    m_rWW8Export.InsUInt16( rKerning.GetValue() );
}

void WW8AttributeOutput::CharAutoKern( const SvxAutoKernItem& rAutoKern )
{
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_CHpsKern );
    else
        m_rWW8Export.pO->push_back( 107 );

    m_rWW8Export.InsUInt16( rAutoKern.GetValue() ? 1 : 0 );
}

void WW8AttributeOutput::CharAnimatedText( const SvxBlinkItem& rBlink )
{
    if ( m_rWW8Export.bWrtWW8 )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_CSfxText );
        
        m_rWW8Export.InsUInt16( rBlink.GetValue() ? 2 : 0 );
    }
}

void WW8AttributeOutput::CharCrossedOut( const SvxCrossedOutItem& rCrossed )
{
    FontStrikeout eSt = rCrossed.GetStrikeout();
    if ( STRIKEOUT_DOUBLE == eSt )
    {
        OutputWW8Attribute( 8, true );
        return;
    }
    if ( STRIKEOUT_NONE != eSt )
    {
        OutputWW8Attribute( 2, true );
        return;
    }

    
    OutputWW8Attribute( 8, false );
    OutputWW8Attribute( 2, false );
}

void WW8AttributeOutput::CharCaseMap( const SvxCaseMapItem& rCaseMap )
{
    sal_uInt16 eSt = rCaseMap.GetValue();
    switch ( eSt )
    {
        case SVX_CASEMAP_KAPITAELCHEN:
            OutputWW8Attribute( 5, true );
            return;
        case SVX_CASEMAP_VERSALIEN:
            OutputWW8Attribute( 6, true );
            return;
        case SVX_CASEMAP_TITEL:
            
            break;
        default:
            
            OutputWW8Attribute( 5, false );
            OutputWW8Attribute( 6, false );
            return;
    }
}

void WW8AttributeOutput::CharHidden( const SvxCharHiddenItem& rHidden )
{
    OutputWW8Attribute( 7, rHidden.GetValue() );
}

void WW8AttributeOutput::CharBorder( const SvxBorderLine* pAllBorder, const sal_uInt16 /*nDist*/, const bool bShadow )
{
    m_rWW8Export.Out_BorderLine( *m_rWW8Export.pO, pAllBorder, 0, NS_sprm::LN_CBrc, bShadow );
}

void WW8AttributeOutput::CharUnderline( const SvxUnderlineItem& rUnderline )
{
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_CKul );
    else
        m_rWW8Export.pO->push_back( 94 );

    const SfxPoolItem* pItem = m_rWW8Export.HasItem( RES_CHRATR_WORDLINEMODE );
    bool bWord = false;
    if (pItem)
        bWord = ((const SvxWordLineModeItem*)pItem)->GetValue() ? true : false;

    
    
    
    
    
    sal_uInt8 b = 0;
    switch ( rUnderline.GetLineStyle() )
    {
        case UNDERLINE_SINGLE:
            b = ( bWord ) ? 2 : 1;
            break;
        case UNDERLINE_BOLD:
            b = m_rWW8Export.bWrtWW8 ?  6 : 1;
            break;
        case UNDERLINE_DOUBLE:
            b = 3;
            break;
        case UNDERLINE_DOTTED:
            b = 4;
            break;
        case UNDERLINE_DASH:
            b = m_rWW8Export.bWrtWW8 ?  7 : 4;
            break;
        case UNDERLINE_DASHDOT:
            b = m_rWW8Export.bWrtWW8 ?  9 : 4;
            break;
        case UNDERLINE_DASHDOTDOT:
            b = m_rWW8Export.bWrtWW8 ? 10 : 4;
            break;
        case UNDERLINE_WAVE:
            b = m_rWW8Export.bWrtWW8 ? 11 : 3;
            break;
        
        case UNDERLINE_BOLDDOTTED:
            b = m_rWW8Export.bWrtWW8 ? 20 : 4;
            break;
        case UNDERLINE_BOLDDASH:
            b = m_rWW8Export.bWrtWW8 ? 23 : 4;
            break;
        case UNDERLINE_LONGDASH:
            b = m_rWW8Export.bWrtWW8 ? 39 : 4;
            break;
        case UNDERLINE_BOLDLONGDASH:
            b = m_rWW8Export.bWrtWW8 ? 55 : 4;
            break;
        case UNDERLINE_BOLDDASHDOT:
            b = m_rWW8Export.bWrtWW8 ? 25 : 4;
            break;
        case UNDERLINE_BOLDDASHDOTDOT:
            b = m_rWW8Export.bWrtWW8 ? 26 : 4;
            break;
        case UNDERLINE_BOLDWAVE:
            b = m_rWW8Export.bWrtWW8 ? 27 : 3;
            break;
        case UNDERLINE_DOUBLEWAVE:
            b = m_rWW8Export.bWrtWW8 ? 43 : 3;
            break;
        case UNDERLINE_NONE:
            b = 0;
            break;
        default:
            OSL_ENSURE( rUnderline.GetLineStyle() == UNDERLINE_NONE, "Unhandled underline type" );
            break;
    }

    m_rWW8Export.pO->push_back( b );
    Color aColor = rUnderline.GetColor();
    if( aColor != COL_TRANSPARENT )
    {
        if( m_rWW8Export.bWrtWW8 )
        {
            m_rWW8Export.InsUInt16( 0x6877 );

            m_rWW8Export.InsUInt32( wwUtility::RGBToBGR( aColor.GetColor() ) );
        }
    }
}

void WW8AttributeOutput::CharLanguage( const SvxLanguageItem& rLanguage )
{
    sal_uInt16 nId = 0;
    if ( m_rWW8Export.bWrtWW8 )
    {
        switch ( rLanguage.Which() )
        {
            case RES_CHRATR_LANGUAGE:
                nId = NS_sprm::LN_CRgLid0_80;
                break;
            case RES_CHRATR_CJK_LANGUAGE:
                nId = NS_sprm::LN_CRgLid1_80;
                break;
            case RES_CHRATR_CTL_LANGUAGE:
                nId = NS_sprm::LN_CLidBi;
                break;
        }
    }
    else
        nId = 97;

    if ( nId )
    {
        if ( m_rWW8Export.bWrtWW8 ) 
            m_rWW8Export.InsUInt16( nId );
        else
            m_rWW8Export.pO->push_back( static_cast<sal_uInt8>(nId) );
        m_rWW8Export.InsUInt16( rLanguage.GetLanguage() );

        
        
        if ( nId == NS_sprm::LN_CRgLid0_80 )
        {
            m_rWW8Export.InsUInt16( NS_sprm::LN_CRgLid0 );
            m_rWW8Export.InsUInt16( rLanguage.GetLanguage() );
        }
        else if ( nId == NS_sprm::LN_CRgLid1_80 )
        {
            m_rWW8Export.InsUInt16( NS_sprm::LN_CRgLid1 );
            m_rWW8Export.InsUInt16( rLanguage.GetLanguage() );
        }
    }
}

void WW8AttributeOutput::CharEscapement( const SvxEscapementItem& rEscapement )
{
    sal_uInt8 b = 0xFF;
    short nEsc = rEscapement.GetEsc(), nProp = rEscapement.GetProp();
    if ( !nEsc )
    {
        b = 0;
        nEsc = 0;
        nProp = 100;
    }
    else if ( DFLT_ESC_PROP == nProp )
    {
        if ( DFLT_ESC_SUB == nEsc || DFLT_ESC_AUTO_SUB == nEsc )
            b = 2;
        else if ( DFLT_ESC_SUPER == nEsc || DFLT_ESC_AUTO_SUPER == nEsc )
            b = 1;
    }

    if ( 0xFF != b )
    {
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_CIss );
        else
            m_rWW8Export.pO->push_back( 104 );

        m_rWW8Export.pO->push_back( b );
    }

    if ( 0 == b || 0xFF == b )
    {
        long nHeight = ((SvxFontHeightItem&)m_rWW8Export.GetItem(
                                    RES_CHRATR_FONTSIZE )).GetHeight();
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_CHpsPos );
        else
            m_rWW8Export.pO->push_back( 101 );

        m_rWW8Export.InsUInt16( (short)(( nHeight * nEsc + 500 ) / 1000 ));

        if( 100 != nProp || !b )
        {
            if( m_rWW8Export.bWrtWW8 )
                m_rWW8Export.InsUInt16( NS_sprm::LN_CHps );
            else
                m_rWW8Export.pO->push_back( 99 );

            m_rWW8Export.InsUInt16(
                msword_cast<sal_uInt16>((nHeight * nProp + 500 ) / 1000));
        }
    }
}

void WW8AttributeOutput::CharFontSize( const SvxFontHeightItem& rHeight )
{
    sal_uInt16 nId = 0;
    if ( m_rWW8Export.bWrtWW8 )
    {
        switch ( rHeight.Which() )
        {
            case RES_CHRATR_FONTSIZE:
            case RES_CHRATR_CJK_FONTSIZE:
                nId = NS_sprm::LN_CHps;
                break;
            case RES_CHRATR_CTL_FONTSIZE:
                nId = NS_sprm::LN_CHpsBi;
                break;
        }
    }
    else
        nId = 99;

    if ( nId )
    {
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( nId );
        else
            m_rWW8Export.pO->push_back( static_cast<sal_uInt8>(nId) );

        m_rWW8Export.InsUInt16( (sal_uInt16)(( rHeight.GetHeight() + 5 ) / 10 ) );
    }
}

void WW8AttributeOutput::CharScaleWidth( const SvxCharScaleWidthItem& rScaleWidth )
{
    if ( m_rWW8Export.bWrtWW8 )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_CCharScale );
        m_rWW8Export.InsUInt16( rScaleWidth.GetValue() );
    }
}

void WW8AttributeOutput::CharRelief( const SvxCharReliefItem& rRelief )
{
    if ( m_rWW8Export.bWrtWW8 )
    {
        sal_uInt16 nId;
        switch ( rRelief.GetValue() )
        {
            case RELIEF_EMBOSSED:   nId = NS_sprm::LN_CFEmboss;     break;
            case RELIEF_ENGRAVED:   nId = NS_sprm::LN_CFImprint;    break;
            default:                nId = 0;                        break;
        }

        if( nId )
        {
            m_rWW8Export.InsUInt16( nId );
            m_rWW8Export.pO->push_back( (sal_uInt8)0x81 );
        }
        else
        {
            
            m_rWW8Export.InsUInt16( NS_sprm::LN_CFEmboss );
            m_rWW8Export.pO->push_back( (sal_uInt8)0x0 );
            m_rWW8Export.InsUInt16( NS_sprm::LN_CFImprint );
            m_rWW8Export.pO->push_back( (sal_uInt8)0x0 );
        }
    }
}

void WW8AttributeOutput::CharRotate( const SvxCharRotateItem& rRotate )
{
    
    if ( !rRotate.GetValue() )
        return;

    if ( m_rWW8Export.bWrtWW8 && !m_rWW8Export.IsInTable() )
    {
        
        
        

        m_rWW8Export.InsUInt16( NS_sprm::LN_CEastAsianLayout );
        m_rWW8Export.pO->push_back( (sal_uInt8)0x06 ); 
        m_rWW8Export.pO->push_back( (sal_uInt8)0x01 );

        m_rWW8Export.InsUInt16( rRotate.IsFitToLine() ? 1 : 0 );
        static const sal_uInt8 aZeroArr[ 3 ] = { 0, 0, 0 };
        m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), aZeroArr, aZeroArr+3);
    }
}

void WW8AttributeOutput::CharEmphasisMark( const SvxEmphasisMarkItem& rEmphasisMark )
{
    if ( m_rWW8Export.bWrtWW8 )
    {
        sal_uInt8 nVal;
        switch ( rEmphasisMark.GetValue() )
        {
            case EMPHASISMARK_NONE:             nVal = 0;   break;
            case EMPHASISMARK_SIDE_DOTS:        nVal = 2;   break;
            case EMPHASISMARK_CIRCLE_ABOVE:     nVal = 3;   break;
            case EMPHASISMARK_DOTS_BELOW:       nVal = 4;   break;
            
            default:                            nVal = 1;   break;
        }

        m_rWW8Export.InsUInt16( NS_sprm::LN_CKcd );
        m_rWW8Export.pO->push_back( nVal );
    }
}






sal_uInt8 WW8Export::TransCol( const Color& rCol )
{
    sal_uInt8 nCol = 0;      
    switch( rCol.GetColor() )
    {
    case COL_BLACK:         nCol = 1;   break;
    case COL_BLUE:          nCol = 9;   break;
    case COL_GREEN:         nCol = 11;  break;
    case COL_CYAN:          nCol = 10;  break;
    case COL_RED:           nCol = 13;  break;
    case COL_MAGENTA:       nCol = 12;  break;
    case COL_BROWN:         nCol = 14;  break;
    case COL_GRAY:          nCol = 15;  break;
    case COL_LIGHTGRAY:     nCol = 16;  break;
    case COL_LIGHTBLUE:     nCol = 2;   break;
    case COL_LIGHTGREEN:    nCol = 4;   break;
    case COL_LIGHTCYAN:     nCol = 3;   break;
    case COL_LIGHTRED:      nCol = 6;   break;
    case COL_LIGHTMAGENTA:  nCol = 5;   break;
    case COL_YELLOW:        nCol = 7;   break;
    case COL_WHITE:         nCol = 8;   break;
    case COL_AUTO:          nCol = 0;   break;

    default:
        if( !pBmpPal )
        {
            pBmpPal = new BitmapPalette( 16 );
            static const ColorData aColArr[ 16 ] = {
                COL_BLACK,      COL_LIGHTBLUE,  COL_LIGHTCYAN,  COL_LIGHTGREEN,
                COL_LIGHTMAGENTA,COL_LIGHTRED,  COL_YELLOW,     COL_WHITE,
                COL_BLUE,       COL_CYAN,       COL_GREEN,      COL_MAGENTA,
                COL_RED,        COL_BROWN,      COL_GRAY,       COL_LIGHTGRAY
            };

            for( sal_uInt16 i = 0; i < 16; ++i )
                pBmpPal->operator[]( i ) = Color( aColArr[ i ] );
        }
        nCol = static_cast< sal_uInt8 >(pBmpPal->GetBestIndex( rCol ) + 1);
        break;
    }
    return nCol;
}







bool WW8Export::TransBrush(const Color& rCol, WW8_SHD& rShd)
{
    if( rCol.GetTransparency() )
        rShd = WW8_SHD();               
    else
    {
        rShd.SetFore( 0);
        rShd.SetBack( TransCol( rCol ) );
        rShd.SetStyle( bWrtWW8, 0 );
    }
    return !rCol.GetTransparency();
}

sal_uInt32 SuitableBGColor(sal_uInt32 nIn)
{
    if (nIn == COL_AUTO)
        return 0xFF000000;
    return wwUtility::RGBToBGR(nIn);
}

void WW8AttributeOutput::CharColor( const SvxColorItem& rColor )
{
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_CIco );
    else
        m_rWW8Export.pO->push_back( 98 );

    sal_uInt8 nColor = m_rWW8Export.TransCol( rColor.GetValue() );
    m_rWW8Export.pO->push_back( nColor );

    if ( m_rWW8Export.bWrtWW8 && nColor )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_CCv );
        m_rWW8Export.InsUInt32( wwUtility::RGBToBGR( rColor.GetValue().GetColor() ) );
    }
}

void WW8AttributeOutput::CharBackground( const SvxBrushItem& rBrush )
{
    if( m_rWW8Export.bWrtWW8 )   
    {
        WW8_SHD aSHD;

        m_rWW8Export.TransBrush( rBrush.GetColor(), aSHD );
        
        m_rWW8Export.InsUInt16( NS_sprm::LN_CShd );
        m_rWW8Export.InsUInt16( aSHD.GetValue() );

        
        
        m_rWW8Export.InsUInt16( 0xCA71 );
        m_rWW8Export.pO->push_back( 10 );
        m_rWW8Export.InsUInt32( 0xFF000000 );
        m_rWW8Export.InsUInt32( SuitableBGColor( rBrush.GetColor().GetColor() ) );
        m_rWW8Export.InsUInt16( 0x0000);
    }
}

void WW8AttributeOutput::TextINetFormat( const SwFmtINetFmt& rINet )
{
    if ( !rINet.GetValue().isEmpty() )
    {
        const sal_uInt16 nId = rINet.GetINetFmtId();
        const OUString& rStr = rINet.GetINetFmt();
        if (rStr.isEmpty())
        {
            OSL_ENSURE( false, "WW8AttributeOutput::TextINetFormat(..) - missing unvisited character format at hyperlink attribute" );
        }

        const SwCharFmt* pFmt = IsPoolUserFmt( nId )
                        ? m_rWW8Export.pDoc->FindCharFmtByName( rStr )
                        : m_rWW8Export.pDoc->GetCharFmtFromPool( nId );

        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_CIstd );
        else
            m_rWW8Export.pO->push_back( 80 );

        m_rWW8Export.InsUInt16( m_rWW8Export.GetId( *pFmt ) );
    }
}






static void InsertSpecialChar( WW8Export& rWrt, sal_uInt8 c,
                               OUString* pLinkStr = 0L,
                               bool bIncludeEmptyPicLocation = false )
{
    ww::bytes aItems;
    rWrt.GetCurrentItems(aItems);

    if (c == 0x13)
        rWrt.pChpPlc->AppendFkpEntry(rWrt.Strm().Tell());
    else
        rWrt.pChpPlc->AppendFkpEntry(rWrt.Strm().Tell(), aItems.size(), aItems.data());

    rWrt.WriteChar(c);

    
    if ( bIncludeEmptyPicLocation &&
         ( c == 0x13 || c == 0x14 || c == 0x15 ) )
    {
        SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CPicLocation );
        SwWW8Writer::InsUInt32( aItems, 0x00000000 );
    }

    
    if ( rWrt.bWrtWW8 && c == 0x01 && pLinkStr )
    {
        
        SvStream& rStrm = *rWrt.pDataStrm;
        
        const sal_uInt32 nLinkPosInDataStrm = rStrm.Tell();
        
        const sal_uInt16 nEmptyHdrLen = 0x44;
        sal_uInt8 aEmptyHeader[ nEmptyHdrLen ] = { 0 };
        aEmptyHeader[ 4 ] = 0x44;
        rStrm.Write( aEmptyHeader, nEmptyHdrLen );
        
        const sal_uInt16 nFixHdrLen = 0x19;
        sal_uInt8 aFixHeader[ nFixHdrLen ] =
        {
            0x08, 0xD0, 0xC9, 0xEA, 0x79, 0xF9, 0xBA, 0xCE,
            0x11, 0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9,
            0x0B, 0x02, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00,
            0x00,
        };
        rStrm.Write( aFixHeader, nFixHdrLen );
        
        sal_uInt32 nStrLen( pLinkStr->getLength() + 1 );
        SwWW8Writer::WriteLong( rStrm, nStrLen );
        SwWW8Writer::WriteString16( rStrm, *(pLinkStr), false );
        
        SwWW8Writer::WriteLong( rStrm, 0 );
        
        const sal_uInt32 nCurrPos = rStrm.Tell();
        rStrm.Seek( nLinkPosInDataStrm );
        SVBT32 nLen;
        UInt32ToSVBT32( nCurrPos - nLinkPosInDataStrm, nLen );
        rStrm.Write( nLen, 4 );
        rStrm.Seek( nCurrPos );

        
        SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CFFldVanish );
        aItems.push_back( (sal_uInt8)0x81 );
        SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CPicLocation );
        SwWW8Writer::InsUInt32( aItems, nLinkPosInDataStrm );
        SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CFData );
        aItems.push_back( (sal_uInt8)0x01 );
    }

    
    
    
    
    
    if( rWrt.bWrtWW8 )
    {
        SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CFSpec );
        aItems.push_back( 1 );
    }
    else
    {
        aItems.push_back( 117 ); 
        aItems.push_back( 1 );
    }

    rWrt.pChpPlc->AppendFkpEntry(rWrt.Strm().Tell(), aItems.size(), aItems.data());
}

static OUString lcl_GetExpandedField(const SwField &rFld)
{
    OUString sRet(rFld.ExpandField(true));

    
    return sRet.replace(0x0A, 0x0B);
}

WW8_WrPlcFld* WW8Export::CurrentFieldPlc() const
{
    WW8_WrPlcFld* pFldP = NULL;
    switch (nTxtTyp)
    {
        case TXT_MAINTEXT:
            pFldP = pFldMain;
            break;
        case TXT_HDFT:
            pFldP = pFldHdFt;
            break;
        case TXT_FTN:
            pFldP = pFldFtn;
            break;
        case TXT_EDN:
            pFldP = pFldEdn;
            break;
        case TXT_ATN:
            pFldP = pFldAtn;
            break;
        case TXT_TXTBOX:
            pFldP = pFldTxtBxs;
            break;
        case TXT_HFTXTBOX:
            pFldP = pFldHFTxtBxs;
            break;
        default:
            OSL_ENSURE( !this, "was ist das fuer ein SubDoc-Type?" );
    }
    return pFldP;
}

void WW8Export::OutputField( const SwField* pFld, ww::eField eFldType,
    const OUString& rFldCmd, sal_uInt8 nMode )
{
    bool bUnicode = IsUnicode();
    WW8_WrPlcFld* pFldP = CurrentFieldPlc();

    const bool bIncludeEmptyPicLocation = ( eFldType == ww::ePAGE );
    if (WRITEFIELD_START & nMode)
    {
        sal_uInt8 aFld13[2] = { 0x13, 0x00 };  
        
        if (eFldType == ww::eSHAPE)
            aFld13[0] |= 0x80;
        aFld13[1] = static_cast< sal_uInt8 >(eFldType);  
        pFldP->Append( Fc2Cp( Strm().Tell() ), aFld13 );
        InsertSpecialChar( *this, 0x13, 0, bIncludeEmptyPicLocation );
    }
    if (WRITEFIELD_CMD_START & nMode)
    {
        if (bUnicode)
            SwWW8Writer::WriteString16(Strm(), rFldCmd, false);
        else
        {
            SwWW8Writer::WriteString8(Strm(), rFldCmd, false,
                RTL_TEXTENCODING_MS_1252);
        }
        
        
        bool bHandleBookmark = false;

        if (pFld)
        {
            if (pFld->GetTyp()->Which() == RES_GETREFFLD &&
                ( eFldType == ww::ePAGEREF || eFldType == ww::eREF ||
                  eFldType == ww::eNOTEREF || eFldType == ww::eFOOTREF ))
                bHandleBookmark = true;
        }

        if ( bHandleBookmark )
        {
            
            OUString aLinkStr;
            const sal_uInt16 nSubType = pFld->GetSubType();
            const SwGetRefField& rRFld = *(static_cast<const SwGetRefField*>(pFld));
            if ( nSubType == REF_SETREFATTR ||
                 nSubType == REF_BOOKMARK )
            {
                const OUString aRefName(rRFld.GetSetRefName());
                aLinkStr = GetBookmarkName( nSubType, &aRefName, 0 );
            }
            else if ( nSubType == REF_FOOTNOTE ||
                      nSubType == REF_ENDNOTE )
            {
                aLinkStr = GetBookmarkName( nSubType, 0, rRFld.GetSeqNo() );
            }
            else if ( nSubType == REF_SEQUENCEFLD )
            {
                aLinkStr = pFld->GetPar2();
            }
            
            InsertSpecialChar( *this, 0x01, &aLinkStr );
        }
    }
    if (WRITEFIELD_CMD_END & nMode)
    {
        static const sal_uInt8 aFld14[2] = { 0x14, 0xff };
        pFldP->Append( Fc2Cp( Strm().Tell() ), aFld14 );
        pFldP->ResultAdded();
        InsertSpecialChar( *this, 0x14, 0, bIncludeEmptyPicLocation );
    }
    if (WRITEFIELD_END & nMode)
    {
        OUString sOut;
        if( pFld )
            sOut = lcl_GetExpandedField(*pFld);
        else
            sOut = rFldCmd;
        if( !sOut.isEmpty() )
        {
            if( bUnicode )
                SwWW8Writer::WriteString16(Strm(), sOut, false);
            else
            {
                SwWW8Writer::WriteString8(Strm(), sOut, false,
                    RTL_TEXTENCODING_MS_1252);
            }

            if (pFld)
            {
                if (pFld->GetTyp()->Which() == RES_INPUTFLD &&
                    eFldType == ww::eFORMTEXT)
                {
                    sal_uInt8 aArr[12];
                    sal_uInt8 *pArr = aArr;

                    if ( bWrtWW8 )
                    {
                        Set_UInt16( pArr, NS_sprm::LN_CPicLocation );
                        Set_UInt32( pArr, 0x0 );

                        Set_UInt16( pArr, NS_sprm::LN_CFSpec );
                        Set_UInt8( pArr, 1 );

                        Set_UInt16( pArr, NS_sprm::LN_CFNoProof );
                        Set_UInt8( pArr, 1 );
                    }
                    else
                    {
                        Set_UInt8(pArr, 0x68); 
                        Set_UInt32(pArr, 0x0);

                        Set_UInt8( pArr, 117 ); 
                        Set_UInt8( pArr, 1 );
                    }
                    pChpPlc->AppendFkpEntry( Strm().Tell(), static_cast< short >(pArr - aArr), aArr );
                }
            }
        }
    }
    if (WRITEFIELD_CLOSE & nMode)
    {
        sal_uInt8 aFld15[2] = { 0x15, 0x80 };

        if (pFld)
        {
            if (pFld->GetTyp()->Which() == RES_INPUTFLD &&
                eFldType == ww::eFORMTEXT)
            {
                sal_uInt16 nSubType = pFld->GetSubType();

                if (nSubType == REF_SEQUENCEFLD)
                    aFld15[0] |= (0x4 << 5);
            }
        }

        pFldP->Append( Fc2Cp( Strm().Tell() ), aFld15 );
        InsertSpecialChar( *this, 0x15, 0, bIncludeEmptyPicLocation );
    }
}

void WW8Export::StartCommentOutput(const OUString& rName)
{
    OUString sStr(FieldString(ww::eQUOTE));
    sStr += "[" + rName + "] ";
    OutputField(0, ww::eQUOTE, sStr, WRITEFIELD_START | WRITEFIELD_CMD_START);
}

void WW8Export::EndCommentOutput(const OUString& rName)
{
    OUString sStr(" [");
    sStr += rName + "] ";
    OutputField(0, ww::eQUOTE, sStr, WRITEFIELD_CMD_END | WRITEFIELD_END |
        WRITEFIELD_CLOSE);
}

sal_uInt16 MSWordExportBase::GetId( const SwTOXType& rTOXType )
{
    std::vector<const SwTOXType*>::iterator it
        = std::find( aTOXArr.begin(), aTOXArr.end(), &rTOXType );
    if ( it != aTOXArr.end() )
    {
        return it - aTOXArr.begin();
    }
    aTOXArr.push_back( &rTOXType );
    return aTOXArr.size() - 1;
}





static int lcl_CheckForm( const SwForm& rForm, sal_uInt8 nLvl, OUString& rText )
{
    int nRet = 4;
    rText = "";

    
    SwFormTokens aPattern = rForm.GetPattern(nLvl);
    SwFormTokens::iterator aIt = aPattern.begin();
    bool bPgNumFnd = false;
    FormTokenType eTType;

    
    if (! aPattern.empty())
    {
        
        while( ++aIt != aPattern.end() && !bPgNumFnd )
        {
            eTType = aIt->eTokenType;

            switch( eTType )
            {
            case TOKEN_PAGE_NUMS:
                bPgNumFnd = true;
                break;

            case TOKEN_TAB_STOP:
                nRet = 2;
                break;
            case TOKEN_TEXT:
                nRet = 3;
                rText = aIt->sText.copy( 0, 5 ); 
                break;

            case TOKEN_LINK_START:
            case TOKEN_LINK_END:
                break;

            default:
                nRet = 4;
                break;
            }
        }

        if( !bPgNumFnd )
            nRet = 1;
    }

    return nRet;
}

static bool lcl_IsHyperlinked(const SwForm& rForm, sal_uInt16 nTOXLvl)
{
    bool bRes = false;
    for (sal_uInt16 nI = 1; nI < nTOXLvl; ++nI)
    {
        
        SwFormTokens aPattern = rForm.GetPattern(nI);

        if ( !aPattern.empty() )
        {
            SwFormTokens::iterator aIt = aPattern.begin();

            FormTokenType eTType;

            
            while ( ++aIt != aPattern.end() )
            {
                eTType = aIt->eTokenType;
                switch (eTType)
                {
                case TOKEN_LINK_START:
                case TOKEN_LINK_END:
                    bRes = true;
                break;
                default:
                    ;
                }
            }
        }
    }
    return bRes;
}

void AttributeOutputBase::StartTOX( const SwSection& rSect )
{
    if ( const SwTOXBase* pTOX = rSect.GetTOXBase() )
    {
        static const sal_Char sEntryEnd[] = "\" ";

        ww::eField eCode = ww::eTOC;
        OUString sStr = pTOX ->GetMSTOCExpression();
        if ( sStr.isEmpty() )
        {
            switch (pTOX->GetType())
            {
            case TOX_INDEX:
                eCode = ww::eINDEX;
                sStr = FieldString(eCode);

                if (pTOX->GetTOXForm().IsCommaSeparated())
                    sStr += "\\r ";

                if (nsSwTOIOptions::TOI_ALPHA_DELIMITTER & pTOX->GetOptions())
                    sStr += "\\h \"A\" ";

                {
                    OUString aFillTxt;
                    for (sal_uInt8 n = 1; n <= 3; ++n)
                    {
                        OUString aTxt;
                        int nRet = ::lcl_CheckForm(pTOX->GetTOXForm(), n, aTxt);

                        if( 3 == nRet )
                            aFillTxt = aTxt;
                        else if ((4 == nRet) || (2 == nRet)) 
                            aFillTxt = "\t";
                        else
                            aFillTxt = "";
                    }
                    sStr += "\\e \"";
                    sStr += aFillTxt;
                    sStr += sEntryEnd;
                }
                break;

                
            case TOX_ILLUSTRATIONS:
            case TOX_OBJECTS:
            case TOX_TABLES:
                if (!pTOX->IsFromObjectNames())
                {
                    sStr = FieldString(eCode);

                    sStr += "\\c \"";
                    sStr += pTOX->GetSequenceName();
                    sStr += sEntryEnd;

                    OUString aTxt;
                    int nRet = ::lcl_CheckForm( pTOX->GetTOXForm(), 1, aTxt );
                    if (1 == nRet)
                        sStr += "\\n ";
                    else if( 3 == nRet || 4 == nRet )
                    {
                        sStr += "\\p \"";
                        sStr += aTxt;
                        sStr += sEntryEnd;
                    }
                }
                break;

                
                
            default:
                {
                    sStr = FieldString(eCode);

                    OUString sTOption;
                    sal_uInt16 n, nTOXLvl = pTOX->GetLevel();
                    if( !nTOXLvl )
                        ++nTOXLvl;

                    if(nsSwTOXElement::TOX_TABLEADER & pTOX->GetCreateType())
                    {
                        sStr +="\\z " ;
                        GetExport( ).bHideTabLeaderAndPageNumbers = true ;
                    }
                    if(nsSwTOXElement::TOX_TAB_IN_TOC & pTOX->GetCreateType())
                    {
                        sStr +="\\w " ;
                        GetExport( ).bTabInTOC = true ;
                    }
                    if(nsSwTOXElement::TOX_NEWLINE & pTOX->GetCreateType())
                    {
                        sStr +="\\x " ;
                    }
                    if( nsSwTOXElement::TOX_MARK & pTOX->GetCreateType() )
                    {
                        sStr += "\\f ";

                        if( TOX_USER == pTOX->GetType() )
                        {
                            sStr += "\"";
                            sStr += OUString((sal_Char)( 'A' + GetExport( ).GetId( *pTOX->GetTOXType() ) ));
                            sStr += sEntryEnd;
                        }
                    }
                    if(nsSwTOXElement::TOX_BOOKMARK & pTOX->GetCreateType())
                    {
                        sStr += "\\b ";
                        OUString bName = pTOX->GetBookmarkName();
                        sStr += bName;
                        sStr += sEntryEnd;
                    }

                    if( nsSwTOXElement::TOX_OUTLINELEVEL & pTOX->GetCreateType() )
                    {
                        
                        
                        
                        
                        
                        
                        
                        //
                        
                        
                        
                        
                        
                        
                        

                        
                        
                        
                        
                        sal_uInt8 nPosOfLowestNonStandardLvl = MAXLEVEL;
                        const SwTxtFmtColls& rColls = *GetExport().pDoc->GetTxtFmtColls();
                        for( n = rColls.size(); n; )
                        {
                            const SwTxtFmtColl* pColl = rColls[ --n ];
                            sal_uInt16 nPoolId = pColl->GetPoolFmtId();
                            if (
                                
                                (RES_POOLCOLL_HEADLINE1 > nPoolId || RES_POOLCOLL_HEADLINE9 < nPoolId) &&
                                
                                (pColl->IsAssignedToListLevelOfOutlineStyle()) &&
                                
                                (pColl->GetAssignedOutlineStyleLevel() < nPosOfLowestNonStandardLvl)
                                )
                            {
                                nPosOfLowestNonStandardLvl = ::sal::static_int_cast<sal_uInt8>(pColl->GetAssignedOutlineStyleLevel());
                            }
                        }

                        sal_uInt8 nMaxMSAutoEvaluate = nPosOfLowestNonStandardLvl < nTOXLvl ? nPosOfLowestNonStandardLvl : (sal_uInt8)nTOXLvl;

                        
                        if ( nMaxMSAutoEvaluate )
                        {
                            if (nMaxMSAutoEvaluate > WW8ListManager::nMaxLevel)
                                nMaxMSAutoEvaluate = WW8ListManager::nMaxLevel;

                            sStr += "\\o \"1-";
                            sStr += OUString::number(nMaxMSAutoEvaluate);
                            sStr += sEntryEnd;
                        }

                        
                        
                        
                        if( nMaxMSAutoEvaluate < nTOXLvl )
                        {
                            
                            for( n = rColls.size(); n;)
                            {
                                const SwTxtFmtColl* pColl = rColls[ --n ];
                                if (!pColl->IsAssignedToListLevelOfOutlineStyle())
                                    continue;
                                sal_uInt8 nTestLvl =  ::sal::static_int_cast<sal_uInt8>(pColl->GetAssignedOutlineStyleLevel());
                                if (nTestLvl < nTOXLvl && nTestLvl >= nMaxMSAutoEvaluate)
                                {
                                    if (!sTOption.isEmpty())
                                        sTOption += ",";
                                    sTOption += pColl->GetName() + "," + OUString::number( nTestLvl + 1 );
                                }
                            }
                        }
                    }

                    if( nsSwTOXElement::TOX_PARAGRAPH_OUTLINE_LEVEL & pTOX->GetCreateType() )
                    {
                        sStr +="\\u " ;
                    }

                        if( nsSwTOXElement::TOX_TEMPLATE & pTOX->GetCreateType() )
                        {
                            
                            for( n = 0; n < MAXLEVEL; ++n )
                            {
                                const OUString& rStyles = pTOX->GetStyleNames( n );
                                if( !rStyles.isEmpty() )
                                {
                                    sal_Int32 nPos = 0;
                                    OUString sLvl = OUString(',');
                                    sLvl += OUString::number( n + 1 );
                                    do {
                                        OUString sStyle( rStyles.getToken( 0, TOX_STYLE_DELIMITER, nPos ));
                                        if( !sStyle.isEmpty() )
                                        {
                                            SwTxtFmtColl* pColl = GetExport().pDoc->FindTxtFmtCollByName(sStyle);
                                            if (pColl)
                                            {
                                                if (!pColl->IsAssignedToListLevelOfOutlineStyle() || pColl->GetAssignedOutlineStyleLevel() < nTOXLvl)
                                                {
                                                    if( !sTOption.isEmpty() )
                                                        sTOption += ",";
                                                    sTOption += sStyle + sLvl;
                                                }
                                            }
                                        }
                                    } while( -1 != nPos );
                                }
                            }
                        }

                    
                    {
                        OUString aFillTxt;
                        sal_uInt8 nNoPgStt = MAXLEVEL, nNoPgEnd = MAXLEVEL;
                        bool bFirstFillTxt = true, bOnlyText = true;
                        for( n = 0; n < nTOXLvl; ++n )
                        {
                            OUString aTxt;
                            int nRet = ::lcl_CheckForm( pTOX->GetTOXForm(),
                                static_cast< sal_uInt8 >(n+1), aTxt );
                            if( 1 == nRet )
                            {
                                bOnlyText = false;
                                if( MAXLEVEL == nNoPgStt )
                                    nNoPgStt = static_cast< sal_uInt8 >(n+1);
                            }
                            else
                            {
                                if( MAXLEVEL != nNoPgStt &&
                                    MAXLEVEL == nNoPgEnd )
                                    nNoPgEnd = sal_uInt8(n);

                                bOnlyText = bOnlyText && 3 == nRet;
                                if( 3 == nRet || 4 == nRet )
                                {
                                    if( bFirstFillTxt )
                                        aFillTxt = aTxt;
                                    else if( aFillTxt != aTxt )
                                        aFillTxt = "";
                                    bFirstFillTxt = false;
                                }
                            }
                        }
                        if( MAXLEVEL != nNoPgStt )
                        {
                            if (WW8ListManager::nMaxLevel < nNoPgEnd)
                                nNoPgEnd = WW8ListManager::nMaxLevel;
                            sStr += "\\n ";
                            sStr += OUString::number( nNoPgStt );
                            sStr += "-";
                            sStr += OUString::number( nNoPgEnd  );
                            sStr += " ";
                        }
                        if( bOnlyText )
                        {
                            sStr += "\\p \"";
                            sStr += aFillTxt;
                            sStr += sEntryEnd;
                        }
                    }

                    if( !sTOption.isEmpty() )
                    {
                        sStr += "\\t \"";
                        sStr += sTOption;
                        sStr += sEntryEnd;
                    }

                        if (lcl_IsHyperlinked(pTOX->GetTOXForm(), nTOXLvl))
                            sStr += "\\h";
                    break;
                }
            }
        }

        if (!sStr.isEmpty())
        {
            GetExport( ).bInWriteTOX = true;
            GetExport( ).OutputField( 0, eCode, sStr, WRITEFIELD_START | WRITEFIELD_CMD_START |
                WRITEFIELD_CMD_END );
        }
    }

    GetExport( ).bStartTOX = false;
}

void AttributeOutputBase::EndTOX( const SwSection& rSect,bool bCareEnd )
{
    const SwTOXBase* pTOX = rSect.GetTOXBase();
    if ( pTOX )
    {
        ww::eField eCode = TOX_INDEX == pTOX->GetType() ? ww::eINDEX : ww::eTOC;
        GetExport( ).OutputField( 0, eCode, OUString(), WRITEFIELD_CLOSE );
    }
    GetExport( ).bInWriteTOX = false;
    if (bCareEnd)
        OnTOXEnding();
}

bool MSWordExportBase::GetNumberFmt(const SwField& rFld, OUString& rStr)
{
    
    bool bHasFmt = false;
    SvNumberFormatter* pNFmtr = pDoc->GetNumberFormatter();
    sal_uInt32 nFmtIdx = rFld.GetFormat();
    const SvNumberformat* pNumFmt = pNFmtr->GetEntry( nFmtIdx );
    if( pNumFmt )
    {
        sal_uInt16 nLng = rFld.GetLanguage();
        LocaleDataWrapper aLocDat(pNFmtr->GetComponentContext(),
                                  LanguageTag(nLng));

        OUString sFmt(pNumFmt->GetMappedFormatstring(GetNfKeywordTable(),
            aLocDat));

        if (!sFmt.isEmpty())
        {
            sw::ms::SwapQuotesInField(sFmt);

            rStr = "\\@\"" + sFmt + "\" " ;
            bHasFmt = true;
        }
    }
    return bHasFmt;
}

void AttributeOutputBase::GetNumberPara( OUString& rStr, const SwField& rFld )
{
    switch(rFld.GetFormat())
    {
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:
            rStr += "\\* ALPHABETIC ";
            break;
        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:
            rStr += "\\* alphabetic ";
            break;
        case SVX_NUM_ROMAN_UPPER:
            rStr += "\\* ROMAN ";
            break;
        case SVX_NUM_ROMAN_LOWER:
            rStr += "\\* roman ";
            break;
        default:
            OSL_ENSURE(rFld.GetFormat() == SVX_NUM_ARABIC,
                "Unknown numbering type exported as default of Arabic\n");
            
        case SVX_NUM_ARABIC:
            rStr += "\\* ARABIC ";
            break;
        case SVX_NUM_PAGEDESC:
            
            break;
    }
}

void WW8Export::WritePostItBegin( ww::bytes* pOut )
{
    sal_uInt8 aArr[ 3 ];
    sal_uInt8* pArr = aArr;

    
    if( bWrtWW8 )
        Set_UInt16( pArr, NS_sprm::LN_CFSpec );
    else
        Set_UInt8( pArr, 117 ); 
    Set_UInt8( pArr, 1 );

    pChpPlc->AppendFkpEntry( Strm().Tell() );
    WriteChar( 0x05 );              

    if( pOut )
        pOut->insert( pOut->end(), aArr, pArr );
    else
        pChpPlc->AppendFkpEntry( Strm().Tell(), static_cast< short >(pArr - aArr), aArr );
}

OUString FieldString(ww::eField eIndex)
{
    if (const char *pField = ww::GetEnglishFieldName(eIndex))
        return " " + OUString::createFromAscii(pField) + " ";
    return OUString("  ");
}

void WW8AttributeOutput::HiddenField( const SwField& rFld )
{
    OUString sExpand(rFld.GetPar2());

    
    sExpand = sExpand.replace(0x0A, 0x0B);
    m_rWW8Export.pChpPlc->AppendFkpEntry(m_rWW8Export.Strm().Tell());
    if (m_rWW8Export.IsUnicode())
    {
        SwWW8Writer::WriteString16(m_rWW8Export.Strm(), sExpand, false);
        static sal_uInt8 aArr[] =
        {
            0x3C, 0x08, 0x1
        };
        m_rWW8Export.pChpPlc->AppendFkpEntry(m_rWW8Export.Strm().Tell(), sizeof(aArr), aArr);
    }
    else
    {
        SwWW8Writer::WriteString8(m_rWW8Export.Strm(), sExpand, false,
            RTL_TEXTENCODING_MS_1252);
        static sal_uInt8 aArr[] =
        {
            92, 0x1
        };
        m_rWW8Export.pChpPlc->AppendFkpEntry(m_rWW8Export.Strm().Tell(), sizeof(aArr), aArr);
    }
}

void WW8AttributeOutput::SetField( const SwField& rFld, ww::eField eType, const OUString& rCmd )
{
    const SwSetExpField* pSet=(const SwSetExpField*)(&rFld);
    const OUString &rVar = pSet->GetPar2();

    sal_uLong nFrom = m_rWW8Export.Fc2Cp(m_rWW8Export.Strm().Tell());

    GetExport().OutputField(&rFld, eType, rCmd, WRITEFIELD_START |
        WRITEFIELD_CMD_START | WRITEFIELD_CMD_END);

    /*
    Is there a bookmark at the start position of this field, if so
    move it to the 0x14 of the result of the field.  This is what word
    does. MoveFieldMarks moves any bookmarks at this position to
    the beginning of the field result, and marks the bookmark as a
    fieldbookmark which is to be ended before the field end mark
    instead of after it like a normal bookmark.
    */
    m_rWW8Export.MoveFieldMarks(nFrom,m_rWW8Export.Fc2Cp(m_rWW8Export.Strm().Tell()));

    if (!rVar.isEmpty())
    {
        if (m_rWW8Export.IsUnicode())
            SwWW8Writer::WriteString16(m_rWW8Export.Strm(), rVar, false);
        else
        {
            SwWW8Writer::WriteString8(m_rWW8Export.Strm(), rVar, false,
                RTL_TEXTENCODING_MS_1252);
        }
    }
    GetExport().OutputField(&rFld, eType, rCmd, WRITEFIELD_CLOSE);
}

void WW8AttributeOutput::PostitField( const SwField* pFld )
{
    const SwPostItField *pPFld = (const SwPostItField*)pFld;
    m_rWW8Export.pAtn->Append( m_rWW8Export.Fc2Cp( m_rWW8Export.Strm().Tell() ), pPFld );
    m_rWW8Export.WritePostItBegin( m_rWW8Export.pO );
}

bool WW8AttributeOutput::DropdownField( const SwField* pFld )
{
    bool bExpand = true;
    if ( m_rWW8Export.bWrtWW8 )
    {
        const SwDropDownField& rFld2 = *(SwDropDownField*)pFld;
        uno::Sequence<OUString> aItems =
            rFld2.GetItemSequence();
        GetExport().DoComboBox(rFld2.GetName(),
                           rFld2.GetHelp(),
                           rFld2.GetToolTip(),
                           rFld2.GetSelectedItem(), aItems);
        bExpand = false;
    }
    return bExpand;
}

bool WW8AttributeOutput::PlaceholderField( const SwField* )
{
    return true; 
}

void WW8AttributeOutput::RefField( const SwField &rFld, const OUString &rRef)
{
    OUString sStr( FieldString( ww::eREF ) );
    sStr += "\"" + rRef + "\" ";
    m_rWW8Export.OutputField( &rFld, ww::eREF, sStr, WRITEFIELD_START |
        WRITEFIELD_CMD_START | WRITEFIELD_CMD_END );
    OUString sVar = lcl_GetExpandedField( rFld );
    if ( !sVar.isEmpty() )
    {
        if ( m_rWW8Export.IsUnicode() )
            SwWW8Writer::WriteString16( m_rWW8Export.Strm(), sVar, false );
        else
        {
            SwWW8Writer::WriteString8( m_rWW8Export.Strm(), sVar, false,
                RTL_TEXTENCODING_MS_1252 );
        }
    }
    m_rWW8Export.OutputField( &rFld, ww::eREF, sStr, WRITEFIELD_CLOSE );
}

void WW8AttributeOutput::WriteExpand( const SwField* pFld )
{
    OUString sExpand( lcl_GetExpandedField( *pFld ) );
    if ( m_rWW8Export.IsUnicode() )
        SwWW8Writer::WriteString16( m_rWW8Export.Strm(), sExpand, false );
    else
    {
        SwWW8Writer::WriteString8( m_rWW8Export.Strm(), sExpand, false,
            RTL_TEXTENCODING_MS_1252 );
    }
}

void AttributeOutputBase::TextField( const SwFmtFld& rField )
{
    const SwField* pFld = rField.GetField();
    bool bWriteExpand = false;
    const sal_uInt16 nSubType = pFld->GetSubType();

    switch (pFld->GetTyp()->Which())
    {
    case RES_GETEXPFLD:
        if (nSubType == nsSwGetSetExpType::GSE_STRING)
        {
            const SwGetExpField *pGet=(const SwGetExpField*)(pFld);
            RefField( *pGet, pGet->GetFormula() );
        }
        else
            bWriteExpand = true;
        break;
    case RES_SETEXPFLD:
        if (nsSwGetSetExpType::GSE_SEQ == nSubType)
        {
            OUString sStr;
            if (GetExport().FieldsQuoted())
                sStr = FieldString(ww::eSEQ) +  pFld->GetTyp()->GetName() + " ";
            else
                sStr = FieldString(ww::eSEQ) + "\"" + pFld->GetTyp()->GetName() +"\" ";
            GetNumberPara( sStr, *pFld );
            GetExport().OutputField(pFld, ww::eSEQ, sStr);
        }
        else if (nSubType & nsSwGetSetExpType::GSE_STRING)
        {
            bool bShowAsWell = false;
            ww::eField eFieldNo;
            const SwSetExpField *pSet=(const SwSetExpField*)(pFld);
            const OUString sVar = pSet->GetPar2();
            OUString sStr;
            if (pSet->GetInputFlag())
            {
                sStr = FieldString(ww::eASK) + "\""
                    + pSet->GetPar1() + "\" "
                    + pSet->GetPromptText() + " \\d "
                    + sVar;
                eFieldNo = ww::eASK;
            }
            else
            {
                sStr = FieldString(ww::eSET)
                    + pSet->GetPar1() + " \""
                    + sVar + "\" ";
                eFieldNo = ww::eSET;
                bShowAsWell = (nSubType & nsSwExtendedSubType::SUB_INVISIBLE) ? false : true;
            }

            SetField( *pFld, eFieldNo, sStr );

            if (bShowAsWell)
                RefField( *pSet, pSet->GetPar1() );
        }
        else
            bWriteExpand = true;
        break;
    case RES_PAGENUMBERFLD:
        {
            OUString sStr = FieldString(ww::ePAGE);
            GetNumberPara(sStr, *pFld);
            GetExport().OutputField(pFld, ww::ePAGE, sStr);
        }
        break;
    case RES_FILENAMEFLD:
        {
            OUString sStr = FieldString(ww::eFILENAME);
            if (pFld->GetFormat() == FF_PATHNAME)
                sStr += "\\p ";
            GetExport().OutputField(pFld, ww::eFILENAME, sStr);
        }
        break;
    case RES_DBNAMEFLD:
        {
            SwDBData aData = GetExport().pDoc->GetDBData();
            const OUString sStr = FieldString(ww::eDATABASE)
                + aData.sDataSource
                + OUString(DB_DELIM)
                + aData.sCommand;
            GetExport().OutputField(pFld, ww::eDATABASE, sStr);
        }
        break;
    case RES_AUTHORFLD:
        {
            ww::eField eFld =
                (AF_SHORTCUT & pFld->GetFormat() ? ww::eUSERINITIALS : ww::eUSERNAME);
            GetExport().OutputField(pFld, eFld, FieldString(eFld));
        }
        break;
    case RES_TEMPLNAMEFLD:
        GetExport().OutputField(pFld, ww::eTEMPLATE, FieldString(ww::eTEMPLATE));
        break;
    case RES_DOCINFOFLD:    
        if( DI_SUB_FIXED & nSubType )
            bWriteExpand = true;
        else
        {
            OUString sStr;
            ww::eField eFld(ww::eNONE);
            switch (0xff & nSubType)
            {
                case DI_TITEL:
                    eFld = ww::eTITLE;
                    break;
                case DI_THEMA:
                    eFld = ww::eSUBJECT;
                    break;
                case DI_KEYS:
                    eFld = ww::eKEYWORDS;
                    break;
                case DI_COMMENT:
                    eFld = ww::eCOMMENTS;
                    break;
                case DI_DOCNO:
                    eFld = ww::eREVNUM;
                    break;
                case DI_CREATE:
                    if (DI_SUB_AUTHOR == (nSubType & DI_SUB_MASK))
                        eFld = ww::eAUTHOR;
                    else if (GetExport().GetNumberFmt(*pFld, sStr))
                        eFld = ww::eCREATEDATE;
                    break;

                case DI_CHANGE:
                    if (DI_SUB_AUTHOR == (nSubType & DI_SUB_MASK))
                        eFld = ww::eLASTSAVEDBY;
                    else if (GetExport().GetNumberFmt(*pFld, sStr))
                        eFld = ww::eSAVEDATE;
                    break;

                case DI_PRINT:
                    if (DI_SUB_AUTHOR != (nSubType & DI_SUB_MASK) &&
                        GetExport().GetNumberFmt(*pFld, sStr))
                        eFld = ww::ePRINTDATE;
                    break;
                case DI_EDIT:
                    if( DI_SUB_AUTHOR != (nSubType & DI_SUB_MASK ) &&
                        GetExport().GetNumberFmt( *pFld, sStr ))
                        eFld = ww::eSAVEDATE;
                    break;
                case DI_CUSTOM:
                    eFld = ww::eDOCPROPERTY;
                    {
                        OUString sQuotes('\"');
                        const SwDocInfoField * pDocInfoField =
                        dynamic_cast<const SwDocInfoField *> (pFld);

                        if (pDocInfoField != NULL)
                        {
                            OUString sFieldname = pDocInfoField->GetFieldName();
                            sal_Int32 nIndex = sFieldname.indexOf(':');

                            if (nIndex != sFieldname.getLength())
                                sFieldname = sFieldname.copy(nIndex + 1);

                            sStr = sQuotes + sFieldname + sQuotes;
                        }
                    }
                    break;
                default:
                    break;
            }

            if (eFld != ww::eNONE)
            {
                GetExport().OutputField(pFld, eFld, FieldString(eFld) + sStr);
            }
            else
                bWriteExpand = true;
        }
        break;
    case RES_DATETIMEFLD:
        {
            OUString sStr;
            if (FIXEDFLD & nSubType || !GetExport().GetNumberFmt(*pFld, sStr))
                bWriteExpand = true;
            else
            {
                ww::eField eFld = (DATEFLD & nSubType) ? ww::eDATE : ww::eTIME;
                GetExport().OutputField(pFld, eFld, FieldString(eFld) + sStr);
            }
        }
        break;
    case RES_DOCSTATFLD:
        {
            ww::eField eFld = ww::eNONE;

            switch (nSubType)
            {
                case DS_PAGE:
                    eFld = ww::eNUMPAGE;
                    break;
                case DS_WORD:
                    eFld = ww::eNUMWORDS;
                    break;
                case DS_CHAR:
                    eFld = ww::eNUMCHARS;
                    break;
            }

            if (eFld != ww::eNONE)
            {
                OUString sStr = FieldString(eFld);
                GetNumberPara(sStr, *pFld);
                GetExport().OutputField(pFld, eFld, sStr);
            }
            else
                bWriteExpand = true;
        }
        break;
    case RES_EXTUSERFLD:
        {
            ww::eField eFld = ww::eNONE;
            switch (0xFF & nSubType)
            {
                case EU_FIRSTNAME:
                case EU_NAME:
                    eFld = ww::eUSERNAME;
                    break;
                case EU_SHORTCUT:
                    eFld = ww::eUSERINITIALS;
                    break;
                case EU_STREET:
                case EU_COUNTRY:
                case EU_ZIP:
                case EU_CITY:
                    eFld = ww::eUSERADDRESS;
                    break;
            }

            if (eFld != ww::eNONE)
            {
                GetExport().OutputField(pFld, eFld, FieldString(eFld));
            }
            else
                bWriteExpand = true;
        }
        break;
    case RES_POSTITFLD:
        
        if (GetExport().nTxtTyp == TXT_MAINTEXT)
        {
            PostitField( pFld );
        }
        break;
    case RES_INPUTFLD:
        {
            const SwInputField * pInputField = dynamic_cast<const SwInputField *>(pFld);

            if (pInputField->isFormField())
                GetExport().DoFormText(pInputField);
            else
            {
                const OUString sStr = FieldString(ww::eFILLIN) + "\""
                    + pFld->GetPar2() + "\"";

                GetExport().OutputField(pFld, ww::eFILLIN, sStr);
            }
        }
        break;
    case RES_GETREFFLD:
        {
            ww::eField eFld = ww::eNONE;
            OUString sStr;
            const SwGetRefField& rRFld = *(SwGetRefField*)pFld;
            switch (nSubType)
            {
                case REF_SETREFATTR:
                case REF_BOOKMARK:
                    switch (pFld->GetFormat())
                    {
                        case REF_PAGE_PGDESC:
                        case REF_PAGE:
                            eFld = ww::ePAGEREF;
                            break;
                        default:
                            eFld = ww::eREF;
                            break;
                    }
                    {
                        const OUString aRefName(rRFld.GetSetRefName());
                        sStr = FieldString(eFld)
                            + GetExport().GetBookmarkName(nSubType, &aRefName, 0);
                    }
                    switch (pFld->GetFormat())
                    {
                        case REF_NUMBER:
                            sStr += " \\r";
                            break;
                        case REF_NUMBER_NO_CONTEXT:
                            sStr += " \\n";
                            break;
                        case REF_NUMBER_FULL_CONTEXT:
                            sStr += " \\w";
                            break;
                    }
                    break;
                case REF_FOOTNOTE:
                case REF_ENDNOTE:
                    switch (pFld->GetFormat())
                    {
                        case REF_PAGE_PGDESC:
                        case REF_PAGE:
                            eFld = ww::ePAGEREF;
                            break;
                        case REF_UPDOWN:
                            eFld = ww::eREF;
                            break;
                        default:
                            eFld =
                                REF_ENDNOTE == nSubType ? ww::eNOTEREF : ww::eFOOTREF;
                            break;
                    }
                    sStr = FieldString(eFld)
                        + GetExport().GetBookmarkName(nSubType, 0, rRFld.GetSeqNo());
                    break;
            }

            if (eFld != ww::eNONE)
            {
                switch (pFld->GetFormat())
                {
                    case REF_UPDOWN:
                        sStr += " \\p \\h ";   
                        break;
                    case REF_CHAPTER:
                        sStr += " \\n \\h ";   
                        break;
                    default:
                        sStr += " \\h ";       
                        break;
                }
                GetExport().OutputField(pFld, eFld, sStr);
            }
            else
                bWriteExpand = true;
        }
        break;
    case RES_COMBINED_CHARS:
        {
        /*
        We need a font size to fill in the defaults, if these are overridden
        (as they generally are) by character properties then those properties
        win.

        The fontsize that is used in MS for determing the defaults is always
        the CJK fontsize even if the text is not in that language, in OOo the
        largest fontsize used in the field is the one we should take, but
        whatever we do, word will actually render using the fontsize set for
        CJK text. Nevertheless we attempt to guess whether the script is in
        asian or western text based up on the first character and use the
        font size of that script as our default.
        */
        sal_uInt16 nScript;
        if( g_pBreakIt->GetBreakIter().is() )
            nScript = g_pBreakIt->GetBreakIter()->getScriptType( pFld->GetPar1(), 0);
        else
            nScript = i18n::ScriptType::ASIAN;

        long nHeight = ((SvxFontHeightItem&)(GetExport().GetItem(
            GetWhichOfScript(RES_CHRATR_FONTSIZE,nScript)))).GetHeight();

        nHeight = (nHeight + 10) / 20; 

        /*
        Divide the combined char string into its up and down part. Get the
        font size and fill in the defaults as up == half the font size and
        down == a fifth the font size
        */
        const sal_Int32 nAbove = (pFld->GetPar1().getLength()+1)/2;
        const OUString sStr = FieldString(ww::eEQ)
            + "\\o (\\s\\up "
            + OUString::number(nHeight/2)
            + "("
            + pFld->GetPar1().copy(0, nAbove)
            + "), \\s\\do "
            + OUString::number(nHeight/5)
            + "("
            + pFld->GetPar1().copy(nAbove)
            + "))";
        GetExport().OutputField(pFld, ww::eEQ, sStr);
        }
        break;
    case RES_DROPDOWN:
        bWriteExpand = DropdownField( pFld );
        break;
    case RES_CHAPTERFLD:
        bWriteExpand = true;
        if (GetExport().bOutKF && rField.GetTxtFld())
        {
            const SwTxtNode *pTxtNd = GetExport().GetHdFtPageRoot();
            if (!pTxtNd)
            {
                if (const SwNode *pNd = GetExport().pCurPam->GetNode())
                    pTxtNd = pNd->GetTxtNode();
            }

            if (pTxtNd)
            {
                SwChapterField aCopy(*(const SwChapterField*)pFld);
                aCopy.ChangeExpansion(*pTxtNd, false);
                WriteExpand( &aCopy );
                bWriteExpand = false;
            }
        }
        break;
    case RES_HIDDENTXTFLD:
        {
            OUString sExpand(pFld->GetPar2());
            if (!sExpand.isEmpty())
            {
                HiddenField( *pFld );
            }
        }
        break;
    case RES_JUMPEDITFLD:
        bWriteExpand = PlaceholderField( pFld );
        break;
    case RES_MACROFLD:
        {
            const OUString sStr = " MACROBUTTON"
                + pFld->GetPar1().replaceFirst("StarOffice.Standard.Modul1.", " ")
                + " "
                + lcl_GetExpandedField(*pFld);
            GetExport().OutputField( pFld, ww::eMACROBUTTON, sStr );
        }
        break;
    default:
        bWriteExpand = true;
        break;
    }

    if (bWriteExpand)
        WriteExpand( pFld );
}

void AttributeOutputBase::TextFlyContent( const SwFmtFlyCnt& rFlyContent )
{
    if ( GetExport().pOutFmtNode && GetExport().pOutFmtNode->ISA( SwCntntNode ) )
    {
        SwTxtNode* pTxtNd = (SwTxtNode*)GetExport().pOutFmtNode;

        Point aLayPos;
        aLayPos = pTxtNd->FindLayoutRect( false, &aLayPos ).Pos();

        SwPosition aPos( *pTxtNd );
        sw::Frame aFrm( *rFlyContent.GetFrmFmt(), aPos );

        OutputFlyFrame_Impl( aFrm, aLayPos );
    }
}









void WW8AttributeOutput::ParaHyphenZone( const SvxHyphenZoneItem& rHyphenZone )
{
    
    if( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFNoAutoHyph );
    else
        m_rWW8Export.pO->push_back( 44 );

    m_rWW8Export.pO->push_back( rHyphenZone.IsHyphen() ? 0 : 1 );
}

void WW8AttributeOutput::ParaScriptSpace( const SfxBoolItem& rScriptSpace )
{
    if ( !m_rWW8Export.bWrtWW8 )
        return;

    m_rWW8Export.InsUInt16( NS_sprm::LN_PFAutoSpaceDE );
    m_rWW8Export.pO->push_back( rScriptSpace.GetValue() ? 1 : 0 );
}

void WW8AttributeOutput::ParaHangingPunctuation( const SfxBoolItem& rItem )
{
    if ( !m_rWW8Export.bWrtWW8 )
        return;

    m_rWW8Export.InsUInt16( NS_sprm::LN_PFOverflowPunct );
    m_rWW8Export.pO->push_back( rItem.GetValue() ? 1 : 0 );
}

void WW8AttributeOutput::ParaForbiddenRules( const SfxBoolItem& rItem )
{
    if ( !m_rWW8Export.bWrtWW8 )
        return;

    m_rWW8Export.InsUInt16( NS_sprm::LN_PFKinsoku );
    m_rWW8Export.pO->push_back( rItem.GetValue() ? 1 : 0 );
}

void WW8AttributeOutput::ParaSnapToGrid( const SvxParaGridItem& rGrid )
{
    
    
    if ( !m_rWW8Export.bWrtWW8 )
        return;

    m_rWW8Export.InsUInt16( NS_sprm::LN_PFUsePgsuSettings );
    m_rWW8Export.pO->push_back( rGrid.GetValue() );
}

void WW8AttributeOutput::ParaVerticalAlign( const SvxParaVertAlignItem& rAlign )
{
    
    
    if( !m_rWW8Export.bWrtWW8 )
        return;

    m_rWW8Export.InsUInt16( NS_sprm::LN_PWAlignFont );

    sal_Int16 nVal = rAlign.GetValue();
    switch ( nVal )
    {
        case SvxParaVertAlignItem::BASELINE:
            nVal = 2;
            break;
        case SvxParaVertAlignItem::TOP:
            nVal = 0;
            break;
        case SvxParaVertAlignItem::CENTER:
            nVal = 1;
            break;
        case SvxParaVertAlignItem::BOTTOM:
            nVal = 3;
            break;
        case SvxParaVertAlignItem::AUTOMATIC:
            nVal = 4;
            break;
        default:
            nVal = 4;
            OSL_FAIL( "Unknown vert alignment" );
            break;
    }
    m_rWW8Export.InsUInt16( nVal );
}





void WW8Export::WriteFtnBegin( const SwFmtFtn& rFtn, ww::bytes* pOutArr )
{
    ww::bytes aAttrArr;
    const bool bAutoNum = rFtn.GetNumStr().isEmpty();    
    if( bAutoNum )
    {
        if( bWrtWW8 )
        {
            static const sal_uInt8 aSpec[] =
            {
                0x03, 0x6a, 0, 0, 0, 0, 
                0x55, 0x08, 1           
            };

            aAttrArr.insert(aAttrArr.end(), aSpec, aSpec+sizeof(aSpec));
        }
        else
        {
            static sal_uInt8 const aSpec[] =
            {
                117, 1,                         
                68, 4, 0, 0, 0, 0               
            };

            aAttrArr.insert(aAttrArr.end(), aSpec, aSpec+sizeof(aSpec));
        }
    }

    
    const SwEndNoteInfo* pInfo;
    if( rFtn.IsEndNote() )
        pInfo = &pDoc->GetEndNoteInfo();
    else
        pInfo = &pDoc->GetFtnInfo();
    const SwCharFmt* pCFmt = pOutArr
                                ? pInfo->GetAnchorCharFmt( *pDoc )
                                : pInfo->GetCharFmt( *pDoc );
    if( bWrtWW8 )
        SwWW8Writer::InsUInt16( aAttrArr, NS_sprm::LN_CIstd );
    else
        aAttrArr.push_back( 80 );
    SwWW8Writer::InsUInt16( aAttrArr, GetId( *pCFmt ) );

                                                
                            
                            
    pChpPlc->AppendFkpEntry( Strm().Tell() );
    if( bAutoNum )
        WriteChar( 0x02 );              
    else
        
        OutSwString( rFtn.GetNumStr(), 0, rFtn.GetNumStr().getLength(),
                        IsUnicode(), RTL_TEXTENCODING_MS_1252 );

    if( pOutArr )
    {
        
        
        pOutArr->insert( pOutArr->begin(), aAttrArr.begin(), aAttrArr.end() );
    }
    else
    {
        ww::bytes aOutArr;

        
        
        aOutArr.insert( aOutArr.begin(), aAttrArr.begin(), aAttrArr.end() );

        
        const SwTxtFtn* pTxtFtn = rFtn.GetTxtFtn();
        if( pTxtFtn )
        {
            ww::bytes* pOld = pO;
            pO = &aOutArr;
            SfxItemSet aSet( pDoc->GetAttrPool(), RES_CHRATR_FONT,
                                                  RES_CHRATR_FONT );

            pCFmt = pInfo->GetCharFmt( *pDoc );
            aSet.Set( pCFmt->GetAttrSet() );

            pTxtFtn->GetTxtNode().GetAttr( aSet, *pTxtFtn->GetStart(),
                                            (*pTxtFtn->GetStart()) + 1 );
            m_pAttrOutput->OutputItem( aSet.Get( RES_CHRATR_FONT ) );
            pO = pOld;
        }
        pChpPlc->AppendFkpEntry( Strm().Tell(), aOutArr.size(),
                                                aOutArr.data() );
    }
}

static bool lcl_IsAtTxtEnd(const SwFmtFtn& rFtn)
{
    bool bRet = true;
    if( rFtn.GetTxtFtn() )
    {
        sal_uInt16 nWh = static_cast< sal_uInt16 >(rFtn.IsEndNote() ? RES_END_AT_TXTEND
                                      : RES_FTN_AT_TXTEND);
        const SwSectionNode* pSectNd = rFtn.GetTxtFtn()->GetTxtNode().
                                                        FindSectionNode();
        while( pSectNd && FTNEND_ATPGORDOCEND ==
                ((const SwFmtFtnAtTxtEnd&)pSectNd->GetSection().GetFmt()->
                GetFmtAttr( nWh, true)).GetValue() )
            pSectNd = pSectNd->StartOfSectionNode()->FindSectionNode();

        if (!pSectNd)
            bRet = false;   
    }
    return bRet;
}

void AttributeOutputBase::TextFootnote( const SwFmtFtn& rFtn )
{
    sal_uInt16 nTyp;
    if ( rFtn.IsEndNote() )
    {
        nTyp = REF_ENDNOTE;
        if ( GetExport().bEndAtTxtEnd )
            GetExport().bEndAtTxtEnd = lcl_IsAtTxtEnd( rFtn );
    }
    else
    {
        nTyp = REF_FOOTNOTE;
        if ( GetExport().bFtnAtTxtEnd )
            GetExport().bFtnAtTxtEnd = lcl_IsAtTxtEnd( rFtn );
    }

    
    
    OUString sBkmkNm;
    if ( GetExport().HasRefToObject( nTyp, 0, rFtn.GetTxtFtn()->GetSeqRefNo() ))
    {
        sBkmkNm = GetExport().GetBookmarkName( nTyp, 0,
                                    rFtn.GetTxtFtn()->GetSeqRefNo() );
        GetExport().AppendBookmark( sBkmkNm );
    }

    TextFootnote_Impl( rFtn );

    if ( !sBkmkNm.isEmpty() )
        GetExport().AppendBookmark( sBkmkNm ); 
}

void WW8AttributeOutput::TextFootnote_Impl( const SwFmtFtn& rFtn )
{
    WW8_WrPlcFtnEdn* pFtnEnd;
    if ( rFtn.IsEndNote() )
        pFtnEnd = m_rWW8Export.pEdn;
    else
        pFtnEnd = m_rWW8Export.pFtn;

    pFtnEnd->Append( m_rWW8Export.Fc2Cp( m_rWW8Export.Strm().Tell() ), rFtn );
    m_rWW8Export.WriteFtnBegin( rFtn, m_rWW8Export.pO );
}

void WW8AttributeOutput::TextCharFormat( const SwFmtCharFmt& rCharFmt )
{
    if( rCharFmt.GetCharFmt() )
    {
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_CIstd );
        else
            m_rWW8Export.pO->push_back( 80 );

        m_rWW8Export.InsUInt16( m_rWW8Export.GetId( *rCharFmt.GetCharFmt() ) );
    }
}

/*
 See ww8par6.cxx Read_DoubleLine for some more info
 */
void WW8AttributeOutput::CharTwoLines( const SvxTwoLinesItem& rTwoLines )
{
    
    if ( rTwoLines.GetValue() )
    {
        
        if( !m_rWW8Export.bWrtWW8 )
            return;

        m_rWW8Export.InsUInt16( NS_sprm::LN_CEastAsianLayout );
        m_rWW8Export.pO->push_back( (sal_uInt8)0x06 ); 
        m_rWW8Export.pO->push_back( (sal_uInt8)0x02 );

        sal_Unicode cStart = rTwoLines.GetStartBracket();
        sal_Unicode cEnd = rTwoLines.GetEndBracket();

        /*
        As per usual we have problems. We can have separate left and right brackets
        in OOo, it doesn't appear that you can in word. Also in word there appear
        to only be a limited number of possibilities, we can use pretty much
        anything.

        So if we have none, we export none, if either bracket is set to a known
        word type we export both as that type (with the bracket winning out in
        the case of a conflict simply being the order of test here.

        Upshot being a documented created in word will be reexported with no
        ill effects.
        */

        sal_uInt16 nType;
        if (!cStart && !cEnd)
            nType = 0;
        else if ((cStart == '{') || (cEnd == '}'))
            nType = 4;
        else if ((cStart == '<') || (cEnd == '>'))
            nType = 3;
        else if ((cStart == '[') || (cEnd == ']'))
            nType = 2;
        else
            nType = 1;
        m_rWW8Export.InsUInt16( nType );
        static const sal_uInt8 aZeroArr[ 3 ] = { 0, 0, 0 };
        m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), aZeroArr, aZeroArr+3);
    }
}

void AttributeOutputBase::ParaNumRule( const SwNumRuleItem& rNumRule )
{
    const SwTxtNode* pTxtNd = 0;
    sal_uInt16 nNumId;
    sal_uInt8 nLvl = 0;
    if (!rNumRule.GetValue().isEmpty())
    {
        const SwNumRule* pRule = GetExport().pDoc->FindNumRulePtr(
                                        rNumRule.GetValue() );
        if ( pRule && USHRT_MAX != ( nNumId = GetExport().GetId( *pRule ) ) )
        {
            ++nNumId;
            if ( GetExport().pOutFmtNode )
            {
                if ( GetExport().pOutFmtNode->ISA( SwCntntNode ) )
                {
                    pTxtNd = (SwTxtNode*)GetExport().pOutFmtNode;

                    if( pTxtNd->IsCountedInList())
                    {
                        int nLevel = pTxtNd->GetActualListLevel();

                        if (nLevel < 0)
                            nLevel = 0;

                        if (nLevel >= MAXLEVEL)
                            nLevel = MAXLEVEL - 1;

                        nLvl = static_cast< sal_uInt8 >(nLevel);

                        if ( pTxtNd->IsListRestart() )
                        {
                            sal_uInt16 nStartWith = static_cast< sal_uInt16 >( pTxtNd->GetActualListStartValue() );
                            nNumId = GetExport().DuplicateNumRule( pRule, nLvl, nStartWith );
                            if ( USHRT_MAX != nNumId )
                                ++nNumId;
                        }
                    }
                    else
                    {
                        
                        
                        
                        
                        nNumId = 0;
                    }
                }
                else if ( GetExport().pOutFmtNode->ISA( SwTxtFmtColl ) )
                {
                    const SwTxtFmtColl* pC = (SwTxtFmtColl*)GetExport().pOutFmtNode;
                    if ( pC && pC->IsAssignedToListLevelOfOutlineStyle() )
                        nLvl = static_cast< sal_uInt8 >( pC->GetAssignedOutlineStyleLevel() );
                }
            }
        }
        else
            nNumId = USHRT_MAX;
    }
    else
        nNumId = 0;

    if ( USHRT_MAX != nNumId )
    {
        if ( nLvl >= WW8ListManager::nMaxLevel )
            nLvl = WW8ListManager::nMaxLevel - 1;

        ParaNumRule_Impl( pTxtNd, nLvl, nNumId );
    }
}

void WW8AttributeOutput::ParaNumRule_Impl( const SwTxtNode* pTxtNd, sal_Int32 nLvl, sal_Int32 nNumId )
{
    if ( m_rWW8Export.bWrtWW8 )
    {
        
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_PIlvl );
        m_rWW8Export.pO->push_back( ::sal::static_int_cast<sal_uInt8>(nLvl) );
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_PIlfo );
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, ::sal::static_int_cast<sal_uInt16>(nNumId) );
    }
    else if ( pTxtNd && m_rWW8Export.Out_SwNum( pTxtNd ) )                
        m_rWW8Export.pSepx->SetNum( pTxtNd );
}

/* File FRMATR.HXX */

void WW8AttributeOutput::FormatFrameSize( const SwFmtFrmSize& rSize )
{
    if( m_rWW8Export.bOutFlyFrmAttrs )                   
    {
        if( m_rWW8Export.bOutGrf )
            return;                


        if ( rSize.GetWidth() && rSize.GetWidthSizeType() == ATT_FIX_SIZE)
        {
            
            if( m_rWW8Export.bWrtWW8 )
                m_rWW8Export.InsUInt16( NS_sprm::LN_PDxaWidth );
            else
                m_rWW8Export.pO->push_back( 28 );
            m_rWW8Export.InsUInt16( (sal_uInt16)rSize.GetWidth() );
        }

        if ( rSize.GetHeight() )
        {
            
            if( m_rWW8Export.bWrtWW8 )
                m_rWW8Export.InsUInt16( NS_sprm::LN_PWHeightAbs );
            else
                m_rWW8Export.pO->push_back( 45 );

            sal_uInt16 nH = 0;
            switch ( rSize.GetHeightSizeType() )
            {
                case ATT_VAR_SIZE: break;
                case ATT_FIX_SIZE: nH = (sal_uInt16)rSize.GetHeight() & 0x7fff; break;
                default:           nH = (sal_uInt16)rSize.GetHeight() | 0x8000; break;
            }
            m_rWW8Export.InsUInt16( nH );
        }
    }
    else if( m_rWW8Export.bOutPageDescs )            
    {
        if( m_rWW8Export.pAktPageDesc->GetLandscape() )
        {
            /*sprmSBOrientation*/
            if( m_rWW8Export.bWrtWW8 )
                m_rWW8Export.InsUInt16( NS_sprm::LN_SBOrientation );
            else
                m_rWW8Export.pO->push_back( 162 );
            m_rWW8Export.pO->push_back( 2 );
        }

        /*sprmSXaPage*/
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_SXaPage );
        else
            m_rWW8Export.pO->push_back( 164 );
        m_rWW8Export.InsUInt16(
            msword_cast<sal_uInt16>(SvxPaperInfo::GetSloppyPaperDimension(rSize.GetWidth())));

        /*sprmSYaPage*/
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_SYaPage );
        else
            m_rWW8Export.pO->push_back( 165 );
        m_rWW8Export.InsUInt16(
            msword_cast<sal_uInt16>(SvxPaperInfo::GetSloppyPaperDimension(rSize.GetHeight())));
    }
}








sal_uLong WW8Export::ReplaceCr( sal_uInt8 nChar )
{
    OSL_ENSURE( nChar, "gegen 0 ersetzt bringt WW97/95 zum Absturz" );

    bool bReplaced = false;
    SvStream& rStrm = Strm();
    sal_uLong nRetPos = 0, nPos = rStrm.Tell();
    
    if (nPos - (IsUnicode() ? 2 : 1) >= sal_uLong(pFib->fcMin))
    {
        sal_uInt8 nBCode=0;
        sal_uInt16 nUCode=0;

        rStrm.SeekRel(IsUnicode() ? -2 : -1);
        if (IsUnicode())
            rStrm.ReadUInt16( nUCode );
        else
        {
            rStrm.ReadUChar( nBCode );
            nUCode = nBCode;
        }
        
        if (nUCode == 0x0d)             
        {
            if ((nChar == 0x0c) &&
                (nPos - (IsUnicode() ? 4 : 2) >= sal_uLong(pFib->fcMin)))
            {
                rStrm.SeekRel( IsUnicode() ? -4 : -2 );
                if (IsUnicode())
                    rStrm.ReadUInt16( nUCode );
                else
                {
                    rStrm.ReadUInt16( nUCode );
                    nUCode = nBCode;
                }
            }
            else
            {
                rStrm.SeekRel( IsUnicode() ? -2 : -1 );
                nUCode = 0x0;
            }
            
            
            if( nUCode == 0x0d )
                bReplaced = false;
            else
            {
                bReplaced = true;
                WriteChar(nChar);
                nRetPos = nPos;
            }
        }
        else if ((nUCode == 0x0c) && (nChar == 0x0e))
        {
            
            bReplaced = true;
        }
        rStrm.Seek( nPos );
    }
    else
        bReplaced = true;

    if (!bReplaced)
    {
        
        WriteChar(nChar);
        pPiece->SetParaBreak();
        pPapPlc->AppendFkpEntry(rStrm.Tell());
        pChpPlc->AppendFkpEntry(rStrm.Tell());
        nRetPos = rStrm.Tell();
    }
    return nRetPos;
}

void WW8AttributeOutput::TableRowEnd(sal_uInt32 nDepth)
{
    if ( nDepth == 1 )
        m_rWW8Export.WriteChar( (sal_uInt8)0x07 );
    else if ( nDepth > 1 )
        m_rWW8Export.WriteChar( (sal_uInt8)0x0d );

    
    
    
    
}

void AttributeOutputBase::FormatPageDescription( const SwFmtPageDesc& rPageDesc )
{
    if ( GetExport().bStyDef && GetExport().pOutFmtNode && GetExport().pOutFmtNode->ISA( SwTxtFmtColl ) )
    {
        const SwTxtFmtColl* pC = (SwTxtFmtColl*)GetExport().pOutFmtNode;
        if ( (SFX_ITEM_SET != pC->GetItemState( RES_BREAK, false ) ) && rPageDesc.KnowsPageDesc() )
            FormatBreak( SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE, RES_BREAK ) );
    }
}

void WW8AttributeOutput::PageBreakBefore( bool bBreak )
{
    
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFPageBreakBefore );
    else
        m_rWW8Export.pO->push_back( 9 );

    m_rWW8Export.pO->push_back( bBreak ? 1 : 0 );
}




void AttributeOutputBase::FormatBreak( const SvxFmtBreakItem& rBreak )
{
    if ( GetExport().bStyDef )
    {
        switch ( rBreak.GetBreak() )
        {
            case SVX_BREAK_NONE:
            case SVX_BREAK_PAGE_BEFORE:
            case SVX_BREAK_PAGE_BOTH:
                PageBreakBefore( rBreak.GetValue() );
                break;
            default:
                break;
        }
    }
    else if ( !GetExport().mpParentFrame )
    {
        sal_uInt8 nC = 0;
        bool bBefore = false;
        
        bool bCheckForFollowPageDesc = false;

        switch ( rBreak.GetBreak() )
        {
            case SVX_BREAK_NONE:                                
                if ( !GetExport().bBreakBefore )
                    PageBreakBefore( false );
                return;

            case SVX_BREAK_COLUMN_BEFORE:                       
                bBefore = true;
                
            case SVX_BREAK_COLUMN_AFTER:
            case SVX_BREAK_COLUMN_BOTH:
                if ( GetExport().Sections().CurrentNumberOfColumns( *GetExport().pDoc ) > 1 || GetExport().SupportsOneColumnBreak() )
                {
                    nC = msword::ColumnBreak;
                }
                break;

            case SVX_BREAK_PAGE_BEFORE:                         
                
                
                
                if ( !GetExport().bBreakBefore )
                    PageBreakBefore( true );
                break;

            case SVX_BREAK_PAGE_AFTER:
            case SVX_BREAK_PAGE_BOTH:
                nC = msword::PageBreak;
                
                
                if ( dynamic_cast< const SwTxtNode* >( GetExport().pOutFmtNode ) &&
                     GetExport().GetCurItemSet() )
                {
                    bCheckForFollowPageDesc = true;
                }
                break;

            default:
                break;
        }

        if ( ( bBefore == GetExport().bBreakBefore ) && nC )
        {
            
            bool bFollowPageDescWritten = false;
            if ( bCheckForFollowPageDesc && !bBefore )
            {
                bFollowPageDescWritten =
                    GetExport().OutputFollowPageDesc( GetExport().GetCurItemSet(),
                            dynamic_cast<const SwTxtNode*>( GetExport().pOutFmtNode ) );
            }
            if ( !bFollowPageDescWritten )
            {
                SectionBreak( nC );
            }
        }
    }
}

void WW8AttributeOutput::SectionBreak( sal_uInt8 nC, const WW8_SepInfo* /*pSectionInfo*/ )
{
    m_rWW8Export.ReplaceCr( nC );
}

sal_uInt32 AttributeOutputBase::GridCharacterPitch( const SwTextGridItem& rGrid ) const
{
    MSWordStyles * pStyles = GetExport().pStyles;
    const SwFmt * pSwFmt = pStyles->GetSwFmt(0);

    sal_uInt32 nPageCharSize = 0;

    if (pSwFmt != NULL)
    {
        nPageCharSize = ItemGet<SvxFontHeightItem>
            (*pSwFmt, RES_CHRATR_FONTSIZE).GetHeight();
    }
    sal_uInt16 nPitch = rGrid.IsSquaredMode() ? rGrid.GetBaseHeight() :
        rGrid.GetBaseWidth( );

    sal_Int32 nCharWidth = nPitch - nPageCharSize;
    sal_Int32 nFraction = nCharWidth % 20;
    if ( nCharWidth < 0 )
        nFraction = 20 + nFraction;
    nFraction = ( nFraction * 0xFFF ) / 20;
    nFraction = ( nFraction & 0x00000FFF );

    sal_Int32 nMain = nCharWidth / 20;
    if ( nCharWidth < 0 )
        nMain -= 1;
    nMain = nMain * 0x1000;
    nMain = ( nMain & 0xFFFFF000 );

    return sal_uInt32( nFraction + nMain );
}

void WW8AttributeOutput::FormatTextGrid( const SwTextGridItem& rGrid )
{
    if ( m_rWW8Export.bOutPageDescs && m_rWW8Export.bWrtWW8 )
    {
        sal_uInt16 nGridType = 0;
        switch ( rGrid.GetGridType() )
        {
            default:
                OSL_FAIL("Unknown grid type");
            case GRID_NONE:
                nGridType = 0;
                break;
            case GRID_LINES_ONLY:
                nGridType = 2;
                break;
            case GRID_LINES_CHARS:
                if ( rGrid.IsSnapToChars() )
                    nGridType = 3;
                else
                    nGridType = 1;
                break;
        }
        m_rWW8Export.InsUInt16( NS_sprm::LN_SClm );
        m_rWW8Export.InsUInt16( nGridType );

        sal_uInt16 nHeight = rGrid.GetBaseHeight() + rGrid.GetRubyHeight();
        m_rWW8Export.InsUInt16( NS_sprm::LN_SDyaLinePitch );
        m_rWW8Export.InsUInt16( nHeight );

        m_rWW8Export.InsUInt16( NS_sprm::LN_SDxtCharSpace );
        m_rWW8Export.InsUInt32( GridCharacterPitch( rGrid ) );
    }
}

void WW8AttributeOutput::FormatPaperBin( const SvxPaperBinItem& rPaperBin )
{
    if ( m_rWW8Export.bOutPageDescs )
    {
        sal_uInt16 nVal;
        switch ( rPaperBin.GetValue() )
        {
            case 0: nVal = 15;  break;      
            case 1: nVal = 1;   break;      
            case 2: nVal = 4;   break;      
            default: nVal = 0;  break;
        }

        if ( nVal )
        {
            if( m_rWW8Export.bWrtWW8 )
                m_rWW8Export.InsUInt16( m_rWW8Export.bOutFirstPage? NS_sprm::LN_SDmBinFirst: NS_sprm::LN_SDmBinOther );
            else
                m_rWW8Export.pO->push_back( m_rWW8Export.bOutFirstPage? 140: 141 );

            m_rWW8Export.InsUInt16( nVal );
        }
    }
}

void WW8AttributeOutput::FormatLRSpace( const SvxLRSpaceItem& rLR )
{
    

    if ( m_rWW8Export.bOutFlyFrmAttrs )                   
    {
        
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PDxaFromText10 );
        else
            m_rWW8Export.pO->push_back( 49 );
        
        m_rWW8Export.InsUInt16( (sal_uInt16) ( ( rLR.GetLeft() + rLR.GetRight() ) / 2 ) );
    }
    else if ( m_rWW8Export.bOutPageDescs )                
    {
        sal_uInt16 nLDist, nRDist;
        const SfxPoolItem* pItem = m_rWW8Export.HasItem( RES_BOX );
        if ( pItem )
        {
            nRDist = ((SvxBoxItem*)pItem)->CalcLineSpace( BOX_LINE_LEFT );
            nLDist = ((SvxBoxItem*)pItem)->CalcLineSpace( BOX_LINE_RIGHT );
        }
        else
            nLDist = nRDist = 0;
        nLDist = nLDist + (sal_uInt16)rLR.GetLeft();
        nRDist = nRDist + (sal_uInt16)rLR.GetRight();

        
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_SDxaLeft );
        else
            m_rWW8Export.pO->push_back( 166 );
        m_rWW8Export.InsUInt16( nLDist );

        
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_SDxaRight );
        else
            m_rWW8Export.pO->push_back( 167 );
        m_rWW8Export.InsUInt16( nRDist );
    }
    else
    {                                          
        
        if( m_rWW8Export.bWrtWW8 )
        {
            m_rWW8Export.InsUInt16( 0x845E );        
            m_rWW8Export.InsUInt16( (sal_uInt16)rLR.GetTxtLeft() );
        }
        else
        {
            m_rWW8Export.pO->push_back( 17 );
            m_rWW8Export.InsUInt16( (sal_uInt16)rLR.GetTxtLeft() );
        }

        
        if( m_rWW8Export.bWrtWW8 )
        {
            m_rWW8Export.InsUInt16( 0x845D );        
            m_rWW8Export.InsUInt16( (sal_uInt16)rLR.GetRight() );
        }
        else
        {
            m_rWW8Export.pO->push_back( 16 );
            m_rWW8Export.InsUInt16( (sal_uInt16)rLR.GetRight() );
        }

        
        if( m_rWW8Export.bWrtWW8 )
        {
            m_rWW8Export.InsUInt16( 0x8460 );        
            m_rWW8Export.InsUInt16( rLR.GetTxtFirstLineOfst() );
        }
        else
        {
            m_rWW8Export.pO->push_back( 19 );
            m_rWW8Export.InsUInt16( rLR.GetTxtFirstLineOfst() );
        }
    }
}

void WW8AttributeOutput::FormatULSpace( const SvxULSpaceItem& rUL )
{
    

    if ( m_rWW8Export.bOutFlyFrmAttrs )                   
    {
        
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaFromText );
        else
            m_rWW8Export.pO->push_back( 48 );
        
        m_rWW8Export.InsUInt16( (sal_uInt16) ( ( rUL.GetUpper() + rUL.GetLower() ) / 2 ) );
    }
    else if ( m_rWW8Export.bOutPageDescs )            
    {
        OSL_ENSURE( m_rWW8Export.GetCurItemSet(), "Impossible" );
        if ( !m_rWW8Export.GetCurItemSet() )
            return;

        HdFtDistanceGlue aDistances( *m_rWW8Export.GetCurItemSet() );

        if ( aDistances.HasHeader() )
        {
            
            if ( m_rWW8Export.bWrtWW8 )
                m_rWW8Export.InsUInt16( NS_sprm::LN_SDyaHdrTop );
            else
                m_rWW8Export.pO->push_back( 156 );
            m_rWW8Export.InsUInt16( aDistances.dyaHdrTop );
        }

        
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_SDyaTop );
        else
            m_rWW8Export.pO->push_back( 168 );
        m_rWW8Export.InsUInt16( aDistances.dyaTop );

        if ( aDistances.HasFooter() )
        {
            
            if ( m_rWW8Export.bWrtWW8 )
                m_rWW8Export.InsUInt16( NS_sprm::LN_SDyaHdrBottom );
            else
                m_rWW8Export.pO->push_back( 157 );
            m_rWW8Export.InsUInt16( aDistances.dyaHdrBottom );
        }

        
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_SDyaBottom );
        else
            m_rWW8Export.pO->push_back( 169 );
        m_rWW8Export.InsUInt16( aDistances.dyaBottom );
    }
    else
    {
        
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaBefore );
        else
            m_rWW8Export.pO->push_back( 21 );
        m_rWW8Export.InsUInt16( rUL.GetUpper() );
        
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaAfter );
        else
            m_rWW8Export.pO->push_back( 22 );
        m_rWW8Export.InsUInt16( rUL.GetLower() );
        
        if (m_rWW8Export.bWrtWW8 && rUL.GetContext())
        {
            m_rWW8Export.InsUInt16(NS_sprm::LN_PContextualSpacing);
            m_rWW8Export.pO->push_back( (sal_uInt8)rUL.GetContext() );
        }
    }
}



void WW8AttributeOutput::FormatSurround( const SwFmtSurround& rSurround )
{
    if ( m_rWW8Export.bOutFlyFrmAttrs )
    {
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PWr );
        else
            m_rWW8Export.pO->push_back( 37 );

        m_rWW8Export.pO->push_back(
                ( SURROUND_NONE != rSurround.GetSurround() ) ? 2 : 1 );
    }
}

void WW8AttributeOutput::FormatVertOrientation( const SwFmtVertOrient& rFlyVert )
{


    if ( m_rWW8Export.bOutFlyFrmAttrs )
    {
        short nPos;
        switch( rFlyVert.GetVertOrient() )
        {
            case text::VertOrientation::NONE:
                nPos = (short)rFlyVert.GetPos();
                break;
            case text::VertOrientation::CENTER:
            case text::VertOrientation::LINE_CENTER:
                nPos = -8;
                break;
            case text::VertOrientation::BOTTOM:
            case text::VertOrientation::LINE_BOTTOM:
                nPos = -12;
                break;
            case text::VertOrientation::TOP:
            case text::VertOrientation::LINE_TOP:
            default:
                nPos = -4;
                break;
        }

        
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaAbs );
        else
            m_rWW8Export.pO->push_back( 27 );
        m_rWW8Export.InsUInt16( nPos );
    }
}

void WW8AttributeOutput::FormatHorizOrientation( const SwFmtHoriOrient& rFlyHori )
{
    if ( !m_rWW8Export.mpParentFrame )
    {
        OSL_ENSURE( m_rWW8Export.mpParentFrame, "HoriOrient without mpParentFrame !!" );
        return;
    }


    if ( m_rWW8Export.bOutFlyFrmAttrs )
    {
        short nPos;
        switch( rFlyHori.GetHoriOrient() )
        {
            case text::HoriOrientation::NONE:
                nPos = (short)rFlyHori.GetPos();
                if( !nPos )
                    nPos = 1;   
                break;
            case text::HoriOrientation::LEFT:
                nPos = rFlyHori.IsPosToggle() ? -12 : 0;
                break;
            case text::HoriOrientation::RIGHT:
                nPos = rFlyHori.IsPosToggle() ? -16 : -8;
                break;
            case text::HoriOrientation::CENTER:
            case text::HoriOrientation::FULL:                         
            default:
                nPos = -4;
                break;
        }

        
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PDxaAbs );
        else
            m_rWW8Export.pO->push_back( 26 );
        m_rWW8Export.InsUInt16( nPos );
    }
}

void WW8AttributeOutput::FormatAnchor( const SwFmtAnchor& rAnchor )
{
    OSL_ENSURE( m_rWW8Export.mpParentFrame, "Anchor without mpParentFrame !!" );

    if ( m_rWW8Export.bOutFlyFrmAttrs )
    {
        sal_uInt8 nP = 0;
        switch ( rAnchor.GetAnchorId() )
        {
            case FLY_AT_PAGE:
                
                nP |= (1 << 4) | (2 << 6);
                break;
            
            case FLY_AT_FLY:
            case FLY_AT_CHAR:
            case FLY_AT_PARA:
            case FLY_AS_CHAR:
                
                nP |= (2 << 4) | (0 << 6);
                break;
            default:
                break;
        }

        
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PPc );
        else
            m_rWW8Export.pO->push_back( 29 );
        m_rWW8Export.pO->push_back( nP );
    }
}

void WW8AttributeOutput::FormatBackground( const SvxBrushItem& rBrush )
{
    
    if ( !m_rWW8Export.bOutPageDescs )
    {
        WW8_SHD aSHD;

        m_rWW8Export.TransBrush( rBrush.GetColor(), aSHD );
        
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PShd );
        else
            m_rWW8Export.pO->push_back(47);
        m_rWW8Export.InsUInt16( aSHD.GetValue() );

        
        
        if ( m_rWW8Export.bWrtWW8 )
        {
            m_rWW8Export.InsUInt16( 0xC64D );
            m_rWW8Export.pO->push_back( 10 );
            m_rWW8Export.InsUInt32( 0xFF000000 );
            m_rWW8Export.InsUInt32( SuitableBGColor( rBrush.GetColor().GetColor() ) );
            m_rWW8Export.InsUInt16( 0x0000 );
        }
    }
}

void WW8AttributeOutput::FormatFillStyle( const XFillStyleItem& /*rFillStyle*/ )
{
}

void WW8AttributeOutput::FormatFillGradient( const XFillGradientItem& /*rFillGradient*/ )
{
}

WW8_BRC WW8Export::TranslateBorderLine(const SvxBorderLine& rLine,
    sal_uInt16 nDist, bool bShadow)
{
    
    
    
    WW8_BRC aBrc;
    sal_uInt16 nWidth = ::editeng::ConvertBorderWidthToWord(
            rLine.GetBorderLineStyle(), rLine.GetWidth());
    sal_uInt8 brcType = 0, nColCode = 0;

    if( nWidth )                                
    {
        
        bool bThick = !rLine.isDouble() && !bWrtWW8 && nWidth > 75;
        if( bThick )
            brcType = 2;
        else
        {
            brcType = 0;
            if ( bWrtWW8 )
            {
                
                
                switch (rLine.GetBorderLineStyle())
                {
                    case table::BorderLineStyle::SOLID:
                        {
                            if ( rLine.GetWidth( ) == DEF_LINE_WIDTH_0 )
                                brcType = 5;
                            else
                                brcType = 1;
                        }
                        break;
                    case table::BorderLineStyle::DOTTED:
                        brcType = 6;
                        break;
                    case table::BorderLineStyle::DASHED:
                        brcType = 7;
                        break;
                    case table::BorderLineStyle::DOUBLE:
                        brcType = 3;
                        break;
                    case table::BorderLineStyle::THINTHICK_SMALLGAP:
                        brcType = 11;
                        break;
                    case table::BorderLineStyle::THINTHICK_MEDIUMGAP:
                        brcType = 14;
                        break;
                    case table::BorderLineStyle::THINTHICK_LARGEGAP:
                        brcType = 17;
                        break;
                    case table::BorderLineStyle::THICKTHIN_SMALLGAP:
                        brcType = 12;
                        break;
                    case table::BorderLineStyle::THICKTHIN_MEDIUMGAP:
                        brcType = 15;
                        break;
                    case table::BorderLineStyle::THICKTHIN_LARGEGAP:
                        brcType = 18;
                        break;
                    case table::BorderLineStyle::EMBOSSED:
                        brcType = 24;
                        break;
                    case table::BorderLineStyle::ENGRAVED:
                        brcType = 25;
                        break;
                    case table::BorderLineStyle::OUTSET:
                        brcType = 26;
                        break;
                    case table::BorderLineStyle::INSET:
                        brcType = 27;
                        break;
                    default:
                        break;
                }
            }
        }

        
        if( bThick )
            nWidth /= 2;

        if( bWrtWW8 )
        {
            
            nWidth = (( nWidth * 8 ) + 10 ) / 20;
            if( 0xff < nWidth )
                nWidth = 0xff;
        }
        else
        {
            
            nWidth = ( nWidth + 7 ) / 15;
            if( nWidth > 5 )
                nWidth = 5;
            ::editeng::SvxBorderStyle const eStyle(rLine.GetBorderLineStyle());
            if (table::BorderLineStyle::DOTTED == eStyle)
                nWidth = 6;
            else if (table::BorderLineStyle::DASHED == eStyle)
                nWidth = 7;
        }

        if( 0 == nWidth )                       
            nWidth = 1;                         

        
        nColCode = TransCol( rLine.GetColor() );
    }

    
    sal_uInt16 nLDist = nDist;
    nLDist /= 20;               
    if( nLDist > 0x1f )
        nLDist = 0x1f;

    if( bWrtWW8 )
    {
        aBrc.aBits1[0] = sal_uInt8(nWidth);
        aBrc.aBits1[1] = brcType;
        aBrc.aBits2[0] = nColCode;
        aBrc.aBits2[1] = sal_uInt8(nLDist);

        
        if( bShadow )
            aBrc.aBits2[1] |= 0x20;
    }
    else
    {
        sal_uInt16 aBits = nWidth + ( brcType << 3 );
        aBits |= (nColCode & 0x1f) << 6;
        aBits |= nLDist << 11;
        
        if( bShadow )
            aBits |= 0x20;
        ShortToSVBT16( aBits, aBrc.aBits1);
    }

    return aBrc;
}





void WW8Export::Out_BorderLine(ww::bytes& rO, const SvxBorderLine* pLine,
    sal_uInt16 nDist, sal_uInt16 nSprmNo, bool bShadow)
{
    OSL_ENSURE( ( nSprmNo == 0 ) ||
            ( nSprmNo >= 38 && nSprmNo <= 41 ) ||
            ( nSprmNo >= NS_sprm::LN_PBrcTop && nSprmNo <= NS_sprm::LN_PBrcRight ) ||
            ( nSprmNo >= NS_sprm::LN_SBrcTop && nSprmNo <= NS_sprm::LN_SBrcRight ),
            "Sprm for border out is of range" );

    WW8_BRC aBrc;

    if (pLine)
        aBrc = TranslateBorderLine( *pLine, nDist, bShadow );

    if( bWrtWW8 )
    {
        
        if ( nSprmNo != 0 )
            SwWW8Writer::InsUInt16( rO, nSprmNo );

        rO.insert( rO.end(), aBrc.aBits1, aBrc.aBits1+2 );
        rO.insert( rO.end(), aBrc.aBits2, aBrc.aBits2+2 );
    }
    else
    {
        
        if ( nSprmNo != 0 )
            rO.push_back( static_cast<sal_uInt8>( nSprmNo ) );
        rO.insert( rO.end(), aBrc.aBits1, aBrc.aBits1+2 );
    }
}



void WW8Export::Out_SwFmtBox(const SvxBoxItem& rBox, bool bShadow)
{
    if ( bOutPageDescs && !bWrtWW8 )
        return; 

    static const sal_uInt16 aBorders[] =
    {
        BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
    };
    static const sal_uInt16 aPBrc[] =
    {
        NS_sprm::LN_PBrcTop, NS_sprm::LN_PBrcLeft, NS_sprm::LN_PBrcBottom, NS_sprm::LN_PBrcRight
    };
    static const sal_uInt16 aSBrc[] =
    {
        NS_sprm::LN_SBrcTop, NS_sprm::LN_SBrcLeft, NS_sprm::LN_SBrcBottom, NS_sprm::LN_SBrcRight
    };
    static const sal_uInt16 aWW6PBrc[] =
    {
        38, 39, 40, 41
    };

    const sal_uInt16* pBrd = aBorders;
    for( sal_uInt16 i = 0; i < 4; ++i, ++pBrd )
    {
        const SvxBorderLine* pLn = rBox.GetLine( *pBrd );

        sal_uInt16 nSprmNo = 0;
        if ( !bWrtWW8 )
            nSprmNo = aWW6PBrc[i];
        else if ( bOutPageDescs )
            nSprmNo = aSBrc[i];
        else
            nSprmNo = aPBrc[i];

        Out_BorderLine( *pO, pLn, rBox.GetDistance( *pBrd ), nSprmNo, bShadow );
    }
}







void WW8Export::Out_SwFmtTableBox( ww::bytes& rO, const SvxBoxItem * pBox )
{
    
    static const sal_uInt16 aBorders[] =
    {
        BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
    };
    static const SvxBorderLine aBorderLine;

    const sal_uInt16* pBrd = aBorders;
    for( int i = 0; i < 4; ++i, ++pBrd )
    {
        const SvxBorderLine* pLn;
        if (pBox != NULL)
            pLn = pBox->GetLine( *pBrd );
        else
            pLn = & aBorderLine;

        Out_BorderLine(rO, pLn, 0, 0, false);
    }
}

void WW8AttributeOutput::FormatBox( const SvxBoxItem& rBox )
{
    
    
    if ( !m_rWW8Export.bOutGrf )
    {
        bool bShadow = false;
        const SfxPoolItem* pItem = m_rWW8Export.HasItem( RES_SHADOW );
        if ( pItem )
        {
            const SvxShadowItem* p = (const SvxShadowItem*)pItem;
            bShadow = ( p->GetLocation() != SVX_SHADOW_NONE )
                      && ( p->GetWidth() != 0 );
        }

        m_rWW8Export.Out_SwFmtBox( rBox, bShadow );
    }
}

SwTwips WW8Export::CurrentPageWidth(SwTwips &rLeft, SwTwips &rRight) const
{
    const SwFrmFmt* pFmt = pAktPageDesc ? &pAktPageDesc->GetMaster()
        : &pDoc->GetPageDesc(0).GetMaster();

    const SvxLRSpaceItem& rLR = pFmt->GetLRSpace();
    SwTwips nPageSize = pFmt->GetFrmSize().GetWidth();
    rLeft = rLR.GetLeft();
    rRight = rLR.GetRight();
    return nPageSize;
}

void WW8AttributeOutput::FormatColumns_Impl( sal_uInt16 nCols, const SwFmtCol & rCol, bool bEven, SwTwips nPageSize )
{
    
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_SCcolumns );
    else
        m_rWW8Export.pO->push_back( 144 );
    m_rWW8Export.InsUInt16( nCols - 1 );

    
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_SDxaColumns );
    else
        m_rWW8Export.pO->push_back( 145 );
    m_rWW8Export.InsUInt16( rCol.GetGutterWidth( true ) );

    
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_SLBetween );
    else
        m_rWW8Export.pO->push_back( 158 );
    m_rWW8Export.pO->push_back( COLADJ_NONE == rCol.GetLineAdj(  )? 0 : 1 );

    const SwColumns & rColumns = rCol.GetColumns(  );

    
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_SFEvenlySpaced );
    else
        m_rWW8Export.pO->push_back( 138 );
    m_rWW8Export.pO->push_back( bEven ? 1 : 0 );

    if ( !bEven )
    {
        for ( sal_uInt16 n = 0; n < nCols; ++n )
        {
            
            if ( m_rWW8Export.bWrtWW8 )
                m_rWW8Export.InsUInt16( NS_sprm::LN_SDxaColWidth );
            else
                m_rWW8Export.pO->push_back( 136 );
            m_rWW8Export.pO->push_back( static_cast<sal_uInt8>(n) );
            m_rWW8Export.InsUInt16( rCol.
                                    CalcPrtColWidth( n,
                                                     ( sal_uInt16 ) nPageSize ) );

            if ( n + 1 != nCols )
            {
                
                if ( m_rWW8Export.bWrtWW8 )
                    m_rWW8Export.InsUInt16( NS_sprm::LN_SDxaColSpacing );
                else
                    m_rWW8Export.pO->push_back( 137 );
                m_rWW8Export.pO->push_back( static_cast<sal_uInt8>(n) );
                m_rWW8Export.InsUInt16( rColumns[n].GetRight(  ) +
                                        rColumns[n + 1].GetLeft(  ) );
            }
        }
    }
}

void AttributeOutputBase::FormatColumns( const SwFmtCol& rCol )
{
    const SwColumns& rColumns = rCol.GetColumns();

    sal_uInt16 nCols = rColumns.size();
    if ( 1 < nCols && !GetExport( ).bOutFlyFrmAttrs )
    {
        

        const SwFrmFmt* pFmt = GetExport( ).pAktPageDesc ? &GetExport( ).pAktPageDesc->GetMaster() : &const_cast<const SwDoc *>(GetExport( ).pDoc)->GetPageDesc(0).GetMaster();
        const SvxFrameDirectionItem &frameDirection = pFmt->GetFrmDir();
        SwTwips nPageSize;
        if ( frameDirection.GetValue() == FRMDIR_VERT_TOP_RIGHT || frameDirection.GetValue() == FRMDIR_VERT_TOP_LEFT )
        {
            const SvxULSpaceItem &rUL = pFmt->GetULSpace();
            nPageSize = pFmt->GetFrmSize().GetHeight();
            nPageSize -= rUL.GetUpper() + rUL.GetLower();

            const SwFmtHeader *header = dynamic_cast<const SwFmtHeader *>(pFmt->GetAttrSet().GetItem(RES_HEADER));
            if ( header )
            {
                const SwFrmFmt *headerFmt = header->GetHeaderFmt();
                if (headerFmt)
                {
                    nPageSize -= headerFmt->GetFrmSize().GetHeight();
                }
            }
            const SwFmtFooter *footer = dynamic_cast<const SwFmtFooter *>(pFmt->GetAttrSet().GetItem(RES_FOOTER));
            if ( footer )
            {
                const SwFrmFmt *footerFmt = footer->GetFooterFmt();
                if ( footerFmt )
                {
                    nPageSize -= footerFmt->GetFrmSize().GetHeight();
                }
            }
        }
        else
        {
            const SvxLRSpaceItem &rLR = pFmt->GetLRSpace();
            nPageSize = pFmt->GetFrmSize().GetWidth();
            nPageSize -= rLR.GetLeft() + rLR.GetRight();
            
            nPageSize -= rCol.GetAdjustValue();

        }

        
        bool bEven = true;
        sal_uInt16 n;
        sal_uInt16 nColWidth = rCol.CalcPrtColWidth( 0, (sal_uInt16)nPageSize );
        for ( n = 1; n < nCols; n++ )
        {
            short nDiff = nColWidth -
                rCol.CalcPrtColWidth( n, (sal_uInt16)nPageSize );

            if ( nDiff > 10 || nDiff < -10 )      
            {
                bEven = false;
                break;
            }
        }

        FormatColumns_Impl( nCols, rCol, bEven, nPageSize );
    }
}


void WW8AttributeOutput::FormatKeep( const SvxFmtKeepItem& rKeep )
{
    
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFKeepFollow );
    else
        m_rWW8Export.pO->push_back( 8 );

    m_rWW8Export.pO->push_back( rKeep.GetValue() ? 1 : 0 );
}


void WW8AttributeOutput::FormatLineNumbering( const SwFmtLineNumber& rNumbering )
{
    
    if( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFNoLineNumb );
    else
        m_rWW8Export.pO->push_back( 14 );

    m_rWW8Export.pO->push_back( rNumbering.IsCount() ? 0 : 1 );
}


/* File PARATR.HXX  */

void WW8AttributeOutput::ParaLineSpacing_Impl( short nSpace, short nMulti )
{
    
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaLine );
    else
        m_rWW8Export.pO->push_back( 20 );

    m_rWW8Export.InsUInt16( nSpace );
    m_rWW8Export.InsUInt16( nMulti );
}

void AttributeOutputBase::ParaLineSpacing( const SvxLineSpacingItem& rSpacing )
{
    short nSpace = 240, nMulti = 0;

    switch ( rSpacing.GetLineSpaceRule() )
    {
        default:
            break;
        case SVX_LINE_SPACE_FIX: 
            nSpace = -(short)rSpacing.GetLineHeight();
            break;
        case SVX_LINE_SPACE_MIN: 
            nSpace = (short)rSpacing.GetLineHeight();
            break;
        case SVX_LINE_SPACE_AUTO:
        {
            if( rSpacing.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX ) 
            {
                
                
                nSpace = (short)rSpacing.GetInterLineSpace();
                sal_uInt16 nScript =
                    i18n::ScriptType::LATIN;
                const SwAttrSet *pSet = 0;
                if ( GetExport().pOutFmtNode && GetExport().pOutFmtNode->ISA( SwFmt ) )
                {
                    const SwFmt *pFmt = (const SwFmt*)( GetExport().pOutFmtNode );
                    pSet = &pFmt->GetAttrSet();
                }
                else if ( GetExport().pOutFmtNode && GetExport().pOutFmtNode->ISA( SwTxtNode ) )
                {
                    const SwTxtNode* pNd = (const SwTxtNode*)GetExport().pOutFmtNode;
                    pSet = &pNd->GetSwAttrSet();
                    if ( g_pBreakIt->GetBreakIter().is() )
                    {
                        nScript = g_pBreakIt->GetBreakIter()->
                            getScriptType(pNd->GetTxt(), 0);
                    }
                }
                OSL_ENSURE( pSet, "No attrset for lineheight :-(" );
                if ( pSet )
                {
                    nSpace = nSpace + (short)( AttrSetToLineHeight( *GetExport().pDoc,
                        *pSet, *Application::GetDefaultDevice(), nScript ) );
                }
            }
            else 
            {
                nSpace = (short)( ( 240L * rSpacing.GetPropLineSpace() ) / 100L );
                nMulti = 1;
                break;
            }
        }
        break;
    }
    
    
    
    ParaLineSpacing_Impl( nSpace, nMulti );
}

void WW8AttributeOutput::ParaAdjust( const SvxAdjustItem& rAdjust )
{
    
    sal_uInt8 nAdj = 255;
    sal_uInt8 nAdjBiDi = 255;
    switch ( rAdjust.GetAdjust() )
    {
        case SVX_ADJUST_LEFT:
            nAdj = 0;
            nAdjBiDi = 2;
            break;
        case SVX_ADJUST_RIGHT:
            nAdj = 2;
            nAdjBiDi = 0;
            break;
        case SVX_ADJUST_BLOCKLINE:
        case SVX_ADJUST_BLOCK:
            nAdj = nAdjBiDi = 3;
            break;
        case SVX_ADJUST_CENTER:
            nAdj = nAdjBiDi = 1;
            break;
        default:
            return;    
    }

    if ( 255 != nAdj )        
    {
        if ( m_rWW8Export.bWrtWW8 )
        {
            m_rWW8Export.InsUInt16( NS_sprm::LN_PJc );
            m_rWW8Export.pO->push_back( nAdj );

            /*
            Sadly for left to right paragraphs both these values are the same,
            for right to left paragraphs the bidi one is the reverse of the
            normal one.
            */
            m_rWW8Export.InsUInt16( NS_sprm::LN_PJcExtra ); 
            bool bBiDiSwap = false;
            if ( m_rWW8Export.pOutFmtNode )
            {
                short nDirection = FRMDIR_HORI_LEFT_TOP;
                if ( m_rWW8Export.pOutFmtNode->ISA( SwTxtNode ) )
                {
                    SwPosition aPos(*(const SwCntntNode*)m_rWW8Export.pOutFmtNode);
                    nDirection = m_rWW8Export.pDoc->GetTextDirection(aPos);
                }
                else if ( m_rWW8Export.pOutFmtNode->ISA( SwTxtFmtColl ) )
                {
                    const SwTxtFmtColl* pC =
                        (const SwTxtFmtColl*)m_rWW8Export.pOutFmtNode;
                    const SvxFrameDirectionItem &rItem =
                        ItemGet<SvxFrameDirectionItem>(*pC, RES_FRAMEDIR);
                    nDirection = rItem.GetValue();
                }
                if ( ( nDirection == FRMDIR_HORI_RIGHT_TOP ) ||
                     ( nDirection == FRMDIR_ENVIRONMENT && Application::GetSettings().GetLayoutRTL() ) )
                {
                    bBiDiSwap = true;
                }
            }

            if ( bBiDiSwap )
                m_rWW8Export.pO->push_back( nAdjBiDi );
            else
                m_rWW8Export.pO->push_back( nAdj );
        }
        else
        {
            m_rWW8Export.pO->push_back( 5 );
            m_rWW8Export.pO->push_back( nAdj );
        }
    }
}

void WW8AttributeOutput::FormatFrameDirection( const SvxFrameDirectionItem& rDirection )
{
    if ( !m_rWW8Export.bWrtWW8 )   
        return;

    sal_uInt16 nTextFlow=0;
    bool bBiDi = false;
    short nDir = rDirection.GetValue();

    if ( nDir == FRMDIR_ENVIRONMENT )
    {
        if ( m_rWW8Export.bOutPageDescs )
            nDir = m_rWW8Export.GetCurrentPageDirection();
        else if ( m_rWW8Export.pOutFmtNode )
        {
            if ( m_rWW8Export.bOutFlyFrmAttrs )  
            {
                nDir = m_rWW8Export.TrueFrameDirection(
                    *(const SwFrmFmt*)m_rWW8Export.pOutFmtNode );
            }
            else if ( m_rWW8Export.pOutFmtNode->ISA( SwCntntNode ) )   
            {
                const SwCntntNode* pNd =
                    (const SwCntntNode*)m_rWW8Export.pOutFmtNode;
                SwPosition aPos( *pNd );
                nDir = m_rWW8Export.pDoc->GetTextDirection( aPos );
            }
            else if ( m_rWW8Export.pOutFmtNode->ISA( SwTxtFmtColl ) )
                nDir = FRMDIR_HORI_LEFT_TOP;    
        }

        if ( nDir == FRMDIR_ENVIRONMENT )
            nDir = FRMDIR_HORI_LEFT_TOP;    
    }

    switch ( nDir )
    {
        default:
            
            OSL_FAIL("Unknown frame direction");
        case FRMDIR_HORI_LEFT_TOP:
            nTextFlow = 0;
            break;
        case FRMDIR_HORI_RIGHT_TOP:
            nTextFlow = 0;
            bBiDi = true;
            break;
        case FRMDIR_VERT_TOP_LEFT:  
        case FRMDIR_VERT_TOP_RIGHT:
            nTextFlow = 1;
            break;
    }

    if ( m_rWW8Export.bOutPageDescs )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_STextFlow );
        m_rWW8Export.InsUInt16( nTextFlow );
        m_rWW8Export.InsUInt16( NS_sprm::LN_SFBiDi );
        m_rWW8Export.pO->push_back( bBiDi );
    }
    else if ( !m_rWW8Export.bOutFlyFrmAttrs )  
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFBiDi );
        m_rWW8Export.pO->push_back( bBiDi );
    }
}

void WW8AttributeOutput::ParaGrabBag(const SfxGrabBagItem& /*rItem*/)
{
}

void WW8AttributeOutput::CharGrabBag(const SfxGrabBagItem& /*rItem*/)
{
}

void WW8AttributeOutput::ParaOutlineLevel(const SfxUInt16Item& /*rItem*/)
{
}


void WW8AttributeOutput::ParaSplit( const SvxFmtSplitItem& rSplit )
{
    
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFKeep );
    else
        m_rWW8Export.pO->push_back( 7 );
    m_rWW8Export.pO->push_back( rSplit.GetValue() ? 0 : 1 );
}




void WW8AttributeOutput::ParaWidows( const SvxWidowsItem& rWidows )
{

    if( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFWidowControl );
    else
        m_rWW8Export.pO->push_back( 51 );
    m_rWW8Export.pO->push_back( rWidows.GetValue() ? 1 : 0 );
}


class SwWW8WrTabu
{
    sal_uInt8* pDel;                 
    sal_uInt8* pAddPos;              
    sal_uInt8* pAddTyp;              
    sal_uInt16 nAdd;            
    sal_uInt16 nDel;            
public:
    SwWW8WrTabu(sal_uInt16 nDelMax, sal_uInt16 nAddMax);
    ~SwWW8WrTabu();

    void Add(const SvxTabStop &rTS, long nAdjustment);
    void Del(const SvxTabStop &rTS, long nAdjustment);
    void PutAll(WW8Export& rWW8Wrt);
};

SwWW8WrTabu::SwWW8WrTabu(sal_uInt16 nDelMax, sal_uInt16 nAddMax)
    : nAdd(0), nDel(0)
{
    pDel = nDelMax ? new sal_uInt8[nDelMax * 2] : 0;
    pAddPos = new sal_uInt8[nAddMax * 2];
    pAddTyp = new sal_uInt8[nAddMax];
}

SwWW8WrTabu::~SwWW8WrTabu()
{
    delete[] pAddTyp;
    delete[] pAddPos;
    delete[] pDel;
}


void SwWW8WrTabu::Add(const SvxTabStop & rTS, long nAdjustment)
{
    
    ShortToSVBT16(msword_cast<sal_Int16>(rTS.GetTabPos() + nAdjustment),
        pAddPos + (nAdd * 2));

    
    sal_uInt8 nPara = 0;
    switch (rTS.GetAdjustment())
    {
        case SVX_TAB_ADJUST_RIGHT:
            nPara = 2;
            break;
        case SVX_TAB_ADJUST_CENTER:
            nPara = 1;
            break;
        case SVX_TAB_ADJUST_DECIMAL:
            /*
            Theres nothing we can do btw the decimal separator has been
            customized, but if you think different remember that different
            locales have different separators, i.e. german is a , while english
            is a .
            */
            nPara = 3;
            break;
        default:
            break;
    }

    switch( rTS.GetFill() )
    {
        case '.':   
            nPara |= 1 << 3;
            break;
        case '_':   
            nPara |= 3 << 3;
            break;
        case '-':   
            nPara |= 2 << 3;
            break;
        case '=':   
            nPara |= 4 << 3;
            break;
    }

    pAddTyp[nAdd] = nPara;
    ++nAdd;
}



void SwWW8WrTabu::Del(const SvxTabStop &rTS, long nAdjustment)
{
    
    ShortToSVBT16(msword_cast<sal_Int16>(rTS.GetTabPos() + nAdjustment),
        pDel + (nDel * 2));
    ++nDel;
}


void SwWW8WrTabu::PutAll(WW8Export& rWrt)
{
    if (!nAdd && !nDel) 
        return;
    OSL_ENSURE(nAdd <= 255, "more than 255 added tabstops ?");
    OSL_ENSURE(nDel <= 255, "more than 244 removed tabstops ?");
    if (nAdd > 255)
        nAdd = 255;
    if (nDel > 255)
        nDel = 255;

    sal_uInt16 nSiz = 2 * nDel + 3 * nAdd + 2;
    if (nSiz > 255)
        nSiz = 255;

    if (rWrt.bWrtWW8)
        rWrt.InsUInt16(NS_sprm::LN_PChgTabsPapx);
    else
        rWrt.pO->push_back(15);
    
    rWrt.pO->push_back(msword_cast<sal_uInt8>(nSiz));
    
    rWrt.pO->push_back(msword_cast<sal_uInt8>(nDel));
    rWrt.OutSprmBytes(pDel, nDel * 2);
    
    rWrt.pO->push_back(msword_cast<sal_uInt8>(nAdd));
    rWrt.OutSprmBytes(pAddPos, 2 * nAdd);         
    rWrt.OutSprmBytes(pAddTyp, nAdd);             
}

static void ParaTabStopAdd( WW8Export& rWrt,
                            const SvxTabStopItem& rTStops,
                            const long nLParaMgn )
{
    SwWW8WrTabu aTab( 0, rTStops.Count());

    for( sal_uInt16 n = 0; n < rTStops.Count(); n++ )
    {
        const SvxTabStop& rTS = rTStops[n];
        
        if (SVX_TAB_ADJUST_DEFAULT != rTS.GetAdjustment())
            aTab.Add(rTS, nLParaMgn);
    }
    aTab.PutAll( rWrt );
}

static bool lcl_IsEqual(long nOneLeft, const SvxTabStop &rOne,
    long nTwoLeft, const SvxTabStop &rTwo)
{
    return(
            nOneLeft == nTwoLeft &&
            rOne.GetAdjustment() == rTwo.GetAdjustment() &&
            rOne.GetDecimal() == rTwo.GetDecimal() &&
            rOne.GetFill() == rTwo.GetFill()
          );
}

static void ParaTabStopDelAdd( WW8Export& rWrt,
                               const SvxTabStopItem& rTStyle,
                               const long nLStypeMgn,
                               const SvxTabStopItem& rTNew,
                               const long nLParaMgn )
{
    SwWW8WrTabu aTab(rTStyle.Count(), rTNew.Count());

    sal_uInt16 nO = 0;      
    sal_uInt16 nN = 0;      

    do {
        const SvxTabStop* pTO;
        long nOP;
        if( nO < rTStyle.Count() )                  
        {
            pTO = &rTStyle[ nO ];
            nOP = pTO->GetTabPos() + nLStypeMgn;
            if( SVX_TAB_ADJUST_DEFAULT == pTO->GetAdjustment() )
            {
                nO++;                                
                continue;
            }
        }
        else
        {
            pTO = 0;
            nOP = LONG_MAX;
        }

        const SvxTabStop* pTN;
        long nNP;
        if( nN < rTNew.Count() )                    
        {
            pTN = &rTNew[ nN ];
            nNP = pTN->GetTabPos() + nLParaMgn;
            if( SVX_TAB_ADJUST_DEFAULT == pTN->GetAdjustment() )
            {
                nN++;                               
                continue;
            }
        }
        else
        {
            pTN = 0;
            nNP = LONG_MAX;
        }

        if( nOP == LONG_MAX && nNP == LONG_MAX )
            break;                                  

        if( nOP < nNP )                             
        {
            aTab.Del(*pTO, nLStypeMgn);             
            nO++;
        }
        else if( nNP < nOP )                        
        {
            aTab.Add(*pTN, nLParaMgn);              
            nN++;
        }
        else if (lcl_IsEqual(nOP, *pTO, nNP, *pTN)) 
        {
            nO++;                                   
            nN++;
        }
        else                                        
        {
            aTab.Del(*pTO, nLStypeMgn);             
            aTab.Add(*pTN, nLParaMgn);              
            nO++;
            nN++;
        }
    } while( true );

    aTab.PutAll( rWrt );
}

void WW8AttributeOutput::ParaTabStop( const SvxTabStopItem& rTabStops )
{
    const bool bTabsRelativeToIndex = m_rWW8Export.pCurPam->GetDoc()->get( IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT );

    long nCurrentLeft = 0;
    if ( bTabsRelativeToIndex )
    {
        const SfxPoolItem* pLR = m_rWW8Export.HasItem( RES_LR_SPACE );

        if ( pLR != NULL )
            nCurrentLeft = static_cast<const SvxLRSpaceItem*>(pLR)->GetTxtLeft();
    }

    
    if ( m_rWW8Export.bStyDef &&
         m_rWW8Export.pCurrentStyle != NULL &&
         m_rWW8Export.pCurrentStyle->DerivedFrom() != NULL )
    {
        SvxTabStopItem aParentTabs( 0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
        const SwFmt *pParentStyle = m_rWW8Export.pCurrentStyle->DerivedFrom();
        {
            const SvxTabStopItem* pParentTabs = HasItem<SvxTabStopItem>( pParentStyle->GetAttrSet(), RES_PARATR_TABSTOP );
            if ( pParentTabs )
            {
                aParentTabs.Insert( pParentTabs );
            }
        }

        
        long nParentLeft = 0;
        if ( bTabsRelativeToIndex )
        {
            const SvxLRSpaceItem &rStyleLR = ItemGet<SvxLRSpaceItem>( pParentStyle->GetAttrSet(), RES_LR_SPACE );
            nParentLeft = rStyleLR.GetTxtLeft();
        }

        ParaTabStopDelAdd( m_rWW8Export, aParentTabs, nParentLeft, rTabStops, nCurrentLeft );
        return;
    }

    const SvxTabStopItem* pStyleTabs = 0;
    if ( !m_rWW8Export.bStyDef && m_rWW8Export.pStyAttr )
    {
        pStyleTabs = HasItem<SvxTabStopItem>( *m_rWW8Export.pStyAttr, RES_PARATR_TABSTOP );
    }

    if ( !pStyleTabs )
    {
        ParaTabStopAdd(m_rWW8Export, rTabStops, nCurrentLeft);
    }
    else
    {
        long nStyleLeft = 0;
        if ( bTabsRelativeToIndex )
        {
            const SvxLRSpaceItem &rStyleLR = ItemGet<SvxLRSpaceItem>(*m_rWW8Export.pStyAttr, RES_LR_SPACE);
            nStyleLeft = rStyleLR.GetTxtLeft();
        }

        ParaTabStopDelAdd( m_rWW8Export,
                           *pStyleTabs, nStyleLeft,
                           rTabStops, nCurrentLeft);
    }
}



void AttributeOutputBase::OutputItem( const SfxPoolItem& rHt )
{
    
    switch ( rHt.Which() )
    {
        case RES_CHRATR_CASEMAP:
            CharCaseMap( static_cast< const SvxCaseMapItem& >( rHt ) );
            break;
        case RES_CHRATR_COLOR:
            CharColor( static_cast< const SvxColorItem& >( rHt ) );
            break;
        case RES_CHRATR_CONTOUR:
            CharContour( static_cast< const SvxContourItem& >( rHt ) );
            break;
        case RES_CHRATR_CROSSEDOUT:
            CharCrossedOut( static_cast< const SvxCrossedOutItem& >( rHt ) );
            break;
        case RES_CHRATR_ESCAPEMENT:
            CharEscapement( static_cast< const SvxEscapementItem& >( rHt ) );
            break;
        case RES_CHRATR_FONT:
            CharFont( static_cast< const SvxFontItem& >( rHt ) );
            break;
        case RES_CHRATR_FONTSIZE:
            CharFontSize( static_cast< const SvxFontHeightItem& >( rHt ) );
            break;
        case RES_CHRATR_KERNING:
            CharKerning( static_cast< const SvxKerningItem& >( rHt ) );
            break;
        case RES_CHRATR_LANGUAGE:
            CharLanguage( static_cast< const SvxLanguageItem& >( rHt ) );
            break;
        case RES_CHRATR_POSTURE:
            CharPosture( static_cast< const SvxPostureItem& >( rHt ) );
            break;
        case RES_CHRATR_SHADOWED:
            CharShadow( static_cast< const SvxShadowedItem& >( rHt ) );
            break;
        case RES_CHRATR_UNDERLINE:
            CharUnderline( static_cast< const SvxUnderlineItem& >( rHt ) );
            break;
        case RES_CHRATR_WEIGHT:
            CharWeight( static_cast< const SvxWeightItem& >( rHt ) );
            break;
        case RES_CHRATR_AUTOKERN:
            CharAutoKern( static_cast< const SvxAutoKernItem& >( rHt ) );
            break;
        case RES_CHRATR_BLINK:
            CharAnimatedText( static_cast< const SvxBlinkItem& >( rHt ) );
            break;
        case RES_CHRATR_BACKGROUND:
            CharBackground( static_cast< const SvxBrushItem& >( rHt ) );
            break;

        case RES_CHRATR_CJK_FONT:
            CharFontCJK( static_cast< const SvxFontItem& >( rHt ) );
            break;
        case RES_CHRATR_CJK_FONTSIZE:
            CharFontSizeCJK( static_cast< const SvxFontHeightItem& >( rHt ) );
            break;
        case RES_CHRATR_CJK_LANGUAGE:
            CharLanguageCJK( static_cast< const SvxLanguageItem& >( rHt ) );
            break;
        case RES_CHRATR_CJK_POSTURE:
            CharPostureCJK( static_cast< const SvxPostureItem& >( rHt ) );
            break;
        case RES_CHRATR_CJK_WEIGHT:
            CharWeightCJK( static_cast< const SvxWeightItem& >( rHt ) );
            break;

        case RES_CHRATR_CTL_FONT:
            CharFontCTL( static_cast< const SvxFontItem& >( rHt ) );
            break;
        case RES_CHRATR_CTL_FONTSIZE:
            CharFontSizeCTL( static_cast< const SvxFontHeightItem& >( rHt ) );
            break;
        case RES_CHRATR_CTL_LANGUAGE:
            CharLanguageCTL( static_cast< const SvxLanguageItem& >( rHt ) );
            break;
        case RES_CHRATR_CTL_POSTURE:
            CharPostureCTL( static_cast< const SvxPostureItem& >( rHt ) );
            break;
        case RES_CHRATR_CTL_WEIGHT:
            CharWeightCTL( static_cast< const SvxWeightItem& >( rHt ) );
            break;

        case RES_CHRATR_ROTATE:
            CharRotate( static_cast< const SvxCharRotateItem& >( rHt ) );
            break;
        case RES_CHRATR_EMPHASIS_MARK:
            CharEmphasisMark( static_cast< const SvxEmphasisMarkItem& >( rHt ) );
            break;
        case RES_CHRATR_TWO_LINES:
            CharTwoLines( static_cast< const SvxTwoLinesItem& >( rHt ) );
            break;
        case RES_CHRATR_SCALEW:
            CharScaleWidth( static_cast< const SvxCharScaleWidthItem& >( rHt ) );
            break;
        case RES_CHRATR_RELIEF:
            CharRelief( static_cast< const SvxCharReliefItem& >( rHt ) );
            break;
        case RES_CHRATR_HIDDEN:
            CharHidden( static_cast< const SvxCharHiddenItem& >( rHt ) );
            break;
        case RES_CHRATR_BOX:
            FormatCharBorder( static_cast< const SvxBoxItem& >( rHt ) );
            break;
        case RES_CHRATR_HIGHLIGHT:
            CharHighlight( static_cast< const SvxBrushItem& >( rHt ) );
            break;

        case RES_TXTATR_INETFMT:
            TextINetFormat( static_cast< const SwFmtINetFmt& >( rHt ) );
            break;
        case RES_TXTATR_CHARFMT:
            TextCharFormat( static_cast< const SwFmtCharFmt& >( rHt ) );
            break;

        case RES_TXTATR_FIELD:
        case RES_TXTATR_ANNOTATION:
        case RES_TXTATR_INPUTFIELD:
            TextField( static_cast< const SwFmtFld& >( rHt ) );
            break;

        case RES_TXTATR_FLYCNT:
            TextFlyContent( static_cast< const SwFmtFlyCnt& >( rHt ) );
            break;
        case RES_TXTATR_FTN:
            TextFootnote( static_cast< const SwFmtFtn& >( rHt ) );
            break;

        case RES_PARATR_LINESPACING:
            ParaLineSpacing( static_cast< const SvxLineSpacingItem& >( rHt ) );
            break;
        case RES_PARATR_ADJUST:
            ParaAdjust( static_cast< const SvxAdjustItem& >( rHt ) );
            break;
        case RES_PARATR_SPLIT:
            ParaSplit( static_cast< const SvxFmtSplitItem& >( rHt ) );
            break;
        case RES_PARATR_WIDOWS:
            ParaWidows( static_cast< const SvxWidowsItem& >( rHt ) );
            break;
        case RES_PARATR_TABSTOP:
            ParaTabStop( static_cast< const SvxTabStopItem& >( rHt ) );
            break;
        case RES_PARATR_HYPHENZONE:
            ParaHyphenZone( static_cast< const SvxHyphenZoneItem& >( rHt ) );
            break;
        case RES_PARATR_NUMRULE:
            ParaNumRule( static_cast< const SwNumRuleItem& >( rHt ) );
            break;
        case RES_PARATR_SCRIPTSPACE:
            ParaScriptSpace( static_cast< const SfxBoolItem& >( rHt ) );
            break;
        case RES_PARATR_HANGINGPUNCTUATION:
            ParaHangingPunctuation( static_cast< const SfxBoolItem& >( rHt ) );
            break;
        case RES_PARATR_FORBIDDEN_RULES:
            ParaForbiddenRules( static_cast< const SfxBoolItem& >( rHt ) );
            break;
        case RES_PARATR_VERTALIGN:
            ParaVerticalAlign( static_cast< const SvxParaVertAlignItem& >( rHt ) );
            break;
        case RES_PARATR_SNAPTOGRID:
            ParaSnapToGrid( static_cast< const SvxParaGridItem& >( rHt ) );
            break;

        case RES_FRM_SIZE:
            FormatFrameSize( static_cast< const SwFmtFrmSize& >( rHt ) );
            break;
        case RES_PAPER_BIN:
            FormatPaperBin( static_cast< const SvxPaperBinItem& >( rHt ) );
            break;
        case RES_LR_SPACE:
            FormatLRSpace( static_cast< const SvxLRSpaceItem& >( rHt ) );
            break;
        case RES_UL_SPACE:
            FormatULSpace( static_cast< const SvxULSpaceItem& >( rHt ) );
            break;
        case RES_PAGEDESC:
            FormatPageDescription( static_cast< const SwFmtPageDesc& >( rHt ) );
            break;
        case RES_BREAK:
            FormatBreak( static_cast< const SvxFmtBreakItem& >( rHt ) );
            break;
        case RES_SURROUND:
            FormatSurround( static_cast< const SwFmtSurround& >( rHt ) );
            break;
        case RES_VERT_ORIENT:
            FormatVertOrientation( static_cast< const SwFmtVertOrient& >( rHt ) );
            break;
        case RES_HORI_ORIENT:
            FormatHorizOrientation( static_cast< const SwFmtHoriOrient& >( rHt ) );
            break;
        case RES_ANCHOR:
            FormatAnchor( static_cast< const SwFmtAnchor& >( rHt ) );
            break;
        case RES_BACKGROUND:
            FormatBackground( static_cast< const SvxBrushItem& >( rHt ) );
            break;
        case RES_FILL_STYLE:
            FormatFillStyle( static_cast< const XFillStyleItem& >( rHt ) );
            break;
        case RES_FILL_GRADIENT:
            FormatFillGradient( static_cast< const XFillGradientItem& >( rHt ) );
            break;
        case RES_BOX:
            FormatBox( static_cast< const SvxBoxItem& >( rHt ) );
            break;
        case RES_COL:
            FormatColumns( static_cast< const SwFmtCol& >( rHt ) );
            break;
        case RES_KEEP:
            FormatKeep( static_cast< const SvxFmtKeepItem& >( rHt ) );
            break;
        case RES_TEXTGRID:
            FormatTextGrid( static_cast< const SwTextGridItem& >( rHt ) );
            break;
        case RES_LINENUMBER:
            FormatLineNumbering( static_cast< const SwFmtLineNumber& >( rHt ) );
            break;
        case RES_FRAMEDIR:
            FormatFrameDirection( static_cast< const SvxFrameDirectionItem& >( rHt ) );
            break;
        case RES_PARATR_GRABBAG:
            ParaGrabBag(static_cast<const SfxGrabBagItem&>(rHt));
            break;
        case RES_PARATR_OUTLINELEVEL:
            ParaOutlineLevel(static_cast<const SfxUInt16Item&>(rHt));
            break;
        case RES_CHRATR_GRABBAG:
            CharGrabBag(static_cast<const SfxGrabBagItem&>(rHt));
            break;

        default:
            SAL_INFO("sw.ww8", "Unhandled SfxPoolItem with id " << rHt.Which() );
            break;
    }
}

void AttributeOutputBase::OutputStyleItemSet( const SfxItemSet& rSet, sal_Bool bDeep, sal_Bool bTestForDefault )
{
    

    const SfxItemPool& rPool = *rSet.GetPool();
    const SfxItemSet* pSet = &rSet;
    if ( !pSet->Count() )
    {
        if ( !bDeep )
            return;

        while ( 0 != ( pSet = pSet->GetParent() ) && !pSet->Count() )
            ;

        if ( !pSet )
            return;
    }

    const SfxPoolItem* pItem;
    if ( !bDeep || !pSet->GetParent() )
    {
        OSL_ENSURE( rSet.Count(), "Wurde doch schon behandelt oder?" );
        SfxItemIter aIter( *pSet );
        pItem = aIter.GetCurItem();
        do {
            OutputItem( *pItem );
        } while ( !aIter.IsAtEnd() && 0 != ( pItem = aIter.NextItem() ) );
    }
    else
    {
        SfxWhichIter aIter( *pSet );
        sal_uInt16 nWhich = aIter.FirstWhich();
        while ( nWhich )
        {
            if ( SFX_ITEM_SET == pSet->GetItemState( nWhich, bDeep, &pItem ) &&
                 ( !bTestForDefault ||
                   *pItem != rPool.GetDefaultItem( nWhich ) ||
                   ( pSet->GetParent() && *pItem != pSet->GetParent()->Get( nWhich ) ) ) )
            {
                OutputItem( *pItem );
            }
            nWhich = aIter.NextWhich();
        }
    }
}

void AttributeOutputBase::FormatCharBorder( const SvxBoxItem& rBox )
{
    
    const SvxBorderLine* pBorderLine = 0;
    sal_uInt16 nDist = 0;
    if( rBox.GetTop() )
    {
       pBorderLine = rBox.GetTop();
       nDist = rBox.GetDistance( BOX_LINE_TOP );
    }
    else if( rBox.GetLeft() )
    {
       pBorderLine = rBox.GetLeft();
       nDist = rBox.GetDistance( BOX_LINE_LEFT );
    }
    else if( rBox.GetBottom() )
    {
       pBorderLine = rBox.GetBottom();
       nDist = rBox.GetDistance( BOX_LINE_BOTTOM );
    }
    else if( rBox.GetRight() )
    {
       pBorderLine = rBox.GetRight();
       nDist = rBox.GetDistance( BOX_LINE_RIGHT );
    }

    if( pBorderLine )
    {
        const SfxPoolItem* pItem = GetExport().HasItem( RES_CHRATR_SHADOW );
        const SvxShadowItem* pShadowItem = static_cast<const SvxShadowItem*>(pItem);
        const bool bShadow =
            pShadowItem && pShadowItem->GetLocation() != SVX_SHADOW_NONE &&
            pShadowItem->GetWidth() > 0;

        CharBorder( pBorderLine, nDist, bShadow );
    }
}

/*
 * This function is used to check if the current SwTxtNode (paragraph) has a redline object
 * that is attached to the paragraph marker.
 * This is done by checking if the range (SwPaM) of the redline is :
 * - Start = the last character of the current paragraph
 * - End = the first character of the next paragraph
 */
const SwRedlineData* AttributeOutputBase::GetParagraphMarkerRedline( const SwTxtNode& rNode, RedlineType_t aRedlineType)
{
    
    for( sal_uInt16 nRedlinePos = 0; nRedlinePos < GetExport().pDoc->GetRedlineTbl().size(); ++nRedlinePos )
    {
        const SwRangeRedline* pRedl = GetExport().pDoc->GetRedlineTbl()[ nRedlinePos ];

        
        if ( pRedl->GetRedlineData().GetType() != aRedlineType )
            continue;

        const SwPosition* pCheckedStt = pRedl->Start();
        const SwPosition* pCheckedEnd = pRedl->End();

        if( pCheckedStt->nNode == rNode )
        {
            if ( !pCheckedEnd )
                continue;

            sal_uLong uStartNodeIndex = pCheckedStt->nNode.GetIndex();
            sal_uLong uStartCharIndex = pCheckedStt->nContent.GetIndex();
            sal_uLong uEndNodeIndex   = pCheckedEnd->nNode.GetIndex();
            sal_uLong uEndCharIndex   = pCheckedEnd->nContent.GetIndex();

            
            if ( ( uStartNodeIndex == uEndNodeIndex - 1 ) &&
                 ( uStartCharIndex == (sal_uLong)rNode.Len() ) &&
                 ( uEndCharIndex == 0)
               )
            {
                return &( pRedl->GetRedlineData() );
            }
        }
    }
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
