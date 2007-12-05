/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: findtxt.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-05 16:43:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _COM_SUN_STAR_UTIL_SEARCHOPTIONS_HPP_
#include <com/sun/star/util/SearchOptions.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHFLAGS_HPP_
#include <com/sun/star/util/SearchFlags.hpp>
#endif



#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>

#ifndef _SV_SVAPP_HXX //autogen wg. Application
#include <vcl/svapp.hxx>
#endif

#ifndef _TXATRITR_HXX
#include <txatritr.hxx>
#endif
#ifndef _FLDBAS_HXX //autogen
#include <fldbas.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _TXTATR_HXX //autogen
#include <txtatr.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAMTYP_HXX
#include <pamtyp.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif

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

    USHORT n = 0;
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
            if ( !pHt->GetEnd() && nStt >= nStart )
            {
                //JP 17.05.00: Task 75806 ask for ">=" and not for ">"
                   switch( pHt->Which() )
                {
                case RES_TXTATR_FLYCNT:
                case RES_TXTATR_FTN:
                   case RES_TXTATR_FIELD:
                case RES_TXTATR_REFMARK:
                   case RES_TXTATR_TOXMARK:
                       {
                        // JP 06.05.98: mit Bug 50100 werden sie als Trenner erwuenscht und nicht
                        //              mehr zum Wort dazu gehoerend.
                        // MA 23.06.98: mit Bug 51215 sollen sie konsequenterweise auch am
                        //              Satzanfang und -ende ignoriert werden wenn sie Leer sind.
                        //              Dazu werden sie schlicht entfernt. Fuer den Anfang entfernen
                        //              wir sie einfach.
                        //              Fuer das Ende merken wir uns die Ersetzungen und entferenen
                        //              hinterher alle am Stringende (koenten ja 'normale' 0x7f drinstehen
                           BOOL bEmpty = RES_TXTATR_FIELD != pHt->Which() ||
                            !((SwTxtFld*)pHt)->GetFld().GetFld()->Expand().Len();
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
                   case RES_TXTATR_HARDBLANK:
                    rRet.SetChar( nAkt, ((SwTxtHardBlank*)pHt)->GetChar() );
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

    for( USHORT i = aReplaced.Count(); i; )
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



BYTE SwPaM::Find( const SearchOptions& rSearchOpt, utl::TextSearch& rSTxt,
                    SwMoveFn fnMove, const SwPaM * pRegion,
                    BOOL bInReadOnly )
{
    if( !rSearchOpt.searchString.getLength() )
        return FALSE;

    SwPaM* pPam = MakeRegion( fnMove, pRegion );
    BOOL bSrchForward = fnMove == fnMoveForward;
    SwNodeIndex& rNdIdx = pPam->GetPoint()->nNode;
    SwIndex& rCntntIdx = pPam->GetPoint()->nContent;

    // Wenn am Anfang/Ende, aus dem Node moven
    // beim leeren Node nicht weiter
    if( bSrchForward
        ? ( rCntntIdx.GetIndex() == pPam->GetCntntNode()->Len() &&
            rCntntIdx.GetIndex() )
        : !rCntntIdx.GetIndex() && pPam->GetCntntNode()->Len() )
    {
        if( !(*fnMove->fnNds)( &rNdIdx, FALSE ))
        {
            delete pPam;
            return FALSE;
        }
        SwCntntNode *pNd = rNdIdx.GetNode().GetCntntNode();
        xub_StrLen nTmpPos = bSrchForward ? 0 : pNd->Len();
        rCntntIdx.Assign( pNd, nTmpPos );
    }

    /*
     * Ist bFound == TRUE, dann wurde der String gefunden und in
     * nStart und nEnde steht der gefundenen String
     */
    BOOL bFound = FALSE;
    /*
     * StartPostion im Text oder Anfangsposition
     */
    BOOL bFirst = TRUE;
    SwCntntNode * pNode;
    String sCleanStr;
    SvULongs aFltArr;

    xub_StrLen nStart, nEnde, nTxtLen;
    const SwNode* pSttNd = &rNdIdx.GetNode();

    BOOL bRegSearch = SearchAlgorithms_REGEXP == rSearchOpt.algorithmType;
    BOOL bChkEmptyPara = bRegSearch && 2 == rSearchOpt.searchString.getLength() &&
                        ( !rSearchOpt.searchString.compareToAscii( "^$" ) ||
                          !rSearchOpt.searchString.compareToAscii( "$^" ) );
    BOOL bChkParaEnd = bRegSearch && 1 == rSearchOpt.searchString.getLength() &&
                      !rSearchOpt.searchString.compareToAscii( "$" );

    LanguageType eLastLang = 0;
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
            USHORT nSearchScript = 0;
            USHORT nCurrScript = 0;

            if ( SearchAlgorithms_APPROXIMATE == rSearchOpt.algorithmType &&
                 pBreakIt->xBreak.is() )
            {
                pScriptIter = new SwScriptIterator( sCleanStr, nStart, bSrchForward );
                nSearchScript = pBreakIt->GetRealScriptOfText( rSearchOpt.searchString, 0 );
            }

            xub_StrLen nStringEnd = nEnde;
            while ( bSrchForward && nStart < nStringEnd ||
                    ! bSrchForward && nStart > nStringEnd )
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
                    bFound = TRUE;
                    break;
                }

                nStart = nEnde;
            } // end of script while

            delete pScriptIter;

            if ( bFound )
                break;
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
                    bFound = TRUE;
                    break;
                }
            }
        }
    }
    delete pPam;
    return bFound;
}


// Parameter fuers Suchen und Ersetzen von Text
struct SwFindParaText : public SwFindParas
{
    const SearchOptions& rSearchOpt;
    SwCursor& rCursor;
    utl::TextSearch aSTxt;
    BOOL bReplace;

    SwFindParaText( const SearchOptions& rOpt, int bRepl, SwCursor& rCrsr )
        : rSearchOpt( rOpt ), rCursor( rCrsr ), aSTxt( rOpt ), bReplace( 0 != bRepl )
    {}
    virtual int Find( SwPaM* , SwMoveFn , const SwPaM*, BOOL bInReadOnly );
    virtual int IsReplaceMode() const;
    virtual ~SwFindParaText();
};

SwFindParaText::~SwFindParaText()
{
}

int SwFindParaText::Find( SwPaM* pCrsr, SwMoveFn fnMove,
                            const SwPaM* pRegion, BOOL bInReadOnly )
{
    if( bInReadOnly && bReplace )
        bInReadOnly = FALSE;

    BOOL bFnd = (BOOL)pCrsr->Find( rSearchOpt, aSTxt, fnMove, pRegion, bInReadOnly );
    // kein Bereich ??
    if( bFnd && *pCrsr->GetMark() == *pCrsr->GetPoint() )
        return FIND_NOT_FOUND;

    if( bFnd && bReplace )          // String ersetzen ??
    {
        // Replace-Methode vom SwDoc benutzen
        int bRegExp = SearchAlgorithms_REGEXP == rSearchOpt.algorithmType;
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

        String *pRepl = bRegExp ? ReplaceBackReferences( rSearchOpt, pCrsr ) : 0;
        if( pRepl )
            rCursor.GetDoc()->Replace( *pCrsr, *pRepl, bRegExp );
        else
            rCursor.GetDoc()->Replace( *pCrsr, rSearchOpt.replaceString, bRegExp );
        delete pRepl;
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
        rSttCntIdx = nSttCnt;
        return FIND_NO_RING;
    }
    return bFnd ? FIND_FOUND : FIND_NOT_FOUND;
}


int SwFindParaText::IsReplaceMode() const
{
    return bReplace;
}


ULONG SwCursor::Find( const SearchOptions& rSearchOpt,
                        SwDocPositions nStart, SwDocPositions nEnde,
                        BOOL& bCancel,
                        FindRanges eFndRngs, int bReplace )
{
    // OLE-Benachrichtigung abschalten !!
    SwDoc* pDoc = GetDoc();
    Link aLnk( pDoc->GetOle2Link() );
    pDoc->SetOle2Link( Link() );

    BOOL bSttUndo = pDoc->DoesUndo() && bReplace;
    if( bSttUndo )
        pDoc->StartUndo( UNDO_REPLACE, NULL );

    BOOL bSearchSel = 0 != (rSearchOpt.searchFlag & SearchFlags::REG_NOT_BEGINOFLINE);
    if( bSearchSel )
        eFndRngs = (FindRanges)(eFndRngs | FND_IN_SEL);
    SwFindParaText aSwFindParaText( rSearchOpt, bReplace, *this );
    ULONG nRet = FindAll( aSwFindParaText, nStart, nEnde, eFndRngs, bCancel );
    pDoc->SetOle2Link( aLnk );
    if( nRet && bReplace )
        pDoc->SetModified();

    if( bSttUndo )
        pDoc->EndUndo( UNDO_REPLACE, NULL );
    return nRet;
}

String *ReplaceBackReferences( const SearchOptions& rSearchOpt, SwPaM* pPam )
{
    String *pRet = 0;
    if( pPam && pPam->HasMark() &&
        SearchAlgorithms_REGEXP == rSearchOpt.algorithmType )
    {
        const SwCntntNode* pTxtNode = pPam->GetCntntNode( TRUE );
        if( pTxtNode && pTxtNode->IsTxtNode() && pTxtNode == pPam->GetCntntNode( FALSE ) )
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
            if( pPam->End()->nContent < (static_cast<const SwTxtNode*>(pTxtNode))->GetTxt().Len() )
                aStr.Insert( sX );
            SearchResult aResult;
            if( aSTxt.SearchFrwrd( aStr, &nStart, &nEnd, &aResult ) )
            {
                aSTxt.ReplaceBackReferences( aReplaceStr, aStr, aResult );
                pRet = new String( aReplaceStr );
            }
        }
    }
    return pRet;
}


