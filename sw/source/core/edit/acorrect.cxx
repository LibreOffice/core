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
#include <ndtxt.hxx>
#include <acorrect.hxx>
#include <shellio.hxx>
#include <swundo.hxx>
#include <viscrs.hxx>

#include <editeng/acorrcfg.hxx>

using namespace ::com::sun::star;

class _PaMIntoCrsrShellRing
{
    SwCrsrShell& rSh;
    SwPaM &rDelPam, &rCrsr;
    SwPaM* pPrevDelPam;
    SwPaM* pPrevCrsr;

    static void RemoveFromRing( SwPaM& rPam, SwPaM* pPrev );
public:
    _PaMIntoCrsrShellRing( SwCrsrShell& rSh, SwPaM& rCrsr, SwPaM& rPam );
    ~_PaMIntoCrsrShellRing();
};

_PaMIntoCrsrShellRing::_PaMIntoCrsrShellRing( SwCrsrShell& rCSh,
                                            SwPaM& rShCrsr, SwPaM& rPam )
    : rSh( rCSh ), rDelPam( rPam ), rCrsr( rShCrsr )
{
    SwPaM* pShCrsr = rSh._GetCrsr();

    pPrevDelPam = rDelPam.GetPrev();
    pPrevCrsr = rCrsr.GetPrev();

    rDelPam.GetRingContainer().merge( pShCrsr->GetRingContainer() );
    rCrsr.GetRingContainer().merge( pShCrsr->GetRingContainer() );
}

_PaMIntoCrsrShellRing::~_PaMIntoCrsrShellRing()
{
    // and take out the Pam again:
    RemoveFromRing( rDelPam, pPrevDelPam );
    RemoveFromRing( rCrsr, pPrevCrsr );
}

void _PaMIntoCrsrShellRing::RemoveFromRing( SwPaM& rPam, SwPaM* pPrev )
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
    : rEditSh( rEditShell ), rCrsr( rPam ), pIdx( 0 )
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
    delete pIdx;
}

void SwAutoCorrDoc::DeleteSel( SwPaM& rDelPam )
{
    SwDoc* pDoc = rEditSh.GetDoc();
    if( pDoc->IsAutoFmtRedline() )
    {
        // so that also the DelPam be moved,  include it in the
        // Shell-Cursr-Ring !!
        _PaMIntoCrsrShellRing aTmp( rEditSh, rCrsr, rDelPam );
        pDoc->getIDocumentContentOperations().DeleteAndJoin( rDelPam );
    }
    else
    {
        pDoc->getIDocumentContentOperations().DeleteRange( rDelPam );
    }
}

bool SwAutoCorrDoc::Delete( sal_Int32 nStt, sal_Int32 nEnd )
{
    const SwNodeIndex& rNd = rCrsr.GetPoint()->nNode;
    SwPaM aSel( rNd, nStt, rNd, nEnd );
    DeleteSel( aSel );

    if( bUndoIdInitialized )
        bUndoIdInitialized = true;
    return true;
}

bool SwAutoCorrDoc::Insert( sal_Int32 nPos, const OUString& rTxt )
{
    SwPaM aPam( rCrsr.GetPoint()->nNode.GetNode(), nPos );
    rEditSh.GetDoc()->getIDocumentContentOperations().InsertString( aPam, rTxt );
    if( !bUndoIdInitialized )
    {
        bUndoIdInitialized = true;
        if( 1 == rTxt.getLength() )
        {
            rEditSh.StartUndo( UNDO_AUTOCORRECT );
            ++m_nEndUndoCounter;
        }
    }
    return true;
}

bool SwAutoCorrDoc::Replace( sal_Int32 nPos, const OUString& rTxt )
{
    return ReplaceRange( nPos, rTxt.getLength(), rTxt );
}

bool SwAutoCorrDoc::ReplaceRange( sal_Int32 nPos, sal_Int32 nSourceLength, const OUString& rTxt )
{
    SwPaM* pPam = &rCrsr;
    if( pPam->GetPoint()->nContent.GetIndex() != nPos )
    {
        pPam = new SwPaM( *rCrsr.GetPoint() );
        pPam->GetPoint()->nContent = nPos;
    }

    SwTxtNode * const pNd = pPam->GetNode().GetTxtNode();
    if ( !pNd )
    {
        return false;
    }

    // text attributes with dummy characters must not be replaced!
    bool bDoReplace = true;
    sal_Int32 const nLen = rTxt.getLength();
    for ( sal_Int32 n = 0; n < nLen; ++n )
    {
        sal_Unicode const Char = pNd->GetTxt()[n + nPos];
        if ( ( CH_TXTATR_BREAKWORD == Char || CH_TXTATR_INWORD == Char )
             && pNd->GetTxtAttrForCharAt( n + nPos ) )
        {
            bDoReplace = false;
            break;
        }
    }

    if ( bDoReplace )
    {
        SwDoc* pDoc = rEditSh.GetDoc();

        if( pDoc->IsAutoFmtRedline() )
        {
            if (nPos == pNd->GetTxt().getLength()) // at the End do an Insert
            {
                pDoc->getIDocumentContentOperations().InsertString( *pPam, rTxt );
            }
            else
            {
                _PaMIntoCrsrShellRing aTmp( rEditSh, rCrsr, *pPam );

                pPam->SetMark();
                pPam->GetPoint()->nContent = std::min<sal_Int32>(
                        pNd->GetTxt().getLength(), nPos + nSourceLength);
                pDoc->getIDocumentContentOperations().ReplaceRange( *pPam, rTxt, false );
                pPam->Exchange();
                pPam->DeleteMark();
            }
        }
        else
        {
            if( nSourceLength != rTxt.getLength() )
            {
                pPam->SetMark();
                pPam->GetPoint()->nContent = std::min<sal_Int32>(
                        pNd->GetTxt().getLength(), nPos + nSourceLength);
                pDoc->getIDocumentContentOperations().ReplaceRange( *pPam, rTxt, false );
                pPam->Exchange();
                pPam->DeleteMark();
            }
            else
                pDoc->getIDocumentContentOperations().Overwrite( *pPam, rTxt );
        }

        if( bUndoIdInitialized )
        {
            bUndoIdInitialized = true;
            if( 1 == rTxt.getLength() )
            {
                rEditSh.StartUndo( UNDO_AUTOCORRECT );
                ++m_nEndUndoCounter;
            }
        }
    }

    if( pPam != &rCrsr )
        delete pPam;

    return true;
}

bool SwAutoCorrDoc::SetAttr( sal_Int32 nStt, sal_Int32 nEnd, sal_uInt16 nSlotId,
                                        SfxPoolItem& rItem )
{
    const SwNodeIndex& rNd = rCrsr.GetPoint()->nNode;
    SwPaM aPam( rNd, nStt, rNd, nEnd );

    SfxItemPool& rPool = rEditSh.GetDoc()->GetAttrPool();
    sal_uInt16 nWhich = rPool.GetWhich( nSlotId, false );
    if( nWhich )
    {
        rItem.SetWhich( nWhich );

        SfxItemSet aSet( rPool, aCharFmtSetRange );
        SetAllScriptItem( aSet, rItem );

        rEditSh.GetDoc()->SetFmtItemByAutoFmt( aPam, aSet );

        if( bUndoIdInitialized )
            bUndoIdInitialized = true;
    }
    return 0 != nWhich;
}

bool SwAutoCorrDoc::SetINetAttr( sal_Int32 nStt, sal_Int32 nEnd, const OUString& rURL )
{
    const SwNodeIndex& rNd = rCrsr.GetPoint()->nNode;
    SwPaM aPam( rNd, nStt, rNd, nEnd );

    SfxItemSet aSet( rEditSh.GetDoc()->GetAttrPool(),
                        RES_TXTATR_INETFMT, RES_TXTATR_INETFMT );
    aSet.Put( SwFmtINetFmt( rURL, OUString() ));
    rEditSh.GetDoc()->SetFmtItemByAutoFmt( aPam, aSet );
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
    OUString const* pStr(0);

    if( bAtNormalPos || !pIdx )
        pIdx = new SwNodeIndex( rCrsr.GetPoint()->nNode, -1 );
    else
        --(*pIdx);

    SwTxtNode* pTNd = pIdx->GetNode().GetTxtNode();
    while (pTNd && !pTNd->GetTxt().getLength())
    {
        --(*pIdx);
        pTNd = pIdx->GetNode().GetTxtNode();
    }
    if( pTNd && 0 == pTNd->GetAttrOutlineLevel() )
        pStr = & pTNd->GetTxt();

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
    SwTxtNode* pTxtNd = rCrsr.GetNode().GetTxtNode();
    OSL_ENSURE( pTxtNd, "where is the TextNode?" );

    bool bRet = false;
    if( nEndPos == rSttPos )
        return bRet;

    LanguageType eLang = GetLanguage(nEndPos, false);
    if(LANGUAGE_SYSTEM == eLang)
        eLang = GetAppLanguage();
    LanguageTag aLanguageTag( eLang);

    //JP 22.04.99: Bug 63883 - Special treatment for dots.
    bool bLastCharIsPoint = nEndPos < pTxtNd->GetTxt().getLength() &&
                            ('.' == pTxtNd->GetTxt()[nEndPos]);

    const SvxAutocorrWord* pFnd = rACorrect.SearchWordsInList(
                                pTxtNd->GetTxt(), rSttPos, nEndPos, *this, aLanguageTag );
    SwDoc* pDoc = rEditSh.GetDoc();
    if( pFnd )
    {
        const SwNodeIndex& rNd = rCrsr.GetPoint()->nNode;
        SwPaM aPam( rNd, rSttPos, rNd, nEndPos );

        if( pFnd->IsTextOnly() )
        {
            //JP 22.04.99: Bug 63883 - Special treatment for dots.
            if( !bLastCharIsPoint || pFnd->GetLong().isEmpty() ||
                '.' != pFnd->GetLong()[ pFnd->GetLong().getLength() - 1 ] )
            {
                // replace the selection
                pDoc->getIDocumentContentOperations().ReplaceRange( aPam, pFnd->GetLong(), false);
                bRet = true;
            }
        }
        else
        {
            SwTextBlocks aTBlks( rACorrect.GetAutoCorrFileName( aLanguageTag, false, true, false ));
            sal_uInt16 nPos = aTBlks.GetIndex( pFnd->GetShort() );
            if( USHRT_MAX != nPos && aTBlks.BeginGetDoc( nPos ) )
            {
                DeleteSel( aPam );
                pDoc->DontExpandFmt( *aPam.GetPoint() );

                if( pPara )
                {
                    OSL_ENSURE( !pIdx, "who has not deleted his Index?" );
                    pIdx = new SwNodeIndex( rCrsr.GetPoint()->nNode, -1 );
                }

                SwDoc* pAutoDoc = aTBlks.GetDoc();
                SwNodeIndex aSttIdx( pAutoDoc->GetNodes().GetEndOfExtras(), 1 );
                SwCntntNode* pCntntNd = pAutoDoc->GetNodes().GoNext( &aSttIdx );
                SwPaM aCpyPam( aSttIdx );

                const SwTableNode* pTblNd = pCntntNd->FindTableNode();
                if( pTblNd )
                {
                    aCpyPam.GetPoint()->nContent.Assign( 0, 0 );
                    aCpyPam.GetPoint()->nNode = *pTblNd;
                }
                aCpyPam.SetMark();

                // then until the end of the Nodes Array
                aCpyPam.GetPoint()->nNode.Assign( pAutoDoc->GetNodes().GetEndOfContent(), -1 );
                pCntntNd = aCpyPam.GetCntntNode();
                aCpyPam.GetPoint()->nContent.Assign(
                       pCntntNd, (pCntntNd) ? pCntntNd->Len() : 0);

                SwDontExpandItem aExpItem;
                aExpItem.SaveDontExpandItems( *aPam.GetPoint() );

                pAutoDoc->getIDocumentContentOperations().CopyRange( aCpyPam, *aPam.GetPoint(), /*bCopyAll=*/false, /*bCheckPos=*/true );

                aExpItem.RestoreDontExpandItems( *aPam.GetPoint() );

                if( pPara )
                {
                    ++(*pIdx);
                    pTxtNd = pIdx->GetNode().GetTxtNode();
                }
                bRet = true;
            }
            aTBlks.EndGetDoc();
        }
    }

    if( bRet && pPara && pTxtNd )
        *pPara = pTxtNd->GetTxt();

    return bRet;
}

// Called by the functions:
//  - FnCptlSttWrd
//  - FnCptlSttSntnc
// after the exchange of characters. Then the words, if necessary, can be inserted
// into the exception list.
void SwAutoCorrDoc::SaveCpltSttWord( sal_uLong nFlag, sal_Int32 nPos,
                                            const OUString& rExceptWord,
                                            sal_Unicode cChar )
{
    sal_uLong nNode = pIdx ? pIdx->GetIndex() : rCrsr.GetPoint()->nNode.GetIndex();
    LanguageType eLang = GetLanguage(nPos, false);
    rEditSh.GetDoc()->SetAutoCorrExceptWord( new SwAutoCorrExceptWord( nFlag,
                                        nNode, nPos, rExceptWord, cChar, eLang ));
}

LanguageType SwAutoCorrDoc::GetLanguage( sal_Int32 nPos, bool bPrevPara ) const
{
    LanguageType eRet = LANGUAGE_SYSTEM;

    SwTxtNode* pNd = (( bPrevPara && pIdx )
                            ? *pIdx
                            : rCrsr.GetPoint()->nNode ).GetNode().GetTxtNode();

    if( pNd )
        eRet = pNd->GetLang( nPos, 0 );
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
        if (CptlSttWrd & m_nFlags)
            pACorr->AddWrtSttException(m_sWord, m_eLanguage);
        else if (CptlSttSntnc & m_nFlags)
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
    delete pDontExpItems;
}

void SwDontExpandItem::SaveDontExpandItems( const SwPosition& rPos )
{
    const SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( pTxtNd )
    {
        pDontExpItems = new SfxItemSet( const_cast<SwDoc*>(pTxtNd->GetDoc())->GetAttrPool(),
                                            aCharFmtSetRange );
        const sal_Int32 n = rPos.nContent.GetIndex();
        if( !pTxtNd->GetAttr( *pDontExpItems, n, n,
                                n != pTxtNd->GetTxt().getLength() ))
            delete pDontExpItems, pDontExpItems = 0;
    }
}

void SwDontExpandItem::RestoreDontExpandItems( const SwPosition& rPos )
{
    SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
    if( pTxtNd )
    {
        const sal_Int32 nStart = rPos.nContent.GetIndex();
        if( nStart == pTxtNd->GetTxt().getLength() )
            pTxtNd->FmtToTxtAttr( pTxtNd );

        if( pTxtNd->GetpSwpHints() && pTxtNd->GetpSwpHints()->Count() )
        {
            const size_t nSize = pTxtNd->GetpSwpHints()->Count();
            sal_Int32 nAttrStart;

            for( size_t n = 0; n < nSize; ++n )
            {
                SwTxtAttr* pHt = pTxtNd->GetpSwpHints()->GetTextHint( n );
                nAttrStart = pHt->GetStart();
                if( nAttrStart > nStart )       // beyond the area
                    break;

                const sal_Int32* pAttrEnd;
                if( 0 != ( pAttrEnd = pHt->End() ) &&
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
