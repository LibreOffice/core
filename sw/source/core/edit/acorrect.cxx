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

#include <svx/svxids.hrc>
#include <editeng/langitem.hxx>
#include <fmtinfmt.hxx>
#include <txtatr.hxx>
#include <txtinet.hxx>
#include <editsh.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <unocrsr.hxx>
#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <acorrect.hxx>
#include <shellio.hxx>
#include <swundo.hxx>
#include <viscrs.hxx>

#include <editeng/acorrcfg.hxx>
#include <o3tl/make_unique.hxx>

using namespace ::com::sun::star;

class PaMIntoCursorShellRing
{
    SwCursorShell& rSh;
    SwPaM &rDelPam, &rCursor;
    SwPaM* pPrevDelPam;
    SwPaM* pPrevCursor;

    static void RemoveFromRing( SwPaM& rPam, SwPaM const * pPrev );
public:
    PaMIntoCursorShellRing( SwCursorShell& rSh, SwPaM& rCursor, SwPaM& rPam );
    ~PaMIntoCursorShellRing();
};

PaMIntoCursorShellRing::PaMIntoCursorShellRing( SwCursorShell& rCSh,
                                            SwPaM& rShCursor, SwPaM& rPam )
    : rSh( rCSh ), rDelPam( rPam ), rCursor( rShCursor )
{
    SwPaM* pShCursor = rSh.GetCursor_();

    pPrevDelPam = rDelPam.GetPrev();
    pPrevCursor = rCursor.GetPrev();

    rDelPam.GetRingContainer().merge( pShCursor->GetRingContainer() );
    rCursor.GetRingContainer().merge( pShCursor->GetRingContainer() );
}

PaMIntoCursorShellRing::~PaMIntoCursorShellRing()
{
    // and take out the Pam again:
    RemoveFromRing( rDelPam, pPrevDelPam );
    RemoveFromRing( rCursor, pPrevCursor );
}

void PaMIntoCursorShellRing::RemoveFromRing( SwPaM& rPam, SwPaM const * pPrev )
{
    SwPaM* p;
    SwPaM* pNext = &rPam;
    do {
        p = pNext;
        pNext = p->GetNext();
        p->MoveTo( &rPam );
    } while( p != pPrev );
}

SwAutoCorrDoc::SwAutoCorrDoc( SwEditShell& rEditShell, SwPaM& rPam,
                                sal_Unicode cIns )
    : rEditSh( rEditShell ), rCursor( rPam )
    , m_nEndUndoCounter(0)
    , bUndoIdInitialized( cIns == 0 )
{
}

SwAutoCorrDoc::~SwAutoCorrDoc()
{
    for (int i = 0; i < m_nEndUndoCounter; ++i)
    {
        rEditSh.EndUndo();
    }
}

void SwAutoCorrDoc::DeleteSel( SwPaM& rDelPam )
{
    // this should work with plain SwPaM as well because start and end
    // are always in same node, but since there is GetRanges already...
    std::vector<std::shared_ptr<SwUnoCursor>> ranges;
    if (sw::GetRanges(ranges, *rEditSh.GetDoc(), rDelPam))
    {
        DeleteSelImpl(rDelPam);
    }
    else
    {
        for (auto const& pCursor : ranges)
        {
            DeleteSelImpl(*pCursor);
        }
    }
}

void SwAutoCorrDoc::DeleteSelImpl(SwPaM & rDelPam)
{
    SwDoc* pDoc = rEditSh.GetDoc();
    if( pDoc->IsAutoFormatRedline() )
    {
        // so that also the DelPam be moved,  include it in the
        // Shell-Cursr-Ring !!
        // ??? is that really necessary - this should never join nodes, so Update should be enough?
//        PaMIntoCursorShellRing aTmp( rEditSh, rCursor, rDelPam );
        assert(rDelPam.GetPoint()->nNode == rDelPam.GetMark()->nNode);
        pDoc->getIDocumentContentOperations().DeleteAndJoin( rDelPam );
    }
    else
    {
        pDoc->getIDocumentContentOperations().DeleteRange( rDelPam );
    }
}

bool SwAutoCorrDoc::Delete( sal_Int32 nStt, sal_Int32 nEnd )
{
    SwTextNode const*const pTextNd = rCursor.GetNode().GetTextNode();
    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                pTextNd->getLayoutFrame(rEditSh.GetLayout())));
    assert(pFrame);
    SwPaM aSel(pFrame->MapViewToModelPos(TextFrameIndex(nStt)),
               pFrame->MapViewToModelPos(TextFrameIndex(nEnd)));
    DeleteSel( aSel );

    if( bUndoIdInitialized )
        bUndoIdInitialized = true;
    return true;
}

bool SwAutoCorrDoc::Insert( sal_Int32 nPos, const OUString& rText )
{
    SwTextNode const*const pTextNd = rCursor.GetNode().GetTextNode();
    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                pTextNd->getLayoutFrame(rEditSh.GetLayout())));
    assert(pFrame);
    SwPaM aPam(pFrame->MapViewToModelPos(TextFrameIndex(nPos)));
    rEditSh.GetDoc()->getIDocumentContentOperations().InsertString( aPam, rText );
    if( !bUndoIdInitialized )
    {
        bUndoIdInitialized = true;
        if( 1 == rText.getLength() )
        {
            rEditSh.StartUndo( SwUndoId::AUTOCORRECT );
            ++m_nEndUndoCounter;
        }
    }
    return true;
}

bool SwAutoCorrDoc::Replace( sal_Int32 nPos, const OUString& rText )
{
    return ReplaceRange( nPos, rText.getLength(), rText );
}

bool SwAutoCorrDoc::ReplaceRange( sal_Int32 nPos, sal_Int32 nSourceLength, const OUString& rText )
{
    assert(nSourceLength == 1); // sw_redlinehide: this is currently the case,
    // and ensures that the replace range cannot *contain* delete redlines,
    // so we don't need something along the lines of:
    //    if (sw::GetRanges(ranges, *rEditSh.GetDoc(), aPam))
    //        ReplaceImpl(...)
    //    else
    //        ReplaceImpl(ranges.begin())
    //        for (ranges.begin() + 1; ranges.end(); )
    //            DeleteImpl(*it)

    SwTextNode * const pNd = rCursor.GetNode().GetTextNode();
    if ( !pNd )
    {
        return false;
    }

    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                pNd->getLayoutFrame(rEditSh.GetLayout())));
    assert(pFrame);
    std::pair<SwTextNode *, sal_Int32> const pos(pFrame->MapViewToModel(TextFrameIndex(nPos)));

    SwPaM* pPam = &rCursor;
    if (pPam->GetPoint()->nNode != *pos.first
        || pPam->GetPoint()->nContent != pos.second)
    {
        pPam = new SwPaM(*pos.first, pos.second);
    }

    // text attributes with dummy characters must not be replaced!
    bool bDoReplace = true;
    sal_Int32 const nLen = rText.getLength();
    for (sal_Int32 n = 0; n < nLen && n + nPos < pFrame->GetText().getLength(); ++n)
    {
        sal_Unicode const Char = pFrame->GetText()[n + nPos];
        if (CH_TXTATR_BREAKWORD == Char || CH_TXTATR_INWORD == Char)
        {
            assert(pFrame->MapViewToModel(TextFrameIndex(n+nPos)).first->GetTextAttrForCharAt(pFrame->MapViewToModel(TextFrameIndex(n+nPos)).second));
            bDoReplace = false;
            break;
        }
    }

    if ( bDoReplace )
    {
        SwDoc* pDoc = rEditSh.GetDoc();

        if( pDoc->IsAutoFormatRedline() )
        {
            if (nPos == pFrame->GetText().getLength()) // at the End do an Insert
            {
                pDoc->getIDocumentContentOperations().InsertString( *pPam, rText );
            }
            else
            {
                assert(pos.second != pos.first->Len()); // must be _before_ char
                PaMIntoCursorShellRing aTmp( rEditSh, rCursor, *pPam );

                pPam->SetMark();
                pPam->GetPoint()->nContent = std::min<sal_Int32>(
                    pos.first->GetText().getLength(), pos.second + nSourceLength);
                pDoc->getIDocumentContentOperations().ReplaceRange( *pPam, rText, false );
                pPam->Exchange();
                pPam->DeleteMark();
            }
        }
        else
        {
            if( nSourceLength != rText.getLength() )
            {
                pPam->SetMark();
                pPam->GetPoint()->nContent = std::min<sal_Int32>(
                    pos.first->GetText().getLength(), pos.second + nSourceLength);
                pDoc->getIDocumentContentOperations().ReplaceRange( *pPam, rText, false );
                pPam->Exchange();
                pPam->DeleteMark();
            }
            else
                pDoc->getIDocumentContentOperations().Overwrite( *pPam, rText );
        }

        if( bUndoIdInitialized )
        {
            bUndoIdInitialized = true;
            if( 1 == rText.getLength() )
            {
                rEditSh.StartUndo( SwUndoId::AUTOCORRECT );
                ++m_nEndUndoCounter;
            }
        }
    }

    if( pPam != &rCursor )
        delete pPam;

    return true;
}

void SwAutoCorrDoc::SetAttr( sal_Int32 nStt, sal_Int32 nEnd, sal_uInt16 nSlotId,
                                        SfxPoolItem& rItem )
{
    SwTextNode const*const pTextNd = rCursor.GetNode().GetTextNode();
    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                pTextNd->getLayoutFrame(rEditSh.GetLayout())));
    assert(pFrame);
    SwPaM aPam(pFrame->MapViewToModelPos(TextFrameIndex(nStt)),
               pFrame->MapViewToModelPos(TextFrameIndex(nEnd)));

    SfxItemPool& rPool = rEditSh.GetDoc()->GetAttrPool();
    sal_uInt16 nWhich = rPool.GetWhich( nSlotId, false );
    if( nWhich )
    {
        rItem.SetWhich( nWhich );

        SfxItemSet aSet( rPool, aCharFormatSetRange );
        SetAllScriptItem( aSet, rItem );

        rEditSh.GetDoc()->SetFormatItemByAutoFormat( aPam, aSet );

        if( bUndoIdInitialized )
            bUndoIdInitialized = true;
    }
}

bool SwAutoCorrDoc::SetINetAttr( sal_Int32 nStt, sal_Int32 nEnd, const OUString& rURL )
{
    SwTextNode const*const pTextNd = rCursor.GetNode().GetTextNode();
    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                pTextNd->getLayoutFrame(rEditSh.GetLayout())));
    assert(pFrame);
    SwPaM aPam(pFrame->MapViewToModelPos(TextFrameIndex(nStt)),
               pFrame->MapViewToModelPos(TextFrameIndex(nEnd)));

    SfxItemSet aSet( rEditSh.GetDoc()->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>{} );
    aSet.Put( SwFormatINetFormat( rURL, OUString() ));
    rEditSh.GetDoc()->SetFormatItemByAutoFormat( aPam, aSet );
    if( bUndoIdInitialized )
        bUndoIdInitialized = true;
    return true;
}

/** Return the text of a previous paragraph
 *
 * @param bAtNormalPos If <true> before the normal insert position; if <false> in which the
 *                     corrected word was inserted. (Doesn't need to be the same paragraph!)
 * @return text or 0, if previous paragraph does not exists or there are only blankness
 */
OUString const* SwAutoCorrDoc::GetPrevPara(bool const bAtNormalPos)
{
    OUString const* pStr(nullptr);

    if( bAtNormalPos || !pIdx )
    {
        pIdx.reset(new SwNodeIndex(rCursor.GetPoint()->nNode));
    }
    sw::GotoPrevLayoutTextFrame(*pIdx, rEditSh.GetLayout());

    SwTextFrame const* pFrame(nullptr);
    for (SwTextNode * pTextNd = pIdx->GetNode().GetTextNode();
             pTextNd; pTextNd = pIdx->GetNode().GetTextNode())
    {
        pFrame = static_cast<SwTextFrame const*>(
                pTextNd->getLayoutFrame(rEditSh.GetLayout()));
        if (pFrame && !pFrame->GetText().isEmpty())
        {
            break;
        }
        sw::GotoPrevLayoutTextFrame(*pIdx, rEditSh.GetLayout());
    }
    if (pFrame && 0 == pFrame->GetTextNodeForParaProps()->GetAttrOutlineLevel())
        pStr = & pFrame->GetText();

    if( bUndoIdInitialized )
        bUndoIdInitialized = true;

    return pStr;
}

bool SwAutoCorrDoc::ChgAutoCorrWord( sal_Int32& rSttPos, sal_Int32 nEndPos,
                                         SvxAutoCorrect& rACorrect,
                                         OUString* pPara )
{
    if( bUndoIdInitialized )
        bUndoIdInitialized = true;

    // Found a beginning of a paragraph or a Blank,
    // search for the word Kuerzel (Shortcut) in the Auto
    SwTextNode* pTextNd = rCursor.GetNode().GetTextNode();
    OSL_ENSURE( pTextNd, "where is the TextNode?" );

    bool bRet = false;
    if( nEndPos == rSttPos )
        return bRet;

    LanguageType eLang = GetLanguage(nEndPos);
    if(LANGUAGE_SYSTEM == eLang)
        eLang = GetAppLanguage();
    LanguageTag aLanguageTag( eLang);

    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                pTextNd->getLayoutFrame(rEditSh.GetLayout())));
    assert(pFrame);

    //JP 22.04.99: Bug 63883 - Special treatment for dots.
    bool bLastCharIsPoint = nEndPos < pFrame->GetText().getLength() &&
                            ('.' == pFrame->GetText()[nEndPos]);

    const SvxAutocorrWord* pFnd = rACorrect.SearchWordsInList(
                pFrame->GetText(), rSttPos, nEndPos, *this, aLanguageTag);
    SwDoc* pDoc = rEditSh.GetDoc();
    if( pFnd )
    {
        // replace also last colon of keywords surrounded by colons (for example, ":name:")
        bool replaceLastChar = pFnd->GetShort()[0] == ':' && pFnd->GetShort().endsWith(":");

        SwPaM aPam(pFrame->MapViewToModelPos(TextFrameIndex(rSttPos)),
                   pFrame->MapViewToModelPos(TextFrameIndex(nEndPos + (replaceLastChar ? 1 : 0))));

        if( pFnd->IsTextOnly() )
        {
            //JP 22.04.99: Bug 63883 - Special treatment for dots.
            if( !bLastCharIsPoint || pFnd->GetLong().isEmpty() ||
                '.' != pFnd->GetLong()[ pFnd->GetLong().getLength() - 1 ] )
            {
                // replace the selection
                std::vector<std::shared_ptr<SwUnoCursor>> ranges;
                if (sw::GetRanges(ranges, *rEditSh.GetDoc(), aPam))
                {
                    pDoc->getIDocumentContentOperations().ReplaceRange(aPam, pFnd->GetLong(), false);
                }
                else
                {
                    assert(!ranges.empty());
                    assert(ranges.front()->GetPoint()->nNode == ranges.front()->GetMark()->nNode);
                    pDoc->getIDocumentContentOperations().ReplaceRange(
                            *ranges.front(), pFnd->GetLong(), false);
                    for (auto it = ranges.begin() + 1; it != ranges.end(); ++it)
                    {
                        DeleteSelImpl(**it);
                    }
                }

                // tdf#83260 After calling sw::DocumentContentOperationsManager::ReplaceRange
                // pTextNd may become invalid when change tracking is on and Edit -> Track Changes -> Show == OFF.
                // ReplaceRange shows changes, this moves deleted nodes from special section to document.
                // Then Show mode is disabled again. As a result pTextNd may be invalidated.
                pTextNd = rCursor.GetNode().GetTextNode();

                bRet = true;
            }
        }
        else
        {
            SwTextBlocks aTBlks( rACorrect.GetAutoCorrFileName( aLanguageTag, false, true ));
            sal_uInt16 nPos = aTBlks.GetIndex( pFnd->GetShort() );
            if( USHRT_MAX != nPos && aTBlks.BeginGetDoc( nPos ) )
            {
                DeleteSel( aPam );
                pDoc->DontExpandFormat( *aPam.GetPoint() );

                if( pPara )
                {
                    OSL_ENSURE( !pIdx, "who has not deleted his Index?" );
                    pIdx.reset(new SwNodeIndex( rCursor.GetPoint()->nNode ));
                    sw::GotoPrevLayoutTextFrame(*pIdx, rEditSh.GetLayout());
                }

                SwDoc* pAutoDoc = aTBlks.GetDoc();
                SwNodeIndex aSttIdx( pAutoDoc->GetNodes().GetEndOfExtras(), 1 );
                SwContentNode* pContentNd = pAutoDoc->GetNodes().GoNext( &aSttIdx );
                SwPaM aCpyPam( aSttIdx );

                const SwTableNode* pTableNd = pContentNd->FindTableNode();
                if( pTableNd )
                {
                    aCpyPam.GetPoint()->nContent.Assign( nullptr, 0 );
                    aCpyPam.GetPoint()->nNode = *pTableNd;
                }
                aCpyPam.SetMark();

                // then until the end of the Nodes Array
                aCpyPam.GetPoint()->nNode.Assign( pAutoDoc->GetNodes().GetEndOfContent(), -1 );
                pContentNd = aCpyPam.GetContentNode();
                aCpyPam.GetPoint()->nContent.Assign(
                       pContentNd, pContentNd ? pContentNd->Len() : 0);

                SwDontExpandItem aExpItem;
                aExpItem.SaveDontExpandItems( *aPam.GetPoint() );

                pAutoDoc->getIDocumentContentOperations().CopyRange( aCpyPam, *aPam.GetPoint(), /*bCopyAll=*/false, /*bCheckPos=*/true );

                aExpItem.RestoreDontExpandItems( *aPam.GetPoint() );

                if( pPara )
                {
                    sw::GotoNextLayoutTextFrame(*pIdx, rEditSh.GetLayout());
                    pTextNd = pIdx->GetNode().GetTextNode();
                }
                bRet = true;
            }
            aTBlks.EndGetDoc();
        }
    }

    if( bRet && pPara && pTextNd )
    {
        SwTextFrame const*const pNewFrame(static_cast<SwTextFrame const*>(
                    pTextNd->getLayoutFrame(rEditSh.GetLayout())));
        *pPara = pNewFrame->GetText();
    }

    return bRet;
}

// Called by the functions:
//  - FnCapitalStartWord
//  - FnCapitalStartSentence
// after the exchange of characters. Then the words, if necessary, can be inserted
// into the exception list.
void SwAutoCorrDoc::SaveCpltSttWord( ACFlags nFlag, sal_Int32 nPos,
                                            const OUString& rExceptWord,
                                            sal_Unicode cChar )
{
    sal_uLong nNode = pIdx ? pIdx->GetIndex() : rCursor.GetPoint()->nNode.GetIndex();
    LanguageType eLang = GetLanguage(nPos);
    rEditSh.GetDoc()->SetAutoCorrExceptWord( o3tl::make_unique<SwAutoCorrExceptWord>( nFlag,
                                        nNode, nPos, rExceptWord, cChar, eLang ));
}

LanguageType SwAutoCorrDoc::GetLanguage( sal_Int32 nPos ) const
{
    LanguageType eRet = LANGUAGE_SYSTEM;

    SwTextNode* pNd = rCursor.GetPoint()->nNode.GetNode().GetTextNode();

    if( pNd )
    {
        SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                    pNd->getLayoutFrame(rEditSh.GetLayout())));
        assert(pFrame);
        eRet = pFrame->GetLangOfChar(TextFrameIndex(nPos), 0, true);
    }
    if(LANGUAGE_SYSTEM == eRet)
        eRet = GetAppLanguage();
    return eRet;
}

void SwAutoCorrExceptWord::CheckChar( const SwPosition& rPos, sal_Unicode cChr )
{
    // test only if this is a improvement.
    // If yes, then add the word to the list.
    if (m_cChar == cChr && rPos.nNode.GetIndex() == m_nNode && rPos.nContent.GetIndex() == m_nContent)
    {
        // get the current autocorrection:
        SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get().GetAutoCorrect();

        // then add to the list:
        if (ACFlags::CapitalStartWord & m_nFlags)
            pACorr->AddWrtSttException(m_sWord, m_eLanguage);
        else if (ACFlags::CapitalStartSentence & m_nFlags)
            pACorr->AddCplSttException(m_sWord, m_eLanguage);
    }
}

bool SwAutoCorrExceptWord::CheckDelChar( const SwPosition& rPos )
{
    bool bRet = false;
    if (!m_bDeleted && rPos.nNode.GetIndex() == m_nNode && rPos.nContent.GetIndex() == m_nContent)
        m_bDeleted = bRet = true;
    return bRet;
}

SwDontExpandItem::~SwDontExpandItem()
{
}

void SwDontExpandItem::SaveDontExpandItems( const SwPosition& rPos )
{
    const SwTextNode* pTextNd = rPos.nNode.GetNode().GetTextNode();
    if( pTextNd )
    {
        pDontExpItems.reset( new SfxItemSet( const_cast<SwDoc*>(pTextNd->GetDoc())->GetAttrPool(),
                                            aCharFormatSetRange ) );
        const sal_Int32 n = rPos.nContent.GetIndex();
        if (!pTextNd->GetParaAttr( *pDontExpItems, n, n,
                                n != pTextNd->GetText().getLength() ))
        {
            pDontExpItems.reset();
        }
    }
}

void SwDontExpandItem::RestoreDontExpandItems( const SwPosition& rPos )
{
    SwTextNode* pTextNd = rPos.nNode.GetNode().GetTextNode();
    if( pTextNd )
    {
        const sal_Int32 nStart = rPos.nContent.GetIndex();
        if( nStart == pTextNd->GetText().getLength() )
            pTextNd->FormatToTextAttr( pTextNd );

        if( pTextNd->GetpSwpHints() && pTextNd->GetpSwpHints()->Count() )
        {
            const size_t nSize = pTextNd->GetpSwpHints()->Count();
            sal_Int32 nAttrStart;

            for( size_t n = 0; n < nSize; ++n )
            {
                SwTextAttr* pHt = pTextNd->GetpSwpHints()->Get( n );
                nAttrStart = pHt->GetStart();
                if( nAttrStart > nStart )       // beyond the area
                    break;

                const sal_Int32* pAttrEnd;
                if( nullptr != ( pAttrEnd = pHt->End() ) &&
                    ( ( nAttrStart < nStart &&
                        ( pHt->DontExpand() ? nStart < *pAttrEnd
                                            : nStart <= *pAttrEnd )) ||
                      ( nStart == nAttrStart &&
                        ( nAttrStart == *pAttrEnd || !nStart ))) )
                {
                    const SfxPoolItem* pItem;
                    if( !pDontExpItems || SfxItemState::SET != pDontExpItems->
                        GetItemState( pHt->Which(), false, &pItem ) ||
                        *pItem != pHt->GetAttr() )
                    {
                        // The attribute was not previously set in this form in the
                        // paragraph, so it can only be created through insert/copy
                        // Because of that it is a candidate for DontExpand
                        pHt->SetDontExpand( true );
                    }
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
