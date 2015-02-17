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

#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <txatritr.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <txtatr.hxx>
#include <txtfld.hxx>
#include <swcrsr.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <pamtyp.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>
#include <UndoInsert.hxx>
#include <breakit.hxx>
#include <docsh.hxx>
#include <PostItMgr.hxx>
#include <viewsh.hxx>

using namespace ::com::sun::star;
using namespace util;

OUString *ReplaceBackReferences( const SearchOptions& rSearchOpt, SwPaM* pPam );

static OUString
lcl_CleanStr(const SwTxtNode& rNd, sal_Int32 const nStart, sal_Int32& rEnd,
             std::vector<sal_Int32> &rArr, bool const bRemoveSoftHyphen)
{
    OUStringBuffer buf(rNd.GetTxt());
    rArr.clear();

    const SwpHints *pHts = rNd.GetpSwpHints();

    size_t n = 0;
    sal_Int32 nSoftHyphen = nStart;
    sal_Int32 nHintStart = -1;
    bool bNewHint       = true;
    bool bNewSoftHyphen = true;
    const sal_Int32 nEnd = rEnd;
    std::vector<sal_Int32> aReplaced;

    do
    {
        if ( bNewHint )
            nHintStart = pHts && n < pHts->Count() ?
                         (*pHts)[n]->GetStart() :
                         -1;

        if ( bNewSoftHyphen )
        {
            nSoftHyphen = (bRemoveSoftHyphen)
                    ?  rNd.GetTxt().indexOf(CHAR_SOFTHYPHEN, nSoftHyphen)
                    : -1;
        }

        bNewHint       = false;
        bNewSoftHyphen = false;
        sal_Int32 nStt = 0;

        // Check if next stop is a hint.
        if ( nHintStart>=0
            && (-1 == nSoftHyphen || nHintStart < nSoftHyphen)
            && nHintStart < nEnd )
        {
            nStt = nHintStart;
            bNewHint = true;
        }
        // Check if next stop is a soft hyphen.
        else if (   -1 != nSoftHyphen
                 && (-1 == nHintStart || nSoftHyphen < nHintStart)
                 && nSoftHyphen < nEnd)
        {
            nStt = nSoftHyphen;
            bNewSoftHyphen = true;
        }
        // If nSoftHyphen == nHintStart, the current hint *must* be a hint with an end.
        else if (-1 != nSoftHyphen && nSoftHyphen == nHintStart)
        {
            nStt = nSoftHyphen;
            bNewHint = true;
            bNewSoftHyphen = true;
        }
        else
            break;

        const sal_Int32 nAkt = nStt - rArr.size();

        if ( bNewHint )
        {
            const SwTxtAttr* pHt = (*pHts)[n];
            if ( pHt->HasDummyChar() && (nStt >= nStart) )
            {
                switch( pHt->Which() )
                {
                case RES_TXTATR_FLYCNT:
                case RES_TXTATR_FTN:
                case RES_TXTATR_FIELD:
                case RES_TXTATR_ANNOTATION:
                case RES_TXTATR_REFMARK:
                case RES_TXTATR_TOXMARK:
                case RES_TXTATR_META:
                case RES_TXTATR_METAFIELD:
                    {
                        // (1998) they are desired as separators and
                        // belong not any longer to a word.
                        // they should also be ignored at a
                        // beginning/end of a sentence if blank. Those are
                        // simply removed if first. If at the end, we keep the
                        // replacement and remove afterwards all at a string's
                        // end (might be normal 0x7f).
                        const bool bEmpty =
                            ( pHt->Which() != RES_TXTATR_FIELD
                              && pHt->Which() != RES_TXTATR_ANNOTATION )
                            || (static_txtattr_cast<SwTxtFld const*>(pHt)->GetFmtFld().GetField()->ExpandField(true).isEmpty());;
                        if ( bEmpty && nStart == nAkt )
                        {
                            rArr.push_back( nAkt );
                            --rEnd;
                            buf.remove(nAkt, 1);
                        }
                        else
                        {
                            if ( bEmpty )
                                aReplaced.push_back( nAkt );
                            buf[nAkt] = '\x7f';
                           }
                    }
                    break;
                    default:
                        OSL_FAIL( "unknown case in lcl_CleanStr" );
                        break;
                }
            }
            ++n;
        }

        if ( bNewSoftHyphen )
        {
            rArr.push_back( nAkt );
            --rEnd;
            buf.remove(nAkt, 1);
            ++nSoftHyphen;
        }
    }
    while ( true );

    for( sal_uInt16 i = aReplaced.size(); i; )
    {
        const sal_Int32 nTmp = aReplaced[ --i ];
        if (nTmp == buf.getLength() - 1)
        {
            buf.truncate(nTmp);
            rArr.push_back( nTmp );
            --rEnd;
        }
    }

    return buf.makeStringAndClear();
}

// skip all non SwPostIts inside the array
size_t GetPostIt(sal_Int32 aCount,const SwpHints *pHts)
{
    size_t aIndex = 0;
    while (aCount)
    {
        for (size_t i = 0; i < pHts->Count(); ++i )
        {
            aIndex++;
            const SwTxtAttr* pTxtAttr = (*pHts)[i];
            if ( pTxtAttr->Which() == RES_TXTATR_ANNOTATION )
            {
                aCount--;
                if (!aCount)
                    break;
            }
        }
    }
    // throw away all following non postits
    for( size_t i = aIndex; i < pHts->Count(); ++i )
    {
        const SwTxtAttr* pTxtAttr = (*pHts)[i];
        if ( pTxtAttr->Which() == RES_TXTATR_ANNOTATION )
            break;
        else
            aIndex++;
    }
    return aIndex;
}

bool SwPaM::Find( const SearchOptions& rSearchOpt, bool bSearchInNotes , utl::TextSearch& rSTxt,
                  SwMoveFn fnMove, const SwPaM * pRegion,
                  bool bInReadOnly )
{
    if( rSearchOpt.searchString.isEmpty() )
        return false;

    SwPaM* pPam = MakeRegion( fnMove, pRegion );
    const bool bSrchForward = fnMove == fnMoveForward;
    SwNodeIndex& rNdIdx = pPam->GetPoint()->nNode;
    SwIndex& rCntntIdx = pPam->GetPoint()->nContent;

    // If bFound is true then the string was found and is between nStart and nEnd
    bool bFound = false;
    // start position in text or initial position
    bool bFirst = true;
    SwCntntNode * pNode;

    const bool bRegSearch = SearchAlgorithms_REGEXP == rSearchOpt.algorithmType;
    const bool bChkEmptyPara = bRegSearch && 2 == rSearchOpt.searchString.getLength() &&
                        ( rSearchOpt.searchString.equalsAscii( "^$" ) ||
                          rSearchOpt.searchString.equalsAscii( "$^" ) );
    const bool bChkParaEnd = bRegSearch && rSearchOpt.searchString.equalsAscii( "$" );

    // LanguageType eLastLang = 0;
    while( 0 != ( pNode = ::GetNode( *pPam, bFirst, fnMove, bInReadOnly ) ))
    {
        if( pNode->IsTxtNode() )
        {
            sal_Int32 nTxtLen = pNode->GetTxtNode()->GetTxt().getLength();
            sal_Int32 nEnd;
            if( rNdIdx == pPam->GetMark()->nNode )
                nEnd = pPam->GetMark()->nContent.GetIndex();
            else
                nEnd = bSrchForward ? nTxtLen : 0;
            sal_Int32 nStart = rCntntIdx.GetIndex();

            /* #i80135# */
            // if there are SwPostItFields inside our current node text, we
            // split the text into separate pieces and search for text inside
            // the pieces as well as inside the fields
            const SwpHints *pHts = pNode->GetTxtNode()->GetpSwpHints();

            // count PostItFields by looping over all fields
            sal_Int32 aNumberPostits = 0;
            sal_Int32 aIgnore = 0;
            if (pHts && bSearchInNotes)
            {
                if (!bSrchForward)
                {
                    std::swap(nStart, nEnd);
                }

                for( size_t i = 0; i < pHts->Count(); ++i )
                {
                    const SwTxtAttr* pTxtAttr = (*pHts)[i];
                    if ( pTxtAttr->Which()==RES_TXTATR_ANNOTATION )
                    {
                        const sal_Int32 aPos = pTxtAttr->GetStart();
                        if ( (aPos >= nStart) && (aPos <= nEnd) )
                            aNumberPostits++;
                        else
                        {
                            if (bSrchForward)
                                aIgnore++;
                        }
                    }
                }

                if (!bSrchForward)
                {
                    std::swap(nStart, nEnd);
                }

            }

            SwDocShell *const pDocShell = pNode->GetDoc()->GetDocShell();
            SwViewShell *const pWrtShell = (pDocShell) ? (SwViewShell*)(pDocShell->GetWrtShell()) : 0;
            SwPostItMgr *const pPostItMgr = (pWrtShell) ? pWrtShell->GetPostItMgr() : 0;

            sal_Int32 aStart = 0;
            // do we need to finish a note?
            if (pPostItMgr && pPostItMgr->HasActiveSidebarWin())
            {
                if (bSearchInNotes)
                {
                    if (bSrchForward)
                        aStart++;
                    else
                    {
                        if (aNumberPostits)
                            --aNumberPostits;
                    }
                    //search inside and finsih and put focus back into the doc
                    if (pPostItMgr->FinishSearchReplace(rSearchOpt,bSrchForward))
                    {
                        bFound = true ;
                        break;
                    }
                }
                else
                {
                    pPostItMgr->SetActiveSidebarWin(0);
                }
            }

            if (aNumberPostits)
            {
                // now we have to split
                sal_Int32 nStartInside = 0;
                sal_Int32 nEndInside = 0;
                sal_Int16 aLoop= bSrchForward ? aStart : aNumberPostits;

                while ( (aLoop>=0) && (aLoop<=aNumberPostits))
                {
                    if (bSrchForward)
                    {
                        nStartInside = aLoop==0 ? nStart : (*pHts)[GetPostIt(aLoop+aIgnore-1,pHts)]->GetStart()+1;
                        nEndInside = aLoop==aNumberPostits ? nEnd : (*pHts)[GetPostIt(aLoop+aIgnore,pHts)]->GetStart();
                        nTxtLen = nEndInside - nStartInside;
                    }
                    else
                    {
                        nStartInside =  aLoop==aNumberPostits ? nStart : (*pHts)[GetPostIt(aLoop+aIgnore,pHts)]->GetStart();
                        nEndInside = aLoop==0 ? nEnd : (*pHts)[GetPostIt(aLoop+aIgnore-1,pHts)]->GetStart()+1;
                        nTxtLen = nStartInside - nEndInside;
                    }
                    // search inside the text between a note
                    bFound = DoSearch( rSearchOpt, rSTxt, fnMove, bSrchForward,
                                       bRegSearch, bChkEmptyPara, bChkParaEnd,
                                       nStartInside, nEndInside, nTxtLen, pNode,
                                       pPam );
                    if ( bFound )
                        break;
                    else
                    {
                        // we should now be right in front of a note, search inside
                        if ( (bSrchForward && (GetPostIt(aLoop + aIgnore,pHts) < pHts->Count()) ) || ( !bSrchForward && (aLoop!=0) ))
                        {
                            const SwTxtAttr* pTxtAttr = bSrchForward ?  (*pHts)[GetPostIt(aLoop+aIgnore,pHts)] : (*pHts)[GetPostIt(aLoop+aIgnore-1,pHts)];
                            if (pPostItMgr && pPostItMgr->SearchReplace(
                                    static_txtattr_cast<SwTxtFld const*>(pTxtAttr)->GetFmtFld(),rSearchOpt,bSrchForward))
                            {
                                bFound = true ;
                                break;
                            }
                        }
                    }
                    aLoop = bSrchForward ? aLoop+1 : aLoop-1;
                }
            }
            else
            {
                // if there is no SwPostItField inside or searching inside notes
                // is disabled, we search the whole length just like before
                bFound = DoSearch( rSearchOpt, rSTxt, fnMove, bSrchForward,
                                   bRegSearch, bChkEmptyPara, bChkParaEnd,
                                   nStart, nEnd, nTxtLen, pNode, pPam );
            }
            if (bFound)
                break;
        }
    }
    delete pPam;
    return bFound;
}

bool SwPaM::DoSearch( const SearchOptions& rSearchOpt, utl::TextSearch& rSTxt,
                      SwMoveFn fnMove, bool bSrchForward, bool bRegSearch,
                      bool bChkEmptyPara, bool bChkParaEnd,
                      sal_Int32 &nStart, sal_Int32 &nEnd, sal_Int32 nTxtLen,
                      SwNode* pNode, SwPaM* pPam)
{
    bool bFound = false;
    SwNodeIndex& rNdIdx = pPam->GetPoint()->nNode;
    const SwNode* pSttNd = &rNdIdx.GetNode();
    OUString sCleanStr;
    std::vector<sal_Int32> aFltArr;
    LanguageType eLastLang = 0;
    // if the search string contains a soft hypen,
    // we don't strip them from the text:
    bool bRemoveSoftHyphens = true;

    if ( bRegSearch )
    {
        if (   -1 != rSearchOpt.searchString.indexOf("\\xAD")
            || -1 != rSearchOpt.searchString.indexOf("\\x{00AD}")
            || -1 != rSearchOpt.searchString.indexOf("\\u00AD")
            || -1 != rSearchOpt.searchString.indexOf("\\U000000AD")
            || -1 != rSearchOpt.searchString.indexOf("\\N{SOFT HYPHEN}"))
        {
             bRemoveSoftHyphens = false;
        }
    }
    else
    {
        if ( 1 == rSearchOpt.searchString.getLength() &&
             CHAR_SOFTHYPHEN == rSearchOpt.searchString.toChar() )
             bRemoveSoftHyphens = false;
    }

    if( bSrchForward )
        sCleanStr = lcl_CleanStr(*pNode->GetTxtNode(), nStart, nEnd,
                        aFltArr, bRemoveSoftHyphens);
    else
        sCleanStr = lcl_CleanStr(*pNode->GetTxtNode(), nEnd, nStart,
                        aFltArr, bRemoveSoftHyphens);

    SwScriptIterator* pScriptIter = 0;
    sal_uInt16 nSearchScript = 0;
    sal_uInt16 nCurrScript = 0;

    if ( SearchAlgorithms_APPROXIMATE == rSearchOpt.algorithmType &&
         g_pBreakIt->GetBreakIter().is() )
    {
        pScriptIter = new SwScriptIterator( sCleanStr, nStart, bSrchForward );
        nSearchScript = g_pBreakIt->GetRealScriptOfText( rSearchOpt.searchString, 0 );
    }

    const sal_Int32 nStringEnd = nEnd;
    bool bZeroMatch = false;    // zero-length match, i.e. only $ anchor as regex
    while ( ((bSrchForward && nStart < nStringEnd) ||
            (! bSrchForward && nStart > nStringEnd)) && !bZeroMatch )
    {
        // SearchAlgorithms_APPROXIMATE works on a per word base so we have to
        // provide the text searcher with the correct locale, because it uses
        // the break-iterator
        if ( pScriptIter )
        {
            nEnd = pScriptIter->GetScriptChgPos();
            nCurrScript = pScriptIter->GetCurrScript();
            if ( nSearchScript == nCurrScript )
            {
                const LanguageType eCurrLang =
                        pNode->GetTxtNode()->GetLang( bSrchForward ?
                                                      nStart :
                                                      nEnd );

                if ( eCurrLang != eLastLang )
                {
                    const lang::Locale aLocale(
                            g_pBreakIt->GetLocale( eCurrLang ) );
                    rSTxt.SetLocale( rSearchOpt, aLocale );
                    eLastLang = eCurrLang;
                }
            }
            pScriptIter->Next();
        }
        sal_Int32 nProxyStart = nStart;
        sal_Int32 nProxyEnd = nEnd;
        if( nSearchScript == nCurrScript &&
                (rSTxt.*fnMove->fnSearch)( sCleanStr, &nProxyStart, &nProxyEnd, 0 ) &&
                !(bZeroMatch = (nProxyStart == nProxyEnd)))
        {
            nStart = nProxyStart;
            nEnd = nProxyEnd;
            // set section correctly
            *GetPoint() = *pPam->GetPoint();
            SetMark();

            // adjust start and end
            if( !aFltArr.empty() )
            {
                // if backward search, switch positions temporarily
                if( !bSrchForward ) { std::swap(nStart, nEnd); }

                sal_Int32 nNew = nStart;
                for (size_t n = 0; n < aFltArr.size() && aFltArr[ n ] <= nStart; ++n )
                {
                    ++nNew;
                }

                nStart = nNew;
                nNew = nEnd;
                for( size_t n = 0; n < aFltArr.size() && aFltArr[ n ] < nEnd; ++n )
                {
                    ++nNew;
                }

                nEnd = nNew;
                // if backward search, switch positions temporarily
                if( !bSrchForward ) { std::swap(nStart, nEnd); }
            }
            GetMark()->nContent = nStart;
            GetPoint()->nContent = nEnd;

            // if backward search, switch point and mark
            if( !bSrchForward )
                Exchange();
            bFound = true;
            break;
        }
        else
        {
            nStart = nProxyStart;
            nEnd = nProxyEnd;
        }
        nStart = nEnd;
    }

    delete pScriptIter;

    if ( bFound )
        return true;
    else if( ( bChkEmptyPara && !nStart && !nTxtLen ) || bChkParaEnd)
    {
        *GetPoint() = *pPam->GetPoint();
        GetPoint()->nContent = bChkParaEnd ? nTxtLen : 0;
        SetMark();
        /* FIXME: this condition does not work for !bSrchForward backward
         * search, it probably never did. (pSttNd != &rNdIdx.GetNode())
         * is never true in this case. */
        if( (bSrchForward || pSttNd != &rNdIdx.GetNode()) &&
            Move( fnMoveForward, fnGoCntnt ) &&
            (!bSrchForward || pSttNd != &GetPoint()->nNode.GetNode()) &&
            1 == std::abs( (int)( GetPoint()->nNode.GetIndex() -
                             GetMark()->nNode.GetIndex()) ) )
        {
            // if backward search, switch point and mark
            if( !bSrchForward )
                Exchange();
            return true;
        }
    }
    return bFound;
}

/// parameters for search and replace in text
struct SwFindParaText : public SwFindParas
{
    const SearchOptions& rSearchOpt;
    SwCursor& rCursor;
    utl::TextSearch aSTxt;
    bool bReplace;
    bool bSearchInNotes;

    SwFindParaText( const SearchOptions& rOpt, bool bSearchNotes, bool bRepl, SwCursor& rCrsr )
        : rSearchOpt( rOpt ), rCursor( rCrsr ), aSTxt( rOpt ), bReplace( bRepl ), bSearchInNotes( bSearchNotes )
    {}
    virtual int Find( SwPaM* , SwMoveFn , const SwPaM*, bool bInReadOnly ) SAL_OVERRIDE;
    virtual bool IsReplaceMode() const SAL_OVERRIDE;
    virtual ~SwFindParaText();
};

SwFindParaText::~SwFindParaText()
{
}

int SwFindParaText::Find( SwPaM* pCrsr, SwMoveFn fnMove,
                          const SwPaM* pRegion, bool bInReadOnly )
{
    if( bInReadOnly && bReplace )
        bInReadOnly = false;

    const bool bFnd = pCrsr->Find( rSearchOpt, bSearchInNotes, aSTxt, fnMove, pRegion, bInReadOnly );

    if( bFnd && bReplace ) // replace string
    {
        // use replace method in SwDoc
        const bool bRegExp(SearchAlgorithms_REGEXP == rSearchOpt.algorithmType);
        SwIndex& rSttCntIdx = pCrsr->Start()->nContent;
        const sal_Int32 nSttCnt = rSttCntIdx.GetIndex();
        // add to shell-cursor-ring so that the regions will be moved enventually
        Ring *pPrev(0);
        if( bRegExp )
        {
            pPrev = pRegion->GetPrev();
            ((Ring*)pRegion)->MoveRingTo( &rCursor );
        }

        boost::scoped_ptr<OUString> pRepl( (bRegExp)
                ? ReplaceBackReferences( rSearchOpt, pCrsr ) : 0 );
        rCursor.GetDoc()->getIDocumentContentOperations().ReplaceRange( *pCrsr,
            (pRepl.get()) ? *pRepl : rSearchOpt.replaceString,
            bRegExp );
        rCursor.SaveTblBoxCntnt( pCrsr->GetPoint() );

        if( bRegExp )
        {
            // and remove region again
            Ring *p, *pNext = (Ring*)pRegion;
            do {
                p = pNext;
                pNext = p->GetNext();
                p->MoveTo( (Ring*)pRegion );
            } while( p != pPrev );
        }
        pCrsr->Start()->nContent = nSttCnt;
        return FIND_NO_RING;
    }
    return bFnd ? FIND_FOUND : FIND_NOT_FOUND;
}

bool SwFindParaText::IsReplaceMode() const
{
    return bReplace;
}

sal_uLong SwCursor::Find( const SearchOptions& rSearchOpt, bool bSearchInNotes,
                          SwDocPositions nStart, SwDocPositions nEnd,
                          bool& bCancel, FindRanges eFndRngs, bool bReplace )
{
    // switch off OLE-notifications
    SwDoc* pDoc = GetDoc();
    Link aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    bool const bStartUndo = pDoc->GetIDocumentUndoRedo().DoesUndo() && bReplace;
    if (bStartUndo)
    {
        pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_REPLACE, NULL );
    }

    bool bSearchSel = 0 != (rSearchOpt.searchFlag & SearchFlags::REG_NOT_BEGINOFLINE);
    if( bSearchSel )
        eFndRngs = (FindRanges)(eFndRngs | FND_IN_SEL);
    SwFindParaText aSwFindParaText( rSearchOpt, bSearchInNotes, bReplace, *this );
    sal_uLong nRet = FindAll( aSwFindParaText, nStart, nEnd, eFndRngs, bCancel );
    pDoc->SetOle2Link( aLnk );
    if( nRet && bReplace )
        pDoc->getIDocumentState().SetModified();

    if (bStartUndo)
    {
        SwRewriter rewriter(MakeUndoReplaceRewriter(
                nRet, rSearchOpt.searchString, rSearchOpt.replaceString));
        pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_REPLACE, & rewriter );
    }
    return nRet;
}

OUString *ReplaceBackReferences( const SearchOptions& rSearchOpt, SwPaM* pPam )
{
    OUString *pRet = 0;
    if( pPam && pPam->HasMark() &&
        SearchAlgorithms_REGEXP == rSearchOpt.algorithmType )
    {
        const SwCntntNode* pTxtNode = pPam->GetCntntNode( true );
        if( pTxtNode && pTxtNode->IsTxtNode() && pTxtNode == pPam->GetCntntNode( false ) )
        {
            utl::TextSearch aSTxt( rSearchOpt );
            const OUString& rStr = pTxtNode->GetTxtNode()->GetTxt();
            sal_Int32 nStart = pPam->Start()->nContent.GetIndex();
            sal_Int32 nEnd = pPam->End()->nContent.GetIndex();
            SearchResult aResult;
            if( aSTxt.SearchForward( rStr, &nStart, &nEnd, &aResult ) )
            {
                OUString aReplaceStr( rSearchOpt.replaceString );
                aSTxt.ReplaceBackReferences( aReplaceStr, rStr, aResult );
                pRet = new OUString( aReplaceStr );
            }
        }
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
