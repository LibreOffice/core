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

#include "chpfld.hxx"
#include "cntfrm.hxx"
#include "fchrfmt.hxx"
#include "doc.hxx"
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include "fmtinfmt.hxx"
#include "ndtxt.hxx"
#include "pagedesc.hxx"
#include "tox.hxx"
#include "txmsrt.hxx"
#include "fmtautofmt.hxx"
#include "DocumentSettingManager.hxx"
#include "SwStyleNameMapper.hxx"
#include "swatrset.hxx"
#include "ToxWhitespaceStripper.hxx"
#include "ToxLinkProcessor.hxx"
#include "ToxTabStopTokenHandler.hxx"
#include "txatbase.hxx"

#include "svl/itemiter.hxx"

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
ToxTextGenerator::GetNumStringOfFirstNode( const SwTOXSortTabBase& rBase, bool bUsePrefix, sal_uInt8 nLevel )
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

    const SwNumRule* pRule = pNd->GetNumRule();
    if (!pRule) {
        return sRet;
    }

    if (pNd->GetActualListLevel() < MAXLEVEL) {
        sRet = pNd->GetNumString(bUsePrefix, nLevel);
    }

    if(sRet != "") {
        sRet += " ";// Makes sure spacing is done only when there is outline numbering
    }

    return sRet;
}


ToxTextGenerator::ToxTextGenerator(const SwForm& toxForm,
        std::shared_ptr<ToxTabStopTokenHandler> tabStopHandler)
: mToxForm(toxForm),
  mLinkProcessor(new ToxLinkProcessor()),
  mTabStopTokenHandler(tabStopHandler)
{;}

ToxTextGenerator::~ToxTextGenerator()
{;}

OUString
ToxTextGenerator::HandleChapterToken(const SwTOXSortTabBase& rBase, const SwFormToken& aToken,
        SwDoc* pDoc) const
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
    const SwContentFrame* contentFrame = contentNode->getLayoutFrame(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    if (!contentFrame) {
        return OUString();
    }

    return GenerateTextForChapterToken(aToken, contentFrame, contentNode);
}

OUString
ToxTextGenerator::GenerateTextForChapterToken(const SwFormToken& chapterToken, const SwContentFrame* contentFrame,
        const SwContentNode *contentNode) const
{
    OUString retval;

    SwChapterFieldType chapterFieldType;
    SwChapterField aField = ObtainChapterField(&chapterFieldType, &chapterToken, contentFrame, contentNode);

    //---> #i89791#
    // continue to support CF_NUMBER and CF_NUM_TITLE in order to handle ODF 1.0/1.1 written by OOo 3.x
    // in the same way as OOo 2.x would handle them.
    if (CF_NUM_NOPREPST_TITLE == chapterToken.nChapterFormat || CF_NUMBER == chapterToken.nChapterFormat) {
        retval += aField.GetNumber(); // get the string number without pre/postfix
    }
    else if (CF_NUMBER_NOPREPST == chapterToken.nChapterFormat || CF_NUM_TITLE == chapterToken.nChapterFormat) {
        retval += aField.GetNumber();
        retval += " ";
        retval += aField.GetTitle();
    } else if (CF_TITLE == chapterToken.nChapterFormat) {
        retval += aField.GetTitle();
    }
    return retval;
}

// Add parameter <_TOXSectNdIdx> and <_pDefaultPageDesc> in order to control,
// which page description is used, no appropriate one is found.
void
ToxTextGenerator::GenerateText(SwDoc* pDoc, const std::vector<SwTOXSortTabBase*> &entries,
        sal_uInt16 indexOfEntryToProcess, sal_uInt16 numberOfEntriesToProcess)
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

        SvxTabStopItem aTStops( 0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
        // create an enumerator
        // #i21237#
        SwFormTokens aPattern = mToxForm.GetPattern(nLvl);
        SwFormTokens::iterator aIt = aPattern.begin();
        // remove text from node
        while(aIt != aPattern.end()) // #i21237#
        {
            SwFormToken aToken = *aIt; // #i21237#
            sal_Int32 nStartCharStyle = rText.getLength();
            switch( aToken.eTokenType )
            {
            case TOKEN_ENTRY_NO:
                // for TOC numbering
                rText += GetNumStringOfFirstNode( rBase, aToken.nChapterFormat == CF_NUMBER, static_cast<sal_uInt8>(aToken.nOutlineLevel - 1) ) ;
                break;

            case TOKEN_ENTRY_TEXT: {
                HandledTextToken htt = HandleTextToken(rBase, pDoc->GetAttrPool());
                ApplyHandledTextToken(htt, *pTOXNd);
            }
                break;

            case TOKEN_ENTRY:
                {
                    // for TOC numbering
                    rText += GetNumStringOfFirstNode( rBase, true, MAXLEVEL );
                    SwIndex aIdx( pTOXNd, rText.getLength() );
                    ToxWhitespaceStripper stripper(rBase.GetText().sText);
                    pTOXNd->InsertText(stripper.GetStrippedString(), aIdx);
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
                rText += HandleChapterToken(rBase, aToken, pDoc);
                break;

            case TOKEN_LINK_START:
                mLinkProcessor->StartNewLink(rText.getLength(), aToken.sCharStyleName);
                break;

            case TOKEN_LINK_END:
                mLinkProcessor->CloseLink(rText.getLength(), rBase.GetURL());
                break;

            case TOKEN_AUTHORITY:
                {
                    ToxAuthorityField eField = (ToxAuthorityField)aToken.nAuthorityField;
                    SwIndex aIdx( pTOXNd, rText.getLength() );
                    rBase.FillText( *pTOXNd, aIdx, static_cast<sal_uInt16>(eField) );
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

            ++aIt; // #i21237#
        }

        pTOXNd->SetAttr( aTStops );
    }
    mLinkProcessor->InsertLinkAttributes(*pTOXNd);
}

/*static*/ std::shared_ptr<SfxItemSet>
ToxTextGenerator::CollectAttributesForTox(const SwTextAttr& hint, SwAttrPool& pool)
{
    std::shared_ptr<SfxItemSet> retval(new SfxItemSet(pool));
    if (hint.Which() != RES_TXTATR_AUTOFMT) {
        return retval;
    }
    const SwFormatAutoFormat& afmt = hint.GetAutoFormat();
    SfxItemIter aIter( *afmt.GetStyleHandle());
    const SfxPoolItem* pItem = aIter.GetCurItem();
    while (true) {
        if (pItem->Which() == RES_CHRATR_ESCAPEMENT ||
            pItem->Which() == RES_CHRATR_POSTURE ||
            pItem->Which() == RES_CHRATR_CJK_POSTURE ||
            pItem->Which() == RES_CHRATR_CTL_POSTURE) {
            SfxPoolItem* clonedItem = pItem->Clone();
            retval->Put(*clonedItem);
        }
        if (aIter.IsAtEnd()) {
            break;
        }
        pItem = aIter.NextItem();
    }
    return retval;
}

ToxTextGenerator::HandledTextToken
ToxTextGenerator::HandleTextToken(const SwTOXSortTabBase& source, SwAttrPool& pool)
{
    HandledTextToken result;
    ToxWhitespaceStripper stripper(source.GetText().sText);
    result.text = stripper.GetStrippedString();

    const SwTextNode* pSrc = source.aTOXSources.at(0).pNd->GetTextNode();
    if (!pSrc->HasHints()) {
        return result;
    }
    const SwpHints& hints = pSrc->GetSwpHints();
    for (size_t i = 0; i < hints.Count(); ++i) {
        const SwTextAttr* hint = hints.Get(i);
        std::shared_ptr<SfxItemSet> attributesToClone = CollectAttributesForTox(*hint, pool);
        if (attributesToClone->Count() <= 0) {
            continue;
        }
        SwFormatAutoFormat* clone = static_cast<SwFormatAutoFormat*>(hint->GetAutoFormat().Clone());
        clone->SetStyleHandle(attributesToClone);

        result.autoFormats.push_back(clone);
        result.startPositions.push_back(stripper.GetPositionInStrippedString(hint->GetStart()));
        result.endPositions.push_back(stripper.GetPositionInStrippedString(*hint->GetAnyEnd()));
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
    OUString retval;
    if (numberOfToxSources == 0) {
        return retval;
    }
    // Place holder for the PageNumber; we only respect the first one
    retval += OUString(C_NUM_REPL);
    for (size_t i = 1; i < numberOfToxSources; ++i) {
        retval += S_PAGE_DELI;
        retval += OUString(C_NUM_REPL);
    }
    retval += OUString(C_END_PAGE_NUM);
    return retval;
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
    retval.ChangeExpansion(contentFrame, contentNode, true);
    return retval;
}
} // end namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
