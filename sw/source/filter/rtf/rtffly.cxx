/*************************************************************************
 *
 *  $RCSfile: rtffly.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:56 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _CACHESTR_HXX //autogen
#include <tools/cachestr.hxx>
#endif
#ifndef _RTFTOKEN_H
#include <svtools/rtftoken.h>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SVX_PRNTITEM_HXX //autogen
#include <svx/prntitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen wg. SvxBoxItem
#include <svx/boxitem.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _SWPARRTF_HXX
#include <swparrtf.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _RTF_HXX
#include <rtf.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _TXTFLCNT_HXX
#include <txtflcnt.hxx>
#endif
#ifndef _FMTFLCNT_HXX
#include <fmtflcnt.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif


#define ANCHOR(p)   ((SwFmtAnchor*)p)

// steht in shellio.hxx
extern SwCntntNode* GoNextNds( SwNodeIndex * pIdx, FASTBOOL bChk );

SV_IMPL_PTRARR( SwFlySaveArr, SwFlySave* )

inline const SwFmtFrmSize GetFrmSize(const SfxItemSet& rSet, BOOL bInP=TRUE)
    { return (const SwFmtFrmSize&)rSet.Get( RES_FRM_SIZE,bInP); }


/*  */


SwFlySave::SwFlySave( const SwPaM& rPam, SfxItemSet& rSet )
    : aFlySet( rSet ),
    nSttNd( rPam.GetPoint()->nNode ), nEndNd( nSttNd ),
    nEndCnt( 0 ), nPageWidth( ATT_MIN_SIZE ),
    nDropLines( 0 ), nDropAnchor( 0 )
{
}

int SwFlySave::IsEqualFly( const SwPaM& rPos, SfxItemSet& rSet )
{
    if( rSet.Count() != aFlySet.Count() || nDropAnchor )
        return FALSE;

    // nur TextNodes zusammenfassen
    if( nSttNd == nEndNd && nEndNd.GetNode().IsNoTxtNode() )
        return FALSE;

    // teste auf gleiche / naechste Position
    if( rPos.GetPoint()->nNode.GetIndex() == nEndNd.GetIndex() )
    {
        if( 1 < (rPos.GetPoint()->nContent.GetIndex() - nEndCnt) )
            return FALSE;
    }
    else if( rPos.GetPoint()->nContent.GetIndex() )
        return FALSE;
    else
    {
        SwNodeIndex aIdx( nEndNd );
        SwCntntNode* pCNd = rPos.GetDoc()->GetNodes()[ aIdx ]->GetCntntNode();
        if( !GoNextNds( &aIdx, TRUE ) ||
            aIdx.GetIndex() != rPos.GetPoint()->nNode.GetIndex() ||
            ( pCNd && pCNd->Len() != nEndCnt ))
        {
            return FALSE;
        }
    }

    if( rSet.Count() )
    {
        SfxItemIter aIter( rSet );
        const SfxPoolItem *pItem, *pCurr = aIter.GetCurItem();
        while( TRUE )
        {
            if( SFX_ITEM_SET != aFlySet.GetItemState( pCurr->Which(),
                FALSE, &pItem ) ||
                // Ankerattribute gesondert behandeln
                ( RES_ANCHOR == pCurr->Which()
                    ? (ANCHOR(pCurr)->GetAnchorId() != ANCHOR(pItem)->GetAnchorId() ||
                       ANCHOR(pCurr)->GetPageNum() != ANCHOR(pItem)->GetPageNum())
                    : *pItem != *pCurr ))
                        return FALSE;

            if( aIter.IsAtEnd() )
                break;
            pCurr = aIter.NextItem();
        }
    }
    return TRUE;
}

void SwFlySave::SetFlySize( const SwTableNode& rTblNd )
{
    // sollte der Fly kleiner als diese Tabelle sein, dann
    // korrigiere diesen (nur bei abs. Angaben!)
    SwTwips nWidth = rTblNd.GetTable().GetFrmFmt()->GetFrmSize().GetWidth();
    const SwFmtFrmSize& rSz = GetFrmSize( aFlySet );
    if( nWidth > rSz.GetWidth() )
        aFlySet.Put( SwFmtFrmSize( rSz.GetSizeType(), nWidth, rSz.GetHeight() ));
}

BOOL lcl_HasBreakAttrs( const SwCntntNode& rNd )
{
    BOOL bRet = FALSE;
    const SfxItemSet& rSet = rNd.GetSwAttrSet();
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( RES_BREAK, TRUE, &pItem ) &&
        SVX_BREAK_NONE != ((SvxFmtBreakItem*)pItem)->GetBreak() )
        bRet = TRUE;
    else if( SFX_ITEM_SET == rSet.GetItemState( RES_PAGEDESC, TRUE, &pItem )&&
         0 != ((SwFmtPageDesc*)pItem)->GetPageDesc() )
        bRet = TRUE;
    return bRet;
}


void lcl_CpyBreakAttrs( SwCntntNode* pSrcNd, SwCntntNode* pDstNd,
                        SwNodeIndex* pNewIdx )
{
    SwAttrSet* pSet;
    if( pSrcNd && pDstNd && 0 != ( pSet = pSrcNd->GetpSwAttrSet() ) )
    {
        const SfxPoolItem *pDescItem, *pBreakItem;

        if( SFX_ITEM_SET != pSet->GetItemState( RES_BREAK,
                                        FALSE, &pBreakItem ) )
            pBreakItem = 0;

        if( SFX_ITEM_SET != pSet->GetItemState( RES_PAGEDESC,
                                        FALSE, &pDescItem ) )
            pDescItem = 0;

        if( pDescItem || pBreakItem )
        {
            if( lcl_HasBreakAttrs( *pDstNd ))
            {
                SwPosition aPos( *pDstNd, SwIndex( pDstNd ));
                aPos.nNode--;
                pDstNd->GetDoc()->AppendTxtNode( aPos );
                if( pNewIdx )
                    *pNewIdx = aPos.nNode;

                SwCntntNode* pOldNd = pDstNd;
                pDstNd = aPos.nNode.GetNode().GetCntntNode();
                pDstNd->ChgFmtColl( pOldNd->GetFmtColl() );
                if( pDstNd->GetpSwAttrSet() )
                {
                    SfxItemSet aSet( *pDstNd->GetpSwAttrSet() );
                    aSet.ClearItem( RES_BREAK );
                    aSet.ClearItem( RES_PAGEDESC );
                    pDstNd->SetAttr( aSet );
                }
            }
            if( pBreakItem )
            {
                pDstNd->SetAttr( *pBreakItem );
                pSrcNd->ResetAttr( RES_BREAK );
            }
            if( pDescItem )
            {
                pDstNd->SetAttr( *pDescItem );
                pSrcNd->ResetAttr( RES_PAGEDESC );
            }
        }
    }
}


void SwRTFParser::SetFlysInDoc()
{
    // !! von Oben abarbeiten, CntntPos ist kein Index !
    SwNodes & rNds = pDoc->GetNodes();
    SvPtrarr aPrevFmts( 255 < aFlyArr.Count() ? aFlyArr.Count() : 255 );
    SwFrmFmt* pParent = pDoc->GetFrmFmtFromPool( RES_POOLFRM_FRAME );
    for( USHORT n = 0; n < aFlyArr.Count(); ++n )
    {
        SwFlySave* pFlySave = aFlyArr[ n ];

        ASSERT( !pFlySave->nSttNd.GetNode().FindFlyStartNode(),
                "Content vom Fly steht in einem Fly" );
        ASSERT( pFlySave->nSttNd.GetIndex() <= pFlySave->nEndNd.GetIndex(),
                "Fly hat falschen Bereich" );

        //JP 21.09.98: wenn ein DropCap ist, dann Text im Node belassen, am
        //              Absatz das Absatz Attribut setzen. Ggfs noch die
        //              FontSize zuruecksetzen, damit das DropCap nicht zu
        //              groá wird.
        if( pFlySave->nDropAnchor )
        {
            SwTxtNode* pSttNd = pFlySave->nSttNd.GetNode().GetTxtNode(),
                     * pEndNd = pFlySave->nEndNd.GetNode().GetTxtNode();
            if( pSttNd && pEndNd &&
                pSttNd->GetIndex() + 1 == pEndNd->GetIndex() )
            {
                BOOL bJoined;
                {
                    SwPaM aTmp( *pEndNd, 0, *pSttNd, pSttNd->GetTxt().Len() );
                    bJoined = pDoc->DeleteAndJoin( aTmp );
                }
                if( bJoined )
                {
                    SwFmtDrop aDropCap;
                    aDropCap.GetLines() = (BYTE)pFlySave->nDropLines;
                    aDropCap.GetChars() = 1;

                    SwIndex aIdx( pEndNd );
                    pEndNd->RstAttr( aIdx, 1, RES_CHRATR_FONTSIZE );
                    pEndNd->SwCntntNode::SetAttr( aDropCap );
                }
            }
            delete pFlySave;
            continue;
        }

        // liegt Ende und Start vom Naechsten im gleichen Node, dann muss
        // gesplittet werden
        if( n + 1 < aFlyArr.Count() && pFlySave->nEndCnt &&
            pFlySave->nEndNd == aFlyArr[ n + 1 ]->nSttNd )
        {
            SwCntntNode* pCNd = rNds[ pFlySave->nEndNd ]->GetCntntNode();
            if( pCNd )
            {
                SwPosition aPos( pFlySave->nEndNd,
                                SwIndex( pCNd, pFlySave->nEndCnt ));
                pDoc->SplitNode( aPos );
                pFlySave->nEndNd--;
            }
            else
                pFlySave->nEndCnt = 0;
        }

        // verschiebe den Inhalt von diesem Anchor in den Auto-TextBereich
        // und erzeuge dadurch den richtigen SwG-Rahmen
        SwNodeRange aRg( pFlySave->nSttNd, 0, pFlySave->nEndNd,
                        (pFlySave->nEndCnt ? 1 : 0) );
        BOOL bMakeEmptySection = aRg.aStart < aRg.aEnd;

        {
            // Nur TextNodes koennen in Tabellen stehen !!
            const SwNode* pNd = &pFlySave->nSttNd.GetNode();
            if( pNd->IsNoTxtNode() )
            {
                // die Size muss noch korrigiert werden!
                nAktPageDesc = 0;       // Standart PageDesc
                if( SFX_ITEM_SET != pFlySave->aFlySet.GetItemState(
                    RES_FRM_SIZE, FALSE ) )
                    _SetPictureSize( *(SwNoTxtNode*)pNd, aRg.aStart,
                                    pFlySave->aFlySet );
                if( 0 != ( pNd = pNd->FindTableNode() ) )
                    pFlySave->SetFlySize( *(SwTableNode*)pNd );
            }
            else
            {
                // TabelleNodes beachten
                BOOL bMoveFromCell = FALSE;
                pNd = pNd->FindTableNode();
                if( pNd )   // am Anfang eine Tabelle, -> Bereich auf TabStart
                {
                    if( pFlySave->nSttNd.GetNode().FindStartNode() ==
                        pFlySave->nEndNd.GetNode().FindStartNode() )
                    {
                        // dann nur den Node verschieben
                        SwNodeIndex& rIdx = pFlySave->nSttNd;
                        if( rIdx.GetNode().EndOfSectionIndex() -
                            rIdx.GetNode().FindStartNode()->GetIndex() ==
                            aRg.aEnd.GetIndex() - aRg.aStart.GetIndex() + 1 )
                        {
                            // dann bliebe kein Node mehr stehen, also
                            // erzeuge eine leere Text Section!
                            bMakeEmptySection = FALSE;
                        }
                        bMoveFromCell = TRUE;
                    }
                    else
                        aRg.aStart = *pNd;
                }

                if( bMakeEmptySection )
                {
                    pNd = &aRg.aEnd.GetNode();
                    ULONG nSectEnd = pNd->EndOfSectionIndex();
                    if( !pFlySave->nEndCnt )
                        ++nSectEnd;

                    if( !bMoveFromCell && !pNd->IsTableNode() &&
                        0 != (pNd = pNd->FindTableNode() ) )
                    {
                        const SwNode* pTblBxNd;
                        // Ende der Tabelle ist hinter dieser Box ??
                        if( pNd->EndOfSectionIndex() == nSectEnd )
                            aRg.aEnd = nSectEnd+1;
                        // is the end in the first box of the table, then
                        // move before the table (Bug 67663)
                        else if( 0 != ( pTblBxNd = aRg.aEnd.GetNode().
                                                FindTableBoxStartNode()) &&
                                 pTblBxNd->GetIndex() - 1 == pNd->GetIndex())
                            aRg.aEnd = *pNd;
                        else
                        {
                            // Tabelle ist noch groesser, also splitte sie hier.
                            rNds.SplitTable( aRg.aEnd, TRUE );
                            aRg.aEnd = pNd->EndOfSectionIndex() + 1;
                        }
                    }
                }
            }
        }

        // vorm verschieben muss sich der Index auf die alte Position
        // gemerkt werden, der Index wird mit verschoben !!!

        SwNodeIndex aTmpIdx( rNds.GetEndOfAutotext() );
        SwStartNode* pSttNd = bMakeEmptySection
                ? rNds.MakeEmptySection( aTmpIdx, SwFlyStartNode )
                : rNds.MakeTextSection( aTmpIdx, SwFlyStartNode,
                        (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl() );

        // das ist die Verankerungs-Position (fuers Layout!)
        pFlySave->nSttNd = aRg.aStart.GetIndex()-1;
        if( bMakeEmptySection )
        {
            // check: the move does not clear the surrounded section. If all
            // nodes moved away, then create a new TxtNode
            {
                SwNodeIndex aPrev( aRg.aStart, -1 );
                if( aPrev.GetNode().IsStartNode() &&
                    aPrev.GetNode().EndOfSectionNode() == &aRg.aEnd.GetNode())
                {
                    // create new txtnode, because the section does never be empty
                    pDoc->GetNodes().MakeTxtNode( aRg.aEnd,
                            (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl() );
                    aRg.aEnd--;
                }
            }
            aTmpIdx = *pSttNd->EndOfSectionNode();
            pDoc->Move( aRg, aTmpIdx );
        }

        pFlySave->aFlySet.Put( SwFmtCntnt( pSttNd ));

        CalculateFlySize( pFlySave->aFlySet, pFlySave->nSttNd,
                          pFlySave->nPageWidth );

        // if the section only contains one Node and this has a
        // border or backgorund, then put it to the frame
        // Not in our own RTF-Format!
        if( pSttNd->GetIndex() + 2 == pSttNd->EndOfSectionIndex() &&
            !bSwPageDesc )
        {
            SwTxtNode* pSrcNd = pDoc->GetNodes()[ pSttNd->GetIndex() + 1 ]
                                    ->GetTxtNode();
            if( pSrcNd && pSrcNd->GetpSwAttrSet() )
            {
                SfxItemSet aTmpSet( pDoc->GetAttrPool(),
                                    RES_BACKGROUND, RES_BOX );
                aTmpSet.Put( *pSrcNd->GetpSwAttrSet() );
                if( aTmpSet.Count() )
                {
                    pFlySave->aFlySet.Put( aTmpSet );
                    pSrcNd->ResetAttr( RES_BACKGROUND, RES_BOX );
                }
            }
        }

        SwFlyFrmFmt* pFmt = pDoc->MakeFlyFrmFmt( aEmptyStr, pParent );
        pFmt->SetAttr( pFlySave->aFlySet );

        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        if( FLY_IN_CNTNT != rAnchor.GetAnchorId() )
        {
            // korrigiere noch den Absatz, ist immer der vorhergehende !
            // JP 20.09.95: wenn es diesen gibt! (DocAnfang!)

            //JP 02.08.99: that is wrong. The anchor is ever the NEXT!
            //JP 05.08.99: there are an Bug in the ExportFilter which will
            //              be fixed in the Version 517 - by SWG-Export
            //              the fly will be after the paragraph - but in RTF
            //              the flys will be before the paragraph.
            if( !bSwPageDesc || 5430 < GetVersionNo() )
                pFlySave->nSttNd++;

//            if( !pFlySave->nSttNd.GetNode().IsCntntNode() )
            {
                // Seitenumbrueche in den Bodybereich verschieben!
                SwCntntNode* pSrcNd = aRg.aStart.GetNode().GetCntntNode();
                SwCntntNode* pDstNd = pFlySave->nSttNd.GetNode().GetCntntNode();
                if( !pDstNd )
                    pDstNd = pDoc->GetNodes().GoNext( &pFlySave->nSttNd );

                ::lcl_CpyBreakAttrs( pSrcNd, pDstNd, &pFlySave->nSttNd );
            }


            SwPosition aPos( pFlySave->nSttNd );
            SwFmtAnchor aAnchor( rAnchor );
            aAnchor.SetAnchor( &aPos );
            pFmt->SetAttr( aAnchor );

            // check if the new Frame contains any anchors to previous
            // flys. This is in RTF not allowed, so move all anchors to
            // the prev position
            {
                ULONG nSttNd = pSttNd->GetIndex(),
                      nEndNd = pSttNd->EndOfSectionIndex();
                for( USHORT nPrevFmts = aPrevFmts.Count(); nPrevFmts; )
                {
                    SwFmt* pTmpFmt = (SwFmt*)aPrevFmts[ --nPrevFmts ];
                    const SwFmtAnchor& rAn = pTmpFmt->GetAnchor();
                    ULONG nNd = rAn.GetCntntAnchor()->nNode.GetIndex();
                    if( nSttNd < nNd && nNd < nEndNd )
                    {
                        SwFmtAnchor aAnchor( rAn );
                        aAnchor.SetAnchor( &aPos );
                        pTmpFmt->SetAttr( aAnchor );
                    }
                    else
                        // then forget it
                        aPrevFmts.Remove( nPrevFmts, 1 );
                }

                void* p = (void*)pFmt;
                aPrevFmts.Insert( p, aPrevFmts.Count() );
            }
        }
        delete pFlySave;
    }
    aFlyArr.Remove( 0, aFlyArr.Count() );
}

void SwRTFParser::ReadFly( int nToken, SfxItemSet* pSet )
{
    // ein Set fuer die FrmFmt-Attribute
    SfxItemSet aSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    if( !IsNewDoc() )
        Reader::ResetFrmFmtAttrs( aSet );

    // der Fly beginnt immer in einem neuen Absatz
    if( pPam->GetPoint()->nContent.GetIndex() )
        InsertPara();

    // RTF-Defaults setzen:
    SwFmtAnchor aAnchor( FLY_PAGE, 1 );
    SwFmtHoriOrient aHori( 0, HORI_LEFT, /*FRAME*/REL_PG_PRTAREA );
    SwFmtVertOrient aVert( 0, VERT_TOP, REL_PG_PRTAREA );

    USHORT nCols = USHRT_MAX, nColSpace = USHRT_MAX, nAktCol = 0;
    SvUShorts aColumns;

    BOOL bChkDropCap = 0 == pSet;
    USHORT nDropCapLines = 0, nDropCapAnchor = 0;
    int nOpenBrakets = GetOpenBrakets();

    if( !pSet )
        pSet = &aSet;
    else
    {
        // die Werte aus dem uebergebenen!
        aAnchor = (SwFmtAnchor&)pSet->Get( RES_ANCHOR );
        aHori = (SwFmtHoriOrient&)pSet->Get( RES_HORI_ORIENT );
        aVert = (SwFmtVertOrient&)pSet->Get( RES_VERT_ORIENT );
    }

    // dann sammel mal alle Attribute zusammen
    int bWeiter = TRUE;
    do {
        USHORT nVal = USHORT(nTokenValue);
        switch( nToken )
        {
        case RTF_ABSW:
            {
                SwFmtFrmSize aSz( ATT_MIN_SIZE, nTokenValue, 0 );
                const SfxPoolItem* pItem;
                if( SFX_ITEM_SET == pSet->GetItemState( RES_FRM_SIZE, TRUE,
                    &pItem ))
                {
                    aSz.SetSizeType( ((SwFmtFrmSize*)pItem)->GetSizeType() );
                    aSz.SetHeight( ((SwFmtFrmSize*)pItem)->GetHeight() );
                }
                if( MINFLY > nTokenValue )  nTokenValue = MINFLY;
                aSet.Put( aSz );
            }
            break;
        case RTF_ABSH:
            {
                SwFmtFrmSize aSz( ATT_MIN_SIZE, 0, MINFLY );
                const SfxPoolItem* pItem;
                if( SFX_ITEM_SET == pSet->GetItemState( RES_FRM_SIZE, TRUE,
                    &pItem ))
                {
                    aSz.SetWidth( ((SwFmtFrmSize*)pItem)->GetWidth() );
                }

                if( 0 > nTokenValue )
                {
                    nTokenValue = -nTokenValue;
                    aSz.SetSizeType( ATT_FIX_SIZE );
                }
                if( MINFLY > nTokenValue )  nTokenValue = MINFLY;
                aSz.SetHeight( nTokenValue );
                aSet.Put( aSz );
            }
            break;

        case RTF_NOWRAP:
            {
                pSet->Put( SwFmtSurround( SURROUND_NONE ));
            }
            break;
        case RTF_DXFRTEXT:
                {
                    SvxULSpaceItem aUL;
                    SvxLRSpaceItem aLR;
                    aUL.SetUpper( nVal );   aUL.SetLower( nVal );
                    aLR.SetLeft( nVal );    aLR.SetRight( nVal );
                    pSet->Put( aUL );
                    pSet->Put( aLR );
                }
                break;

        case RTF_DFRMTXTX:
                {
                    SvxLRSpaceItem aLR;
                    aLR.SetLeft( nVal );    aLR.SetRight( nVal );
                    pSet->Put( aLR );
                }
                break;
        case RTF_DFRMTXTY:
                {
                    SvxULSpaceItem aUL;
                    aUL.SetUpper( nVal );   aUL.SetLower( nVal );
                    pSet->Put( aUL );
                }
                break;

        case RTF_POSNEGX:
        case RTF_POSX:      aHori.SetHoriOrient( HORI_NONE );
                            aHori.SetPos( (long)nTokenValue );
                            break;
        case RTF_POSXC:     aHori.SetHoriOrient( HORI_CENTER );     break;
        case RTF_POSXI:     aHori.SetHoriOrient( HORI_LEFT );
                            aHori.SetPosToggle( TRUE );
                            break;
        case RTF_POSXO:     aHori.SetHoriOrient( HORI_RIGHT );
                            aHori.SetPosToggle( TRUE );
                            break;
        case RTF_POSXL:     aHori.SetHoriOrient( HORI_LEFT );       break;
        case RTF_POSXR:     aHori.SetHoriOrient( HORI_RIGHT );      break;

        case RTF_POSNEGY:
        case RTF_POSY:      aVert.SetVertOrient( VERT_NONE );
                            aVert.SetPos( (long)nTokenValue );
                            break;
        case RTF_POSYT:     aVert.SetVertOrient( VERT_TOP );    break;
        case RTF_POSYB:     aVert.SetVertOrient( VERT_BOTTOM ); break;
        case RTF_POSYC:     aVert.SetVertOrient( VERT_CENTER ); break;

        case RTF_PHMRG:     aHori.SetRelationOrient( REL_PG_PRTAREA ); break;
        case RTF_PVMRG:     aVert.SetRelationOrient( REL_PG_PRTAREA ); break;
        case RTF_PHPG:      aHori.SetRelationOrient( REL_PG_FRAME ); break;
        case RTF_PVPG:      aVert.SetRelationOrient( REL_PG_FRAME );break;
        case RTF_PHCOL:     aHori.SetRelationOrient( FRAME ); break;
        case RTF_PVPARA:    aVert.SetRelationOrient( FRAME ); break;

        case RTF_POSYIL:
        case RTF_ABSLOCK:
                                break;

        case RTF_DROPCAPLI:                         // Dropcaps !!
                if( bChkDropCap )
                {
                    nDropCapLines = USHORT( nTokenValue );
                    if( !nDropCapAnchor )
                        nDropCapAnchor = 1;
                }
                break;
        case RTF_DROPCAPLT:
                if( bChkDropCap )
                {
                    nDropCapAnchor = USHORT( nTokenValue );
                    if( !nDropCapLines )
                        nDropCapLines = 3;
                }
                break;


        // fuer die "alten" Writer - haben die Spaltigkeit falsch heraus-
        // geschrieben
        case RTF_COLS:          nCols = USHORT( nTokenValue );      break;
        case RTF_COLSX:         nColSpace = USHORT( nTokenValue );  break;
        case RTF_COLNO:
            nAktCol = USHORT( nTokenValue );
            if( RTF_COLW == GetNextToken() )
            {
                USHORT nWidth = USHORT( nTokenValue ), nSpace = 0;
                if( RTF_COLSR == GetNextToken() )
                    nSpace = USHORT( nTokenValue );
                else
                    SkipToken( -1 );        // wieder zurueck

                if( --nAktCol == ( aColumns.Count() / 2 ) )
                {
                    aColumns.Insert( nWidth + nSpace, aColumns.Count() );
                    aColumns.Insert( nSpace, aColumns.Count() );
                }
            }
            break;

        case '{':
            {
                short nSkip = 0;
                if( RTF_IGNOREFLAG != ( nToken = GetNextToken() ))
                {
                    if( RTF_SHADINGDEF == (nToken & ~0xff) )
                    {
                        ReadBackgroundAttr( nToken, aSet );
                        GetNextToken();     // Klammer ueberlesen
                    }
                    else
                        nSkip = -1;
                }
                else if( RTF_APOCTL ==
                    ((nToken = GetNextToken() ) & ~(0xff | RTF_SWGDEFS)) )
                {
                    bReadSwFly = TRUE;      // alles kommt in den akt. Fly
                    SvxLRSpaceItem aLR;
                    SvxULSpaceItem aUL;
                    nCols = USHRT_MAX;      // neu aufsetzen
                    nColSpace = USHRT_MAX;
                    do {
                    nVal = USHORT(nTokenValue);
                    switch( nToken )
                    {
                    // Swg-Frame-Tokens
                    case RTF_FLYPRINT:
                        {
                            pSet->Put( SvxPrintItem( RES_PRINT, FALSE ));
                        }
                        break;
                    case RTF_FLYOPAQUE:
                        {
                            pSet->Put( SvxOpaqueItem( RES_OPAQUE, FALSE ));
                        }
                        break;

                    case RTF_FLYPRTCTD:
                        {
                            RTFProtect aP( (BYTE)nTokenValue );
                            SvxProtectItem aProtectItem;
                            aProtectItem.SetCntntProtect( aP.GetCntnt() );
                            aProtectItem.SetSizeProtect( aP.GetSize() );
                            aProtectItem.SetPosProtect( aP.GetPos() );
                            pSet->Put( aProtectItem );
                        }
                        break;

                    case RTF_FLYMAINCNT:
                        {
                            RTFSurround aMC( (BYTE)nTokenValue );
                            SwFmtSurround aSurr( (SwSurround)aMC.GetOrder());
                            if( aMC.GetGoldCut() )
                                aSurr.SetSurround( SURROUND_IDEAL );
                            pSet->Put( aSurr );
                        }
                        break;
                    case RTF_FLYVERT:
                        {
                            RTFVertOrient aVO( nVal );
                            aVert.SetVertOrient( (SwVertOrient)aVO.GetOrient() );
                            aVert.SetRelationOrient( (SwRelationOrient)aVO.GetRelation() );
                        }
                        break;
                    case RTF_FLYHORZ:
                        {
                            RTFHoriOrient aHO( nVal );
                            aHori.SetHoriOrient( (SwHoriOrient)aHO.GetOrient() );
                            aHori.SetRelationOrient( (SwRelationOrient)aHO.GetRelation() );
                        }
                        break;
                    case RTF_FLYOUTLEFT:        aLR.SetLeft( nVal );        break;
                    case RTF_FLYOUTRIGHT:       aLR.SetRight( nVal );       break;
                    case RTF_FLYOUTUPPER:       aUL.SetUpper( nVal );       break;
                    case RTF_FLYOUTLOWER:       aUL.SetLower( nVal );       break;
                    case RTF_FLYANCHOR:
                            switch( GetNextToken() )
                            {
                            case RTF_FLY_PAGE:
                                aAnchor.SetType( FLY_PAGE );
                                aAnchor.SetPageNum( USHORT(nTokenValue));
                                aAnchor.SetAnchor( 0 );
                                break;

                            case RTF_FLY_CNTNT:
                                {
                                    SwNodeIndex aIdx( pPam->GetPoint()->nNode );
                                    pDoc->GetNodes().GoPrevious( &aIdx );
                                    SwPosition aPos( aIdx );
                                    aAnchor.SetType( FLY_AT_CNTNT );
                                    aAnchor.SetAnchor( &aPos );
                                }
                                break;

// JP 26.09.94: die Bindung an die Spalte gibt es nicht mehr !!
//                          case RTF_FLY_COLUMN:
                            }
                            break;
                    case RTF_COLS:  nCols = USHORT( nTokenValue );      break;
                    case RTF_COLSX: nColSpace = USHORT( nTokenValue );  break;
                    case RTF_COLNO:
                        nAktCol = USHORT( nTokenValue );
                        if( RTF_COLW == GetNextToken() )
                        {
                            USHORT nWidth = USHORT( nTokenValue ), nSpace = 0;
                            if( RTF_COLSR == GetNextToken() )
                                nSpace = USHORT( nTokenValue );
                            else
                                SkipToken( -1 );        // wieder zurueck

                            if( --nAktCol == ( aColumns.Count() / 2 ) )
                            {
                                aColumns.Insert( nWidth + nSpace, aColumns.Count() );
                                aColumns.Insert( nSpace, aColumns.Count() );
                            }
                        }
                        break;

                    case '{':
                        if( RTF_BRDBOX == ( nToken = GetNextToken() ) )
                            ReadBorderAttr( nToken, aSet );
                        else if( RTF_SHADINGDEF == (nToken & ~0xff ) )
                            ReadBackgroundAttr( nToken, aSet );
                        else if( RTF_IGNOREFLAG == nToken )
                        {
                            int bSkipGrp = TRUE;
                            switch( nToken = GetNextToken() )
                            {
                            case RTF_SHADOW:
                            case RTF_BRDBOX:
                                ReadAttr( SkipToken( -2 ), &aSet );
                                bSkipGrp = FALSE;
                                break;

                            case RTF_BRDRT:
                            case RTF_BRDRB:
                            case RTF_BRDRR:
                            case RTF_BRDRL:
                                bSkipGrp = FALSE;
                                ReadBorderAttr( SkipToken( -2 ), aSet );
                                break;
                            }

                                // keine weitere Klammer mehr ueberlesen!!!
                            if( !bSkipGrp )
                                break;

                            SkipGroup();
                        }
                        else
                            SkipGroup();
                        GetNextToken();     // Klammer ueberlesen
                        break;
                    }
                    } while( IsParserWorking() &&
                                '}' != ( nToken = GetNextToken() ));

                    if( aUL.GetUpper() || aUL.GetLower() )
                        pSet->Put( aUL );
                    if( aLR.GetLeft() || aLR.GetRight() )
                        pSet->Put( aLR );
                }
                else if( RTF_BRDBOX == nToken )
                    ReadBorderAttr( nToken, aSet );
                else if( RTF_SHADOW == nToken )
                    ReadAttr( SkipToken( -2 ), &aSet );
                else if( RTF_SHADINGDEF == (nToken & ~0xff ) )
                    ReadBackgroundAttr( nToken, aSet );
                else if( RTF_UNKNOWNCONTROL == nToken )
                    SkipGroup();
                else
                    nSkip = -2;

                if( nSkip )
                {
                    nToken = SkipToken( nSkip );
                    bWeiter = FALSE;
                }
            }
            break;

        default:
            bWeiter = FALSE;
        }

        if( bWeiter )
            nToken = GetNextToken();
    } while( bWeiter && IsParserWorking() );

    if( !bReadSwFly && ( FRAME == aVert.GetRelationOrient() ||
        //JP 21.07.99: for Bug 67630 - it read into Header od Footer, set
        //              never the anchor to Page. Because it is then only
        //              on the first used page .
        ( FLY_PAGE == aAnchor.GetAnchorId() &&
          pPam->GetPoint()->nNode < pDoc->GetNodes().GetEndOfExtras() &&
          ( pPam->GetNode()->FindHeaderStartNode() ||
            pPam->GetNode()->FindFooterStartNode() )) ))
        aAnchor.SetType( FLY_AT_CNTNT );

    pSet->Put( aAnchor );
    pSet->Put( aHori );
    pSet->Put( aVert );

    if( nCols && USHRT_MAX != nCols )
    {
        SwFmtCol aCol;
        if( USHRT_MAX == nColSpace )
            nColSpace = 720;

        const SvxLRSpaceItem& rLR = (SvxLRSpaceItem&)pSet->Get( RES_LR_SPACE );
        ULONG nWidth = USHRT_MAX;
        aCol.Init( nCols, nColSpace, USHORT( nWidth ) );
        if( nCols == ( aColumns.Count() / 2 ) )
        {
            for( USHORT n = 0, i = 0; n < aColumns.Count(); n += 2, ++i )
            {
                SwColumn* pCol = aCol.GetColumns()[ i ];
                ULONG nTmp = aColumns[ n ];
                nTmp *= USHRT_MAX;
                nTmp /= nWidth;
                pCol->SetWishWidth( USHORT(nTmp) );
/*
    JP 07.07.95: der Dialog kennt nur eine Breite fuer alle Spalten
                 darum hier nicht weiter beachten
                nTmp = aColumns[ n+1 ];
                if( nTmp )
                    pCol->SetRight( USHORT(nTmp) );
                else
                    pCol->SetRight( 0 );
                pCol->SetLeft( 0 );
*/
            }
        }
        pSet->Put( aCol );
    }

    if( pSet != &aSet )         // wurde der Set uebergeben, dann wars das
        return ;

    // ein neues FlyFormat anlegen oder das alte benutzen ?
    // (teste ob es die selben Attribute besitzt!)
    SwFlySave* pFlySave;
    USHORT nFlyArrCnt = aFlyArr.Count();
    if( !nFlyArrCnt ||
        !( pFlySave = aFlyArr[ nFlyArrCnt-1 ])->IsEqualFly( *pPam, aSet ))
    {
        pFlySave = new SwFlySave( *pPam, aSet );
        Size aPgSize;
        GetPageSize( aPgSize );
        pFlySave->nPageWidth = aPgSize.Width();

        if( nDropCapAnchor )
        {
            pFlySave->nDropAnchor = nDropCapAnchor;
            pFlySave->nDropLines = nDropCapLines;
        }
        aFlyArr.Insert(  pFlySave, nFlyArrCnt++ );
    }

    SetPardTokenRead( FALSE );
    const SwTableNode* pTblNd = pPam->GetNode()->FindTableNode();

    while( !IsPardTokenRead() && IsParserWorking() )
    {
        if( RTF_PARD == nToken || nOpenBrakets > GetOpenBrakets() )
            break;

        NextToken( nToken );

        if( !IsPardTokenRead() )
        {
            nToken = GetNextToken();

            // BUG 22036: kommt zwischen Fly-Attributen ein unbekanntes,
            //              dann erzeuge nie 2 FlyFrames, sondern fasse
            //              beide zusammen !!!
            while( RTF_APOCTL == ( nToken & ~(0xff | RTF_SWGDEFS) ))
            {
                if( RTF_FLY_INPARA == nToken )
                    break;

                if( RTF_IGNOREFLAG == SkipToken( -1 ) )
                {
                    if( '{' == SkipToken( -1 ) )
                        nToken = '{';
                    else
                        SkipToken( 2 );
                }
                else
                    SkipToken( 1 );

                ReadFly( nToken, &pFlySave->aFlySet );
                nToken = GetNextToken();
            }
        }
    }

    if( pTblNd && !pPam->GetPoint()->nContent.GetIndex() &&
        pTblNd->EndOfSectionIndex() + 1 ==
            pPam->GetPoint()->nNode.GetIndex() )
    {
        // nicht mehr in der Tabelle, sondern dahinter ?
        // Dann aber wieder zurueck in die Tabelle
        pPam->Move( fnMoveBackward );
    }
    else
        pTblNd = 0;

    // wurde garnichts eingefuegt?
    if( !pTblNd &&
        pPam->GetPoint()->nNode == pFlySave->nSttNd &&
        !pPam->GetPoint()->nContent.GetIndex() )
    {
//      // dann erzeuge mindestens einen leeren TextNode
//      pDoc->AppendTxtNode(*pPam);
        // dann zerstoere den FlySave wieder.
        aFlyArr.DeleteAndDestroy( --nFlyArrCnt );

    }
    else
    {
        FASTBOOL bMovePaM = 0 != pTblNd;

        pFlySave->nEndNd = pPam->GetPoint()->nNode;
        pFlySave->nEndCnt = pPam->GetPoint()->nContent.GetIndex();

        if( bMovePaM )
            pPam->Move( fnMoveForward );

        pTblNd = pFlySave->nSttNd.GetNode().FindTableNode();
        if( pTblNd && !pFlySave->nEndCnt &&
            pTblNd == pFlySave->nEndNd.GetNode().FindTableNode() )
        {
            // dann teste mal, ob das \pard nicht zu spaet kam und
            // eigentlich in die vorherige Zelle gehoert
            const SwStartNode* pSttBoxNd = pFlySave->nSttNd.GetNode().
                                            FindTableBoxStartNode(),
                            * pEndBoxNd = pFlySave->nEndNd.GetNode().
                                            FindTableBoxStartNode();
            if( pSttBoxNd && pEndBoxNd &&
                bMovePaM ? ( pSttBoxNd == pEndBoxNd )
                         : ( pSttBoxNd->EndOfSectionIndex() + 1 ==
                                pEndBoxNd->GetIndex() &&
                                pEndBoxNd->GetIndex() + 1 ==
                                pFlySave->nEndNd.GetIndex() ))
            {
                // dann gehoert das Ende in die vorherige Box!
                SwPosition aPos( *pPam->GetPoint() );
                pPam->GetPoint()->nNode = *pSttBoxNd->EndOfSectionNode();
                pPam->Move( fnMoveBackward, fnGoNode );

                DelLastNode();

                pPam->GetPoint()->nNode = *pSttBoxNd->EndOfSectionNode();
                pPam->Move( fnMoveBackward, fnGoNode );

                pFlySave->nEndNd = pPam->GetPoint()->nNode;
                pFlySave->nEndCnt = pPam->GetPoint()->nContent.GetIndex();

                *pPam->GetPoint() = aPos;
            }
        }
        else if( !bReadSwFly && !pFlySave->nEndCnt &&
            pFlySave->nSttNd.GetIndex() + 1 == pFlySave->nEndNd.GetIndex() &&
            pFlySave->nSttNd.GetNode().IsTxtNode() )
        {

            SwTxtNode* pTxtNd = pFlySave->nSttNd.GetNode().GetTxtNode();
            SwTxtFlyCnt* pFlyCnt;
            if( 1 == pTxtNd->GetTxt().Len() &&
                0 != ( pFlyCnt = (SwTxtFlyCnt*)pTxtNd->GetTxtAttr(
                                                0, RES_TXTATR_FLYCNT )) &&
                pFlyCnt->GetFlyCnt().GetFrmFmt() )
            {
                // then move the content into the surrounded fly
                SwFrmFmt* pFlyFmt = pFlyCnt->GetFlyCnt().GetFrmFmt();
                const SwNodeIndex* pFlySNd = pFlyFmt->GetCntnt().GetCntntIdx();
                SwNodeRange aRg( *pFlySNd, 1,
                                 *pFlySNd->GetNode().EndOfSectionNode(), 0 );

                // merge the itemsets
                SwFmtFrmSize aSz1( (SwFmtFrmSize&)pFlyFmt->GetAttrSet().
                                                Get( RES_FRM_SIZE ));
                SwFmtFrmSize aSz2( (SwFmtFrmSize&)pFlySave->aFlySet.
                                                Get( RES_FRM_SIZE ));
                // if
                if( !aRg.aStart.GetNode().IsNoTxtNode() ||
                    !aSz1.GetHeight() || !aSz1.GetWidth() ||
                    !aSz2.GetHeight() || !aSz2.GetWidth() ||
                    ( aSz1.GetHeight() == aSz2.GetHeight() &&
                      aSz1.GetWidth() == aSz2.GetWidth() ) )
                {
                    SfxItemSet aDiffs( pFlyFmt->GetAttrSet() );
                    aDiffs.ClearItem( RES_ANCHOR );
                    aDiffs.ClearItem( RES_FRM_SIZE );
                    aDiffs.ClearItem( RES_CNTNT );
                    aDiffs.Differentiate( pFlySave->aFlySet );
                    pFlySave->aFlySet.Put( aDiffs );

                    BOOL bSet = FALSE;
                    if( aSz1.GetHeight() && !aSz2.GetHeight() )
                    {
                        bSet = TRUE;
                        aSz2.SetHeight( aSz1.GetHeight() );
                    }
                    if( aSz1.GetWidth() && !aSz2.GetWidth() )
                    {
                        bSet = TRUE;
                        aSz2.SetWidth( aSz1.GetWidth() );
                    }
                    if( bSet )
                        pFlySave->aFlySet.Put( aSz2 );

                    // move any PageBreak/Desc Attr to the next Para
                    {
                        SwCntntNode* pSrcNd = pFlySave->nSttNd.GetNode().GetCntntNode();
                        SwCntntNode* pDstNd = pFlySave->nEndNd.GetNode().GetCntntNode();

                        ::lcl_CpyBreakAttrs( pSrcNd, pDstNd, &pFlySave->nEndNd );
                    }

                    // create new txtnode, because the section does never be empty
                    pDoc->GetNodes().MakeTxtNode( aRg.aStart,
                                (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl() );

                    SwNodeIndex aTmp( pFlySave->nSttNd, +1 );
                    pDoc->Move( aRg, aTmp );

                    // now delete the redundant txtnode
                    pDoc->GetNodes().Delete( pFlySave->nSttNd, 1 );
                }
            }
        }
    }

    bReadSwFly = FALSE;
    SkipToken( -1 );
}


void SwRTFParser::InsPicture( const String& rGrfNm, const Graphic* pGrf,
                                const SvxRTFPictureType* pPicType )
{
    // kennzeichen fuer Swg-Dokumente:
    // (dann ist das FlyFmt fuer die Grafik!)
    SwGrfNode * pGrfNd;
    if( bReadSwFly )
    {
        // erzeuge nur einen normalen GrafikNode und ersetze diesen gegen
        // den vorhandenen Textnode
        SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
        pGrfNd = pDoc->GetNodes().MakeGrfNode( rIdx,
                    rGrfNm, aEmptyStr,    // Name der Graphic !!
                    pGrf,
                    (SwGrfFmtColl*)pDoc->GetDfltGrfFmtColl() );

        if( pGrfAttrSet )
            pGrfNd->SetAttr( *pGrfAttrSet );

        SwFlySave* pFlySave = aFlyArr[ aFlyArr.Count()-1 ];
        pFlySave->nSttNd = rIdx.GetIndex() - 1;

        if( 1 < aFlyArr.Count() )
        {
            pFlySave = aFlyArr[ aFlyArr.Count() - 2 ];
            if( pFlySave->nEndNd == rIdx )
                pFlySave->nEndNd = rIdx.GetIndex() - 1;
        }
    }
    else
    {
        // wenn normale RTF-Grafik, dann steht diese im Textfluss !
        SwAttrSet aFlySet( pDoc->GetAttrPool(), RES_VERT_ORIENT, RES_ANCHOR );
        const SwPosition* pPos = pPam->GetPoint();

        SwFmtAnchor aAnchor( FLY_IN_CNTNT );
        aAnchor.SetAnchor( pPos );
        aFlySet.Put( aAnchor );
        aFlySet.Put( SwFmtVertOrient( 0, VERT_TOP ));

        SwFrmFmt* pFlyFmt = pDoc->Insert( *pPam,
                    rGrfNm, aEmptyStr,      // Name der Graphic !!
                    pGrf,
                    &aFlySet,               // Attribute fuer den FlyFrm
                    pGrfAttrSet );          // Attribute fuer die Grafik

        pGrfNd = pDoc->GetNodes()[ pFlyFmt->GetCntnt().GetCntntIdx()->
                                            GetIndex()+1 ]->GetGrfNode();

        _SetPictureSize( *pGrfNd, pPos->nNode,
                        (SfxItemSet&)pFlyFmt->GetAttrSet(),
                        pPicType );
    }

    if( pGrfAttrSet )
        DELETEZ( pGrfAttrSet );
}

void SwRTFParser::_SetPictureSize( const SwNoTxtNode& rNd,
                                    const SwNodeIndex& rAnchor,
                                    SfxItemSet& rSet,
                                    const SvxRTFPictureType* pPicType )
{
    Size aSize( ((SwNoTxtNode&)rNd).GetTwipSize() );
    if( pPicType )
    {
        if( rNd.IsGrfNode() )
        {
            if( SvxRTFPictureType::WIN_METAFILE != pPicType->eStyle &&
                pPicType->nGoalWidth && pPicType->nGoalHeight )
            {
                aSize.Width() = pPicType->nGoalWidth;
                aSize.Height() =pPicType->nGoalHeight;
            }
            else if( SvxRTFPictureType::MAC_QUICKDRAW == pPicType->eStyle )
            {
                // IMMER auf 72 DPI bezogen, also 1pt == 20 Twip !!
                aSize.Width() = pPicType->nWidth * 20;
                aSize.Height() = pPicType->nHeight * 20;
            }
            else
            {
                // von 100TH_MM nach TWIP umrechenen!
                aSize.Width() = pPicType->nWidth * 144 / 254;
                aSize.Height() = pPicType->nHeight * 144 / 254;
            }
            ((SwGrfNode&)rNd).SetTwipSize( aSize );
        }

        if( 100 != pPicType->nScalX )
            aSize.Width() = (((long)pPicType->nScalX) * ( aSize.Width() -
                        ( pPicType->nCropL + pPicType->nCropR ))) / 100L;

        if( 100 != pPicType->nScalY )
            aSize.Height() = (((long)pPicType->nScalY) * ( aSize.Height() -
                        ( pPicType->nCropT + pPicType->nCropB ))) / 100L;
    }

    // sorge dafuer, das der Rahmen nicht groesser als der akt.
    // Pagedescriptor wird.
    Size aPgSize;
    GetPageSize( aPgSize );

    if( aSize.Height() > aPgSize.Height() )
        aSize.Height() = aPgSize.Height();

    if( aSize.Width() > aPgSize.Width() )
        aSize.Width() = aPgSize.Width();

    //steht der Fly etwa in einer Tabelle ?
    const SwNode* pAnchorNd = pDoc->GetNodes()[ rAnchor ];
    const SwTableNode* pTblNd = pAnchorNd->FindTableNode();
    if( pTblNd )
    {
        // Box feststellen:
        const SwTableBox* pBox = pTblNd->GetTable().GetTblBox(
                                pAnchorNd->StartOfSectionIndex() );
        if( pBox )
        {
            long nBoxWidth = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
            if( aSize.Width() > nBoxWidth )
                aSize.Width() = nBoxWidth;
        }
    }

        // min. Werte einhalten !!
    if( aSize.Width() < /*2268*/MINFLY )
        aSize.Width() = /*2268*/MINFLY;
    if( aSize.Height() < MINFLY)
        aSize.Height() = MINFLY;

    if( pPicType )
    {
        BOOL bChg = FALSE;
        SwCropGrf aCrop;

/*
 JP 28.07.99: Bug 67800 - no crop by MAC_QUICKDRAW. At time i dont know why
                            it has been coded. But this has used for any
                            RTF-File, but i dont found them.
        if( SvxRTFPictureType::MAC_QUICKDRAW == pPicType->eStyle )
        {
            // evt. ein wenig Croppen ??
            // IMMER auf 72 DPI bezogen, also 1pt == 20 Twip !!
            long nTmp = pPicType->nWidth * 20;
            if( nTmp != aSize.Width() )
            {
                // in der Breite (also rechts) croppen
                aCrop.Right() = nTmp - aSize.Width();
                aSize.Width() = nTmp;
                bChg = TRUE;
            }

            nTmp = pPicType->nHeight * 20;
            if( nTmp != aSize.Height() )
            {
                // in der Hoehe (also unten) croppen
                aCrop.Bottom() = nTmp - aSize.Height();
                aSize.Height() = nTmp;
                bChg = TRUE;
            }
        }
*/
        if( pPicType->nCropT )
            aCrop.SetTop( pPicType->nCropT ), bChg = TRUE;
        if( pPicType->nCropB )
            aCrop.SetBottom( pPicType->nCropB ), bChg = TRUE;
        if( pPicType->nCropL )
            aCrop.SetLeft( pPicType->nCropL ), bChg = TRUE;
        if( pPicType->nCropR )
            aCrop.SetRight( pPicType->nCropR ), bChg = TRUE;

        if( bChg )
        {
            // dann mal an die CropWerte an die GrafikSize anpassen.
            ((SwNoTxtNode&)rNd).SetAttr( aCrop );
        }
    }
    rSet.Put( SwFmtFrmSize( ATT_FIX_SIZE, aSize.Width(), aSize.Height() ));
}

void SwRTFParser::GetPageSize( Size& rSize )
{
    const SwFrmFmt& rPgFmt = pDoc->GetPageDesc(nAktPageDesc).GetMaster();

    const SwFmtFrmSize& rSz   = rPgFmt.GetFrmSize();
    const SvxLRSpaceItem& rLR = rPgFmt.GetLRSpace();
    const SvxULSpaceItem& rUL = rPgFmt.GetULSpace();
    const SwFmtCol& rCol = rPgFmt.GetCol();

    rSize.Width() = rSz.GetWidth() - rLR.GetLeft() - rLR.GetRight();
    rSize.Height() = rSz.GetHeight() - rUL.GetUpper() - rUL.GetLower();

    if( 1 < rCol.GetNumCols() )
    {
        rSize.Width() /= rCol.GetNumCols();
        rSize.Height() /= rCol.GetNumCols();
    }
}

void __EXPORT SwRTFParser::ReadBitmapData()
{
    Graphic aGrf;
    SvxRTFPictureType aPicType;
    if( ReadBmpData( aGrf, aPicType ) )
        InsPicture( aEmptyStr, &aGrf, &aPicType );
}

#ifdef READ_OLE_OBJECT
void __EXPORT SwRTFParser::ReadOLEData()
{
    SvCacheStream aTmpFile( 0xA000 );
    Graphic aGrf;
    SvxRTFPictureType aPicType, aOleType;

    int nToken, bValidOle = TRUE, bWeiter = TRUE;
    int nOpenBrakets = 1;       // die erste wurde schon vorher erkannt !!

    String* pStr = 0;
    String sObjClass, sObjName, sObjData;

    while( nOpenBrakets && IsParserWorking() && bWeiter && bValidOle )
    {
        nToken = GetNextToken();
        USHORT nVal = USHORT( nTokenValue );
        switch( nToken )
        {
        case '}':       --nOpenBrakets; pStr = 0; break;
        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                    nToken = SkipToken( -2 );
                else
                {
                    // gleich herausfiltern
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++nOpenBrakets;
            }
            break;

        case RTF_OBJECT:
        case RTF_OBJEMB:        // default ist embedded
        case RTF_LINKSELF:      // ??
        case RTF_OBJLOCK:       // ??
        case RTF_OBJUPDATE:     // ??
        case RTF_OBJTIME:       // ??
        case RTF_OBJSETSIZE:
        case RTF_OBJALIGN:
        case RTF_OBJTRANSY:
        case RTF_OBJATTPH:
            break;

        case RTF_OBJLINK:       // ?? welche Daten sind das ??
        case RTF_OBJAUTLINK:    // ??       -""-            ??
        case RTF_OBJSUB:
        case RTF_OBJPUB:
        case RTF_OBJICEMB:
        case RTF_OBJOCX:
        case RTF_OBJHTML:
        case RTF_OBJALIAS:
        case RTF_OBJSECT:
            bValidOle = FALSE;      // diese Typen koennen wir nicht
            break;

        case RTF_OBJCLASS:
            // Daten lesen
            pStr = &sObjClass;
            break;

        case RTF_OBJNAME:
            // Daten lesen
            pStr = &sObjName;
            break;

        case RTF_OBJDATA:
            pStr = &sObjData;
            break;

        case RTF_RESULT:
            {
                // hier weitermachen, wenn das OLE-Object ungueltig ist
                bWeiter = FALSE;
            }
            break;
        case RTF_RSLTBMP:           // diese sollten wir ignorieren
        case RTF_RSLTMERGE:
        case RTF_RSLTPICT:
        case RTF_RSLTRTF:
        case RTF_RSLTTXT:
            break;

        case RTF_OBJW:          aOleType.nWidth = nVal; break;
        case RTF_OBJH:          aOleType.nHeight = nVal; break;
        case RTF_OBJCROPT:      aOleType.nCropT = (short)nTokenValue; break;
        case RTF_OBJCROPB:      aOleType.nCropB = (short)nTokenValue; break;
        case RTF_OBJCROPL:      aOleType.nCropL = (short)nTokenValue; break;
        case RTF_OBJCROPR:      aOleType.nCropR = (short)nTokenValue; break;
        case RTF_OBJSCALEX:     aOleType.nScalX = nVal; break;
        case RTF_OBJSCALEY:     aOleType.nScalY = nVal; break;

        case RTF_TEXTTOKEN:
            if( 1 < nOpenBrakets && pStr )
            {
                if( pStr == &sObjData )
                {
                    xub_StrLen nHexLen = HexToBin( aToken );
                    if( STRING_NOTFOUND != nHexLen )
                        bValidOle = FALSE;
                    else
                    {
                        aTmpFile.Write( (sal_Char*)aToken.GetBuffer(), nHexLen );
                        bValidOle = 0 == aTmpFile.GetError();
                    }
                }
                else
                    *pStr += aToken;
            }
            break;
        }
    }

    if( bValidOle )
    {
        bValidOle = FALSE;      // erstmal
    }

    if( !bWeiter )      // dann stehen wir noch im Result
    {
        // ist das Ole-Object Ok?
        // -> dann solange SkipGroup rufen, bis zur letzten
        //      schliessenden Klammer
        // ansonsten alle Token verarbeiten, bis zur letzten
        //      schliessenden Klammer

        bWeiter = TRUE;
        while( nOpenBrakets && IsParserWorking() && bWeiter )
        {
            switch( nToken = GetNextToken() )
            {
            case '}':       --nOpenBrakets; break;
            case '{':       ++nOpenBrakets;  break;
            }
            if( nOpenBrakets && !bValidOle )
                NextToken( nToken );
        }
    }

    if( !bValidOle && '}' != nToken )
        SkipGroup();

    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
}
#endif

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/rtf/rtffly.cxx,v 1.1.1.1 2000-09-18 17:14:56 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.86  2000/09/18 16:04:50  willem.vandorp
      OpenOffice header added.

      Revision 1.85  2000/08/25 12:27:59  jp
      Graphic Crop-Attribut exported to SVX

      Revision 1.84  2000/05/09 17:22:49  jp
      Changes for Unicode

      Revision 1.83  2000/04/14 14:06:17  khz
      Task #70451# use CalculateFlySize() for WW frames with 'auto'width

      Revision 1.82  1999/12/13 13:17:43  jp
      Bug #70652#: read tokens posnegx/y

      Revision 1.81  1999/08/19 10:33:32  JP
      Bug #68304#: by windows metafile ignore the picgoal size, dont delete sourrounded flys if the size unequal


      Rev 1.80   19 Aug 1999 12:33:32   JP
   Bug #68304#: by windows metafile ignore the picgoal size, dont delete sourrounded flys if the size unequal

      Rev 1.79   09 Aug 1999 17:37:48   JP
   read owne border of flyframes

      Rev 1.78   05 Aug 1999 23:04:16   JP
   Bug #67974#: SetFlysInDoc - anchor to next or cur paragraph

      Rev 1.77   03 Aug 1999 19:46:44   JP
   Bug #68007#,#67978#: changes for import of flys from interleave/WP7/Aplixware

      Rev 1.76   28 Jul 1999 14:21:32   JP
   Bug #67800#: set no crop by MAC-Pictures, if the size not equal

      Rev 1.75   21 Jul 1999 18:27:02   JP
   Bug #67630#,#67663#: changes for read of flys

      Rev 1.74   08 Jul 1999 21:24:24   JP
   has an APO no framesize, then get from the txtnode the min/max size

      Rev 1.73   17 Jun 1999 14:46:20   JP
   Bug #66301#: SetFlyInDoc - create new Node if the 'anchor' Node has pagebreak attrs

      Rev 1.72   19 Apr 1999 12:48:44   JP
   Vorbereitungen fuer OLE-Objecte in RTF lesen (anstoss ist Bug #60140#)

      Rev 1.71   24 Mar 1999 14:48:58   JP
   Bug #63882#: Flys in/mit Tabellen korrekt lesen

      Rev 1.70   19 Nov 1998 22:05:58   JP
   Bug #59697#: Token ABSLOCK ignorieren, inner/out richtig behandeln

      Rev 1.69   21 Sep 1998 17:16:14   JP
   DropCaps unterstuetzen

      Rev 1.68   27 Jul 1998 14:57:06   JP
   Bug #54001#: Wert vom AbsH an der Size setzen

      Rev 1.67   06 Jul 1998 13:36:00   MH
   int. comp. error

      Rev 1.66   13 May 1998 18:16:20   JP
   CheckSize: nicht mit undefiniertem Pointer arbeiten

      Rev 1.65   12 May 1998 15:51:20   JP
   rund um Flys/DrawObjs im Doc/FESh umgestellt/optimiert

      Rev 1.64   07 May 1998 15:40:08   JP
   MacPict Grafiken haben Point und keine MM100 angaben

      Rev 1.63   08 Apr 1998 14:28:34   RG
   C40_INSERT ging unter unxsols nicht

      Rev 1.62   20 Feb 1998 13:36:32   MA
   headerfiles gewandert

      Rev 1.61   17 Feb 1998 12:24:54   RG
   Mac: sysdep.hxx raus

      Rev 1.60   11 Feb 1998 14:07:16   JP
   kleinere Bugs behoben bei Rahmen & Tabellen

      Rev 1.59   29 Jan 1998 21:35:24   JP
   GetEndOfIcons ersetzt durch GetEndOfExtras, das auf GetEndOfRedlines mappt

      Rev 1.58   25 Nov 1997 15:11:30   JP
   Headerfiles

      Rev 1.57   20 Nov 1997 12:16:50   AMA
   Opt. SwSurround: GoldCut jetzt als Enum; nicht implementierte Enums entfernt

      Rev 1.56   03 Nov 1997 14:10:36   MA
   precomp entfernt

      Rev 1.55   30 Oct 1997 17:49:32   AMA
   Chg: Kein AutoFlag mehr an Break bzw. PageDesc-Attributen

      Rev 1.54   09 Oct 1997 14:27:50   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.53   15 Aug 1997 12:51:44   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.52   13 Aug 1997 19:18:30   JP
   Bug #41892#: ReadFly - nur bei RTF-Absatzbindung ggfs. auf die Seienbind. umschalten

      Rev 1.51   13 Aug 1997 18:35:02   JP
   Bug #42480#: spaltige Rahmen kennt RTF nicht, nur wir

      Rev 1.50   11 Aug 1997 17:49:34   OS
   Header-Umstellung

      Rev 1.49   07 Aug 1997 15:06:24   OM
   Headerfile-Umstellung

      Rev 1.48   22 May 1997 20:39:54   JP
   Bug #40118#: kein Inhalt im Fly -> verwerfen

      Rev 1.47   07 Jan 1997 16:53:22   JP
   Bug #34627#: GrafikGroesse setzen

      Rev 1.46   06 Jan 1997 19:04:10   JP
   IsEqual: Flys zusammenfassen, wenns die akt. Position vom vorhergeheden ist

      Rev 1.45   06 Jan 1997 15:15:48   JP
   Bug #34622#: TeilNodes bei Flys absplitten, FlyInFlys korrekt lesen

      Rev 1.44   29 Oct 1996 12:58:50   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.43   09 Oct 1996 13:05:18   JP
   SwNodes::GoNext/-Prev (-Sect) returnt statt BOOL den gefundenen ContentNode

      Rev 1.42   08 Oct 1996 13:53:42   JP
   IsEqualFly: Header/Footer Absaetze nicht zusammenfassen;absatzgebundene Flys

      Rev 1.41   27 Sep 1996 17:59:46   MIB
   In Filtern in Rahmenvorlagen Umrandungs/Abstaende loeschen

      Rev 1.40   29 Aug 1996 10:10:46   JP
   beim StartNode muss jetzt ein spz. Sectiontype gesetzt werden

      Rev 1.39   20 Aug 1996 14:53:34   TRI
   C40_INSERT statt Insert

      Rev 1.38   04 Jul 1996 15:01:20   JP
   ReadFly: auch bei eigenen APOCTL ReadFly rufen

      Rev 1.37   28 Jun 1996 15:07:30   MA
   includes

      Rev 1.36   18 Jun 1996 16:36:04   JP
   ReadFly: erweitert um optionalen Itemset erweitert(FlyInFly lesen); MACPict: ggfs. Croppen

      Rev 1.35   13 Jun 1996 09:21:26   JP
   Flys lesen: bei Fly in Fly eine while schleife

      Rev 1.34   10 Jun 1996 11:24:24   JP
   Crop-Werte im CropAttribut setzen

      Rev 1.33   25 Apr 1996 13:56:42   MIB
   Verschiebung SvHTMLParser in Gooedies

      Rev 1.32   25 Apr 1996 09:23:44   JP
   lesen der Grafik-Daten in svxparser verschoben

*************************************************************************/

