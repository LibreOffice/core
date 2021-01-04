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

#include <fmtinfmt.hxx>
#include <editsh.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <unocrsr.hxx>
#include <txatbase.hxx>
#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <acorrect.hxx>
#include <shellio.hxx>
#include <swundo.hxx>
#include <viscrs.hxx>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <osl/diagnose.h>
#include <svl/zforlist.hxx>

#include <editeng/acorrcfg.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <rootfrm.hxx>

using namespace ::com::sun::star;

namespace {

class PaMIntoCursorShellRing
{
    SwPaM &m_rDelPam, &m_rCursor;
    SwPaM* m_pPrevDelPam;
    SwPaM* m_pPrevCursor;

    static void RemoveFromRing( SwPaM& rPam, SwPaM const * pPrev );
public:
    PaMIntoCursorShellRing( SwCursorShell& rSh, SwPaM& rCursor, SwPaM& rPam );
    ~PaMIntoCursorShellRing();
};

}

PaMIntoCursorShellRing::PaMIntoCursorShellRing(SwCursorShell& rCSh, SwPaM& rShCursor, SwPaM& rPam)
    : m_rDelPam(rPam)
    , m_rCursor(rShCursor)
{
    SwPaM* pShCursor = rCSh.GetCursor_();

    m_pPrevDelPam = m_rDelPam.GetPrev();
    m_pPrevCursor = m_rCursor.GetPrev();

    m_rDelPam.GetRingContainer().merge(pShCursor->GetRingContainer());
    m_rCursor.GetRingContainer().merge(pShCursor->GetRingContainer());
}

PaMIntoCursorShellRing::~PaMIntoCursorShellRing()
{
    // and take out the Pam again:
    RemoveFromRing(m_rDelPam, m_pPrevDelPam);
    RemoveFromRing(m_rCursor, m_pPrevCursor);
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
    : m_rEditSh( rEditShell ), m_rCursor( rPam )
    , m_nEndUndoCounter(0)
    , m_bUndoIdInitialized( cIns == 0 )
{
}

SwAutoCorrDoc::~SwAutoCorrDoc()
{
    for (int i = 0; i < m_nEndUndoCounter; ++i)
    {
        m_rEditSh.EndUndo();
    }
}

void SwAutoCorrDoc::DeleteSel( SwPaM& rDelPam )
{
    // this should work with plain SwPaM as well because start and end
    // are always in same node, but since there is GetRanges already...
    std::vector<std::shared_ptr<SwUnoCursor>> ranges;
    if (sw::GetRanges(ranges, *m_rEditSh.GetDoc(), rDelPam))
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
    SwDoc* pDoc = m_rEditSh.GetDoc();
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
    SwTextNode const*const pTextNd = m_rCursor.GetNode().GetTextNode();
    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                pTextNd->getLayoutFrame(m_rEditSh.GetLayout())));
    assert(pFrame);
    SwPaM aSel(pFrame->MapViewToModelPos(TextFrameIndex(nStt)),
               pFrame->MapViewToModelPos(TextFrameIndex(nEnd)));
    DeleteSel( aSel );

    if( m_bUndoIdInitialized )
        m_bUndoIdInitialized = true;
    return true;
}

bool SwAutoCorrDoc::Insert( sal_Int32 nPos, const OUString& rText )
{
    SwTextNode const*const pTextNd = m_rCursor.GetNode().GetTextNode();
    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                pTextNd->getLayoutFrame(m_rEditSh.GetLayout())));
    assert(pFrame);
    SwPaM aPam(pFrame->MapViewToModelPos(TextFrameIndex(nPos)));
    m_rEditSh.GetDoc()->getIDocumentContentOperations().InsertString( aPam, rText );
    if( !m_bUndoIdInitialized )
    {
        m_bUndoIdInitialized = true;
        if( 1 == rText.getLength() )
        {
            m_rEditSh.StartUndo( SwUndoId::AUTOCORRECT );
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

    SwTextNode * const pNd = m_rCursor.GetNode().GetTextNode();
    if ( !pNd )
    {
        return false;
    }

    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                pNd->getLayoutFrame(m_rEditSh.GetLayout())));
    assert(pFrame);
    std::pair<SwTextNode *, sal_Int32> const pos(pFrame->MapViewToModel(TextFrameIndex(nPos)));

    SwPaM* pPam = &m_rCursor;
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

    // tdf#83419 avoid bad autocorrect with visible redlines
    // e.g. replacing the first letter of the tracked deletion
    // with its capitalized (and not deleted) version.
    if ( bDoReplace && !pFrame->getRootFrame()->IsHideRedlines() &&
         m_rEditSh.GetDoc()->getIDocumentRedlineAccess().HasRedline( *pPam, RedlineType::Delete, /*bStartOrEndInRange=*/false ) )
    {
        bDoReplace = false;
    }

    if ( bDoReplace )
    {
        SwDoc* pDoc = m_rEditSh.GetDoc();

        if( pDoc->IsAutoFormatRedline() )
        {
            if (nPos == pFrame->GetText().getLength()) // at the End do an Insert
            {
                pDoc->getIDocumentContentOperations().InsertString( *pPam, rText );
            }
            else
            {
                assert(pos.second != pos.first->Len()); // must be _before_ char
                PaMIntoCursorShellRing aTmp( m_rEditSh, m_rCursor, *pPam );

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
            pPam->SetMark();
            pPam->GetPoint()->nContent = std::min<sal_Int32>(
                pos.first->GetText().getLength(), pos.second + nSourceLength);
            pDoc->getIDocumentContentOperations().ReplaceRange( *pPam, rText, false );
            pPam->Exchange();
            pPam->DeleteMark();
        }

        if( m_bUndoIdInitialized )
        {
            m_bUndoIdInitialized = true;
            if( 1 == rText.getLength() )
            {
                m_rEditSh.StartUndo( SwUndoId::AUTOCORRECT );
                ++m_nEndUndoCounter;
            }
        }
    }

    if( pPam != &m_rCursor )
        delete pPam;

    return true;
}

void SwAutoCorrDoc::SetAttr( sal_Int32 nStt, sal_Int32 nEnd, sal_uInt16 nSlotId,
                                        SfxPoolItem& rItem )
{
    SwTextNode const*const pTextNd = m_rCursor.GetNode().GetTextNode();
    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                pTextNd->getLayoutFrame(m_rEditSh.GetLayout())));
    assert(pFrame);
    SwPaM aPam(pFrame->MapViewToModelPos(TextFrameIndex(nStt)),
               pFrame->MapViewToModelPos(TextFrameIndex(nEnd)));

    SfxItemPool& rPool = m_rEditSh.GetDoc()->GetAttrPool();
    sal_uInt16 nWhich = rPool.GetWhich( nSlotId, false );
    if( nWhich )
    {
        rItem.SetWhich( nWhich );

        SfxItemSet aSet( rPool, aCharFormatSetRange );
        SetAllScriptItem( aSet, rItem );

        m_rEditSh.GetDoc()->SetFormatItemByAutoFormat( aPam, aSet );

        if( m_bUndoIdInitialized )
            m_bUndoIdInitialized = true;
    }
}

bool SwAutoCorrDoc::SetINetAttr( sal_Int32 nStt, sal_Int32 nEnd, const OUString& rURL )
{
    SwTextNode const*const pTextNd = m_rCursor.GetNode().GetTextNode();
    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                pTextNd->getLayoutFrame(m_rEditSh.GetLayout())));
    assert(pFrame);
    SwPaM aPam(pFrame->MapViewToModelPos(TextFrameIndex(nStt)),
               pFrame->MapViewToModelPos(TextFrameIndex(nEnd)));

    SfxItemSet aSet( m_rEditSh.GetDoc()->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>{} );
    aSet.Put( SwFormatINetFormat( rURL, OUString() ));
    m_rEditSh.GetDoc()->SetFormatItemByAutoFormat( aPam, aSet );
    if( m_bUndoIdInitialized )
        m_bUndoIdInitialized = true;
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

    if( bAtNormalPos || !m_pIndex )
    {
        m_pIndex.reset(new SwNodeIndex(m_rCursor.GetPoint()->nNode));
    }
    sw::GotoPrevLayoutTextFrame(*m_pIndex, m_rEditSh.GetLayout());

    SwTextFrame const* pFrame(nullptr);
    for (SwTextNode * pTextNd = m_pIndex->GetNode().GetTextNode();
             pTextNd; pTextNd = m_pIndex->GetNode().GetTextNode())
    {
        pFrame = static_cast<SwTextFrame const*>(
                pTextNd->getLayoutFrame(m_rEditSh.GetLayout()));
        if (pFrame && !pFrame->GetText().isEmpty())
        {
            break;
        }
        sw::GotoPrevLayoutTextFrame(*m_pIndex, m_rEditSh.GetLayout());
    }
    if (pFrame && 0 == pFrame->GetTextNodeForParaProps()->GetAttrOutlineLevel())
        pStr = & pFrame->GetText();

    if( m_bUndoIdInitialized )
        m_bUndoIdInitialized = true;

    return pStr;
}

bool SwAutoCorrDoc::ChgAutoCorrWord( sal_Int32& rSttPos, sal_Int32 nEndPos,
                                         SvxAutoCorrect& rACorrect,
                                         OUString* pPara )
{
    if( m_bUndoIdInitialized )
        m_bUndoIdInitialized = true;

    // Found a beginning of a paragraph or a Blank,
    // search for the word Kuerzel (Shortcut) in the Auto
    SwTextNode* pTextNd = m_rCursor.GetNode().GetTextNode();
    OSL_ENSURE( pTextNd, "where is the TextNode?" );

    bool bRet = false;
    if( nEndPos == rSttPos )
        return bRet;

    LanguageType eLang = GetLanguage(nEndPos);
    if(LANGUAGE_SYSTEM == eLang)
        eLang = GetAppLanguage();
    LanguageTag aLanguageTag( eLang);

    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                pTextNd->getLayoutFrame(m_rEditSh.GetLayout())));
    assert(pFrame);

    const OUString sFrameText = pFrame->GetText();
    const SvxAutocorrWord* pFnd = rACorrect.SearchWordsInList(
                sFrameText, rSttPos, nEndPos, *this, aLanguageTag);
    SwDoc* pDoc = m_rEditSh.GetDoc();
    if( pFnd )
    {
        // replace also last colon of keywords surrounded by colons (for example, ":name:")
        const bool replaceLastChar = sFrameText.getLength() > nEndPos && pFnd->GetShort()[0] == ':'
                                     && pFnd->GetShort().endsWith(":");

        SwPosition aStartPos( pFrame->MapViewToModelPos(TextFrameIndex(rSttPos) ));
        SwPosition aEndPos( pFrame->MapViewToModelPos(TextFrameIndex(nEndPos + (replaceLastChar ? 1 : 0))) );
        SwPaM aPam(aStartPos, aEndPos);

        // don't replace, if a redline starts or ends within the original text
        if ( pDoc->getIDocumentRedlineAccess().HasRedline( aPam, RedlineType::Any, /*bStartOrEndInRange=*/true ) )
        {
            return bRet;
        }

        if( pFnd->IsTextOnly() )
        {
            //JP 22.04.99: Bug 63883 - Special treatment for dots.
            const bool bLastCharIsPoint
                = nEndPos < sFrameText.getLength() && ('.' == sFrameText[nEndPos]);
            if( !bLastCharIsPoint || pFnd->GetLong().isEmpty() ||
                '.' != pFnd->GetLong()[ pFnd->GetLong().getLength() - 1 ] )
            {
                // replace the selection
                std::vector<std::shared_ptr<SwUnoCursor>> ranges;
                if (sw::GetRanges(ranges, *m_rEditSh.GetDoc(), aPam))
                {
                    pDoc->getIDocumentContentOperations().ReplaceRange(aPam, pFnd->GetLong(), false);
                    bRet = true;
                }
                else if (!ranges.empty())
                {
                    assert(ranges.front()->GetPoint()->nNode == ranges.front()->GetMark()->nNode);
                    pDoc->getIDocumentContentOperations().ReplaceRange(
                            *ranges.front(), pFnd->GetLong(), false);
                    for (auto it = ranges.begin() + 1; it != ranges.end(); ++it)
                    {
                        DeleteSelImpl(**it);
                    }
                    bRet = true;
                }

                // tdf#83260 After calling sw::DocumentContentOperationsManager::ReplaceRange
                // pTextNd may become invalid when change tracking is on and Edit -> Track Changes -> Show == OFF.
                // ReplaceRange shows changes, this moves deleted nodes from special section to document.
                // Then Show mode is disabled again. As a result pTextNd may be invalidated.
                pTextNd = m_rCursor.GetNode().GetTextNode();
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
                    OSL_ENSURE( !m_pIndex, "who has not deleted his Index?" );
                    m_pIndex.reset(new SwNodeIndex( m_rCursor.GetPoint()->nNode ));
                    sw::GotoPrevLayoutTextFrame(*m_pIndex, m_rEditSh.GetLayout());
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

                pAutoDoc->getIDocumentContentOperations().CopyRange(aCpyPam, *aPam.GetPoint(), SwCopyFlags::CheckPosInFly);

                aExpItem.RestoreDontExpandItems( *aPam.GetPoint() );

                if( pPara )
                {
                    sw::GotoNextLayoutTextFrame(*m_pIndex, m_rEditSh.GetLayout());
                    pTextNd = m_pIndex->GetNode().GetTextNode();
                }
                bRet = true;
            }
            aTBlks.EndGetDoc();
        }
    }

    if( bRet && pPara && pTextNd )
    {
        SwTextFrame const*const pNewFrame(static_cast<SwTextFrame const*>(
                    pTextNd->getLayoutFrame(m_rEditSh.GetLayout())));
        *pPara = pNewFrame->GetText();
    }

    return bRet;
}

bool SwAutoCorrDoc::TransliterateRTLWord( sal_Int32& rSttPos, sal_Int32 nEndPos )
{
    if( m_bUndoIdInitialized )
        m_bUndoIdInitialized = true;

    SwTextNode* pTextNd = m_rCursor.GetNode().GetTextNode();
    OSL_ENSURE( pTextNd, "where is the TextNode?" );

    bool bRet = false;
    if( nEndPos == rSttPos )
        return bRet;

    LanguageType eLang = GetLanguage(nEndPos);
    if(LANGUAGE_SYSTEM == eLang)
        eLang = GetAppLanguage();
    LanguageTag aLanguageTag(eLang);

    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                pTextNd->getLayoutFrame(m_rEditSh.GetLayout())));
    assert(pFrame);

    const OUString sFrameText = pFrame->GetText();
    SwDoc* pDoc = m_rEditSh.GetDoc();
    if ( pFrame->IsRightToLeft() )
    {
        // transliterate to Old Hungarian using Numbertext via NatNum12 number format modifier
        OUString sWord(sFrameText.copy(rSttPos, nEndPos - rSttPos));
        // Consonant disambiguation using hyphenation
        uno::Reference< linguistic2::XHyphenator >  xHyph;
        xHyph = ::GetHyphenator();
        OUStringBuffer sDisambiguatedWord;

        const ::css::uno::Sequence< ::css::beans::PropertyValue > aProperties;
        css::uno::Reference< css::linguistic2::XHyphenatedWord >  xHyphWord;
        for (int i = 0; i+1 < sWord.getLength(); i++ )
        {
            xHyphWord = xHyph->hyphenate( sWord,
                        aLanguageTag.getLocale(),
                        i,
                        aProperties );
            // insert ZWSP at a hyphenation point, if it's not an alternative one (i.e. ssz->sz-sz)
            if (xHyphWord.is() && xHyphWord->getHyphenationPos()+1 == i && !xHyphWord->isAlternativeSpelling())
            {
                sDisambiguatedWord.append(CHAR_ZWSP);
            }
            sDisambiguatedWord.append(sWord[i]);
        }
        sDisambiguatedWord.append(sWord[sWord.getLength()-1]);

        SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();
        OUString sConverted;
        if (pFormatter && !sWord.isEmpty())
        {
            const Color* pColor = nullptr;
            // Send text as NatNum12 prefix
            OUString sPrefix("[NatNum12 " + sDisambiguatedWord.makeStringAndClear() + "]0");
            if (pFormatter->GetPreviewString(sPrefix, 0, sConverted, &pColor, LANGUAGE_USER_HUNGARIAN_ROVAS))
                bRet = true;
        }

        SwPaM aPam(pFrame->MapViewToModelPos(TextFrameIndex(rSttPos)),
            pFrame->MapViewToModelPos(TextFrameIndex(nEndPos)));
        if (bRet && nEndPos <= sFrameText.getLength())
            pDoc->getIDocumentContentOperations().ReplaceRange(aPam, sConverted, false);
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
    sal_uLong nNode = m_pIndex ? m_pIndex->GetIndex() : m_rCursor.GetPoint()->nNode.GetIndex();
    LanguageType eLang = GetLanguage(nPos);
    m_rEditSh.GetDoc()->SetAutoCorrExceptWord( std::make_unique<SwAutoCorrExceptWord>( nFlag,
                                        nNode, nPos, rExceptWord, cChar, eLang ));
}

LanguageType SwAutoCorrDoc::GetLanguage( sal_Int32 nPos ) const
{
    LanguageType eRet = LANGUAGE_SYSTEM;

    SwTextNode* pNd = m_rCursor.GetPoint()->nNode.GetNode().GetTextNode();

    if( pNd )
    {
        SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
                    pNd->getLayoutFrame(m_rEditSh.GetLayout())));
        assert(pFrame);
        eRet = pFrame->GetLangOfChar(TextFrameIndex(nPos), 0, true);
    }
    if(LANGUAGE_SYSTEM == eRet)
        eRet = GetAppLanguage();
    return eRet;
}

void SwAutoCorrExceptWord::CheckChar( const SwPosition& rPos, sal_Unicode cChr )
{
    // test only if this is an improvement.
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
        m_pDontExpandItems.reset( new SfxItemSet( const_cast<SwDoc&>(pTextNd->GetDoc()).GetAttrPool(),
                                            aCharFormatSetRange ) );
        const sal_Int32 n = rPos.nContent.GetIndex();
        if (!pTextNd->GetParaAttr( *m_pDontExpandItems, n, n,
                                n != pTextNd->GetText().getLength() ))
        {
            m_pDontExpandItems.reset();
        }
    }
}

void SwDontExpandItem::RestoreDontExpandItems( const SwPosition& rPos )
{
    SwTextNode* pTextNd = rPos.nNode.GetNode().GetTextNode();
    if( !pTextNd )
        return;

    const sal_Int32 nStart = rPos.nContent.GetIndex();
    if( nStart == pTextNd->GetText().getLength() )
        pTextNd->FormatToTextAttr( pTextNd );

    if( !(pTextNd->GetpSwpHints() && pTextNd->GetpSwpHints()->Count()) )
        return;

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
            if( !m_pDontExpandItems || SfxItemState::SET != m_pDontExpandItems->
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
