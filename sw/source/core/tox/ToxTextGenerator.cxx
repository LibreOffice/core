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
#include "pagefrm.hxx"
#include "fchrfmt.hxx"
#include "doc.hxx"
#include "fmtinfmt.hxx"
#include "ndtxt.hxx"
#include "pagedesc.hxx"
#include "tox.hxx"
#include "txmsrt.hxx"
#include "fmtautofmt.hxx"
#include "fmtfsize.hxx"
#include "fmtpdsc.hxx"
#include "DocumentSettingManager.hxx"
#include "SwStyleNameMapper.hxx"
#include "swatrset.hxx"
#include "ToxWhitespaceStripper.hxx"
#include "ToxLinkProcessor.hxx"
#include "txatbase.hxx"

#include "editeng/tstpitem.hxx"
#include "editeng/lrspitem.hxx"
#include "rtl/ustring.hxx"
#include "svl/itemiter.hxx"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

namespace sw {

OUString
ToxTextGenerator::GetNumStringOfFirstNode( const SwTOXSortTabBase& rBase, bool bUsePrefix, sal_uInt8 nLevel )
{
    OUString sRet;
    if (!rBase.pTxtMark) { // only if it's not a Mark
        return sRet;
    }

    if (rBase.aTOXSources.empty()) {
        return sRet;
    }

    const SwTxtNode* pNd = rBase.aTOXSources[0].pNd->GetTxtNode();
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

    return sRet;
}


ToxTextGenerator::ToxTextGenerator(const SwForm& toxForm)
:mToxForm(toxForm),
 mLinkProcessor(new ToxLinkProcessor())
{;}

ToxTextGenerator::~ToxTextGenerator()
{;}

// Add parameter <_TOXSectNdIdx> and <_pDefaultPageDesc> in order to control,
// which page description is used, no appropriate one is found.
void ToxTextGenerator::GenerateText(SwDoc* pDoc, const std::vector<SwTOXSortTabBase*> &entries,
        sal_uInt16 indexOfEntryToProcess, sal_uInt16 numberOfEntriesToProcess, sal_uInt32 _nTOXSectNdIdx,
        const SwPageDesc* _pDefaultPageDesc)
{
    // pTOXNd is only set at the first mark
    SwTxtNode* pTOXNd = (SwTxtNode*)entries.at(indexOfEntryToProcess)->pTOXNd;
    // FIXME this operates directly on the node text
    OUString & rTxt = const_cast<OUString&>(pTOXNd->GetTxt());
    rTxt = "";
    for(sal_uInt16 nIndex = indexOfEntryToProcess; nIndex < indexOfEntryToProcess + numberOfEntriesToProcess; nIndex++)
    {
        if(nIndex > indexOfEntryToProcess)
            rTxt += ", "; // comma separation
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
            sal_Int32 nStartCharStyle = rTxt.getLength();
            switch( aToken.eTokenType )
            {
            case TOKEN_ENTRY_NO:
                // for TOC numbering
                rTxt += GetNumStringOfFirstNode( rBase, aToken.nChapterFormat == CF_NUMBER, static_cast<sal_uInt8>(aToken.nOutlineLevel - 1) ) ;
                break;

            case TOKEN_ENTRY_TEXT: {
                HandledTextToken htt = HandleTextToken(rBase, pDoc->GetAttrPool());
                ApplyHandledTextToken(htt, *pTOXNd);
            }
                break;

            case TOKEN_ENTRY:
                {
                    // for TOC numbering
                    rTxt += GetNumStringOfFirstNode( rBase, true, MAXLEVEL );
                    SwIndex aIdx( pTOXNd, rTxt.getLength() );
                    ToxWhitespaceStripper stripper(rBase.GetTxt().sText);
                    pTOXNd->InsertText(stripper.GetStrippedString(), aIdx);
                }
                break;

            case TOKEN_TAB_STOP:
                if (aToken.bWithTab) // #i21237#
                    rTxt += "\t";

                if(SVX_TAB_ADJUST_END > aToken.eTabAlign)
                {
                    const SvxLRSpaceItem& rLR =
                        (SvxLRSpaceItem&)pTOXNd->
                        SwCntntNode::GetAttr( RES_LR_SPACE, true );

                    long nTabPosition = aToken.nTabStopPosition;
                    if( !mToxForm.IsRelTabPos() && rLR.GetTxtLeft() )
                        nTabPosition -= rLR.GetTxtLeft();
                    aTStops.Insert( SvxTabStop( nTabPosition,
                                                aToken.eTabAlign,
                                                cDfltDecimalChar,
                                                aToken.cTabFillChar ));
                }
                else
                {
                    const SwPageDesc* pPageDesc = ((SwFmtPageDesc&)pTOXNd->
                                SwCntntNode::GetAttr( RES_PAGEDESC )).GetPageDesc();

                    bool bCallFindRect = true;
                    long nRightMargin;
                    if( pPageDesc )
                    {
                        const SwFrm* pFrm = pTOXNd->getLayoutFrm( pDoc->GetCurrentLayout(), 0, 0, true );
                        if( !pFrm || 0 == ( pFrm = pFrm->FindPageFrm() ) ||
                            pPageDesc != ((SwPageFrm*)pFrm)->GetPageDesc() )
                            // we have to go via the PageDesc here
                            bCallFindRect = false;
                    }

                    SwRect aNdRect;
                    if( bCallFindRect )
                        aNdRect = pTOXNd->FindLayoutRect( true );

                    if( aNdRect.IsEmpty() )
                    {
                        // Nothing helped so far, so we go via the PageDesc
                        sal_uInt32 nPgDescNdIdx = pTOXNd->GetIndex() + 1;
                        sal_uInt32* pPgDescNdIdx = &nPgDescNdIdx;
                        pPageDesc = pTOXNd->FindPageDesc( false, pPgDescNdIdx );
                        if ( !pPageDesc ||
                             *pPgDescNdIdx < _nTOXSectNdIdx )
                        {
                            // Use default page description, if none is found
                            // or the found one is given by a Node before the
                            // table-of-content section.
                            pPageDesc = _pDefaultPageDesc;
                        }

                        const SwFrmFmt& rPgDscFmt = pPageDesc->GetMaster();
                        nRightMargin = rPgDscFmt.GetFrmSize().GetWidth() -
                                         rPgDscFmt.GetLRSpace().GetLeft() -
                                         rPgDscFmt.GetLRSpace().GetRight();
                    }
                    else
                        nRightMargin = aNdRect.Width();
                    //#i24363# tab stops relative to indent
                    if( pDoc->GetDocumentSettingManager().get(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT) )
                    {
                        // left margin of paragraph style
                        const SvxLRSpaceItem& rLRSpace = pTOXNd->GetTxtColl()->GetLRSpace();
                        nRightMargin -= rLRSpace.GetLeft();
                        nRightMargin -= rLRSpace.GetTxtFirstLineOfst();
                    }

                    aTStops.Insert( SvxTabStop( nRightMargin, SVX_TAB_ADJUST_RIGHT,
                                                cDfltDecimalChar,
                                                aToken.cTabFillChar ));
                }
                break;

            case TOKEN_TEXT:
                rTxt += aToken.sText;
                break;

            case TOKEN_PAGE_NUMS:
                rTxt += ConstructPageNumberPlaceholder(rBase.aTOXSources.size());
                break;

            case TOKEN_CHAPTER_INFO:
                {
                    // A bit tricky: Find a random Frame
                    const SwTOXSource* pTOXSource = 0;
                    if (!rBase.aTOXSources.empty())
                        pTOXSource = &rBase.aTOXSources[0];

                    // #i53420#
                    if ( pTOXSource && pTOXSource->pNd &&
                         pTOXSource->pNd->IsCntntNode() )
                    {
                        const SwCntntFrm* pFrm = pTOXSource->pNd->getLayoutFrm( pDoc->GetCurrentLayout() );
                        if( pFrm )
                        {
                            SwChapterFieldType aFldTyp;
                            SwChapterField aFld( &aFldTyp, aToken.nChapterFormat );
                            aFld.SetLevel( static_cast<sal_uInt8>(aToken.nOutlineLevel - 1) );
                            // #i53420#
                            aFld.ChangeExpansion( pFrm,
                                dynamic_cast<const SwCntntNode*>(pTOXSource->pNd),
                                true );
                            //---> #i89791#
                            // continue to support CF_NUMBER
                            // and CF_NUM_TITLE in order to handle ODF 1.0/1.1
                            // written by OOo 3.x in the same way as OOo 2.x
                            // would handle them.
                            if ( CF_NUM_NOPREPST_TITLE == aToken.nChapterFormat ||
                                 CF_NUMBER == aToken.nChapterFormat )
                                rTxt += aFld.GetNumber(); // get the string number without pre/postfix
                            else if ( CF_NUMBER_NOPREPST == aToken.nChapterFormat ||
                                      CF_NUM_TITLE == aToken.nChapterFormat )
                            {
                                rTxt += aFld.GetNumber();
                                rTxt += " ";
                                rTxt += aFld.GetTitle();
                            }
                            else if(CF_TITLE == aToken.nChapterFormat)
                                rTxt += aFld.GetTitle();
                        }
                    }
                }
                break;

            case TOKEN_LINK_START:
                mLinkProcessor->StartNewLink(rTxt.getLength(), aToken.sCharStyleName);
                break;

            case TOKEN_LINK_END:
                mLinkProcessor->CloseLink(rTxt.getLength(), rBase.GetURL());
                break;

            case TOKEN_AUTHORITY:
                {
                    ToxAuthorityField eField = (ToxAuthorityField)aToken.nAuthorityField;
                    SwIndex aIdx( pTOXNd, rTxt.getLength() );
                    rBase.FillText( *pTOXNd, aIdx, static_cast<sal_uInt16>(eField) );
                }
                break;
            case TOKEN_END: break;
            }

            if ( !aToken.sCharStyleName.isEmpty() )
            {
                SwCharFmt* pCharFmt;
                if( USHRT_MAX != aToken.nPoolId )
                    pCharFmt = pDoc->GetCharFmtFromPool( aToken.nPoolId );
                else
                    pCharFmt = pDoc->FindCharFmtByName( aToken.sCharStyleName);

                if (pCharFmt)
                {
                    SwFmtCharFmt aFmt( pCharFmt );
                    pTOXNd->InsertItem( aFmt, nStartCharStyle,
                        rTxt.getLength(), nsSetAttrMode::SETATTR_DONTEXPAND );
                }
            }

            ++aIt; // #i21237#
        }

        pTOXNd->SetAttr( aTStops );
    }
    mLinkProcessor->InsertLinkAttributes(*pTOXNd);
}

/*static*/ boost::shared_ptr<SfxItemSet>
ToxTextGenerator::CollectAttributesForTox(const SwTxtAttr& hint, SwAttrPool& pool)
{
    boost::shared_ptr<SfxItemSet> retval = boost::make_shared<SfxItemSet>(pool);
    if (hint.Which() != RES_TXTATR_AUTOFMT) {
        return retval;
    }
    const SwFmtAutoFmt& afmt = hint.GetAutoFmt();
    SfxItemIter aIter( *afmt.GetStyleHandle());
    const SfxPoolItem* pItem = aIter.GetCurItem();
    while (true) {
        if (pItem->Which() == RES_CHRATR_ESCAPEMENT ||
            pItem->Which() == RES_CHRATR_POSTURE ||
            pItem->Which() == RES_CHRATR_CJK_POSTURE ||
            pItem->Which() == RES_CHRATR_CTL_POSTURE) {
            SfxPoolItem* clonedItem = pItem->Clone(NULL);
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
    ToxWhitespaceStripper stripper(source.GetTxt().sText);
    result.text = stripper.GetStrippedString();

    const SwTxtNode* pSrc = source.aTOXSources.at(0).pNd->GetTxtNode();
    if (!pSrc->HasHints()) {
        return result;
    }
    const SwpHints& hints = pSrc->GetSwpHints();
    for (sal_uInt16 i = 0; i < hints.Count(); ++i) {
        const SwTxtAttr* hint = hints[i];
        boost::shared_ptr<SfxItemSet> attributesToClone = CollectAttributesForTox(*hint, pool);
        if (attributesToClone->Count() <= 0) {
            continue;
        }
        SwFmtAutoFmt* clone = static_cast<SwFmtAutoFmt*>(hint->GetAutoFmt().Clone());
        clone->SetStyleHandle(attributesToClone);

        result.autoFormats.push_back(clone);
        result.startPositions.push_back(stripper.GetPositionInStrippedString(*hint->GetStart()));
        result.endPositions.push_back(stripper.GetPositionInStrippedString(*hint->GetAnyEnd()));
    }
    return result;
}

/*static*/ void
ToxTextGenerator::ApplyHandledTextToken(const HandledTextToken& htt, SwTxtNode& targetNode)
{
    sal_Int32 offset = targetNode.GetTxt().getLength();
    SwIndex aIdx(&targetNode, offset);
    targetNode.InsertText(htt.text, aIdx);
    for (size_t i=0; i < htt.autoFormats.size(); ++i) {
        targetNode.InsertItem(*htt.autoFormats.at(i),
                htt.startPositions.at(i) + offset,
                htt.endPositions.at(i) + offset);
    }
}

OUString
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

} // end namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
