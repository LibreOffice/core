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
#include <IDocumentSettingAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
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
#include <svx/unobrushitemhelper.hxx>
#include <svx/xenum.hxx>
#include <tgrditem.hxx>
#include <flddropdown.hxx>
#include <chpfld.hxx>
#include <fmthdft.hxx>
#include <authfld.hxx>

#include <sprmids.hxx>

#include <fmtcntnt.hxx>
#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include "wrtww8.hxx"
#include "ww8par.hxx"
#include "ww8attributeoutput.hxx"
#include "fields.hxx"
#include <vcl/outdev.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/fltrcfg.hxx>
#include <o3tl/enumrange.hxx>

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
            default:
                break;
        }
    }
    else if ( nScript == i18n::ScriptType::COMPLEX )
    {
        //Complex is ok in ww8, but for ww6 there is only
        //one font, one fontsize, one fontsize (weight/posture)
        //and only one language
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
            default:
                break;
        }
    }
    return bRet;
}

//  Hilfsroutinen fuer Styles

void MSWordExportBase::ExportPoolItemsToCHP( ww8::PoolItems &rItems, sal_uInt16 nScript )
{
    ww8::cPoolItemIter aEnd = rItems.end();
    for ( ww8::cPoolItemIter aI = rItems.begin(); aI != aEnd; ++aI )
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

void MSWordExportBase::OutputItemSet( const SfxItemSet& rSet, bool bPapFormat, bool bChpFormat, sal_uInt16 nScript,
                                      bool bExportParentItemSet )
{
    if( bExportParentItemSet || rSet.Count() )
    {
        const SfxPoolItem* pItem;
        m_pISet = &rSet;                  // fuer Doppel-Attribute

        // If frame dir is set, but not adjust, then force adjust as well
        if ( bPapFormat && SfxItemState::SET == rSet.GetItemState( RES_FRAMEDIR, bExportParentItemSet ) )
        {
            // No explicit adjust set ?
            if ( SfxItemState::SET != rSet.GetItemState( RES_PARATR_ADJUST, bExportParentItemSet ) )
            {
                if ( nullptr != ( pItem = rSet.GetItem( RES_PARATR_ADJUST, bExportParentItemSet ) ) )
                {
                    // then set the adjust used by the parent format
                    AttrOutput().OutputItem( *pItem );
                }
            }
        }

        if ( bPapFormat && SfxItemState::SET == rSet.GetItemState( RES_PARATR_NUMRULE, bExportParentItemSet, &pItem ) )
        {
            AttrOutput().OutputItem( *pItem );

            // switch off the numerbering?
            if ( static_cast<const SwNumRuleItem*>(pItem)->GetValue().isEmpty() &&
                 SfxItemState::SET != rSet.GetItemState( RES_LR_SPACE, false) &&
                 SfxItemState::SET == rSet.GetItemState( RES_LR_SPACE, true, &pItem ) )
            {
                // the set the LR-Space of the parentformat!
                AttrOutput().OutputItem( *pItem );
            }
        }

        ww8::PoolItems aItems;
        GetPoolItems( rSet, aItems, bExportParentItemSet );
        if ( bChpFormat )
            ExportPoolItemsToCHP(aItems, nScript);
        if ( bPapFormat )
        {
            ww8::cPoolItemIter aEnd = aItems.end();
            for ( ww8::cPoolItemIter aI = aItems.begin(); aI != aEnd; ++aI )
            {
                pItem = aI->second;
                sal_uInt16 nWhich = pItem->Which();
                // Handle fill attributes just like frame attributes for now.
                if ( (nWhich >= RES_PARATR_BEGIN && nWhich < RES_FRMATR_END && nWhich != RES_PARATR_NUMRULE ) ||
                     (nWhich >= XATTR_FILL_FIRST && nWhich < XATTR_FILL_LAST))
                    AttrOutput().OutputItem( *pItem );
            }

            // Has to be called after RES_PARATR_GRABBAG is processed.
            const XFillStyleItem* pXFillStyleItem(static_cast<const XFillStyleItem*>(rSet.GetItem(XATTR_FILLSTYLE)));
            if (pXFillStyleItem && pXFillStyleItem->GetValue() == drawing::FillStyle_SOLID && !rSet.HasItem(RES_BACKGROUND))
            {
                // Construct an SvxBrushItem, as expected by the exporters.
                AttrOutput().OutputItem(getSvxBrushItemFromSourceSet(rSet, RES_BACKGROUND));
            }
        }
        m_pISet = nullptr;                      // fuer Doppel-Attribute
    }
}

#include <calbck.hxx>

void MSWordExportBase::GatherChapterFields()
{
    //If the header/footer contains a chapter field
    SwFieldType* pType = m_pDoc->getIDocumentFieldsAccess().GetSysFieldType( RES_CHAPTERFLD );
    SwIterator<SwFormatField,SwFieldType> aFormatFields( *pType );
    for ( SwFormatField* pField = aFormatFields.First(); pField; pField = aFormatFields.Next() )
    {
        if (const SwTextField *pTextField = pField->GetTextField())
        {
            const SwTextNode &rTextNode = pTextField->GetTextNode();
            m_aChapterFieldLocs.push_back(rTextNode.GetIndex());
        }
    }
}

bool MSWordExportBase::ContentContainsChapterField(const SwFormatContent &rContent) const
{
    bool bRet = false;
    if ( const SwNodeIndex* pSttIdx = rContent.GetContentIdx() )
    {
        SwNodeIndex aIdx( *pSttIdx, 1 );
        SwNodeIndex aEnd( *pSttIdx->GetNode().EndOfSectionNode() );
        sal_uLong nStart = aIdx.GetIndex();
        sal_uLong nEnd = aEnd.GetIndex();
        //If the header/footer contains a chapter field
        mycCFIter aIEnd = m_aChapterFieldLocs.end();
        for ( mycCFIter aI = m_aChapterFieldLocs.begin(); aI != aIEnd; ++aI )
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

bool MSWordExportBase::FormatHdFtContainsChapterField(const SwFrameFormat &rFormat) const
{
    if ( m_aChapterFieldLocs.empty() )
        return false;

    const SwFrameFormat *pFormat = nullptr;

    pFormat = rFormat.GetHeader().GetHeaderFormat();
    if ( pFormat && ContentContainsChapterField( pFormat->GetContent() ) )
        return true;

    pFormat = rFormat.GetFooter().GetFooterFormat();
    if ( pFormat && ContentContainsChapterField( pFormat->GetContent() ) )
        return true;

    return false;
}

bool MSWordExportBase::SetAktPageDescFromNode(const SwNode &rNd)
{
    bool bNewPageDesc = false;
    const SwPageDesc* pCurrent = SwPageDesc::GetPageDescOfNode(rNd);
    OSL_ENSURE(pCurrent && m_pAktPageDesc, "Not possible surely");
    if (m_pAktPageDesc && pCurrent)
    {
        if (pCurrent != m_pAktPageDesc)
        {
            if (m_pAktPageDesc->GetFollow() != pCurrent)
                bNewPageDesc = true;
            else
            {
                const SwFrameFormat& rTitleFormat = m_pAktPageDesc->GetFirstMaster();
                const SwFrameFormat& rFollowFormat = pCurrent->GetMaster();

                bNewPageDesc = !IsPlausableSingleWordSection(rTitleFormat,
                    rFollowFormat);
            }
            m_pAktPageDesc = pCurrent;
        }
        else
        {
            const SwFrameFormat &rFormat = pCurrent->GetMaster();
            bNewPageDesc = FormatHdFtContainsChapterField(rFormat);
        }
    }
    return bNewPageDesc;
}

// Da WW nur Break-After ( Pagebreak und Sectionbreaks ) kennt, im SW aber
// Pagebreaks "vor" und "nach" und Pagedescs nur "vor" existieren, werden
// die Breaks 2* durchgeklimpert, naemlich vor und hinter jeder Zeile.
// Je nach BreakTyp werden sie vor oder nach der Zeile gesetzt.
// Es duerfen nur Funktionen gerufen werden, die nicht in den
// Ausgabebereich pO schreiben, da dieser nur einmal fuer CHP und PAP existiert
// und damit im falschen landen wuerden.
void MSWordExportBase::OutputSectionBreaks( const SfxItemSet *pSet, const SwNode& rNd, bool isCellOpen, bool isTextNodeEmpty)
{
    if ( m_bStyDef || m_bOutKF || m_bInWriteEscher || m_bOutPageDescs )
        return;

    m_bBreakBefore = true;
    bool bNewPageDesc = false;
    const SfxPoolItem* pItem=nullptr;
    const SwFormatPageDesc *pPgDesc=nullptr;

    //Output a sectionbreak if theres a new pagedesciptor.  otherwise output a
    //pagebreak if there is a pagebreak here, unless the new page (follow
    //style) is different to the current one, in which case plump for a
    //section.
    bool bBreakSet = false;

    const SwPageDesc * pPageDesc = rNd.FindPageDesc(false);

    // Even if pAktPageDesc != pPageDesc ,it might be because of the different header & footer types.
    if (m_pAktPageDesc != pPageDesc)
    {
        if ( ( isCellOpen && ( m_pAktPageDesc->GetName() != pPageDesc->GetName() )) ||
             ( isTextNodeEmpty || m_bPrevTextNodeIsEmpty ))
        {
            /* Do not output a section break in the following scenarios.
                1) Table cell is open and page header types are different
                2) PageBreak is present but text node has no string - it is an empty node.
                3) If the previous node was an empty text node and current node is a non empty text node or vice versa.
                4) If previous node and current node both are empty text nodes.
                Converting a page break to section break would cause serious issues while importing
                the RT files with different first page being set.
            */
            bNewPageDesc = false;

            /*
             * If Table cell is open and page header types are different
             * set pSet to NULL as we don't want to add any section breaks.
             */
            if ( isCellOpen && ( m_pAktPageDesc->GetName() != pPageDesc->GetName() ) )
                pSet = nullptr;
        }
        else if (!sw::util::IsPlausableSingleWordSection(m_pAktPageDesc->GetFirstMaster(), pPageDesc->GetMaster()))
        {
            bBreakSet = true;
            bNewPageDesc = true;
            m_pAktPageDesc = pPageDesc;
        }
    }

    if ( pSet && pSet->Count() )
    {
        if ( SfxItemState::SET == pSet->GetItemState( RES_PAGEDESC, false, &pItem ) &&
             static_cast<const SwFormatPageDesc*>(pItem)->GetRegisteredIn() != nullptr)
        {
            bBreakSet = true;
            bNewPageDesc = true;
            pPgDesc = static_cast<const SwFormatPageDesc*>(pItem);
            m_pAktPageDesc = pPgDesc->GetPageDesc();
        }
        else if ( SfxItemState::SET == pSet->GetItemState( RES_BREAK, false, &pItem ) )
        {
            // Word does not like hard break attributes in some table cells
            bool bRemoveHardBreakInsideTable = false;
            if ( m_bOutTable )
            {
                const SwTableNode* pTableNode = rNd.FindTableNode();
                if ( pTableNode )
                {
                    const SwTableBox* pBox = rNd.GetTableBox();
                    const SwTableLine* pLine = pBox ? pBox->GetUpper() : nullptr;
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
            bNewPageDesc = false; // if next node has RES_BREAK(page break) then bNewPageDesc value should be false.
            bBreakSet = true;

            if ( !bRemoveHardBreakInsideTable )
            {
                OSL_ENSURE(m_pAktPageDesc, "should not be possible");
                /*
                 If because of this pagebreak the page desc following the page
                 break is the follow style of the current page desc then output a
                 section break using that style instead.  At least in those cases
                 we end up with the same style in word and writer, nothing can be
                 done when it happens when we get a new pagedesc because we
                 overflow from the first page style.
                */
                if ( m_pAktPageDesc )
                {
                    // #i76301# - assure that there is a page break before set at the node.
                    const SvxFormatBreakItem* pBreak = dynamic_cast<const SvxFormatBreakItem*>(pItem);
                    if ( pBreak &&
                         pBreak->GetBreak() == SVX_BREAK_PAGE_BEFORE )
                    {
                        bNewPageDesc |= SetAktPageDescFromNode( rNd );
                    }
                    if( isTextNodeEmpty )
                       bNewPageDesc = false;
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
        if ( const SwContentNode *pNd = rNd.GetContentNode() )
        {
            const SvxFormatBreakItem &rBreak =
                ItemGet<SvxFormatBreakItem>( *pNd, RES_BREAK );
            if ( rBreak.GetBreak() == SVX_BREAK_PAGE_BEFORE )
                bHackInBreak = true;
            else
            {   // Even a pagedesc item is set, the break item can be set 'NONE',
                // but a pagedesc item is an implicit page break before...
                const SwFormatPageDesc &rPageDesc =
                    ItemGet<SwFormatPageDesc>( *pNd, RES_PAGEDESC );
                if ( rPageDesc.KnowsPageDesc() )
                    bHackInBreak = true;
            }
        }
    }

    if ( bHackInBreak )
    {
        OSL_ENSURE( m_pAktPageDesc, "should not be possible" );
        if ( m_pAktPageDesc )
            bNewPageDesc = SetAktPageDescFromNode( rNd );
    }

    if ( bNewPageDesc && m_pAktPageDesc )
    {
        PrepareNewPageDesc( pSet, rNd, pPgDesc, m_pAktPageDesc );
    }
    m_bBreakBefore = false;
    m_bPrevTextNodeIsEmpty = isTextNodeEmpty ;
}

// #i76300#
bool MSWordExportBase::OutputFollowPageDesc( const SfxItemSet* pSet, const SwTextNode* pNd )
{
    bool bRet = false;

    if ( pNd &&
         m_pAktPageDesc &&
         m_pAktPageDesc != m_pAktPageDesc->GetFollow() )
    {
        PrepareNewPageDesc( pSet, *pNd, nullptr, m_pAktPageDesc->GetFollow() );
        bRet = true;
    }

    return bRet;
}

const SwSectionFormat* MSWordExportBase::GetSectionFormat( const SwNode& rNd )
{
    const SwSectionFormat* pFormat = nullptr;
    const SwSectionNode* pSect = rNd.FindSectionNode();
    if ( pSect &&
         CONTENT_SECTION == pSect->GetSection().GetType() )
    {
        pFormat = pSect->GetSection().GetFormat();
    }

    return pFormat;
}

sal_uLong MSWordExportBase::GetSectionLineNo( const SfxItemSet* pSet, const SwNode& rNd )
{
    const SwFormatLineNumber* pNItem = nullptr;
    if ( pSet )
    {
        pNItem = &( ItemGet<SwFormatLineNumber>( *pSet, RES_LINENUMBER ) );
    }
    else if ( const SwContentNode *pNd = rNd.GetContentNode() )
    {
        pNItem = &( ItemGet<SwFormatLineNumber>( *pNd, RES_LINENUMBER ) );
    }

    return pNItem? pNItem->GetStartValue() : 0;
}

void WW8Export::PrepareNewPageDesc( const SfxItemSet*pSet,
                                      const SwNode& rNd,
                                      const SwFormatPageDesc* pNewPgDescFormat,
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

    const SwSectionFormat* pFormat = GetSectionFormat( rNd );
    const sal_uLong nLnNm = GetSectionLineNo( pSet, rNd );

    OSL_ENSURE( pNewPgDescFormat || pNewPgDesc, "Neither page desc format nor page desc provided." );

    if ( pNewPgDescFormat )
    {
        pSepx->AppendSep( Fc2Cp( nFcPos ), *pNewPgDescFormat, rNd, pFormat, nLnNm );
    }
    else if ( pNewPgDesc )
    {
        pSepx->AppendSep( Fc2Cp( nFcPos ), pNewPgDesc, rNd, pFormat, nLnNm );
    }
}

void MSWordExportBase::CorrectTabStopInSet( SfxItemSet& rSet, sal_uInt16 nAbsLeft )
{
    if (const SvxTabStopItem *pItem = rSet.GetItem<SvxTabStopItem>(RES_PARATR_TABSTOP))
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

void WW8AttributeOutput::OutlineNumbering(sal_uInt8 nLvl)
{
    if ( nLvl >= WW8ListManager::nMaxLevel )
        nLvl = WW8ListManager::nMaxLevel-1;

    // write sprmPOutLvl sprmPIlvl and sprmPIlfo
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_POutLvl );
    m_rWW8Export.pO->push_back( nLvl );
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_PIlvl );
    m_rWW8Export.pO->push_back( nLvl );
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_PIlfo );
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO,
            1 + m_rWW8Export.GetId( *m_rWW8Export.m_pDoc->GetOutlineNumRule() ) );
}

// #i77805#
bool WW8Export::DisallowInheritingOutlineNumbering(const SwFormat &rFormat)
{
    bool bRet( false );

    //If there is no numbering on this fmt, but its parent was outline
    //numbered, then in writer this is no inheritied, but in word it would
    //be, so we must export "no numbering" and "body level" to make word
    //behave like writer (see #i25755)
    if (SfxItemState::SET != rFormat.GetItemState(RES_PARATR_NUMRULE, false))
    {
        if (const SwFormat *pParent = rFormat.DerivedFrom())
        {
            if (static_cast<const SwTextFormatColl*>(pParent)->IsAssignedToListLevelOfOutlineStyle())
            {
                SwWW8Writer::InsUInt16(*pO, NS_sprm::LN_POutLvl);
                pO->push_back(sal_uInt8(9));
                SwWW8Writer::InsUInt16(*pO, NS_sprm::LN_PIlfo);
                SwWW8Writer::InsUInt16(*pO, 0);

                bRet = true;
            }
        }
    }

    return bRet;
}

void MSWordExportBase::OutputFormat( const SwFormat& rFormat, bool bPapFormat, bool bChpFormat, bool bFlyFormat )
{
    bool bCallOutSet = true;
    const SwModify* pOldMod = m_pOutFormatNode;
    m_pOutFormatNode = &rFormat;

    switch( rFormat.Which() )
    {
    case RES_CONDTXTFMTCOLL:
    case RES_TXTFMTCOLL:
        if( bPapFormat )
        {
            int nLvl = MAXLEVEL;

            if (static_cast<const SwTextFormatColl&>(rFormat).IsAssignedToListLevelOfOutlineStyle())
                nLvl = static_cast<const SwTextFormatColl&>(rFormat).GetAssignedOutlineStyleLevel();

            if (nLvl >= 0 && nLvl < MAXLEVEL)
            {
                //if outline numbered
                // if Write StyleDefinition then write the OutlineRule
                const SwNumFormat& rNFormat = m_pDoc->GetOutlineNumRule()->Get( static_cast<sal_uInt16>( nLvl ) );
                if ( m_bStyDef )
                    AttrOutput().OutlineNumbering(static_cast<sal_uInt8>(nLvl));

                if ( rNFormat.GetPositionAndSpaceMode() ==
                                           SvxNumberFormat::LABEL_WIDTH_AND_POSITION  &&
                     rNFormat.GetAbsLSpace() )
                {
                    SfxItemSet aSet( rFormat.GetAttrSet() );
                    SvxLRSpaceItem aLR(
                        ItemGet<SvxLRSpaceItem>(aSet, RES_LR_SPACE));

                    aLR.SetTextLeft( aLR.GetTextLeft() + rNFormat.GetAbsLSpace() );
                    aLR.SetTextFirstLineOfst( GetWordFirstLineOffset(rNFormat));

                    aSet.Put( aLR );
                    CorrectTabStopInSet( aSet, rNFormat.GetAbsLSpace() );
                    OutputItemSet( aSet, bPapFormat, bChpFormat,
                        i18n::ScriptType::LATIN, m_bExportModeRTF);
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
                if ( m_bStyDef && DisallowInheritingOutlineNumbering(rFormat) )
                {
                    SfxItemSet aSet( rFormat.GetAttrSet() );
                    SvxLRSpaceItem aLR(
                        ItemGet<SvxLRSpaceItem>(aSet, RES_LR_SPACE));
                    aSet.Put( aLR );
                    OutputItemSet( aSet, bPapFormat, bChpFormat,
                        css::i18n::ScriptType::LATIN, m_bExportModeRTF);
                    bCallOutSet = false;
                }
            }
        }
        break;

    case RES_CHRFMT:
        break;
    case RES_FLYFRMFMT:
        if (bFlyFormat)
        {
            OSL_ENSURE(m_pParentFrame, "No parent frame, all broken");

            if (m_pParentFrame)
            {
                const SwFrameFormat &rFrameFormat = m_pParentFrame->GetFrameFormat();

                SfxItemSet aSet(m_pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                    RES_FRMATR_END-1,
                    XATTR_FILL_FIRST, XATTR_FILL_LAST,
                    0);
                aSet.Set(rFrameFormat.GetAttrSet());

                // Fly als Zeichen werden bei uns zu Absatz-gebundenen
                // jetzt den Abstand vom Absatz-Rand setzen
                if (m_pFlyOffset)
                {
                    aSet.Put(SwFormatHoriOrient(m_pFlyOffset->X()));
                    aSet.Put(SwFormatVertOrient(m_pFlyOffset->Y()));
                    SwFormatAnchor aAnchor(rFrameFormat.GetAnchor());
                    aAnchor.SetType(m_eNewAnchorType);
                    aSet.Put(aAnchor);
                }

                if (SfxItemState::SET != aSet.GetItemState(RES_SURROUND))
                    aSet.Put(SwFormatSurround(SURROUND_NONE));

                const XFillStyleItem* pXFillStyleItem(static_cast< const XFillStyleItem*  >(rFrameFormat.GetAttrSet().GetItem(XATTR_FILLSTYLE)));
                if (pXFillStyleItem)
                {
                    switch (pXFillStyleItem->GetValue())
                    {
                    case drawing::FillStyle_NONE:
                        break;
                    case drawing::FillStyle_SOLID:
                    {
                        // Construct an SvxBrushItem, as expected by the exporters.
                        aSet.Put(getSvxBrushItemFromSourceSet(rFrameFormat.GetAttrSet(), RES_BACKGROUND));
                        break;
                    }
                    default:
                        break;
                    }
                }

                m_bOutFlyFrameAttrs = true;
                //script doesn't matter if not exporting chp
                OutputItemSet(aSet, true, false,
                    i18n::ScriptType::LATIN, m_bExportModeRTF);
                m_bOutFlyFrameAttrs = false;

                bCallOutSet = false;
            }
        }
        break;
    case RES_FRMFMT:
        break;
    default:
        OSL_ENSURE( false, "Which format is exported here?" );
        break;
    }

    if( bCallOutSet )
        OutputItemSet( rFormat.GetAttrSet(), bPapFormat, bChpFormat,
            i18n::ScriptType::LATIN, m_bExportModeRTF);
    m_pOutFormatNode = pOldMod;
}

bool MSWordExportBase::HasRefToObject( sal_uInt16 nTyp, const OUString* pName, sal_uInt16 nSeqNo )
{

    SwFieldType* pType = m_pDoc->getIDocumentFieldsAccess().GetSysFieldType( RES_GETREFFLD );
    SwIterator<SwFormatField, SwFieldType> aFormatFields( *pType );
    for ( SwFormatField* pFormatField = aFormatFields.First(); pFormatField; pFormatField = aFormatFields.Next() )
    {
        const SwTextNode* pNd;
        if ( pFormatField->GetTextField() && nTyp == pFormatField->GetField()->GetSubType() &&
             nullptr != ( pNd  = pFormatField->GetTextField()->GetpTextNode() ) &&
             pNd->GetNodes().IsDocNodes() )
        {
            const SwGetRefField& rRField = *static_cast< SwGetRefField* >( pFormatField->GetField() );
            switch ( nTyp )
            {
                case REF_BOOKMARK:
                case REF_SETREFATTR:
                    if ( pName && *pName == rRField.GetSetRefName() )
                        return true;
                    break;
                case REF_FOOTNOTE:
                case REF_ENDNOTE:
                    if ( nSeqNo == rRField.GetSeqNo() )
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
            break;      // ???
        case REF_BOOKMARK:
            if ( pName )
                sRet = *pName;
            break;
        case REF_OUTLINE:
            break;      // ???
        case REF_FOOTNOTE:
            sRet += "_RefF";
            sRet += OUString::number( nSeqNo );
            break;
        case REF_ENDNOTE:
            sRet += "_RefE";
            sRet += OUString::number( nSeqNo );
            break;
    }
    return BookmarkToWord( sRet ); // #i43956# - encode bookmark accordingly
}

/* File CHRATR.HXX: */
void WW8AttributeOutput::RTLAndCJKState( bool bIsRTL, sal_uInt16 nScript )
{
    if (bIsRTL)
    {
        if( m_rWW8Export.m_pDoc->GetDocumentType() != SwDoc::DOCTYPE_MSWORD )
        {
            m_rWW8Export.InsUInt16( NS_sprm::LN_CFBiDi );
            m_rWW8Export.pO->push_back( (sal_uInt8)1 );
        }
    }

    // #i46087# patch from james_clark; complex texts needs the undocumented SPRM CComplexScript with param 0x81.
    if (nScript == i18n::ScriptType::COMPLEX && !bIsRTL)
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_CComplexScript );
        m_rWW8Export.pO->push_back( (sal_uInt8)0x81 );
        m_rWW8Export.pDop->bUseThaiLineBreakingRules = true;
    }
}

void WW8AttributeOutput::EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner )
{
    m_rWW8Export.m_pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell() - (mbOnTOXEnding?2:0), m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );
    mbOnTOXEnding = false;
    m_rWW8Export.pO->clear();

    if ( pTextNodeInfoInner.get() != nullptr )
    {
        if ( pTextNodeInfoInner->isEndOfLine() )
        {
            TableRowEnd( pTextNodeInfoInner->getDepth() );

            SVBT16 nSty;
            ShortToSVBT16( 0, nSty );
            m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), nSty, nSty+2 );     // Style #
            TableInfoRow( pTextNodeInfoInner );
            m_rWW8Export.m_pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data());
            m_rWW8Export.pO->clear();
        }
    }
}

void WW8AttributeOutput::StartRunProperties()
{
    WW8_WrPlcField* pCurrentFields = m_rWW8Export.CurrentFieldPlc();
    m_nFieldResults = pCurrentFields ? pCurrentFields->ResultCount() : 0;
}

void WW8AttributeOutput::StartRun( const SwRedlineData* pRedlineData, bool /*bSingleEmptyRun*/ )
{
    if (pRedlineData)
    {
        const OUString &rComment = pRedlineData->GetComment();
        //Only possible to export to main text
        if (!rComment.isEmpty() && (m_rWW8Export.m_nTextTyp == TXT_MAINTEXT))
        {
            if (m_rWW8Export.m_pAtn->IsNewRedlineComment(pRedlineData))
            {
                m_rWW8Export.m_pAtn->Append( m_rWW8Export.Fc2Cp( m_rWW8Export.Strm().Tell() ), pRedlineData );
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

    WW8_WrPlcField* pCurrentFields = m_rWW8Export.CurrentFieldPlc();
    sal_uInt16 nNewFieldResults = pCurrentFields ? pCurrentFields->ResultCount() : 0;

    bool bExportedFieldResult = ( m_nFieldResults != nNewFieldResults );

    // If we have exported a field result, then we will have been forced to
    // split up the text into a 0x13, 0x14, <result> 0x15 sequence with the
    // properties forced out at the end of the result, so the 0x15 itself
    // should remain clean of all other attributes to avoid #iXXXXX#
    if ( !bExportedFieldResult )
    {
        m_rWW8Export.m_pChpPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(),
                m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );
    }
    m_rWW8Export.pO->clear();
}

void WW8AttributeOutput::RunText( const OUString& rText, rtl_TextEncoding eCharSet )
{
    RawText(rText, eCharSet);
}

void WW8AttributeOutput::RawText(const OUString& rText, rtl_TextEncoding)
{
    m_rWW8Export.OutSwString(rText, 0, rText.getLength());
}

void WW8AttributeOutput::OutputFKP(bool bForce)
{
    if (!m_rWW8Export.pO->empty() || bForce)
    {
        m_rWW8Export.m_pChpPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(),
                m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );
        m_rWW8Export.pO->clear();
    }
}

void WW8AttributeOutput::ParagraphStyle( sal_uInt16 nStyle )
{
    OSL_ENSURE( m_rWW8Export.pO->empty(), " pO ist am ZeilenEnde nicht leer" );

    SVBT16 nSty;
    ShortToSVBT16( nStyle, nSty );
    m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), nSty, nSty+2 );     // Style #
}

void WW8AttributeOutput::OutputWW8Attribute( sal_uInt8 nId, bool bVal )
{
    m_rWW8Export.InsUInt16( 8 == nId ? NS_sprm::LN_CFDStrike : NS_sprm::LN_CFBold + nId );

    m_rWW8Export.pO->push_back( bVal ? 1 : 0 );
}

void WW8AttributeOutput::OutputWW8AttributeCTL( sal_uInt8 nId, bool bVal )
{
    OSL_ENSURE( nId <= 1, "out of range" );
    if (nId > 1)
        return;

    m_rWW8Export.InsUInt16( NS_sprm::LN_CFBoldBi + nId );
    m_rWW8Export.pO->push_back( bVal ? 1 : 0 );
}

void WW8AttributeOutput::CharFont( const SvxFontItem& rFont )
{
    sal_uInt16 nFontID = m_rWW8Export.GetId( rFont );

    m_rWW8Export.InsUInt16( NS_sprm::LN_CRgFtc0 );
    m_rWW8Export.InsUInt16( nFontID );
    m_rWW8Export.InsUInt16( NS_sprm::LN_CRgFtc2 );

    m_rWW8Export.InsUInt16( nFontID );
}

void WW8AttributeOutput::CharFontCTL( const SvxFontItem& rFont )
{
    sal_uInt16 nFontID = m_rWW8Export.GetId( rFont );
    m_rWW8Export.InsUInt16( NS_sprm::LN_CFtcBi );
    m_rWW8Export.InsUInt16( nFontID );
}

void WW8AttributeOutput::CharFontCJK( const SvxFontItem& rFont )
{
    sal_uInt16 nFontID = m_rWW8Export.GetId( rFont );
    m_rWW8Export.InsUInt16( NS_sprm::LN_CRgFtc1 );
    m_rWW8Export.InsUInt16( nFontID );
}

void WW8AttributeOutput::CharWeightCTL( const SvxWeightItem& rWeight )
{
    OutputWW8AttributeCTL( 0, WEIGHT_BOLD == rWeight.GetWeight());
}

void WW8AttributeOutput::CharPostureCTL( const SvxPostureItem& rPosture )
{
    OutputWW8AttributeCTL( 1, ITALIC_NONE != rPosture.GetPosture() );
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
    OutputWW8Attribute( 3, rContour.GetValue() );
}

void WW8AttributeOutput::CharShadow( const SvxShadowedItem& rShadow )
{
    OutputWW8Attribute( 4, rShadow.GetValue() );
}

void WW8AttributeOutput::CharKerning( const SvxKerningItem& rKerning )
{
    m_rWW8Export.InsUInt16( NS_sprm::LN_CDxaSpace );

    m_rWW8Export.InsUInt16( rKerning.GetValue() );
}

void WW8AttributeOutput::CharAutoKern( const SvxAutoKernItem& rAutoKern )
{
    m_rWW8Export.InsUInt16( NS_sprm::LN_CHpsKern );

    m_rWW8Export.InsUInt16( rAutoKern.GetValue() ? 1 : 0 );
}

void WW8AttributeOutput::CharAnimatedText( const SvxBlinkItem& rBlink )
{
    m_rWW8Export.InsUInt16( NS_sprm::LN_CSfxText );
    // At the moment the only animated text effect we support is blinking
    m_rWW8Export.pO->push_back( rBlink.GetValue() ? 2 : 0 );
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

void WW8AttributeOutput::CharBorder( const SvxBorderLine* pAllBorder, const sal_uInt16 /*nDist*/, const bool bShadow )
{
    WW8Export::Out_BorderLine( *m_rWW8Export.pO, pAllBorder, 0, NS_sprm::LN_CBrc80, NS_sprm::LN_CBrc, bShadow );
}

void WW8AttributeOutput::CharHighlight( const SvxBrushItem& rBrush )
{
    if (rBrush.GetColor() != COL_TRANSPARENT)
    {
        sal_uInt8 nColor = msfilter::util::TransColToIco( rBrush.GetColor() );
        // sprmCHighlight
        m_rWW8Export.InsUInt16( NS_sprm::LN_CHighlight );
        m_rWW8Export.pO->push_back( nColor );
    }
}

void WW8AttributeOutput::CharUnderline( const SvxUnderlineItem& rUnderline )
{
    m_rWW8Export.InsUInt16( NS_sprm::LN_CKul );

    const SfxPoolItem* pItem = m_rWW8Export.HasItem( RES_CHRATR_WORDLINEMODE );
    bool bWord = false;
    if (pItem)
        bWord = static_cast<const SvxWordLineModeItem*>(pItem)->GetValue();

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
            b = 6;
            break;
        case UNDERLINE_DOUBLE:
            b = 3;
            break;
        case UNDERLINE_DOTTED:
            b = 4;
            break;
        case UNDERLINE_DASH:
            b = 7;
            break;
        case UNDERLINE_DASHDOT:
            b = 9;
            break;
        case UNDERLINE_DASHDOTDOT:
            b = 10;
            break;
        case UNDERLINE_WAVE:
            b = 11;
            break;
        // new in WW2000
        case UNDERLINE_BOLDDOTTED:
            b = 20;
            break;
        case UNDERLINE_BOLDDASH:
            b = 23;
            break;
        case UNDERLINE_LONGDASH:
            b = 39;
            break;
        case UNDERLINE_BOLDLONGDASH:
            b = 55;
            break;
        case UNDERLINE_BOLDDASHDOT:
            b = 25;
            break;
        case UNDERLINE_BOLDDASHDOTDOT:
            b = 26;
            break;
        case UNDERLINE_BOLDWAVE:
            b = 27;
            break;
        case UNDERLINE_DOUBLEWAVE:
            b = 43;
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
        m_rWW8Export.InsUInt16( NS_sprm::LN_CCvUl );

        m_rWW8Export.InsUInt32( wwUtility::RGBToBGR( aColor.GetColor() ) );
    }
}

void WW8AttributeOutput::CharLanguage( const SvxLanguageItem& rLanguage )
{
    sal_uInt16 nId = 0;
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

    if ( nId )
    {
        // use sprmCRgLid0_80 rather than sprmCLid
        m_rWW8Export.InsUInt16( nId );
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
        m_rWW8Export.InsUInt16( NS_sprm::LN_CIss );

        m_rWW8Export.pO->push_back( b );
    }

    if ( 0 == b || 0xFF == b )
    {
        long nHeight = static_cast<const SvxFontHeightItem&>(m_rWW8Export.GetItem(
                                    RES_CHRATR_FONTSIZE )).GetHeight();
        m_rWW8Export.InsUInt16( NS_sprm::LN_CHpsPos );

        m_rWW8Export.InsUInt16( (short)(( nHeight * nEsc + 500 ) / 1000 ));

        if( 100 != nProp || !b )
        {
            m_rWW8Export.InsUInt16( NS_sprm::LN_CHps );

            m_rWW8Export.InsUInt16(
                msword_cast<sal_uInt16>((nHeight * nProp + 500 ) / 1000));
        }
    }
}

void WW8AttributeOutput::CharFontSize( const SvxFontHeightItem& rHeight )
{
    sal_uInt16 nId = 0;
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

    if ( nId )
    {
        m_rWW8Export.InsUInt16( nId );

        m_rWW8Export.InsUInt16( (sal_uInt16)(( rHeight.GetHeight() + 5 ) / 10 ) );
    }
}

void WW8AttributeOutput::CharScaleWidth( const SvxCharScaleWidthItem& rScaleWidth )
{
    m_rWW8Export.InsUInt16( NS_sprm::LN_CCharScale );
    m_rWW8Export.InsUInt16( rScaleWidth.GetValue() );
}

void WW8AttributeOutput::CharRelief( const SvxCharReliefItem& rRelief )
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

void WW8AttributeOutput::CharBidiRTL( const SfxPoolItem& rHt )
{
    const SfxInt16Item& rAttr = static_cast<const SfxInt16Item&>(rHt);
    if( rAttr.GetValue() == 1 )
    {
        m_rWW8Export.InsUInt16(0x85a);
        m_rWW8Export.pO->push_back((sal_uInt8)(1));
    }
}

void WW8AttributeOutput::CharIdctHint( const SfxPoolItem& rHt )
{
    const SfxInt16Item& rAttr = static_cast<const SfxInt16Item&>(rHt);
    m_rWW8Export.InsUInt16(0x286F);
    m_rWW8Export.pO->push_back((sal_uInt8)(rAttr.GetValue()));
}

void WW8AttributeOutput::CharRotate( const SvxCharRotateItem& rRotate )
{
    // #i28331# - check that a Value is set
    if ( !rRotate.GetValue() )
        return;

    if (!m_rWW8Export.IsInTable())
    {
        // #i36867 In word the text in a table is rotated via the TC or NS_sprm::LN_TTextFlow
        // This means you can only rotate all or none of the text adding NS_sprm::LN_CFELayout
        // here corrupts the table, hence !m_rWW8Export.bIsInTable

        m_rWW8Export.InsUInt16( NS_sprm::LN_CFELayout );
        m_rWW8Export.pO->push_back( (sal_uInt8)0x06 ); //len 6
        m_rWW8Export.pO->push_back( (sal_uInt8)0x01 );

        m_rWW8Export.InsUInt16( rRotate.IsFitToLine() ? 1 : 0 );
        static const sal_uInt8 aZeroArr[ 3 ] = { 0, 0, 0 };
        m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), aZeroArr, aZeroArr+3);
    }
}

void WW8AttributeOutput::CharEmphasisMark( const SvxEmphasisMarkItem& rEmphasisMark )
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
        rShd.SetBack( msfilter::util::TransColToIco( rCol ) );
        rShd.SetStyle( 0 );
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
    m_rWW8Export.InsUInt16( NS_sprm::LN_CIco );

    sal_uInt8 nColor = msfilter::util::TransColToIco( rColor.GetValue() );
    m_rWW8Export.pO->push_back( nColor );

    if (nColor)
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_CCv );
        m_rWW8Export.InsUInt32( wwUtility::RGBToBGR( rColor.GetValue().GetColor() ) );
    }
}

void WW8AttributeOutput::CharBackground( const SvxBrushItem& rBrush )
{
    WW8_SHD aSHD;

    WW8Export::TransBrush( rBrush.GetColor(), aSHD );
    // sprmCShd80
    m_rWW8Export.InsUInt16( NS_sprm::LN_CShd80 );
    m_rWW8Export.InsUInt16( aSHD.GetValue() );

    //Quite a few unknowns, some might be transparency or something
    //of that nature...
    m_rWW8Export.InsUInt16( NS_sprm::LN_CShd );
    m_rWW8Export.pO->push_back( 10 );
    m_rWW8Export.InsUInt32( 0xFF000000 );
    m_rWW8Export.InsUInt32( SuitableBGColor( rBrush.GetColor().GetColor() ) );
    m_rWW8Export.InsUInt16( 0x0000);
}

void WW8AttributeOutput::TextINetFormat( const SwFormatINetFormat& rINet )
{
    if ( !rINet.GetValue().isEmpty() )
    {
        const sal_uInt16 nId = rINet.GetINetFormatId();
        const OUString& rStr = rINet.GetINetFormat();
        if (rStr.isEmpty())
        {
            OSL_ENSURE( false, "WW8AttributeOutput::TextINetFormat(..) - missing unvisited character format at hyperlink attribute" );
        }

        const SwCharFormat* pFormat = IsPoolUserFormat( nId )
                        ? m_rWW8Export.m_pDoc->FindCharFormatByName( rStr )
                        : m_rWW8Export.m_pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool( nId );

        m_rWW8Export.InsUInt16( NS_sprm::LN_CIstd );

        m_rWW8Export.InsUInt16( m_rWW8Export.GetId( pFormat ) );
    }
}

// #i43956# - add optional parameter <pLinkStr>
// It's needed to write the hyperlink data for a certain cross-reference
// - it contains the name of the link target, which is a bookmark.
// add optional parameter <bIncludeEmptyPicLocation>
// It is needed to write an empty picture location for page number field separators
static void InsertSpecialChar( WW8Export& rWrt, sal_uInt8 c,
                               OUString* pLinkStr = nullptr,
                               bool bIncludeEmptyPicLocation = false )
{
    ww::bytes aItems;
    rWrt.GetCurrentItems(aItems);

    if (c == 0x13)
        rWrt.m_pChpPlc->AppendFkpEntry(rWrt.Strm().Tell());
    else
        rWrt.m_pChpPlc->AppendFkpEntry(rWrt.Strm().Tell(), aItems.size(), aItems.data());

    rWrt.WriteChar(c);

    // store empty sprmCPicLocation for field separator
    if ( bIncludeEmptyPicLocation &&
         ( c == 0x13 || c == 0x14 || c == 0x15 ) )
    {
        SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CPicLocation );
        SwWW8Writer::InsUInt32( aItems, 0x00000000 );
    }

    // #i43956# - write hyperlink data and attributes
    if ( c == 0x01 && pLinkStr)
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
        sal_uInt32 nStrLen( pLinkStr->getLength() + 1 );
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
        SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CFFieldVanish );
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
    SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CFSpec );
    aItems.push_back( 1 );

    rWrt.m_pChpPlc->AppendFkpEntry(rWrt.Strm().Tell(), aItems.size(), aItems.data());
}

static OUString lcl_GetExpandedField(const SwField &rField)
{
    OUString sRet(rField.ExpandField(true));

    //replace LF 0x0A with VT 0x0B
    return sRet.replace(0x0A, 0x0B);
}

WW8_WrPlcField* WW8Export::CurrentFieldPlc() const
{
    WW8_WrPlcField* pFieldP = nullptr;
    switch (m_nTextTyp)
    {
        case TXT_MAINTEXT:
            pFieldP = m_pFieldMain;
            break;
        case TXT_HDFT:
            pFieldP = m_pFieldHdFt;
            break;
        case TXT_FTN:
            pFieldP = m_pFieldFootnote;
            break;
        case TXT_EDN:
            pFieldP = m_pFieldEdn;
            break;
        case TXT_ATN:
            pFieldP = m_pFieldAtn;
            break;
        case TXT_TXTBOX:
            pFieldP = m_pFieldTextBxs;
            break;
        case TXT_HFTXTBOX:
            pFieldP = m_pFieldHFTextBxs;
            break;
        default:
            OSL_ENSURE( false, "was ist das fuer ein SubDoc-Type?" );
    }
    return pFieldP;
}

void WW8Export::OutputField( const SwField* pField, ww::eField eFieldType,
    const OUString& rFieldCmd, sal_uInt8 nMode )
{
    OUString sFieldCmd(rFieldCmd);
    switch (eFieldType)
    {
        // map fields that are not supported in WW8 as of Word 2003
        case ww::eBIBLIOGRPAHY:
            eFieldType = ww::eQUOTE;
            assert(rFieldCmd == FieldString(ww::eBIBLIOGRPAHY));
            sFieldCmd = FieldString(ww::eQUOTE);
            break;
        case ww::eCITATION:
            eFieldType = ww::eQUOTE;
            assert(rFieldCmd.startsWith(FieldString(ww::eCITATION)));
            sFieldCmd = rFieldCmd.replaceFirst(FieldString(ww::eCITATION),
                                               FieldString(ww::eQUOTE));
            break;
        default:
            break;
    }

    assert(eFieldType <= 0x5F); // 95 is the highest documented one

    WW8_WrPlcField* pFieldP = CurrentFieldPlc();

    const bool bIncludeEmptyPicLocation = ( eFieldType == ww::ePAGE );
    if (WRITEFIELD_START & nMode)
    {
        sal_uInt8 aField13[2] = { 0x13, 0x00 };  // will change
        //#i3958#, Needed to make this field work correctly in Word 2000
        if (eFieldType == ww::eSHAPE)
            aField13[0] |= 0x80;
        aField13[1] = static_cast< sal_uInt8 >(eFieldType);  // Typ nachtragen
        pFieldP->Append( Fc2Cp( Strm().Tell() ), aField13 );
        InsertSpecialChar( *this, 0x13, nullptr, bIncludeEmptyPicLocation );
    }
    if (WRITEFIELD_CMD_START & nMode)
    {
        SwWW8Writer::WriteString16(Strm(), sFieldCmd, false);
        // #i43956# - write hyperlink character including
        // attributes and corresponding binary data for certain reference fields.
        bool bHandleBookmark = false;

        if (pField)
        {
            if (pField->GetTyp()->Which() == RES_GETREFFLD &&
                ( eFieldType == ww::ePAGEREF || eFieldType == ww::eREF ||
                  eFieldType == ww::eNOTEREF || eFieldType == ww::eFOOTREF ))
                bHandleBookmark = true;
        }

        if ( bHandleBookmark )
        {
            // retrieve reference destination - the name of the bookmark
            OUString aLinkStr;
            const sal_uInt16 nSubType = pField->GetSubType();
            const SwGetRefField& rRField = *(static_cast<const SwGetRefField*>(pField));
            if ( nSubType == REF_SETREFATTR ||
                 nSubType == REF_BOOKMARK )
            {
                const OUString aRefName(rRField.GetSetRefName());
                aLinkStr = GetBookmarkName( nSubType, &aRefName, 0 );
            }
            else if ( nSubType == REF_FOOTNOTE ||
                      nSubType == REF_ENDNOTE )
            {
                aLinkStr = GetBookmarkName( nSubType, nullptr, rRField.GetSeqNo() );
            }
            else if ( nSubType == REF_SEQUENCEFLD )
            {
                aLinkStr = pField->GetPar2();
            }
            // insert hyperlink character including attributes and data.
            InsertSpecialChar( *this, 0x01, &aLinkStr );
        }
    }
    if (WRITEFIELD_CMD_END & nMode)
    {
        static const sal_uInt8 aField14[2] = { 0x14, 0xff };
        pFieldP->Append( Fc2Cp( Strm().Tell() ), aField14 );
        pFieldP->ResultAdded();
        InsertSpecialChar( *this, 0x14, nullptr, bIncludeEmptyPicLocation );
    }
    if (WRITEFIELD_END & nMode)
    {
        OUString sOut;
        if( pField )
            sOut = lcl_GetExpandedField(*pField);
        else
            sOut = sFieldCmd;
        if( !sOut.isEmpty() )
        {
            SwWW8Writer::WriteString16(Strm(), sOut, false);

            if (pField)
            {
                if (pField->GetTyp()->Which() == RES_INPUTFLD &&
                    eFieldType == ww::eFORMTEXT)
                {
                    sal_uInt8 aArr[12];
                    sal_uInt8 *pArr = aArr;

                    Set_UInt16( pArr, NS_sprm::LN_CPicLocation );
                    Set_UInt32( pArr, 0x0 );

                    Set_UInt16( pArr, NS_sprm::LN_CFSpec );
                    Set_UInt8( pArr, 1 );

                    Set_UInt16( pArr, NS_sprm::LN_CFNoProof );
                    Set_UInt8( pArr, 1 );

                    m_pChpPlc->AppendFkpEntry( Strm().Tell(), static_cast< short >(pArr - aArr), aArr );
                }
            }
        }
    }
    if (WRITEFIELD_CLOSE & nMode)
    {
        sal_uInt8 aField15[2] = { 0x15, 0x80 };

        if (pField)
        {
            if (pField->GetTyp()->Which() == RES_INPUTFLD &&
                eFieldType == ww::eFORMTEXT)
            {
                sal_uInt16 nSubType = pField->GetSubType();

                if (nSubType == REF_SEQUENCEFLD)
                    aField15[0] |= (0x4 << 5);
            }
        }

        pFieldP->Append( Fc2Cp( Strm().Tell() ), aField15 );
        InsertSpecialChar( *this, 0x15, nullptr, bIncludeEmptyPicLocation );
    }
}

void WW8Export::StartCommentOutput(const OUString& rName)
{
    OUString sStr(FieldString(ww::eQUOTE));
    sStr += "[" + rName + "] ";
    OutputField(nullptr, ww::eQUOTE, sStr, WRITEFIELD_START | WRITEFIELD_CMD_START);
}

void WW8Export::EndCommentOutput(const OUString& rName)
{
    OUString sStr(" [");
    sStr += rName + "] ";
    OutputField(nullptr, ww::eQUOTE, sStr, WRITEFIELD_CMD_END | WRITEFIELD_END |
        WRITEFIELD_CLOSE);
}

sal_uInt16 MSWordExportBase::GetId( const SwTOXType& rTOXType )
{
    std::vector<const SwTOXType*>::iterator it
        = std::find( m_aTOXArr.begin(), m_aTOXArr.end(), &rTOXType );
    if ( it != m_aTOXArr.end() )
    {
        return it - m_aTOXArr.begin();
    }
    m_aTOXArr.push_back( &rTOXType );
    return m_aTOXArr.size() - 1;
}

// return values:  1 - no PageNum,
//                 2 - TabStop before PageNum,
//                 3 - Text before PageNum - rText hold the text
//                 4 - no Text and no TabStop before PageNum
static int lcl_CheckForm( const SwForm& rForm, sal_uInt8 nLvl, OUString& rText )
{
    int nRet = 4;
    rText.clear();

    // #i21237#
    SwFormTokens aPattern = rForm.GetPattern(nLvl);
    SwFormTokens::iterator aIt = aPattern.begin();
    FormTokenType eTType;

    // #i61362#
    if (! aPattern.empty())
    {
        bool bPgNumFnd = false;

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
            {
                nRet = 3;
                sal_Int32 nCount = std::min<sal_Int32>(5, aIt->sText.getLength());
                rText = aIt->sText.copy(0, nCount); // #i21237#
                break;
            }
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
        OUString sStr = pTOX ->GetMSTOCExpression();
        if ( sStr.isEmpty() )
        {
            switch (pTOX->GetType())
            {
            case TOX_INDEX:
                eCode = ww::eINDEX;
                sStr = FieldString(eCode);

                {
                    const SwFormatCol& rCol = static_cast<const SwFormatCol&>( rSect.GetFormat()->GetFormatAttr( RES_COL ) );
                    const SwColumns& rColumns = rCol.GetColumns();
                    sal_Int32 nCol = rColumns.size();

                    if ( 0 < nCol )
                    {
                        // Add a continuous section break
                        if( GetExport().AddSectionBreaksForTOX() )
                        {
                            sal_uLong nRstLnNum = 0;
                            SwSection *pParent = rSect.GetParent();
                            WW8_SepInfo rInfo(&GetExport( ).m_pDoc->GetPageDesc(0),
                                pParent ? pParent->GetFormat() : nullptr, nRstLnNum);
                            GetExport( ).AttrOutput().SectionBreak( msword::PageBreak, &rInfo );
                        }

                        sStr += "\\c \"" + OUString::number( nCol ) + "\"";
                    }
                }

                if (pTOX->GetTOXForm().IsCommaSeparated())
                    sStr += "\\r ";

                if (nsSwTOIOptions::TOI_ALPHA_DELIMITTER & pTOX->GetOptions())
                    sStr += "\\h \"A\" ";

                if(nsSwTOXElement::TOX_INDEX_ENTRY_TYPE & pTOX->GetCreateType())
                {
                    sStr += "\\f ";
                    OUString sName = pTOX->GetEntryTypeName();
                    if(!sName.isEmpty())
                    {
                       sStr += sName;
                       sStr += sEntryEnd;
                    }
                 }

                if (!pTOX->GetTOXForm().IsCommaSeparated())
                {
                    // In case of Run-in style no separators are added.
                    OUString aFillText;
                    for (sal_uInt8 n = 1; n <= 3; ++n)
                    {
                        OUString aText;
                        int nRet = ::lcl_CheckForm(pTOX->GetTOXForm(), n, aText);

                        if( 3 == nRet )
                            aFillText = aText;
                        else if ((4 == nRet) || (2 == nRet))
                            aFillText = "\t";
                        else
                            aFillText.clear();
                    }
                    sStr += "\\e \"";
                    sStr += aFillText;
                    sStr += sEntryEnd;
                }
                break;

            case TOX_ILLUSTRATIONS:
            case TOX_OBJECTS:
            case TOX_TABLES:
                if (!pTOX->IsFromObjectNames())
                {
                    sStr = FieldString(eCode);

                    sStr += "\\c ";
                    OUString seqName = pTOX->GetSequenceName();
                    if(!seqName.isEmpty())
                    {
                        sStr += "\"";
                        sStr += seqName;
                        sStr += sEntryEnd;
                    }
                    OUString aText;
                    int nRet = ::lcl_CheckForm( pTOX->GetTOXForm(), 1, aText );
                    if (1 == nRet)
                        sStr += "\\n ";
                    else if( 3 == nRet || 4 == nRet )
                    {
                        sStr += "\\p \"";
                        sStr += aText;
                        sStr += sEntryEnd;
                    }
                }
                break;

            case TOX_AUTHORITIES:
                eCode = ww::eBIBLIOGRPAHY;
                sStr = FieldString(eCode);
                 break;
                //      case TOX_USER:
                //      case TOX_CONTENT:
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
                        GetExport( ).m_bHideTabLeaderAndPageNumbers = true ;
                    }
                    if(nsSwTOXElement::TOX_TAB_IN_TOC & pTOX->GetCreateType())
                    {
                        sStr +="\\w " ;
                        GetExport( ).m_bTabInTOC = true ;
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
                        // Take the TOC value of the max level to evaluate to as
                        // the starting point for the \o flag, but reduce it to the
                        // value of the highest outline level filled by a *standard*
                        // Heading 1 - 9 style because \o "Builds a table of
                        // contents from paragraphs formatted with built-in heading
                        // styles". And afterward fill in any outline styles left
                        // uncovered by that range to the \t flag

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
                        const SwTextFormatColls& rColls = *GetExport().m_pDoc->GetTextFormatColls();
                        for( n = rColls.size(); n; )
                        {
                            const SwTextFormatColl* pColl = rColls[ --n ];
                            sal_uInt16 nPoolId = pColl->GetPoolFormatId();
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

                            sStr += "\\o \"1-";
                            sStr += OUString::number(nMaxMSAutoEvaluate);
                            sStr += sEntryEnd;
                        }

                        //collect up any other styles in the writer TOC which will
                        //not already appear in the MS TOC and place then into the
                        //\t option
                        if( nMaxMSAutoEvaluate < nTOXLvl )
                        {
                            // collect this templates into the \t option
                            for( n = rColls.size(); n;)
                            {
                                const SwTextFormatColl* pColl = rColls[ --n ];
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
                            // #i99641# - Consider additional styles regardless of TOX-outlinelevel
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
                                            SwTextFormatColl* pColl = GetExport().m_pDoc->FindTextFormatCollByName(sStyle);
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

                    // No 'else' branch; why the below snippet is a block I have no idea.
                    {
                        OUString aFillText;
                        sal_uInt8 nNoPgStt = MAXLEVEL, nNoPgEnd = MAXLEVEL;
                        bool bFirstFillText = true, bOnlyText = true;
                        for( n = 0; n < nTOXLvl; ++n )
                        {
                            OUString aText;
                            int nRet = ::lcl_CheckForm( pTOX->GetTOXForm(),
                                static_cast< sal_uInt8 >(n+1), aText );
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
                                    if( bFirstFillText )
                                        aFillText = aText;
                                    else if( aFillText != aText )
                                        aFillText.clear();
                                    bFirstFillText = false;
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
                            sStr += aFillText;
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
            GetExport( ).m_bInWriteTOX = true;
            GetExport( ).OutputField( nullptr, eCode, sStr, WRITEFIELD_START | WRITEFIELD_CMD_START |
                WRITEFIELD_CMD_END );
        }
    }

    GetExport( ).m_bStartTOX = false;
}

void AttributeOutputBase::EndTOX( const SwSection& rSect,bool bCareEnd )
{
    const SwTOXBase* pTOX = rSect.GetTOXBase();
    if ( pTOX )
    {
        ww::eField eCode = TOX_INDEX == pTOX->GetType() ? ww::eINDEX : ww::eTOC;
        GetExport( ).OutputField( nullptr, eCode, OUString(), WRITEFIELD_CLOSE );

        if ( pTOX->GetType() == TOX_INDEX && GetExport().AddSectionBreaksForTOX() )
        {
            const SwFormatCol& rCol = static_cast<const SwFormatCol&>( rSect.GetFormat()->GetFormatAttr( RES_COL ) );
            const SwColumns& rColumns = rCol.GetColumns();
            sal_Int32 nCol = rColumns.size();

            if ( 0 < nCol )
            {
                sal_uLong nRstLnNum = 0;
                WW8_SepInfo rInfo( &GetExport( ).m_pDoc->GetPageDesc( 0 ), rSect.GetFormat() , nRstLnNum );
                GetExport( ).AttrOutput().SectionBreak( msword::PageBreak, &rInfo );
            }
        }
    }
    GetExport( ).m_bInWriteTOX = false;
    if (bCareEnd)
        OnTOXEnding();
}

bool MSWordExportBase::GetNumberFormat(const SwField& rField, OUString& rStr)
{
    // Returns a date or time format string by using the US NfKeywordTable
    bool bHasFormat = false;
    SvNumberFormatter* pNFormatr = m_pDoc->GetNumberFormatter();
    sal_uInt32 nFormatIdx = rField.GetFormat();
    const SvNumberformat* pNumFormat = pNFormatr->GetEntry( nFormatIdx );
    if( pNumFormat )
    {
        sal_uInt16 nLng = rField.GetLanguage();
        LocaleDataWrapper aLocDat(pNFormatr->GetComponentContext(),
                                  LanguageTag(nLng));

        OUString sFormat(pNumFormat->GetMappedFormatstring(GetNfKeywordTable(),
            aLocDat));

        if (!sFormat.isEmpty())
        {
            sw::ms::SwapQuotesInField(sFormat);

            rStr = "\\@\"" + sFormat + "\" " ;
            bHasFormat = true;
        }
    }
    return bHasFormat;
}

void AttributeOutputBase::GetNumberPara( OUString& rStr, const SwField& rField )
{
    switch(rField.GetFormat())
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
            OSL_ENSURE(rField.GetFormat() == SVX_NUM_ARABIC,
                "Unknown numbering type exported as default of Arabic\n");
            SAL_FALLTHROUGH;
        case SVX_NUM_ARABIC:
            rStr += "\\* ARABIC ";
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
    Set_UInt16( pArr, NS_sprm::LN_CFSpec );
    Set_UInt8( pArr, 1 );

    m_pChpPlc->AppendFkpEntry( Strm().Tell() );
    WriteChar( 0x05 );              // Annotation reference

    if( pOut )
        pOut->insert( pOut->end(), aArr, pArr );
    else
        m_pChpPlc->AppendFkpEntry( Strm().Tell(), static_cast< short >(pArr - aArr), aArr );
}

OUString FieldString(ww::eField eIndex)
{
    if (const char *pField = ww::GetEnglishFieldName(eIndex))
        return " " + OUString::createFromAscii(pField) + " ";
    return OUString("  ");
}

void WW8AttributeOutput::HiddenField( const SwField& rField )
{
    OUString sExpand(rField.GetPar2());

    //replace LF 0x0A with VT 0x0B
    sExpand = sExpand.replace(0x0A, 0x0B);
    m_rWW8Export.m_pChpPlc->AppendFkpEntry(m_rWW8Export.Strm().Tell());
    SwWW8Writer::WriteString16(m_rWW8Export.Strm(), sExpand, false);
    static sal_uInt8 aArr[] =
    {
        0x3C, 0x08, 0x1
    };
    m_rWW8Export.m_pChpPlc->AppendFkpEntry(m_rWW8Export.Strm().Tell(), sizeof(aArr), aArr);
}

void WW8AttributeOutput::SetField( const SwField& rField, ww::eField eType, const OUString& rCmd )
{
    const SwSetExpField* pSet = static_cast<const SwSetExpField*>(&rField);
    const OUString &rVar = pSet->GetPar2();

    sal_uLong nFrom = m_rWW8Export.Fc2Cp(m_rWW8Export.Strm().Tell());

    GetExport().OutputField(&rField, eType, rCmd, WRITEFIELD_START |
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
        SwWW8Writer::WriteString16(m_rWW8Export.Strm(), rVar, false);
    }
    GetExport().OutputField(&rField, eType, rCmd, WRITEFIELD_CLOSE);
}

void WW8AttributeOutput::PostitField( const SwField* pField )
{
    const SwPostItField *pPField = static_cast<const SwPostItField*>(pField);
    m_rWW8Export.m_pAtn->Append( m_rWW8Export.Fc2Cp( m_rWW8Export.Strm().Tell() ), pPField );
    m_rWW8Export.WritePostItBegin( m_rWW8Export.pO );
}

bool WW8AttributeOutput::DropdownField( const SwField* pField )
{
    const SwDropDownField& rField2 = *static_cast<const SwDropDownField*>(pField);
    uno::Sequence<OUString> aItems =
        rField2.GetItemSequence();
    GetExport().DoComboBox(rField2.GetName(),
                       rField2.GetHelp(),
                       rField2.GetToolTip(),
                       rField2.GetSelectedItem(), aItems);
    return false;
}

bool WW8AttributeOutput::PlaceholderField( const SwField* )
{
    return true; // expand to text?
}

void WW8AttributeOutput::RefField( const SwField &rField, const OUString &rRef)
{
    OUString sStr( FieldString( ww::eREF ) );
    sStr += "\"" + rRef + "\" ";
    m_rWW8Export.OutputField( &rField, ww::eREF, sStr, WRITEFIELD_START |
        WRITEFIELD_CMD_START | WRITEFIELD_CMD_END );
    OUString sVar = lcl_GetExpandedField( rField );
    if ( !sVar.isEmpty() )
    {
        SwWW8Writer::WriteString16( m_rWW8Export.Strm(), sVar, false );
    }
    m_rWW8Export.OutputField( &rField, ww::eREF, sStr, WRITEFIELD_CLOSE );
}

void WW8AttributeOutput::WriteExpand( const SwField* pField )
{
    OUString sExpand( lcl_GetExpandedField( *pField ) );
    SwWW8Writer::WriteString16( m_rWW8Export.Strm(), sExpand, false );
}

void AttributeOutputBase::TextField( const SwFormatField& rField )
{
    const SwField* pField = rField.GetField();
    bool bWriteExpand = false;
    const sal_uInt16 nSubType = pField->GetSubType();

    switch (pField->GetTyp()->Which())
    {
    case RES_GETEXPFLD:
        if (nSubType == nsSwGetSetExpType::GSE_STRING)
        {
            const SwGetExpField *pGet = static_cast<const SwGetExpField*>(pField);
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
                sStr = FieldString(ww::eSEQ) +  pField->GetTyp()->GetName() + " ";
            else
                sStr = FieldString(ww::eSEQ) + "\"" + pField->GetTyp()->GetName() +"\" ";
            GetNumberPara( sStr, *pField );
            GetExport().OutputField(pField, ww::eSEQ, sStr);
        }
        else if (nSubType & nsSwGetSetExpType::GSE_STRING)
        {
            bool bShowAsWell = false;
            ww::eField eFieldNo;
            const SwSetExpField *pSet = static_cast<const SwSetExpField*>(pField);
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
                bShowAsWell = (nSubType & nsSwExtendedSubType::SUB_INVISIBLE) == 0;
            }

            SetField( *pField, eFieldNo, sStr );

            if (bShowAsWell)
                RefField( *pSet, pSet->GetPar1() );
        }
        else
            bWriteExpand = true;
        break;
    case RES_PAGENUMBERFLD:
        {
            OUString sStr = FieldString(ww::ePAGE);
            GetNumberPara(sStr, *pField);
            GetExport().OutputField(pField, ww::ePAGE, sStr);
        }
        break;
    case RES_FILENAMEFLD:
        {
            OUString sStr = FieldString(ww::eFILENAME);
            if (pField->GetFormat() == FF_PATHNAME)
                sStr += "\\p ";
            GetExport().OutputField(pField, ww::eFILENAME, sStr);
        }
        break;
    case RES_DBNAMEFLD:
        {
            SwDBData aData = GetExport().m_pDoc->GetDBData();
            const OUString sStr = FieldString(ww::eDATABASE)
                + aData.sDataSource
                + OUString(DB_DELIM)
                + aData.sCommand;
            GetExport().OutputField(pField, ww::eDATABASE, sStr);
        }
        break;
    case RES_AUTHORFLD:
        {
            ww::eField eField =
                (AF_SHORTCUT & pField->GetFormat() ? ww::eUSERINITIALS : ww::eUSERNAME);
            GetExport().OutputField(pField, eField, FieldString(eField));
        }
        break;
    case RES_TEMPLNAMEFLD:
        GetExport().OutputField(pField, ww::eTEMPLATE, FieldString(ww::eTEMPLATE));
        break;
    case RES_DOCINFOFLD:    // Last printed, last edited,...
        if( DI_SUB_FIXED & nSubType )
            bWriteExpand = true;
        else
        {
            OUString sStr;
            ww::eField eField(ww::eNONE);
            switch (0xff & nSubType)
            {
                case DI_TITEL:
                    eField = ww::eTITLE;
                    break;
                case DI_THEMA:
                    eField = ww::eSUBJECT;
                    break;
                case DI_KEYS:
                    eField = ww::eKEYWORDS;
                    break;
                case DI_COMMENT:
                    eField = ww::eCOMMENTS;
                    break;
                case DI_DOCNO:
                    eField = ww::eREVNUM;
                    break;
                case DI_CREATE:
                    if (DI_SUB_AUTHOR == (nSubType & DI_SUB_MASK))
                        eField = ww::eAUTHOR;
                    else if (GetExport().GetNumberFormat(*pField, sStr))
                        eField = ww::eCREATEDATE;
                    break;

                case DI_CHANGE:
                    if (DI_SUB_AUTHOR == (nSubType & DI_SUB_MASK))
                        eField = ww::eLASTSAVEDBY;
                    else if (GetExport().GetNumberFormat(*pField, sStr))
                        eField = ww::eSAVEDATE;
                    break;

                case DI_PRINT:
                    if (DI_SUB_AUTHOR != (nSubType & DI_SUB_MASK) &&
                        GetExport().GetNumberFormat(*pField, sStr))
                        eField = ww::ePRINTDATE;
                    break;
                case DI_EDIT:
                    if( DI_SUB_AUTHOR != (nSubType & DI_SUB_MASK ) &&
                        GetExport().GetNumberFormat( *pField, sStr ))
                        eField = ww::eSAVEDATE;
                    else
                        eField = ww::eEDITTIME;
                    break;
                case DI_CUSTOM:
                    eField = ww::eDOCPROPERTY;
                    {
                        OUString sQuotes('\"');
                        const SwDocInfoField * pDocInfoField =
                        dynamic_cast<const SwDocInfoField *> (pField);

                        if (pDocInfoField != nullptr)
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

            if (eField != ww::eNONE)
            {
                GetExport().OutputField(pField, eField, FieldString(eField) + sStr);
            }
            else
                bWriteExpand = true;
        }
        break;
    case RES_DATETIMEFLD:
        {
            OUString sStr;
            if (!GetExport().GetNumberFormat(*pField, sStr))
                bWriteExpand = true;
            else
            {
                ww::eField eField = (DATEFLD & nSubType) ? ww::eDATE : ww::eTIME;
                GetExport().OutputField(pField, eField, FieldString(eField) + sStr);
            }
        }
        break;
    case RES_DOCSTATFLD:
        {
            ww::eField eField = ww::eNONE;

            switch (nSubType)
            {
                case DS_PAGE:
                    eField = ww::eNUMPAGE;
                    break;
                case DS_WORD:
                    eField = ww::eNUMWORDS;
                    break;
                case DS_CHAR:
                    eField = ww::eNUMCHARS;
                    break;
            }

            if (eField != ww::eNONE)
            {
                OUString sStr = FieldString(eField);
                GetNumberPara(sStr, *pField);
                GetExport().OutputField(pField, eField, sStr);
            }
            else
                bWriteExpand = true;
        }
        break;
    case RES_EXTUSERFLD:
        {
            ww::eField eField = ww::eNONE;
            switch (0xFF & nSubType)
            {
                case EU_FIRSTNAME:
                case EU_NAME:
                    eField = ww::eUSERNAME;
                    break;
                case EU_SHORTCUT:
                    eField = ww::eUSERINITIALS;
                    break;
                case EU_STREET:
                case EU_COUNTRY:
                case EU_ZIP:
                case EU_CITY:
                    eField = ww::eUSERADDRESS;
                    break;
            }

            if (eField != ww::eNONE)
            {
                GetExport().OutputField(pField, eField, FieldString(eField));
            }
            else
                bWriteExpand = true;
        }
        break;
    case RES_AUTHORITY:
    {
        OUString sRet(static_cast<SwAuthorityField const*>(pField)
                        ->ExpandCitation(AUTH_FIELD_IDENTIFIER));
        // FIXME: DomainMapper_Impl::CloseFieldCommand() stuffs fully formed
        // field instructions in here, but if the field doesn't originate
        // from those filters it won't have that
        if (!sRet.trim().startsWith("CITATION"))
        {
            sRet = FieldString(ww::eCITATION) + " \"" + sRet + "\"";
        }
        GetExport().OutputField( pField, ww::eCITATION, sRet );
    }
    break;
    case RES_POSTITFLD:
        //Sadly only possible for word in main document text
        if (GetExport().m_nTextTyp == TXT_MAINTEXT)
        {
            PostitField( pField );
        }
        break;
    case RES_INPUTFLD:
        {
            const SwInputField * pInputField = dynamic_cast<const SwInputField *>(pField);

            if (pInputField && pInputField->isFormField())
                GetExport().DoFormText(pInputField);
            else
            {
                const OUString sStr = FieldString(ww::eFILLIN) + "\""
                    + pField->GetPar2() + "\"";

                GetExport().OutputField(pField, ww::eFILLIN, sStr);
            }
        }
        break;
    case RES_GETREFFLD:
        {
            ww::eField eField = ww::eNONE;
            OUString sStr;
            const SwGetRefField& rRField = *static_cast<const SwGetRefField*>(pField);
            switch (nSubType)
            {
                case REF_SETREFATTR:
                case REF_BOOKMARK:
                    switch (pField->GetFormat())
                    {
                        case REF_PAGE_PGDESC:
                        case REF_PAGE:
                            eField = ww::ePAGEREF;
                            break;
                        default:
                            eField = ww::eREF;
                            break;
                    }
                    {
                        const OUString aRefName(rRField.GetSetRefName());
                        sStr = FieldString(eField)
                            + MSWordExportBase::GetBookmarkName(nSubType, &aRefName, 0);
                    }
                    switch (pField->GetFormat())
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
                    switch (pField->GetFormat())
                    {
                        case REF_PAGE_PGDESC:
                        case REF_PAGE:
                            eField = ww::ePAGEREF;
                            break;
                        case REF_UPDOWN:
                            eField = ww::eREF;
                            break;
                        default:
                            eField =
                                REF_ENDNOTE == nSubType ? ww::eNOTEREF : ww::eFOOTREF;
                            break;
                    }
                    sStr = FieldString(eField)
                        + MSWordExportBase::GetBookmarkName(nSubType, nullptr, rRField.GetSeqNo());
                    break;
            }

            if (eField != ww::eNONE)
            {
                switch (pField->GetFormat())
                {
                    case REF_UPDOWN:
                        sStr += " \\p \\h ";   // with hyperlink
                        break;
                    case REF_CHAPTER:
                        sStr += " \\n \\h ";   // with hyperlink
                        break;
                    default:
                        sStr += " \\h ";       // insert hyperlink
                        break;
                }
                GetExport().OutputField(pField, eField, sStr);
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
            nScript = g_pBreakIt->GetBreakIter()->getScriptType( pField->GetPar1(), 0);
        else
            nScript = i18n::ScriptType::ASIAN;

        long nHeight = static_cast<const SvxFontHeightItem&>((GetExport().GetItem(
            GetWhichOfScript(RES_CHRATR_FONTSIZE,nScript)))).GetHeight();

        nHeight = (nHeight + 10) / 20; //Font Size in points;

        /*
        Divide the combined char string into its up and down part. Get the
        font size and fill in the defaults as up == half the font size and
        down == a fifth the font size
        */
        const sal_Int32 nAbove = (pField->GetPar1().getLength()+1)/2;
        const OUString sStr = FieldString(ww::eEQ)
            + "\\o (\\s\\up "
            + OUString::number(nHeight/2)
            + "("
            + pField->GetPar1().copy(0, nAbove)
            + "), \\s\\do "
            + OUString::number(nHeight/5)
            + "("
            + pField->GetPar1().copy(nAbove)
            + "))";
        GetExport().OutputField(pField, ww::eEQ, sStr);
        }
        break;
    case RES_DROPDOWN:
        bWriteExpand = DropdownField( pField );
        break;
    case RES_CHAPTERFLD:
        bWriteExpand = true;
        if (GetExport().m_bOutKF && rField.GetTextField())
        {
            const SwTextNode *pTextNd = GetExport().GetHdFtPageRoot();
            if (!pTextNd)
            {
                pTextNd = GetExport().m_pCurPam->GetNode().GetTextNode();
            }

            if (pTextNd)
            {
                SwChapterField aCopy(*static_cast<const SwChapterField*>(pField));
                aCopy.ChangeExpansion(*pTextNd, false);
                const OUString sStr = FieldString(ww::eSTYLEREF)
                    + " "
                    + OUString::number(aCopy.GetLevel() + 1)
                    + " \\* MERGEFORMAT ";
                GetExport().OutputField(pField, ww::eSTYLEREF, sStr);
                bWriteExpand = false;
            }
        }
        break;
    case RES_HIDDENTXTFLD:
        {
            OUString sExpand(pField->GetPar2());
            if (!sExpand.isEmpty())
            {
                HiddenField( *pField );
            }
        }
        break;
    case RES_JUMPEDITFLD:
        bWriteExpand = PlaceholderField( pField );
        break;
    case RES_MACROFLD:
        {
            const OUString sStr = " MACROBUTTON"
                + pField->GetPar1().replaceFirst("StarOffice.Standard.Modul1.", " ")
                + " "
                + lcl_GetExpandedField(*pField);
            GetExport().OutputField( pField, ww::eMACROBUTTON, sStr );
        }
        break;
    default:
        bWriteExpand = true;
        break;
    }

    if (bWriteExpand)
        WriteExpand( pField );
}

void AttributeOutputBase::TextFlyContent( const SwFormatFlyCnt& rFlyContent )
{
    if ( GetExport().m_pOutFormatNode && dynamic_cast< const SwContentNode *>( GetExport().m_pOutFormatNode ) != nullptr  )
    {
        const SwTextNode* pTextNd = static_cast<const SwTextNode*>(GetExport().m_pOutFormatNode);

        Point aLayPos;
        aLayPos = pTextNd->FindLayoutRect( false, &aLayPos ).Pos();

        SwPosition aPos( *pTextNd );
        ww8::Frame aFrame( *rFlyContent.GetFrameFormat(), aPos );

        OutputFlyFrame_Impl( aFrame, aLayPos );
    }
}

// TOXMarks fehlen noch

// Detaillierte Einstellungen zur Trennung erlaubt WW nur dokumentenweise.
// Man koennte folgende Mimik einbauen: Die Werte des Style "Standard" werden,
// falls vorhanden, in die Document Properties ( DOP ) gesetzt.

// ACK.  Dieser Vorschlag passt exakt zu unserer Implementierung des Import,
// daher setze ich das gleich mal um. (KHZ, 07/15/2000)
void WW8AttributeOutput::ParaHyphenZone( const SvxHyphenZoneItem& rHyphenZone )
{
    // sprmPFNoAutoHyph
    m_rWW8Export.InsUInt16( NS_sprm::LN_PFNoAutoHyph );

    m_rWW8Export.pO->push_back( rHyphenZone.IsHyphen() ? 0 : 1 );
}

void WW8AttributeOutput::ParaScriptSpace( const SfxBoolItem& rScriptSpace )
{
    m_rWW8Export.InsUInt16( NS_sprm::LN_PFAutoSpaceDE );
    m_rWW8Export.pO->push_back( rScriptSpace.GetValue() ? 1 : 0 );
}

void WW8AttributeOutput::ParaHangingPunctuation( const SfxBoolItem& rItem )
{
    m_rWW8Export.InsUInt16( NS_sprm::LN_PFOverflowPunct );
    m_rWW8Export.pO->push_back( rItem.GetValue() ? 1 : 0 );
}

void WW8AttributeOutput::ParaForbiddenRules( const SfxBoolItem& rItem )
{
    m_rWW8Export.InsUInt16( NS_sprm::LN_PFKinsoku );
    m_rWW8Export.pO->push_back( rItem.GetValue() ? 1 : 0 );
}

void WW8AttributeOutput::ParaSnapToGrid( const SvxParaGridItem& rGrid )
{
    // sprmPFUsePgsuSettings

    m_rWW8Export.InsUInt16( NS_sprm::LN_PFUsePgsuSettings );
    m_rWW8Export.pO->push_back( rGrid.GetValue() ? 1 : 0 );
}

void WW8AttributeOutput::ParaVerticalAlign( const SvxParaVertAlignItem& rAlign )
{
    // sprmPWAlignFont

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

void WW8Export::WriteFootnoteBegin( const SwFormatFootnote& rFootnote, ww::bytes* pOutArr )
{
    ww::bytes aAttrArr;
    const bool bAutoNum = rFootnote.GetNumStr().isEmpty();    // Auto-Nummer
    if( bAutoNum )
    {
        static const sal_uInt8 aSpec[] =
        {
            0x03, 0x6a, 0, 0, 0, 0, // sprmCObjLocation
            0x55, 0x08, 1           // sprmCFSpec
        };

        aAttrArr.insert(aAttrArr.end(), aSpec, aSpec+sizeof(aSpec));
    }

    // sprmCIstd
    const SwEndNoteInfo* pInfo;
    if( rFootnote.IsEndNote() )
        pInfo = &m_pDoc->GetEndNoteInfo();
    else
        pInfo = &m_pDoc->GetFootnoteInfo();
    const SwCharFormat* pCFormat = pOutArr
                                ? pInfo->GetAnchorCharFormat( *m_pDoc )
                                : pInfo->GetCharFormat( *m_pDoc );
    SwWW8Writer::InsUInt16( aAttrArr, NS_sprm::LN_CIstd );
    SwWW8Writer::InsUInt16( aAttrArr, GetId( pCFormat ) );

                                                // fSpec-Attribut true
                            // Fuer Auto-Nummer muss ein Spezial-Zeichen
                            // in den Text und darum ein fSpec-Attribut
    m_pChpPlc->AppendFkpEntry( Strm().Tell() );
    if( bAutoNum )
        WriteChar( 0x02 );              // Auto-Nummer-Zeichen
    else
        // User-Nummerierung
        OutSwString(rFootnote.GetNumStr(), 0, rFootnote.GetNumStr().getLength());

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
        const SwTextFootnote* pTextFootnote = rFootnote.GetTextFootnote();
        if( pTextFootnote )
        {
            ww::bytes* pOld = pO;
            pO = &aOutArr;
            SfxItemSet aSet( m_pDoc->GetAttrPool(), RES_CHRATR_FONT,
                                                  RES_CHRATR_FONT );

            pCFormat = pInfo->GetCharFormat( *m_pDoc );
            aSet.Set( pCFormat->GetAttrSet() );

            pTextFootnote->GetTextNode().GetAttr( aSet, pTextFootnote->GetStart(),
                                            (pTextFootnote->GetStart()) + 1 );
            m_pAttrOutput->OutputItem( aSet.Get( RES_CHRATR_FONT ) );
            pO = pOld;
        }
        m_pChpPlc->AppendFkpEntry( Strm().Tell(), aOutArr.size(),
                                                aOutArr.data() );
    }
}

static bool lcl_IsAtTextEnd(const SwFormatFootnote& rFootnote)
{
    bool bRet = true;
    if( rFootnote.GetTextFootnote() )
    {
        sal_uInt16 nWh = static_cast< sal_uInt16 >(rFootnote.IsEndNote() ? RES_END_AT_TXTEND
                                      : RES_FTN_AT_TXTEND);
        const SwSectionNode* pSectNd = rFootnote.GetTextFootnote()->GetTextNode().
                                                        FindSectionNode();
        while( pSectNd && FTNEND_ATPGORDOCEND ==
                static_cast<const SwFormatFootnoteEndAtTextEnd&>(pSectNd->GetSection().GetFormat()->
                GetFormatAttr( nWh)).GetValue() )
            pSectNd = pSectNd->StartOfSectionNode()->FindSectionNode();

        if (!pSectNd)
            bRet = false;   // the is ftn/end collected at Page- or Doc-End
    }
    return bRet;
}

void AttributeOutputBase::TextFootnote( const SwFormatFootnote& rFootnote )
{
    sal_uInt16 nTyp;
    if ( rFootnote.IsEndNote() )
    {
        nTyp = REF_ENDNOTE;
        if ( GetExport().m_bEndAtTextEnd )
            GetExport().m_bEndAtTextEnd = lcl_IsAtTextEnd( rFootnote );
    }
    else
    {
        nTyp = REF_FOOTNOTE;
        if ( GetExport().m_bFootnoteAtTextEnd )
            GetExport().m_bFootnoteAtTextEnd = lcl_IsAtTextEnd( rFootnote );
    }

    // if any reference to this footnote/endnote then insert an internal
    // Bookmark.
    OUString sBkmkNm;
    if ( GetExport().HasRefToObject( nTyp, nullptr, rFootnote.GetTextFootnote()->GetSeqRefNo() ))
    {
        sBkmkNm = MSWordExportBase::GetBookmarkName( nTyp, nullptr,
                                    rFootnote.GetTextFootnote()->GetSeqRefNo() );
        GetExport().AppendBookmark( sBkmkNm );
    }

    TextFootnote_Impl( rFootnote );

    if ( !sBkmkNm.isEmpty() )
        GetExport().AppendBookmark( sBkmkNm ); // FIXME: Why is it added twice?  Shouldn't this one go to WW8AttributeOuput::TextFootnote_Impl()?
}

void WW8AttributeOutput::TextFootnote_Impl( const SwFormatFootnote& rFootnote )
{
    WW8_WrPlcFootnoteEdn* pFootnoteEnd;
    if ( rFootnote.IsEndNote() )
        pFootnoteEnd = m_rWW8Export.pEdn;
    else
        pFootnoteEnd = m_rWW8Export.pFootnote;

    pFootnoteEnd->Append( m_rWW8Export.Fc2Cp( m_rWW8Export.Strm().Tell() ), rFootnote );
    m_rWW8Export.WriteFootnoteBegin( rFootnote, m_rWW8Export.pO );
}

void WW8AttributeOutput::TextCharFormat( const SwFormatCharFormat& rCharFormat )
{
    if( rCharFormat.GetCharFormat() )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_CIstd );

        m_rWW8Export.InsUInt16( m_rWW8Export.GetId( rCharFormat.GetCharFormat() ) );
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
        m_rWW8Export.InsUInt16( NS_sprm::LN_CFELayout );
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
    const SwTextNode* pTextNd = nullptr;
    sal_uInt16 nNumId;
    sal_uInt8 nLvl = 0;
    if (!rNumRule.GetValue().isEmpty())
    {
        const SwNumRule* pRule = GetExport().m_pDoc->FindNumRulePtr(
                                        rNumRule.GetValue() );
        if ( pRule && USHRT_MAX != ( nNumId = GetExport().GetId( *pRule ) ) )
        {
            ++nNumId;
            if ( GetExport().m_pOutFormatNode )
            {
                if ( dynamic_cast< const SwContentNode *>( GetExport().m_pOutFormatNode ) != nullptr  )
                {
                    pTextNd = static_cast<const SwTextNode*>(GetExport().m_pOutFormatNode);

                    if( pTextNd->IsCountedInList())
                    {
                        int nLevel = pTextNd->GetActualListLevel();

                        if (nLevel < 0)
                            nLevel = 0;

                        if (nLevel >= MAXLEVEL)
                            nLevel = MAXLEVEL - 1;

                        nLvl = static_cast< sal_uInt8 >(nLevel);

                        if ( pTextNd->IsListRestart() )
                        {
                            sal_uInt16 nStartWith = static_cast< sal_uInt16 >( pTextNd->GetActualListStartValue() );
                            nNumId = GetExport().DuplicateNumRule( pRule, nLvl, nStartWith );
                            if ( USHRT_MAX != nNumId )
                                ++nNumId;
                        }
                    }
                    else
                    {
                        // #i44815# adjust numbering for numbered paragraphs
                        // without number (NO_NUMLEVEL). These paragraphs
                        // will receive a list id 0, which WW interprets as
                        // 'no number'.
                        nNumId = 0;
                    }
                }
                else if ( dynamic_cast< const SwTextFormatColl *>( GetExport().m_pOutFormatNode ) != nullptr  )
                {
                    const SwTextFormatColl* pC = static_cast<const SwTextFormatColl*>(GetExport().m_pOutFormatNode);
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

        ParaNumRule_Impl( pTextNd, nLvl, nNumId );
    }
}

void WW8AttributeOutput::ParaNumRule_Impl(const SwTextNode* /*pTextNd*/,
        sal_Int32 const nLvl, sal_Int32 const nNumId)
{
    // write sprmPIlvl and sprmPIlfo
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_PIlvl );
    m_rWW8Export.pO->push_back( ::sal::static_int_cast<sal_uInt8>(nLvl) );
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, NS_sprm::LN_PIlfo );
    SwWW8Writer::InsUInt16( *m_rWW8Export.pO, ::sal::static_int_cast<sal_uInt16>(nNumId) );
}

/* File FRMATR.HXX */

void WW8AttributeOutput::FormatFrameSize( const SwFormatFrameSize& rSize )
{
    if( m_rWW8Export.m_bOutFlyFrameAttrs )                   // Flys
    {
        if( m_rWW8Export.m_bOutGrf )
            return;                // Fly um Grafik -> Auto-Groesse

//???? was ist bei Prozentangaben ???
        if ( rSize.GetWidth() && rSize.GetWidthSizeType() == ATT_FIX_SIZE)
        {
            //"sprmPDxaWidth"
            m_rWW8Export.InsUInt16( NS_sprm::LN_PDxaWidth );
            m_rWW8Export.InsUInt16( (sal_uInt16)rSize.GetWidth() );
        }

        if ( rSize.GetHeight() )
        {
            // sprmPWHeightAbs
            m_rWW8Export.InsUInt16( NS_sprm::LN_PWHeightAbs );

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
    else if( m_rWW8Export.m_bOutPageDescs )            // PageDesc : Breite + Hoehe
    {
        if( m_rWW8Export.m_pAktPageDesc->GetLandscape() )
        {
            /*sprmSBOrientation*/
            m_rWW8Export.InsUInt16( NS_sprm::LN_SBOrientation );
            m_rWW8Export.pO->push_back( 2 );
        }

        /*sprmSXaPage*/
        m_rWW8Export.InsUInt16( NS_sprm::LN_SXaPage );
        m_rWW8Export.InsUInt16(
            msword_cast<sal_uInt16>(SvxPaperInfo::GetSloppyPaperDimension(rSize.GetWidth())));

        /*sprmSYaPage*/
        m_rWW8Export.InsUInt16( NS_sprm::LN_SYaPage );
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
    if (nPos - 2 >= sal_uLong(pFib->fcMin))
    {
        sal_uInt16 nUCode=0;

        rStrm.SeekRel(-2);
        rStrm.ReadUInt16( nUCode );
        //If the last char was a cr
        if (nUCode == 0x0d)             // CR ?
        {
            if ((nChar == 0x0c) &&
                (nPos - 4 >= sal_uLong(pFib->fcMin)))
            {
                rStrm.SeekRel(-4);
                rStrm.ReadUInt16( nUCode );
            }
            else
            {
                rStrm.SeekRel(-2);
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
        m_pPiece->SetParaBreak();
        m_pPapPlc->AppendFkpEntry(rStrm.Tell());
        m_pChpPlc->AppendFkpEntry(rStrm.Tell());
        nRetPos = rStrm.Tell();
    }
    return nRetPos;
}

void WW8AttributeOutput::TableRowEnd(sal_uInt32 nDepth)
{
    if ( nDepth == 1 )
        m_rWW8Export.WriteChar( 0x07 );
    else if ( nDepth > 1 )
        m_rWW8Export.WriteChar( 0x0d );

    //Technically in a word document this is a different value for a row ends
    //that are not row ends directly after a cell with a graphic. But it
    //doesn't seem to make a difference
    //pMagicTable->Append(Fc2Cp(Strm().Tell()),0x1B6);
}

void AttributeOutputBase::FormatPageDescription( const SwFormatPageDesc& rPageDesc )
{
    if ( GetExport().m_bStyDef && GetExport().m_pOutFormatNode && dynamic_cast< const SwTextFormatColl *>( GetExport().m_pOutFormatNode ) != nullptr )
    {
        const SwTextFormatColl* pC = static_cast<const SwTextFormatColl*>(GetExport().m_pOutFormatNode);
        if ( (SfxItemState::SET != pC->GetItemState( RES_BREAK, false ) ) && rPageDesc.KnowsPageDesc() )
            FormatBreak( SvxFormatBreakItem( SVX_BREAK_PAGE_BEFORE, RES_BREAK ) );
    }
}

void WW8AttributeOutput::PageBreakBefore( bool bBreak )
{
    // sprmPPageBreakBefore/sprmPFPageBreakBefore
    m_rWW8Export.InsUInt16( NS_sprm::LN_PFPageBreakBefore );

    m_rWW8Export.pO->push_back( bBreak ? 1 : 0 );
}

// Breaks schreiben nichts in das Ausgabe-Feld rWrt.pO,
// sondern nur in den Text-Stream ( Bedingung dafuer, dass sie von Out_Break...
// gerufen werden duerfen )
void AttributeOutputBase::FormatBreak( const SvxFormatBreakItem& rBreak )
{
    if ( GetExport().m_bStyDef )
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
    else if ( !GetExport().m_pParentFrame )
    {
        sal_uInt8 nC = 0;
        bool bBefore = false;
        // #i76300# - Note: Can only be <true>, if <bBefore> equals <false>.
        bool bCheckForFollowPageDesc = false;

        switch ( rBreak.GetBreak() )
        {
            case SVX_BREAK_NONE:                                // Ausgeschaltet
                if ( !GetExport().m_bBreakBefore )
                    PageBreakBefore( false );
                return;

            case SVX_BREAK_COLUMN_BEFORE:                       // ColumnBreak
                bBefore = true;
                SAL_FALLTHROUGH;
            case SVX_BREAK_COLUMN_AFTER:
            case SVX_BREAK_COLUMN_BOTH:
                if ( GetExport().Sections().CurrentNumberOfColumns( *GetExport().m_pDoc ) > 1 || GetExport().SupportsOneColumnBreak() )
                {
                    nC = msword::ColumnBreak;
                }
                break;

            case SVX_BREAK_PAGE_BEFORE:                         // PageBreak
                // From now on(fix for #i77900#) we prefer to save a page break
                // as paragraph attribute (if the exporter is OK with that),
                // this has to be done after the export of the paragraph ( =>
                // !GetExport().bBreakBefore )
                if (GetExport().PreferPageBreakBefore())
                {
                    if (!GetExport().m_bBreakBefore)
                        PageBreakBefore(true);
                    break;
                }
            case SVX_BREAK_PAGE_AFTER:
            case SVX_BREAK_PAGE_BOTH:
                nC = msword::PageBreak;
                // #i76300# - check for follow page description,
                // if current writing attributes of a paragraph.
                if ( dynamic_cast< const SwTextNode* >( GetExport().m_pOutFormatNode ) &&
                     GetExport().GetCurItemSet() )
                {
                    bCheckForFollowPageDesc = true;
                }
                break;

            default:
                break;
        }

        if ( (( bBefore != GetExport().m_bBreakBefore ) && ( nC == msword::PageBreak)) ||
             (( bBefore == GetExport().m_bBreakBefore ) && ( nC == msword::ColumnBreak)) )
        {
            // #i76300#
            bool bFollowPageDescWritten = false;
            if ( bCheckForFollowPageDesc && !bBefore )
            {
                bFollowPageDescWritten =
                    GetExport().OutputFollowPageDesc( GetExport().GetCurItemSet(),
                            dynamic_cast<const SwTextNode*>( GetExport().m_pOutFormatNode ) );
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
    MSWordStyles * pStyles = GetExport().m_pStyles;
    const SwFormat * pSwFormat = pStyles->GetSwFormat(0);

    sal_uInt32 nPageCharSize = 0;

    if (pSwFormat != nullptr)
    {
        nPageCharSize = ItemGet<SvxFontHeightItem>
            (*pSwFormat, RES_CHRATR_FONTSIZE).GetHeight();
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
    if (m_rWW8Export.m_bOutPageDescs)
    {
        sal_uInt16 nGridType = 0;
        switch ( rGrid.GetGridType() )
        {
            default:
                OSL_FAIL("Unknown grid type");
                SAL_FALLTHROUGH;
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
    if ( m_rWW8Export.m_bOutPageDescs )
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
            m_rWW8Export.InsUInt16( m_rWW8Export.m_bOutFirstPage
                    ? NS_sprm::LN_SDmBinFirst : NS_sprm::LN_SDmBinOther );

            m_rWW8Export.InsUInt16( nVal );
        }
    }
}

void WW8AttributeOutput::FormatLRSpace( const SvxLRSpaceItem& rLR )
{
    // Flys fehlen noch ( siehe RTF )

    if ( m_rWW8Export.m_bOutFlyFrameAttrs )                   // Flys
    {
        // sprmPDxaFromText10
        m_rWW8Export.InsUInt16( NS_sprm::LN_PDxaFromText10 );
        // Mittelwert nehmen, da WW nur 1 Wert kennt
        m_rWW8Export.InsUInt16( (sal_uInt16) ( ( rLR.GetLeft() + rLR.GetRight() ) / 2 ) );
    }
    else if ( m_rWW8Export.m_bOutPageDescs )                // PageDescs
    {
        sal_uInt16 nLDist, nRDist;
        const SfxPoolItem* pItem = m_rWW8Export.HasItem( RES_BOX );
        if ( pItem )
        {
            nRDist = static_cast<const SvxBoxItem*>(pItem)->CalcLineSpace( SvxBoxItemLine::LEFT );
            nLDist = static_cast<const SvxBoxItem*>(pItem)->CalcLineSpace( SvxBoxItemLine::RIGHT );
        }
        else
            nLDist = nRDist = 0;
        nLDist = nLDist + (sal_uInt16)rLR.GetLeft();
        nRDist = nRDist + (sal_uInt16)rLR.GetRight();

        // sprmSDxaLeft
        m_rWW8Export.InsUInt16( NS_sprm::LN_SDxaLeft );
        m_rWW8Export.InsUInt16( nLDist );

        // sprmSDxaRight
        m_rWW8Export.InsUInt16( NS_sprm::LN_SDxaRight );
        m_rWW8Export.InsUInt16( nRDist );
    }
    else
    {                                          // normale Absaetze
        // sprmPDxaLeft
        m_rWW8Export.InsUInt16( 0x845E );        //asian version ?
        m_rWW8Export.InsUInt16( (sal_uInt16)rLR.GetTextLeft() );

        // sprmPDxaRight
        m_rWW8Export.InsUInt16( 0x845D );        //asian version ?
        m_rWW8Export.InsUInt16( (sal_uInt16)rLR.GetRight() );

        // sprmPDxaLeft1
        m_rWW8Export.InsUInt16( 0x8460 );        //asian version ?
        m_rWW8Export.InsUInt16( rLR.GetTextFirstLineOfst() );
    }
}

void WW8AttributeOutput::FormatULSpace( const SvxULSpaceItem& rUL )
{
    // Flys fehlen noch ( siehe RTF )

    if ( m_rWW8Export.m_bOutFlyFrameAttrs )                   // Flys
    {
        // sprmPDyaFromText
        m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaFromText );
        // Mittelwert nehmen, da WW nur 1 Wert kennt
        m_rWW8Export.InsUInt16( (sal_uInt16) ( ( rUL.GetUpper() + rUL.GetLower() ) / 2 ) );
    }
    else if ( m_rWW8Export.m_bOutPageDescs )            // Page-UL
    {
        OSL_ENSURE( m_rWW8Export.GetCurItemSet(), "Impossible" );
        if ( !m_rWW8Export.GetCurItemSet() )
            return;

        HdFtDistanceGlue aDistances( *m_rWW8Export.GetCurItemSet() );

        if ( aDistances.HasHeader() )
        {
            //sprmSDyaHdrTop
            m_rWW8Export.InsUInt16( NS_sprm::LN_SDyaHdrTop );
            m_rWW8Export.InsUInt16( aDistances.dyaHdrTop );
        }

        // sprmSDyaTop
        m_rWW8Export.InsUInt16( NS_sprm::LN_SDyaTop );
        m_rWW8Export.InsUInt16( aDistances.dyaTop );

        if ( aDistances.HasFooter() )
        {
            //sprmSDyaHdrBottom
            m_rWW8Export.InsUInt16( NS_sprm::LN_SDyaHdrBottom );
            m_rWW8Export.InsUInt16( aDistances.dyaHdrBottom );
        }

        //sprmSDyaBottom
        m_rWW8Export.InsUInt16( NS_sprm::LN_SDyaBottom );
        m_rWW8Export.InsUInt16( aDistances.dyaBottom );
    }
    else
    {
        // sprmPDyaBefore
        m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaBefore );
        m_rWW8Export.InsUInt16( rUL.GetUpper() );
        // sprmPDyaAfter
        m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaAfter );
        m_rWW8Export.InsUInt16( rUL.GetLower() );
        // sprmPFContextualSpacing
        if (rUL.GetContext())
        {
            m_rWW8Export.InsUInt16(NS_sprm::LN_PContextualSpacing);
            m_rWW8Export.pO->push_back( (sal_uInt8)rUL.GetContext() );
        }
    }
}

// Print, Opaque, Protect fehlen noch

void WW8AttributeOutput::FormatSurround( const SwFormatSurround& rSurround )
{
    if ( m_rWW8Export.m_bOutFlyFrameAttrs )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_PWr );

        m_rWW8Export.pO->push_back(
                ( SURROUND_NONE != rSurround.GetSurround() ) ? 2 : 1 );
    }
}

void WW8AttributeOutput::FormatVertOrientation( const SwFormatVertOrient& rFlyVert )
{
//!!!! Ankertyp und entsprechende Umrechnung fehlt noch

    if ( m_rWW8Export.m_bOutFlyFrameAttrs )
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
        m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaAbs );
        m_rWW8Export.InsUInt16( nPos );
    }
}

void WW8AttributeOutput::FormatHorizOrientation( const SwFormatHoriOrient& rFlyHori )
{
    if ( !m_rWW8Export.m_pParentFrame )
    {
        OSL_ENSURE( m_rWW8Export.m_pParentFrame, "HoriOrient without mpParentFrame !!" );
        return;
    }

//!!!! Ankertyp und entsprechende Umrechnung fehlt noch
    if ( m_rWW8Export.m_bOutFlyFrameAttrs )
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
        m_rWW8Export.InsUInt16( NS_sprm::LN_PDxaAbs );
        m_rWW8Export.InsUInt16( nPos );
    }
}

void WW8AttributeOutput::FormatAnchor( const SwFormatAnchor& rAnchor )
{
    OSL_ENSURE( m_rWW8Export.m_pParentFrame, "Anchor without mpParentFrame !!" );

    if ( m_rWW8Export.m_bOutFlyFrameAttrs )
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
        m_rWW8Export.InsUInt16( NS_sprm::LN_PPc );
        m_rWW8Export.pO->push_back( nP );
    }
}

void WW8AttributeOutput::FormatBackground( const SvxBrushItem& rBrush )
{
    // WW cannot have background in a section
    if ( !m_rWW8Export.m_bOutPageDescs )
    {
        WW8_SHD aSHD;

        WW8Export::TransBrush( rBrush.GetColor(), aSHD );
        // sprmPShd
        m_rWW8Export.InsUInt16( NS_sprm::LN_PShd );
        m_rWW8Export.InsUInt16( aSHD.GetValue() );

        // Quite a few unknowns, some might be transparency or something
        // of that nature...
        m_rWW8Export.InsUInt16( 0xC64D );
        m_rWW8Export.pO->push_back( 10 );
        m_rWW8Export.InsUInt32( 0xFF000000 );
        m_rWW8Export.InsUInt32( SuitableBGColor( rBrush.GetColor().GetColor() ) );
        m_rWW8Export.InsUInt16( 0x0000 );
    }
}

void WW8AttributeOutput::FormatFillStyle( const XFillStyleItem& /*rFillStyle*/ )
{
}

void WW8AttributeOutput::FormatFillGradient( const XFillGradientItem& /*rFillGradient*/ )
{
}

WW8_BRCVer9 WW8Export::TranslateBorderLine(const SvxBorderLine& rLine,
    sal_uInt16 nDist, bool bShadow)
{
    sal_uInt32 nColBGR = 0;
    sal_uInt16 nWidth = ::editeng::ConvertBorderWidthToWord(
            rLine.GetBorderLineStyle(), rLine.GetWidth());
    sal_uInt8 brcType = 0;

    if( nWidth )                                // Linie ?
    {
        // BRC.brcType
        brcType = 0;
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
            case table::BorderLineStyle::FINE_DASHED:
                brcType = 22;
                break;
            case table::BorderLineStyle::DASH_DOT:
                brcType = 8;
                break;
            case table::BorderLineStyle::DASH_DOT_DOT:
                brcType = 9;
                break;
            default:
                break;
        }

        // convert width from twips (1/20 pt) to eighths of a point
        nWidth = (( nWidth * 8 ) + 10 ) / 20;
        if( 0xff < nWidth )
            nWidth = 0xff;

        if( 0 == nWidth )                       // ganz duenne Linie
            nWidth = 1;                         //       nicht weglassen

        // BRC.cv
        nColBGR = wwUtility::RGBToBGR(rLine.GetColor().GetRGBColor());
    }

    // BRC.dptSpace
    sal_uInt16 nLDist = nDist;
    nLDist /= 20;               // Masseinheit : pt
    if( nLDist > 0x1f )
        nLDist = 0x1f;

    return WW8_BRCVer9(nColBGR, sal_uInt8(nWidth), brcType, sal_uInt8(nLDist),
        bShadow, false);
}

// MakeBorderLine() bekommt einen WW8Bytes* uebergeben, um die Funktion
// auch fuer die Tabellen-Umrandungen zu benutzen.
// Wenn nSprmNo == 0, dann wird der Opcode nicht ausgegeben.
// bShadow darf bei Tabellenzellen *nicht* gesetzt sein !
void WW8Export::Out_BorderLine(ww::bytes& rO, const SvxBorderLine* pLine,
    sal_uInt16 nDist, sal_uInt16 nSprmNo, sal_uInt16 nSprmNoVer9, bool bShadow)
{
    OSL_ENSURE( ( nSprmNo == 0 ) ||
            ( nSprmNo >= 38 && nSprmNo <= 41 ) ||
            ( nSprmNo >= NS_sprm::LN_PBrcTop80
              && nSprmNo <= NS_sprm::LN_PBrcRight80 ) ||
            ( nSprmNo >= NS_sprm::LN_SBrcTop80
              && nSprmNo <= NS_sprm::LN_SBrcRight80 ),
            "Sprm for border out is of range" );

    WW8_BRCVer9 aBrcVer9;
    WW8_BRC aBrcVer8;

    if( pLine && pLine->GetBorderLineStyle() != table::BorderLineStyle::NONE )
    {
        aBrcVer9 = TranslateBorderLine( *pLine, nDist, bShadow );
        sal_uInt8 ico = msfilter::util::TransColToIco( msfilter::util::BGRToRGB(aBrcVer9.cv()) );
        aBrcVer8 = WW8_BRC( aBrcVer9.dptLineWidth(), aBrcVer9.brcType(), ico,
            aBrcVer9.dptSpace(), aBrcVer9.fShadow(), aBrcVer9.fFrame() );
    }

    // WW97-SprmIds
    if ( nSprmNo != 0 )
        SwWW8Writer::InsUInt16( rO, nSprmNo );

    rO.insert( rO.end(), aBrcVer8.aBits1, aBrcVer8.aBits2+2 );

    if ( nSprmNoVer9 != 0 )
    {
        SwWW8Writer::InsUInt16( rO, nSprmNoVer9 );
        rO.push_back(sizeof(WW8_BRCVer9));
        rO.insert( rO.end(), aBrcVer9.aBits1, aBrcVer9.aBits2+4);
    }
}

// FormatBox1() ist fuer alle Boxen ausser in Tabellen.
// es wird pO des WW8Writers genommen
void WW8Export::Out_SwFormatBox(const SvxBoxItem& rBox, bool bShadow)
{
    static const SvxBoxItemLine aBorders[] =
    {
        SvxBoxItemLine::TOP, SvxBoxItemLine::LEFT, SvxBoxItemLine::BOTTOM, SvxBoxItemLine::RIGHT
    };
    static const sal_uInt16 aPBrc[] =
    {
        // WW8 SPRMs
        NS_sprm::LN_PBrcTop80, NS_sprm::LN_PBrcLeft80,
        NS_sprm::LN_PBrcBottom80, NS_sprm::LN_PBrcRight80,
        // WW9 SPRMs
        NS_sprm::LN_PBrcTop, NS_sprm::LN_PBrcLeft,
        NS_sprm::LN_PBrcBottom, NS_sprm::LN_PBrcRight
    };
    static const sal_uInt16 aSBrc[] =
    {
        // WW8 SPRMs
        NS_sprm::LN_SBrcTop80, NS_sprm::LN_SBrcLeft80,
        NS_sprm::LN_SBrcBottom80, NS_sprm::LN_SBrcRight80,
        // WW9 SPRMs
        NS_sprm::LN_SBrcTop, NS_sprm::LN_SBrcLeft,
        NS_sprm::LN_SBrcBottom, NS_sprm::LN_SBrcRight
    };

    const SvxBoxItemLine* pBrd = aBorders;
    for( sal_uInt16 i = 0; i < 4; ++i, ++pBrd )
    {
        const SvxBorderLine* pLn = rBox.GetLine( *pBrd );

        sal_uInt16 nSprmNo, nSprmNoVer9 = 0;
        if (m_bOutPageDescs)
        {
            nSprmNo = aSBrc[i];
            nSprmNoVer9 = aSBrc[i+4];
        }
        else
        {
            nSprmNo = aPBrc[i];
            nSprmNoVer9 = aPBrc[i+4];
        }

        Out_BorderLine( *pO, pLn, rBox.GetDistance( *pBrd ), nSprmNo,
            nSprmNoVer9, bShadow );
    }
}

// FormatBox2() ist fuer TC-Strukturen in Tabellen. Der Sprm-Opcode
// wird nicht geschrieben, da es in der TC-Structur ohne Opcode gepackt ist.
// dxpSpace wird immer 0, da WW das in Tabellen so verlangt
// ( Tabellenumrandungen fransen sonst aus )
// Ein WW8Bytes-Ptr wird als Ausgabe-Parameter uebergeben

void WW8Export::Out_SwFormatTableBox( ww::bytes& rO, const SvxBoxItem * pBox )
{
    // moeglich und vielleicht besser waere 0xffff
    static const SvxBoxItemLine aBorders[] =
    {
        SvxBoxItemLine::TOP, SvxBoxItemLine::LEFT, SvxBoxItemLine::BOTTOM, SvxBoxItemLine::RIGHT
    };
    static const SvxBorderLine aBorderLine;

    for( int i = 0; i < 4; ++i )
    {
        const SvxBorderLine* pLn;
        if (pBox != nullptr)
            pLn = pBox->GetLine( aBorders[i] );
        else
            pLn = & aBorderLine;

        Out_BorderLine(rO, pLn, 0, 0, 0, false);
    }
}

void WW8Export::Out_CellRangeBorders( const SvxBoxItem * pBox, sal_uInt8 nStart,
       sal_uInt8 nLimit )
{
    static const SvxBoxItemLine aBorders[] =
    {
        SvxBoxItemLine::TOP, SvxBoxItemLine::LEFT, SvxBoxItemLine::BOTTOM, SvxBoxItemLine::RIGHT
    };

    for( int i = 0; i < 4; ++i )
    {
        const SvxBorderLine* pLn = nullptr;
        if (pBox != nullptr)
            pLn = pBox->GetLine( aBorders[i] );
        if (!pLn)
            continue;

        InsUInt16( NS_sprm::LN_TSetBrc );
        pO->push_back( 11 );
        pO->push_back( nStart );
        pO->push_back( nLimit );
        pO->push_back( 1<<i );
        WW8_BRCVer9 aBrcVer9 = TranslateBorderLine( *pLn, 0, false );
        pO->insert( pO->end(), aBrcVer9.aBits1, aBrcVer9.aBits2+4 );
    }
}

void WW8AttributeOutput::FormatBox( const SvxBoxItem& rBox )
{
    // Fly um Grafik-> keine Umrandung hier, da
    // der GrafikHeader bereits die Umrandung hat
    if ( !m_rWW8Export.m_bOutGrf )
    {
        bool bShadow = false;
        const SfxPoolItem* pItem = m_rWW8Export.HasItem( RES_SHADOW );
        if ( pItem )
        {
            const SvxShadowItem* p = static_cast<const SvxShadowItem*>(pItem);
            bShadow = ( p->GetLocation() != SVX_SHADOW_NONE )
                      && ( p->GetWidth() != 0 );
        }

        m_rWW8Export.Out_SwFormatBox( rBox, bShadow );
    }
}

SwTwips WW8Export::CurrentPageWidth(SwTwips &rLeft, SwTwips &rRight) const
{
    const SwFrameFormat* pFormat = m_pAktPageDesc ? &m_pAktPageDesc->GetMaster()
        : &m_pDoc->GetPageDesc(0).GetMaster();

    const SvxLRSpaceItem& rLR = pFormat->GetLRSpace();
    SwTwips nPageSize = pFormat->GetFrameSize().GetWidth();
    rLeft = rLR.GetLeft();
    rRight = rLR.GetRight();
    return nPageSize;
}

void WW8AttributeOutput::FormatColumns_Impl( sal_uInt16 nCols, const SwFormatCol & rCol, bool bEven, SwTwips nPageSize )
{
    // CColumns
    m_rWW8Export.InsUInt16( NS_sprm::LN_SCcolumns );
    m_rWW8Export.InsUInt16( nCols - 1 );

    // DxaColumns
    m_rWW8Export.InsUInt16( NS_sprm::LN_SDxaColumns );
    m_rWW8Export.InsUInt16( rCol.GetGutterWidth( true ) );

    // LBetween
    m_rWW8Export.InsUInt16( NS_sprm::LN_SLBetween );
    m_rWW8Export.pO->push_back( COLADJ_NONE == rCol.GetLineAdj(  )? 0 : 1 );

    const SwColumns & rColumns = rCol.GetColumns(  );

    // FEvenlySpaced
    m_rWW8Export.InsUInt16( NS_sprm::LN_SFEvenlySpaced );
    m_rWW8Export.pO->push_back( bEven ? 1 : 0 );

    if ( !bEven )
    {
        for ( sal_uInt16 n = 0; n < nCols; ++n )
        {
            //sprmSDxaColWidth
            m_rWW8Export.InsUInt16( NS_sprm::LN_SDxaColWidth );
            m_rWW8Export.pO->push_back( static_cast<sal_uInt8>(n) );
            m_rWW8Export.InsUInt16( rCol.
                                    CalcPrtColWidth( n,
                                                     ( sal_uInt16 ) nPageSize ) );

            if ( n + 1 != nCols )
            {
                //sprmSDxaColSpacing
                m_rWW8Export.InsUInt16( NS_sprm::LN_SDxaColSpacing );
                m_rWW8Export.pO->push_back( static_cast<sal_uInt8>(n) );
                m_rWW8Export.InsUInt16( rColumns[n].GetRight(  ) +
                                        rColumns[n + 1].GetLeft(  ) );
            }
        }
    }
}

void AttributeOutputBase::FormatColumns( const SwFormatCol& rCol )
{
    const SwColumns& rColumns = rCol.GetColumns();

    sal_uInt16 nCols = rColumns.size();
    if ( 1 < nCols && !GetExport( ).m_bOutFlyFrameAttrs )
    {
        // dann besorge mal die Seitenbreite ohne Raender !!

        const SwFrameFormat* pFormat = GetExport( ).m_pAktPageDesc ? &GetExport( ).m_pAktPageDesc->GetMaster() : &const_cast<const SwDoc *>(GetExport( ).m_pDoc)->GetPageDesc(0).GetMaster();
        const SvxFrameDirectionItem &frameDirection = pFormat->GetFrameDir();
        SwTwips nPageSize;
        if ( frameDirection.GetValue() == FRMDIR_VERT_TOP_RIGHT || frameDirection.GetValue() == FRMDIR_VERT_TOP_LEFT )
        {
            const SvxULSpaceItem &rUL = pFormat->GetULSpace();
            nPageSize = pFormat->GetFrameSize().GetHeight();
            nPageSize -= rUL.GetUpper() + rUL.GetLower();

            const SwFormatHeader *header = dynamic_cast<const SwFormatHeader *>(pFormat->GetAttrSet().GetItem(RES_HEADER));
            if ( header )
            {
                const SwFrameFormat *headerFormat = header->GetHeaderFormat();
                if (headerFormat)
                {
                    nPageSize -= headerFormat->GetFrameSize().GetHeight();
                }
            }
            const SwFormatFooter *footer = dynamic_cast<const SwFormatFooter *>(pFormat->GetAttrSet().GetItem(RES_FOOTER));
            if ( footer )
            {
                const SwFrameFormat *footerFormat = footer->GetFooterFormat();
                if ( footerFormat )
                {
                    nPageSize -= footerFormat->GetFrameSize().GetHeight();
                }
            }
        }
        else
        {
            const SvxLRSpaceItem &rLR = pFormat->GetLRSpace();
            nPageSize = pFormat->GetFrameSize().GetWidth();
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
void WW8AttributeOutput::FormatKeep( const SvxFormatKeepItem& rKeep )
{
    // sprmFKeepFollow
    m_rWW8Export.InsUInt16( NS_sprm::LN_PFKeepFollow );

    m_rWW8Export.pO->push_back( rKeep.GetValue() ? 1 : 0 );
}

// exclude a paragraph from Line Numbering
void WW8AttributeOutput::FormatLineNumbering( const SwFormatLineNumber& rNumbering )
{
    // sprmPFNoLineNumb
    m_rWW8Export.InsUInt16( NS_sprm::LN_PFNoLineNumb );

    m_rWW8Export.pO->push_back( rNumbering.IsCount() ? 0 : 1 );
}

/* File PARATR.HXX  */

void WW8AttributeOutput::ParaLineSpacing_Impl( short nSpace, short nMulti )
{
    // sprmPDyaLine
    m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaLine );

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
        case SVX_LINE_SPACE_FIX: // Fix
            nSpace = -(short)rSpacing.GetLineHeight();
            break;
        case SVX_LINE_SPACE_MIN: // At least
            nSpace = (short)rSpacing.GetLineHeight();
            break;
        case SVX_LINE_SPACE_AUTO:
        {
            if( rSpacing.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_FIX ) // Leading
            {
                // gibt es aber nicht in WW - also wie kommt man an
                // die MaxLineHeight heran?
                nSpace = (short)rSpacing.GetInterLineSpace();
                sal_uInt16 nScript =
                    i18n::ScriptType::LATIN;
                const SwAttrSet *pSet = nullptr;
                if ( GetExport().m_pOutFormatNode && dynamic_cast< const SwFormat *>( GetExport().m_pOutFormatNode ) != nullptr  )
                {
                    const SwFormat *pFormat = static_cast<const SwFormat*>( GetExport().m_pOutFormatNode );
                    pSet = &pFormat->GetAttrSet();
                }
                else if ( GetExport().m_pOutFormatNode && dynamic_cast< const SwTextNode *>( GetExport().m_pOutFormatNode ) != nullptr  )
                {
                    const SwTextNode* pNd = static_cast<const SwTextNode*>(GetExport().m_pOutFormatNode);
                    pSet = &pNd->GetSwAttrSet();
                    if ( g_pBreakIt->GetBreakIter().is() )
                    {
                        nScript = g_pBreakIt->GetBreakIter()->
                            getScriptType(pNd->GetText(), 0);
                    }
                }
                OSL_ENSURE( pSet, "No attrset for lineheight :-(" );
                if ( pSet )
                {
                    nSpace = nSpace + (short)( AttrSetToLineHeight( GetExport().m_pDoc->getIDocumentSettingAccess(),
                        *pSet, *Application::GetDefaultDevice(), nScript ) );
                }
            }
            else // Proportional
            {
                nSpace = (short)( ( 240L * rSpacing.GetPropLineSpace() ) / 100L );
                nMulti = 1;
                break;
            }
        }
        break;
    }
    // if nSpace is negative, it is a fixed size in 1/20 of a point
    // if nSpace is positive and nMulti is 1, it is 1/240 of a single line height
    // otherwise, it is a minimum size in 1/20 of a point
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
        m_rWW8Export.InsUInt16( NS_sprm::LN_PJc );
        m_rWW8Export.pO->push_back( nAdj );

        /*
        Sadly for left to right paragraphs both these values are the same,
        for right to left paragraphs the bidi one is the reverse of the
        normal one.
        */
        m_rWW8Export.InsUInt16( NS_sprm::LN_PJcExtra ); //bidi version ?
        bool bBiDiSwap = false;
        if ( m_rWW8Export.m_pOutFormatNode )
        {
            short nDirection = FRMDIR_HORI_LEFT_TOP;
            if ( dynamic_cast< const SwTextNode *>( m_rWW8Export.m_pOutFormatNode )  != nullptr )
            {
                SwPosition aPos(*static_cast<const SwContentNode*>(m_rWW8Export.m_pOutFormatNode));
                nDirection = m_rWW8Export.m_pDoc->GetTextDirection(aPos);
            }
            else if ( dynamic_cast< const SwTextFormatColl *>( m_rWW8Export.m_pOutFormatNode ) != nullptr  )
            {
                const SwTextFormatColl* pC =
                    static_cast<const SwTextFormatColl*>(m_rWW8Export.m_pOutFormatNode);
                const SvxFrameDirectionItem &rItem =
                    ItemGet<SvxFrameDirectionItem>(*pC, RES_FRAMEDIR);
                nDirection = rItem.GetValue();
            }
            if ( ( nDirection == FRMDIR_HORI_RIGHT_TOP ) ||
                 ( nDirection == FRMDIR_ENVIRONMENT && AllSettings::GetLayoutRTL() ) )
            {
                bBiDiSwap = true;
            }
        }

        if ( bBiDiSwap )
            m_rWW8Export.pO->push_back( nAdjBiDi );
        else
            m_rWW8Export.pO->push_back( nAdj );
    }
}

void WW8AttributeOutput::FormatFrameDirection( const SvxFrameDirectionItem& rDirection )
{
    sal_uInt16 nTextFlow=0;
    bool bBiDi = false;
    short nDir = rDirection.GetValue();

    if ( nDir == FRMDIR_ENVIRONMENT )
    {
        if ( m_rWW8Export.m_bOutPageDescs )
            nDir = m_rWW8Export.GetCurrentPageDirection();
        else if ( m_rWW8Export.m_pOutFormatNode )
        {
            if ( m_rWW8Export.m_bOutFlyFrameAttrs )  //frame
            {
                nDir = m_rWW8Export.TrueFrameDirection(
                    *static_cast<const SwFrameFormat*>(m_rWW8Export.m_pOutFormatNode) );
            }
            else if ( dynamic_cast< const SwContentNode *>( m_rWW8Export.m_pOutFormatNode ) !=  nullptr )   //pagagraph
            {
                const SwContentNode* pNd =
                    static_cast<const SwContentNode*>(m_rWW8Export.m_pOutFormatNode);
                SwPosition aPos( *pNd );
                nDir = m_rWW8Export.m_pDoc->GetTextDirection( aPos );
            }
            else if ( dynamic_cast< const SwTextFormatColl *>( m_rWW8Export.m_pOutFormatNode ) !=  nullptr )
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
            SAL_FALLTHROUGH;
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

    if ( m_rWW8Export.m_bOutPageDescs )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_STextFlow );
        m_rWW8Export.InsUInt16( nTextFlow );
        m_rWW8Export.InsUInt16( NS_sprm::LN_SFBiDi );
        m_rWW8Export.pO->push_back( bBiDi ? 1 : 0 );
    }
    else if ( !m_rWW8Export.m_bOutFlyFrameAttrs )  //paragraph/style
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_PFBiDi );
        m_rWW8Export.pO->push_back( bBiDi ? 1 : 0 );
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

// "Separate paragraphs"
void WW8AttributeOutput::ParaSplit( const SvxFormatSplitItem& rSplit )
{
    // sprmPFKeep
    m_rWW8Export.InsUInt16( NS_sprm::LN_PFKeep );
    m_rWW8Export.pO->push_back( rSplit.GetValue() ? 0 : 1 );
}

//  Es wird nur das Item "SvxWidowItem" und nicht die Orphans uebersetzt,
//  da es fuer beides im WW nur ein Attribut "Absatzkontrolle" gibt und
//  im SW wahrscheinlich vom Anwender immer Beide oder keiner gesetzt werden.
void WW8AttributeOutput::ParaWidows( const SvxWidowsItem& rWidows )
{
// sprmPFWidowControl
    m_rWW8Export.InsUInt16( NS_sprm::LN_PFWidowControl );
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
    pDel = nDelMax ? new sal_uInt8[nDelMax * 2] : nullptr;
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

    pAddTyp[nAdd] = nPara;
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
    if (!nAdd && !nDel) //If it's a no-op
        return;
    OSL_ENSURE(nAdd <= 255, "more than 255 added tabstops?");
    OSL_ENSURE(nDel <= 255, "more than 244 removed tabstops?");
    if (nAdd > 255)
        nAdd = 255;
    if (nDel > 255)
        nDel = 255;

    sal_uInt16 nSiz = 2 * nDel + 3 * nAdd + 2;
    if (nSiz > 255)
        nSiz = 255;

    rWrt.InsUInt16(NS_sprm::LN_PChgTabsPapx);
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
            pTO = nullptr;
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
            pTN = nullptr;
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
    } while( true );

    aTab.PutAll( rWrt );
}

void WW8AttributeOutput::ParaTabStop( const SvxTabStopItem& rTabStops )
{
    const bool bTabsRelativeToIndex = m_rWW8Export.m_pCurPam->GetDoc()->getIDocumentSettingAccess().get( DocumentSettingId::TABS_RELATIVE_TO_INDENT );

    long nCurrentLeft = 0;
    if ( bTabsRelativeToIndex )
    {
        const SfxPoolItem* pLR = m_rWW8Export.HasItem( RES_LR_SPACE );

        if ( pLR != nullptr )
            nCurrentLeft = static_cast<const SvxLRSpaceItem*>(pLR)->GetTextLeft();
    }

    // #i100264#
    if ( m_rWW8Export.m_bStyDef &&
         m_rWW8Export.m_pCurrentStyle != nullptr &&
         m_rWW8Export.m_pCurrentStyle->DerivedFrom() != nullptr )
    {
        SvxTabStopItem aParentTabs( 0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
        const SwFormat *pParentStyle = m_rWW8Export.m_pCurrentStyle->DerivedFrom();
        {
            if (const SvxTabStopItem* pParentTabs = pParentStyle->GetAttrSet().GetItem<SvxTabStopItem>(RES_PARATR_TABSTOP))
            {
                aParentTabs.Insert( pParentTabs );
            }
        }

        // #i120938# - consider left indentation of style and its parent style
        long nParentLeft = 0;
        if ( bTabsRelativeToIndex )
        {
            const SvxLRSpaceItem &rStyleLR = ItemGet<SvxLRSpaceItem>( pParentStyle->GetAttrSet(), RES_LR_SPACE );
            nParentLeft = rStyleLR.GetTextLeft();
        }

        ParaTabStopDelAdd( m_rWW8Export, aParentTabs, nParentLeft, rTabStops, nCurrentLeft );
        return;
    }

    const SvxTabStopItem* pStyleTabs = nullptr;
    if ( !m_rWW8Export.m_bStyDef && m_rWW8Export.m_pStyAttr )
    {
        pStyleTabs = m_rWW8Export.m_pStyAttr->GetItem<SvxTabStopItem>(RES_PARATR_TABSTOP);
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
            const SvxLRSpaceItem &rStyleLR = ItemGet<SvxLRSpaceItem>(*m_rWW8Export.m_pStyAttr, RES_LR_SPACE);
            nStyleLeft = rStyleLR.GetTextLeft();
        }

        ParaTabStopDelAdd( m_rWW8Export,
                           *pStyleTabs, nStyleLeft,
                           rTabStops, nCurrentLeft);
    }
}

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
            CharBackgroundBase( static_cast< const SvxBrushItem& >( rHt ) );
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
        case RES_CHRATR_BIDIRTL:
            CharBidiRTL( static_cast< const SfxPoolItem& >( rHt ) );
            break;
        case RES_CHRATR_IDCTHINT:
            CharIdctHint( static_cast< const SfxPoolItem& >( rHt ) );
            break;
        case RES_TXTATR_INETFMT:
            TextINetFormat( static_cast< const SwFormatINetFormat& >( rHt ) );
            break;
        case RES_TXTATR_CHARFMT:
            TextCharFormat( static_cast< const SwFormatCharFormat& >( rHt ) );
            break;

        case RES_TXTATR_FIELD:
        case RES_TXTATR_ANNOTATION:
        case RES_TXTATR_INPUTFIELD:
            TextField( static_cast< const SwFormatField& >( rHt ) );
            break;

        case RES_TXTATR_FLYCNT:
            TextFlyContent( static_cast< const SwFormatFlyCnt& >( rHt ) );
            break;
        case RES_TXTATR_FTN:
            TextFootnote( static_cast< const SwFormatFootnote& >( rHt ) );
            break;

        case RES_PARATR_LINESPACING:
            ParaLineSpacing( static_cast< const SvxLineSpacingItem& >( rHt ) );
            break;
        case RES_PARATR_ADJUST:
            ParaAdjust( static_cast< const SvxAdjustItem& >( rHt ) );
            break;
        case RES_PARATR_SPLIT:
            ParaSplit( static_cast< const SvxFormatSplitItem& >( rHt ) );
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
            FormatFrameSize( static_cast< const SwFormatFrameSize& >( rHt ) );
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
            FormatPageDescription( static_cast< const SwFormatPageDesc& >( rHt ) );
            break;
        case RES_BREAK:
            FormatBreak( static_cast< const SvxFormatBreakItem& >( rHt ) );
            break;
        case RES_SURROUND:
            FormatSurround( static_cast< const SwFormatSurround& >( rHt ) );
            break;
        case RES_VERT_ORIENT:
            FormatVertOrientation( static_cast< const SwFormatVertOrient& >( rHt ) );
            break;
        case RES_HORI_ORIENT:
            FormatHorizOrientation( static_cast< const SwFormatHoriOrient& >( rHt ) );
            break;
        case RES_ANCHOR:
            FormatAnchor( static_cast< const SwFormatAnchor& >( rHt ) );
            break;
        case RES_BACKGROUND:
            FormatBackground( static_cast< const SvxBrushItem& >( rHt ) );
            break;
        case XATTR_FILLSTYLE:
            FormatFillStyle( static_cast< const XFillStyleItem& >( rHt ) );
            break;
        case XATTR_FILLGRADIENT:
            FormatFillGradient( static_cast< const XFillGradientItem& >( rHt ) );
            break;
        case RES_BOX:
            FormatBox( static_cast< const SvxBoxItem& >( rHt ) );
            break;
        case RES_COL:
            FormatColumns( static_cast< const SwFormatCol& >( rHt ) );
            break;
        case RES_KEEP:
            FormatKeep( static_cast< const SvxFormatKeepItem& >( rHt ) );
            break;
        case RES_TEXTGRID:
            FormatTextGrid( static_cast< const SwTextGridItem& >( rHt ) );
            break;
        case RES_LINENUMBER:
            FormatLineNumbering( static_cast< const SwFormatLineNumber& >( rHt ) );
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

void AttributeOutputBase::OutputStyleItemSet( const SfxItemSet& rSet, bool bDeep, bool bTestForDefault )
{
    // based on OutputItemSet() from wrt_fn.cxx

    const SfxItemPool& rPool = *rSet.GetPool();
    const SfxItemSet* pSet = &rSet;
    if ( !pSet->Count() )
    {
        if ( !bDeep )
            return;

        while ( nullptr != ( pSet = pSet->GetParent() ) && !pSet->Count() )
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
        } while ( !aIter.IsAtEnd() && nullptr != ( pItem = aIter.NextItem() ) );
    }
    else
    {
        SfxWhichIter aIter( *pSet );
        sal_uInt16 nWhich = aIter.FirstWhich();
        while ( nWhich )
        {
            if ( SfxItemState::SET == pSet->GetItemState( nWhich, bDeep, &pItem ) &&
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
    // Get one of the borders (if there is any border then in docx also will be)
    const SvxBorderLine* pBorderLine = nullptr;
    sal_uInt16 nDist = 0;
    if( rBox.GetTop() )
    {
       pBorderLine = rBox.GetTop();
       nDist = rBox.GetDistance( SvxBoxItemLine::TOP );
    }
    else if( rBox.GetLeft() )
    {
       pBorderLine = rBox.GetLeft();
       nDist = rBox.GetDistance( SvxBoxItemLine::LEFT );
    }
    else if( rBox.GetBottom() )
    {
       pBorderLine = rBox.GetBottom();
       nDist = rBox.GetDistance( SvxBoxItemLine::BOTTOM );
    }
    else if( rBox.GetRight() )
    {
       pBorderLine = rBox.GetRight();
       nDist = rBox.GetDistance( SvxBoxItemLine::RIGHT );
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
 * This function is used to check if the current SwTextNode (paragraph) has a redline object
 * that is attached to the paragraph marker.
 * This is done by checking if the range (SwPaM) of the redline is :
 * - Start = the last character of the current paragraph
 * - End = the first character of the next paragraph
 */
const SwRedlineData* AttributeOutputBase::GetParagraphMarkerRedline( const SwTextNode& rNode, RedlineType_t aRedlineType)
{
    // ToDo : this is not the most ideal ... should start maybe from 'nCurRedlinePos'
    for( size_t nRedlinePos = 0; nRedlinePos < GetExport().m_pDoc->getIDocumentRedlineAccess().GetRedlineTable().size(); ++nRedlinePos )
    {
        const SwRangeRedline* pRedl = GetExport().m_pDoc->getIDocumentRedlineAccess().GetRedlineTable()[ nRedlinePos ];

        // Only check redlines that are of type 'Delete'
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

            // Maybe add here a check that also the start & end of the redline is the entire paragraph
            if ( ( uStartNodeIndex == uEndNodeIndex - 1 ) &&
                 ( uStartCharIndex == (sal_uLong)rNode.Len() ) &&
                 ( uEndCharIndex == 0)
               )
            {
                return &( pRedl->GetRedlineData() );
            }
        }
    }
    return nullptr;
}

void AttributeOutputBase::CharBackgroundBase( const SvxBrushItem& rBrush )
{
    bool bConvertToShading = SvtFilterOptions::Get().IsCharBackground2Shading();
    bool bHasShadingMarker = false;

    // Check shading marker
    {
        const SfxGrabBagItem& aGrabBag = static_cast< const SfxGrabBagItem& >( GetExport().GetItem( RES_CHRATR_GRABBAG ) );
        const std::map<OUString, css::uno::Any>& rMap = aGrabBag.GetGrabBag();
        auto aIterator = rMap.find("CharShadingMarker");
        if( aIterator != rMap.end() )
        {
            aIterator->second >>= bHasShadingMarker;
        }
    }

    if( bConvertToShading || bHasShadingMarker )
    {
        CharBackground(rBrush);
    }
    else
    {
        CharHighlight(rBrush);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
