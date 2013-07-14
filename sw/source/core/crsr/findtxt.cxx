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

String *ReplaceBackReferences( const SearchOptions& rSearchOpt, SwPaM* pPam );

static String& lcl_CleanStr( const SwTxtNode& rNd, xub_StrLen nStart, xub_StrLen& rEnd,
                      std::vector<sal_uLong> &rArr, String& rRet,
                      bool bRemoveSoftHyphen )
{
    rRet = rNd.GetTxt();
    rArr.clear();

    const SwpHints *pHts = rNd.GetpSwpHints();

    sal_uInt16 n = 0;
    sal_Int32 nSoftHyphen = nStart;
    xub_StrLen nHintStart = STRING_LEN;
    bool bNewHint       = true;
    bool bNewSoftHyphen = true;
    const xub_StrLen nEnd = rEnd;
    std::vector<sal_uInt16> aReplaced;

    do
    {
        if ( bNewHint )
            nHintStart = pHts && n < pHts->Count() ?
                         *(*pHts)[n]->GetStart() :
                         STRING_LEN;

        if ( bNewSoftHyphen )
        {
            nSoftHyphen = (bRemoveSoftHyphen)
                    ?  rNd.GetTxt().indexOf(CHAR_SOFTHYPHEN, nSoftHyphen)
                    : -1;
        }

        bNewHint       = false;
        bNewSoftHyphen = false;

        xub_StrLen nStt = 0;

        // Check if next stop is a hint.
        if ( STRING_LEN != nHintStart
            && (-1 == nSoftHyphen || nHintStart < nSoftHyphen)
            && nHintStart < nEnd )
        {
            nStt = nHintStart;
            bNewHint = true;
        }
        // Check if next stop is a soft hyphen.
        else if (-1 != nSoftHyphen && nSoftHyphen < nHintStart && nSoftHyphen < nEnd)
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

        const xub_StrLen nAkt = nStt - rArr.size();

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
                        bool bEmpty = RES_TXTATR_FIELD != pHt->Which() ||
                            (static_cast<SwTxtFld const*>(pHt)
                                ->GetFld().GetFld()->ExpandField(true).isEmpty());
                        if ( bEmpty && nStart == nAkt )
                           {
                            rArr.push_back( nAkt );
                            --rEnd;
                            rRet.Erase( nAkt, 1 );
                           }
                        else
                           {
                            if ( bEmpty )
                                aReplaced.push_back( nAkt );
                            rRet.SetChar( nAkt, '\x7f' );
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
            rRet.Erase( nAkt, 1 );
            ++nSoftHyphen;
        }
    }
    while ( true );

    for( sal_uInt16 i = aReplaced.size(); i; )
    {
        const xub_StrLen nTmp = aReplaced[ --i ];
        if( nTmp == rRet.Len() - 1 )
        {
            rRet.Erase( nTmp );
            rArr.push_back( nTmp );
            --rEnd;
        }
    }

    return rRet;
}

// skip all non SwPostIts inside the array
xub_StrLen GetPostIt(xub_StrLen aCount,const SwpHints *pHts)
{
    xub_StrLen aIndex = 0;
    while (aCount)
    {
        for (xub_StrLen i = 0; i <pHts->Count();i++)
        {
            aIndex++;
            const SwTxtAttr* pTxtAttr = (*pHts)[i];
            if ( (pTxtAttr->Which()==RES_TXTATR_FIELD) &&
                    (pTxtAttr->GetFld().GetFld()->Which()==RES_POSTITFLD))
            {
                aCount--;
                if (!aCount)
                    break;
            }
        }
    }
    // throw away all following non postits
    for (xub_StrLen i = aIndex; i <pHts->Count();i++)
    {
        const SwTxtAttr* pTxtAttr = (*pHts)[i];
        if ( (pTxtAttr->Which()==RES_TXTATR_FIELD) &&
                (pTxtAttr->GetFld().GetFld()->Which()==RES_POSTITFLD))
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

    // If a beginning/end, from out of node; stop if empty node
    if( bSrchForward
        ? ( rCntntIdx.GetIndex() == pPam->GetCntntNode()->Len() &&
            rCntntIdx.GetIndex() )
        : !rCntntIdx.GetIndex() && pPam->GetCntntNode()->Len() )
    {
        if( !(*fnMove->fnNds)( &rNdIdx, sal_False ))
        {
            delete pPam;
            return false;
        }
        SwCntntNode *pNd = rNdIdx.GetNode().GetCntntNode();
        xub_StrLen nTmpPos = bSrchForward ? 0 : pNd->Len();
        rCntntIdx.Assign( pNd, nTmpPos );
    }

    // If bFound is true then the string was found and is between nStart and nEnd
    bool bFound = false;
    // start position in text or initial position
    sal_Bool bFirst = sal_True;
    SwCntntNode * pNode;

    xub_StrLen nStart, nEnd, nTxtLen;

    const bool bRegSearch = SearchAlgorithms_REGEXP == rSearchOpt.algorithmType;
    const bool bChkEmptyPara = bRegSearch && 2 == rSearchOpt.searchString.getLength() &&
                        ( !rSearchOpt.searchString.compareToAscii( "^$" ) ||
                          !rSearchOpt.searchString.compareToAscii( "$^" ) );
    const bool bChkParaEnd = bRegSearch && 1 == rSearchOpt.searchString.getLength() &&
                      !rSearchOpt.searchString.compareToAscii( "$" );

    // LanguageType eLastLang = 0;
    while( 0 != ( pNode = ::GetNode( *pPam, bFirst, fnMove, bInReadOnly ) ))
    {
        if( pNode->IsTxtNode() )
        {
            nTxtLen = static_cast<SwTxtNode*>(pNode)->GetTxt().getLength();
            if( rNdIdx == pPam->GetMark()->nNode )
                nEnd = pPam->GetMark()->nContent.GetIndex();
            else
                nEnd = bSrchForward ? nTxtLen : 0;
            nStart = rCntntIdx.GetIndex();

            /* #i80135# */
            // if there are SwPostItFields inside our current node text, we
            // split the text into separate pieces and search for text inside
            // the pieces as well as inside the fields
            const SwpHints *pHts = ((SwTxtNode*)pNode)->GetpSwpHints();

            // count PostItFields by looping over all fields
            xub_StrLen aNumberPostits = 0;
            xub_StrLen aIgnore = 0;
            if (pHts && bSearchInNotes)
            {
                if (!bSrchForward)
                {
                    xub_StrLen swap = nEnd;
                    nEnd = nStart;
                    nStart = swap;
                }

                for (xub_StrLen i = 0; i <pHts->Count();i++)
                {
                    xub_StrLen aPos = *(*pHts)[i]->GetStart();
                    const SwTxtAttr* pTxtAttr = (*pHts)[i];
                    if ( (pTxtAttr->Which()==RES_TXTATR_FIELD) &&
                                (pTxtAttr->GetFld().GetFld()->Which()==RES_POSTITFLD))
                    {
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
                    xub_StrLen swap = nEnd;
                    nEnd = nStart;
                    nStart = swap;
                }

            }

            SwDocShell *const pDocShell = pNode->GetDoc()->GetDocShell();
            ViewShell *const pWrtShell = (pDocShell) ? (ViewShell*)(pDocShell->GetWrtShell()) : 0;
            SwPostItMgr *const pPostItMgr = (pWrtShell) ? pWrtShell->GetPostItMgr() : 0;

            xub_StrLen aStart = 0;
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
                xub_StrLen nStartInside = 0;
                xub_StrLen nEndInside = 0;
                sal_Int16 aLoop= bSrchForward ? aStart : aNumberPostits;

                while ( (aLoop>=0) && (aLoop<=aNumberPostits))
                {
                    if (bSrchForward)
                    {
                        nStartInside = aLoop==0 ? nStart : *(*pHts)[GetPostIt(aLoop+aIgnore-1,pHts)]->GetStart()+1;
                        nEndInside = aLoop==aNumberPostits ? nEnd : *(*pHts)[GetPostIt(aLoop+aIgnore,pHts)]->GetStart();
                        nTxtLen = nEndInside - nStartInside;
                    }
                    else
                    {
                        nStartInside =  aLoop==aNumberPostits ? nStart : *(*pHts)[GetPostIt(aLoop+aIgnore,pHts)]->GetStart();
                        nEndInside = aLoop==0 ? nEnd : *(*pHts)[GetPostIt(aLoop+aIgnore-1,pHts)]->GetStart()+1;
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
                            if ( pPostItMgr && pPostItMgr->SearchReplace(((SwTxtFld*)pTxtAttr)->GetFld(),rSearchOpt,bSrchForward) )
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
                      xub_StrLen &nStart, xub_StrLen &nEnd, xub_StrLen nTxtLen,
                      SwNode* pNode, SwPaM* pPam)
{
    bool bFound = false;
    SwNodeIndex& rNdIdx = pPam->GetPoint()->nNode;
    const SwNode* pSttNd = &rNdIdx.GetNode();
    String sCleanStr;
    std::vector<sal_uLong> aFltArr;
    LanguageType eLastLang = 0;
    // if the search string contains a soft hypen,
    // we don't strip them from the text:
    bool bRemoveSoftHyphens = true;
    if ( bRegSearch )
    {
        const OUString a00AD("\\x00AD");
        if ( -1 != rSearchOpt.searchString.indexOf( a00AD ) )
             bRemoveSoftHyphens = false;
    }
    else
    {
        if ( 1 == rSearchOpt.searchString.getLength() &&
             CHAR_SOFTHYPHEN == rSearchOpt.searchString.toChar() )
             bRemoveSoftHyphens = false;
    }

    if( bSrchForward )
        lcl_CleanStr( *(SwTxtNode*)pNode, nStart, nEnd,
                        aFltArr, sCleanStr, bRemoveSoftHyphens );
    else
        lcl_CleanStr( *(SwTxtNode*)pNode, nEnd, nStart,
                        aFltArr, sCleanStr, bRemoveSoftHyphens );

    SwScriptIterator* pScriptIter = 0;
    sal_uInt16 nSearchScript = 0;
    sal_uInt16 nCurrScript = 0;

    if ( SearchAlgorithms_APPROXIMATE == rSearchOpt.algorithmType &&
         g_pBreakIt->GetBreakIter().is() )
    {
        pScriptIter = new SwScriptIterator( sCleanStr, nStart, bSrchForward );
        nSearchScript = g_pBreakIt->GetRealScriptOfText( rSearchOpt.searchString, 0 );
    }

    xub_StrLen nStringEnd = nEnd;
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
                        ((SwTxtNode*)pNode)->GetLang( bSrchForward ?
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
            nStart = (sal_uInt16)nProxyStart;
            nEnd = (sal_uInt16)nProxyEnd;
            // set section correctly
            *GetPoint() = *pPam->GetPoint();
            SetMark();

            // adjust start and end
            if( !aFltArr.empty() )
            {
                xub_StrLen n, nNew;
                // if backward search, switch positions temporarily
                if( !bSrchForward ) { n = nStart; nStart = nEnd; nEnd = n; }

                for( n = 0, nNew = nStart;
                    n < aFltArr.size() && aFltArr[ n ] <= nStart;
                    ++n, ++nNew )
                    ;
                nStart = nNew;
                for( n = 0, nNew = nEnd;
                    n < aFltArr.size() && aFltArr[ n ] < nEnd;
                    ++n, ++nNew )
                    ;

                nEnd = nNew;
                // if backward search, switch positions temporarily
                if( !bSrchForward ) { n = nStart; nStart = nEnd; nEnd = n; }
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
            nStart = (sal_uInt16)nProxyStart;
            nEnd = (sal_uInt16)nProxyEnd;
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
    sal_Bool bReplace;
    sal_Bool bSearchInNotes;

    SwFindParaText( const SearchOptions& rOpt, sal_Bool bSearchNotes, int bRepl, SwCursor& rCrsr )
        : rSearchOpt( rOpt ), rCursor( rCrsr ), aSTxt( rOpt ), bReplace( 0 != bRepl ), bSearchInNotes( bSearchNotes )
    {}
    virtual int Find( SwPaM* , SwMoveFn , const SwPaM*, sal_Bool bInReadOnly );
    virtual int IsReplaceMode() const;
    virtual ~SwFindParaText();
};

SwFindParaText::~SwFindParaText()
{
}

int SwFindParaText::Find( SwPaM* pCrsr, SwMoveFn fnMove,
                            const SwPaM* pRegion, sal_Bool bInReadOnly )
{
    if( bInReadOnly && bReplace )
        bInReadOnly = sal_False;

    const bool bFnd = pCrsr->Find( rSearchOpt, bSearchInNotes, aSTxt, fnMove, pRegion, bInReadOnly );

    if( bFnd && bReplace ) // replace string
    {
        // use replace method in SwDoc
        const bool bRegExp(SearchAlgorithms_REGEXP == rSearchOpt.algorithmType);
        SwIndex& rSttCntIdx = pCrsr->Start()->nContent;
        xub_StrLen nSttCnt = rSttCntIdx.GetIndex();
        // add to shell-cursor-ring so that the regions will be moved enventually
        Ring *pPrev(0);
        if( bRegExp )
        {
            pPrev = pRegion->GetPrev();
            ((Ring*)pRegion)->MoveRingTo( &rCursor );
        }

        ::std::auto_ptr<String> pRepl( (bRegExp)
                ? ReplaceBackReferences( rSearchOpt, pCrsr ) : 0 );
        rCursor.GetDoc()->ReplaceRange( *pCrsr,
            (pRepl.get()) ? *pRepl : String(rSearchOpt.replaceString),
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


int SwFindParaText::IsReplaceMode() const
{
    return bReplace;
}


sal_uLong SwCursor::Find( const SearchOptions& rSearchOpt, sal_Bool bSearchInNotes,
                          SwDocPositions nStart, SwDocPositions nEnd,
                          sal_Bool& bCancel, FindRanges eFndRngs, int bReplace )
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
        pDoc->SetModified();

    if (bStartUndo)
    {
        SwRewriter rewriter(MakeUndoReplaceRewriter(
                nRet, rSearchOpt.searchString, rSearchOpt.replaceString));
        pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_REPLACE, & rewriter );
    }
    return nRet;
}

String *ReplaceBackReferences( const SearchOptions& rSearchOpt, SwPaM* pPam )
{
    String *pRet = 0;
    if( pPam && pPam->HasMark() &&
        SearchAlgorithms_REGEXP == rSearchOpt.algorithmType )
    {
        const SwCntntNode* pTxtNode = pPam->GetCntntNode( sal_True );
        if( pTxtNode && pTxtNode->IsTxtNode() && pTxtNode == pPam->GetCntntNode( sal_False ) )
        {
            utl::TextSearch aSTxt( rSearchOpt );
            const String& rStr = static_cast<const SwTxtNode*>(pTxtNode)->GetTxt();
            sal_Int32 nStart = pPam->Start()->nContent.GetIndex();
            sal_Int32 nEnd = pPam->End()->nContent.GetIndex();
            SearchResult aResult;
            if( aSTxt.SearchForward( rStr, &nStart, &nEnd, &aResult ) )
            {
                OUString aReplaceStr( rSearchOpt.replaceString );
                aSTxt.ReplaceBackReferences( aReplaceStr, rStr, aResult );
                pRet = new String( aReplaceStr );
            }
        }
    }
    return pRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
