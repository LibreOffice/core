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

#include <ToxTextGenerator.hxx>

#include <chpfld.hxx>
#include <cntfrm.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <ndindex.hxx>
#include <fchrfmt.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <ndtxt.hxx>
#include <tox.hxx>
#include <txmsrt.hxx>
#include <fmtautofmt.hxx>
#include <swatrset.hxx>
#include <ToxWhitespaceStripper.hxx>
#include <ToxLinkProcessor.hxx>
#include <ToxTabStopTokenHandler.hxx>
#include <txatbase.hxx>
#include <modeltoviewhelper.hxx>

#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <svl/itemiter.hxx>

#include <cassert>
#include <memory>

namespace {

bool sortTabHasNoToxSourcesOrFirstToxSourceHasNoNode(const SwTOXSortTabBase& sortTab)
{
    if (sortTab.aTOXSources.empty()) {
        return true;
    }
    if (sortTab.aTOXSources.at(0).pNd == nullptr) {
        return true;
    }
    return false;
}

} // end anonymous namespace

namespace sw {

OUString
ToxTextGenerator::GetNumStringOfFirstNode(const SwTOXSortTabBase& rBase,
        bool bUsePrefix, sal_uInt8 nLevel,
        SwRootFrame const*const pLayout)
{
    if (sortTabHasNoToxSourcesOrFirstToxSourceHasNoNode(rBase)) {
        return OUString();
    }

    OUString sRet;
    if (rBase.pTextMark) { // only if it's not a Mark
        return sRet;
    }

    const SwTextNode* pNd = rBase.aTOXSources[0].pNd->GetTextNode();
    if (!pNd) {
        return sRet;
    }
    if (pLayout && pLayout->HasMergedParas())
    {   // note: pNd could be any node, since it could be Sequence etc.
        pNd = sw::GetParaPropsNode(*pLayout, SwNodeIndex(*pNd));
    }

    const SwNumRule* pRule = pNd->GetNumRule();
    if (!pRule) {
        return sRet;
    }

    if (pNd->GetActualListLevel() < MAXLEVEL) {
        sRet = pNd->GetNumString(bUsePrefix, nLevel, pLayout);
    }

    if (!sRet.isEmpty()) {
        sRet += " ";// Makes sure spacing is done only when there is outline numbering
    }

    return sRet;
}


ToxTextGenerator::ToxTextGenerator(const SwForm& toxForm,
        std::shared_ptr<ToxTabStopTokenHandler> const & tabStopHandler)
: mToxForm(toxForm),
  mLinkProcessor(std::make_shared<ToxLinkProcessor>()),
  mTabStopTokenHandler(tabStopHandler)
{}

ToxTextGenerator::~ToxTextGenerator()
{}

OUString
ToxTextGenerator::HandleChapterToken(const SwTOXSortTabBase& rBase,
        const SwFormToken& aToken, SwRootFrame const*const pLayout) const
{
    if (sortTabHasNoToxSourcesOrFirstToxSourceHasNoNode(rBase)) {
        return OUString();
    }

    // A bit tricky: Find a random Frame
    const SwContentNode* contentNode = rBase.aTOXSources.at(0).pNd->GetContentNode();
    if (!contentNode) {
        return OUString();
    }

    // #i53420#
    const SwContentFrame* contentFrame = contentNode->getLayoutFrame(pLayout);
    if (!contentFrame) {
        return OUString();
    }

    return GenerateTextForChapterToken(aToken, contentFrame, contentNode, pLayout);
}

OUString
ToxTextGenerator::GenerateTextForChapterToken(const SwFormToken& chapterToken, const SwContentFrame* contentFrame,
        const SwContentNode *contentNode,
        SwRootFrame const*const pLayout) const
{
    OUString retval;

    SwChapterFieldType chapterFieldType;
    SwChapterField aField = ObtainChapterField(&chapterFieldType, &chapterToken, contentFrame, contentNode);

    //---> #i89791#
    // continue to support CF_NUMBER and CF_NUM_TITLE in order to handle ODF 1.0/1.1 written by OOo 3.x
    // in the same way as OOo 2.x would handle them.
    if (CF_NUM_NOPREPST_TITLE == chapterToken.nChapterFormat || CF_NUMBER == chapterToken.nChapterFormat) {
        retval += aField.GetNumber(pLayout); // get the string number without pre/postfix
    }
    else if (CF_NUMBER_NOPREPST == chapterToken.nChapterFormat || CF_NUM_TITLE == chapterToken.nChapterFormat) {
        retval += aField.GetNumber(pLayout) + " ";
        retval += aField.GetTitle(pLayout);
    } else if (CF_TITLE == chapterToken.nChapterFormat) {
        retval += aField.GetTitle(pLayout);
    }
    return retval;
}

// Add parameter <_TOXSectNdIdx> and <_pDefaultPageDesc> in order to control,
// which page description is used, no appropriate one is found.
void
ToxTextGenerator::GenerateText(SwDoc* pDoc,
        std::unordered_map<OUString, int> & rMarkURLs,
        const std::vector<std::unique_ptr<SwTOXSortTabBase>> &entries,
        sal_uInt16 indexOfEntryToProcess, sal_uInt16 numberOfEntriesToProcess,
        SwRootFrame const*const pLayout)
{
    // pTOXNd is only set at the first mark
    SwTextNode* pTOXNd = const_cast<SwTextNode*>(entries.at(indexOfEntryToProcess)->pTOXNd);
    // FIXME this operates directly on the node text
    OUString & rText = const_cast<OUString&>(pTOXNd->GetText());
    rText.clear();
    for(sal_uInt16 nIndex = indexOfEntryToProcess; nIndex < indexOfEntryToProcess + numberOfEntriesToProcess; nIndex++)
    {
        if(nIndex > indexOfEntryToProcess)
            rText += ", "; // comma separation
        // Initialize String with the Pattern from the form
        const SwTOXSortTabBase& rBase = *entries.at(nIndex);
        sal_uInt16 nLvl = rBase.GetLevel();
        OSL_ENSURE( nLvl < mToxForm.GetFormMax(), "invalid FORM_LEVEL");

        SvxTabStopItem aTStops( 0, 0, SvxTabAdjust::Default, RES_PARATR_TABSTOP );
        // create an enumerator
        // #i21237#
        SwFormTokens aPattern = mToxForm.GetPattern(nLvl);
        // remove text from node
        for(const auto& aToken : aPattern) // #i21237#
        {
            sal_Int32 nStartCharStyle = rText.getLength();
            switch( aToken.eTokenType )
            {
            case TOKEN_ENTRY_NO:
                // for TOC numbering
                rText += GetNumStringOfFirstNode(rBase,
                    aToken.nChapterFormat == CF_NUMBER,
                    static_cast<sal_uInt8>(aToken.nOutlineLevel - 1), pLayout);
                break;

            case TOKEN_ENTRY_TEXT: {
                HandledTextToken htt = HandleTextToken(rBase, pDoc->GetAttrPool(), pLayout);
                ApplyHandledTextToken(htt, *pTOXNd);
            }
                break;

            case TOKEN_ENTRY:
                {
                    // for TOC numbering
                    rText += GetNumStringOfFirstNode(rBase, true, MAXLEVEL, pLayout);
                    HandledTextToken htt = HandleTextToken(rBase, pDoc->GetAttrPool(), pLayout);
                    ApplyHandledTextToken(htt, *pTOXNd);
                }
                break;

            case TOKEN_TAB_STOP: {
                ToxTabStopTokenHandler::HandledTabStopToken htst =
                        mTabStopTokenHandler->HandleTabStopToken(aToken, *pTOXNd, pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
                rText += htst.text;
                aTStops.Insert(htst.tabStop);
                break;
            }

            case TOKEN_TEXT:
                rText += aToken.sText;
                break;

            case TOKEN_PAGE_NUMS:
                rText += ConstructPageNumberPlaceholder(rBase.aTOXSources.size());
                break;

            case TOKEN_CHAPTER_INFO:
                rText += HandleChapterToken(rBase, aToken, pLayout);
                break;

            case TOKEN_LINK_START:
                mLinkProcessor->StartNewLink(rText.getLength(), aToken.sCharStyleName);
                break;

            case TOKEN_LINK_END:
                {
                    auto [url, isMark] = rBase.GetURL(pLayout);
                    if (isMark)
                    {
                        auto [iter, _] = rMarkURLs.emplace(url, 0);
                        (void) _; // sigh... ignore it more explicitly
                        ++iter->second;
                        url = "#" + OUString::number(iter->second) + url;
                    }
                    mLinkProcessor->CloseLink(rText.getLength(), url);
                }
                break;

            case TOKEN_AUTHORITY:
                {
                    ToxAuthorityField eField = static_cast<ToxAuthorityField>(aToken.nAuthorityField);
                    SwIndex aIdx( pTOXNd, rText.getLength() );
                    rBase.FillText( *pTOXNd, aIdx, static_cast<sal_uInt16>(eField), pLayout );
                }
                break;
            case TOKEN_END: break;
            }

            if ( !aToken.sCharStyleName.isEmpty() )
            {
                SwCharFormat* pCharFormat;
                if( USHRT_MAX != aToken.nPoolId )
                    pCharFormat = pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool( aToken.nPoolId );
                else
                    pCharFormat = pDoc->FindCharFormatByName( aToken.sCharStyleName);

                if (pCharFormat)
                {
                    SwFormatCharFormat aFormat( pCharFormat );
                    pTOXNd->InsertItem( aFormat, nStartCharStyle,
                        rText.getLength(), SetAttrMode::DONTEXPAND );
                }
            }
        }

        pTOXNd->SetAttr( aTStops );
    }
    mLinkProcessor->InsertLinkAttributes(*pTOXNd);
}

/*static*/ std::shared_ptr<SfxItemSet>
ToxTextGenerator::CollectAttributesForTox(const SwTextAttr& hint, SwAttrPool& pool)
{
    auto retval = std::make_shared<SfxItemSet>(pool);
    if (hint.Which() != RES_TXTATR_AUTOFMT) {
        return retval;
    }
    const SwFormatAutoFormat& afmt = hint.GetAutoFormat();
    SfxItemIter aIter( *afmt.GetStyleHandle());
    const SfxPoolItem* pItem = aIter.GetCurItem();
    do
    {
        if (pItem->Which() == RES_CHRATR_ESCAPEMENT ||
            pItem->Which() == RES_CHRATR_POSTURE ||
            pItem->Which() == RES_CHRATR_CJK_POSTURE ||
            pItem->Which() == RES_CHRATR_CTL_POSTURE)
        {
            retval->Put(std::unique_ptr<SfxPoolItem>(pItem->Clone()));
        }
        pItem = aIter.NextItem();
    } while (pItem);
    return retval;
}

void ToxTextGenerator::GetAttributesForNode(
    ToxTextGenerator::HandledTextToken & rResult,
    sal_Int32 & rOffset,
    SwTextNode const& rNode,
    ToxWhitespaceStripper const& rStripper,
    SwAttrPool & rPool,
    SwRootFrame const*const pLayout)
{
    // note: this *must* use the same flags as SwTextNode::GetExpandText()
    // or indexes will be off!
    ExpandMode eMode = ExpandMode::ExpandFields;
    if (pLayout && pLayout->IsHideRedlines())
    {
        eMode |= ExpandMode::HideDeletions;
    }
    ModelToViewHelper aConversionMap(rNode, pLayout, eMode);
    if (SwpHints const*const pHints = rNode.GetpSwpHints())
    {
        for (size_t i = 0; i < pHints->Count(); ++i)
        {
            const SwTextAttr* pHint = pHints->Get(i);
            std::shared_ptr<SfxItemSet> attributesToClone =
                CollectAttributesForTox(*pHint, rPool);
            if (attributesToClone->Count() <= 0) {
                continue;
            }

            // sw_redlinehide: due to the ... interesting ... multi-level index
            // mapping going on here, can't use the usual merged attr iterators :(

            sal_Int32 const nStart(aConversionMap.ConvertToViewPosition(pHint->GetStart()));
            sal_Int32 const nEnd(aConversionMap.ConvertToViewPosition(pHint->GetAnyEnd()));
            if (nStart != nEnd) // might be in delete redline, and useless anyway
            {
                std::unique_ptr<SwFormatAutoFormat> pClone(pHint->GetAutoFormat().Clone());
                pClone->SetStyleHandle(attributesToClone);
                rResult.autoFormats.push_back(std::move(pClone));
                // note the rStripper is on the whole merged text, so need rOffset
                rResult.startPositions.push_back(
                    rStripper.GetPositionInStrippedString(rOffset + nStart));
                rResult.endPositions.push_back(
                    rStripper.GetPositionInStrippedString(rOffset + nEnd));
            }
        }
    }
    rOffset += aConversionMap.getViewText().getLength();
}

ToxTextGenerator::HandledTextToken
ToxTextGenerator::HandleTextToken(const SwTOXSortTabBase& source,
        SwAttrPool& pool, SwRootFrame const*const pLayout)
{
    HandledTextToken result;
    ToxWhitespaceStripper stripper(source.GetText().sText);
    result.text = stripper.GetStrippedString();

    // FIXME: there is a pre-existing problem that the index mapping of the
    // attributes only works if the paragraph is fully selected
    if (!source.IsFullPara() || source.aTOXSources.empty())
        return result;

    const SwTextNode* pSrc = source.aTOXSources.front().pNd->GetTextNode();
    if (!pSrc)
    {
        return result;
    }

    sal_Int32 nOffset(0);
    GetAttributesForNode(result, nOffset, *pSrc, stripper, pool, pLayout);
    if (pLayout && pLayout->HasMergedParas())
    {
        if (SwTextFrame const*const pFrame = static_cast<SwTextFrame*>(pSrc->getLayoutFrame(pLayout)))
        {
            if (sw::MergedPara const*const pMerged = pFrame->GetMergedPara())
            {
                // pSrc already copied above
                assert(pSrc == pMerged->pParaPropsNode);
                for (sal_uLong i = pSrc->GetIndex() + 1;
                     i <= pMerged->pLastNode->GetIndex(); ++i)
                {
                    SwNode *const pTmp(pSrc->GetNodes()[i]);
                    if (pTmp->GetRedlineMergeFlag() == SwNode::Merge::NonFirst)
                    {
                        GetAttributesForNode(result, nOffset,
                                *pTmp->GetTextNode(), stripper, pool, pLayout);
                    }
                }
            }
        }
    }

    return result;
}

/*static*/ void
ToxTextGenerator::ApplyHandledTextToken(const HandledTextToken& htt, SwTextNode& targetNode)
{
    sal_Int32 offset = targetNode.GetText().getLength();
    SwIndex aIdx(&targetNode, offset);
    targetNode.InsertText(htt.text, aIdx);
    for (size_t i=0; i < htt.autoFormats.size(); ++i) {
        targetNode.InsertItem(*htt.autoFormats.at(i),
                htt.startPositions.at(i) + offset,
                htt.endPositions.at(i) + offset);
    }
}

/*static*/ OUString
ToxTextGenerator::ConstructPageNumberPlaceholder(size_t numberOfToxSources)
{
    if (numberOfToxSources == 0) {
        return OUString();
    }
    OUStringBuffer retval;
    // Place holder for the PageNumber; we only respect the first one
    retval.append(C_NUM_REPL);
    for (size_t i = 1; i < numberOfToxSources; ++i) {
        retval.append(SwTOXMark::S_PAGE_DELI);
        retval.append(C_NUM_REPL);
    }
    retval.append(C_END_PAGE_NUM);
    return retval.makeStringAndClear();
}

/*virtual*/ SwChapterField
ToxTextGenerator::ObtainChapterField(SwChapterFieldType* chapterFieldType,
        const SwFormToken* chapterToken, const SwContentFrame* contentFrame,
        const SwContentNode* contentNode) const
{
    assert(chapterToken);
    assert(chapterToken->nOutlineLevel >= 1);

    SwChapterField retval(chapterFieldType, chapterToken->nChapterFormat);
    retval.SetLevel(static_cast<sal_uInt8>(chapterToken->nOutlineLevel - 1));
    // #i53420#
    retval.ChangeExpansion(*contentFrame, contentNode, true);
    return retval;
}
} // end namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
