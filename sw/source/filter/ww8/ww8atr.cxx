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


/*
 * This file contains methods for the WW8 output
 * (nodes, attributes, formats und chars).
 */

#include <hintids.hxx>

#include <vcl/svapp.hxx>
#include <vcl/salbtype.hxx>
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
#include <doc.hxx>          // Doc for footnotes
#include <docary.hxx>
#include <pam.hxx>
#include <paratr.hxx>
#include <fldbas.hxx>       // for SwField
#include <docufld.hxx>      // for SwField
#include <expfld.hxx>
#include <pagedesc.hxx>     // for SwPageDesc
#include <flddat.hxx>       // for Date fields
#include <ndtxt.hxx>        // for Numrules
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

#include <doctok/sprmids.hxx>

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
        //for asian in ww8, there is only one fontsize
        //and one fontstyle (posture/weight) for ww6
        //there is the additional problem that there
        //is only one font setting for all three scripts
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
        //Complex is ok in ww8, but for ww6 there is only
        //one font, one fontsize, one fontsize (weight/posture)
        //and only one language
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
        //for western in ww8, there is only one fontsize
        //and one fontstyle (posture/weight) for ww6
        //there is the additional problem that there
        //is only one font setting for all three scripts
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

//------------------------------------------------------------
//  Hilfsroutinen fuer Styles
//------------------------------------------------------------

void MSWordExportBase::ExportPoolItemsToCHP( sw::PoolItems &rItems, sal_uInt16 nScript )
{
    sw::cPoolItemIter aEnd = rItems.end();
    for ( sw::cPoolItemIter aI = rItems.begin(); aI != aEnd; ++aI )
    {
        const SfxPoolItem *pItem = aI->second;
        sal_uInt16 nWhich = pItem->Which();
        if ( ( isCHRATR( nWhich ) || isTXTATR( nWhich ) ) && CollapseScriptsforWordOk( nScript, nWhich ) )
        {
             //In the id definition, RES_TXTATR_INETFMT must precede RES_TXTATR_CHARFMT, so that link style can overwrite char style.
             //and in #i24291# it describes "All we want to do is ensure for now is that if a charfmt exist in the character
             //properties that it rises to the top and is exported first."
             //In bug 119649, it is in such situation, so we need to ignore the link style when doing ms word filter exports and
             //add the second judgement for #i24291# definition.
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
        pISet = &rSet;                  // fuer Doppel-Attribute

        // If frame dir is set, but not adjust, then force adjust as well
        if ( bPapFmt && SFX_ITEM_SET == rSet.GetItemState( RES_FRAMEDIR, bExportParentItemSet ) )
        {
            // No explicit adjust set ?
            if ( SFX_ITEM_SET != rSet.GetItemState( RES_PARATR_ADJUST, bExportParentItemSet ) )
            {
                if ( 0 != ( pItem = rSet.GetItem( RES_PARATR_ADJUST, bExportParentItemSet ) ) )
                {
                    // then set the adjust used by the parent format
                    AttrOutput().OutputItem( *pItem );
                }
            }
        }

        if ( bPapFmt && SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_NUMRULE, bExportParentItemSet, &pItem ) )
        {
            AttrOutput().OutputItem( *pItem );

            // switch off the numerbering?
            if ( ( (SwNumRuleItem*)pItem )->GetValue().isEmpty() &&
                 SFX_ITEM_SET != rSet.GetItemState( RES_LR_SPACE, false) &&
                 SFX_ITEM_SET == rSet.GetItemState( RES_LR_SPACE, true, &pItem ) )
            {
                // the set the LR-Space of the parentformat!
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
        pISet = 0;                      // fuer Doppel-Attribute
    }
}
#include "switerator.hxx"
void MSWordExportBase::GatherChapterFields()
{
    //If the header/footer contains a chapter field
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
        //If the header/footer contains a chapter field
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
                const SwFrmFmt& rTitleFmt = pAktPageDesc->GetFirst();
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

// Da WW nur Break-After ( Pagebreak und Sectionbreaks ) kennt, im SW aber
// Bagebreaks "vor" und "nach" und Pagedescs nur "vor" existieren, werden
// die Breaks 2* durchgeklimpert, naemlich vor und hinter jeder Zeile.
// Je nach BreakTyp werden sie vor oder nach der Zeile gesetzt.
// Es duerfen nur Funktionen gerufen werden, die nicht in den
// Ausgabebereich pO schreiben, da dieser nur einmal fuer CHP und PAP existiert
// und damit im falschen landen wuerden.
void MSWordExportBase::OutputSectionBreaks( const SfxItemSet *pSet, const SwNode& rNd )
{
    if ( bStyDef || bOutKF || bInWriteEscher || bOutPageDescs )
        return;

    bBreakBefore = true;

    bool bNewPageDesc = false;
    const SfxPoolItem* pItem=0;
    const SwFmtPageDesc *pPgDesc=0;

    //Output a sectionbreak if theres a new pagedesciptor.  otherwise output a
    //pagebreak if there is a pagebreak here, unless the new page (follow
    //style) is different to the current one, in which case plump for a
    //section.
    bool bBreakSet = false;

    const SwPageDesc * pPageDesc = rNd.FindPageDesc(sal_False);

    if (pAktPageDesc != pPageDesc)
    {
        bBreakSet = true;
        bNewPageDesc = true;
        pAktPageDesc = pPageDesc;
    }

    if ( pSet && pSet->Count() )
    {
        if ( SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, false, &pItem ) &&
             dynamic_cast<const SwFmtPageDesc*>(pItem)->GetRegisteredIn() != NULL)
        {
            bBreakSet = true;
            bNewPageDesc = true;
            pPgDesc = (const SwFmtPageDesc*)pItem;
            pAktPageDesc = pPgDesc->GetPageDesc();
        }
        else if ( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, false, &pItem ) )
        {
            // Word does not like hard break attributes in some table cells
            bool bRemoveHardBreakInsideTable = false;
            if ( bOutTable )
            {
                const SwTableNode* pTableNode = rNd.FindTableNode();
                if ( pTableNode )
                {
                    const SwTableBox* pBox = rNd.GetTblBox();
                    const SwTableLine* pLine = pBox ? pBox->GetUpper() : 0;
                    // but only for non-complex tables
                    if ( pLine && !pLine->GetUpper() )
                    {
                        // check if box is not first in that line:
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
                    // #i76301# - assure that there is a page break before set at the node.
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
            {   // Even a pagedesc item is set, the break item can be set 'NONE',
                // but a pagedesc item is an implicit page break before...
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

// #i76300#
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
    // Die PageDescs werden beim Auftreten von PageDesc-Attributen nur in
    // WW8Writer::pSepx mit der entsprechenden Position eingetragen.  Das
    // Aufbauen und die Ausgabe der am PageDesc haengenden Attribute und
    // Kopf/Fusszeilen passiert nach dem Haupttext und seinen Attributen.

    sal_uLong nFcPos = ReplaceCr( msword::PageBreak ); // Page/Section-Break

    // tatsaechlich wird hier NOCH NICHTS ausgegeben, sondern
    // nur die Merk-Arrays aCps, aSects entsprechend ergaenzt
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
        // dann muss das fuer die Ausgabe korrigiert werden
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

    // nix, macht WW undokumentiert auch so
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
        // write sprmPOutLvl sprmPIlvl and sprmPIlfo
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

// #i77805#
bool WW8Export::DisallowInheritingOutlineNumbering(const SwFmt &rFmt)
{
    bool bRet( false );

    //If there is no numbering on this fmt, but its parent was outline
    //numbered, then in writer this is no inheritied, but in word it would
    //be, so we must export "no numbering" and "body level" to make word
    //behave like writer (see #i25755)
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

                //if outline numbered
                // if Write StyleDefinition then write the OutlineRule
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
                //otherwise we might have to remove outline numbering from
                //what gets exported if the parent style was outline numbered
                // #i77805#
                // If inherited outline numbering is suppress, the left/right
                // margins has to be exported explicitly.
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

                // Fly als Zeichen werden bei uns zu Absatz-gebundenen
                // jetzt den Abstand vom Absatz-Rand setzen
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
                //script doesn't matter if not exporting chp
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
    for ( SwFmtFld* pFld = aFmtFlds.First(); pFld; pFld = aFmtFlds.Next() )
    {
        if ( pFld->GetTxtFld() && nTyp == pFld->GetFld()->GetSubType() &&
             0 != ( pNd = pFld->GetTxtFld()->GetpTxtNode() ) &&
             pNd->GetNodes().IsDocNodes() )
        {
            const SwGetRefField& rRFld = *static_cast< SwGetRefField* >( pFld->GetFld() );
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
                    break;      // ???
                case REF_OUTLINE:
                    break;      // ???
            }
        }
    }

    return false;
}

String MSWordExportBase::GetBookmarkName( sal_uInt16 nTyp, const OUString* pName, sal_uInt16 nSeqNo )
{
    String sRet;
    switch ( nTyp )
    {
        case REF_SETREFATTR:
            if ( pName )
            {
                sRet.AppendAscii( "Ref_" );
                sRet += *pName;
            }
            break;
        case REF_SEQUENCEFLD:
            break;      // ???
        case REF_BOOKMARK:
            if ( pName )
                sRet = *pName;
            break;
        case REF_OUTLINE:
            break;      // ???
        case REF_FOOTNOTE:
            sRet.AppendAscii( "_RefF" );
            sRet += OUString::number( nSeqNo );
            break;
        case REF_ENDNOTE:
            sRet.AppendAscii( "_RefE" );
            sRet += OUString::number( nSeqNo );
            break;
    }
    return BookmarkToWord( sRet ); // #i43956# - encode bookmark accordingly
}

//-----------------------------------------------------------------------
/* File CHRATR.HXX: */
void WW8AttributeOutput::RTLAndCJKState( bool bIsRTL, sal_uInt16 nScript )
{
    if ( m_rWW8Export.bWrtWW8 && bIsRTL )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_CFBiDi );
        m_rWW8Export.pO->push_back( (sal_uInt8)1 );
    }

    // #i46087# patch from james_clark; complex texts needs the undocumented SPRM CComplexScript with param 0x81.
    if ( m_rWW8Export.bWrtWW8 && nScript == i18n::ScriptType::COMPLEX && !bIsRTL )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_CComplexScript );
        m_rWW8Export.pO->push_back( (sal_uInt8)0x81 );
        m_rWW8Export.pDop->bUseThaiLineBreakingRules = true;
    }
}

void WW8AttributeOutput::EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner )
{
    m_rWW8Export.pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );
    m_rWW8Export.pO->clear();

    if ( pTextNodeInfoInner.get() != NULL )
    {
        if ( pTextNodeInfoInner->isEndOfLine() )
        {
            TableRowEnd( pTextNodeInfoInner->getDepth() );

            SVBT16 nSty;
            ShortToSVBT16( 0, nSty );
            m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), (sal_uInt8*)&nSty, (sal_uInt8*)&nSty+2 );     // Style #
            TableInfoRow( pTextNodeInfoInner );
            m_rWW8Export.pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data());
            m_rWW8Export.pO->clear();
            //For Bug 119650, should break the properties of CHP PLC after a paragraph end.
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
        const String &rComment = pRedlineData->GetComment();
        //Only possible to export to main text
        if (rComment.Len() && (m_rWW8Export.nTxtTyp == TXT_MAINTEXT))
        {
            if (m_rWW8Export.pAtn->IsNewRedlineComment(pRedlineData))
            {
                m_rWW8Export.pAtn->Append( m_rWW8Export.Fc2Cp( m_rWW8Export.Strm().Tell() ), pRedlineData );
                m_rWW8Export.WritePostItBegin( m_rWW8Export.pO );
            }
        }
    }
}

void WW8AttributeOutput::EndRunProperties( const SwRedlineData* pRedlineData )
{
    Redline( pRedlineData );

    WW8_WrPlcFld* pCurrentFields = m_rWW8Export.CurrentFieldPlc();
    sal_uInt16 nNewFieldResults = pCurrentFields ? pCurrentFields->ResultCount() : 0;

    bool bExportedFieldResult = ( m_nFieldResults != nNewFieldResults );

    // If we have exported a field result, then we will have been forced to
    // split up the text into a 0x13, 0x14, <result> 0x15 sequence with the
    // properties forced out at the end of the result, so the 0x15 itself
    // should remain clean of all other attributes to avoid #iXXXXX#
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

void WW8AttributeOutput::RawText( const String& rText, bool bForceUnicode, rtl_TextEncoding eCharSet )
{
    m_rWW8Export.OutSwString( rText, 0, rText.Len(), bForceUnicode, eCharSet );
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
    m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), (sal_uInt8*)&nSty, (sal_uInt8*)&nSty+2 );     // Style #
}

void WW8AttributeOutput::OutputWW8Attribute( sal_uInt8 nId, bool bVal )
{
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( 8 == nId ? NS_sprm::LN_CFDStrike : NS_sprm::LN_CFBold + nId );
    else if (8 == nId )
        return; // no such attribute in WW6
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
    //Can only export in 8+, in 7- export as normal varient and expect that
    //upperlevel code has blocked exporting clobbering attributes
    sal_uInt16 nFontID = m_rWW8Export.GetId( rFont );
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_CFtcBi );
    else
        m_rWW8Export.pO->push_back( 93 );
    m_rWW8Export.InsUInt16( nFontID );
}

void WW8AttributeOutput::CharFontCJK( const SvxFontItem& rFont )
{
    //Can only export in 8+, in 7- export as normal varient and expect that
    //upperlevel code has blocked exporting clobbering attributes
    sal_uInt16 nFontID = m_rWW8Export.GetId( rFont );
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_CRgFtc1 );
    else
        m_rWW8Export.pO->push_back( 93 );
    m_rWW8Export.InsUInt16( nFontID );
}

void WW8AttributeOutput::CharWeightCTL( const SvxWeightItem& rWeight )
{
    //Can only export in 8+, in 7- export as normal varient and expect that
    //upperlevel code has blocked exporting clobbering attributes
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
    // Can only export in 8+, in 7- export as normal varient and expect that
    // upperlevel code has blocked exporting clobbering attributes
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

// Shadowed und Contour are not in WW-UI. JP: ??
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
        // At the moment the only animated text effect we support is blinking
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

    // otherwise both off
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
            // no such feature in word
            break;
        default:
            // otherwise both off
            OutputWW8Attribute( 5, false );
            OutputWW8Attribute( 6, false );
            return;
    }
}

void WW8AttributeOutput::CharHidden( const SvxCharHiddenItem& rHidden )
{
    OutputWW8Attribute( 7, rHidden.GetValue() );
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

    // WW95 - parameters:   0 = none,   1 = single, 2 = by Word,
    //                      3 = double, 4 = dotted, 5 = hidden
    // WW97 - additional parameters:
    //                      6 = thick,   7 = dash,       8 = dot(not used)
    //                      9 = dotdash 10 = dotdotdash, 11 = wave
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
        // ------------  new in WW2000  -------------------------------------
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
        if ( m_rWW8Export.bWrtWW8 ) // use sprmCRgLid0_80 rather than sprmCLid
            m_rWW8Export.InsUInt16( nId );
        else
            m_rWW8Export.pO->push_back( static_cast<sal_uInt8>(nId) );
        m_rWW8Export.InsUInt16( rLanguage.GetLanguage() );

        // Word 2000 and above apparently require both old and new versions of
        // these sprms to be set, without it spellchecking doesn't work
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
            // switch both flags off
            m_rWW8Export.InsUInt16( NS_sprm::LN_CFEmboss );
            m_rWW8Export.pO->push_back( (sal_uInt8)0x0 );
            m_rWW8Export.InsUInt16( NS_sprm::LN_CFImprint );
            m_rWW8Export.pO->push_back( (sal_uInt8)0x0 );
        }
    }
}

void WW8AttributeOutput::CharRotate( const SvxCharRotateItem& rRotate )
{
    // #i28331# - check that a Value is set
    if ( !rRotate.GetValue() )
        return;

    if ( m_rWW8Export.bWrtWW8 && !m_rWW8Export.IsInTable() )
    {
        // #i36867 In word the text in a table is rotated via the TC or NS_sprm::LN_TTextFlow
        // This means you can only rotate all or none of the text adding NS_sprm::LN_CEastAsianLayout
        // here corrupts the table, hence !m_rWW8Export.bIsInTable

        m_rWW8Export.InsUInt16( NS_sprm::LN_CEastAsianLayout );
        m_rWW8Export.pO->push_back( (sal_uInt8)0x06 ); //len 6
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
            // case 1:
            default:                            nVal = 1;   break;
        }

        m_rWW8Export.InsUInt16( NS_sprm::LN_CKcd );
        m_rWW8Export.pO->push_back( nVal );
    }
}

// TransCol uebersetzt SW-Farben in WW. Heraus kommt die bei WW fuer
// Text- und Hintergrundfarbe benutzte Codierung.
// Gibt es keine direkte Entsprechung, dann wird versucht, eine moeglichst
// aehnliche WW-Farbe zu finden.
// return: 5-Bit-Wert ( 0..16 )
sal_uInt8 WW8Export::TransCol( const Color& rCol )
{
    sal_uInt8 nCol = 0;      // ->Auto
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

// TransBrush uebersetzt SW-Brushes in WW. Heraus kommt WW8_SHD.
// Nicht-Standardfarben des SW werden noch nicht in die
// Misch-Werte ( 0 .. 95% ) vom WW uebersetzt.
// Return: Echte Brush ( nicht transparent )
// auch bei Transparent wird z.B. fuer Tabellen eine transparente Brush
// geliefert
bool WW8Export::TransBrush(const Color& rCol, WW8_SHD& rShd)
{
    if( rCol.GetTransparency() )
        rShd = WW8_SHD();               // alles Nullen : transparent
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
        m_rWW8Export.InsUInt16( 0x6870 );
        m_rWW8Export.InsUInt32( wwUtility::RGBToBGR( rColor.GetValue().GetColor() ) );
    }
}

void WW8AttributeOutput::CharBackground( const SvxBrushItem& rBrush )
{
    if( m_rWW8Export.bWrtWW8 )   // nur WW8 kann ZeichenHintergrund
    {
        WW8_SHD aSHD;

        m_rWW8Export.TransBrush( rBrush.GetColor(), aSHD );
        // sprmCShd
        m_rWW8Export.InsUInt16( NS_sprm::LN_CShd );
        m_rWW8Export.InsUInt16( aSHD.GetValue() );

        //Quite a few unknowns, some might be transparency or something
        //of that nature...
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
        sal_uInt16 nId;
        const OUString rStr = rINet.GetINetFmt();
        if ( !rStr.isEmpty() )
            nId = rINet.GetINetFmtId();
        else
            nId = RES_POOLCHR_INET_NORMAL;

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

// #i43956# - add optional parameter <pLinkStr>
// It's needed to write the hyperlink data for a certain cross-reference
// - it contains the name of the link target, which is a bookmark.
// add optional parameter <bIncludeEmptyPicLocation>
// It is needed to write an empty picture location for page number field separators
static void InsertSpecialChar( WW8Export& rWrt, sal_uInt8 c,
                               String* pLinkStr = 0L,
                               bool bIncludeEmptyPicLocation = false )
{
    ww::bytes aItems;
    rWrt.GetCurrentItems(aItems);

    if (c == 0x13)
        rWrt.pChpPlc->AppendFkpEntry(rWrt.Strm().Tell());
    else
        rWrt.pChpPlc->AppendFkpEntry(rWrt.Strm().Tell(), aItems.size(), aItems.data());

    rWrt.WriteChar(c);

    // store empty sprmCPicLocation for field separator
    if ( bIncludeEmptyPicLocation &&
         ( c == 0x13 || c == 0x14 || c == 0x15 ) )
    {
        SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CPicLocation );
        SwWW8Writer::InsUInt32( aItems, 0x00000000 );
    }

    // #i43956# - write hyperlink data and attributes
    if ( rWrt.bWrtWW8 && c == 0x01 && pLinkStr )
    {
        // write hyperlink data to data stream
        SvStream& rStrm = *rWrt.pDataStrm;
        // position of hyperlink data
        const sal_uInt32 nLinkPosInDataStrm = rStrm.Tell();
        // write empty header
        const sal_uInt16 nEmptyHdrLen = 0x44;
        sal_uInt8 aEmptyHeader[ nEmptyHdrLen ] = { 0 };
        aEmptyHeader[ 4 ] = 0x44;
        rStrm.Write( aEmptyHeader, nEmptyHdrLen );
        // writer fixed header
        const sal_uInt16 nFixHdrLen = 0x19;
        sal_uInt8 aFixHeader[ nFixHdrLen ] =
        {
            0x08, 0xD0, 0xC9, 0xEA, 0x79, 0xF9, 0xBA, 0xCE,
            0x11, 0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9,
            0x0B, 0x02, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00,
            0x00,
        };
        rStrm.Write( aFixHeader, nFixHdrLen );
        // write reference string including length+1
        sal_uInt32 nStrLen( pLinkStr->Len() + 1 );
        SwWW8Writer::WriteLong( rStrm, nStrLen );
        SwWW8Writer::WriteString16( rStrm, *(pLinkStr), false );
        // write additional two NULL Bytes
        SwWW8Writer::WriteLong( rStrm, 0 );
        // write length of hyperlink data
        const sal_uInt32 nCurrPos = rStrm.Tell();
        rStrm.Seek( nLinkPosInDataStrm );
        SVBT32 nLen;
        UInt32ToSVBT32( nCurrPos - nLinkPosInDataStrm, nLen );
        rStrm.Write( nLen, 4 );
        rStrm.Seek( nCurrPos );

        // write attributes of hyperlink character 0x01
        SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CFFldVanish );
        aItems.push_back( (sal_uInt8)0x81 );
        SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CPicLocation );
        SwWW8Writer::InsUInt32( aItems, nLinkPosInDataStrm );
        SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CFData );
        aItems.push_back( (sal_uInt8)0x01 );
    }

    //Technically we should probably Remove all attribs
    //here for the 0x13, 0x14, 0x15, but our import
    //is slightly lacking
    //aItems.Remove(0, aItems.Count());
    // fSpec-Attribute true
    if( rWrt.bWrtWW8 )
    {
        SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CFSpec );
        aItems.push_back( 1 );
    }
    else
    {
        aItems.push_back( 117 ); //sprmCFSpec
        aItems.push_back( 1 );
    }

    rWrt.pChpPlc->AppendFkpEntry(rWrt.Strm().Tell(), aItems.size(), aItems.data());
}

static String lcl_GetExpandedField(const SwField &rFld)
{
    String sRet(rFld.ExpandField(true));

    //replace LF 0x0A with VT 0x0B
    sRet.SearchAndReplaceAll(0x0A, 0x0B);

    return sRet;
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
    const String& rFldCmd, sal_uInt8 nMode )
{
    bool bUnicode = IsUnicode();
    WW8_WrPlcFld* pFldP = CurrentFieldPlc();

    const bool bIncludeEmptyPicLocation = ( eFldType == ww::ePAGE );

    if (WRITEFIELD_START & nMode)
    {
        sal_uInt8 aFld13[2] = { 0x13, 0x00 };  // will change
        //#i3958#, Needed to make this field work correctly in Word 2000
        if (eFldType == ww::eSHAPE)
            aFld13[0] |= 0x80;
        aFld13[1] = static_cast< sal_uInt8 >(eFldType);  // Typ nachtragen
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
        // #i43956# - write hyperlink character including
        // attributes and corresponding binary data for certain reference fields.
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
            // retrieve reference destination - the name of the bookmark
            String aLinkStr;
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
            // insert hyperlink character including attributes and data.
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
        String sOut;
        if( pFld )
            sOut = lcl_GetExpandedField(*pFld);
        else
            sOut = rFldCmd;
        if( sOut.Len() )
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
                        Set_UInt8(pArr, 0x68); //sprmCPicLocation
                        Set_UInt32(pArr, 0x0);

                        Set_UInt8( pArr, 117 ); //sprmCFSpec
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

void WW8Export::StartCommentOutput(const String& rName)
{
    String sStr(FieldString(ww::eQUOTE));
    sStr.AppendAscii("[");
    sStr += rName;
    sStr.AppendAscii("] ");
    OutputField(0, ww::eQUOTE, sStr, WRITEFIELD_START | WRITEFIELD_CMD_START);
}

void WW8Export::EndCommentOutput(const String& rName)
{
    String sStr(OUString(" ["));
    sStr += rName;
    sStr.AppendAscii("] ");
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

// return values:  1 - no PageNum,
//                 2 - TabStop before PageNum,
//                 3 - Text before PageNum - rTxt hold the text
//                 4 - no Text and no TabStop before PageNum
static int lcl_CheckForm( const SwForm& rForm, sal_uInt8 nLvl, String& rText )
{
    int nRet = 4;
    rText.Erase();

    // #i21237#
    SwFormTokens aPattern = rForm.GetPattern(nLvl);
    SwFormTokens::iterator aIt = aPattern.begin();
    bool bPgNumFnd = false;
    FormTokenType eTType;

    // #i61362#
    if (! aPattern.empty())
    {
        // #i21237#
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
                rText = aIt->sText.copy( 0, 5 ); // #i21237#
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
        // #i21237#
        SwFormTokens aPattern = rForm.GetPattern(nI);

        if ( !aPattern.empty() )
        {
            SwFormTokens::iterator aIt = aPattern.begin();

            FormTokenType eTType;

            // #i21237#
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
        String sStr;
        switch (pTOX->GetType())
        {
        case TOX_INDEX:
            eCode = ww::eINDEX;
            sStr = FieldString(eCode);

            if (pTOX->GetTOXForm().IsCommaSeparated())
                sStr.AppendAscii("\\r ");

            if (nsSwTOIOptions::TOI_ALPHA_DELIMITTER & pTOX->GetOptions())
                sStr.AppendAscii("\\h \"A\" ");

            {
                String aFillTxt;
                for (sal_uInt8 n = 1; n <= 3; ++n)
                {
                    String aTxt;
                    int nRet = ::lcl_CheckForm(pTOX->GetTOXForm(), n, aTxt);

                    if( 3 == nRet )
                        aFillTxt = aTxt;
                    else if ((4 == nRet) || (2 == nRet))
                        aFillTxt = '\t';
                    else
                        aFillTxt.Erase();
                }
                sStr.AppendAscii("\\e \"");
                sStr += aFillTxt;
                sStr.AppendAscii(sEntryEnd);
            }
            break;

        case TOX_ILLUSTRATIONS:
        case TOX_OBJECTS:
        case TOX_TABLES:
            if (!pTOX->IsFromObjectNames())
            {
                sStr = FieldString(eCode);

                sStr.AppendAscii("\\c \"");
                sStr += pTOX->GetSequenceName();
                sStr.AppendAscii(sEntryEnd);

                String aTxt;
                int nRet = ::lcl_CheckForm( pTOX->GetTOXForm(), 1, aTxt );
                if (1 == nRet)
                    sStr.AppendAscii("\\n ");
                else if( 3 == nRet || 4 == nRet )
                {
                    sStr.AppendAscii("\\p \"");
                    sStr += aTxt;
                    sStr.AppendAscii(sEntryEnd);
                }
            }
            break;

        default:
            {
                sStr = FieldString(eCode);

                String sTOption;
                sal_uInt16 n, nTOXLvl = pTOX->GetLevel();
                if( !nTOXLvl )
                    ++nTOXLvl;

                if( nsSwTOXElement::TOX_MARK & pTOX->GetCreateType() )
                {
                    sStr.AppendAscii( "\\f " );

                    if( TOX_USER == pTOX->GetType() )
                    {
                         sStr += '\"';
                         sStr += (sal_Char)( 'A' + GetExport( ).GetId( *pTOX->GetTOXType() ) );
                         sStr.AppendAscii( sEntryEnd );
                    }

                if( nsSwTOXElement::TOX_OUTLINELEVEL & pTOX->GetCreateType() )
                {
                    const int nMinLvl = nTOXLvl;

                    if ( nMinLvl > 0 )
                    {
                        int nTmpLvl = nMinLvl;
                        if (nTmpLvl > WW8ListManager::nMaxLevel)
                            nTmpLvl = WW8ListManager::nMaxLevel;

                        sStr.AppendAscii( "\\o \"1-" );
                        sStr += OUString::number( nTmpLvl );
                        sStr.AppendAscii(sEntryEnd);

                    }

                }


        if( nsSwTOXElement::TOX_OUTLINELEVEL & pTOX->GetCreateType() )
                  {
            // Take the TOC value of the max level to evaluate to as
            // the starting point for the \o flag, but reduce it to the
            // value of the highest outline level filled by a *standard*
            // Heading 1 - 9 style because \o "Builds a table of
            // contents from paragraphs formatted with built-in heading
            // styles". And afterward fill in any outline styles left
            // uncovered by that range to the \t flag
            //
            // i.e. for
            // Heading 1
            // Heading 2
            // custom-style
            // Heading 4
            // output
            // \o 1-2 \tcustom-style,3,Heading 3,4

            // Search over all the outline styles used and figure out
            // what is the minimum outline level (if any) filled by a
            // non-standard style for that level, i.e. ignore headline
            // styles 1-9 and find the lowest valid outline level
            sal_uInt8 nPosOfLowestNonStandardLvl = MAXLEVEL;
            const SwTxtFmtColls& rColls = *GetExport().pDoc->GetTxtFmtColls();
            for( n = rColls.size(); n; )
                      {
            const SwTxtFmtColl* pColl = rColls[ --n ];
            sal_uInt16 nPoolId = pColl->GetPoolFmtId();
            if (
                //Is a Non-Standard Outline Style
                (RES_POOLCOLL_HEADLINE1 > nPoolId || RES_POOLCOLL_HEADLINE9 < nPoolId) &&
                //Has a valid outline level
                (pColl->IsAssignedToListLevelOfOutlineStyle()) &&
                // Is less than the lowest known non-standard level
                (pColl->GetAssignedOutlineStyleLevel() < nPosOfLowestNonStandardLvl)
                            )
                          {
                              nPosOfLowestNonStandardLvl = ::sal::static_int_cast<sal_uInt8>(pColl->GetAssignedOutlineStyleLevel());
                          }
                      }

            sal_uInt8 nMaxMSAutoEvaluate = nPosOfLowestNonStandardLvl < nTOXLvl ? nPosOfLowestNonStandardLvl : (sal_uInt8)nTOXLvl;

            //output \o 1-X where X is the highest normal outline style to be included in the toc
            if ( nMaxMSAutoEvaluate )
              {
            if (nMaxMSAutoEvaluate > WW8ListManager::nMaxLevel)
              nMaxMSAutoEvaluate = WW8ListManager::nMaxLevel;

            sStr.AppendAscii( "\\o \"1-" );
            sStr += OUString::number( nMaxMSAutoEvaluate );
            sStr.AppendAscii(sEntryEnd);
                      }

            //collect up any other styles in the writer TOC which will
            //not already appear in the MS TOC and place then into the
            //\t option
            if( nMaxMSAutoEvaluate < nTOXLvl )
                      {
            // collect this templates into the \t otion
            for( n = rColls.size(); n;)
                          {
                const SwTxtFmtColl* pColl = rColls[ --n ];
                if (!pColl->IsAssignedToListLevelOfOutlineStyle())
                  continue;
                sal_uInt8 nTestLvl =  ::sal::static_int_cast<sal_uInt8>(pColl->GetAssignedOutlineStyleLevel());
                if (nTestLvl < nTOXLvl && nTestLvl >= nMaxMSAutoEvaluate)
                  {
                if( sTOption.Len() )
                  sTOption += ',';
                (( sTOption += pColl->GetName() ) += ',' )
                  += OUString::number( nTestLvl + 1 );
                  }
              }
              }
                  }

                if( nsSwTOXElement::TOX_TEMPLATE & pTOX->GetCreateType() )
                    // #i99641# - Consider additional styles regardless of TOX-outlinelevel
                    for( n = 0; n < MAXLEVEL; ++n )
                    {
                        const String& rStyles = pTOX->GetStyleNames( n );
                        if( rStyles.Len() )
                        {
                            sal_Int32 nPos = 0;
                            String sLvl = OUString(',');
                            sLvl += OUString::number( n + 1 );
                            do {
                                String sStyle( rStyles.GetToken( 0,
                                            TOX_STYLE_DELIMITER, nPos ));
                                if( sStyle.Len() )
                                {
                                    SwTxtFmtColl* pColl = GetExport().pDoc->FindTxtFmtCollByName(sStyle);
                                    if (!pColl || !pColl->IsAssignedToListLevelOfOutlineStyle() || pColl->GetAssignedOutlineStyleLevel() < nTOXLvl)
                                    {
                                        if( sTOption.Len() )
                                            sTOption += ',';
                                        ( sTOption += sStyle ) += sLvl;
                                    }
                                }
                            } while( -1 != nPos );
                        }
                    }

                {
                    String aFillTxt;
                    sal_uInt8 nNoPgStt = MAXLEVEL, nNoPgEnd = MAXLEVEL;
                    bool bFirstFillTxt = true, bOnlyText = true;
                    for( n = 0; n < nTOXLvl; ++n )
                    {
                        String aTxt;
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
                                    aFillTxt.Erase();
                                bFirstFillTxt = false;
                            }
                        }
                    }
                    if( MAXLEVEL != nNoPgStt )
                    {
                        if (WW8ListManager::nMaxLevel < nNoPgEnd)
                            nNoPgEnd = WW8ListManager::nMaxLevel;
                        sStr.AppendAscii( "\\n " );
                        sStr += OUString::number( nNoPgStt );
                        sStr += '-';
                        sStr += OUString::number( nNoPgEnd  );
                        sStr += ' ';
                    }
                    if( bOnlyText )
                    {
                        sStr.AppendAscii( "\\p \"" );
                        sStr += aFillTxt;
                        sStr.AppendAscii(sEntryEnd);
                    }
                }

                if( sTOption.Len() )
                {
                    sStr.AppendAscii( "\\t \"" );
                    sStr += sTOption;
                    sStr.AppendAscii(sEntryEnd);
                }

                if (lcl_IsHyperlinked(pTOX->GetTOXForm(), nTOXLvl))
                    sStr.AppendAscii("\\h");
            }
            break;
            }
        }

        if( sStr.Len() )
        {
            GetExport( ).bInWriteTOX = true;
            GetExport( ).OutputField( 0, eCode, sStr, WRITEFIELD_START | WRITEFIELD_CMD_START |
                WRITEFIELD_CMD_END );
        }
    }

    GetExport( ).bStartTOX = false;
}

void AttributeOutputBase::EndTOX( const SwSection& rSect )
{
    const SwTOXBase* pTOX = rSect.GetTOXBase();
    if ( pTOX )
    {
        ww::eField eCode = TOX_INDEX == pTOX->GetType() ? ww::eINDEX : ww::eTOC;
        GetExport( ).OutputField( 0, eCode, aEmptyStr, WRITEFIELD_CLOSE );
    }
    GetExport( ).bInWriteTOX = false;
}

bool MSWordExportBase::GetNumberFmt(const SwField& rFld, String& rStr)
{
    // Returns a date or time format string by using the US NfKeywordTable
    bool bHasFmt = false;
    SvNumberFormatter* pNFmtr = pDoc->GetNumberFormatter();
    sal_uInt32 nFmtIdx = rFld.GetFormat();
    const SvNumberformat* pNumFmt = pNFmtr->GetEntry( nFmtIdx );
    if( pNumFmt )
    {
        sal_uInt16 nLng = rFld.GetLanguage();
        LocaleDataWrapper aLocDat(pNFmtr->GetComponentContext(),
                                  LanguageTag(nLng));

        String sFmt(pNumFmt->GetMappedFormatstring(GetNfKeywordTable(),
            aLocDat));

        if (sFmt.Len())
        {
            sw::ms::SwapQuotesInField(sFmt);

            rStr.AppendAscii( "\\@\"" );
            rStr += sFmt;
            rStr.AppendAscii( "\" " );
            bHasFmt = true;
        }
    }
    return bHasFmt;
}

void AttributeOutputBase::GetNumberPara( String& rStr, const SwField& rFld )
{
    switch(rFld.GetFormat())
    {
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:
            rStr.AppendAscii( "\\*ALPHABETIC ");
            break;
        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:
            rStr.AppendAscii("\\*alphabetic ");
            break;
        case SVX_NUM_ROMAN_UPPER:
            rStr.AppendAscii("\\*ROMAN ");
            break;
        case SVX_NUM_ROMAN_LOWER:
            rStr.AppendAscii("\\*roman ");
            break;
        default:
            OSL_ENSURE(rFld.GetFormat() == SVX_NUM_ARABIC,
                "Unknown numbering type exported as default\n");
        case SVX_NUM_ARABIC:
            rStr.AppendAscii("\\*Arabic ");
            break;
        case SVX_NUM_PAGEDESC:
            //Nothing, use word's default
            break;
    }
}

void WW8Export::WritePostItBegin( ww::bytes* pOut )
{
    sal_uInt8 aArr[ 3 ];
    sal_uInt8* pArr = aArr;

    // sprmCFSpec true
    if( bWrtWW8 )
        Set_UInt16( pArr, NS_sprm::LN_CFSpec );
    else
        Set_UInt8( pArr, 117 ); //sprmCFSpec
    Set_UInt8( pArr, 1 );

    pChpPlc->AppendFkpEntry( Strm().Tell() );
    WriteChar( 0x05 );              // Annotation reference

    if( pOut )
        pOut->insert( pOut->end(), aArr, pArr );
    else
        pChpPlc->AppendFkpEntry( Strm().Tell(), static_cast< short >(pArr - aArr), aArr );
}

String FieldString(ww::eField eIndex)
{
    String sRet(OUString("  "));
    if (const char *pField = ww::GetEnglishFieldName(eIndex))
        sRet.InsertAscii(pField, 1);
    return sRet;
}

void WW8AttributeOutput::HiddenField( const SwField& rFld )
{
    String sExpand(rFld.GetPar2());

    //replace LF 0x0A with VT 0x0B
    sExpand.SearchAndReplaceAll(0x0A, 0x0B);
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

void WW8AttributeOutput::SetField( const SwField& rFld, ww::eField eType, const String& rCmd )
{
    const SwSetExpField* pSet=(const SwSetExpField*)(&rFld);
    const String &rVar = pSet->GetPar2();

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

    if (rVar.Len())
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
    return true; // expand to text?
}

void WW8AttributeOutput::RefField( const SwField &rFld, const String &rRef)
{
    String sStr( FieldString( ww::eREF ) );
    sStr.AppendAscii( "\"" );
    sStr += rRef;
    sStr.AppendAscii( "\" " );
    m_rWW8Export.OutputField( &rFld, ww::eREF, sStr, WRITEFIELD_START |
        WRITEFIELD_CMD_START | WRITEFIELD_CMD_END );
    String sVar = lcl_GetExpandedField( rFld );
    if ( sVar.Len() )
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
    String sExpand( lcl_GetExpandedField( *pFld ) );
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
    const SwField* pFld = rField.GetFld();
    String sStr;        // fuer optionale Parameter
    bool bWriteExpand = false;
    sal_uInt16 nSubType = pFld->GetSubType();

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
            sStr = FieldString(ww::eSEQ);
            sStr.AppendAscii("\"");
            sStr += pFld->GetTyp()->GetName();
            sStr.AppendAscii( "\" " );

            GetNumberPara( sStr, *pFld );
            GetExport().OutputField(pFld, ww::eSEQ, sStr);
        }
        else if (nSubType & nsSwGetSetExpType::GSE_STRING)
        {
            bool bShowAsWell = false;
            ww::eField eFieldNo;
            const SwSetExpField *pSet=(const SwSetExpField*)(pFld);
            const String &rVar = pSet->GetPar2();
            if (pSet->GetInputFlag())
            {
                sStr = FieldString(ww::eASK);
                sStr.AppendAscii("\"");
                sStr += pSet->GetPar1();
                sStr.AppendAscii( "\" " );
                sStr += pSet->GetPromptText();
                sStr.AppendAscii( " \\d " );
                sStr += rVar;
                eFieldNo = ww::eASK;
            }
            else
            {
                sStr = FieldString(ww::eSET);
                sStr += pSet->GetPar1();
                sStr.AppendAscii(" \"");
                sStr += rVar;
                sStr.AppendAscii("\" ");
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
        sStr = FieldString(ww::ePAGE);
        GetNumberPara(sStr, *pFld);
        GetExport().OutputField(pFld, ww::ePAGE, sStr);
        break;
    case RES_FILENAMEFLD:
        sStr = FieldString(ww::eFILENAME);
        if (pFld->GetFormat() == FF_PATHNAME)
            sStr.AppendAscii("\\p ");
        GetExport().OutputField(pFld, ww::eFILENAME, sStr);
        break;
    case RES_DBNAMEFLD:
        {
            sStr = FieldString(ww::eDATABASE);
            SwDBData aData = GetExport().pDoc->GetDBData();
            sStr += String(aData.sDataSource);
            sStr += DB_DELIM;
            sStr += String(aData.sCommand);
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
    case RES_DOCINFOFLD:    // Last printed, last edited,...
        if( DI_SUB_FIXED & nSubType )
            bWriteExpand = true;
        else
        {
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
                            String sFieldname = pDocInfoField->GetFieldName();
                            xub_StrLen nIndex = sFieldname.Search(':');

                            if (nIndex != sFieldname.Len())
                                sFieldname = sFieldname.Copy(nIndex + 1);

                            sStr.Insert(sQuotes);
                            sStr.Insert(sFieldname);
                            sStr.Insert(sQuotes);
                        }
                    }
                    break;
                default:
                    break;
            }

            if (eFld != ww::eNONE)
            {
                sStr.Insert(FieldString(eFld), 0);
                GetExport().OutputField(pFld, eFld, sStr);
            }
            else
                bWriteExpand = true;
        }
        break;
    case RES_DATETIMEFLD:
        if (FIXEDFLD & nSubType || !GetExport().GetNumberFmt(*pFld, sStr))
            bWriteExpand = true;
        else
        {
            ww::eField eFld = (DATEFLD & nSubType) ? ww::eDATE : ww::eTIME;
            sStr.Insert(FieldString(eFld), 0);
            GetExport().OutputField(pFld, eFld, sStr);
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
                sStr = FieldString(eFld);
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
                sStr = FieldString(eFld);
                GetExport().OutputField(pFld, eFld, sStr);
            }
            else
                bWriteExpand = true;
        }
        break;
    case RES_POSTITFLD:
        //Sadly only possible for word in main document text
        if (GetExport().nTxtTyp == TXT_MAINTEXT)
        {
            PostitField( pFld );
        }
        break;
    case RES_INPUTFLD:
        {
            const SwInputField * pInputField =
                dynamic_cast<const SwInputField *>(pFld);

            if (pInputField->isFormField())
                GetExport().DoFormText(pInputField);
            else
            {
                sStr = FieldString(ww::eFILLIN);

                sStr.AppendAscii("\"");
                sStr += pFld->GetPar2();
                sStr += '\"';

                GetExport().OutputField(pFld, ww::eFILLIN, sStr);
            }
        }
        break;
    case RES_GETREFFLD:
        {
            ww::eField eFld = ww::eNONE;
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
                        sStr = FieldString(eFld);
                        sStr += GetExport().GetBookmarkName(nSubType, &aRefName, 0);
                    }
                    switch (pFld->GetFormat())
                    {
                        case REF_NUMBER:
                            sStr.AppendAscii(" \\r");
                            break;
                        case REF_NUMBER_NO_CONTEXT:
                            sStr.AppendAscii(" \\n");
                            break;
                        case REF_NUMBER_FULL_CONTEXT:
                            sStr.AppendAscii(" \\w");
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
                    sStr = FieldString(eFld);
                    sStr += GetExport().GetBookmarkName(nSubType, 0,
                        rRFld.GetSeqNo());
                    break;
            }

            if (eFld != ww::eNONE)
            {
                switch (pFld->GetFormat())
                {
                    case REF_UPDOWN:
                        sStr.AppendAscii(" \\p");
                        break;
                    case REF_CHAPTER:
                        sStr.AppendAscii(" \\n");
                        break;
                    default:
                        break;
                }
                sStr.AppendAscii(" \\h ");       // insert hyperlink
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
            GetWhichOfScript(RES_CHRATR_FONTSIZE,nScript)))).GetHeight();;

        nHeight = (nHeight + 10) / 20; //Font Size in points;

        /*
        Divide the combined char string into its up and down part. Get the
        font size and fill in the defaults as up == half the font size and
        down == a fifth the font size
        */
        xub_StrLen nAbove = (pFld->GetPar1().getLength()+1)/2;
        sStr = FieldString(ww::eEQ);
        sStr.AppendAscii("\\o (\\s\\up ");
        sStr += OUString::number(nHeight/2);

        sStr.Append('(');
        sStr += String(pFld->GetPar1(),0,nAbove);
        sStr.AppendAscii("), \\s\\do ");
        sStr += OUString::number(nHeight/5);

        sStr.Append('(');
        sStr += String(pFld->GetPar1(),nAbove,pFld->GetPar1().getLength()-nAbove);
        sStr.AppendAscii("))");
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
            String sExpand(pFld->GetPar2());
            if (sExpand.Len())
            {
                HiddenField( *pFld );
            }
        }
        break;
    case RES_JUMPEDITFLD:
        bWriteExpand = PlaceholderField( pFld );
        break;
    case RES_MACROFLD:
        sStr.AssignAscii(" MACROBUTTON");
        sStr += pFld->GetPar1();
        sStr.SearchAndReplaceAscii("StarOffice.Standard.Modul1.", String(' '));
        sStr += String(' ');
        sStr += lcl_GetExpandedField(*pFld);
        GetExport().OutputField( pFld, ww::eMACROBUTTON, sStr );
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

// TOXMarks fehlen noch

// Detaillierte Einstellungen zur Trennung erlaubt WW nur dokumentenweise.
// Man koennte folgende Mimik einbauen: Die Werte des Style "Standard" werden,
// falls vorhanden, in die Document Properties ( DOP ) gesetzt.
// ---
// ACK.  Dieser Vorschlag passt exakt zu unserer Implementierung des Import,
// daher setze ich das gleich mal um. (KHZ, 07/15/2000)
void WW8AttributeOutput::ParaHyphenZone( const SvxHyphenZoneItem& rHyphenZone )
{
    // sprmPFNoAutoHyph
    if( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFNoAutoHyph );
    else
        m_rWW8Export.pO->push_back( 44 );

    m_rWW8Export.pO->push_back( rHyphenZone.IsHyphen() ? 0 : 1 );
}

void WW8AttributeOutput::ParaScriptSpace( const SfxBoolItem& rScriptSpace )
{
    sal_uInt16 nId = 0;
    if ( m_rWW8Export.bWrtWW8 )
        switch ( rScriptSpace.Which() )
        {
            case RES_PARATR_SCRIPTSPACE:            nId = NS_sprm::LN_PFAutoSpaceDE;   break;
            case RES_PARATR_HANGINGPUNCTUATION:     nId = NS_sprm::LN_PFOverflowPunct;   break;
            case RES_PARATR_FORBIDDEN_RULES:        nId = NS_sprm::LN_PFKinsoku;   break;
        }

    if ( nId )
    {
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( nId );
        else
            m_rWW8Export.pO->push_back( (sal_uInt8)nId );

        m_rWW8Export.pO->push_back( rScriptSpace.GetValue() ? 1 : 0 );
    }
}

void WW8AttributeOutput::ParaSnapToGrid( const SvxParaGridItem& rGrid )
{
    // sprmPFUsePgsuSettings
    // 97+ only
    if ( !m_rWW8Export.bWrtWW8 )
        return;

    m_rWW8Export.InsUInt16( NS_sprm::LN_PFUsePgsuSettings );
    m_rWW8Export.pO->push_back( rGrid.GetValue() );
}

void WW8AttributeOutput::ParaVerticalAlign( const SvxParaVertAlignItem& rAlign )
{
    // sprmPWAlignFont
    // 97+ only
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


// NoHyphen: ich habe keine Entsprechung in der SW-UI und WW-UI gefunden


// RefMark, NoLineBreakHere  fehlen noch

void WW8Export::WriteFtnBegin( const SwFmtFtn& rFtn, ww::bytes* pOutArr )
{
    ww::bytes aAttrArr;
    bool bAutoNum = !rFtn.GetNumStr().Len();    // Auto-Nummer
    if( bAutoNum )
    {
        if( bWrtWW8 )
        {
            static const sal_uInt8 aSpec[] =
            {
                0x03, 0x6a, 0, 0, 0, 0, // sprmCObjLocation
                0x55, 0x08, 1           // sprmCFSpec
            };

            aAttrArr.insert(aAttrArr.end(), aSpec, aSpec+sizeof(aSpec));
        }
        else
        {
            static sal_uInt8 const aSpec[] =
            {
                117, 1,                         // sprmCFSpec
                68, 4, 0, 0, 0, 0               // sprmCObjLocation
            };

            aAttrArr.insert(aAttrArr.end(), aSpec, aSpec+sizeof(aSpec));
        }
    }

    // sprmCIstd
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

                                                // fSpec-Attribut true
                            // Fuer Auto-Nummer muss ein Spezial-Zeichen
                            // in den Text und darum ein fSpec-Attribut
    pChpPlc->AppendFkpEntry( Strm().Tell() );
    if( bAutoNum )
        WriteChar( 0x02 );              // Auto-Nummer-Zeichen
    else
        // User-Nummerierung
        OutSwString( rFtn.GetNumStr(), 0, rFtn.GetNumStr().Len(),
                        IsUnicode(), RTL_TEXTENCODING_MS_1252 );

    if( pOutArr )
    {
        // insert at start of array, so the "hard" attribute overrule the
        // attributes of the character template
        pOutArr->insert( pOutArr->begin(), aAttrArr.begin(), aAttrArr.end() );
    }
    else
    {
        ww::bytes aOutArr;

        // insert at start of array, so the "hard" attribute overrule the
        // attributes of the character template
        aOutArr.insert( aOutArr.begin(), aAttrArr.begin(), aAttrArr.end() );

        // write for the ftn number in the content, the font of the anchor
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
            bRet = false;   // the is ftn/end collected at Page- or Doc-End
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

    // if any reference to this footnote/endnote then insert an internal
    // Bookmark.
    String sBkmkNm;
    if ( GetExport().HasRefToObject( nTyp, 0, rFtn.GetTxtFtn()->GetSeqRefNo() ))
    {
        sBkmkNm = GetExport().GetBookmarkName( nTyp, 0,
                                    rFtn.GetTxtFtn()->GetSeqRefNo() );
        GetExport().AppendBookmark( sBkmkNm );
    }

    TextFootnote_Impl( rFtn );

    if ( sBkmkNm.Len() )
        GetExport().AppendBookmark( sBkmkNm ); // FIXME: Why is it added twice?  Shouldn't this one go to WW8AttributeOuput::TextFootnote_Impl()?
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
    // #i28331# - check that bOn is set
    if ( rTwoLines.GetValue() )
    {
        //97+ only
        if( !m_rWW8Export.bWrtWW8 )
            return;

        m_rWW8Export.InsUInt16( NS_sprm::LN_CEastAsianLayout );
        m_rWW8Export.pO->push_back( (sal_uInt8)0x06 ); //len 6
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
                        nLvl = static_cast< sal_uInt8 >(pTxtNd->GetActualListLevel());

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
                        // #i44815# adjust numbering for numbered paragraphs
                        // without number (NO_NUMLEVEL). These paragaphs
                        // will receive a list id 0, which WW interprets as
                        // 'no number'.
                        nNumId = 0;
                    }
                }
                else if ( GetExport().pOutFmtNode->ISA( SwTxtFmtColl ) )
                {
                    const SwTxtFmtColl* pC = (SwTxtFmtColl*)GetExport().pOutFmtNode;
                    if ( pC && pC->IsAssignedToListLevelOfOutlineStyle() )
                        nLvl = static_cast< sal_uInt8 >( pC->GetAssignedOutlineStyleLevel() ); //<-end,zhaojianwei
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
        // write sprmPIlvl and sprmPIlfo
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_PIlvl );
        m_rWW8Export.pO->push_back( ::sal::static_int_cast<sal_uInt8>(nLvl) );
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_PIlfo );
        SwWW8Writer::InsUInt16( *m_rWW8Export.pO, ::sal::static_int_cast<sal_uInt16>(nNumId) );
    }
    else if ( pTxtNd && m_rWW8Export.Out_SwNum( pTxtNd ) )                // NumRules
        m_rWW8Export.pSepx->SetNum( pTxtNd );
}

/* File FRMATR.HXX */

void WW8AttributeOutput::FormatFrameSize( const SwFmtFrmSize& rSize )
{
    if( m_rWW8Export.bOutFlyFrmAttrs )                   // Flys
    {
        if( m_rWW8Export.bOutGrf )
            return;                // Fly um Grafik -> Auto-Groesse

//???? was ist bei Prozentangaben ???
        if ( rSize.GetWidth() && rSize.GetWidthSizeType() == ATT_FIX_SIZE)
        {
            //"sprmPDxaWidth"
            if( m_rWW8Export.bWrtWW8 )
                m_rWW8Export.InsUInt16( NS_sprm::LN_PDxaWidth );
            else
                m_rWW8Export.pO->push_back( 28 );
            m_rWW8Export.InsUInt16( (sal_uInt16)rSize.GetWidth() );
        }

        if ( rSize.GetHeight() )
        {
            // sprmPWHeightAbs
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
    else if( m_rWW8Export.bOutPageDescs )            // PageDesc : Breite + Hoehe
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

// FillOrder fehlt noch

// ReplaceCr() wird fuer Pagebreaks und Pagedescs gebraucht. Es wird ein
// bereits geschriebenes CR durch ein Break-Zeichen ersetzt. Replace muss
// direkt nach Schreiben des CR gerufen werden.
// Rueckgabe: FilePos des ersetzten CRs + 1 oder 0 fuer nicht ersetzt

sal_uLong WW8Export::ReplaceCr( sal_uInt8 nChar )
{
    OSL_ENSURE( nChar, "gegen 0 ersetzt bringt WW97/95 zum Absturz" );

    bool bReplaced = false;
    SvStream& rStrm = Strm();
    sal_uLong nRetPos = 0, nPos = rStrm.Tell();
    //If there is at least two characters already output
    if (nPos - (IsUnicode() ? 2 : 1) >= sal_uLong(pFib->fcMin))
    {
        sal_uInt8 nBCode=0;
        sal_uInt16 nUCode=0;

        rStrm.SeekRel(IsUnicode() ? -2 : -1);
        if (IsUnicode())
            rStrm >> nUCode;
        else
        {
            rStrm >> nBCode;
            nUCode = nBCode;
        }
        //If the last char was a cr
        if (nUCode == 0x0d)             // CR ?
        {
            if ((nChar == 0x0c) &&
                (nPos - (IsUnicode() ? 4 : 2) >= sal_uLong(pFib->fcMin)))
            {
                rStrm.SeekRel( IsUnicode() ? -4 : -2 );
                if (IsUnicode())
                    rStrm >> nUCode;
                else
                {
                    rStrm >> nUCode;
                    nUCode = nBCode;
                }
            }
            else
            {
                rStrm.SeekRel( IsUnicode() ? -2 : -1 );
                nUCode = 0x0;
            }
            //And the para is not of len 0, then replace this cr with the mark
            //#120140# If there is a cr before a column break, need replace the cr. So remove the "nChar==0x0e" check.
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
            // a column break after a section has no effect in writer
            bReplaced = true;
        }
        rStrm.Seek( nPos );
    }
    else
        bReplaced = true;

    if (!bReplaced)
    {
        // then write as normal char
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

    //Technically in a word document this is a different value for a row ends
    //that are not row ends directly after a cell with a graphic. But it
    //doesn't seem to make a difference
    //pMagicTable->Append(Fc2Cp(Strm().Tell()),0x1B6);
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
    // sprmPPageBreakBefore/sprmPFPageBreakBefore
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFPageBreakBefore );
    else
        m_rWW8Export.pO->push_back( 9 );

    m_rWW8Export.pO->push_back( bBreak ? 1 : 0 );
}

// Breaks schreiben nichts in das Ausgabe-Feld rWrt.pO,
// sondern nur in den Text-Stream ( Bedingung dafuer, dass sie von Out_Break...
// gerufen werden duerfen )
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
        // #i76300# - Note: Can only be <true>, if <bBefore> equals <false>.
        bool bCheckForFollowPageDesc = false;

        switch ( rBreak.GetBreak() )
        {
            case SVX_BREAK_NONE:                                // Ausgeschaltet
                if ( !GetExport().bBreakBefore )
                    PageBreakBefore( false );
                return;

            case SVX_BREAK_COLUMN_BEFORE:                       // ColumnBreak
                bBefore = true;
                // no break;
            case SVX_BREAK_COLUMN_AFTER:
            case SVX_BREAK_COLUMN_BOTH:
                if ( GetExport().Sections().CurrentNumberOfColumns( *GetExport().pDoc ) > 1 )
                {
                    nC = msword::ColumnBreak;
                }
                break;

            case SVX_BREAK_PAGE_BEFORE:                         // PageBreak
                // From now on(fix for #i77900#) we prefer to save a page break as
                // paragraph attribute, this has to be done after the export of the
                // paragraph ( => !GetExport().bBreakBefore )
                if ( !GetExport().bBreakBefore )
                    PageBreakBefore( true );
                break;

            case SVX_BREAK_PAGE_AFTER:
            case SVX_BREAK_PAGE_BOTH:
                nC = msword::PageBreak;
                // #i76300# - check for follow page description,
                // if current writing attributes of a paragraph.
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
            // #i76300#
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
    const SwFmt * pSwFmt = pStyles->GetSwFmt();

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
            case 0: nVal = 15;  break;      // Automatically select
            case 1: nVal = 1;   break;      // Upper paper tray
            case 2: nVal = 4;   break;      // Manual paper feed
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
    // Flys fehlen noch ( siehe RTF )

    if ( m_rWW8Export.bOutFlyFrmAttrs )                   // Flys
    {
        // sprmPDxaFromText10
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PDxaFromText10 );
        else
            m_rWW8Export.pO->push_back( 49 );
        // Mittelwert nehmen, da WW nur 1 Wert kennt
        m_rWW8Export.InsUInt16( (sal_uInt16) ( ( rLR.GetLeft() + rLR.GetRight() ) / 2 ) );
    }
    else if ( m_rWW8Export.bOutPageDescs )                // PageDescs
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

        // sprmSDxaLeft
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_SDxaLeft );
        else
            m_rWW8Export.pO->push_back( 166 );
        m_rWW8Export.InsUInt16( nLDist );

        // sprmSDxaRight
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_SDxaRight );
        else
            m_rWW8Export.pO->push_back( 167 );
        m_rWW8Export.InsUInt16( nRDist );
    }
    else
    {                                          // normale Absaetze
        // sprmPDxaLeft
        if( m_rWW8Export.bWrtWW8 )
        {
            m_rWW8Export.InsUInt16( 0x845E );        //asian version ?
            m_rWW8Export.InsUInt16( (sal_uInt16)rLR.GetTxtLeft() );
        }
        else
        {
            m_rWW8Export.pO->push_back( 17 );
            m_rWW8Export.InsUInt16( (sal_uInt16)rLR.GetTxtLeft() );
        }

        // sprmPDxaRight
        if( m_rWW8Export.bWrtWW8 )
        {
            m_rWW8Export.InsUInt16( 0x845D );        //asian version ?
            m_rWW8Export.InsUInt16( (sal_uInt16)rLR.GetRight() );
        }
        else
        {
            m_rWW8Export.pO->push_back( 16 );
            m_rWW8Export.InsUInt16( (sal_uInt16)rLR.GetRight() );
        }

        // sprmPDxaLeft1
        if( m_rWW8Export.bWrtWW8 )
        {
            m_rWW8Export.InsUInt16( 0x8460 );        //asian version ?
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
    // Flys fehlen noch ( siehe RTF )

    if ( m_rWW8Export.bOutFlyFrmAttrs )                   // Flys
    {
        // sprmPDyaFromText
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaFromText );
        else
            m_rWW8Export.pO->push_back( 48 );
        // Mittelwert nehmen, da WW nur 1 Wert kennt
        m_rWW8Export.InsUInt16( (sal_uInt16) ( ( rUL.GetUpper() + rUL.GetLower() ) / 2 ) );
    }
    else if ( m_rWW8Export.bOutPageDescs )            // Page-UL
    {
        OSL_ENSURE( m_rWW8Export.GetCurItemSet(), "Impossible" );
        if ( !m_rWW8Export.GetCurItemSet() )
            return;

        HdFtDistanceGlue aDistances( *m_rWW8Export.GetCurItemSet() );

        if ( aDistances.HasHeader() )
        {
            //sprmSDyaHdrTop
            if ( m_rWW8Export.bWrtWW8 )
                m_rWW8Export.InsUInt16( NS_sprm::LN_SDyaHdrTop );
            else
                m_rWW8Export.pO->push_back( 156 );
            m_rWW8Export.InsUInt16( aDistances.dyaHdrTop );
        }

        // sprmSDyaTop
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_SDyaTop );
        else
            m_rWW8Export.pO->push_back( 168 );
        m_rWW8Export.InsUInt16( aDistances.dyaTop );

        if ( aDistances.HasFooter() )
        {
            //sprmSDyaHdrBottom
            if ( m_rWW8Export.bWrtWW8 )
                m_rWW8Export.InsUInt16( NS_sprm::LN_SDyaHdrBottom );
            else
                m_rWW8Export.pO->push_back( 157 );
            m_rWW8Export.InsUInt16( aDistances.dyaHdrBottom );
        }

        //sprmSDyaBottom
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_SDyaBottom );
        else
            m_rWW8Export.pO->push_back( 169 );
        m_rWW8Export.InsUInt16( aDistances.dyaBottom );
    }
    else
    {
        // sprmPDyaBefore
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaBefore );
        else
            m_rWW8Export.pO->push_back( 21 );
        m_rWW8Export.InsUInt16( rUL.GetUpper() );
        // sprmPDyaAfter
        if( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaAfter );
        else
            m_rWW8Export.pO->push_back( 22 );
        m_rWW8Export.InsUInt16( rUL.GetLower() );
        // sprmPFContextualSpacing
        if (m_rWW8Export.bWrtWW8 && rUL.GetContext())
        {
            m_rWW8Export.InsUInt16(NS_sprm::LN_PContextualSpacing);
            m_rWW8Export.pO->push_back( (sal_uInt8)rUL.GetContext() );
        }
    }
}

// Print, Opaque, Protect fehlen noch

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
//!!!! Ankertyp und entsprechende Umrechnung fehlt noch

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

        // sprmPDyaAbs
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

//!!!! Ankertyp und entsprechende Umrechnung fehlt noch
    if ( m_rWW8Export.bOutFlyFrmAttrs )
    {
        short nPos;
        switch( rFlyHori.GetHoriOrient() )
        {
            case text::HoriOrientation::NONE:
                nPos = (short)rFlyHori.GetPos();
                if( !nPos )
                    nPos = 1;   // WW: 0 ist reserviert
                break;
            case text::HoriOrientation::LEFT:
                nPos = rFlyHori.IsPosToggle() ? -12 : 0;
                break;
            case text::HoriOrientation::RIGHT:
                nPos = rFlyHori.IsPosToggle() ? -16 : -8;
                break;
            case text::HoriOrientation::CENTER:
            case text::HoriOrientation::FULL:                         // FULL nur fuer Tabellen
            default:
                nPos = -4;
                break;
        }

        // sprmPDxaAbs
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
                // Vert: Page | Horz: Page
                nP |= (1 << 4) | (2 << 6);
                break;
            // Im Fall eine Flys als Zeichen: Absatz-gebunden setzen!!!
            case FLY_AT_FLY:
            case FLY_AT_CHAR:
            case FLY_AT_PARA:
            case FLY_AS_CHAR:
                // Vert: Page | Horz: Page
                nP |= (2 << 4) | (0 << 6);
                break;
            default:
                break;
        }

        // sprmPPc
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PPc );
        else
            m_rWW8Export.pO->push_back( 29 );
        m_rWW8Export.pO->push_back( nP );
    }
}

void WW8AttributeOutput::FormatBackground( const SvxBrushItem& rBrush )
{
    // WW cannot have background in a section
    if ( !m_rWW8Export.bOutPageDescs )
    {
        WW8_SHD aSHD;

        m_rWW8Export.TransBrush( rBrush.GetColor(), aSHD );
        // sprmPShd
        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( NS_sprm::LN_PShd );
        else
            m_rWW8Export.pO->push_back(47);
        m_rWW8Export.InsUInt16( aSHD.GetValue() );

        // Quite a few unknowns, some might be transparency or something
        // of that nature...
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
    // M.M. This function writes out border lines to the word format similar to
    // what SwRTFWriter::OutRTFBorder does in the RTF filter Eventually it
    // would be nice if all this functionality was in the one place
    WW8_BRC aBrc;
    sal_uInt16 nWidth = ::editeng::ConvertBorderWidthToWord(
            rLine.GetBorderLineStyle(), rLine.GetWidth());
    sal_uInt8 brcType = 0, nColCode = 0;

    if( nWidth )                                // Linie ?
    {
        // BRC.brcType
        bool bThick = !rLine.isDouble() && !bWrtWW8 && nWidth > 75;
        if( bThick )
            brcType = 2;
        else
        {
            brcType = 0;
            if ( bWrtWW8 )
            {
                // All the border types values are available on
                // http://msdn.microsoft.com/en-us/library/dd908142%28v=office.12%29.aspx
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

        // BRC.dxpLineWidth
        if( bThick )
            nWidth /= 2;

        if( bWrtWW8 )
        {
            // Angabe in 8tel Punkten, also durch 2.5, da 1 Punkt = 20 Twips
            nWidth = (( nWidth * 8 ) + 10 ) / 20;
            if( 0xff < nWidth )
                nWidth = 0xff;
        }
        else
        {
            // Angabe in 0.75 pt
            nWidth = ( nWidth + 7 ) / 15;
            if( nWidth > 5 )
                nWidth = 5;
            ::editeng::SvxBorderStyle const eStyle(rLine.GetBorderLineStyle());
            if (table::BorderLineStyle::DOTTED == eStyle)
                nWidth = 6;
            else if (table::BorderLineStyle::DASHED == eStyle)
                nWidth = 7;
        }

        if( 0 == nWidth )                       // ganz duenne Linie
            nWidth = 1;                         //       nicht weglassen

        // BRC.ico
        nColCode = TransCol( rLine.GetColor() );
    }

    // BRC.dxpSpace
    sal_uInt16 nLDist = nDist;
    nLDist /= 20;               // Masseinheit : pt
    if( nLDist > 0x1f )
        nLDist = 0x1f;

    if( bWrtWW8 )
    {
        aBrc.aBits1[0] = sal_uInt8(nWidth);
        aBrc.aBits1[1] = brcType;
        aBrc.aBits2[0] = nColCode;
        aBrc.aBits2[1] = sal_uInt8(nLDist);

        // fShadow, keine weiteren Einstellungen im WW moeglich
        if( bShadow )
            aBrc.aBits2[1] |= 0x20;
    }
    else
    {
        sal_uInt16 aBits = nWidth + ( brcType << 3 );
        aBits |= (nColCode & 0x1f) << 6;
        aBits |= nLDist << 11;
        // fShadow, keine weiteren Einstellungen im WW moeglich
        if( bShadow )
            aBits |= 0x20;
        ShortToSVBT16( aBits, aBrc.aBits1);
    }

    return aBrc;
}

// MakeBorderLine() bekommt einen WW8Bytes* uebergeben, um die Funktion
// auch fuer die Tabellen-Umrandungen zu benutzen.
// Wenn nSprmNo == 0, dann wird der Opcode nicht ausgegeben.
// bShadow darf bei Tabellenzellen *nicht* gesetzt sein !
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
        // WW97-SprmIds
        if ( nSprmNo != 0 )
            SwWW8Writer::InsUInt16( rO, nSprmNo );

        rO.insert( rO.end(), aBrc.aBits1, aBrc.aBits1+2 );
        rO.insert( rO.end(), aBrc.aBits2, aBrc.aBits2+2 );
    }
    else
    {
        // WW95-SprmIds
        if ( nSprmNo != 0 )
            rO.push_back( static_cast<sal_uInt8>( nSprmNo ) );
        rO.insert( rO.end(), aBrc.aBits1, aBrc.aBits1+2 );
    }
}

// FormatBox1() ist fuer alle Boxen ausser in Tabellen.
// es wird pO des WW8Writers genommen
void WW8Export::Out_SwFmtBox(const SvxBoxItem& rBox, bool bShadow)
{
    if ( bOutPageDescs && !bWrtWW8 )
        return; // no page ouline in WW6

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

// FormatBox2() ist fuer TC-Strukturen in Tabellen. Der Sprm-Opcode
// wird nicht geschrieben, da es in der TC-Structur ohne Opcode gepackt ist.
// dxpSpace wird immer 0, da WW das in Tabellen so verlangt
// ( Tabellenumrandungen fransen sonst aus )
// Ein WW8Bytes-Ptr wird als Ausgabe-Parameter uebergeben

void WW8Export::Out_SwFmtTableBox( ww::bytes& rO, const SvxBoxItem * pBox )
{
    // moeglich und vielleicht besser waere 0xffff
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
    // Fly um Grafik-> keine Umrandung hier, da
    // der GrafikHeader bereits die Umrandung hat
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
    // CColumns
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_SCcolumns );
    else
        m_rWW8Export.pO->push_back( 144 );
    m_rWW8Export.InsUInt16( nCols - 1 );

    // DxaColumns
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_SDxaColumns );
    else
        m_rWW8Export.pO->push_back( 145 );
    m_rWW8Export.InsUInt16( rCol.GetGutterWidth( true ) );

    // LBetween
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_SLBetween );
    else
        m_rWW8Export.pO->push_back( 158 );
    m_rWW8Export.pO->push_back( COLADJ_NONE == rCol.GetLineAdj(  )? 0 : 1 );

    const SwColumns & rColumns = rCol.GetColumns(  );

    // FEvenlySpaced
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_SFEvenlySpaced );
    else
        m_rWW8Export.pO->push_back( 138 );
    m_rWW8Export.pO->push_back( bEven ? 1 : 0 );

    if ( !bEven )
    {
        for ( sal_uInt16 n = 0; n < nCols; ++n )
        {
            //sprmSDxaColWidth
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
                //sprmSDxaColSpacing
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
        // dann besorge mal die Seitenbreite ohne Raender !!

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
            //i120133: The Section width should consider page indent value.
            nPageSize -= rCol.GetAdjustValue();

        }

        // Nachsehen, ob alle Spalten gleich sind
        bool bEven = true;
        sal_uInt16 n;
        sal_uInt16 nColWidth = rCol.CalcPrtColWidth( 0, (sal_uInt16)nPageSize );
        for ( n = 1; n < nCols; n++ )
        {
            short nDiff = nColWidth -
                rCol.CalcPrtColWidth( n, (sal_uInt16)nPageSize );

            if ( nDiff > 10 || nDiff < -10 )      // Toleranz: 10 tw
            {
                bEven = false;
                break;
            }
        }

        FormatColumns_Impl( nCols, rCol, bEven, nPageSize );
    }
}

// "Paragraphs together"
void WW8AttributeOutput::FormatKeep( const SvxFmtKeepItem& rKeep )
{
    // sprmFKeepFollow
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFKeepFollow );
    else
        m_rWW8Export.pO->push_back( 8 );

    m_rWW8Export.pO->push_back( rKeep.GetValue() ? 1 : 0 );
}

// exclude a paragraph from Line Numbering
void WW8AttributeOutput::FormatLineNumbering( const SwFmtLineNumber& rNumbering )
{
    // sprmPFNoLineNumb
    if( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFNoLineNumb );
    else
        m_rWW8Export.pO->push_back( 14 );

    m_rWW8Export.pO->push_back( rNumbering.IsCount() ? 0 : 1 );
}


/* File PARATR.HXX  */

void WW8AttributeOutput::ParaLineSpacing_Impl( short nSpace, short nMulti )
{
    // sprmPDyaLine
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
        case SVX_LINE_SPACE_AUTO:
        case SVX_LINE_SPACE_FIX:
        case SVX_LINE_SPACE_MIN:
        {
            switch ( rSpacing.GetInterLineSpaceRule() )
            {
                case SVX_INTER_LINE_SPACE_FIX:      // unser Durchschuss
                {
                    // gibt es aber nicht in WW - also wie kommt man an
                    // die MaxLineHeight heran?
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
                break;
            case SVX_INTER_LINE_SPACE_PROP:
                nSpace = (short)( ( 240L * rSpacing.GetPropLineSpace() ) / 100L );
                nMulti = 1;
                break;
            default:                    // z.B. Minimum oder FIX?
                if ( SVX_LINE_SPACE_FIX == rSpacing.GetLineSpaceRule() )
                    nSpace = -(short)rSpacing.GetLineHeight();
                else
                    nSpace = (short)rSpacing.GetLineHeight();
                break;
            }
        }
        break;
    }
    // if nSpace is negative, it is a fixed size in 1/20 of a point
    // if nSpace is positive and nMulti is 1, it is 1/240 of a single line height
    // otherwise, I have no clue what the heck it is
    ParaLineSpacing_Impl( nSpace, nMulti );
}

void WW8AttributeOutput::ParaAdjust( const SvxAdjustItem& rAdjust )
{
    // sprmPJc
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
            return;    // not a supported Attribut
    }

    if ( 255 != nAdj )        // supported Attribut?
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
            m_rWW8Export.InsUInt16( NS_sprm::LN_PJcExtra ); //bidi version ?
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
    if ( !m_rWW8Export.bWrtWW8 )   //8+ only
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
            if ( m_rWW8Export.bOutFlyFrmAttrs )  //frame
            {
                nDir = m_rWW8Export.TrueFrameDirection(
                    *(const SwFrmFmt*)m_rWW8Export.pOutFmtNode );
            }
            else if ( m_rWW8Export.pOutFmtNode->ISA( SwCntntNode ) )   //pagagraph
            {
                const SwCntntNode* pNd =
                    (const SwCntntNode*)m_rWW8Export.pOutFmtNode;
                SwPosition aPos( *pNd );
                nDir = m_rWW8Export.pDoc->GetTextDirection( aPos );
            }
            else if ( m_rWW8Export.pOutFmtNode->ISA( SwTxtFmtColl ) )
                nDir = FRMDIR_HORI_LEFT_TOP;    //what else can we do :-(
        }

        if ( nDir == FRMDIR_ENVIRONMENT )
            nDir = FRMDIR_HORI_LEFT_TOP;    //Set something
    }

    switch ( nDir )
    {
        default:
            //Can't get an unknown type here
            OSL_FAIL("Unknown frame direction");
        case FRMDIR_HORI_LEFT_TOP:
            nTextFlow = 0;
            break;
        case FRMDIR_HORI_RIGHT_TOP:
            nTextFlow = 0;
            bBiDi = true;
            break;
        case FRMDIR_VERT_TOP_LEFT:  //word doesn't have this
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
    else if ( !m_rWW8Export.bOutFlyFrmAttrs )  //paragraph/style
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFBiDi );
        m_rWW8Export.pO->push_back( bBiDi );
    }
}

void WW8AttributeOutput::ParaGrabBag(const SfxGrabBagItem& /*rItem*/)
{
}

// "Separate paragraphs"
void WW8AttributeOutput::ParaSplit( const SvxFmtSplitItem& rSplit )
{
    // sprmPFKeep
    if ( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFKeep );
    else
        m_rWW8Export.pO->push_back( 7 );
    m_rWW8Export.pO->push_back( rSplit.GetValue() ? 0 : 1 );
}

//  Es wird nur das Item "SvxWidowItem" und nicht die Orphans uebersetzt,
//  da es fuer beides im WW nur ein Attribut "Absatzkontrolle" gibt und
//  im SW wahrscheinlich vom Anwender immer Beide oder keiner gesetzt werden.
void WW8AttributeOutput::ParaWidows( const SvxWidowsItem& rWidows )
{
// sprmPFWidowControl
    if( m_rWW8Export.bWrtWW8 )
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFWidowControl );
    else
        m_rWW8Export.pO->push_back( 51 );
    m_rWW8Export.pO->push_back( rWidows.GetValue() ? 1 : 0 );
}


class SwWW8WrTabu
{
    sal_uInt8* pDel;                 // DelArray
    sal_uInt8* pAddPos;              // AddPos-Array
    sal_uInt8* pAddTyp;              // AddTyp-Array
    sal_uInt16 nAdd;            // so viele Tabs kommen hinzu
    sal_uInt16 nDel;            // so viele Tabs fallen weg
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

// Add( const SvxTabStop & rTS ) fuegt einen Tab in die WW-Struktur ein
void SwWW8WrTabu::Add(const SvxTabStop & rTS, long nAdjustment)
{
    // Tab-Position eintragen
    ShortToSVBT16(msword_cast<sal_Int16>(rTS.GetTabPos() + nAdjustment),
        pAddPos + (nAdd * 2));

    // Tab-Typ eintragen
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
        case '.':   // dotted leader
            nPara |= 1 << 3;
            break;
        case '_':   // Single line leader
            nPara |= 3 << 3;
            break;
        case '-':   // hyphenated leader
            nPara |= 2 << 3;
            break;
        case '=':   // heavy line leader
            nPara |= 4 << 3;
            break;
    }

    ByteToSVBT8(nPara, pAddTyp + nAdd);
    ++nAdd;
}

// Del( const SvxTabStop & rTS ) fuegt einen zu loeschenden Tab
// in die WW-Struktur ein
void SwWW8WrTabu::Del(const SvxTabStop &rTS, long nAdjustment)
{
    // Tab-Position eintragen
    ShortToSVBT16(msword_cast<sal_Int16>(rTS.GetTabPos() + nAdjustment),
        pDel + (nDel * 2));
    ++nDel;
}

//  PutAll( WW8Export& rWW8Wrt ) schreibt das Attribut nach rWrt.pO
void SwWW8WrTabu::PutAll(WW8Export& rWrt)
{
    if (!nAdd && !nDel) //It its a no-op
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
    // cch eintragen
    rWrt.pO->push_back(msword_cast<sal_uInt8>(nSiz));
    // DelArr schreiben
    rWrt.pO->push_back(msword_cast<sal_uInt8>(nDel));
    rWrt.OutSprmBytes(pDel, nDel * 2);
    // InsArr schreiben
    rWrt.pO->push_back(msword_cast<sal_uInt8>(nAdd));
    rWrt.OutSprmBytes(pAddPos, 2 * nAdd);         // AddPosArray
    rWrt.OutSprmBytes(pAddTyp, nAdd);             // AddTypArray
}


static void ParaTabStopAdd( WW8Export& rWrt,
                            const SvxTabStopItem& rTStops,
                            const long nLParaMgn )
{
    SwWW8WrTabu aTab( 0, rTStops.Count());

    for( sal_uInt16 n = 0; n < rTStops.Count(); n++ )
    {
        const SvxTabStop& rTS = rTStops[n];
        // Def-Tabs ignorieren
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

    sal_uInt16 nO = 0;      // rTStyle Index
    sal_uInt16 nN = 0;      // rTNew Index

    do {
        const SvxTabStop* pTO;
        long nOP;
        if( nO < rTStyle.Count() )                  // alt noch nicht am Ende ?
        {
            pTO = &rTStyle[ nO ];
            nOP = pTO->GetTabPos() + nLStypeMgn;
            if( SVX_TAB_ADJUST_DEFAULT == pTO->GetAdjustment() )
            {
                nO++;                                // Default-Tab ignorieren
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
        if( nN < rTNew.Count() )                    // neu noch nicht am Ende
        {
            pTN = &rTNew[ nN ];
            nNP = pTN->GetTabPos() + nLParaMgn;
            if( SVX_TAB_ADJUST_DEFAULT == pTN->GetAdjustment() )
            {
                nN++;                               // Default-Tab ignorieren
                continue;
            }
        }
        else
        {
            pTN = 0;
            nNP = LONG_MAX;
        }

        if( nOP == LONG_MAX && nNP == LONG_MAX )
            break;                                  // alles fertig

        if( nOP < nNP )                             // naechster Tab ist alt
        {
            aTab.Del(*pTO, nLStypeMgn);             // muss geloescht werden
            nO++;
        }
        else if( nNP < nOP )                        // naechster Tab ist neu
        {
            aTab.Add(*pTN, nLParaMgn);              // muss eigefuegt werden
            nN++;
        }
        else if (lcl_IsEqual(nOP, *pTO, nNP, *pTN)) // Tabs sind gleich:
        {
            nO++;                                   // nichts zu tun
            nN++;
        }
        else                                        // Tabs selbe Pos, diff Typ
        {
            aTab.Del(*pTO, nLStypeMgn);             // alten loeschen
            aTab.Add(*pTN, nLParaMgn);              // neuen einfuegen
            nO++;
            nN++;
        }
    } while( 1 );

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

    // #i100264#
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

        // #i120938# - consider left indentation of style and its parent style
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

//-----------------------------------------------------------------------

void AttributeOutputBase::OutputItem( const SfxPoolItem& rHt )
{
    // FIXME maybe use 'item_cast', like 'item_cast<SvxCharHiddenItem>( rHt )'?
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

        case RES_TXTATR_INETFMT:
            TextINetFormat( static_cast< const SwFmtINetFmt& >( rHt ) );
            break;
        case RES_TXTATR_CHARFMT:
            TextCharFormat( static_cast< const SwFmtCharFmt& >( rHt ) );
            break;
        case RES_TXTATR_FIELD:
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

        default:
            SAL_INFO("sw.ww8", "Unhandled SfxPoolItem with id " << rHt.Which() );
            break;
    }
}

void AttributeOutputBase::OutputStyleItemSet( const SfxItemSet& rSet, sal_Bool bDeep, sal_Bool bTestForDefault )
{
    // based on OutputItemSet() from wrt_fn.cxx

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
