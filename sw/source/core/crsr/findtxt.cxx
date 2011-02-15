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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>

#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#include <svl/svstdarr.hxx>

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

String& lcl_CleanStr( const SwTxtNode& rNd, xub_StrLen nStart,
                      xub_StrLen& rEnde, SvULongs& rArr, String& rRet,
                      bool bRemoveSoftHyphen )
{
    rRet = rNd.GetTxt();
    if( rArr.Count() )
        rArr.Remove( 0, rArr.Count() );

    const SwpHints *pHts = rNd.GetpSwpHints();

    sal_uInt16 n = 0;
    xub_StrLen nSoftHyphen = nStart;
    xub_StrLen nHintStart = STRING_LEN;
    bool bNewHint       = true;
    bool bNewSoftHyphen = true;
    const xub_StrLen nEnd = rEnde;
    SvUShorts aReplaced;

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

        const xub_StrLen nAkt = nStt - rArr.Count();

        if ( bNewHint )
        {
            const SwTxtAttr* pHt = (*pHts)[n];
            if ( pHt->HasDummyChar() && (nStt >= nStart) )
            {
                //JP 17.05.00: Task 75806 ask for ">=" and not for ">"
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
                        // JP 06.05.98: mit Bug 50100 werden sie als Trenner erwuenscht und nicht
                        //              mehr zum Wort dazu gehoerend.
                        // MA 23.06.98: mit Bug 51215 sollen sie konsequenterweise auch am
                        //              Satzanfang und -ende ignoriert werden wenn sie Leer sind.
                        //              Dazu werden sie schlicht entfernt. Fuer den Anfang entfernen
                        //              wir sie einfach.
                        //              Fuer das Ende merken wir uns die Ersetzungen und entferenen
                        //              hinterher alle am Stringende (koenten ja 'normale' 0x7f drinstehen
                           sal_Bool bEmpty = RES_TXTATR_FIELD != pHt->Which() ||
                            !(static_cast<SwTxtFld const*>(pHt)
                                ->GetFld().GetFld()->ExpandField(true).Len());
                        if ( bEmpty && nStart == nAkt )
                           {
                            rArr.Insert( nAkt, rArr.Count() );
                            --rEnde;
                            rRet.Erase( nAkt, 1 );
                           }
                        else
                           {
                            if ( bEmpty )
                                aReplaced.Insert( nAkt, aReplaced.Count() );
                            rRet.SetChar( nAkt, '\x7f' );
                           }
                       }
                       break;
                   default:
                    ASSERT( false, "unknown case in lcl_CleanStr" )
                    break;
                }
            }
            ++n;
        }

        if ( bNewSoftHyphen )
        {
              rArr.Insert( nAkt, rArr.Count() );
            --rEnde;
               rRet.Erase( nAkt, 1 );
            ++nSoftHyphen;
        }
    }
    while ( true );

    for( sal_uInt16 i = aReplaced.Count(); i; )
    {
        const xub_StrLen nTmp = aReplaced[ --i ];
        if( nTmp == rRet.Len() - 1 )
        {
            rRet.Erase( nTmp );
            rArr.Insert( nTmp, rArr.Count() );
            --rEnde;
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
    if( !rSearchOpt.searchString.getLength() )
        return sal_False;

    SwPaM* pPam = MakeRegion( fnMove, pRegion );
    sal_Bool bSrchForward = fnMove == fnMoveForward;
    SwNodeIndex& rNdIdx = pPam->GetPoint()->nNode;
    SwIndex& rCntntIdx = pPam->GetPoint()->nContent;

    // Wenn am Anfang/Ende, aus dem Node moven
    // beim leeren Node nicht weiter
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

    /*
     * Ist bFound == sal_True, dann wurde der String gefunden und in
     * nStart und nEnde steht der gefundenen String
     */
    sal_Bool bFound = sal_False;
    /*
     * StartPostion im Text oder Anfangsposition
     */
    sal_Bool bFirst = sal_True;
    SwCntntNode * pNode;
    //testarea
    //String sCleanStr;
    //SvULongs aFltArr;
    //const SwNode* pSttNd = &rNdIdx.GetNode();

    xub_StrLen nStart, nEnde, nTxtLen;

    sal_Bool bRegSearch = SearchAlgorithms_REGEXP == rSearchOpt.algorithmType;
    sal_Bool bChkEmptyPara = bRegSearch && 2 == rSearchOpt.searchString.getLength() &&
                        ( !rSearchOpt.searchString.compareToAscii( "^$" ) ||
                          !rSearchOpt.searchString.compareToAscii( "$^" ) );
    sal_Bool bChkParaEnd = bRegSearch && 1 == rSearchOpt.searchString.getLength() &&
                      !rSearchOpt.searchString.compareToAscii( "$" );

//    LanguageType eLastLang = 0;
    while( 0 != ( pNode = ::GetNode( *pPam, bFirst, fnMove, bInReadOnly ) ))
    {
        if( pNode->IsTxtNode() )
        {
            nTxtLen = ((SwTxtNode*)pNode)->GetTxt().Len();
            if( rNdIdx == pPam->GetMark()->nNode )
                nEnde = pPam->GetMark()->nContent.GetIndex();
            else
                nEnde = bSrchForward ? nTxtLen : 0;
            nStart = rCntntIdx.GetIndex();

            /* #i80135# */
            // if there are SwPostItFields inside our current node text, we split the text into seperate pieces
            // and search for text inside the pieces as well as inside the fields
            const SwpHints *pHts = ((SwTxtNode*)pNode)->GetpSwpHints();

            // count postitfields by looping over all fields
            xub_StrLen aNumberPostits = 0;
            xub_StrLen aIgnore = 0;
            if (pHts && bSearchInNotes)
            {
                if (!bSrchForward)
                {
                    xub_StrLen swap = nEnde;
                    nEnde = nStart;
                    nStart = swap;
                }

                for (xub_StrLen i = 0; i <pHts->Count();i++)
                {
                    xub_StrLen aPos = *(*pHts)[i]->GetStart();
                    const SwTxtAttr* pTxtAttr = (*pHts)[i];
                    if ( (pTxtAttr->Which()==RES_TXTATR_FIELD) &&
                                (pTxtAttr->GetFld().GetFld()->Which()==RES_POSTITFLD))
                    {
                        if ( (aPos >= nStart) && (aPos <= nEnde) )
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
                    xub_StrLen swap = nEnde;
                    nEnde = nStart;
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
                xub_StrLen nEndeInside = 0;
                sal_Int16 aLoop= bSrchForward ? aStart : aNumberPostits;

                while ( (aLoop>=0) && (aLoop<=aNumberPostits))
                {
                    if (bSrchForward)
                    {
                        nStartInside =  aLoop==0 ? nStart : *(*pHts)[GetPostIt(aLoop+aIgnore-1,pHts)]->GetStart()+1;
                        nEndeInside = aLoop==aNumberPostits? nEnde : *(*pHts)[GetPostIt(aLoop+aIgnore,pHts)]->GetStart();
                        nTxtLen = nEndeInside-nStartInside;
                    }
                    else
                    {
                        nStartInside =  aLoop==aNumberPostits ? nStart : *(*pHts)[GetPostIt(aLoop+aIgnore,pHts)]->GetStart();
                        nEndeInside = aLoop==0 ? nEnde : *(*pHts)[GetPostIt(aLoop+aIgnore-1,pHts)]->GetStart()+1;
                        nTxtLen = nStartInside-nEndeInside;
                    }
                    // search inside the text between a note
                    bFound = DoSearch(rSearchOpt,rSTxt,fnMove,bSrchForward,bRegSearch,bChkEmptyPara,bChkParaEnd,
                                nStartInside,nEndeInside,nTxtLen, pNode,pPam);
                    if (bFound)
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
                // if there is no SwPostItField inside or searching inside notes is disabled, we search the whole length just like before
                bFound = DoSearch(rSearchOpt,rSTxt,fnMove,bSrchForward,bRegSearch,bChkEmptyPara,bChkParaEnd,
                            nStart,nEnde,nTxtLen, pNode,pPam);
            }
            if (bFound)
                break;
        }
    }
    delete pPam;
    return bFound;
}

bool SwPaM::DoSearch( const SearchOptions& rSearchOpt, utl::TextSearch& rSTxt,
                    SwMoveFn fnMove,
                    sal_Bool bSrchForward, sal_Bool bRegSearch, sal_Bool bChkEmptyPara, sal_Bool bChkParaEnd,
                    xub_StrLen &nStart, xub_StrLen &nEnde, xub_StrLen nTxtLen,SwNode* pNode, SwPaM* pPam)
{
    bool bFound = false;
    SwNodeIndex& rNdIdx = pPam->GetPoint()->nNode;
    const SwNode* pSttNd = &rNdIdx.GetNode();
    String sCleanStr;
    SvULongs aFltArr;
    LanguageType eLastLang = 0;
    // if the search string contains a soft hypen, we don't strip them from the text:
    bool bRemoveSoftHyphens = true;
    if ( bRegSearch )
    {
        const rtl::OUString a00AD( rtl::OUString::createFromAscii( "\\x00AD" ) );
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
        lcl_CleanStr( *(SwTxtNode*)pNode, nStart, nEnde,
                        aFltArr, sCleanStr, bRemoveSoftHyphens );
    else
        lcl_CleanStr( *(SwTxtNode*)pNode, nEnde, nStart,
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

    xub_StrLen nStringEnd = nEnde;
    while ( (bSrchForward && nStart < nStringEnd) ||
            (! bSrchForward && nStart > nStringEnd) )
    {
        // SearchAlgorithms_APPROXIMATE works on a per word base
        // so we have to provide the text searcher with the correct
        // locale, because it uses the breakiterator
        if ( pScriptIter )
        {
            nEnde = pScriptIter->GetScriptChgPos();
            nCurrScript = pScriptIter->GetCurrScript();
            if ( nSearchScript == nCurrScript )
            {
                const LanguageType eCurrLang =
                        ((SwTxtNode*)pNode)->GetLang( bSrchForward ?
                                                      nStart :
                                                      nEnde );

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
            (rSTxt.*fnMove->fnSearch)( sCleanStr, &nStart, &nEnde, 0 ))
        {
            // setze den Bereich richtig
            *GetPoint() = *pPam->GetPoint();
            SetMark();

            // Start und Ende wieder korrigieren !!
            if( aFltArr.Count() )
            {
                xub_StrLen n, nNew;
                // bei Rueckwaertssuche die Positionen temp. vertauschen
                if( !bSrchForward ) { n = nStart; nStart = nEnde; nEnde = n; }

                for( n = 0, nNew = nStart;
                    n < aFltArr.Count() && aFltArr[ n ] <= nStart;
                    ++n, ++nNew )
                    ;
                nStart = nNew;
                for( n = 0, nNew = nEnde;
                    n < aFltArr.Count() && aFltArr[ n ] < nEnde;
                    ++n, ++nNew )
                    ;
                nEnde = nNew;

                // bei Rueckwaertssuche die Positionen temp. vertauschen
                if( !bSrchForward ) { n = nStart; nStart = nEnde; nEnde = n; }
            }
            GetMark()->nContent = nStart;       // Startposition setzen
            GetPoint()->nContent = nEnde;

            if( !bSrchForward )         // rueckwaerts Suche?
                Exchange();             // Point und Mark tauschen
            bFound = sal_True;
            break;
        }

        nStart = nEnde;
    } // end of script while

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
            if( !bSrchForward )         // rueckwaerts Suche?
                Exchange();             // Point und Mark tauschen
            //bFound = sal_True;
            //break;
            return true;
        }
    }
    return bFound;
}

// Parameter fuers Suchen und Ersetzen von Text
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

    /*   #i80135# if we found something in a note, Mark and Point is the same
    if( bFnd && *pCrsr->GetMark() == *pCrsr->GetPoint() )
        return FIND_NOT_FOUND;
    */

    if( bFnd && bReplace )          // String ersetzen ??
    {
        // Replace-Methode vom SwDoc benutzen
        const bool bRegExp(SearchAlgorithms_REGEXP == rSearchOpt.algorithmType);
        SwIndex& rSttCntIdx = pCrsr->Start()->nContent;
        xub_StrLen nSttCnt = rSttCntIdx.GetIndex();
        // damit die Region auch verschoben wird, in den Shell-Cursr-Ring
        // mit aufnehmen !!
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
            // und die Region wieder herausnehmen:
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
                        SwDocPositions nStart, SwDocPositions nEnde,
                        sal_Bool& bCancel,
                        FindRanges eFndRngs, int bReplace )
{
    // OLE-Benachrichtigung abschalten !!
    SwDoc* pDoc = GetDoc();
    Link aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    bool const bStartUndo = pDoc->GetIDocumentUndoRedo().DoesUndo() && bReplace;
    if (bStartUndo)
    {
        pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_REPLACE, NULL );
    }

    sal_Bool bSearchSel = 0 != (rSearchOpt.searchFlag & SearchFlags::REG_NOT_BEGINOFLINE);
    if( bSearchSel )
        eFndRngs = (FindRanges)(eFndRngs | FND_IN_SEL);
    SwFindParaText aSwFindParaText( rSearchOpt, bSearchInNotes, bReplace, *this );
    sal_uLong nRet = FindAll( aSwFindParaText, nStart, nEnde, eFndRngs, bCancel );
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
             aStr.EraseAllChars( CH_TXTATR_BREAKWORD );
             aStr.EraseAllChars( CH_TXTATR_INWORD );
            xub_StrLen nStart = 0;
            String sX( 'x' );
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


