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

#include <hintids.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <charfmt.hxx>
#include <fmtautofmt.hxx>

#include <viewsh.hxx>
#include <doc.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <viewopt.hxx>
#include <flyfrm.hxx>
#include <viewimp.hxx>
#include <swfont.hxx>
#include <swmodule.hxx>
#include "porfld.hxx"
#include "porftn.hxx"
#include "porref.hxx"
#include "portox.hxx"
#include "porfly.hxx"
#include "itrform2.hxx"
#include <chpfld.hxx>
#include <dbfld.hxx>
#include <expfld.hxx>
#include <docufld.hxx>
#include <pagedesc.hxx>
#include <fmtmeta.hxx>
#include <reffld.hxx>
#include <flddat.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <redline.hxx>
#include <sfx2/docfile.hxx>
#include <svl/grabbagitem.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <editeng/colritem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <officecfg/Office/Writer.hxx>

static bool lcl_IsInBody( SwFrame const *pFrame )
{
    if ( pFrame->IsInDocBody() )
        return true;

    while (const SwFlyFrame* pFly = pFrame->FindFlyFrame())
        pFrame = pFly->GetAnchorFrame();
    return pFrame->IsInDocBody();
}

static OUString ExpandField(const SwField& rField, const SwTextFormatter& rFormatter,
                            const SwTextFormatInfo& rInf)
{
    if (rInf.GetOpt().IsFieldName())
        return rField.GetFieldName();

    const SwViewShell* pSh = rInf.GetVsh();
    const SwDoc* pDoc(pSh ? pSh->GetDoc() : nullptr);
    const bool bInClipboard(!pDoc || pDoc->IsClipBoard());
    return rField.ExpandField(bInClipboard, rFormatter.GetTextFrame()->getRootFrame());
}

SwExpandPortion *SwTextFormatter::NewFieldPortion( SwTextFormatInfo &rInf,
                                                const SwTextAttr *pHint ) const
{
    SwField *pField = const_cast<SwField*>(pHint->GetFormatField().GetField());
    const bool bName = rInf.GetOpt().IsFieldName();

    // set language
    const_cast<SwTextFormatter*>(this)->SeekAndChg( rInf );
    if (pField->GetLanguage() != GetFnt()->GetLanguage())
        pField->SetLanguage( GetFnt()->GetLanguage() );

    SwViewShell *pSh = rInf.GetVsh();

    switch (pField->GetTyp()->Which())
    {
        case SwFieldIds::Script:
        case SwFieldIds::Postit:
            return new SwPostItsPortion(SwFieldIds::Script == pField->GetTyp()->Which());
        case SwFieldIds::CombinedChars:
            if (!bName)
                return new SwCombinedPortion(ExpandField(*pField, *this, rInf));
            break;
        case SwFieldIds::HiddenText:
            return new SwHiddenPortion(ExpandField(*pField, *this, rInf));
        case SwFieldIds::Chapter:
            if (!bName && pSh && !pSh->Imp()->IsUpdateExpFields())
            {
                static_cast<SwChapterField*>(pField)->ChangeExpansion(
                    *m_pFrame, &static_txtattr_cast<SwTextField const*>(pHint)->GetTextNode());
            }
            break;
        case SwFieldIds::DocStat:
            if (!bName && pSh && !pSh->Imp()->IsUpdateExpFields())
            {
                static_cast<SwDocStatField*>(pField)->ChangeExpansion(m_pFrame);
            }
            break;
        case SwFieldIds::PageNumber:
            if (!bName && pSh && pSh->GetLayout() && !pSh->Imp()->IsUpdateExpFields())
            {
                auto pPageNr = static_cast<SwPageNumberFieldType*>(pField->GetTyp());

                const SwRootFrame* pTmpRootFrame = pSh->GetLayout();
                const bool bVirt = pTmpRootFrame->IsVirtPageNum();

                sal_uInt16 nVirtNum = m_pFrame->GetVirtPageNum();
                sal_uInt16 nNumPages = pTmpRootFrame->GetPageNum();
                SvxNumType nNumFormat = SvxNumType(-1);
                if (SVX_NUM_PAGEDESC == pField->GetFormat())
                    nNumFormat
                        = m_pFrame->FindPageFrame()->GetPageDesc()->GetNumType().GetNumberingType();
                static_cast<SwPageNumberField*>(pField)->ChangeExpansion(nVirtNum, nNumPages);
                pPageNr->ChangeExpansion(pSh->GetDoc(), bVirt,
                                         nNumFormat != SvxNumType(-1) ? &nNumFormat : nullptr);
            }
            break;
        case SwFieldIds::GetExp:
            if (!bName && pSh && !pSh->Imp()->IsUpdateExpFields())
            {
                auto pExpField = static_cast<SwGetExpField*>(pField);
                if (!::lcl_IsInBody(m_pFrame))
                {
                    pExpField->ChgBodyTextFlag(false);
                    pExpField->ChangeExpansion(*m_pFrame,
                                               *static_txtattr_cast<SwTextField const*>(pHint));
                }
                else if (!pExpField->IsInBodyText())
                {
                    // Was something else previously, thus: expand first, then convert it!
                    pExpField->ChangeExpansion(*m_pFrame,
                                               *static_txtattr_cast<SwTextField const*>(pHint));
                    pExpField->ChgBodyTextFlag(true);
                }
            }
            break;
        case SwFieldIds::Database:
            if (!bName)
            {
                static_cast<SwDBField*>(pField)->ChgBodyTextFlag(::lcl_IsInBody(m_pFrame));
            }
            break;
        case SwFieldIds::RefPageGet:
            if (!bName && pSh && !pSh->Imp()->IsUpdateExpFields())
            {
                static_cast<SwRefPageGetField*>(pField)->ChangeExpansion(
                    *m_pFrame, static_txtattr_cast<SwTextField const*>(pHint));
            }
            break;
        case SwFieldIds::JumpEdit:
        {
            std::unique_ptr<SwFont> pFont;
            if (!bName)
            {
                pFont = std::make_unique<SwFont>(*m_pFont);
                pFont->SetDiffFnt(
                    &static_cast<SwJumpEditField*>(pField)->GetCharFormat()->GetAttrSet(),
                    &m_pFrame->GetDoc().getIDocumentSettingAccess());
            }
            return new SwJumpFieldPortion(ExpandField(*pField, *this, rInf), pField->GetPar2(),
                                          std::move(pFont), pField->GetFormat());
        }
        case SwFieldIds::GetRef:
            if (!bName)
            {
                auto pGetRef = static_cast<SwGetRefField*>(pField);
                if (pGetRef->GetSubType() == REF_STYLE)
                    pGetRef->UpdateField(static_txtattr_cast<SwTextField const*>(pHint), m_pFrame);
            }
            break;
        default:
            break;
    }
    return new SwFieldPortion(ExpandField(*pField, *this, rInf));
}

static SwFieldPortion * lcl_NewMetaPortion(SwTextAttr & rHint, const bool bPrefix)
{
    ::sw::Meta *const pMeta(
        static_cast<SwFormatMeta &>(rHint.GetAttr()).GetMeta() );
    OUString fix;
    ::sw::MetaField *const pField( dynamic_cast< ::sw::MetaField * >(pMeta) );
    OSL_ENSURE(pField, "lcl_NewMetaPortion: no meta field?");
    if (pField)
    {
        OUString color;
        pField->GetPrefixAndSuffix(bPrefix ? &fix : nullptr, bPrefix ? nullptr : &fix, &color);
    }
    return new SwFieldPortion( fix );
}

/**
 * Try to create a new portion with zero length, for an end of a hint
 * (where there is no CH_TXTATR). Because there may be multiple hint ends at a
 * given index, m_pByEndIter is used to keep track of the already created
 * portions. But the portions created here may actually be deleted again,
 * due to Underflow. In that case, m_pByEndIter must be decremented,
 * so the portion will be created again on the next line.
 */
SwExpandPortion * SwTextFormatter::TryNewNoLengthPortion(SwTextFormatInfo const & rInfo)
{
    const TextFrameIndex nIdx(rInfo.GetIdx());

    // sw_redlinehide: because there is a dummy character at the start of these
    // hints, it's impossible to have ends of hints from different nodes at the
    // same view position, so it's sufficient to check the hints of the current
    // node.  However, m_pByEndIter exists for the whole text frame, so
    // it's necessary to iterate all hints for that purpose...
    if (!m_pByEndIter)
    {
        m_pByEndIter.reset(new sw::MergedAttrIterByEnd(*rInfo.GetTextFrame()));
    }
    SwTextNode const* pNode(nullptr);
    for (SwTextAttr const* pHint = m_pByEndIter->NextAttr(pNode); pHint;
         pHint = m_pByEndIter->NextAttr(pNode))
    {
        SwTextAttr & rHint(const_cast<SwTextAttr&>(*pHint));
        TextFrameIndex const nEnd(
            rInfo.GetTextFrame()->MapModelToView(pNode, rHint.GetAnyEnd()));
        if (nEnd > nIdx)
        {
            m_pByEndIter->PrevAttr();
            break;
        }
        if (nEnd == nIdx)
        {
            if (RES_TXTATR_METAFIELD == rHint.Which())
            {
                SwFieldPortion *const pPortion(
                        lcl_NewMetaPortion(rHint, false));
                pPortion->SetNoLength(); // no CH_TXTATR at hint end!
                return pPortion;
            }
        }
    }
    return nullptr;
}

SwLinePortion *SwTextFormatter::NewExtraPortion( SwTextFormatInfo &rInf )
{
    SwTextAttr *pHint = GetAttr( rInf.GetIdx() );
    SwLinePortion *pRet = nullptr;
    if( !pHint )
    {
        pRet = new SwTextPortion;
        pRet->SetLen(TextFrameIndex(1));
        rInf.SetLen(TextFrameIndex(1));
        return pRet;
    }

    switch( pHint->Which() )
    {
    case RES_TXTATR_FLYCNT :
        {
            pRet = NewFlyCntPortion( rInf, pHint );
            break;
        }
    case RES_TXTATR_FTN :
        {
            pRet = NewFootnotePortion( rInf, pHint );
            break;
        }
    case RES_TXTATR_FIELD :
    case RES_TXTATR_ANNOTATION :
        {
            pRet = NewFieldPortion( rInf, pHint );
            break;
        }
    case RES_TXTATR_REFMARK :
        {
            pRet = new SwIsoRefPortion;
            break;
        }
    case RES_TXTATR_TOXMARK :
        {
            pRet = new SwIsoToxPortion;
            break;
        }
    case RES_TXTATR_METAFIELD:
        {
            pRet = lcl_NewMetaPortion( *pHint, true );
            break;
        }
    default: ;
    }
    if( !pRet )
    {
        auto pFieldPortion = new SwFieldPortion( "" );
        if (pHint->Which() == RES_TXTATR_CONTENTCONTROL)
        {
            pFieldPortion->SetContentControl(true);
        }
        pRet = pFieldPortion;
        rInf.SetLen(TextFrameIndex(1));
    }
    return pRet;
}

/**
 * OOXML spec says that w:rPr inside w:pPr specifies formatting for the paragraph mark symbol (i.e. the control
 * character than can be configured to be shown). However, in practice MSO also uses it as direct formatting
 * for numbering in that paragraph. I don't know if the problem is in the spec or in MSWord.
 */
static void checkApplyParagraphMarkFormatToNumbering(SwFont* pNumFnt, SwTextFormatInfo& rInf,
                                                     const IDocumentSettingAccess* pIDSA,
                                                     const SwAttrSet* pFormat)
{
    if( !pIDSA->get(DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING ))
        return;

    SwFormatAutoFormat const& rListAutoFormat(rInf.GetTextFrame()->GetTextNodeForParaProps()->GetAttr(RES_PARATR_LIST_AUTOFMT));
    std::shared_ptr<SfxItemSet> pSet(rListAutoFormat.GetStyleHandle());

    // Check each item and in case it should be ignored, then clear it.
    if (!pSet)
        return;

    std::unique_ptr<SfxItemSet> const pCleanedSet = pSet->Clone();

    if (pCleanedSet->HasItem(RES_TXTATR_CHARFMT))
    {
        // Insert attributes of referenced char format into current set
        const SwFormatCharFormat& rCharFormat = pCleanedSet->Get(RES_TXTATR_CHARFMT);
        const SwAttrSet& rStyleAttrs = static_cast<const SwCharFormat *>(rCharFormat.GetRegisteredIn())->GetAttrSet();
        SfxWhichIter aIter(rStyleAttrs);
        sal_uInt16 nWhich = aIter.FirstWhich();
        while (nWhich)
        {
            if (!SwTextNode::IsIgnoredCharFormatForNumbering(nWhich, /*bIsCharStyle=*/true)
                && !pCleanedSet->HasItem(nWhich)
                && !(pFormat && pFormat->HasItem(nWhich))
                && rStyleAttrs.GetItemState(nWhich) > SfxItemState::DEFAULT)
            {
                // Copy from parent sets only allowed items which will not overwrite
                // values explicitly defined in current set (pCleanedSet) or in pFormat
                if (const SfxPoolItem* pItem = rStyleAttrs.GetItem(nWhich, true))
                    pCleanedSet->Put(*pItem);
            }
            nWhich = aIter.NextWhich();
        }

        // It is not required here anymore, all referenced items are inserted
        pCleanedSet->ClearItem(RES_TXTATR_CHARFMT);
    };

    SfxItemIter aIter(*pSet);
    const SfxPoolItem* pItem = aIter.GetCurItem();
    while (pItem)
    {
        if (SwTextNode::IsIgnoredCharFormatForNumbering(pItem->Which()))
            pCleanedSet->ClearItem(pItem->Which());
        else if (pFormat && pFormat->HasItem(pItem->Which()))
            pCleanedSet->ClearItem(pItem->Which());
        else if (pItem->Which() == RES_CHRATR_BACKGROUND)
        {
            bool bShadingWasImported = false;
            // If Shading was imported, it should not be converted to a Highlight,
            // but remain as Shading which is ignored for numbering.
            if (pCleanedSet->HasItem(RES_CHRATR_GRABBAG))
            {
                SfxGrabBagItem aGrabBag = pCleanedSet->Get(RES_CHRATR_GRABBAG, /*bSrchInParent=*/false);
                std::map<OUString, css::uno::Any>& rMap = aGrabBag.GetGrabBag();
                auto aIterator = rMap.find("CharShadingMarker");
                if (aIterator != rMap.end())
                    aIterator->second >>= bShadingWasImported;
            }

            // If used, BACKGROUND is converted to HIGHLIGHT. So also ignore if a highlight already exists.
            if (bShadingWasImported
                || pCleanedSet->HasItem(RES_CHRATR_HIGHLIGHT)
                || (pFormat && pFormat->HasItem(RES_CHRATR_HIGHLIGHT)))
            {
                pCleanedSet->ClearItem(pItem->Which());
            }
        }
        pItem = aIter.NextItem();
    };

    // SetDiffFnt resets the background color (why?), so capture it and re-apply if it had a value,
    // because an existing value should override anything inherited from the paragraph marker.
    const std::optional<Color> oFontBackColor = pNumFnt->GetBackColor();
    // The same is true for the highlight color.
    const Color aHighlight = pNumFnt->GetHighlightColor();

    pNumFnt->SetDiffFnt(pCleanedSet.get(), pIDSA);

    if (oFontBackColor)
        pNumFnt->SetBackColor(oFontBackColor);
    if (aHighlight != COL_TRANSPARENT && !pCleanedSet->HasItem(RES_CHRATR_HIGHLIGHT))
        pNumFnt->SetHighlightColor(aHighlight);
}

static const SwRangeRedline* lcl_GetRedlineAtNodeInsertionOrDeletion( const SwTextNode& rTextNode,
        bool& bIsMoved )
{
    const SwDoc& rDoc = rTextNode.GetDoc();
    SwRedlineTable::size_type nRedlPos = rDoc.getIDocumentRedlineAccess().GetRedlinePos( rTextNode, RedlineType::Any );

    if( SwRedlineTable::npos != nRedlPos )
    {
        const SwNodeOffset nNdIdx = rTextNode.GetIndex();
        const SwRedlineTable& rTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();
        for( ; nRedlPos < rTable.size() ; ++nRedlPos )
        {
            const SwRangeRedline* pTmp = rTable[ nRedlPos ];
            SwNodeOffset nStart = pTmp->GetPoint()->GetNodeIndex(),
                         nEnd = pTmp->GetMark()->GetNodeIndex();
            if( nStart > nEnd )
                std::swap(nStart, nEnd);
            if( RedlineType::Delete == pTmp->GetType() ||
                RedlineType::Insert == pTmp->GetType() )
            {
                if( nStart <= nNdIdx && nEnd > nNdIdx )
                {
                    bIsMoved = pTmp->IsMoved();
                    return pTmp;
                }
            }
            if( nStart > nNdIdx )
                break;
        }
    }
    return nullptr;
}

static bool lcl_setRedlineAttr( SwTextFormatInfo &rInf, const SwTextNode& rTextNode, const std::unique_ptr<SwFont>& pNumFnt )
{
    if ( rInf.GetVsh()->GetLayout()->IsHideRedlines() )
        return false;

    bool bIsMoved;
    const SwRangeRedline* pRedlineNum = lcl_GetRedlineAtNodeInsertionOrDeletion( rTextNode, bIsMoved );
    if (!pRedlineNum)
        return false;

    // moved text: dark green with double underline or strikethrough
    if ( bIsMoved )
    {
        pNumFnt->SetColor(COL_GREEN);
        if ( RedlineType::Delete == pRedlineNum->GetType() )
            pNumFnt->SetStrikeout(STRIKEOUT_DOUBLE);
        else
            pNumFnt->SetUnderline(LINESTYLE_DOUBLE);
        return true;
    }

    SwAttrPool& rPool = rInf.GetVsh()->GetDoc()->GetAttrPool();
    SfxItemSetFixed<RES_CHRATR_BEGIN, RES_CHRATR_END-1> aSet(rPool);

    std::size_t aAuthor = (1 < pRedlineNum->GetStackCount())
            ? pRedlineNum->GetAuthor( 1 )
            : pRedlineNum->GetAuthor();

    if ( RedlineType::Delete == pRedlineNum->GetType() )
        SW_MOD()->GetDeletedAuthorAttr(aAuthor, aSet);
    else
        SW_MOD()->GetInsertAuthorAttr(aAuthor, aSet);

    if (const SvxColorItem* pItem = aSet.GetItemIfSet(RES_CHRATR_COLOR))
        pNumFnt->SetColor(pItem->GetValue());
    if (const SvxUnderlineItem* pItem = aSet.GetItemIfSet(RES_CHRATR_UNDERLINE))
        pNumFnt->SetUnderline(pItem->GetLineStyle());
    if (const SvxCrossedOutItem* pItem = aSet.GetItemIfSet(RES_CHRATR_CROSSEDOUT))
        pNumFnt->SetStrikeout( pItem->GetStrikeout() );

    return true;
}

SwNumberPortion *SwTextFormatter::NewNumberPortion( SwTextFormatInfo &rInf ) const
{
    if( rInf.IsNumDone() || rInf.GetTextStart() != m_nStart
                || rInf.GetTextStart() != rInf.GetIdx() )
        return nullptr;

    SwNumberPortion *pRet = nullptr;
    // sw_redlinehide: at this point it's certain that pTextNd is the node with
    // the numbering of the frame; only the actual number-vector (GetNumString)
    // depends on the hide-mode in the layout so other calls don't need to care
    const SwTextNode *const pTextNd = GetTextFrame()->GetTextNodeForParaProps();
    const SwNumRule* pNumRule = pTextNd->GetNumRule();

    // Has a "valid" number?
    // sw_redlinehide: check that pParaPropsNode is the correct one
    assert(pTextNd->IsNumbered(m_pFrame->getRootFrame()) == pTextNd->IsNumbered(nullptr));
    if (pTextNd->IsNumbered(m_pFrame->getRootFrame()) && pTextNd->IsCountedInList())
    {
        int nLevel = pTextNd->GetActualListLevel();

        if (nLevel < 0)
            nLevel = 0;

        if (nLevel >= MAXLEVEL)
            nLevel = MAXLEVEL - 1;

        const SwNumFormat &rNumFormat = pNumRule->Get( nLevel );
        const bool bLeft = SvxAdjust::Left == rNumFormat.GetNumAdjust();
        const bool bCenter = SvxAdjust::Center == rNumFormat.GetNumAdjust();
        const bool bLabelAlignmentPosAndSpaceModeActive(
                rNumFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT );
        const sal_uInt16 nMinDist = bLabelAlignmentPosAndSpaceModeActive
                                ? 0 : rNumFormat.GetCharTextDistance();

        if( SVX_NUM_BITMAP == rNumFormat.GetNumberingType() )
        {
            OUString referer;
            if (auto const sh1 = rInf.GetVsh()) {
                if (auto const doc = sh1->GetDoc()) {
                    auto const sh2 = doc->GetPersist();
                    if (sh2 != nullptr && sh2->HasName()) {
                        referer = sh2->GetMedium()->GetName();
                    }
                }
            }
            pRet = new SwGrfNumPortion( pTextNd->GetLabelFollowedBy(),
                                        rNumFormat.GetBrush(), referer,
                                        rNumFormat.GetGraphicOrientation(),
                                        rNumFormat.GetGraphicSize(),
                                        bLeft, bCenter, nMinDist,
                                        bLabelAlignmentPosAndSpaceModeActive );
            tools::Long nTmpA = rInf.GetLast()->GetAscent();
            tools::Long nTmpD = rInf.GetLast()->Height() - nTmpA;
            if( !rInf.IsTest() )
                static_cast<SwGrfNumPortion*>(pRet)->SetBase( nTmpA, nTmpD, nTmpA, nTmpD );
        }
        else
        {
            // The SwFont is created dynamically and passed in the ctor,
            // as the CharFormat only returns an SV-Font.
            // In the dtor of SwNumberPortion, the SwFont is deleted.
            const SwAttrSet* pFormat = rNumFormat.GetCharFormat() ?
                                    &rNumFormat.GetCharFormat()->GetAttrSet() :
                                    nullptr;
            const IDocumentSettingAccess* pIDSA = pTextNd->getIDocumentSettingAccess();

            if( SVX_NUM_CHAR_SPECIAL == rNumFormat.GetNumberingType() )
            {
                const std::optional<vcl::Font> pFormatFnt = rNumFormat.GetBulletFont();

                // Build a new bullet font basing on the current paragraph font:
                std::unique_ptr<SwFont> pNumFnt(new SwFont( &rInf.GetCharAttr(), pIDSA ));

                // #i53199#
                if ( !pIDSA->get(DocumentSettingId::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT) )
                {
                    // i18463:
                    // Underline style of paragraph font should not be considered
                    // Overline style of paragraph font should not be considered
                    // Weight style of paragraph font should not be considered
                    // Posture style of paragraph font should not be considered
                    pNumFnt->SetUnderline( LINESTYLE_NONE );
                    pNumFnt->SetOverline( LINESTYLE_NONE );
                    pNumFnt->SetItalic( ITALIC_NONE, SwFontScript::Latin );
                    pNumFnt->SetItalic( ITALIC_NONE, SwFontScript::CJK );
                    pNumFnt->SetItalic( ITALIC_NONE, SwFontScript::CTL );
                    pNumFnt->SetWeight( WEIGHT_NORMAL, SwFontScript::Latin );
                    pNumFnt->SetWeight( WEIGHT_NORMAL, SwFontScript::CJK );
                    pNumFnt->SetWeight( WEIGHT_NORMAL, SwFontScript::CTL );
                }

                // Apply the explicit attributes from the character style
                // associated with the numbering to the new bullet font.
                if( pFormat )
                    pNumFnt->SetDiffFnt( pFormat, pIDSA );

                checkApplyParagraphMarkFormatToNumbering(pNumFnt.get(), rInf, pIDSA, pFormat);

                if ( pFormatFnt )
                {
                    const SwFontScript nAct = pNumFnt->GetActual();
                    pNumFnt->SetFamily( pFormatFnt->GetFamilyType(), nAct );
                    pNumFnt->SetName( pFormatFnt->GetFamilyName(), nAct );
                    pNumFnt->SetStyleName( pFormatFnt->GetStyleName(), nAct );
                    pNumFnt->SetCharSet( pFormatFnt->GetCharSet(), nAct );
                    pNumFnt->SetPitch( pFormatFnt->GetPitch(), nAct );
                }

                // we do not allow a vertical font
                pNumFnt->SetVertical( pNumFnt->GetOrientation(),
                                      m_pFrame->IsVertical() );

                lcl_setRedlineAttr( rInf, *pTextNd, pNumFnt );

                // --> OD 2008-01-23 #newlistelevelattrs#
                if (rNumFormat.GetBulletChar())
                {
                    pRet = new SwBulletPortion(rNumFormat.GetBulletChar(),
                        pTextNd->GetLabelFollowedBy(),
                        std::move(pNumFnt),
                        bLeft, bCenter, nMinDist,
                        bLabelAlignmentPosAndSpaceModeActive);
                }
            }
            else
            {
                // Show Changes mode shows the actual numbering (SwListRedlineType::HIDDEN) and
                // the original one (SwListRedlineType::ORIGTEXT) instead of the fake numbering
                // (SwListRedlineType::SHOW, which counts removed and inserted numbered paragraphs
                // in a single list)
                bool bHasHiddenNum = false;
                OUString aText( pTextNd->GetNumString(true, MAXLEVEL, m_pFrame->getRootFrame(), SwListRedlineType::HIDDEN) );
                const SwDoc& rDoc = pTextNd->GetDoc();
                const SwRedlineTable& rTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();
                if ( rTable.size() && !rInf.GetVsh()->GetLayout()->IsHideRedlines() )
                {
                    OUString aHiddenText( pTextNd->GetNumString(true, MAXLEVEL, m_pFrame->getRootFrame(), SwListRedlineType::ORIGTEXT) );

                    if ( !aText.isEmpty() || !aHiddenText.isEmpty() )
                    {
                        bool bDisplayChangedParagraphNumbering = officecfg::Office::Writer::Comparison::DisplayChangedParagraphNumbering::get();
                        if (bDisplayChangedParagraphNumbering && aText != aHiddenText && !aHiddenText.isEmpty())
                        {
                            bHasHiddenNum = true;
                            // show also original number after the actual one enclosed in [ and ],
                            // and replace tabulator with space to avoid messy indentation
                            // resulted by the longer numbering, e.g. "1.[2.]" instead of "1.".
                            aText = aText +  "[" + aHiddenText + "]"
                                     + pTextNd->GetLabelFollowedBy().replaceAll("\t", " ");
                        }
                        else if (!aText.isEmpty())
                            aText += pTextNd->GetLabelFollowedBy();
                    }
                }
                else if (pTextNd->getIDocumentSettingAccess()->get(DocumentSettingId::NO_NUMBERING_SHOW_FOLLOWBY)
                    || !aText.isEmpty())
                    aText += pTextNd->GetLabelFollowedBy();

                // Not just an optimization ...
                // A number portion without text will be assigned a width of 0.
                // The succeeding text portion will flow into the BreakCut in the BreakLine,
                // although  we have rInf.GetLast()->GetFlyPortion()!
                if( !aText.isEmpty() )
                {

                    // Build a new numbering font basing on the current paragraph font:
                    std::unique_ptr<SwFont> pNumFnt(new SwFont( &rInf.GetCharAttr(), pIDSA ));

                    const SwTextNode& rTextNode = *rInf.GetTextFrame()->GetTextNodeForParaProps();
                    if (const SwpHints* pHints = rTextNode.GetpSwpHints())
                    {
                        // Also look for an empty character hint that sits at the paragraph end:
                        for (size_t i = 0; i < pHints->Count(); ++i)
                        {
                            const SwTextAttr* pHint = pHints->GetSortedByEnd(i);
                            if (pHint->Which() == RES_TXTATR_AUTOFMT && pHint->GetEnd()
                                && pHint->GetStart() == *pHint->GetEnd()
                                && pHint->GetStart() == rTextNode.GetText().getLength())
                            {
                                std::shared_ptr<SfxItemSet> pSet
                                    = pHint->GetAutoFormat().GetStyleHandle();
                                if (pSet)
                                {
                                    pNumFnt->SetDiffFnt(pSet.get(), pIDSA);
                                    break;
                                }
                            }
                        }
                    }

                    // #i53199#
                    if ( !pIDSA->get(DocumentSettingId::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT) )
                    {
                        // i18463:
                        // Underline style of paragraph font should not be considered
                        pNumFnt->SetUnderline( LINESTYLE_NONE );
                        // Overline style of paragraph font should not be considered
                        pNumFnt->SetOverline( LINESTYLE_NONE );
                    }

                    // Apply the explicit attributes from the character style
                    // associated with the numbering to the new bullet font.
                    if( pFormat )
                        pNumFnt->SetDiffFnt( pFormat, pIDSA );

                    checkApplyParagraphMarkFormatToNumbering(pNumFnt.get(), rInf, pIDSA, pFormat);

                    if ( !lcl_setRedlineAttr( rInf, *pTextNd, pNumFnt ) && bHasHiddenNum )
                        pNumFnt->SetColor(NON_PRINTING_CHARACTER_COLOR);

                    // we do not allow a vertical font
                    pNumFnt->SetVertical( pNumFnt->GetOrientation(), m_pFrame->IsVertical() );

                    pRet = new SwNumberPortion( aText, std::move(pNumFnt),
                                                bLeft, bCenter, nMinDist,
                                                bLabelAlignmentPosAndSpaceModeActive );
                }
            }
        }
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
