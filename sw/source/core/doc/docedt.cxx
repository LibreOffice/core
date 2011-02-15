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

#include <string.h>         // fuer strchr()
#include <hintids.hxx>

#include <vcl/sound.hxx>
#include <editeng/cscoitem.hxx>
#include <editeng/brkitem.hxx>
#include <linguistic/lngprops.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/i18n/WordType.hdl>
#include <unotools/charclass.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <fmtpdsc.hxx>
#include <txtftn.hxx>
#include <acorrect.hxx>     // Autokorrektur
#include <IMark.hxx>        // fuer SwBookmark
#include <cntfrm.hxx>           // fuers Spell
#include <crsrsh.hxx>
#include <doc.hxx>
#include <UndoManager.hxx>
#include <docsh.hxx>
#include <docary.hxx>
#include <doctxm.hxx>       // beim Move: Verzeichnisse korrigieren
#include <ftnidx.hxx>
#include <ftninfo.hxx>
#include <mdiexp.hxx>       // Statusanzeige
#include <mvsave.hxx>       // Strukturen zum Sichern beim Move/Delete
#include <ndtxt.hxx>
#include <pam.hxx>
#include <redline.hxx>
#include <rootfrm.hxx>          // fuers UpdateFtn
#include <splargs.hxx>      // fuer Spell
#include <swtable.hxx>
#include <swundo.hxx>       // fuer die UndoIds
#include <txtfrm.hxx>
#include <hints.hxx>
#include <UndoSplitMove.hxx>
#include <UndoRedline.hxx>
#include <UndoOverwrite.hxx>
#include <UndoInsert.hxx>
#include <UndoDelete.hxx>
#include <breakit.hxx>
#include <hhcwrp.hxx>
#include <breakit.hxx>
#include <vcl/msgbox.hxx>
#include "comcore.hrc"
#include "editsh.hxx"
#include <unoflatpara.hxx>
#include <SwGrammarMarkUp.hxx>

#include <vector>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::i18n;

//using namespace ::utl;
#ifndef S2U
#define S2U(rString) OUString::createFromAscii(rString)
#endif

struct _SaveRedline
{
    SwRedline* pRedl;
    sal_uInt32 nStt, nEnd;
    xub_StrLen nSttCnt, nEndCnt;

    _SaveRedline( SwRedline* pR, const SwNodeIndex& rSttIdx )
        : pRedl( pR )
    {
        const SwPosition* pStt = pR->Start(),
            * pEnd = pR->GetMark() == pStt ? pR->GetPoint() : pR->GetMark();
        sal_uInt32 nSttIdx = rSttIdx.GetIndex();
        nStt = pStt->nNode.GetIndex() - nSttIdx;
        nSttCnt = pStt->nContent.GetIndex();
        if( pR->HasMark() )
        {
            nEnd = pEnd->nNode.GetIndex() - nSttIdx;
            nEndCnt = pEnd->nContent.GetIndex();
        }

        pRedl->GetPoint()->nNode = 0;
        pRedl->GetPoint()->nContent.Assign( 0, 0 );
        pRedl->GetMark()->nNode = 0;
        pRedl->GetMark()->nContent.Assign( 0, 0 );
    }

    _SaveRedline( SwRedline* pR, const SwPosition& rPos )
        : pRedl( pR )
    {
        const SwPosition* pStt = pR->Start(),
            * pEnd = pR->GetMark() == pStt ? pR->GetPoint() : pR->GetMark();
        sal_uInt32 nSttIdx = rPos.nNode.GetIndex();
        nStt = pStt->nNode.GetIndex() - nSttIdx;
        nSttCnt = pStt->nContent.GetIndex();
        if( nStt == 0 )
            nSttCnt = nSttCnt - rPos.nContent.GetIndex();
        if( pR->HasMark() )
        {
            nEnd = pEnd->nNode.GetIndex() - nSttIdx;
            nEndCnt = pEnd->nContent.GetIndex();
            if( nEnd == 0 )
                nEndCnt = nEndCnt - rPos.nContent.GetIndex();
        }

        pRedl->GetPoint()->nNode = 0;
        pRedl->GetPoint()->nContent.Assign( 0, 0 );
        pRedl->GetMark()->nNode = 0;
        pRedl->GetMark()->nContent.Assign( 0, 0 );
    }

    void SetPos( sal_uInt32 nInsPos )
    {
        pRedl->GetPoint()->nNode = nInsPos + nStt;
        pRedl->GetPoint()->nContent.Assign( pRedl->GetCntntNode(), nSttCnt );
        if( pRedl->HasMark() )
        {
            pRedl->GetMark()->nNode = nInsPos + nEnd;
            pRedl->GetMark()->nContent.Assign( pRedl->GetCntntNode(sal_False), nEndCnt );
        }
    }

    void SetPos( const SwPosition& aPos )
    {
        pRedl->GetPoint()->nNode = aPos.nNode.GetIndex() + nStt;
        pRedl->GetPoint()->nContent.Assign( pRedl->GetCntntNode(), nSttCnt + ( nStt == 0 ? aPos.nContent.GetIndex() : 0 ) );
        if( pRedl->HasMark() )
        {
            pRedl->GetMark()->nNode = aPos.nNode.GetIndex() + nEnd;
            pRedl->GetMark()->nContent.Assign( pRedl->GetCntntNode(sal_False), nEndCnt  + ( nEnd == 0 ? aPos.nContent.GetIndex() : 0 ) );
        }
    }
};

SV_DECL_PTRARR_DEL( _SaveRedlines, _SaveRedline*, 0, 4 )

SV_IMPL_VARARR( _SaveFlyArr, _SaveFly )
SV_IMPL_PTRARR( _SaveRedlines, _SaveRedline* )

bool lcl_MayOverwrite( const SwTxtNode *pNode, const xub_StrLen nPos )
{
    sal_Unicode cChr = pNode->GetTxt().GetChar( nPos );
    return !( ( CH_TXTATR_BREAKWORD == cChr || CH_TXTATR_INWORD == cChr ) &&
              (0 != pNode->GetTxtAttrForCharAt( nPos ) ) );
}

void lcl_SkipAttr( const SwTxtNode *pNode, SwIndex &rIdx, xub_StrLen &rStart )
{
    if( !lcl_MayOverwrite( pNode, rStart ) )
    {
        // ueberspringe alle SonderAttribute
        do {
            // "Beep" bei jedem ausgelassenen
            Sound::Beep(SOUND_ERROR);
            rIdx++;
        } while( (rStart = rIdx.GetIndex()) < pNode->GetTxt().Len()
               && !lcl_MayOverwrite(pNode, rStart) );
    }
}

// -----------------------------------------------------------------

void _RestFlyInRange( _SaveFlyArr & rArr, const SwNodeIndex& rSttIdx,
                      const SwNodeIndex* pInsertPos )
{
    SwPosition aPos( rSttIdx );
    for( sal_uInt16 n = 0; n < rArr.Count(); ++n )
    {
        // neuen Anker anlegen
        _SaveFly& rSave = rArr[n];
        SwFrmFmt* pFmt = rSave.pFrmFmt;

        if( rSave.bInsertPosition )
        {
            if( pInsertPos != NULL )
                aPos.nNode = *pInsertPos;
            else
                aPos.nNode = rSttIdx.GetIndex();
        }
        else
            aPos.nNode = rSttIdx.GetIndex() + rSave.nNdDiff;

        aPos.nContent.Assign( 0, 0 );
        SwFmtAnchor aAnchor( pFmt->GetAnchor() );
        aAnchor.SetAnchor( &aPos );
        pFmt->GetDoc()->GetSpzFrmFmts()->Insert(
                pFmt, pFmt->GetDoc()->GetSpzFrmFmts()->Count() );
        pFmt->SetFmtAttr( aAnchor );
        SwCntntNode* pCNd = aPos.nNode.GetNode().GetCntntNode();
        if( pCNd && pCNd->GetFrm( 0, 0, sal_False ) )
            pFmt->MakeFrms();
    }
}

void _SaveFlyInRange( const SwNodeRange& rRg, _SaveFlyArr& rArr )
{
    SwSpzFrmFmts& rFmts = *rRg.aStart.GetNode().GetDoc()->GetSpzFrmFmts();
    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        SwFrmFmt *const pFmt = static_cast<SwFrmFmt*>(rFmts[n]);
        SwFmtAnchor const*const pAnchor = &pFmt->GetAnchor();
        SwPosition const*const pAPos = pAnchor->GetCntntAnchor();
        if (pAPos &&
            ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
             (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
            rRg.aStart <= pAPos->nNode && pAPos->nNode < rRg.aEnd )
        {
            _SaveFly aSave( pAPos->nNode.GetIndex() - rRg.aStart.GetIndex(),
                            pFmt, sal_False );
            rArr.Insert( aSave, rArr.Count());
            pFmt->DelFrms();
            rFmts.Remove( n--, 1 );
        }
    }
}

void _SaveFlyInRange( const SwPaM& rPam, const SwNodeIndex& rInsPos,
                       _SaveFlyArr& rArr, bool bMoveAllFlys )
{
    SwSpzFrmFmts& rFmts = *rPam.GetPoint()->nNode.GetNode().GetDoc()->GetSpzFrmFmts();
    SwFrmFmt* pFmt;
    const SwFmtAnchor* pAnchor;

    const SwPosition* pPos = rPam.Start();
    const SwNodeIndex& rSttNdIdx = pPos->nNode;
    short nSttOff = (!bMoveAllFlys && rSttNdIdx.GetNode().IsCntntNode() &&
                    pPos->nContent.GetIndex()) ? 1 : 0;

    pPos = rPam.GetPoint() == pPos ? rPam.GetMark() : rPam.GetPoint();
    const SwNodeIndex& rEndNdIdx = pPos->nNode;
    short nOff = ( bMoveAllFlys || ( rEndNdIdx.GetNode().IsCntntNode() &&
                pPos->nContent == rEndNdIdx.GetNode().GetCntntNode()->Len() ))
                    ? 0 : 1;

    const SwNodeIndex* pCntntIdx;

    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        sal_Bool bInsPos = sal_False;
        pFmt = (SwFrmFmt*)rFmts[n];
        pAnchor = &pFmt->GetAnchor();
        const SwPosition* pAPos = pAnchor->GetCntntAnchor();
        if (pAPos &&
            ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
             (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
            // nicht verschieben, wenn die InsPos im CntntBereich vom Fly ist
            ( 0 == ( pCntntIdx = pFmt->GetCntnt().GetCntntIdx() ) ||
              !( *pCntntIdx < rInsPos &&
                rInsPos < pCntntIdx->GetNode().EndOfSectionIndex() )) )
        {
            if( !bMoveAllFlys && rEndNdIdx == pAPos->nNode )
            {
                // wenn nur teil vom EndNode oder der EndNode und SttNode
                // identisch sind, chaos::Anchor nicht anfassen
                if( rSttNdIdx != pAPos->nNode )
                {
                    // Anker nur an Anfang/Ende haengen
                    SwPosition aPos( rSttNdIdx );
                    SwFmtAnchor aAnchor( *pAnchor );
                    aAnchor.SetAnchor( &aPos );
                    pFmt->SetFmtAttr( aAnchor );
//                  ((SwFmtAnchor*)pAnchor)->SetAnchor( &aPos );
                }
            }
            else if( ( rSttNdIdx.GetIndex() + nSttOff <= pAPos->nNode.GetIndex()
                    && pAPos->nNode.GetIndex() <= rEndNdIdx.GetIndex() - nOff ) ||
                        0 != ( bInsPos = rInsPos == pAPos->nNode ))

            {
                _SaveFly aSave( pAPos->nNode.GetIndex() - rSttNdIdx.GetIndex(),
                                pFmt, bInsPos );
                rArr.Insert( aSave, rArr.Count());
                pFmt->DelFrms();
                rFmts.Remove( n--, 1 );
            }
        }
    }
}

// -----------------------------------------------------------------

// loesche und verschiebe alle "Fly's am Absatz", die in der SSelection
// liegen. Steht am SPoint ein Fly, wird dieser auf den Mark verschoben.

void DelFlyInRange( const SwNodeIndex& rMkNdIdx,
                    const SwNodeIndex& rPtNdIdx )
{
    const sal_Bool bDelFwrd = rMkNdIdx.GetIndex() <= rPtNdIdx.GetIndex();

    SwDoc* pDoc = rMkNdIdx.GetNode().GetDoc();
    SwSpzFrmFmts& rTbl = *pDoc->GetSpzFrmFmts();
    for ( sal_uInt16 i = rTbl.Count(); i; )
    {
        SwFrmFmt *pFmt = rTbl[--i];
        const SwFmtAnchor &rAnch = pFmt->GetAnchor();
        SwPosition const*const pAPos = rAnch.GetCntntAnchor();
        if (pAPos &&
            ((rAnch.GetAnchorId() == FLY_AT_PARA) ||
             (rAnch.GetAnchorId() == FLY_AT_CHAR)) &&
            ( bDelFwrd
                ? rMkNdIdx < pAPos->nNode && pAPos->nNode <= rPtNdIdx
                : rPtNdIdx <= pAPos->nNode && pAPos->nNode < rMkNdIdx ))
        {
            // nur den Anker verschieben ??
            if( rPtNdIdx == pAPos->nNode )
            {
                SwFmtAnchor aAnch( pFmt->GetAnchor() );
                SwPosition aPos( rMkNdIdx );
                aAnch.SetAnchor( &aPos );
                pFmt->SetFmtAttr( aAnch );
            }
            else
            {
                // wird der Fly geloescht muss auch im seinem Inhalt alle
                // Flys geloescht werden !!
                const SwFmtCntnt &rCntnt = pFmt->GetCntnt();
                if( rCntnt.GetCntntIdx() )
                {
                    DelFlyInRange( *rCntnt.GetCntntIdx(),
                                    SwNodeIndex( *rCntnt.GetCntntIdx()->
                                            GetNode().EndOfSectionNode() ));
                    // Position kann sich verschoben haben !
                    if( i > rTbl.Count() )
                        i = rTbl.Count();
                    else if( pFmt != rTbl[i] )
                        i = rTbl.GetPos( pFmt );
                }

                pDoc->DelLayoutFmt( pFmt );

                // --> FME 2004-10-06 #117913# DelLayoutFmt can also
                // trigger the deletion of objects.
                if( i > rTbl.Count() )
                    i = rTbl.Count();
                // <--
            }
        }
    }
}


bool lcl_SaveFtn( const SwNodeIndex& rSttNd, const SwNodeIndex& rEndNd,
                 const SwNodeIndex& rInsPos,
                 SwFtnIdxs& rFtnArr, SwFtnIdxs& rSaveArr,
                 const SwIndex* pSttCnt = 0, const SwIndex* pEndCnt = 0 )
{
    bool bUpdateFtn = sal_False;
    const SwNodes& rNds = rInsPos.GetNodes();
    const bool bDelFtn = rInsPos.GetIndex() < rNds.GetEndOfAutotext().GetIndex() &&
                rSttNd.GetIndex() >= rNds.GetEndOfAutotext().GetIndex();
    const bool bSaveFtn = !bDelFtn &&
                    rInsPos.GetIndex() >= rNds.GetEndOfExtras().GetIndex();
    if( rFtnArr.Count() )
    {

        sal_uInt16 nPos;
        rFtnArr.SeekEntry( rSttNd, &nPos );
        SwTxtFtn* pSrch;
        const SwNode* pFtnNd;

        // loesche/sicher erstmal alle, die dahinter stehen
        while( nPos < rFtnArr.Count() && ( pFtnNd =
            &( pSrch = rFtnArr[ nPos ] )->GetTxtNode())->GetIndex()
                    <= rEndNd.GetIndex() )
        {
            xub_StrLen nFtnSttIdx = *pSrch->GetStart();
            if( ( pEndCnt && pSttCnt )
                ? (( &rSttNd.GetNode() == pFtnNd &&
                     pSttCnt->GetIndex() > nFtnSttIdx) ||
                   ( &rEndNd.GetNode() == pFtnNd &&
                    nFtnSttIdx >= pEndCnt->GetIndex() ))
                : ( &rEndNd.GetNode() == pFtnNd ))
            {
                ++nPos;     // weiter suchen
            }
            else
            {
                // dann weg damit
                if( bDelFtn )
                {
                    SwTxtNode& rTxtNd = (SwTxtNode&)pSrch->GetTxtNode();
                    SwIndex aIdx( &rTxtNd, nFtnSttIdx );
                    rTxtNd.EraseText( aIdx, 1 );
                }
                else
                {
                    pSrch->DelFrms();
                    rFtnArr.Remove( nPos );
                    if( bSaveFtn )
                        rSaveArr.Insert( pSrch );
                }
                bUpdateFtn = sal_True;
            }
        }

        while( nPos-- && ( pFtnNd = &( pSrch = rFtnArr[ nPos ] )->
                GetTxtNode())->GetIndex() >= rSttNd.GetIndex() )
        {
            xub_StrLen nFtnSttIdx = *pSrch->GetStart();
            if( !pEndCnt || !pSttCnt ||
                !( (( &rSttNd.GetNode() == pFtnNd &&
                    pSttCnt->GetIndex() > nFtnSttIdx ) ||
                   ( &rEndNd.GetNode() == pFtnNd &&
                    nFtnSttIdx >= pEndCnt->GetIndex() )) ))
            {
                if( bDelFtn )
                {
                    // dann weg damit
                    SwTxtNode& rTxtNd = (SwTxtNode&)pSrch->GetTxtNode();
                    SwIndex aIdx( &rTxtNd, nFtnSttIdx );
                    rTxtNd.EraseText( aIdx, 1 );
                }
                else
                {
                    pSrch->DelFrms();
                    rFtnArr.Remove( nPos );
                    if( bSaveFtn )
                        rSaveArr.Insert( pSrch );
                }
                bUpdateFtn = sal_True;
            }
        }
    }
    // When moving from redline section into document content section, e.g.
    // after loading a document with (delete-)redlines, the footnote array
    // has to be adjusted... (#i70572)
    if( bSaveFtn )
    {
        SwNodeIndex aIdx( rSttNd );
        while( aIdx < rEndNd ) // Check the moved section
        {
            SwNode* pNode = &aIdx.GetNode();
            if( pNode->IsTxtNode() ) // Looking for text nodes...
            {
                SwpHints *pHints =
                    static_cast<SwTxtNode*>(pNode)->GetpSwpHints();
                if( pHints && pHints->HasFtn() ) //...with footnotes
                {
                    bUpdateFtn = sal_True; // Heureka
                    sal_uInt16 nCount = pHints->Count();
                    for( sal_uInt16 i = 0; i < nCount; ++i )
                    {
                        SwTxtAttr *pAttr = pHints->GetTextHint( i );
                        if ( pAttr->Which() == RES_TXTATR_FTN )
                        {
                            rSaveArr.Insert( static_cast<SwTxtFtn*>(pAttr) );
                        }
                    }
                }
            }
            ++aIdx;
        }
    }
    return bUpdateFtn;
}

void lcl_SaveRedlines( const SwPaM& aPam, _SaveRedlines& rArr )
{
    SwDoc* pDoc = aPam.GetNode()->GetDoc();

    const SwPosition* pStart = aPam.Start();
    const SwPosition* pEnd = aPam.End();

    // get first relevant redline
    sal_uInt16 nCurrentRedline;
    pDoc->GetRedline( *pStart, &nCurrentRedline );
    if( nCurrentRedline > 0)
        nCurrentRedline--;

    // redline mode REDLINE_IGNORE|REDLINE_ON; save old mode
    RedlineMode_t eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern( (RedlineMode_t)(( eOld & ~nsRedlineMode_t::REDLINE_IGNORE) | nsRedlineMode_t::REDLINE_ON ));

    // iterate over relevant redlines and decide for each whether it should
    // be saved, or split + saved
    SwRedlineTbl& rRedlineTable = const_cast<SwRedlineTbl&>( pDoc->GetRedlineTbl() );
    for( ; nCurrentRedline < rRedlineTable.Count(); nCurrentRedline++ )
    {
        SwRedline* pCurrent = rRedlineTable[ nCurrentRedline ];
        SwComparePosition eCompare =
            ComparePosition( *pCurrent->Start(), *pCurrent->End(),
                             *pStart, *pEnd);

        // we must save this redline if it overlaps aPam
        // (we may have to split it, too)
        if( eCompare == POS_OVERLAP_BEHIND  ||
            eCompare == POS_OVERLAP_BEFORE  ||
            eCompare == POS_OUTSIDE ||
            eCompare == POS_INSIDE ||
            eCompare == POS_EQUAL )
        {
            rRedlineTable.Remove( nCurrentRedline-- );

            // split beginning, if necessary
            if( eCompare == POS_OVERLAP_BEFORE  ||
                eCompare == POS_OUTSIDE )
            {

                SwRedline* pNewRedline = new SwRedline( *pCurrent );
                *pNewRedline->End() = *pStart;
                *pCurrent->Start() = *pStart;
                pDoc->AppendRedline( pNewRedline, true );
            }

            // split end, if necessary
            if( eCompare == POS_OVERLAP_BEHIND  ||
                eCompare == POS_OUTSIDE )
            {
                SwRedline* pNewRedline = new SwRedline( *pCurrent );
                *pNewRedline->Start() = *pEnd;
                *pCurrent->End() = *pEnd;
                pDoc->AppendRedline( pNewRedline, true );
            }

            // save the current redline
            _SaveRedline* pSave = new _SaveRedline( pCurrent, *pStart );
            rArr.C40_INSERT( _SaveRedline, pSave, rArr.Count() );
        }
    }

    // restore old redline mode
    pDoc->SetRedlineMode_intern( eOld );
}

void lcl_RestoreRedlines( SwDoc* pDoc, const SwPosition& rPos, _SaveRedlines& rArr )
{
    RedlineMode_t eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern( (RedlineMode_t)(( eOld & ~nsRedlineMode_t::REDLINE_IGNORE) | nsRedlineMode_t::REDLINE_ON ));

    for( sal_uInt16 n = 0; n < rArr.Count(); ++n )
    {
        _SaveRedline* pSave = rArr[ n ];
        pSave->SetPos( rPos );
        pDoc->AppendRedline( pSave->pRedl, true );
    }

    pDoc->SetRedlineMode_intern( eOld );
}


void lcl_SaveRedlines( const SwNodeRange& rRg, _SaveRedlines& rArr )
{
    SwDoc* pDoc = rRg.aStart.GetNode().GetDoc();
    sal_uInt16 nRedlPos;
    SwPosition aSrchPos( rRg.aStart ); aSrchPos.nNode--;
    aSrchPos.nContent.Assign( aSrchPos.nNode.GetNode().GetCntntNode(), 0 );
    if( pDoc->GetRedline( aSrchPos, &nRedlPos ) && nRedlPos )
        --nRedlPos;
    else if( nRedlPos >= pDoc->GetRedlineTbl().Count() )
        return ;

    RedlineMode_t eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern( (RedlineMode_t)(( eOld & ~nsRedlineMode_t::REDLINE_IGNORE) | nsRedlineMode_t::REDLINE_ON ));
    SwRedlineTbl& rRedlTbl = (SwRedlineTbl&)pDoc->GetRedlineTbl();

    do {
        SwRedline* pTmp = rRedlTbl[ nRedlPos ];

        const SwPosition* pRStt = pTmp->Start(),
                        * pREnd = pTmp->GetMark() == pRStt
                            ? pTmp->GetPoint() : pTmp->GetMark();

        if( pRStt->nNode < rRg.aStart )
        {
            if( pREnd->nNode > rRg.aStart && pREnd->nNode < rRg.aEnd )
            {
                // Kopie erzeugen und Ende vom Original ans Ende des
                // MoveBereiches setzen. Die Kopie wird mit verschoben
                SwRedline* pNewRedl = new SwRedline( *pTmp );
                SwPosition* pTmpPos = pNewRedl->Start();
                pTmpPos->nNode = rRg.aStart;
                pTmpPos->nContent.Assign(
                            pTmpPos->nNode.GetNode().GetCntntNode(), 0 );

                _SaveRedline* pSave = new _SaveRedline( pNewRedl, rRg.aStart );
//              rArr.Insert( pSave, rArr.Count() );
                rArr.C40_INSERT( _SaveRedline, pSave, rArr.Count() );

                pTmpPos = pTmp->End();
                pTmpPos->nNode = rRg.aEnd;
                pTmpPos->nContent.Assign(
                            pTmpPos->nNode.GetNode().GetCntntNode(), 0 );
            }
            else if( pREnd->nNode == rRg.aStart )
            {
                SwPosition* pTmpPos = pTmp->End();
                pTmpPos->nNode = rRg.aEnd;
                pTmpPos->nContent.Assign(
                            pTmpPos->nNode.GetNode().GetCntntNode(), 0 );
            }
        }
        else if( pRStt->nNode < rRg.aEnd )
        {
            rRedlTbl.Remove( nRedlPos-- );
            if( pREnd->nNode < rRg.aEnd ||
                ( pREnd->nNode == rRg.aEnd && !pREnd->nContent.GetIndex()) )
            {
                // gesamt verschieben
                _SaveRedline* pSave = new _SaveRedline( pTmp, rRg.aStart );
//              rArr.Insert( pSave, rArr.Count() );
                rArr.C40_INSERT( _SaveRedline, pSave, rArr.Count() );
            }
            else
            {
                // aufsplitten
                SwRedline* pNewRedl = new SwRedline( *pTmp );
                SwPosition* pTmpPos = pNewRedl->End();
                pTmpPos->nNode = rRg.aEnd;
                pTmpPos->nContent.Assign(
                            pTmpPos->nNode.GetNode().GetCntntNode(), 0 );

                _SaveRedline* pSave = new _SaveRedline( pNewRedl, rRg.aStart );
//              rArr.Insert( pSave, rArr.Count() );
                rArr.C40_INSERT( _SaveRedline, pSave, rArr.Count() );

                pTmpPos = pTmp->Start();
                pTmpPos->nNode = rRg.aEnd;
                pTmpPos->nContent.Assign(
                            pTmpPos->nNode.GetNode().GetCntntNode(), 0 );
                pDoc->AppendRedline( pTmp, true );
            }
        }
        else
            break;

    } while( ++nRedlPos < pDoc->GetRedlineTbl().Count() );
    pDoc->SetRedlineMode_intern( eOld );
}

void lcl_RestoreRedlines( SwDoc* pDoc, sal_uInt32 nInsPos, _SaveRedlines& rArr )
{
    RedlineMode_t eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern( (RedlineMode_t)(( eOld & ~nsRedlineMode_t::REDLINE_IGNORE) | nsRedlineMode_t::REDLINE_ON ));

    for( sal_uInt16 n = 0; n < rArr.Count(); ++n )
    {
        _SaveRedline* pSave = rArr[ n ];
        pSave->SetPos( nInsPos );
        pDoc->AppendRedline( pSave->pRedl, true );
    }

    pDoc->SetRedlineMode_intern( eOld );
}

// ------------------------------------------------------------------------
// #i59534: Redo of insertion of multiple text nodes runs into trouble
// because of unnecessary expanded redlines
// From now on this class saves the redline positions of all redlines which ends exact at the
// insert position (node _and_ content index)

_SaveRedlEndPosForRestore::_SaveRedlEndPosForRestore( const SwNodeIndex& rInsIdx, xub_StrLen nCnt )
    : pSavArr( 0 ), pSavIdx( 0 ), nSavCntnt( nCnt )
{
    SwNode& rNd = rInsIdx.GetNode();
    SwDoc* pDest = rNd.GetDoc();
    if( pDest->GetRedlineTbl().Count() )
    {
        sal_uInt16 nFndPos;
        const SwPosition* pEnd;
        SwPosition aSrcPos( rInsIdx, SwIndex( rNd.GetCntntNode(), nCnt ));
        const SwRedline* pRedl = pDest->GetRedline( aSrcPos, &nFndPos );
        while( nFndPos-- && *( pEnd = ( pRedl =
            pDest->GetRedlineTbl()[ nFndPos ] )->End() ) == aSrcPos && *pRedl->Start() < aSrcPos )
        {
            if( !pSavArr )
            {
                pSavArr = new SvPtrarr( 2, 2 );
                pSavIdx = new SwNodeIndex( rInsIdx, -1 );
            }
            void* p = (void*)pEnd;
            pSavArr->Insert( p, pSavArr->Count() );
        }
    }
}

_SaveRedlEndPosForRestore::~_SaveRedlEndPosForRestore()
{
    if( pSavArr )
    {
        delete pSavArr;
        delete pSavIdx;
    }
}

void _SaveRedlEndPosForRestore::_Restore()
{
    (*pSavIdx)++;
    SwCntntNode* pNode = pSavIdx->GetNode().GetCntntNode();
    // If there's no content node at the remembered position, we will not restore the old position
    // This may happen if a table (or section?) will be inserted.
    if( pNode )
    {
        SwPosition aPos( *pSavIdx, SwIndex( pNode, nSavCntnt ));
        for( sal_uInt16 n = pSavArr->Count(); n; )
            *((SwPosition*)pSavArr->GetObject( --n )) = aPos;
    }
}


// ------------------------------------------------------------------------

// Loeschen einer vollstaendigen Section des NodesArray.
// Der uebergebene Node steht irgendwo in der gewuenschten Section
void SwDoc::DeleteSection( SwNode *pNode )
{
    ASSERT( pNode, "Kein Node uebergeben." );
    SwStartNode* pSttNd = pNode->IsStartNode() ? (SwStartNode*)pNode
                                               : pNode->StartOfSectionNode();
    SwNodeIndex aSttIdx( *pSttNd ), aEndIdx( *pNode->EndOfSectionNode() );

    // dann loesche mal alle Fly's, text::Bookmarks, ...
    DelFlyInRange( aSttIdx, aEndIdx );
    DeleteRedline( *pSttNd, true, USHRT_MAX );
    _DelBookmarks(aSttIdx, aEndIdx);

    {
        // alle Crsr/StkCrsr/UnoCrsr aus dem Loeschbereich verschieben
        SwNodeIndex aMvStt( aSttIdx, 1 );
        CorrAbs( aMvStt, aEndIdx, SwPosition( aSttIdx ), sal_True );
    }

    GetNodes().DelNodes( aSttIdx, aEndIdx.GetIndex() - aSttIdx.GetIndex() + 1 );
}


void SwDoc::SetModified(SwPaM &rPaM)
{
    SwDataChanged aTmp( rPaM, 0 );
    SetModified();
}

/*************************************************************************
 *                SwDoc::Overwrite()
 ************************************************************************/

bool SwDoc::Overwrite( const SwPaM &rRg, const String &rStr )
{
    SwPosition& rPt = *(SwPosition*)rRg.GetPoint();
    if( pACEWord )                  // Aufnahme in die Autokorrektur
    {
        if( 1 == rStr.Len() )
            pACEWord->CheckChar( rPt, rStr.GetChar( 0 ) );
        delete pACEWord, pACEWord = 0;
    }

    SwTxtNode *pNode = rPt.nNode.GetNode().GetTxtNode();
    if(!pNode)
        return sal_False;

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo(); // AppendUndo not always called
    }

    sal_uInt16 nOldAttrCnt = pNode->GetpSwpHints()
                                ? pNode->GetpSwpHints()->Count() : 0;
    SwDataChanged aTmp( rRg, 0 );
    SwIndex& rIdx = rPt.nContent;
    xub_StrLen nStart = 0;

    sal_Unicode c;
    String aStr;

    sal_Bool bOldExpFlg = pNode->IsIgnoreDontExpand();
    pNode->SetIgnoreDontExpand( sal_True );

    for( xub_StrLen nCnt = 0; nCnt < rStr.Len(); ++nCnt )
    {
        // hinter das Zeichen (zum aufspannen der Attribute !!)
        nStart = rIdx.GetIndex();
        if ( nStart < pNode->GetTxt().Len() )
        {
            lcl_SkipAttr( pNode, rIdx, nStart );
        }
        c = rStr.GetChar( nCnt );
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            bool bMerged(false);
            if (GetIDocumentUndoRedo().DoesGroupUndo())
            {
                SwUndo *const pUndo = GetUndoManager().GetLastUndo();
                SwUndoOverwrite *const pUndoOW(
                    dynamic_cast<SwUndoOverwrite *>(pUndo) );
                if (pUndoOW)
                {
                    // if CanGrouping() returns true it's already merged
                    bMerged = pUndoOW->CanGrouping( this, rPt, c );
                }
            }
            if (!bMerged)
            {
                SwUndo *const pUndoOW( new SwUndoOverwrite(this, rPt, c) );
                GetIDocumentUndoRedo().AppendUndo(pUndoOW);
            }
        }
        else
        {
            // hinter das Zeichen (zum Aufspannen der Attribute !!)
            if( nStart < pNode->GetTxt().Len() )
                rIdx++;
            pNode->InsertText( c, rIdx, INS_EMPTYEXPAND );
            if( nStart+1 < rIdx.GetIndex() )
            {
                rIdx = nStart;
                pNode->EraseText( rIdx, 1 );
                rIdx++;
            }
        }
    }
    pNode->SetIgnoreDontExpand( bOldExpFlg );

    sal_uInt16 nNewAttrCnt = pNode->GetpSwpHints()
                                ? pNode->GetpSwpHints()->Count() : 0;
    if( nOldAttrCnt != nNewAttrCnt )
    {
        SwUpdateAttr aHint( 0, 0, 0 );
        SwClientIter aIter( *pNode );
        SwClient* pGTO = aIter.First(TYPE( SwCrsrShell ));
        while( pGTO )
        {
            pGTO->Modify( 0, &aHint );
            pGTO = aIter.Next();
        }
    }

    if (!GetIDocumentUndoRedo().DoesUndo() &&
        !IsIgnoreRedline() && GetRedlineTbl().Count())
    {
        SwPaM aPam( rPt.nNode, nStart, rPt.nNode, rPt.nContent.GetIndex() );
        DeleteRedline( aPam, true, USHRT_MAX );
    }
    else if( IsRedlineOn() )
    {
        // FIXME: this redline is WRONG: there is no DELETE, and the skipped
        // characters are also included in aPam
        SwPaM aPam( rPt.nNode, nStart, rPt.nNode, rPt.nContent.GetIndex() );
        AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
    }

    SetModified();
    return sal_True;
}


bool SwDoc::MoveAndJoin( SwPaM& rPaM, SwPosition& rPos, SwMoveFlags eMvFlags )
{
    SwNodeIndex aIdx( rPaM.Start()->nNode );
    sal_Bool bJoinTxt = aIdx.GetNode().IsTxtNode();
    sal_Bool bOneNode = rPaM.GetPoint()->nNode == rPaM.GetMark()->nNode;
    aIdx--;             // vor den Move Bereich !!

    bool bRet = MoveRange( rPaM, rPos, eMvFlags );
    if( bRet && !bOneNode )
    {
        if( bJoinTxt )
            aIdx++;
        SwTxtNode * pTxtNd = aIdx.GetNode().GetTxtNode();
        SwNodeIndex aNxtIdx( aIdx );
        if( pTxtNd && pTxtNd->CanJoinNext( &aNxtIdx ) )
        {
            {   // Block wegen SwIndex in den Node !!
                CorrRel( aNxtIdx, SwPosition( aIdx, SwIndex( pTxtNd,
                            pTxtNd->GetTxt().Len() ) ), 0, sal_True );
            }
            pTxtNd->JoinNext();
        }
    }
    return bRet;
}

// mst: it seems that this is mostly used by SwDoc internals; the only
// way to call this from the outside seems to be the special case in
// SwDoc::CopyRange (but i have not managed to actually hit that case)
bool SwDoc::MoveRange( SwPaM& rPaM, SwPosition& rPos, SwMoveFlags eMvFlags )
{
    // keine Moves-Abfangen
    const SwPosition *pStt = rPaM.Start(), *pEnd = rPaM.End();
    if( !rPaM.HasMark() || *pStt >= *pEnd || (*pStt <= rPos && rPos < *pEnd))
        return false;

    // sicher die absatzgebundenen Flys, damit sie verschoben werden koennen.
    _SaveFlyArr aSaveFlyArr;
    _SaveFlyInRange( rPaM, rPos.nNode, aSaveFlyArr, 0 != ( DOC_MOVEALLFLYS & eMvFlags ) );

    // save redlines (if DOC_MOVEREDLINES is used)
    _SaveRedlines aSaveRedl( 0, 4 );
    if( DOC_MOVEREDLINES & eMvFlags && GetRedlineTbl().Count() )
    {
        lcl_SaveRedlines( rPaM, aSaveRedl );

        // #i17764# unfortunately, code below relies on undos being
        //          in a particular order, and presence of bookmarks
        //          will change this order. Hence, we delete bookmarks
        //          here without undo.
        ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());
        _DelBookmarks(
            pStt->nNode,
            pEnd->nNode,
            NULL,
            &pStt->nContent,
            &pEnd->nContent);
    }


    int bUpdateFtn = sal_False;
    SwFtnIdxs aTmpFntIdx;

    // falls Undo eingeschaltet, erzeuge das UndoMove-Objekt
    SwUndoMove * pUndoMove = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo();
        pUndoMove = new SwUndoMove( rPaM, rPos );
        pUndoMove->SetMoveRedlines( eMvFlags == DOC_MOVEREDLINES );
    }
    else
    {
        bUpdateFtn = lcl_SaveFtn( pStt->nNode, pEnd->nNode, rPos.nNode,
                                    GetFtnIdxs(), aTmpFntIdx,
                                    &pStt->nContent, &pEnd->nContent );
    }

    sal_Bool bSplit = sal_False;
    SwPaM aSavePam( rPos, rPos );

    // stelle den SPoint an den Anfang vom Bereich (Definition)
    if( rPaM.GetPoint() == pEnd )
        rPaM.Exchange();

    // in der EditShell wird nach dem Move ein JoinNext erzeugt, wenn
    // vor und nach dem Move ein Text-Node steht.
    SwTxtNode* pSrcNd = rPaM.GetPoint()->nNode.GetNode().GetTxtNode();
    sal_Bool bCorrSavePam = pSrcNd && pStt->nNode != pEnd->nNode;

    // werden ein oder mehr TextNodes bewegt, so wird
    // im SwNodes::Move ein SplitNode erzeugt. Dieser Updated aber nicht
    // den Cursor. Um das zu verhindern, wird hier ein TextNode angelegt,
    // um die Updaterei der Indizies zu erhalten. Nach dem Move wird
    // evt. der Node geloescht.

    SwTxtNode * pTNd = rPos.nNode.GetNode().GetTxtNode();
    if( pTNd && rPaM.GetPoint()->nNode != rPaM.GetMark()->nNode &&
        ( rPos.nContent.GetIndex() || ( pTNd->Len() && bCorrSavePam  )) )
    {
        bSplit = sal_True;
        xub_StrLen nMkCntnt = rPaM.GetMark()->nContent.GetIndex();

        SvULongs aBkmkArr( 15, 15 );
        _SaveCntntIdx( this, rPos.nNode.GetIndex(), rPos.nContent.GetIndex(),
                        aBkmkArr, SAVEFLY_SPLIT );

        pTNd = static_cast<SwTxtNode*>(pTNd->SplitCntntNode( rPos ));

        if( aBkmkArr.Count() )
            _RestoreCntntIdx( this, aBkmkArr, rPos.nNode.GetIndex()-1, 0, sal_True );

        // jetzt noch den Pam berichtigen !!
        if( rPos.nNode == rPaM.GetMark()->nNode )
        {
            rPaM.GetMark()->nNode = rPos.nNode.GetIndex()-1;
            rPaM.GetMark()->nContent.Assign( pTNd, nMkCntnt );
        }
    }

    // setze den Pam um einen "Inhalt" zurueck; dadurch steht er immer
    // ausserhalb des manipulierten Bereiches. Falls kein Inhalt mehr vor-
    // handen, dann auf den StartNode (es ist immer einer vorhanden !!!)
    sal_Bool bNullCntnt = !aSavePam.Move( fnMoveBackward, fnGoCntnt );
    if( bNullCntnt )
    {
        aSavePam.GetPoint()->nNode--;
    }

    // kopiere alle Bookmarks, die im Move Bereich stehen in ein
    // Array, das alle Angaben auf die Position als Offset speichert.
    // Die neue Zuordung erfolgt nach dem Moven.
    ::std::vector< ::sw::mark::SaveBookmark> aSaveBkmks;
    _DelBookmarks(
        pStt->nNode,
        pEnd->nNode,
        &aSaveBkmks,
        &pStt->nContent,
        &pEnd->nContent);

    // falls durch die vorherigen Loeschungen (z.B. der Fussnoten) kein
    // Bereich mehr existiert, ist das immernoch ein gueltiger Move!
    if( *rPaM.GetPoint() != *rPaM.GetMark() )
    {
        // now do the actual move
        GetNodes().MoveRange( rPaM, rPos, GetNodes() );

        // after a MoveRange() the Mark is deleted
        if ( rPaM.HasMark() ) // => no Move occurred!
        {
            delete pUndoMove;
            return false;
        }
    }
    else
        rPaM.DeleteMark();

    ASSERT( *aSavePam.GetMark() == rPos ||
            ( aSavePam.GetMark()->nNode.GetNode().GetCntntNode() == NULL ),
            "PaM wurde nicht verschoben, am Anfang/Ende keine ContentNodes?" );
    *aSavePam.GetMark() = rPos;

    rPaM.SetMark();         // um den neuen Bereich eine Sel. aufspannen
    pTNd = aSavePam.GetNode()->GetTxtNode();
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        // korrigiere erstmal den Content vom SavePam
        if( bNullCntnt )
        {
            aSavePam.GetPoint()->nContent = 0;
        }

        // die Methode SwEditShell::Move() fuegt nach dem Move den Text-Node
        // zusammen, in dem der rPaM steht. Wurde der Inhalt nach hinten
        // geschoben und liegt der SPoint vom SavePam im naechsten Node, so
        // muss beim Speichern vom Undo-Object das beachtet werden !!
        SwTxtNode * pPamTxtNd = 0;

        // wird ans SwUndoMove weitergegeben, das dann beim Undo JoinNext
        // aufruft. (falls es hier nicht moeglich ist).
        sal_Bool bJoin = bSplit && pTNd;
        bCorrSavePam = bCorrSavePam &&
                        0 != ( pPamTxtNd = rPaM.GetNode()->GetTxtNode() )
                        && pPamTxtNd->CanJoinNext()
                        && (*rPaM.GetPoint() <= *aSavePam.GetPoint());

        // muessen am SavePam 2 Nodes zusammengefasst werden ??
        if( bJoin && pTNd->CanJoinNext() )
        {
            pTNd->JoinNext();
            // kein temp. sdbcx::Index bei &&
            // es sollten wohl nur die Indexwerte verglichen werden.
            if( bCorrSavePam && rPaM.GetPoint()->nNode.GetIndex()+1 ==
                                aSavePam.GetPoint()->nNode.GetIndex() )
            {
                aSavePam.GetPoint()->nContent += pPamTxtNd->Len();
            }
            bJoin = sal_False;
        }
//      else if( !bCorrSavePam && !pSavePam->Move( fnMoveForward, fnGoCntnt ))
        else if ( !aSavePam.Move( fnMoveForward, fnGoCntnt ) )
        {
            aSavePam.GetPoint()->nNode++;
        }

        // zwischen SPoint und GetMark steht jetzt der neu eingefuegte Bereich
        pUndoMove->SetDestRange( aSavePam, *rPaM.GetPoint(),
                                    bJoin, bCorrSavePam );
        GetIDocumentUndoRedo().AppendUndo( pUndoMove );
    }
    else
    {
        bool bRemove = true;
        // muessen am SavePam 2 Nodes zusammengefasst werden ??
        if( bSplit && pTNd )
        {
            if( pTNd->CanJoinNext())
            {
                // --> OD 2009-08-20 #i100466#
                // Always join next, because <pTNd> has to stay as it is.
                // A join previous from its next would more or less delete <pTNd>
                pTNd->JoinNext();
                // <--
                bRemove = false;
            }
        }
        if( bNullCntnt )
        {
            aSavePam.GetPoint()->nNode++;
            aSavePam.GetPoint()->nContent.Assign( aSavePam.GetCntntNode(), 0 );
        }
        else if( bRemove ) // No move forward after joining with next paragraph
        {
            aSavePam.Move( fnMoveForward, fnGoCntnt );
        }
    }

    // setze jetzt wieder die text::Bookmarks in das Dokument
    *rPaM.GetMark() = *aSavePam.Start();
    for(
        ::std::vector< ::sw::mark::SaveBookmark>::iterator pBkmk = aSaveBkmks.begin();
        pBkmk != aSaveBkmks.end();
        ++pBkmk)
        pBkmk->SetInDoc(
            this,
            rPaM.GetMark()->nNode,
            &rPaM.GetMark()->nContent);
    *rPaM.GetPoint() = *aSavePam.End();

    // verschiebe die Flys an die neue Position
    _RestFlyInRange( aSaveFlyArr, rPaM.Start()->nNode, &(rPos.nNode) );

    // restore redlines (if DOC_MOVEREDLINES is used)
    if( aSaveRedl.Count() )
    {
        lcl_RestoreRedlines( this, *aSavePam.Start(), aSaveRedl );
    }

    if( bUpdateFtn )
    {
        if( aTmpFntIdx.Count() )
        {
            GetFtnIdxs().Insert( &aTmpFntIdx );
            aTmpFntIdx.Remove( sal_uInt16( 0 ), aTmpFntIdx.Count() );
        }

        GetFtnIdxs().UpdateAllFtn();
    }

    SetModified();
    return true;
}

bool SwDoc::MoveNodeRange( SwNodeRange& rRange, SwNodeIndex& rPos,
        SwMoveFlags eMvFlags )
{
    // bewegt alle Nodes an die neue Position. Dabei werden die
    // text::Bookmarks mit verschoben !! (zur Zeit ohne Undo)

    // falls durchs Move Fussnoten in den Sonderbereich kommen sollten,
    // dann entferne sie jetzt.
    //JP 13.07.95:
    // ansonsten bei allen Fussnoten, die verschoben werden, die Frames
    // loeschen und nach dem Move wieder aufbauen lassen (Fussnoten koennen
    // die Seite wechseln). Zusaetzlich muss natuerlich die Sortierung
    // der FtnIdx-Array wieder korrigiert werden.

    int bUpdateFtn = sal_False;
    SwFtnIdxs aTmpFntIdx;

    SwUndoMove* pUndo = 0;
    if ((DOC_CREATEUNDOOBJ & eMvFlags ) && GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoMove( this, rRange, rPos );
    }
    else
    {
        bUpdateFtn = lcl_SaveFtn( rRange.aStart, rRange.aEnd, rPos,
                                    GetFtnIdxs(), aTmpFntIdx );
    }

    _SaveRedlines aSaveRedl( 0, 4 );
    SvPtrarr aSavRedlInsPosArr( 0, 4 );
    if( DOC_MOVEREDLINES & eMvFlags && GetRedlineTbl().Count() )
    {
        lcl_SaveRedlines( rRange, aSaveRedl );

        // suche alle Redlines, die an der InsPos aufhoeren. Diese muessen
        // nach dem Move wieder an die "alte" Position verschoben werden
        sal_uInt16 nRedlPos = GetRedlinePos( rPos.GetNode(), USHRT_MAX );
        if( USHRT_MAX != nRedlPos )
        {
            const SwPosition *pRStt, *pREnd;
            do {
                SwRedline* pTmp = GetRedlineTbl()[ nRedlPos ];
                pRStt = pTmp->Start();
                pREnd = pTmp->End();
                if( pREnd->nNode == rPos && pRStt->nNode < rPos )
                {
                    void* p = pTmp;
                    aSavRedlInsPosArr.Insert( p, aSavRedlInsPosArr.Count() );
                }
            } while( pRStt->nNode < rPos && ++nRedlPos < GetRedlineTbl().Count());
        }
    }

    // kopiere alle Bookmarks, die im Move Bereich stehen in ein
    // Array, das alle Angaben auf die Position als Offset speichert.
    // Die neue Zuordung erfolgt nach dem Moven.
    ::std::vector< ::sw::mark::SaveBookmark> aSaveBkmks;
    _DelBookmarks(rRange.aStart, rRange.aEnd, &aSaveBkmks);

    // sicher die absatzgebundenen Flys, damit verschoben werden koennen.
    _SaveFlyArr aSaveFlyArr;
    if( GetSpzFrmFmts()->Count() )
        _SaveFlyInRange( rRange, aSaveFlyArr );

    // vor die Position setzen, damit er nicht weitergeschoben wird
    SwNodeIndex aIdx( rPos, -1 );

    SwNodeIndex* pSaveInsPos = 0;
    if( pUndo )
        pSaveInsPos = new SwNodeIndex( rRange.aStart, -1 );

    // verschiebe die Nodes
    sal_Bool bNoDelFrms = 0 != (DOC_NO_DELFRMS & eMvFlags);
    if( GetNodes()._MoveNodes( rRange, GetNodes(), rPos, !bNoDelFrms ) )
    {
        aIdx++;     // wieder auf alte Position
        if( pSaveInsPos )
            (*pSaveInsPos)++;
    }
    else
    {
        aIdx = rRange.aStart;
        delete pUndo, pUndo = 0;
    }

    // verschiebe die Flys an die neue Position
    if( aSaveFlyArr.Count() )
        _RestFlyInRange( aSaveFlyArr, aIdx, NULL );

    // setze jetzt wieder die text::Bookmarks in das Dokument
    for(
        ::std::vector< ::sw::mark::SaveBookmark>::iterator pBkmk = aSaveBkmks.begin();
        pBkmk != aSaveBkmks.end();
        ++pBkmk)
        pBkmk->SetInDoc(this, aIdx);

    if( aSavRedlInsPosArr.Count() )
    {
        SwNode* pNewNd = &aIdx.GetNode();
        for( sal_uInt16 n = 0; n < aSavRedlInsPosArr.Count(); ++n )
        {
            SwRedline* pTmp = (SwRedline*)aSavRedlInsPosArr[ n ];
            if( USHRT_MAX != GetRedlineTbl().GetPos( pTmp ) )
            {
                SwPosition* pEnd = pTmp->End();
                pEnd->nNode = aIdx;
                pEnd->nContent.Assign( pNewNd->GetCntntNode(), 0 );
            }
        }
    }

    if( aSaveRedl.Count() )
        lcl_RestoreRedlines( this, aIdx.GetIndex(), aSaveRedl );

    if( pUndo )
    {
        pUndo->SetDestRange( aIdx, rPos, *pSaveInsPos );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    if( pSaveInsPos )
        delete pSaveInsPos;

    if( bUpdateFtn )
    {
        if( aTmpFntIdx.Count() )
        {
            GetFtnIdxs().Insert( &aTmpFntIdx );
            aTmpFntIdx.Remove( sal_uInt16( 0 ), aTmpFntIdx.Count() );
        }

        GetFtnIdxs().UpdateAllFtn();
    }

    SetModified();
    return sal_True;
}

/* #107318# Convert list of ranges of whichIds to a corresponding list
    of whichIds*/
SvUShorts * lcl_RangesToUShorts(sal_uInt16 * pRanges)
{
    SvUShorts * pResult = new SvUShorts();

    int i = 0;
    while (pRanges[i] != 0)
    {
        ASSERT(pRanges[i+1] != 0, "malformed ranges");

        for (sal_uInt16 j = pRanges[i]; j < pRanges[i+1]; j++)
            pResult->Insert(j, pResult->Count());

        i += 2;
    }

    return pResult;
}

bool lcl_StrLenOverFlow( const SwPaM& rPam )
{
    // If we try to merge two paragraph we have to test if afterwards
    // the string doesn't exceed the allowed string length
    bool bRet = false;
    if( rPam.GetPoint()->nNode != rPam.GetMark()->nNode )
    {
        const SwPosition* pStt = rPam.Start(), *pEnd = rPam.End();
        const SwTxtNode* pEndNd = pEnd->nNode.GetNode().GetTxtNode();
        if( (0 != pEndNd) && pStt->nNode.GetNode().IsTxtNode() )
        {
            sal_uInt64 nSum = pStt->nContent.GetIndex() +
                pEndNd->GetTxt().Len() - pEnd->nContent.GetIndex();
            if( nSum > STRING_LEN )
                bRet = true;
        }
    }
    return bRet;
}

void lcl_GetJoinFlags( SwPaM& rPam, sal_Bool& rJoinTxt, sal_Bool& rJoinPrev )
{
    rJoinTxt = sal_False;
    rJoinPrev = sal_False;
    if( rPam.GetPoint()->nNode != rPam.GetMark()->nNode )
    {
        const SwPosition* pStt = rPam.Start(), *pEnd = rPam.End();
        SwTxtNode *pSttNd = pStt->nNode.GetNode().GetTxtNode();
        if( pSttNd )
        {
            SwTxtNode *pEndNd = pEnd->nNode.GetNode().GetTxtNode();
            rJoinTxt = 0 != pEndNd;
            if( rJoinTxt )
            {
                bool bExchange = pStt == rPam.GetPoint();
                if( !pStt->nContent.GetIndex() &&
                    pEndNd->GetTxt().Len() != pEnd->nContent.GetIndex() )
                    bExchange = !bExchange;
                if( bExchange )
                    rPam.Exchange();
                rJoinPrev = rPam.GetPoint() == pStt;
                ASSERT( !pStt->nContent.GetIndex() &&
                    pEndNd->GetTxt().Len() != pEnd->nContent.GetIndex()
                    ? rPam.GetPoint()->nNode < rPam.GetMark()->nNode
                    : rPam.GetPoint()->nNode > rPam.GetMark()->nNode,
                    "lcl_GetJoinFlags");
            }
        }
    }
}

void lcl_JoinText( SwPaM& rPam, sal_Bool bJoinPrev )
{
    SwNodeIndex aIdx( rPam.GetPoint()->nNode );
    SwTxtNode *pTxtNd = aIdx.GetNode().GetTxtNode();
    SwNodeIndex aOldIdx( aIdx );
    SwTxtNode *pOldTxtNd = pTxtNd;

    if( pTxtNd && pTxtNd->CanJoinNext( &aIdx ) )
    {
        SwDoc* pDoc = rPam.GetDoc();
        if( bJoinPrev )
        {
            // N.B.: we do not need to handle xmlids in this case, because
            // it is only invoked if one paragraph is completely empty
            // (see lcl_GetJoinFlags)
            {
                // falls PageBreaks geloescht / gesetzt werden, darf das
                // nicht in die Undo-History aufgenommen werden !!
                // (das loeschen vom Node geht auch am Undo vorbei !!!)
                ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

                /* PageBreaks, PageDesc, ColumnBreaks */
                // Sollte an der Logik zum Kopieren der PageBreak's ...
                // etwas geaendert werden, muss es auch im SwUndoDelete
                // geandert werden. Dort wird sich das AUTO-PageBreak
                // aus dem GetMarkNode kopiert.!!!

                /* Der GetMarkNode */
                if( ( pTxtNd = aIdx.GetNode().GetTxtNode())->HasSwAttrSet() )
                {
                    const SfxPoolItem* pItem;
                    if( SFX_ITEM_SET == pTxtNd->GetpSwAttrSet()->GetItemState(
                        RES_BREAK, sal_False, &pItem ) )
                        pTxtNd->ResetAttr( RES_BREAK );
                    if( pTxtNd->HasSwAttrSet() &&
                        SFX_ITEM_SET == pTxtNd->GetpSwAttrSet()->GetItemState(
                        RES_PAGEDESC, sal_False, &pItem ) )
                        pTxtNd->ResetAttr( RES_PAGEDESC );
                }

                /* Der PointNode */
                if( pOldTxtNd->HasSwAttrSet() )
                {
                    const SfxPoolItem* pItem;
                    SfxItemSet aSet( pDoc->GetAttrPool(), aBreakSetRange );
                    const SfxItemSet* pSet = pOldTxtNd->GetpSwAttrSet();
                    if( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK,
                        sal_False, &pItem ) )
                        aSet.Put( *pItem );
                    if( SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC,
                        sal_False, &pItem ) )
                        aSet.Put( *pItem );
                    if( aSet.Count() )
                        pTxtNd->SetAttr( aSet );
                }
                pOldTxtNd->FmtToTxtAttr( pTxtNd );

                SvULongs aBkmkArr( 15, 15 );
                ::_SaveCntntIdx( pDoc, aOldIdx.GetIndex(),
                                    pOldTxtNd->Len(), aBkmkArr );

                SwIndex aAlphaIdx(pTxtNd);
                pOldTxtNd->CutText( pTxtNd, aAlphaIdx, SwIndex(pOldTxtNd),
                                    pOldTxtNd->Len() );
                SwPosition aAlphaPos( aIdx, aAlphaIdx );
                pDoc->CorrRel( rPam.GetPoint()->nNode, aAlphaPos, 0, sal_True );

                // verschiebe noch alle Bookmarks/TOXMarks
                if( aBkmkArr.Count() )
                    ::_RestoreCntntIdx( pDoc, aBkmkArr, aIdx.GetIndex() );

                // falls der uebergebene PaM nicht im Crsr-Ring steht,
                // gesondert behandeln (z.B. Aufruf aus dem Auto-Format)
                if( pOldTxtNd == rPam.GetBound( sal_True ).nContent.GetIdxReg() )
                    rPam.GetBound( sal_True ) = aAlphaPos;
                if( pOldTxtNd == rPam.GetBound( sal_False ).nContent.GetIdxReg() )
                    rPam.GetBound( sal_False ) = aAlphaPos;
            }
            // jetzt nur noch den Node loeschen
            pDoc->GetNodes().Delete( aOldIdx, 1 );
        }
        else
        {
            SwTxtNode* pDelNd = aIdx.GetNode().GetTxtNode();
            if( pTxtNd->Len() )
                pDelNd->FmtToTxtAttr( pTxtNd );
            else
            {
                /* #107318# This case was missed:

                   <something></something>   <-- pTxtNd
                   <other>ccc</other>        <-- pDelNd

                   <something> and <other> are paragraph
                   attributes. The attribute <something> stayed if not
                   overwritten by an attribute in "ccc". Fixed by
                   first resetting all character attributes in first
                   paragraph (pTxtNd).
                */
                SvUShorts * pShorts =
                    lcl_RangesToUShorts(aCharFmtSetRange);
                pTxtNd->ResetAttr(*pShorts);
                delete pShorts;

                if( pDelNd->HasSwAttrSet() )
                {
                    // nur die Zeichenattribute kopieren
                    SfxItemSet aTmpSet( pDoc->GetAttrPool(), aCharFmtSetRange );
                    aTmpSet.Put( *pDelNd->GetpSwAttrSet() );
                    pTxtNd->SetAttr( aTmpSet );
                }
            }

            pDoc->CorrRel( aIdx, *rPam.GetPoint(), 0, sal_True );
            // --> OD 2009-08-20 #i100466#
            // adjust given <rPam>, if it does not belong to the cursors
            if ( pDelNd == rPam.GetBound( sal_True ).nContent.GetIdxReg() )
            {
                rPam.GetBound( sal_True ) = SwPosition( SwNodeIndex( *pTxtNd ), SwIndex( pTxtNd ) );
            }
            if( pDelNd == rPam.GetBound( sal_False ).nContent.GetIdxReg() )
            {
                rPam.GetBound( sal_False ) = SwPosition( SwNodeIndex( *pTxtNd ), SwIndex( pTxtNd ) );
            }
            // <--
            pTxtNd->JoinNext();
        }
    }
}

static void
lcl_CalcBreaks( ::std::vector<xub_StrLen> & rBreaks, SwPaM const & rPam )
{
    SwTxtNode const * const pTxtNode(
            rPam.End()->nNode.GetNode().GetTxtNode() );
    if (!pTxtNode)
        return; // left-overlap only possible at end of selection...

    const xub_StrLen nStart(rPam.Start()->nContent.GetIndex());
    const xub_StrLen nEnd  (rPam.End  ()->nContent.GetIndex());
    if (nEnd == pTxtNode->Len())
        return; // paragraph selected until the end

    for (xub_StrLen i = nStart; i < nEnd; ++i)
    {
        const sal_Unicode c(pTxtNode->GetTxt().GetChar(i));
        if ((CH_TXTATR_INWORD == c) || (CH_TXTATR_BREAKWORD == c))
        {
            SwTxtAttr const * const pAttr( pTxtNode->GetTxtAttrForCharAt(i) );
            if (pAttr && pAttr->GetEnd() && (*pAttr->GetEnd() > nEnd))
            {
                ASSERT(pAttr->HasDummyChar(), "GetTxtAttrForCharAt broken?");
                rBreaks.push_back(i);
            }
        }
    }
}

bool lcl_DoWithBreaks(SwDoc & rDoc, SwPaM & rPam,
        bool (SwDoc::*pFunc)(SwPaM&, bool), const bool bForceJoinNext = false)
{
    ::std::vector<xub_StrLen> Breaks;

    lcl_CalcBreaks(Breaks, rPam);

    if (!Breaks.size())
    {
        return (rDoc.*pFunc)(rPam, bForceJoinNext);
    }

    // N.B.: deletion must be split into several parts if the text node
    // contains a text attribute with end and with dummy character
    // and the selection does not contain the text attribute completely,
    // but overlaps its start (left), where the dummy character is.

    SwPosition const & rSelectionEnd( *rPam.End() );

    bool bRet( true );
    // iterate from end to start, to avoid invalidating the offsets!
    ::std::vector<xub_StrLen>::reverse_iterator iter( Breaks.rbegin() );
    SwPaM aPam( rSelectionEnd, rSelectionEnd ); // end node!
    SwPosition & rEnd( *aPam.End() );
    SwPosition & rStart( *aPam.Start() );

    while (iter != Breaks.rend())
    {
        rStart.nContent = *iter + 1;
        if (rEnd.nContent > rStart.nContent) // check if part is empty
        {
            bRet &= (rDoc.*pFunc)(aPam, bForceJoinNext);
        }
        rEnd.nContent = *iter;
        ++iter;
    }

    rStart = *rPam.Start(); // set to original start
    if (rEnd.nContent > rStart.nContent) // check if part is empty
    {
        bRet &= (rDoc.*pFunc)(aPam, bForceJoinNext);
    }

    return bRet;
}


bool SwDoc::DeleteAndJoinWithRedlineImpl( SwPaM & rPam, const bool )
{
    ASSERT( IsRedlineOn(), "DeleteAndJoinWithRedline: redline off" );

    {
        SwUndoRedlineDelete* pUndo = 0;
        RedlineMode_t eOld = GetRedlineMode();
        checkRedlining(eOld);
        if (GetIDocumentUndoRedo().DoesUndo())
        {

    //JP 06.01.98: MUSS noch optimiert werden!!!
    SetRedlineMode(
           (RedlineMode_t)(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE ));

            GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);
            pUndo = new SwUndoRedlineDelete( rPam, UNDO_DELETE );
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
        if( *rPam.GetPoint() != *rPam.GetMark() )
            AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_DELETE, rPam ), true);
        SetModified();

        if( pUndo )
        {
            GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);
            // ??? why the hell is the AppendUndo not below the
            // CanGrouping, so this hideous cleanup wouldn't be necessary?
            // bah, this is redlining, probably changing this would break it...
            if (GetIDocumentUndoRedo().DoesGroupUndo())
            {
                SwUndo *const pLastUndo( GetUndoManager().GetLastUndo() );
                SwUndoRedlineDelete *const pUndoRedlineDel(
                        dynamic_cast<SwUndoRedlineDelete*>(pLastUndo) );
                if (pUndoRedlineDel)
                {
                    bool const bMerged = pUndoRedlineDel->CanGrouping(*pUndo);
                    if (bMerged)
                    {
                        ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());
                        SwUndo const*const pDeleted =
                            GetUndoManager().RemoveLastUndo();
                        OSL_ENSURE(pDeleted == pUndo,
                            "DeleteAndJoinWithRedlineImpl: "
                            "undo removed is not undo inserted?");
                        delete pDeleted;
                    }
                }
            }
//JP 06.01.98: MUSS noch optimiert werden!!!
SetRedlineMode( eOld );
        }
        return true;
    }
}

bool SwDoc::DeleteAndJoinImpl( SwPaM & rPam,
                               const bool bForceJoinNext )
{
    sal_Bool bJoinTxt, bJoinPrev;
    lcl_GetJoinFlags( rPam, bJoinTxt, bJoinPrev );
    // --> OD 2009-08-20 #i100466#
    if ( bForceJoinNext )
    {
        bJoinPrev = sal_False;
    }
    // <--
    {
        bool const bSuccess( DeleteRangeImpl( rPam ) );
        if (!bSuccess)
            return false;
    }

    if( bJoinTxt )
    {
        lcl_JoinText( rPam, bJoinPrev );
    }

    return true;
}

bool SwDoc::DeleteRangeImpl(SwPaM & rPam, const bool)
{
    // move all cursors out of the deleted range.
    // but first copy the given PaM, because it could be a cursor that
    // would be moved!
    SwPaM aDelPam( *rPam.GetMark(), *rPam.GetPoint() );
    ::PaMCorrAbs( aDelPam, *aDelPam.GetPoint() );

    bool const bSuccess( DeleteRangeImplImpl( aDelPam ) );
    if (bSuccess)
    {   // now copy position from temp copy to given PaM
        *rPam.GetPoint() = *aDelPam.GetPoint();
    }

    return bSuccess;
}

bool SwDoc::DeleteRangeImplImpl(SwPaM & rPam)
{
    SwPosition *pStt = (SwPosition*)rPam.Start(), *pEnd = (SwPosition*)rPam.End();

    if( !rPam.HasMark() || *pStt >= *pEnd )
        return false;

    if( pACEWord )
    {
        // ggfs. das gesicherte Word fuer die Ausnahme
        if( pACEWord->IsDeleted() ||  pStt->nNode != pEnd->nNode ||
            pStt->nContent.GetIndex() + 1 != pEnd->nContent.GetIndex() ||
            !pACEWord->CheckDelChar( *pStt ))
            delete pACEWord, pACEWord = 0;
    }

    {
        // loesche alle leeren TextHints an der Mark-Position
        SwTxtNode* pTxtNd = rPam.GetMark()->nNode.GetNode().GetTxtNode();
        SwpHints* pHts;
        if( pTxtNd &&  0 != ( pHts = pTxtNd->GetpSwpHints()) && pHts->Count() )
        {
            const xub_StrLen *pEndIdx;
            xub_StrLen nMkCntPos = rPam.GetMark()->nContent.GetIndex();
            for( sal_uInt16 n = pHts->Count(); n; )
            {
                const SwTxtAttr* pAttr = (*pHts)[ --n ];
                if( nMkCntPos > *pAttr->GetStart() )
                    break;

                if( nMkCntPos == *pAttr->GetStart() &&
                    0 != (pEndIdx = pAttr->GetEnd()) &&
                    *pEndIdx == *pAttr->GetStart() )
                    pTxtNd->DestroyAttr( pHts->Cut( n ) );
            }
        }
    }

    {
        // Bug 26675:   DataChanged vorm loeschen verschicken, dann bekommt
        //          man noch mit, welche Objecte sich im Bereich befinden.
        //          Danach koennen sie vor/hinter der Position befinden.
        SwDataChanged aTmp( rPam, 0 );
    }


    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo();
        bool bMerged(false);
        if (GetIDocumentUndoRedo().DoesGroupUndo())
        {
            SwUndo *const pLastUndo( GetUndoManager().GetLastUndo() );
            SwUndoDelete *const pUndoDelete(
                    dynamic_cast<SwUndoDelete *>(pLastUndo) );
            if (pUndoDelete)
            {
                bMerged = pUndoDelete->CanGrouping( this, rPam );
                // if CanGrouping() returns true it's already merged
            }
        }
        if (!bMerged)
        {
            GetIDocumentUndoRedo().AppendUndo( new SwUndoDelete( rPam ) );
        }

        SetModified();

        return true;
    }

    if( !IsIgnoreRedline() && GetRedlineTbl().Count() )
        DeleteRedline( rPam, true, USHRT_MAX );

    // loesche und verschiebe erstmal alle "Fly's am Absatz", die in der
    // Selection liegen
    DelFlyInRange(rPam.GetMark()->nNode, rPam.GetPoint()->nNode);
    _DelBookmarks(
        pStt->nNode,
        pEnd->nNode,
        NULL,
        &pStt->nContent,
        &pEnd->nContent);

    SwNodeIndex aSttIdx( pStt->nNode );
    SwCntntNode * pCNd = aSttIdx.GetNode().GetCntntNode();

    do {        // middle checked loop!
        if( pCNd )
        {
            SwTxtNode * pStartTxtNode( pCNd->GetTxtNode() );
            if ( pStartTxtNode )
            {
                // verschiebe jetzt noch den Inhalt in den neuen Node
                sal_Bool bOneNd = pStt->nNode == pEnd->nNode;
                xub_StrLen nLen = ( bOneNd ? pEnd->nContent.GetIndex()
                                           : pCNd->Len() )
                                        - pStt->nContent.GetIndex();

                // falls schon leer, dann nicht noch aufrufen
                if( nLen )
                {
                    pStartTxtNode->EraseText( pStt->nContent, nLen );

                    if( !pStartTxtNode->Len() )
                    {
                // METADATA: remove reference if empty (consider node deleted)
                        pStartTxtNode->RemoveMetadataReference();
                    }
                }

                if( bOneNd )        // das wars schon
                    break;

                aSttIdx++;
            }
            else
            {
                // damit beim loeschen keine Indizies mehr angemeldet sind,
                // wird hier der SwPaM aus dem Content entfernt !!
                pStt->nContent.Assign( 0, 0 );
            }
        }

        pCNd = pEnd->nNode.GetNode().GetCntntNode();
        if( pCNd )
        {
            SwTxtNode * pEndTxtNode( pCNd->GetTxtNode() );
            if( pEndTxtNode )
            {
                // falls schon leer, dann nicht noch aufrufen
                if( pEnd->nContent.GetIndex() )
                {
                    SwIndex aIdx( pCNd, 0 );
                    pEndTxtNode->EraseText( aIdx, pEnd->nContent.GetIndex() );

                    if( !pEndTxtNode->Len() )
                    {
                // METADATA: remove reference if empty (consider node deleted)
                        pEndTxtNode->RemoveMetadataReference();
                    }
                }
            }
            else
            {
                // damit beim Loeschen keine Indizies mehr angemeldet sind,
                // wird hier der SwPaM aus dem Content entfernt !!
                pEnd->nContent.Assign( 0, 0 );
            }
        }

        // if the end is not a content node, delete it as well
        sal_uInt32 nEnde = pEnd->nNode.GetIndex();
        if( pCNd == NULL )
            nEnde++;

        if( aSttIdx != nEnde )
        {
            // loesche jetzt die Nodes in das NodesArary
            GetNodes().Delete( aSttIdx, nEnde - aSttIdx.GetIndex() );
        }

        // falls der Node geloescht wurde, in dem der Cursor stand, so
        // muss der Content im akt. Content angemeldet werden !!!
        pStt->nContent.Assign( pStt->nNode.GetNode().GetCntntNode(),
                                pStt->nContent.GetIndex() );

        // der PaM wird korrigiert, denn falls ueber Nodegrenzen geloescht
        // wurde, so stehen sie in unterschieden Nodes. Auch die Selektion
        // wird aufgehoben !
        *pEnd = *pStt;
        rPam.DeleteMark();

    } while( sal_False );

    if( !IsIgnoreRedline() && GetRedlineTbl().Count() )
        CompressRedlines();
    SetModified();

    return true;
}

// OD 2009-08-20 #i100466#
// Add handling of new optional parameter <bForceJoinNext>
bool SwDoc::DeleteAndJoin( SwPaM & rPam,
                           const bool bForceJoinNext )
{
    if ( lcl_StrLenOverFlow( rPam ) )
        return false;

    return lcl_DoWithBreaks( *this, rPam, (IsRedlineOn())
                ? &SwDoc::DeleteAndJoinWithRedlineImpl
                : &SwDoc::DeleteAndJoinImpl,
                bForceJoinNext );
}

bool SwDoc::DeleteRange( SwPaM & rPam )
{
    return lcl_DoWithBreaks( *this, rPam, &SwDoc::DeleteRangeImpl );
}


void lcl_syncGrammarError( SwTxtNode &rTxtNode, linguistic2::ProofreadingResult& rResult,
    xub_StrLen /*nBeginGrammarCheck*/, const ModelToViewHelper::ConversionMap* pConversionMap )
{
    if( rTxtNode.IsGrammarCheckDirty() )
        return;
    SwGrammarMarkUp* pWrong = rTxtNode.GetGrammarCheck();
    linguistic2::SingleProofreadingError* pArray = rResult.aErrors.getArray();
    sal_uInt16 i, j = 0;
    if( pWrong )
    {
        for( i = 0; i < rResult.aErrors.getLength(); ++i )
        {
            const linguistic2::SingleProofreadingError &rError = rResult.aErrors[i];
            xub_StrLen nStart = (xub_StrLen)ModelToViewHelper::ConvertToModelPosition( pConversionMap, rError.nErrorStart ).mnPos;
            xub_StrLen nEnd = (xub_StrLen)ModelToViewHelper::ConvertToModelPosition( pConversionMap, rError.nErrorStart + rError.nErrorLength ).mnPos;
            if( i != j )
                pArray[j] = pArray[i];
            if( pWrong->LookForEntry( nStart, nEnd ) )
                ++j;
        }
    }
    if( rResult.aErrors.getLength() > j )
        rResult.aErrors.realloc( j );
}


uno::Any SwDoc::Spell( SwPaM& rPaM,
                    uno::Reference< XSpellChecker1 >  &xSpeller,
                    sal_uInt16* pPageCnt, sal_uInt16* pPageSt,
                    bool bGrammarCheck,
                    SwConversionArgs *pConvArgs  ) const
{
    SwPosition* pSttPos = rPaM.Start(), *pEndPos = rPaM.End();
    uno::Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );

    SwSpellArgs      *pSpellArgs = 0;
    //SwConversionArgs *pConvArgs  = 0;
    if (pConvArgs)
    {
        pConvArgs->SetStart(pSttPos->nNode.GetNode().GetTxtNode(), pSttPos->nContent);
        pConvArgs->SetEnd(  pEndPos->nNode.GetNode().GetTxtNode(), pEndPos->nContent );
    }
    else
        pSpellArgs = new SwSpellArgs( xSpeller,
                            pSttPos->nNode.GetNode().GetTxtNode(), pSttPos->nContent,
                            pEndPos->nNode.GetNode().GetTxtNode(), pEndPos->nContent,
                            bGrammarCheck );

    sal_uLong nCurrNd = pSttPos->nNode.GetIndex();
    sal_uLong nEndNd = pEndPos->nNode.GetIndex();

    uno::Any aRet;
    if( nCurrNd <= nEndNd )
    {
        SwCntntFrm* pCntFrm;
        sal_Bool bGoOn = sal_True;
        while( bGoOn )
        {
            SwNode* pNd = GetNodes()[ nCurrNd ];
            switch( pNd->GetNodeType() )
            {
            case ND_TEXTNODE:
                if( 0 != ( pCntFrm = ((SwTxtNode*)pNd)->GetFrm()) )
                {
                    // geschutze Cellen/Flys ueberspringen, ausgeblendete
                    //ebenfalls
                    if( pCntFrm->IsProtected() )
                    {
                        nCurrNd = pNd->EndOfSectionIndex();
                    }
                    else if( !((SwTxtFrm*)pCntFrm)->IsHiddenNow() )
                    {
                        if( pPageCnt && *pPageCnt && pPageSt )
                        {
                            sal_uInt16 nPageNr = pCntFrm->GetPhyPageNum();
                            if( !*pPageSt )
                            {
                                *pPageSt = nPageNr;
                                if( *pPageCnt < *pPageSt )
                                    *pPageCnt = *pPageSt;
                            }
                            long nStat;
                            if( nPageNr >= *pPageSt )
                                nStat = nPageNr - *pPageSt + 1;
                            else
                                nStat = nPageNr + *pPageCnt - *pPageSt + 1;
                            ::SetProgressState( nStat, (SwDocShell*)GetDocShell() );
                        }
                        //Spell() changes the pSpellArgs in case an error is found
                        xub_StrLen nBeginGrammarCheck = 0;
                        xub_StrLen nEndGrammarCheck = 0;
                        if( pSpellArgs && pSpellArgs->bIsGrammarCheck)
                        {
                            nBeginGrammarCheck = pSpellArgs->pStartNode == pNd ?  pSpellArgs->pStartIdx->GetIndex() : 0;
                            // if grammar checking starts inside of a sentence the start position has to be adjusted
                            if( nBeginGrammarCheck )
                            {
                                SwIndex aStartIndex( dynamic_cast< SwTxtNode* >( pNd ), nBeginGrammarCheck );
                                SwPosition aStart( *pNd, aStartIndex );
                                SwCursor aCrsr(aStart, 0, false);
                                SwPosition aOrigPos = *aCrsr.GetPoint();
                                aCrsr.GoSentence( SwCursor::START_SENT );
                                if( aOrigPos != *aCrsr.GetPoint() )
                                {
                                    nBeginGrammarCheck = aCrsr.GetPoint()->nContent.GetIndex();
                                }
                            }
                            nEndGrammarCheck = pSpellArgs->pEndNode == pNd ? pSpellArgs->pEndIdx->GetIndex() : ((SwTxtNode*)pNd)->GetTxt().Len();
                        }

                        xub_StrLen nSpellErrorPosition = ((SwTxtNode*)pNd)->GetTxt().Len();
                        if( (!pConvArgs &&
                                ((SwTxtNode*)pNd)->Spell( pSpellArgs )) ||
                            ( pConvArgs &&
                                ((SwTxtNode*)pNd)->Convert( *pConvArgs )))
                        {
                            // Abbrechen und Position merken
                            pSttPos->nNode = nCurrNd;
                            pEndPos->nNode = nCurrNd;
                            nCurrNd = nEndNd;
                            if( pSpellArgs )
                                nSpellErrorPosition = pSpellArgs->pStartIdx->GetIndex() > pSpellArgs->pEndIdx->GetIndex() ?
                                            pSpellArgs->pEndIdx->GetIndex() :
                                            pSpellArgs->pStartIdx->GetIndex();
                        }


                        if( pSpellArgs && pSpellArgs->bIsGrammarCheck )
                        {
                            uno::Reference< linguistic2::XProofreadingIterator >  xGCIterator( GetGCIterator() );
                            if (xGCIterator.is())
                            {
                                String aText( ((SwTxtNode*)pNd)->GetTxt().Copy( nBeginGrammarCheck, nEndGrammarCheck - nBeginGrammarCheck ) );
                                uno::Reference< lang::XComponent > xDoc( ((SwDocShell*)GetDocShell())->GetBaseModel(), uno::UNO_QUERY );
                                // Expand the string:
                                rtl::OUString aExpandText;
                                const ModelToViewHelper::ConversionMap* pConversionMap =
                                        ((SwTxtNode*)pNd)->BuildConversionMap( aExpandText );
                                // get XFlatParagraph to use...
                                uno::Reference< text::XFlatParagraph > xFlatPara = new SwXFlatParagraph( *((SwTxtNode*)pNd), aExpandText, pConversionMap );

                                // get error position of cursor in XFlatParagraph
                                sal_Int32 nGrammarErrorPosInText;
                                linguistic2::ProofreadingResult aResult;
                                sal_Int32 nGrammarErrors;
                                do
                                {
                                    nGrammarErrorPosInText = ModelToViewHelper::ConvertToViewPosition( pConversionMap, nBeginGrammarCheck );
                                    aResult = xGCIterator->checkSentenceAtPosition(
                                            xDoc, xFlatPara, aExpandText, lang::Locale(), nBeginGrammarCheck, -1, -1 );

                                    lcl_syncGrammarError( *((SwTxtNode*)pNd), aResult, nBeginGrammarCheck, pConversionMap );

                                    // get suggestions to use for the specific error position
                                    nGrammarErrors = aResult.aErrors.getLength();
                                    // if grammar checking doesn't have any progress then quit
                                    if( aResult.nStartOfNextSentencePosition <= nBeginGrammarCheck )
                                        break;
                                    // prepare next iteration
                                    nBeginGrammarCheck = (xub_StrLen)aResult.nStartOfNextSentencePosition;
                                }
                                while( nSpellErrorPosition > aResult.nBehindEndOfSentencePosition && !nGrammarErrors && aResult.nBehindEndOfSentencePosition < nEndGrammarCheck );

                                if( nGrammarErrors > 0 && nSpellErrorPosition >= aResult.nBehindEndOfSentencePosition )
                                {
                                    aRet <<= aResult;
                                    //put the cursor to the current error
                                    const linguistic2::SingleProofreadingError &rError = aResult.aErrors[0];
                                    nCurrNd = pNd->GetIndex();
                                    pSttPos->nNode = nCurrNd;
                                    pEndPos->nNode = nCurrNd;
                                    pSpellArgs->pStartNode = ((SwTxtNode*)pNd);
                                    pSpellArgs->pEndNode = ((SwTxtNode*)pNd);
                                    pSpellArgs->pStartIdx->Assign(((SwTxtNode*)pNd), (xub_StrLen)ModelToViewHelper::ConvertToModelPosition( pConversionMap, rError.nErrorStart ).mnPos );
                                    pSpellArgs->pEndIdx->Assign(((SwTxtNode*)pNd), (xub_StrLen)ModelToViewHelper::ConvertToModelPosition( pConversionMap, rError.nErrorStart + rError.nErrorLength ).mnPos );
                                    nCurrNd = nEndNd;
                                }
                            }
                        }
                    }
                }
                break;
            case ND_SECTIONNODE:
                if( ( ((SwSectionNode*)pNd)->GetSection().IsProtect() ||
                    ((SwSectionNode*)pNd)->GetSection().IsHidden() ) )
                    nCurrNd = pNd->EndOfSectionIndex();
                break;
            case ND_ENDNODE:
                {
                    break;
                }
            }

            bGoOn = nCurrNd < nEndNd;
            ++nCurrNd;
        }
    }

    if( !aRet.hasValue() )
    {
        if (pConvArgs)
            aRet <<= pConvArgs->aConvText;
        else
            aRet <<= pSpellArgs->xSpellAlt;
    }
    delete pSpellArgs;

    return aRet;
}

class SwHyphArgs : public SwInterHyphInfo
{
    const SwNode *pStart;
    const SwNode *pEnd;
          SwNode *pNode;
    sal_uInt16 *pPageCnt;
    sal_uInt16 *pPageSt;

    sal_uInt32 nNode;
    xub_StrLen nPamStart;
    xub_StrLen nPamLen;

public:
         SwHyphArgs( const SwPaM *pPam, const Point &rPoint,
                         sal_uInt16* pPageCount, sal_uInt16* pPageStart );
    void SetPam( SwPaM *pPam ) const;
    inline void SetNode( SwNode *pNew ) { pNode = pNew; }
    inline const SwNode *GetNode() const { return pNode; }
    inline void SetRange( const SwNode *pNew );
    inline void NextNode() { ++nNode; }
    inline sal_uInt16 *GetPageCnt() { return pPageCnt; }
    inline sal_uInt16 *GetPageSt() { return pPageSt; }
};

SwHyphArgs::SwHyphArgs( const SwPaM *pPam, const Point &rCrsrPos,
                         sal_uInt16* pPageCount, sal_uInt16* pPageStart )
     : SwInterHyphInfo( rCrsrPos ), pNode(0),
     pPageCnt( pPageCount ), pPageSt( pPageStart )
{
    // Folgende Bedingungen muessen eingehalten werden:
    // 1) es gibt mindestens eine Selektion
    // 2) SPoint() == Start()
    ASSERT( pPam->HasMark(), "SwDoc::Hyphenate: blowing in the wind");
    ASSERT( *pPam->GetPoint() <= *pPam->GetMark(),
            "SwDoc::Hyphenate: New York, New York");

    const SwPosition *pPoint = pPam->GetPoint();
    nNode = pPoint->nNode.GetIndex();

    // Start einstellen
    pStart = pPoint->nNode.GetNode().GetTxtNode();
    nPamStart = pPoint->nContent.GetIndex();

    // Ende und Laenge einstellen.
    const SwPosition *pMark = pPam->GetMark();
    pEnd = pMark->nNode.GetNode().GetTxtNode();
    nPamLen = pMark->nContent.GetIndex();
    if( pPoint->nNode == pMark->nNode )
        nPamLen = nPamLen - pPoint->nContent.GetIndex();
}

inline void SwHyphArgs::SetRange( const SwNode *pNew )
{
    nStart = pStart == pNew ? nPamStart : 0;
    nLen   = pEnd   == pNew ? nPamLen : STRING_NOTFOUND;
}

void SwHyphArgs::SetPam( SwPaM *pPam ) const
{
    if( !pNode )
        *pPam->GetPoint() = *pPam->GetMark();
    else
    {
        pPam->GetPoint()->nNode = nNode;
        pPam->GetPoint()->nContent.Assign( pNode->GetCntntNode(), nWordStart );
        pPam->GetMark()->nNode = nNode;
        pPam->GetMark()->nContent.Assign( pNode->GetCntntNode(),
                                          nWordStart + nWordLen );
        ASSERT( nNode == pNode->GetIndex(),
                "SwHyphArgs::SetPam: Pam desaster" );
    }
}

// liefert sal_True zurueck, wenn es weitergehen soll.
sal_Bool lcl_HyphenateNode( const SwNodePtr& rpNd, void* pArgs )
{
    // Hyphenate liefert sal_True zurueck, wenn eine Trennstelle anliegt
    // und stellt pPam ein.
    SwTxtNode *pNode = rpNd->GetTxtNode();
    SwHyphArgs *pHyphArgs = (SwHyphArgs*)pArgs;
    if( pNode )
    {
        SwCntntFrm* pCntFrm = pNode->GetFrm();
        if( pCntFrm && !((SwTxtFrm*)pCntFrm)->IsHiddenNow() )
        {
            sal_uInt16 *pPageSt = pHyphArgs->GetPageSt();
            sal_uInt16 *pPageCnt = pHyphArgs->GetPageCnt();
            if( pPageCnt && *pPageCnt && pPageSt )
            {
                sal_uInt16 nPageNr = pCntFrm->GetPhyPageNum();
                if( !*pPageSt )
                {
                    *pPageSt = nPageNr;
                    if( *pPageCnt < *pPageSt )
                        *pPageCnt = *pPageSt;
                }
                long nStat = nPageNr >= *pPageSt ? nPageNr - *pPageSt + 1
                                         : nPageNr + *pPageCnt - *pPageSt + 1;
                ::SetProgressState( nStat, (SwDocShell*)pNode->GetDoc()->GetDocShell() );
            }
            pHyphArgs->SetRange( rpNd );
            if( pNode->Hyphenate( *pHyphArgs ) )
            {
                pHyphArgs->SetNode( rpNd );
                return sal_False;
            }
        }
    }
    pHyphArgs->NextNode();
    return sal_True;
}

uno::Reference< XHyphenatedWord >  SwDoc::Hyphenate(
                            SwPaM *pPam, const Point &rCrsrPos,
                             sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    ASSERT(this == pPam->GetDoc(), "SwDoc::Hyphenate: strangers in the night");

    if( *pPam->GetPoint() > *pPam->GetMark() )
        pPam->Exchange();

    SwHyphArgs aHyphArg( pPam, rCrsrPos, pPageCnt, pPageSt );
    SwNodeIndex aTmpIdx( pPam->GetMark()->nNode, 1 );
    GetNodes().ForEach( pPam->GetPoint()->nNode, aTmpIdx,
                    lcl_HyphenateNode, &aHyphArg );
    aHyphArg.SetPam( pPam );
    return aHyphArg.GetHyphWord();  // will be set by lcl_HyphenateNode
}


sal_Bool lcl_GetTokenToParaBreak( String& rStr, String& rRet, sal_Bool bRegExpRplc )
{
    sal_Bool bRet = sal_False;
    if( bRegExpRplc )
    {
        xub_StrLen nPos = 0;
        String sPara( String::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "\\n" )));
        while( STRING_NOTFOUND != ( nPos = rStr.Search( sPara, nPos )) )
        {
            // wurde das escaped?
            if( nPos && '\\' == rStr.GetChar( nPos-1 ))
            {
                if( ++nPos >= rStr.Len() )
                    break;
            }
            else
            {
                rRet = rStr.Copy( 0, nPos );
                rStr.Erase( 0, nPos + sPara.Len() );
                bRet = sal_True;
                break;
            }
        }
    }
    if( !bRet )
    {
        rRet = rStr;
        rStr.Erase();
    }
    return bRet;
}

bool SwDoc::ReplaceRange( SwPaM& rPam, const String& rStr,
        const bool bRegExReplace )
{
    // unfortunately replace works slightly differently from delete,
    // so we cannot use lcl_DoWithBreaks here...

    ::std::vector<xub_StrLen> Breaks;

    SwPaM aPam( *rPam.GetMark(), *rPam.GetPoint() );
    aPam.Normalize(sal_False);
    if (aPam.GetPoint()->nNode != aPam.GetMark()->nNode)
    {
        aPam.Move(fnMoveBackward);
    }
    ASSERT((aPam.GetPoint()->nNode == aPam.GetMark()->nNode), "invalid pam?");

    lcl_CalcBreaks(Breaks, aPam);

    while (!Breaks.empty() // skip over prefix of dummy chars
            && (aPam.GetMark()->nContent.GetIndex() == *Breaks.begin()) )
    {
        // skip!
        ++aPam.GetMark()->nContent; // always in bounds if Breaks valid
        Breaks.erase(Breaks.begin());
    }
    *rPam.Start() = *aPam.GetMark(); // update start of original pam w/ prefix

    if (!Breaks.size())
    {
        return ReplaceRangeImpl(rPam, rStr, bRegExReplace); // original pam!
    }

    // N.B.: deletion must be split into several parts if the text node
    // contains a text attribute with end and with dummy character
    // and the selection does not contain the text attribute completely,
    // but overlaps its start (left), where the dummy character is.

    bool bRet( true );
    // iterate from end to start, to avoid invalidating the offsets!
    ::std::vector<xub_StrLen>::reverse_iterator iter( Breaks.rbegin() );
    ASSERT(aPam.GetPoint() == aPam.End(), "wrong!");
    SwPosition & rEnd( *aPam.End() );
    SwPosition & rStart( *aPam.Start() );

    // set end of temp pam to original end (undo Move backward above)
    rEnd = *rPam.End();
    // after first deletion, rEnd will point into the original text node again!

    while (iter != Breaks.rend())
    {
        rStart.nContent = *iter + 1;
        if (rEnd.nContent != rStart.nContent) // check if part is empty
        {
            bRet &= (IsRedlineOn())
                ? DeleteAndJoinWithRedlineImpl(aPam)
                : DeleteAndJoinImpl(aPam, false);
        }
        rEnd.nContent = *iter;
        ++iter;
    }

    rStart = *rPam.Start(); // set to original start
    ASSERT(rEnd.nContent > rStart.nContent, "replace part empty!");
    if (rEnd.nContent > rStart.nContent) // check if part is empty
    {
        bRet &= ReplaceRangeImpl(aPam, rStr, bRegExReplace);
    }

    rPam = aPam; // update original pam (is this required?)

    return bRet;
}

// N.B.: it is possible to call Replace with a PaM that spans 2 paragraphs:
// search with regex for "$", then replace _all_
bool SwDoc::ReplaceRangeImpl( SwPaM& rPam, const String& rStr,
        const bool bRegExReplace )
{
    if( !rPam.HasMark() || *rPam.GetPoint() == *rPam.GetMark() )
        return false;

    sal_Bool bJoinTxt, bJoinPrev;
    lcl_GetJoinFlags( rPam, bJoinTxt, bJoinPrev );

    {
        // dann eine Kopie vom Cursor erzeugen um alle Pams aus den
        // anderen Sichten aus dem Loeschbereich zu verschieben
        // ABER NICHT SICH SELBST !!
        SwPaM aDelPam( *rPam.GetMark(), *rPam.GetPoint() );
        ::PaMCorrAbs( aDelPam, *aDelPam.GetPoint() );

        SwPosition *pStt = (SwPosition*)aDelPam.Start(),
                   *pEnd = (SwPosition*)aDelPam.End();
        ASSERT( pStt->nNode == pEnd->nNode ||
                ( pStt->nNode.GetIndex() + 1 == pEnd->nNode.GetIndex() &&
                    !pEnd->nContent.GetIndex() ),
                "invalid range: Point and Mark on different nodes" );
        sal_Bool bOneNode = pStt->nNode == pEnd->nNode;

        // eigenes Undo ????
        String sRepl( rStr );
        SwTxtNode* pTxtNd = pStt->nNode.GetNode().GetTxtNode();
        xub_StrLen nStt = pStt->nContent.GetIndex(),
                nEnd = bOneNode ? pEnd->nContent.GetIndex()
                                : pTxtNd->GetTxt().Len();

        SwDataChanged aTmp( aDelPam, 0 );

        if( IsRedlineOn() )
        {
            RedlineMode_t eOld = GetRedlineMode();
            checkRedlining(eOld);
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);

                // Bug 68584 - if any Redline will change (split!) the node
                const ::sw::mark::IMark* pBkmk = getIDocumentMarkAccess()->makeMark( aDelPam, ::rtl::OUString(), IDocumentMarkAccess::UNO_BOOKMARK );

                //JP 06.01.98: MUSS noch optimiert werden!!!
                SetRedlineMode(
                    (RedlineMode_t)(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE ));

                *aDelPam.GetPoint() = pBkmk->GetMarkPos();
                if(pBkmk->IsExpanded())
                    *aDelPam.GetMark() = pBkmk->GetOtherMarkPos();
                getIDocumentMarkAccess()->deleteMark(pBkmk);
                pStt = aDelPam.Start();
                pTxtNd = pStt->nNode.GetNode().GetTxtNode();
                nStt = pStt->nContent.GetIndex();
            }

            if( sRepl.Len() )
            {
                // Attribute des 1. Zeichens ueber den ReplaceText setzen
                SfxItemSet aSet( GetAttrPool(),
                            RES_CHRATR_BEGIN,     RES_TXTATR_WITHEND_END - 1,
                            RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
                            0 );
                pTxtNd->GetAttr( aSet, nStt+1, nStt+1 );

                aSet.ClearItem( RES_TXTATR_REFMARK );
                aSet.ClearItem( RES_TXTATR_TOXMARK );
                aSet.ClearItem( RES_TXTATR_CJK_RUBY );
                aSet.ClearItem( RES_TXTATR_INETFMT );
                aSet.ClearItem( RES_TXTATR_META );
                aSet.ClearItem( RES_TXTATR_METAFIELD );

                if( aDelPam.GetPoint() != aDelPam.End() )
                    aDelPam.Exchange();

                // das Ende merken
                SwNodeIndex aPtNd( aDelPam.GetPoint()->nNode, -1 );
                xub_StrLen nPtCnt = aDelPam.GetPoint()->nContent.GetIndex();

                sal_Bool bFirst = sal_True;
                String sIns;
                while ( lcl_GetTokenToParaBreak( sRepl, sIns, bRegExReplace ) )
                {
                    InsertString( aDelPam, sIns );
                    if( bFirst )
                    {
                        SwNodeIndex aMkNd( aDelPam.GetMark()->nNode, -1 );
                        xub_StrLen nMkCnt = aDelPam.GetMark()->nContent.GetIndex();

                        SplitNode( *aDelPam.GetPoint(), false );

                        aMkNd++;
                        aDelPam.GetMark()->nNode = aMkNd;
                        aDelPam.GetMark()->nContent.Assign(
                                    aMkNd.GetNode().GetCntntNode(), nMkCnt );
                        bFirst = sal_False;
                    }
                    else
                        SplitNode( *aDelPam.GetPoint(), false );
                }
                if( sIns.Len() )
                {
                    InsertString( aDelPam, sIns );
                }

                SwPaM aTmpRange( *aDelPam.GetPoint() );
                aTmpRange.SetMark();

                aPtNd++;
                aDelPam.GetPoint()->nNode = aPtNd;
                aDelPam.GetPoint()->nContent.Assign( aPtNd.GetNode().GetCntntNode(),
                                                    nPtCnt);
                *aTmpRange.GetMark() = *aDelPam.GetPoint();

                RstTxtAttrs( aTmpRange );
                InsertItemSet( aTmpRange, aSet, 0 );
            }

            if (GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo *const pUndoRD =
                    new SwUndoRedlineDelete( aDelPam, UNDO_REPLACE );
                GetIDocumentUndoRedo().AppendUndo(pUndoRD);
            }
            AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_DELETE, aDelPam ), true);

            *rPam.GetMark() = *aDelPam.GetMark();
            if (GetIDocumentUndoRedo().DoesUndo())
            {
                *aDelPam.GetPoint() = *rPam.GetPoint();
                GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);

                // Bug 68584 - if any Redline will change (split!) the node
                const ::sw::mark::IMark* pBkmk = getIDocumentMarkAccess()->makeMark( aDelPam, ::rtl::OUString(), IDocumentMarkAccess::UNO_BOOKMARK );

                SwIndex& rIdx = aDelPam.GetPoint()->nContent;
                rIdx.Assign( 0, 0 );
                aDelPam.GetMark()->nContent = rIdx;
                rPam.GetPoint()->nNode = 0;
                rPam.GetPoint()->nContent = rIdx;
                *rPam.GetMark() = *rPam.GetPoint();
//JP 06.01.98: MUSS noch optimiert werden!!!
SetRedlineMode( eOld );

                *rPam.GetPoint() = pBkmk->GetMarkPos();
                if(pBkmk->IsExpanded())
                    *rPam.GetMark() = pBkmk->GetOtherMarkPos();
                getIDocumentMarkAccess()->deleteMark(pBkmk);
            }
            bJoinTxt = sal_False;
        }
        else
        {
            if( !IsIgnoreRedline() && GetRedlineTbl().Count() )
                DeleteRedline( aDelPam, true, USHRT_MAX );

            SwUndoReplace* pUndoRpl = 0;
            bool const bDoesUndo = GetIDocumentUndoRedo().DoesUndo();
            if (bDoesUndo)
            {
                pUndoRpl = new SwUndoReplace(aDelPam, sRepl, bRegExReplace);
                GetIDocumentUndoRedo().AppendUndo(pUndoRpl);
            }
            ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

            if( aDelPam.GetPoint() != pStt )
                aDelPam.Exchange();

            SwNodeIndex aPtNd( pStt->nNode, -1 );
            xub_StrLen nPtCnt = pStt->nContent.GetIndex();

            // die Werte nochmal setzen, falls schohn Rahmen oder Fussnoten
            // auf dem Text entfernt wurden!
            nStt = nPtCnt;
            nEnd = bOneNode ? pEnd->nContent.GetIndex()
                            : pTxtNd->GetTxt().Len();

            sal_Bool bFirst = sal_True;
            String sIns;
            while ( lcl_GetTokenToParaBreak( sRepl, sIns, bRegExReplace ) )
            {
                if( !bFirst || nStt == pTxtNd->GetTxt().Len() )
                {
                    InsertString( aDelPam, sIns );
                }
                else if( nStt < nEnd || sIns.Len() )
                {
                    pTxtNd->ReplaceText( pStt->nContent, nEnd - nStt, sIns );
                }
                SplitNode( *pStt, false);
                bFirst = sal_False;
            }

            if( bFirst || sIns.Len() )
            {
                if( !bFirst || nStt == pTxtNd->GetTxt().Len() )
                {
                    InsertString( aDelPam, sIns );
                }
                else if( nStt < nEnd || sIns.Len() )
                {
                    pTxtNd->ReplaceText( pStt->nContent, nEnd - nStt, sIns );
                }
            }

            *rPam.GetMark() = *aDelPam.GetMark();

            aPtNd++;
            rPam.GetMark()->nNode = aPtNd;
            rPam.GetMark()->nContent.Assign( aPtNd.GetNode().GetCntntNode(),
                                                nPtCnt );
            if( bJoinTxt )
                rPam.Move( fnMoveBackward );

            if( pUndoRpl )
            {
                pUndoRpl->SetEnd(rPam);
            }
        }
    }

    if( bJoinTxt )
        lcl_JoinText( rPam, bJoinPrev );

    SetModified();
    return true;
}

    // speicher die akt. Werte fuer die automatische Aufnahme von Ausnahmen
    // in die Autokorrektur
void SwDoc::SetAutoCorrExceptWord( SwAutoCorrExceptWord* pNew )
{
    if( pACEWord && pNew != pACEWord )
        delete pACEWord;
    pACEWord = pNew;
}

bool SwDoc::DelFullPara( SwPaM& rPam )
{
    const SwPosition &rStt = *rPam.Start(), &rEnd = *rPam.End();
    const SwNode* pNd = &rStt.nNode.GetNode();
    sal_uInt32 nSectDiff = pNd->StartOfSectionNode()->EndOfSectionIndex() -
                        pNd->StartOfSectionIndex();
    sal_uInt32 nNodeDiff = rEnd.nNode.GetIndex() - rStt.nNode.GetIndex();

    if ( nSectDiff-2 <= nNodeDiff || IsRedlineOn() ||
         /* #i9185# Prevent getting the node after the end node (see below) */
        rEnd.nNode.GetIndex() + 1 == GetNodes().Count() )
    {
        return sal_False;
    }

    // harte SeitenUmbrueche am nachfolgenden Node verschieben
    sal_Bool bSavePageBreak = sal_False, bSavePageDesc = sal_False;

    /* #i9185# This whould lead to a segmentation fault if not catched
       above. */
    sal_uLong nNextNd = rEnd.nNode.GetIndex() + 1;
    SwTableNode *const pTblNd = GetNodes()[ nNextNd ]->GetTableNode();

    if( pTblNd && pNd->IsCntntNode() )
    {
        SwFrmFmt* pTableFmt = pTblNd->GetTable().GetFrmFmt();
//JP 24.08.98: will man wirklich den PageDesc/Break vom
//              nachfolgen Absatz ueberbuegeln?
//      const SwAttrSet& rAttrSet = pTableFmt->GetAttrSet();
//      if( SFX_ITEM_SET != rAttrSet.GetItemState( RES_PAGEDESC ) &&
//          SFX_ITEM_SET != rAttrSet.GetItemState( RES_BREAK ))
        {
            const SfxPoolItem *pItem;
            const SfxItemSet* pSet = ((SwCntntNode*)pNd)->GetpSwAttrSet();
            if( pSet && SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC,
                sal_False, &pItem ) )
            {
                pTableFmt->SetFmtAttr( *pItem );
                bSavePageDesc = sal_True;
            }

            if( pSet && SFX_ITEM_SET == pSet->GetItemState( RES_BREAK,
                sal_False, &pItem ) )
            {
                pTableFmt->SetFmtAttr( *pItem );
                bSavePageBreak = sal_True;
            }
        }
    }

    bool const bDoesUndo = GetIDocumentUndoRedo().DoesUndo();
    if( bDoesUndo )
    {
        if( !rPam.HasMark() )
            rPam.SetMark();
        else if( rPam.GetPoint() == &rStt )
            rPam.Exchange();
        rPam.GetPoint()->nNode++;

        SwCntntNode *pTmpNode = rPam.GetPoint()->nNode.GetNode().GetCntntNode();
        rPam.GetPoint()->nContent.Assign( pTmpNode, 0 );
        bool bGoNext = (0 == pTmpNode);
        pTmpNode = rPam.GetMark()->nNode.GetNode().GetCntntNode();
        rPam.GetMark()->nContent.Assign( pTmpNode, 0 );

        GetIDocumentUndoRedo().ClearRedo();

        SwPaM aDelPam( *rPam.GetMark(), *rPam.GetPoint() );
        {
            SwPosition aTmpPos( *aDelPam.GetPoint() );
            if( bGoNext )
            {
                pTmpNode = GetNodes().GoNext( &aTmpPos.nNode );
                aTmpPos.nContent.Assign( pTmpNode, 0 );
            }
            ::PaMCorrAbs( aDelPam, aTmpPos );
        }

        SwUndoDelete* pUndo = new SwUndoDelete( aDelPam, sal_True );

        *rPam.GetPoint() = *aDelPam.GetPoint();
        pUndo->SetPgBrkFlags( bSavePageBreak, bSavePageDesc );
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }
    else
    {
        SwNodeRange aRg( rStt.nNode, rEnd.nNode );
        if( rPam.GetPoint() != &rEnd )
            rPam.Exchange();

        // versuche hinters Ende zu verschieben
        if( !rPam.Move( fnMoveForward, fnGoNode ) )
        {
            // na gut, dann an den Anfang
            rPam.Exchange();
            if( !rPam.Move( fnMoveBackward, fnGoNode ))
            {
                ASSERT( sal_False, "kein Node mehr vorhanden" );
                return sal_False;
            }
        }
            // text::Bookmarks usw. verschieben
        CorrAbs( aRg.aStart, aRg.aEnd, *rPam.GetPoint(), sal_True );

            // was ist mit Fly's ??
        {
            // stehen noch FlyFrames rum, loesche auch diese
            for( sal_uInt16 n = 0; n < GetSpzFrmFmts()->Count(); ++n )
            {
                SwFrmFmt* pFly = (*GetSpzFrmFmts())[n];
                const SwFmtAnchor* pAnchor = &pFly->GetAnchor();
                SwPosition const*const pAPos = pAnchor->GetCntntAnchor();
                if (pAPos &&
                    ((FLY_AT_PARA == pAnchor->GetAnchorId()) ||
                     (FLY_AT_CHAR == pAnchor->GetAnchorId())) &&
                    aRg.aStart <= pAPos->nNode && pAPos->nNode <= aRg.aEnd )
                {
                    DelLayoutFmt( pFly );
                    --n;
                }
            }
        }

        SwCntntNode *pTmpNode = rPam.GetBound( sal_True ).nNode.GetNode().GetCntntNode();
        rPam.GetBound( sal_True ).nContent.Assign( pTmpNode, 0 );
        pTmpNode = rPam.GetBound( sal_False ).nNode.GetNode().GetCntntNode();
        rPam.GetBound( sal_False ).nContent.Assign( pTmpNode, 0 );
        GetNodes().Delete( aRg.aStart, nNodeDiff+1 );
    }
    rPam.DeleteMark();
    SetModified();

    return sal_True;
}


void SwDoc::TransliterateText(
    const SwPaM& rPaM,
    utl::TransliterationWrapper& rTrans )
{
    SwUndoTransliterate *const pUndo = (GetIDocumentUndoRedo().DoesUndo())
        ?   new SwUndoTransliterate( rPaM, rTrans )
        :   0;

    const SwPosition* pStt = rPaM.Start(),
                       * pEnd = rPaM.End();
    sal_uLong nSttNd = pStt->nNode.GetIndex(),
          nEndNd = pEnd->nNode.GetIndex();
    xub_StrLen nSttCnt = pStt->nContent.GetIndex(),
               nEndCnt = pEnd->nContent.GetIndex();

    SwTxtNode* pTNd = pStt->nNode.GetNode().GetTxtNode();
    if( pStt == pEnd && pTNd )  // no selection?
    {
        // set current word as 'area of effect'

        Boundary aBndry;
        if( pBreakIt->GetBreakIter().is() )
            aBndry = pBreakIt->GetBreakIter()->getWordBoundary(
                        pTNd->GetTxt(), nSttCnt,
                        pBreakIt->GetLocale( pTNd->GetLang( nSttCnt ) ),
                        WordType::ANY_WORD /*ANYWORD_IGNOREWHITESPACES*/,
                        sal_True );

        if( aBndry.startPos < nSttCnt && nSttCnt < aBndry.endPos )
        {
            nSttCnt = (xub_StrLen)aBndry.startPos;
            nEndCnt = (xub_StrLen)aBndry.endPos;
        }
    }

    if( nSttNd != nEndNd )  // is more than one text node involved?
    {
        // iterate over all effected text nodes, the first and the last one
        // may be incomplete because the selection starts and/or ends there

        SwNodeIndex aIdx( pStt->nNode );
        if( nSttCnt )
        {
            aIdx++;
            if( pTNd )
                pTNd->TransliterateText( rTrans, nSttCnt, pTNd->GetTxt().Len(), pUndo );
        }

        for( ; aIdx.GetIndex() < nEndNd; aIdx++ )
        {
            if( 0 != ( pTNd = aIdx.GetNode().GetTxtNode() ))
                pTNd->TransliterateText( rTrans, 0, pTNd->GetTxt().Len(), pUndo );
        }

        if( nEndCnt && 0 != ( pTNd = pEnd->nNode.GetNode().GetTxtNode() ))
            pTNd->TransliterateText( rTrans, 0, nEndCnt, pUndo );
    }
    else if( pTNd && nSttCnt < nEndCnt )
        pTNd->TransliterateText( rTrans, nSttCnt, nEndCnt, pUndo );

    if( pUndo )
    {
        if( pUndo->HasData() )
        {
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
        else
            delete pUndo;
    }
    SetModified();
}


#define MAX_REDLINE_COUNT   250
// -----------------------------------------------------------------------------
void SwDoc::checkRedlining(RedlineMode_t& _rReadlineMode)
{
    const SwRedlineTbl& rRedlineTbl = GetRedlineTbl();
    SwEditShell* pEditShell = GetEditShell();
    Window* pParent = pEditShell ? pEditShell->GetWin() : NULL;
    if ( pParent && !mbReadlineChecked && rRedlineTbl.Count() > MAX_REDLINE_COUNT
        && !((_rReadlineMode & nsRedlineMode_t::REDLINE_SHOW_DELETE) == nsRedlineMode_t::REDLINE_SHOW_DELETE) )
    {
        WarningBox aWarning( pParent,SW_RES(MSG_DISABLE_READLINE_QUESTION));
        sal_uInt16 nResult = aWarning.Execute();
        mbReadlineChecked = sal_True;
        if ( nResult == RET_YES )
        {
            sal_Int32 nMode = (sal_Int32)_rReadlineMode;
            nMode |= nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE;
            _rReadlineMode = (RedlineMode_t)nMode;
        }
    }
}
// -----------------------------------------------------------------------------

void SwDoc::CountWords( const SwPaM& rPaM, SwDocStat& rStat ) const
{
    // This is a modified version of SwDoc::TransliterateText
    const SwPosition* pStt = rPaM.Start();
    const SwPosition* pEnd = pStt == rPaM.GetPoint() ? rPaM.GetMark()
                                                     : rPaM.GetPoint();

    const sal_uLong nSttNd = pStt->nNode.GetIndex();
    const sal_uLong nEndNd = pEnd->nNode.GetIndex();

    const xub_StrLen nSttCnt = pStt->nContent.GetIndex();
    const xub_StrLen nEndCnt = pEnd->nContent.GetIndex();

    const SwTxtNode* pTNd = pStt->nNode.GetNode().GetTxtNode();
    if( pStt == pEnd && pTNd )                  // no region ?
    {
        // do nothing
        return;
    }

    if( nSttNd != nEndNd )
    {
        SwNodeIndex aIdx( pStt->nNode );
        if( nSttCnt )
        {
            aIdx++;
            if( pTNd )
                pTNd->CountWords( rStat, nSttCnt, pTNd->GetTxt().Len() );
        }

        for( ; aIdx.GetIndex() < nEndNd; aIdx++ )
            if( 0 != ( pTNd = aIdx.GetNode().GetTxtNode() ))
                pTNd->CountWords( rStat, 0, pTNd->GetTxt().Len() );

        if( nEndCnt && 0 != ( pTNd = pEnd->nNode.GetNode().GetTxtNode() ))
            pTNd->CountWords( rStat, 0, nEndCnt );
    }
    else if( pTNd && nSttCnt < nEndCnt )
        pTNd->CountWords( rStat, nSttCnt, nEndCnt );
}

void SwDoc::RemoveLeadingWhiteSpace(const SwPosition & rPos )
{
    const SwTxtNode* pTNd = rPos.nNode.GetNode().GetTxtNode();
    if ( pTNd )
    {
        const String& rTxt = pTNd->GetTxt();
        xub_StrLen nIdx = 0;
        sal_Unicode cCh;
        while( nIdx < rTxt.Len() &&
                ( '\t' == ( cCh = rTxt.GetChar( nIdx ) ) ||
                (  ' ' == cCh ) ) )
            ++nIdx;

        if ( nIdx > 0 )
        {
            SwPaM aPam(rPos);
            aPam.GetPoint()->nContent = 0;
            aPam.SetMark();
            aPam.GetMark()->nContent = nIdx;
            DeleteRange( aPam );
        }
    }
}
