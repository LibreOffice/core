/*************************************************************************
 *
 *  $RCSfile: docedt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <string.h>         // fuer strchr()

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _WORDSEL_HXX //autogen
#include <svtools/wordsel.hxx>
#endif
#ifndef _SVX_CSCOITEM_HXX //autogen
#include <svx/cscoitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _LINGU_LNGPROPS_HHX_
#include <lingu/lngprops.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _ACORRECT_HXX
#include <acorrect.hxx>     // Autokorrektur
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>      // fuer SwBookmark
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>           // fuers Spell
#endif
#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>
#endif
#ifndef _CRSTATE_HXX
#include <crstate.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>       // beim Move: Verzeichnisse korrigieren
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>       // Statusanzeige
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>       // Strukturen zum Sichern beim Move/Delete
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>          // fuers UpdateFtn
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _SPLARGS_HXX
#include <splargs.hxx>      // fuer Spell
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>       // fuer die UndoIds
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::rtl;
//using namespace ::utl;

#define S2U(rString) OUString::createFromAscii(rString)


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
};

SV_DECL_PTRARR_DEL( _SaveRedlines, _SaveRedline*, 0, 4 )

SV_IMPL_VARARR( _SaveFlyArr, _SaveFly )
SV_IMPL_PTRARR( SaveBookmarks, SaveBookmark* )
SV_IMPL_PTRARR( _SaveRedlines, _SaveRedline* )

sal_Bool lcl_MayOverwrite( const SwTxtNode *pNode, const xub_StrLen nPos )
{
    sal_Bool bRet = sal_True;
    const SwTxtAttr *pHt;
    sal_Unicode cChr = pNode->GetTxt().GetChar( nPos );
    if( ( CH_TXTATR_BREAKWORD == cChr || CH_TXTATR_INWORD == cChr ) &&
        0 != (pHt = pNode->GetTxtAttr( nPos )) )
            switch( pHt->Which() )
            {
                case RES_TXTATR_FLYCNT:
                case RES_TXTATR_FTN:
                case RES_TXTATR_FIELD:
                case RES_TXTATR_REFMARK:
                case RES_TXTATR_TOXMARK:
                    bRet = sal_False;
                    break;
            }
    return bRet;
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

void _RestFlyInRange( _SaveFlyArr & rArr, const SwNodeIndex& rSttIdx )
{
    SwPosition aPos( rSttIdx );
    for( sal_uInt16 n = 0; n < rArr.Count(); ++n )
    {
        // neuen Anker anlegen
        _SaveFly& rSave = rArr[n];
        SwFrmFmt* pFmt = rSave.pFrmFmt;
        aPos.nNode = rSttIdx.GetIndex() + rSave.nNdDiff;
        aPos.nContent.Assign( 0, 0 );
        SwFmtAnchor aAnchor( pFmt->GetAnchor() );
        aAnchor.SetAnchor( &aPos );
        pFmt->GetDoc()->GetSpzFrmFmts()->Insert(
                pFmt, pFmt->GetDoc()->GetSpzFrmFmts()->Count() );
        pFmt->SetAttr( aAnchor );
        SwCntntNode* pCNd = aPos.nNode.GetNode().GetCntntNode();
        if( pCNd && pCNd->GetFrm( 0, 0, sal_False ) )
            pFmt->MakeFrms();
    }
}

void _SaveFlyInRange( const SwNodeRange& rRg, _SaveFlyArr& rArr )
{
    SwFrmFmt* pFmt;
    const SwFmtAnchor* pAnchor;
    const SwPosition* pAPos;
    SwSpzFrmFmts& rFmts = *rRg.aStart.GetNode().GetDoc()->GetSpzFrmFmts();
    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        pFmt = (SwFrmFmt*)rFmts[n];
        pAnchor = &pFmt->GetAnchor();
        if( ( FLY_AT_CNTNT == pAnchor->GetAnchorId() ||
              FLY_AUTO_CNTNT == pAnchor->GetAnchorId() ) &&
            0 != ( pAPos = pAnchor->GetCntntAnchor() ) &&
            rRg.aStart <= pAPos->nNode && pAPos->nNode < rRg.aEnd )
        {
            ASSERT( pAnchor->GetAnchorId() != FLY_AUTO_CNTNT, "FLY-AUTO-Baustelle!" );
            _SaveFly aSave( pAPos->nNode.GetIndex() -
                            rRg.aStart.GetIndex(), pFmt );
            rArr.Insert( aSave, rArr.Count());
            pFmt->DelFrms();
            rFmts.Remove( n--, 1 );
        }
    }
}

void _SaveFlyInRange( const SwPaM& rPam, const SwNodeIndex& rInsPos,
                       _SaveFlyArr& rArr, sal_Bool bMoveAllFlys )
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

    const SwPosition* pAPos;
    const SwNodeIndex* pCntntIdx;

    for( sal_uInt16 n = 0; n < rFmts.Count(); ++n )
    {
        sal_Bool bInsPos = sal_False;
        pFmt = (SwFrmFmt*)rFmts[n];
        pAnchor = &pFmt->GetAnchor();
        if( ( FLY_AT_CNTNT == pAnchor->GetAnchorId() ||
              FLY_AUTO_CNTNT == pAnchor->GetAnchorId() ) &&
            0 != ( pAPos = pAnchor->GetCntntAnchor() ) &&
            // nicht verschieben, wenn die InsPos im CntntBereich vom Fly ist
            ( 0 == ( pCntntIdx = pFmt->GetCntnt().GetCntntIdx() ) ||
              !( *pCntntIdx < rInsPos &&
                rInsPos < pCntntIdx->GetNode().EndOfSectionIndex() )) )
        {
            ASSERT( pAnchor->GetAnchorId() != FLY_AUTO_CNTNT, "FLY-AUTO-Baustelle!" );
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
                    pFmt->SetAttr( aAnchor );
//                  ((SwFmtAnchor*)pAnchor)->SetAnchor( &aPos );
                }
            }
            else if( ( rSttNdIdx.GetIndex() + nSttOff <= pAPos->nNode.GetIndex()
                    && pAPos->nNode.GetIndex() <= rEndNdIdx.GetIndex() - nOff ) ||
                        0 != ( bInsPos = rInsPos == pAPos->nNode ))

            {
                _SaveFly aSave( bInsPos ? 0 : pAPos->nNode.GetIndex() -
                                                rSttNdIdx.GetIndex(), pFmt );
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
    const SwPosition* pAPos;
    for ( sal_uInt16 i = rTbl.Count(); i; )
    {
        SwFrmFmt *pFmt = rTbl[--i];
        const SwFmtAnchor &rAnch = pFmt->GetAnchor();
        if( ( rAnch.GetAnchorId() == FLY_AT_CNTNT ||
              rAnch.GetAnchorId() == FLY_AUTO_CNTNT ) &&
            0 != ( pAPos = rAnch.GetCntntAnchor() ) &&
            ( bDelFwrd
                ? rMkNdIdx < pAPos->nNode && pAPos->nNode <= rPtNdIdx
                : rPtNdIdx <= pAPos->nNode && pAPos->nNode < rMkNdIdx ))
        {
            ASSERT( rAnch.GetAnchorId() != FLY_AUTO_CNTNT, "FLY-AUTO-Baustelle!" );
            // nur den Anker verchieben ??
            if( rPtNdIdx == pAPos->nNode )
            {
                SwFmtAnchor aAnch( pFmt->GetAnchor() );
                SwPosition aPos( rMkNdIdx );
                aAnch.SetAnchor( &aPos );
                pFmt->SetAttr( aAnch );
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
//              i++;    // keinen auslassen
            }
        }
    }
}


int lcl_SaveFtn( const SwNodeIndex& rSttNd, const SwNodeIndex& rEndNd,
                 const SwNodeIndex& rInsPos,
                 SwFtnIdxs& rFtnArr, SwFtnIdxs& rSaveArr,
                 const SwIndex* pSttCnt = 0, const SwIndex* pEndCnt = 0 )
{
    int bUpdateFtn = sal_False;
    if( rFtnArr.Count() )
    {
        const SwNodes& rNds = rInsPos.GetNodes();
        int bDelFtn = rInsPos.GetIndex() < rNds.GetEndOfAutotext().GetIndex() &&
                    rSttNd.GetIndex() >= rNds.GetEndOfAutotext().GetIndex();
        int bSaveFtn = !bDelFtn &&
                        rInsPos.GetIndex() >= rNds.GetEndOfExtras().GetIndex();

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
                    rTxtNd.Erase( aIdx, 1 );
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
                    rTxtNd.Erase( aIdx, 1 );
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
    return bUpdateFtn;
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

    SwRedlineMode eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern( ( eOld & ~REDLINE_IGNORE) | REDLINE_ON );
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
                pDoc->AppendRedline( pTmp );
            }
        }
        else
            break;

    } while( ++nRedlPos < pDoc->GetRedlineTbl().Count() );
    pDoc->SetRedlineMode_intern( eOld );
}

void lcl_RestoreRedlines( SwDoc* pDoc, sal_uInt32 nInsPos, _SaveRedlines& rArr )
{
    SwRedlineMode eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern( ( eOld & ~REDLINE_IGNORE) | REDLINE_ON );

    for( sal_uInt16 n = 0; n < rArr.Count(); ++n )
    {
        _SaveRedline* pSave = rArr[ n ];
        pSave->SetPos( nInsPos );
        pDoc->AppendRedline( pSave->pRedl );
    }

    pDoc->SetRedlineMode_intern( eOld );
}

// ------------------------------------------------------------------------

_SaveRedlEndPosForRestore::_SaveRedlEndPosForRestore( const SwNodeIndex& rInsIdx )
    : pSavArr( 0 ), pSavIdx( 0 )
{
    SwNode& rNd = rInsIdx.GetNode();
    SwDoc* pDest = rNd.GetDoc();
    if( pDest->GetRedlineTbl().Count() )
    {
        sal_uInt16 nFndPos;
        const SwPosition* pEnd;
        SwPosition aSrcPos( rInsIdx, SwIndex( rNd.GetCntntNode(), 0 ));
        const SwRedline* pRedl = pDest->GetRedline( aSrcPos, &nFndPos );
        while( nFndPos-- && *( pEnd = ( pRedl =
            pDest->GetRedlineTbl()[ nFndPos ] )->End() ) == aSrcPos &&
            *pRedl->Start() != aSrcPos )
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
    SwPosition aPos( *pSavIdx, SwIndex( pSavIdx->GetNode().GetCntntNode(), 0 ));
    for( sal_uInt16 n = pSavArr->Count(); n; )
        *((SwPosition*)pSavArr->GetObject( --n )) = aPos;
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
    DeleteRedline( *pSttNd );
    _DelBookmarks( aSttIdx, aEndIdx );

    {
        // alle Crsr/StkCrsr/UnoCrsr aus dem Loeschbereich verschieben
        SwNodeIndex aMvStt( aSttIdx, 1 );
        CorrAbs( aMvStt, aEndIdx, SwPosition( aSttIdx ), sal_True );
    }

    GetNodes().DelNodes( aSttIdx, aEndIdx.GetIndex() - aSttIdx.GetIndex() + 1 );
}



/*************************************************************************
|*                SwDoc::Insert(char)
|*    Beschreibung      Zeichen einfuegen
*************************************************************************/

sal_Bool SwDoc::Insert( const SwPaM &rRg, sal_Unicode c )
{
    if( DoesUndo() )
        ClearRedo();

    const SwPosition & rPos = *rRg.GetPoint();

    if( pACEWord )                  // Aufnahme in die Autokorrektur
    {
        if( pACEWord->IsDeleted() )
            pACEWord->CheckChar( rPos, c );
        delete pACEWord, pACEWord = 0;
    }
    SwTxtNode *pNode = rPos.nNode.GetNode().GetTxtNode();
    if(!pNode)
        return sal_False;
    sal_Bool bInsOneChar = sal_True;

    SwDataChanged aTmp( rRg, 0 );

    pNode->Insert( c, rPos.nContent );

    if ( DoesUndo() )
    {
        sal_uInt16 nUndoSize = pUndos->Count();
        SwUndo * pUndo;
        if( DoesGroupUndo() && bInsOneChar && nUndoSize-- &&
            UNDO_INSERT == ( pUndo = (*pUndos)[ nUndoSize ])->GetId() &&
            ((SwUndoInsert*)pUndo)->CanGrouping( rPos, c ))
            ; // wenn CanGrouping() sal_True returnt, ist schon alles erledigt
        else
            AppendUndo( new SwUndoInsert( rPos.nNode,
                                        rPos.nContent.GetIndex(), 1,
                                        !WordSelection::IsNormalChar( c ) ));
    }

    if( IsRedlineOn() || (!IsIgnoreRedline() && pRedlineTbl->Count() ))
    {
        SwPaM aPam( rPos.nNode, rPos.nContent.GetIndex() - 1,
                    rPos.nNode, rPos.nContent.GetIndex() );
        if( IsRedlineOn() )
            AppendRedline( new SwRedline( REDLINE_INSERT, aPam ));
        else
            SplitRedline( aPam );
    }

    SetModified();
    return sal_True;
}


/*************************************************************************
|*                SwDoc::Overwrite(char)
|*    Beschreibung      Zeichen ueberschreiben
*************************************************************************/

sal_Bool SwDoc::Overwrite( const SwPaM &rRg, sal_Unicode c )
{
    SwPosition& rPt = *(SwPosition*)rRg.GetPoint();
    if( pACEWord )                  // Aufnahme in die Autokorrektur
    {
        pACEWord->CheckChar( rPt, c );
        delete pACEWord, pACEWord = 0;
    }

    SwTxtNode *pNode = rPt.nNode.GetNode().GetTxtNode();
    if(!pNode)
        return sal_False;

    sal_uInt16 nOldAttrCnt = pNode->GetpSwpHints()
                                ? pNode->GetpSwpHints()->Count() : 0;
    SwDataChanged aTmp( rRg, 0 );
    SwIndex& rIdx = rPt.nContent;
    xub_StrLen nStart = rIdx.GetIndex();

    // hinter das Zeichen (zum aufspannen der Attribute !!)
    if( nStart < pNode->GetTxt().Len() )
        lcl_SkipAttr( pNode, rIdx, nStart );

    if( DoesUndo() )
    {
        ClearRedo();
        sal_uInt16 nUndoSize = pUndos->Count();
        SwUndo * pUndo;
        if( DoesGroupUndo() && nUndoSize-- &&
            UNDO_OVERWRITE == ( pUndo = (*pUndos)[ nUndoSize ])->GetId() &&
            ((SwUndoOverwrite*)pUndo)->CanGrouping( this, rPt, c ))
            ;// wenn CanGrouping() sal_True returnt, ist schon alles erledigt
        else
            AppendUndo( new SwUndoOverwrite( this, rPt, c ));
    }
    else
    {
        // hinter das Zeichen (zum aufspannen der Attribute !!)
        if( nStart < pNode->GetTxt().Len() )
            rIdx++;
        pNode->Insert( c, rIdx );
        if( nStart+1 < rIdx.GetIndex() )
        {
            rIdx = nStart;
            pNode->Erase( rIdx, 1 );
            rIdx++;
        }
    }

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

    if( !DoesUndo() && !IsIgnoreRedline() && GetRedlineTbl().Count() )
    {
        SwPaM aPam( rPt.nNode, nStart, rPt.nNode, rPt.nContent.GetIndex() );
        DeleteRedline( aPam );
    }
    else if( IsRedlineOn() )
    {
        SwPaM aPam( rPt.nNode, nStart, rPt.nNode, rPt.nContent.GetIndex() );
        AppendRedline( new SwRedline( REDLINE_INSERT, aPam ));
    }

    SetModified();
    return sal_True;
}

sal_Bool SwDoc::Overwrite( const SwPaM &rRg, const String &rStr )
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

    if( DoesUndo() )
        ClearRedo();

    sal_uInt16 nOldAttrCnt = pNode->GetpSwpHints()
                                ? pNode->GetpSwpHints()->Count() : 0;
    SwDataChanged aTmp( rRg, 0 );
    SwIndex& rIdx = rPt.nContent;
    xub_StrLen nStart;

    sal_uInt16 nUndoSize = pUndos->Count();
    SwUndo * pUndo;
    sal_Unicode c;
    String aStr;

    for( xub_StrLen nCnt = 0; nCnt < rStr.Len(); ++nCnt )
    {
        // hinter das Zeichen (zum aufspannen der Attribute !!)
        if( (nStart = rIdx.GetIndex()) < pNode->GetTxt().Len() )
            lcl_SkipAttr( pNode, rIdx, nStart );
        c = rStr.GetChar( nCnt );
        if( DoesUndo() )
        {
            if( DoesGroupUndo() && nUndoSize &&
                UNDO_OVERWRITE == ( pUndo = (*pUndos)[ nUndoSize-1 ])->GetId() &&
                ((SwUndoOverwrite*)pUndo)->CanGrouping( this, rPt, c ))
                ;// wenn CanGrouping() sal_True returnt, ist schon alles erledigt
            else
            {
                AppendUndo( new SwUndoOverwrite( this, rPt, c ));
                nUndoSize = pUndos->Count();
            }
        }
        else
        {
            // hinter das Zeichen (zum Aufspannen der Attribute !!)
            if( nStart < pNode->GetTxt().Len() )
                rIdx++;
            pNode->Insert( c, rIdx );
            if( nStart+1 < rIdx.GetIndex() )
            {
                rIdx = nStart;
                pNode->Erase( rIdx, 1 );
                rIdx++;
            }
        }
    }

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

    if( !DoesUndo() && !IsIgnoreRedline() && GetRedlineTbl().Count() )
    {
        SwPaM aPam( rPt.nNode, nStart, rPt.nNode, rPt.nContent.GetIndex() );
        DeleteRedline( aPam );
    }
    else if( IsRedlineOn() )
    {
        SwPaM aPam( rPt.nNode, nStart, rPt.nNode, rPt.nContent.GetIndex() );
        AppendRedline( new SwRedline( REDLINE_INSERT, aPam ));
    }

    SetModified();
    return sal_True;
}


sal_Bool SwDoc::MoveAndJoin( SwPaM& rPaM, SwPosition& rPos, SwMoveFlags eMvFlags )
{
    SwNodeIndex aIdx( rPaM.Start()->nNode );
    sal_Bool bJoinTxt = aIdx.GetNode().IsTxtNode();
    sal_Bool bOneNode = rPaM.GetPoint()->nNode == rPaM.GetMark()->nNode;
    aIdx--;             // vor den Move Bereich !!

    sal_Bool bRet = Move( rPaM, rPos, eMvFlags );
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

sal_Bool SwDoc::Move( SwPaM& rPaM, SwPosition& rPos, SwMoveFlags eMvFlags )
{
    // keine Moves-Abfangen
    const SwPosition *pStt = rPaM.Start(), *pEnd = rPaM.End();
    if( !rPaM.HasMark() || *pStt >= *pEnd || (*pStt <= rPos && rPos < *pEnd))
        return sal_False;

    // sicher die absatzgebundenen Flys, damit sie verschoben werden koennen.
    _SaveFlyArr aSaveFlyArr;
    _SaveFlyInRange( rPaM, rPos.nNode, aSaveFlyArr, DOC_MOVEALLFLYS & eMvFlags );

    int bUpdateFtn = sal_False;
    SwFtnIdxs aTmpFntIdx;

    // falls Undo eingeschaltet, erzeuge das UndoMove-Objekt
    SwUndoMove * pUndoMove = 0;
    if( DoesUndo() )
    {
        ClearRedo();
        pUndoMove = new SwUndoMove( rPaM, rPos );
    }
    else
    {
        bUpdateFtn = lcl_SaveFtn( pStt->nNode, pEnd->nNode, rPos.nNode,
                                    GetFtnIdxs(), aTmpFntIdx,
                                    &pStt->nContent, &pEnd->nContent );
    }

    sal_Bool bSplit = sal_False;
    SwPaM * pSavePam = new SwPaM( rPos, rPos );

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

        pTNd = (SwTxtNode*)pTNd->SplitNode( rPos );

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
    sal_Bool bNullCntnt = !pSavePam->Move( fnMoveBackward, fnGoCntnt );
    if( bNullCntnt )
        pSavePam->GetPoint()->nNode--;

    // kopiere alle Bookmarks, die im Move Bereich stehen in ein
    // Array, das alle Angaben auf die Position als Offset speichert.
    // Die neue Zuordung erfolgt nach dem Moven.
    SaveBookmarks aSaveBkmk;
    _DelBookmarks( pStt->nNode, pEnd->nNode, &aSaveBkmk,
                   &pStt->nContent, &pEnd->nContent );

    // falls durch die vorherigen Loeschungen (z.B. der Fussnoten) kein
    // Bereich mehr existiert, ist das immernoch ein gueltiger Move!
    if( *rPaM.GetPoint() != *rPaM.GetMark() )
    {
        // jetzt kommt das eigentliche Verschieben
        GetNodes().Move( rPaM, rPos, GetNodes() );

        if( rPaM.HasMark() )        // es wurde kein Move ausgefuehrt !!
        {
            delete pSavePam;
            delete pUndoMove;
            return sal_False;           // Nach einem Move() ist der GetMark geloescht
        }
    }
    else
        rPaM.DeleteMark();

    ASSERT( *pSavePam->GetMark() == rPos,
            "PaM wurde nicht verschoben, am Anfang/Ende keine ContentNodes?" );
    *pSavePam->GetMark() = rPos;

    rPaM.SetMark();         // um den neuen Bereich eine Sel. aufspannen
    pTNd = pSavePam->GetNode()->GetTxtNode();
    if( DoesUndo() )
    {
        // korrigiere erstmal den Content vom SavePam
        if( bNullCntnt )
            pSavePam->GetPoint()->nContent = 0;

        // die Methode SwEditShell::Move() fuegt nach dem Move den Text-Node
        // zusammen, in dem der rPaM steht. Wurde der Inhalt nach hinten
        // geschoben und liegt der SPoint vom SavePam im naechsten Node, so
        // muss beim Speichern vom Undo-Object das beachtet werden !!
        SwTxtNode * pPamTxtNd;

        // wird ans SwUndoMove weitergegeben, das dann beim Undo JoinNext
        // aufruft. (falls es hier nicht moeglich ist).
        sal_Bool bJoin = bSplit && pTNd;
        bCorrSavePam = bCorrSavePam &&
                        0 != ( pPamTxtNd = rPaM.GetNode()->GetTxtNode() )
                        && pPamTxtNd->CanJoinNext()
                        && *rPaM.GetPoint() <= *pSavePam->GetPoint();

        // muessen am SavePam 2 Nodes zusammengefasst werden ??
        if( bJoin && pTNd->CanJoinNext() )
        {
            pTNd->JoinNext();
            // kein temp. sdbcx::Index bei &&
            // es sollten wohl nur die Indexwerte verglichen werden.
            if( bCorrSavePam && rPaM.GetPoint()->nNode.GetIndex()+1 ==
                                pSavePam->GetPoint()->nNode.GetIndex() )
                pSavePam->GetPoint()->nContent += pPamTxtNd->Len();
            bJoin = sal_False;
        }
//      else if( !bCorrSavePam && !pSavePam->Move( fnMoveForward, fnGoCntnt ))
        else if( !pSavePam->Move( fnMoveForward, fnGoCntnt ))
            pSavePam->GetPoint()->nNode++;

        // zwischen SPoint und GetMark steht jetzt der neu eingefuegte Bereich
        pUndoMove->SetDestRange( *pSavePam, *rPaM.GetPoint(),
                                    bJoin, bCorrSavePam );
        AppendUndo( pUndoMove );
    }
    else
    {
        // muessen am SavePam 2 Nodes zusammengefasst werden ??
        if( bSplit && pTNd )
        {
            if( pTNd->CanJoinNext())
                pTNd->JoinNext();
        }
        if( bNullCntnt )
        {
            pSavePam->GetPoint()->nNode++;
            pSavePam->GetPoint()->nContent.Assign( pSavePam->GetCntntNode(), 0 );
        }
        else
            pSavePam->Move( fnMoveForward, fnGoCntnt );
    }

    // setze jetzt wieder die text::Bookmarks in das Dokument
    *rPaM.GetMark() = *pSavePam->Start();
    for( sal_uInt16 n = 0; n < aSaveBkmk.Count(); ++n )
        aSaveBkmk[n]->SetInDoc( this, rPaM.GetMark()->nNode,
                                    &rPaM.GetMark()->nContent );
    *rPaM.GetPoint() = *pSavePam->End();
    delete pSavePam;

    // verschiebe die Flys an die neue Position
    _RestFlyInRange( aSaveFlyArr, rPaM.Start()->nNode );

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

sal_Bool SwDoc::Move( SwNodeRange& rRange, SwNodeIndex& rPos, SwMoveFlags eMvFlags )
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
    if( (DOC_CREATEUNDOOBJ & eMvFlags ) && DoesUndo() )
        pUndo = new SwUndoMove( this, rRange, rPos );
    else
        bUpdateFtn = lcl_SaveFtn( rRange.aStart, rRange.aEnd, rPos,
                                    GetFtnIdxs(), aTmpFntIdx );

    _SaveRedlines aSaveRedl( 0, 4 );
    SvPtrarr aSavRedlInsPosArr( 0, 4 );
    if( DOC_MOVEREDLINES & eMvFlags && GetRedlineTbl().Count() )
    {
        lcl_SaveRedlines( rRange, aSaveRedl );

        // suche alle Redlines, die an der InsPos aufhoeren. Diese muessen
        // nach dem Move wieder an die "alte" Position verschoben werden
        sal_uInt16 nRedlPos = GetRedlinePos( rPos.GetNode() );
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
    SaveBookmarks aSaveBkmk;
    _DelBookmarks( rRange.aStart, rRange.aEnd, &aSaveBkmk );

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
    if( GetNodes()._MoveNodes( rRange, GetNodes(), rPos ) )
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
        _RestFlyInRange( aSaveFlyArr, aIdx );

    // setze jetzt wieder die text::Bookmarks in das Dokument
    for( sal_uInt16 nCnt = 0; nCnt < aSaveBkmk.Count(); ++nCnt )
        aSaveBkmk[nCnt]->SetInDoc( this, aIdx );

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
        ClearRedo();
        pUndo->SetDestRange( aIdx, rPos, *pSaveInsPos );
        AppendUndo( pUndo );
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

void lcl_GetJoinFlags( SwPaM& rPam, sal_Bool& rJoinTxt, sal_Bool& rJoinPrev )
{
    if( rPam.GetPoint()->nNode != rPam.GetMark()->nNode )
    {
        const SwPosition* pStt = rPam.Start(), *pEnd = rPam.End();
        SwTxtNode* pTxtNd = pStt->nNode.GetNode().GetTxtNode();
        rJoinTxt = 0 != pTxtNd;

        if( rJoinTxt && pStt == rPam.GetPoint() &&
            0 != ( pTxtNd = pEnd->nNode.GetNode().GetTxtNode() ) &&
            pTxtNd->GetTxt().Len() == pEnd->nContent.GetIndex() )
        {
            rPam.Exchange();
            rJoinPrev = sal_False;
        }
        else
            rJoinPrev = rJoinTxt && rPam.GetPoint() == pStt;
    }
    else
        rJoinTxt = sal_False, rJoinPrev = sal_False;
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
            {
                // falls PageBreaks geloescht / gesetzt werden, darf das
                // nicht in die Undo-History aufgenommen werden !!
                // (das loeschen vom Node geht auch am Undo vorbei !!!)
                sal_Bool bDoUndo = pDoc->DoesUndo();
                pDoc->DoUndo( sal_False );

                /* PageBreaks, PageDesc, ColumnBreaks */
                // Sollte an der Logik zum Kopieren der PageBreak's ...
                // etwas geaendert werden, muss es auch im SwUndoDelete
                // geandert werden. Dort wird sich das AUTO-PageBreak
                // aus dem GetMarkNode kopiert.!!!

                /* Der GetMarkNode */
                if( ( pTxtNd = aIdx.GetNode().GetTxtNode())->GetpSwAttrSet() )
                {
                    const SfxPoolItem* pItem;
                    if( SFX_ITEM_SET == pTxtNd->GetpSwAttrSet()->GetItemState(
                        RES_BREAK, sal_False, &pItem ) )
                        pTxtNd->ResetAttr( RES_BREAK );
                    if( pTxtNd->GetpSwAttrSet() &&
                        SFX_ITEM_SET == pTxtNd->GetpSwAttrSet()->GetItemState(
                        RES_PAGEDESC, sal_False, &pItem ) )
                        pTxtNd->ResetAttr( RES_PAGEDESC );
                }

                /* Der PointNode */
                if( pOldTxtNd->GetpSwAttrSet() )
                {
                    const SfxPoolItem* pItem;
                    SfxItemSet aSet( pDoc->GetAttrPool(), aBreakSetRange );
                    SfxItemSet* pSet = pOldTxtNd->GetpSwAttrSet();
                    if( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK,
                        sal_False, &pItem ) )
                        aSet.Put( *pItem );
                    if( SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC,
                        sal_False, &pItem ) )
                        aSet.Put( *pItem );
                    if( aSet.Count() )
                        pTxtNd->SwCntntNode::SetAttr( aSet );
                }
                pOldTxtNd->FmtToTxtAttr( pTxtNd );

                SvULongs aBkmkArr( 15, 15 );
                ::_SaveCntntIdx( pDoc, aOldIdx.GetIndex(),
                                    pOldTxtNd->Len(), aBkmkArr );

                SwIndex aAlphaIdx(pTxtNd);
                pOldTxtNd->Cut( pTxtNd, aAlphaIdx, SwIndex(pOldTxtNd),
                                    pOldTxtNd->Len() );
                SwPosition aAlphaPos( aIdx, aAlphaIdx );
                pDoc->CorrRel( rPam.GetPoint()->nNode, aAlphaPos, 0, sal_True );

                // verschiebe noch alle Bookmarks/TOXMarks
                if( aBkmkArr.Count() )
                    ::_RestoreCntntIdx( pDoc, aBkmkArr, aIdx.GetIndex() );

                pDoc->DoUndo( bDoUndo );

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
            else if( pDelNd->GetpSwAttrSet() )
            {
                // nur die Zeichenattribute kopieren
                SfxItemSet aTmpSet( pDoc->GetAttrPool(), aCharFmtSetRange );
                aTmpSet.Put( *pDelNd->GetpSwAttrSet() );
                pTxtNd->SwCntntNode::SetAttr( aTmpSet );
            }

            pDoc->CorrRel( aIdx, *rPam.GetPoint(), 0, sal_True );
            pTxtNd->JoinNext();
        }
    }
}

sal_Bool SwDoc::DeleteAndJoin( SwPaM & rPam )
{
    if( IsRedlineOn() )
    {
        sal_uInt16 nUndoSize = 0;
        SwUndoRedlineDelete* pUndo = 0;
        SwRedlineMode eOld = GetRedlineMode();
        if( DoesUndo() )
        {
            ClearRedo();

//JP 06.01.98: MUSS noch optimiert werden!!!
SetRedlineMode( REDLINE_ON | REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE );

            nUndoSize = pUndos->Count();
            StartUndo();
            AppendUndo( pUndo = new SwUndoRedlineDelete( rPam, UNDO_DELETE ));
        }
        AppendRedline( new SwRedline( REDLINE_DELETE, rPam ));
        SetModified();

        if( pUndo )
        {
            EndUndo();
            SwUndo* pPrevUndo;
            if( nUndoSize && DoesGroupUndo() &&
                nUndoSize + 1 == pUndos->Count() &&
                UNDO_REDLINE == ( pPrevUndo = (*pUndos)[ nUndoSize-1 ])->GetId() &&
                UNDO_DELETE == ((SwUndoRedline*)pPrevUndo)->GetUserId() &&
                ((SwUndoRedlineDelete*)pPrevUndo)->CanGrouping( *pUndo ))
            {
                DoUndo( sal_False );
                pUndos->DeleteAndDestroy( nUndoSize, 1 );
                --nUndoPos, --nUndoCnt;
                DoUndo( sal_True );
            }
//JP 06.01.98: MUSS noch optimiert werden!!!
SetRedlineMode( eOld );
        }
        return sal_True;
    }

    sal_Bool bJoinTxt, bJoinPrev;
    lcl_GetJoinFlags( rPam, bJoinTxt, bJoinPrev );

    {
        // dann eine Kopie vom Cursor erzeugen um alle Pams aus den
        // anderen Sichten aus dem Loeschbereich zu verschieben
        // ABER NICHT SICH SELBST !!
        SwPaM aDelPam( *rPam.GetMark(), *rPam.GetPoint() );
        ::PaMCorrAbs( aDelPam, *aDelPam.GetPoint() );

        if( !Delete( aDelPam ) )
            return sal_False;

        *rPam.GetPoint() = *aDelPam.GetPoint();
    }

    if( bJoinTxt )
        lcl_JoinText( rPam, bJoinPrev );

    return sal_True;
}

sal_Bool SwDoc::Delete( SwPaM & rPam )
{
    SwPosition *pStt = (SwPosition*)rPam.Start(), *pEnd = (SwPosition*)rPam.End();

    if( !rPam.HasMark() || *pStt >= *pEnd )
        return sal_False;

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


    if( DoesUndo() )
    {
        ClearRedo();
        sal_uInt16 nUndoSize = pUndos->Count();
        SwUndo * pUndo;
        if( DoesGroupUndo() && nUndoSize-- &&
            UNDO_DELETE == ( pUndo = (*pUndos)[ nUndoSize ])->GetId() &&
            ((SwUndoDelete*)pUndo)->CanGrouping( this, rPam ))
            ;// wenn CanGrouping() sal_True returnt, ist schon alles erledigt
        else
            AppendUndo( new SwUndoDelete( rPam ) );

        SetModified();
        return sal_True;
    }

    if( !IsIgnoreRedline() && GetRedlineTbl().Count() )
        DeleteRedline( rPam );

    // loesche und verschiebe erstmal alle "Fly's am Absatz", die in der
    // SSelection liegen
    DelFlyInRange( rPam.GetMark()->nNode, rPam.GetPoint()->nNode );
    _DelBookmarks( pStt->nNode, pEnd->nNode, 0,
                       &pStt->nContent, &pEnd->nContent );

    SwNodeIndex aSttIdx( pStt->nNode );
    SwCntntNode * pCNd = aSttIdx.GetNode().GetCntntNode();

    do {        // middle checked loop!
        if( pCNd )
        {
            if( pCNd->GetTxtNode() )
            {
                // verschiebe jetzt noch den Inhalt in den neuen Node
                sal_Bool bOneNd = pStt->nNode == pEnd->nNode;
                xub_StrLen nLen = ( bOneNd ? pEnd->nContent.GetIndex()
                                           : pCNd->Len() )
                                        - pStt->nContent.GetIndex();

                // falls schon leer, dann nicht noch aufrufen
                if( nLen )
                    ((SwTxtNode*)pCNd)->Erase( pStt->nContent, nLen );

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

        sal_uInt32 nEnde = pEnd->nNode.GetIndex();
        pCNd = pEnd->nNode.GetNode().GetCntntNode();
        if( pCNd )
        {
            if( pCNd->GetTxtNode() )
            {
                // falls schon leer, dann nicht noch aufrufen
                if( pEnd->nContent.GetIndex() )
                {
                    SwIndex aIdx( pCNd, 0 );
                    ((SwTxtNode*)pCNd)->Erase( aIdx, pEnd->nContent.GetIndex() );
                }
                nEnde--;
            }
            else
            {
                // damit beim Loeschen keine Indizies mehr angemeldet sind,
                // wird hier der SwPaM aus dem Content entfernt !!
                pEnd->nContent.Assign( 0, 0 );
                nEnde--;
            }
        }

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
    return sal_True;
}


uno::Reference< uno::XInterface >  SwDoc::Spell( SwPaM& rPaM,
                    uno::Reference< linguistic::XSpellChecker1 >  &xSpeller,
                    sal_uInt16* pPageCnt, sal_uInt16* pPageSt ) const
{
    SwPosition* pSttPos = rPaM.Start(), *pEndPos = rPaM.End();
    uno::Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );
    sal_Bool bReverse = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue( S2U(UPN_IS_WRAP_REVERSE) ).getValue() : sal_False;

    SwSpellArgs aSpellArg( xSpeller,
        pSttPos->nNode.GetNode().GetTxtNode(), pSttPos->nContent,
        pEndPos->nNode.GetNode().GetTxtNode(), pEndPos->nContent );

    sal_uInt32 nCurrNd = pSttPos->nNode.GetIndex();
    sal_uInt32 nEndNd = pEndPos->nNode.GetIndex();

    if( nCurrNd <= nEndNd )
    {
        SwCntntFrm* pCntFrm;
        sal_uInt32 nCount = nEndNd - nCurrNd;
        if( bReverse )
        {
            nCurrNd = nEndNd;
            nEndNd = nCurrNd - nCount;
        }
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
                        nCurrNd = bReverse ? pNd->StartOfSectionIndex()
                                           : pNd->EndOfSectionIndex();
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
                                nStat = bReverse ?
                                    *pPageCnt - nPageNr + *pPageSt + 1 :
                                    nPageNr - *pPageSt + 1;
                            else
                                nStat = bReverse ?
                                    *pPageSt - nPageNr + 1 :
                                    nPageNr + *pPageCnt - *pPageSt + 1;
                            ::SetProgressState( nStat, (SwDocShell*)GetDocShell() );
                        }
                        if( ((SwTxtNode*)pNd)->Spell( &aSpellArg ) )
                        {
                            // Abbrechen und Position merken
                            pSttPos->nNode = nCurrNd;
                            pEndPos->nNode = nCurrNd;
                            nCurrNd = nEndNd;
                        }
                    }
                }
                break;
            case ND_SECTIONNODE:
                if( !bReverse &&
                    ( ((SwSectionNode*)pNd)->GetSection().IsProtect() ||
                    ((SwSectionNode*)pNd)->GetSection().IsHidden() ) )
                    nCurrNd = pNd->EndOfSectionIndex();
                break;
            case ND_ENDNODE:
                {
                    SwNode* pTmp;
                    if( bReverse && 0 != (pTmp = pNd->StartOfSectionNode() ) &&
                        ND_SECTIONNODE == pTmp->GetNodeType() &&
                        ( ((SwSectionNode*)pTmp)->GetSection().IsProtect() ||
                            ((SwSectionNode*)pTmp)->GetSection().IsHidden() ) )
                        nCurrNd = pNd->StartOfSectionIndex();
                    break;
                }
            }

            if( bReverse )
            {
                bGoOn = nCurrNd > nEndNd;
                if( bGoOn )
                    --nCurrNd;
            }
            else
            {
                bGoOn = nCurrNd < nEndNd;
                ++nCurrNd;
            }
        }
    }
    return aSpellArg.xSpellAlt;
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

    const SwNodes &rNds = pPam->GetDoc()->GetNodes();
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
        nPamLen -= pPoint->nContent.GetIndex();
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

uno::Reference< linguistic::XHyphenatedWord >  SwDoc::Hyphenate(
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

void ReplaceTabsStr( String& rStr, const String& rSrch, const String& rRepl )
{
    xub_StrLen nPos = 0;
    while( STRING_NOTFOUND != ( nPos = rStr.Search( rSrch, nPos )) )
    {
        // wurde das escaped?
        if( nPos && '\\' == rStr.GetChar( nPos-1 ))
        {
            // noch nicht am Ende ??
            rStr.Erase( nPos-1, 1 );        // den \\ noch loeschen
            if( nPos >= rStr.Len() )
                break;
        }
        else
        {
            rStr.Erase( nPos, rSrch.Len() );
            rStr.Insert( rRepl, nPos );
            nPos += rRepl.Len();
        }
    }
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

sal_Bool SwDoc::Replace( SwPaM& rPam, const String& rStr, sal_Bool bRegExpRplc )
{
    if( !rPam.HasMark() || *rPam.GetPoint() == *rPam.GetMark() )
        return sal_False;

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
                "Point & Mark zeigen auf verschiedene Nodes" );
        sal_Bool bOneNode = pStt->nNode == pEnd->nNode;

        // eigenes Undo ????
        String sRepl( rStr );
        SwTxtNode* pTxtNd = pStt->nNode.GetNode().GetTxtNode();
        xub_StrLen nStt = pStt->nContent.GetIndex(),
                nEnd = bOneNode ? pEnd->nContent.GetIndex()
                                : pTxtNd->GetTxt().Len();
        if( bRegExpRplc )       // regulaer suchen ??
        {
            String sFndStr( pTxtNd->GetTxt().Copy( nStt, nEnd - nStt ));

//JP 31.03.99: was ist, wenn im gefundenem anderer Inhalt als Text ist,
//              wie z.B. Rahmen, Felder, Fussnoten, ... ???
//          Die aktuelle Implementierung entfernt diese Inhalte einfach.
//          Eigentlich muss der Inhalt immer kopiert werden!
            sFndStr.EraseAllChars( CH_TXTATR_BREAKWORD );
            sFndStr.EraseAllChars( CH_TXTATR_INWORD );

            String sTmp( String::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "\\t" )));
            ReplaceTabsStr( sRepl, String( '&' ), sFndStr );
            ReplaceTabsStr( sRepl, sTmp, String( '\t' ) );
        }

        SwDataChanged aTmp( aDelPam, 0 );

        if( IsRedlineOn() )
        {
            SwRedlineMode eOld = GetRedlineMode();
            if( DoesUndo() )
            {
                StartUndo();

                // Bug 68584 - if any Redline will change (split!) the node
                String sNm; sNm = String::CreateFromInt32( (long)&aDelPam );
                SwBookmark* pBkmk = MakeBookmark( aDelPam,
                                KeyCode(), sNm, sNm, UNO_BOOKMARK );

//JP 06.01.98: MUSS noch optimiert werden!!!
SetRedlineMode( REDLINE_ON | REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE );

                *aDelPam.GetPoint() = pBkmk->GetPos();
                *aDelPam.GetMark() = *pBkmk->GetOtherPos();
                DelBookmark( GetBookmarks().GetPos( pBkmk ));
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

                if( aDelPam.GetPoint() != aDelPam.End() )
                    aDelPam.Exchange();

                // das Ende merken
                SwNodeIndex aPtNd( aDelPam.GetPoint()->nNode, -1 );
                xub_StrLen nPtCnt = aDelPam.GetPoint()->nContent.GetIndex();

                sal_Bool bFirst = sal_True;
                String sIns;
                while( lcl_GetTokenToParaBreak( sRepl, sIns, bRegExpRplc ))
                {
                    Insert( aDelPam, sIns );
                    if( bFirst )
                    {
                        SwNodeIndex aMkNd( aDelPam.GetMark()->nNode, -1 );
                        xub_StrLen nMkCnt = aDelPam.GetMark()->nContent.GetIndex();

                        SplitNode( *aDelPam.GetPoint() );

                        aMkNd++;
                        aDelPam.GetMark()->nNode = aMkNd;
                        aDelPam.GetMark()->nContent.Assign(
                                    aMkNd.GetNode().GetCntntNode(), nMkCnt );
                        bFirst = sal_False;
                    }
                    else
                        SplitNode( *aDelPam.GetPoint() );
                }
                if( sIns.Len() )
                    Insert( aDelPam, sIns );

                SwPaM aTmpRange( *aDelPam.GetPoint() );
                aTmpRange.SetMark();

                aPtNd++;
                aDelPam.GetPoint()->nNode = aPtNd;
                aDelPam.GetPoint()->nContent.Assign( aPtNd.GetNode().GetCntntNode(),
                                                    nPtCnt);
                *aTmpRange.GetMark() = *aDelPam.GetPoint();

                RstTxtAttr( aTmpRange );
                Insert( aTmpRange, aSet );
            }

            if( DoesUndo() )
                AppendUndo( new SwUndoRedlineDelete( aDelPam, UNDO_REPLACE ));
            AppendRedline( new SwRedline( REDLINE_DELETE, aDelPam ));

            if( DoesUndo() )
            {
                EndUndo();

                // Bug 68584 - if any Redline will change (split!) the node
                String sNm; sNm = String::CreateFromInt32( (long)&aDelPam );
                SwBookmark* pBkmk = MakeBookmark( aDelPam,
                                KeyCode(), sNm, sNm, UNO_BOOKMARK );

                SwIndex& rIdx = aDelPam.GetPoint()->nContent;
                rIdx.Assign( 0, 0 );
                aDelPam.GetMark()->nContent = rIdx;
                rPam.GetPoint()->nNode = 0;
                rPam.GetPoint()->nContent = rIdx;
                *rPam.GetMark() = *rPam.GetPoint();
//JP 06.01.98: MUSS noch optimiert werden!!!
SetRedlineMode( eOld );

                *rPam.GetPoint() = pBkmk->GetPos();
                *rPam.GetMark() = *pBkmk->GetOtherPos();
                DelBookmark( GetBookmarks().GetPos( pBkmk ));
            }
            bJoinTxt = sal_False;
        }
        else
        {
            if( !IsIgnoreRedline() && GetRedlineTbl().Count() )
                DeleteRedline( aDelPam );

            SwUndoReplace* pUndoRpl = 0;
            if( DoesUndo() )
            {
                ClearRedo();
                SwUndo* pU;

                if( !pUndos->Count() ||
                    UNDO_REPLACE != ( pU = (*pUndos)[ pUndos->Count()-1 ])->GetId() ||
                    ( pUndoRpl = (SwUndoReplace*)pU )->IsFull() )
                {
                    pUndoRpl = new SwUndoReplace();
                    AppendUndo( pUndoRpl );
                }
                pUndoRpl->AddEntry( aDelPam, sRepl, bRegExpRplc );
                DoUndo( sal_False );
            }

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
            while( lcl_GetTokenToParaBreak( sRepl, sIns, bRegExpRplc ))
            {
                if( !bFirst || nStt == pTxtNd->GetTxt().Len() )
                    Insert( aDelPam, sIns );
                else if( nStt < nEnd || sIns.Len() )
                    pTxtNd->Replace( pStt->nContent, nEnd - nStt, sIns );
                SplitNode( *pStt );
                bFirst = sal_False;
            }

            if( bFirst || sIns.Len() )
            {
                if( !bFirst || nStt == pTxtNd->GetTxt().Len() )
                    Insert( aDelPam, sIns );
                else if( nStt < nEnd || sIns.Len() )
                    pTxtNd->Replace( pStt->nContent, nEnd - nStt, sIns );
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
                pUndoRpl->SetEntryEnd( rPam );
                DoUndo( sal_True );
            }
        }
    }

    if( bJoinTxt )
        lcl_JoinText( rPam, bJoinPrev );

    SetModified();
    return sal_True;
}

    // speicher die akt. Werte fuer die automatische Aufnahme von Ausnahmen
    // in die Autokorrektur
void SwDoc::SetAutoCorrExceptWord( SwAutoCorrExceptWord* pNew )
{
    if( pACEWord && pNew != pACEWord )
        delete pACEWord;
    pACEWord = pNew;
}

sal_Bool SwDoc::DelFullPara( SwPaM& rPam )
{
    const SwPosition &rStt = *rPam.Start(), &rEnd = *rPam.End();
    const SwNode* pNd = &rStt.nNode.GetNode();
    sal_uInt32 nSectDiff = pNd->FindStartNode()->EndOfSectionIndex() -
                        pNd->StartOfSectionIndex();
    sal_uInt32 nNodeDiff = rEnd.nNode.GetIndex() - rStt.nNode.GetIndex();

    if( nSectDiff-2 <= nNodeDiff || IsRedlineOn() )
        return sal_False;

    // harte SeitenUmbrueche am nachfolgenden Node verschieben
    sal_Bool bSavePageBreak = sal_False, bSavePageDesc = sal_False;
    sal_uInt32 nNextNd = rEnd.nNode.GetIndex() + 1;
    SwTableNode* pTblNd = GetNodes()[ nNextNd ]->GetTableNode();
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
                pTableFmt->SetAttr( *pItem );
                bSavePageDesc = sal_True;
            }

            if( pSet && SFX_ITEM_SET == pSet->GetItemState( RES_BREAK,
                sal_False, &pItem ) )
            {
                pTableFmt->SetAttr( *pItem );
                bSavePageBreak = sal_True;
            }
        }
    }

    sal_Bool bDoesUndo = DoesUndo();
    if( bDoesUndo )
    {
        if( !rPam.HasMark() )
            rPam.SetMark();
        else if( rPam.GetPoint() == &rStt )
            rPam.Exchange();
        rPam.GetPoint()->nNode++;

        rPam.GetPoint()->nContent.Assign( 0, 0 );
        rPam.GetMark()->nContent.Assign( 0, 0 );

        ClearRedo();
        SwUndoDelete* pUndo = new SwUndoDelete( rPam, sal_True );
        pUndo->SetPgBrkFlags( bSavePageBreak, bSavePageDesc );
        AppendUndo( pUndo );
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
            const SwPosition* pAPos;
            for( sal_uInt16 n = 0; n < GetSpzFrmFmts()->Count(); ++n )
            {
                SwFrmFmt* pFly = (*GetSpzFrmFmts())[n];
                const SwFmtAnchor* pAnchor = &pFly->GetAnchor();
                if( ( FLY_AT_CNTNT == pAnchor->GetAnchorId() ||
                      FLY_AUTO_CNTNT == pAnchor->GetAnchorId() ) &&
                    0 != ( pAPos = pAnchor->GetCntntAnchor() ) &&
                    aRg.aStart <= pAPos->nNode && pAPos->nNode <= aRg.aEnd )
                {
                    DelLayoutFmt( pFly );
                    --n;
                }
            }
        }

        rPam.GetPoint()->nContent.Assign( 0, 0 );
        rPam.GetMark()->nContent.Assign( 0, 0 );
        GetNodes().Delete( aRg.aStart, nNodeDiff+1 );
    }
    rPam.DeleteMark();
    SetModified();
    return sal_True;
}



