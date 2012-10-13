/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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
    xub_StrLen nSoftHyphen = nStart;
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
            nSoftHyphen = bRemoveSoftHyphen ?
                          rNd.GetTxt().Search( CHAR_SOFTHYPHEN, nSoftHyphen ) :
                          STRING_LEN;

        bNewHint       = false;
        bNewSoftHyphen = false;

        xub_StrLen nStt = 0;

        // Check if next stop is a hint.
        if ( STRING_LEN != nHintStart && nHintStart < nSoftHyphen && nHintStart < nEnd )
        {
            nStt = nHintStart;
            bNewHint = true;
        }
        // Check if next stop is a soft hyphen.
        else if ( STRING_LEN != nSoftHyphen && nSoftHyphen < nHintStart && nSoftHyphen < nEnd )
        {
            nStt = nSoftHyphen;
            bNewSoftHyphen = true;
        }
        // If nSoftHyphen == nHintStart, the current hint *must* be a hint with an end.
        else if ( STRING_LEN != nSoftHyphen && nSoftHyphen == nHintStart )
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
                            !(static_cast<SwTxtFld const*>(pHt)
                                ->GetFld().GetFld()->ExpandField(true).Len());
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

sal_uInt8 SwPaM::Find( const SearchOptions& rSearchOpt, sal_Bool bSearchInNotes , utl::TextSearch& rSTxt,
                    SwMoveFn fnMove, const SwPaM * pRegion,
                    sal_Bool bInReadOnly )
{
    if( rSearchOpt.searchString.isEmpty() )
        return sal_False;

    SwPaM* pPam = MakeRegion( fnMove, pRegion );
    sal_Bool bSrchForward = fnMove == fnMoveForward;
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
            return sal_False;
        }
        SwCntntNode *pNd = rNdIdx.GetNode().GetCntntNode();
        xub_StrLen nTmpPos = bSrchForward ? 0 : pNd->Len();
        rCntntIdx.Assign( pNd, nTmpPos );
    }

    // If bFound is true then the string was found and is between nStart and nEnd
    sal_Bool bFound = sal_False;
    // start position in text or initial position
    sal_Bool bFirst = sal_True;
    SwCntntNode * pNode;

    xub_StrLen nStart, nEnd, nTxtLen;

    sal_Bool bRegSearch = SearchAlgorithms_REGEXP == rSearchOpt.algorithmType;
    sal_Bool bChkEmptyPara = bRegSearch && 2 == rSearchOpt.searchString.getLength() &&
                        ( !rSearchOpt.searchString.compareToAscii( "^$" ) ||
                          !rSearchOpt.searchString.compareToAscii( "$^" ) );
    sal_Bool bChkParaEnd = bRegSearch && 1 == rSearchOpt.searchString.getLength() &&
                      !rSearchOpt.searchString.compareToAscii( "$" );

    // LanguageType eLastLang = 0;
    while( 0 != ( pNode = ::GetNode( *pPam, bFirst, fnMove, bInReadOnly ) ))
    {
        if( pNode->IsTxtNode() )
        {
            nTxtLen = ((SwTxtNode*)pNode)->GetTxt().Len();
            if( rNdIdx == pPam->GetMark()->nNode )
                nEnd = pPam->GetMark()->nContent.GetIndex();
            else
                nEnd = bSrchForward ? nTxtLen : 0;
            nStart = rCntntIdx.GetIndex();

            /* #i80135# */
            // if there are SwPostItFields inside our current node text, we
            // split the text into seperate pieces and search for text inside
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
                      SwMoveFn fnMove, sal_Bool bSrchForward, sal_Bool bRegSearch,
                      sal_Bool bChkEmptyPara, sal_Bool bChkParaEnd,
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
        const rtl::OUString a00AD(RTL_CONSTASCII_USTRINGPARAM("\\x00AD"));
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
         pBreakIt->GetBreakIter().is() )
    {
        pScriptIter = new SwScriptIterator( sCleanStr, nStart, bSrchForward );
        nSearchScript = pBreakIt->GetRealScriptOfText( rSearchOpt.searchString, 0 );
    }

    xub_StrLen nStringEnd = nEnd;
    while ( (bSrchForward && nStart < nStringEnd) ||
            (! bSrchForward && nStart > nStringEnd) )
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
                            pBreakIt->GetLocale( eCurrLang ) );
                    rSTxt.SetLocale( rSearchOpt, aLocale );
                    eLastLang = eCurrLang;
                }
            }
            pScriptIter->Next();
        }

        if( nSearchScript == nCurrScript &&
            (rSTxt.*fnMove->fnSearch)( sCleanStr, &nStart, &nEnd, 0 ))
        {
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
            bFound = sal_True;
            break;
        }
        nStart = nEnd;
    }

    delete pScriptIter;

    if ( bFound )
        return true;
    else if( ( bChkEmptyPara && !nStart && !nTxtLen ) || bChkParaEnd )
    {
        *GetPoint() = *pPam->GetPoint();
        GetPoint()->nContent = bChkParaEnd ? nTxtLen : 0;
        SetMark();
        if( (bSrchForward || pSttNd != &rNdIdx.GetNode()) &&
            Move( fnMoveForward, fnGoCntnt ) &&
            (!bSrchForward || pSttNd != &GetPoint()->nNode.GetNode()) &&
            1 == Abs( (int)( GetPoint()->nNode.GetIndex() -
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

    sal_Bool bFnd = (sal_Bool)pCrsr->Find( rSearchOpt, bSearchInNotes, aSTxt, fnMove, pRegion, bInReadOnly );


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
            String aStr( pPam->GetTxt() );
            String aSearchStr( rSearchOpt.searchString );
            String aReplaceStr( rSearchOpt.replaceString );
            aStr = comphelper::string::remove(aStr, CH_TXTATR_BREAKWORD);
            aStr = comphelper::string::remove(aStr, CH_TXTATR_INWORD);
            xub_StrLen nStart = 0;
            rtl::OUString sX( 'x' );
            if( pPam->Start()->nContent > 0 )
            {
                aStr.Insert( sX, 0 );
                ++nStart;
            }
            xub_StrLen nEnd = aStr.Len();
            bool bDeleteLastX = false;
            if( pPam->End()->nContent < (static_cast<const SwTxtNode*>(pTxtNode))->GetTxt().Len() )
            {
                aStr.Insert( sX );
                bDeleteLastX = true;
            }
            SearchResult aResult;
            if( aSTxt.SearchFrwrd( aStr, &nStart, &nEnd, &aResult ) )
            {
                if( bDeleteLastX )
                    aStr.Erase( aStr.Len() - 1 );
                aSTxt.ReplaceBackReferences( aReplaceStr, aStr, aResult );
                pRet = new String( aReplaceStr );
            }
        }
    }
    return pRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
