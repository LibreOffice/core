/*************************************************************************
 *
 *  $RCSfile: rtftbl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-01 17:55:49 $
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

#ifdef WTC
#define private public
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_BRKITEM_HXX
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen wg. SvxLRSpaceItem
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX
#include <svx/boxitem.hxx>
#endif
#ifndef _RTFTOKEN_H
#include <svtools/rtftoken.h>
#endif

#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWPARRTF_HXX
#include <swparrtf.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif
#ifndef _TBLENUM_HXX
#include <tblenum.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif

typedef SwTableBoxFmt* SwTableBoxFmtPtr;
SV_DECL_PTRARR( SwBoxFrmFmts, SwTableBoxFmtPtr, 25, 50 )

class SwShareBoxFmts;
extern void _DeleteBox( SwTable& rTbl, SwTableBox* pBox, SwUndo* = 0,
                    BOOL = TRUE, const BOOL = TRUE, SwShareBoxFmts* = 0 );

void SwRTFParser::ReadTable( int nToken )
{
    nInsTblRow = USHRT_MAX;
    if( bReadNoTbl )
    {
        // alle Tabellen-Tokens ueberlesen
        nToken = GetNextToken();        // RTF_TROWD ueberlesen
        do {
            if( RTF_TABLEDEF != (nToken & ~(0xff | RTF_SWGDEFS)) &&
                RTF_UNKNOWNCONTROL != nToken )
            {
                SkipToken( -1 );
                break;
            }
            nToken = GetNextToken();
        } while( IsParserWorking() );
        return ;
    }

    // verhinder Tabelle in Tabelle/Footnote
    SwTwips nTblSz = 0;
    int bReadNewCell = FALSE, bChkExistTbl = FALSE;

    // kein TROWD aber ein TabellenToken -> zwischen TROWD und Tab.Token
    // waren andere Zeichen (siehe Bug 27445.rtf)
    if( RTF_TROWD == nToken || !pTableNode )
    {
        if( RTF_TROWD == nToken )
            nToken = GetNextToken();        // RTF_TROWD ueberlesen

        // Flags fuer die gemergten Boxen loeschen
        aMergeBoxes.Remove( 0, aMergeBoxes.Count() );
        aMergeBoxes.Insert( (BOOL)FALSE, USHORT(0) );
        nAktBox = 0;

        // wenn schon in einer Tabellen, dann splitte oder benutze
        // die bisherigen Boxen weiter
        bChkExistTbl = 0 != pDoc->GetNodes()[ pPam->GetPoint()->nNode ]->FindTableNode();
    }
    else
    {
        bReadNewCell = TRUE;
        SwTableLines& rLns = pTableNode->GetTable().GetTabLines();
        SwTableLine* pLine = rLns[ rLns.Count()-1 ];
        for( USHORT n = nAktBox; n; )
            nTblSz += pLine->GetTabBoxes()[ --n ]->GetFrmFmt()->
                            GetFrmSize().GetWidth();
    }


    SwHoriOrient eAdjust = HORI_LEFT;       // default fuer Tabellen
    SwTwips nLSpace = 0;
    USHORT nBrdDist = MIN_BORDER_DIST;
    SwVertOrient eVerOrient = VERT_NONE;
    long nLineHeight = 0;
    USHORT nBoxCnt = aMergeBoxes.Count()-1;
    SwBoxFrmFmts aBoxFmts;
    SwTableBoxFmt* pBoxFmt = pDoc->MakeTableBoxFmt();
    BOOL bHeadlineRepeat = FALSE;

#ifdef SET_TRGAPH
    SvxLRSpaceItem aLR;
#endif

    int bWeiter = TRUE;
    do {
        switch( nToken )
        {
        case RTF_TRRH:
            nLineHeight = nTokenValue;
            break;

        case RTF_CLMRG:
            aMergeBoxes[ nBoxCnt ] = TRUE;
            break;

        case RTF_CELLX:
            {
                SwTableBoxFmt* pFmt = pBoxFmt;
                SwTwips nSize = nTokenValue - nTblSz;
                if( aMergeBoxes[ nBoxCnt ] )
                {
                    // neue Zellen lesen und noch keine Formate vorhanden,
                    // dann benutze das der vorhergebende
                    if( bReadNewCell && !aBoxFmts.Count() )
                    {
                        SwTableLines& rLns = pTableNode->GetTable().GetTabLines();
                        SwTableLine* pLine = rLns[ rLns.Count()-1 ];
                        --nAktBox;
                        pFmt = (SwTableBoxFmt*)pLine->GetTabBoxes()[ nAktBox ]->GetFrmFmt();
                    }
                    else
                        pFmt = aBoxFmts[ aBoxFmts.Count()-1 ];

                    pBoxFmt->ResetAllAttr();
                    nSize += pFmt->GetFrmSize().GetWidth();
                }
                else
                {
                    // Platz zwischen den Spalten angeben und ist
                    // diese auch kleiner als die Breite der Box
#ifdef SET_TRGAPH
                    if( aLR.GetLeft() && 2L * aLR.GetLeft() < nSize )
                        pBoxFmt->SetAttr( aLR );
                    aLR.SetLeft( 0 ); aLR.SetRight( 0 );
#endif
                    aBoxFmts.Insert( pBoxFmt, aBoxFmts.Count() );
                    pBoxFmt = pDoc->MakeTableBoxFmt();
                }

                pFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, nSize, 0 ));
                nTblSz = nTokenValue;
                aMergeBoxes.Insert( (BOOL)FALSE, ++nBoxCnt );
            }
            break;

        case RTF_TRGAPH:
            {
                nBrdDist = (USHORT)nTokenValue;
#ifdef SET_TRGAPH
                aLR.SetLeft( USHORT( nTokenValue ));
                aLR.SetRight( USHORT( nTokenValue ));
#endif
            }
            break;

        case RTF_TRQL:          eAdjust = HORI_LEFT;    break;
        case RTF_TRQR:          eAdjust = HORI_RIGHT;   break;
        case RTF_TRQC:          eAdjust = HORI_CENTER;  break;

                                // mit VERT_TOP kommt der Dialog nicht klar!
                                // Bug #65126#
        case RTF_CLVERTALT:     eVerOrient = VERT_NONE;     break;

        case RTF_CLVERTALC:     eVerOrient = VERT_CENTER;   break;
        case RTF_CLVERTALB:     eVerOrient = VERT_BOTTOM;   break;

        case RTF_TRLEFT:
            if( HORI_LEFT == eAdjust )
                eAdjust = HORI_LEFT_AND_WIDTH;
            nLSpace = nTokenValue;
            nTblSz = nTokenValue;
            break;

        case RTF_TRHDR:
            bHeadlineRepeat = TRUE;
            break;

        case RTF_CLTXLRTB:
        case RTF_CLTXTBRL:
        case RTF_INTBL:     // das wissen wir !
        case RTF_CLMGF:
        case RTF_CLVMGF:
        case RTF_CLVMRG:
        case RTF_LTRROW:
        case RTF_RTLROW:
        case RTF_TRBRDRB:
        case RTF_TRBRDRH:
        case RTF_TRBRDRL:
        case RTF_TRBRDRR:
        case RTF_TRBRDRT:
        case RTF_TRBRDRV:
        case RTF_TRKEEP:
                break;

        default:
            if( ( nToken & ~(0xff | RTF_TABLEDEF)) == RTF_SHADINGDEF )
            {
                if( aMergeBoxes[ nBoxCnt ] )
                    break;
                ReadBackgroundAttr( nToken,
                        (SfxItemSet&)pBoxFmt->GetAttrSet(), TRUE );
            }
            else if( ( nToken & ~(0xff | RTF_TABLEDEF) ) == RTF_BRDRDEF )
            {
                if( aMergeBoxes[ nBoxCnt ] )
                    break;

                const SfxPoolItem* pItem;
                SfxItemSet& rSet = (SfxItemSet&)pBoxFmt->GetAttrSet();
                ReadBorderAttr( nToken, rSet, TRUE );
                if( SFX_ITEM_SET == rSet.GetItemState( RES_BOX, FALSE, &pItem ))
                {
                    SvxBoxItem aBox( *(SvxBoxItem*)pItem );
                    BOOL bChg = FALSE;
                    for ( int nLn = 0; nLn < 4; ++nLn )
                        if( aBox.GetLine( nLn ) && !aBox.GetDistance( nLn ) )
                        {
                            aBox.SetDistance( 2 > nLn ? 18 : nBrdDist, nLn );
                            bChg = TRUE;
                        }
                    if( bChg )
                        rSet.Put( aBox );
                }
            }
            else if( RTF_TABLEDEF != (nToken & ~(0xff | RTF_SWGDEFS)) )
            {
                if( RTF_UNKNOWNCONTROL == nToken )
                    NextToken( nToken );
                else
                    bWeiter = FALSE;
            }
            break;
        }

        if( VERT_NONE != eVerOrient )
        {
            if( !aMergeBoxes[ nBoxCnt ] )
                pBoxFmt->SetAttr( SwFmtVertOrient( 0, eVerOrient ));
            eVerOrient = VERT_NONE;
        }
        if( bWeiter )
            nToken = GetNextToken();
    } while( bWeiter && IsParserWorking() );

    // das letzte temp. BoxFmt loeschen
    delete pBoxFmt;

    // es wurde keine einzige Box erkannt
    if( nAktBox == nBoxCnt || ( bReadNewCell && !pTableNode ))
    {
        SkipToken( -1 );            // zum Letzen gueltigen zurueck
        return;
    }

    nTblSz -= nLSpace;

    int bNewTbl = TRUE;
    SwTableLine* pNewLine;

    // lege eine neue Tabelle an oder erweiter die aktuelle um eine neue Line
    // oder Box !
    SwNode* pNd;

    if( bChkExistTbl )
    {
        // es ist eine Tabelle vorhanden, teste mal ob die weiter benutzt
        // werden kann oder ob sie abgesplittet und neu gefuellt werden
        // muss.
        pTableNode = pPam->GetNode()->FindTableNode();

        // Cursor kann nur in der letzten Line stehen

        // das Attribut darf nicht ueber das Modify an der
        // Tabelle gesetzt werden, denn sonst werden alle
        // Boxen wieder auf 0 zurueck gesetzt !!!!!
        SwFrmFmt* pFmt = pTableNode->GetTable().GetFrmFmt();
        const SwFmtFrmSize& rTblSz = pFmt->GetFrmSize();
        const SwFmtHoriOrient& rHoriz = pFmt->GetHoriOrient();

        const SwTableLines* pLns = &pTableNode->GetTable().GetTabLines();

        if( 1 == pLns->Count() )
        {
            if( eAdjust != rHoriz.GetHoriOrient() )
            {
                ((SfxItemSet&)pFmt->GetAttrSet()).Put( SwFmtHoriOrient( 0,
                                                            eAdjust ) );
            }
            if( rTblSz.GetWidth() != nTblSz )
            {
                SwFmtFrmSize aSz( rTblSz );
                aSz.SetWidth( nTblSz );
                ((SfxItemSet&)pFmt->GetAttrSet()).Put( aSz );
            }

            if( HORI_LEFT_AND_WIDTH == eAdjust &&
                nLSpace != pFmt->GetLRSpace().GetLeft() )
            {
                SvxLRSpaceItem aL; aL.SetLeft( nLSpace );
                ((SfxItemSet&)pFmt->GetAttrSet()).Put( aL );
            }
        }
        else if( 1 < pLns->Count() && ( rTblSz.GetWidth() != nTblSz
            || rHoriz.GetHoriOrient() != eAdjust ||
            ( HORI_LEFT_AND_WIDTH == eAdjust &&
                nLSpace != pFmt->GetLRSpace().GetLeft() ) ||
            bHeadlineRepeat != pTableNode->GetTable().IsHeadlineRepeat() ))
        {
            // Tabelle ab der PaM-Position splitten
            pNewLine = (*pLns)[ pLns->Count() - 2 ];        // die vorherige Line!
            SwTableBox* pBox = pNewLine->GetTabBoxes()[ 0 ];
            while( ( pLns = &pBox->GetTabLines() )->Count() )
                pBox = (*pLns)[ 0 ]->GetTabBoxes()[ 0 ];

            SwNodeIndex aTmpIdx( *pBox->GetSttNd() );
            pDoc->GetNodes().SplitTable( aTmpIdx, HEADLINE_NONE, FALSE );
            pTableNode = pPam->GetNode()->FindTableNode();
            pFmt = pTableNode->GetTable().GetFrmFmt();

            SwFmtFrmSize aSz( rTblSz );
            aSz.SetWidth( nTblSz );
            ((SfxItemSet&)pFmt->GetAttrSet()).Put( aSz );
            ((SfxItemSet&)pFmt->GetAttrSet()).Put( SwFmtHoriOrient( 0,
                                                            eAdjust ) );
            if( HORI_LEFT_AND_WIDTH == eAdjust && nLSpace )
            {
                SvxLRSpaceItem aL; aL.SetLeft( nLSpace );
                ((SfxItemSet&)pFmt->GetAttrSet()).Put( aL );
            }
            pTableNode->GetTable().SetHeadlineRepeat( bHeadlineRepeat );
        }

        pLns = &pTableNode->GetTable().GetTabLines();
        pNewLine = (*pLns)[ pLns->Count() - 1 ];

        // jetzt die Boxen abgleichen
        USHORT nBoxes = Min( pNewLine->GetTabBoxes().Count(), aBoxFmts.Count() );
        for( USHORT n = 0; n < nBoxes; ++n )
        {
            SwTableBox* pBox = pNewLine->GetTabBoxes()[ n ];
            *pBox->GetFrmFmt() = *aBoxFmts[ n ];
            delete aBoxFmts[ n ];
        }
        aBoxFmts.Remove( 0, n );

        if( aBoxFmts.Count() )      // es muessen noch neue zugefuegt werden
            nAktBox = n;
        else                        // es mussen noch Boxen geloescht werden
        {
            // remove ContentIndex of other Bound
            pPam->SetMark(); pPam->DeleteMark();
            while( n < pNewLine->GetTabBoxes().Count() )
                _DeleteBox( pTableNode->GetTable(),
                            pNewLine->GetTabBoxes()[ n ], 0, FALSE, FALSE );
        }

        pOldTblNd = pTableNode;
        bNewTbl = FALSE;

        {
            // JP 13.08.98: TabellenUmrandungen optimieren - Bug 53525
            void* p = pFmt;
            aTblFmts.Insert( p, aTblFmts.Count() );
        }
    }
    else
    {
        if( !bReadNewCell && ( pNd = pDoc->GetNodes()[
            pPam->GetPoint()->nNode.GetIndex()-1 ])->IsEndNode() )
        {
            pTableNode = pNd->StartOfSectionNode()->GetTableNode();
            if( pTableNode )
            {
                // dann test mal ob wirklich nur eine neue Line eingefuegt
                // werden soll!
                SwFrmFmt* pFmt = pTableNode->GetTable().GetFrmFmt();
                const SwFmtFrmSize& rTblSz = pFmt->GetFrmSize();
                const SwFmtHoriOrient& rHoriz = pFmt->GetHoriOrient();
                if( rTblSz.GetWidth() != nTblSz
                    || rHoriz.GetHoriOrient() != eAdjust )
                    pTableNode = 0;
            }
        }

        if( pTableNode )
        {

            // das Attribut darf nicht ueber das Modify an der
            // Tabelle gesetzt werden, denn sonst werden alle
            // Boxen wieder auf 0 zurueck gesetzt !!!!!
            SwFrmFmt* pFmt = pTableNode->GetTable().GetFrmFmt();
            const SwFmtFrmSize& rTblSz = pFmt->GetFrmSize();
            if( rTblSz.GetWidth() < nTblSz )
            {
                SwFmtFrmSize aSz( rTblSz );
                aSz.SetWidth( nTblSz );
                ((SfxItemSet&)pFmt->GetAttrSet()).Put( aSz );
            }

            SwTableLines& rLns = pTableNode->GetTable().GetTabLines();

            if( bReadNewCell )
                pNewLine = rLns[ rLns.Count()-1 ];
            else
            {
                pNewLine = new SwTableLine(
                        (SwTableLineFmt*)rLns[ rLns.Count()-1 ]->GetFrmFmt(),
                        aBoxFmts.Count(), 0 );
                pNewLine->ClaimFrmFmt();
                pNewLine->GetFrmFmt()->ResetAttr( RES_FRM_SIZE );
                rLns.C40_INSERT( SwTableLine, pNewLine, rLns.Count() );
            }
            bNewTbl = FALSE;
        }
        else
        {
            if( pPam->GetPoint()->nContent.GetIndex() )
                InsertPara();

            pDoc->InsertTable( *pPam->GetPoint(), 1, 1, eAdjust );
            pTableNode = pDoc->GetNodes()[ pPam->GetPoint()->nNode.
                            GetIndex() - 5 ]->GetTableNode();
            ASSERT( pTableNode, "Wo ist mein TabellenNode?" );

            SwTableLines& rLns = pTableNode->GetTable().GetTabLines();
            pNewLine = rLns[ rLns.Count()-1 ];

            SwFrmFmt* pFmt = pTableNode->GetTable().GetFrmFmt();
            SwFmtFrmSize aSz( pFmt->GetFrmSize() );
            aSz.SetWidth( nTblSz );
            ((SfxItemSet&)pFmt->GetAttrSet()).Put( aSz );

            if( HORI_LEFT_AND_WIDTH == eAdjust && nLSpace )
            {
                SvxLRSpaceItem aL; aL.SetLeft( nLSpace );
                ((SfxItemSet&)pFmt->GetAttrSet()).Put( aL );
            }
            pTableNode->GetTable().SetHeadlineRepeat( bHeadlineRepeat );

            nAktBox = 0;
            pOldTblNd = pTableNode;

            {
                // JP 13.08.98: TabellenUmrandungen optimieren - Bug 53525
                void* p = pFmt;
                aTblFmts.Insert( p, aTblFmts.Count() );
            }
        }
    }

    if( nLineHeight )
    {
        SwFrmSize eSize;
        if( 0 > nLineHeight )
            eSize = ATT_FIX_SIZE, nLineHeight = -nLineHeight;
        else
            eSize = ATT_MIN_SIZE;
        pNewLine->ClaimFrmFmt()->SetAttr( SwFmtFrmSize( eSize, 0, nLineHeight ));
    }

    if( aBoxFmts.Count() )
    {
        // setze das default Style
        SwTxtFmtColl* pColl = aTxtCollTbl.Get( 0 );
        if( !pColl )
            pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );

        USHORT nStt = 0;
        if( bNewTbl )
        {
            SwTableBox* pBox = pNewLine->GetTabBoxes()[0];
            pBoxFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();
            pBoxFmt->Remove( pBox );
            delete pBoxFmt;
            aBoxFmts[0]->Add( pBox );
            SwTxtNode* pTNd = pDoc->GetNodes()[ pBox->GetSttIdx()+1 ]
                                            ->GetTxtNode();
            ASSERT( pTNd, "wo ist der Textnode dieser Box?" );
            pTNd->ChgFmtColl( pColl );
            ++nStt;
        }

        for( ; nStt < aBoxFmts.Count(); ++nStt )
        {
            pDoc->GetNodes().InsBoxen( pTableNode, pNewLine,
                    aBoxFmts[ nStt ],
                    // Formate fuer den TextNode der Box
                    pColl, 0,
                    nAktBox + nStt, 1 );
        }
    }

    if( bChkExistTbl )
        nAktBox = 0;

    if( pDoc->GetRootFrm() )
    {
        // exitiert schon ein Layout, dann muss an dieser Tabelle die
        // BoxFrames neu erzeugt werden.
        pTableNode->DelFrms();
        pTableNode->MakeFrms( &pPam->GetPoint()->nNode );
    }

    ULONG nOldPos = pPam->GetPoint()->nNode.GetIndex();
    SwTableBox* pBox = pNewLine->GetTabBoxes()[ nAktBox ];
    pPam->GetPoint()->nNode = *pBox->GetSttNd()->EndOfSectionNode();
    pPam->Move( fnMoveBackward, fnGoCntnt );

    // alle Attribute, die schon auf den nachfolgen zeigen auf die neue
    // Box umsetzen !!
    SvxRTFItemStack& rAttrStk = GetAttrStack();
    const SvxRTFItemStackType* pStk;
    for( USHORT n = 0; n < rAttrStk.Count(); ++n )
        if( ( pStk = rAttrStk[ n ])->GetSttNodeIdx() == ULONG(nOldPos) &&
            !pStk->GetSttCnt() )
            ((SvxRTFItemStackType*)pStk)->SetStartPos( SwxPosition( pPam ) );

    SkipToken( -1 );            // zum Letzen gueltigen zurueck
}


// in die naechste Box dieser Line (opt.: falls es nicht die letzte ist)
void SwRTFParser::GotoNextBox()
{
    nInsTblRow = USHRT_MAX;

    ASSERT( pTableNode, "Kein Tabellennode, dann auch keine Box" );

    SwTableLines& rLns = pTableNode->GetTable().GetTabLines();
    SwTableLine* pLine = rLns[ rLns.Count()-1 ];
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    SwTableBox* pBox = rBoxes[ rBoxes.Count()-1 ];

    if( ++nAktBox >= aMergeBoxes.Count() )
        nAktBox = aMergeBoxes.Count()-1;

    if( !aMergeBoxes[ nAktBox ] )
    {
        int bMove = TRUE;
        if( pBox->GetSttIdx() > pPam->GetPoint()->nNode.GetIndex() )
        {
            USHORT nRealBox = 0;
            for( USHORT nTmp = 0; nTmp < nAktBox; ++nTmp )
                if( !aMergeBoxes[ nTmp ] )
                    ++nRealBox;

            if( nRealBox < rBoxes.Count() )
            {
                pPam->GetPoint()->nNode = *rBoxes[ nRealBox ]->GetSttNd()->EndOfSectionNode();
                pPam->Move( fnMoveBackward, fnGoCntnt );
                bMove = FALSE;
            }
        }

        if( bMove && nAktBox + 1 == aMergeBoxes.Count() )
            // dann hinter die Tabelle
            pPam->Move( fnMoveForward, fnGoNode );
    }
    else if( !pDoc->GetNodes()[ pPam->GetPoint()->nNode ]->IsCntntNode() )
        // dann in die vorherige ans Ende
        pPam->Move( fnMoveBackward, fnGoCntnt );
}


void SwRTFParser::NewTblLine()
{
    nInsTblRow = USHRT_MAX;

    // erweiter die aktuelle um eine neue Line
    FASTBOOL bMakeCopy = FALSE;
    SwNode* pNd = pDoc->GetNodes()[ pPam->GetPoint()->nNode.GetIndex()-1 ];
    if( !pNd->IsEndNode() ||
        !(pNd = pNd->StartOfSectionNode())->IsTableNode() )
    {
        if( !pOldTblNd )
            return ;

        bMakeCopy = TRUE;
        pNd = pOldTblNd;
    }
    pTableNode = (SwTableNode*)pNd;

    SwTableLines* pLns = &pTableNode->GetTable().GetTabLines();
    SwTableLine* pLine = (*pLns)[ pLns->Count()-1 ];
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    SwTableBox* pBox = rBoxes[ rBoxes.Count()-1 ];

    if( !bMakeCopy &&
        64000 < pTableNode->GetTable().GetTabSortBoxes().Count() )
    {
        bMakeCopy = TRUE;       // spaetestens jetzt eine neue anfangen!
    }

    if( bMakeCopy )
    {
        // und die Selektion kopieren
        SwSelBoxes aBoxes;
        pTableNode->GetTable().SelLineFromBox( pBox, aBoxes );
        pTableNode->GetTable().MakeCopy( pDoc, *pPam->GetPoint(),
                                        aBoxes, FALSE );
        ULONG nNd = pPam->GetPoint()->nNode.GetIndex()-1;
        pTableNode = pDoc->GetNodes()[ nNd ]->FindTableNode();
        pOldTblNd = pTableNode;

        pLns = &pTableNode->GetTable().GetTabLines();
    }
    else
//      pDoc->InsertRow( aBoxes );
        pTableNode->GetTable().AppendRow( pDoc );

    pBox = (*pLns)[ pLns->Count()-1 ]->GetTabBoxes()[0];

    ULONG nOldPos = pPam->GetPoint()->nNode.GetIndex();
    pPam->GetPoint()->nNode = *pBox->GetSttNd();
    pPam->Move( fnMoveForward );
    nAktBox = 0;

    // alle Nodes in den Boxen auf die "default" Vorlage setzten
    {
        SwTxtFmtColl* pColl = aTxtCollTbl.Get( 0 );
        if( !pColl )
            pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
        pPam->SetMark();

        pLine = (*pLns)[ pLns->Count()-1 ];
        pBox = pLine->GetTabBoxes()[ pLine->GetTabBoxes().Count() -1 ];
        pPam->GetPoint()->nNode = *pBox->GetSttNd()->EndOfSectionNode();
        pPam->Move( fnMoveBackward );
        pDoc->SetTxtFmtColl( *pPam, pColl );
        // Bug 73940 - remove ALL attributes (NumRules/Break/etc.)
        {
            SwNodeIndex aIdx( pPam->GetMark()->nNode );
            SwNodeIndex& rEndIdx = pPam->GetPoint()->nNode;
            while( aIdx <= rEndIdx )
            {
                SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
                if( pCNd && pCNd->GetpSwAttrSet() )
                    pCNd->ResetAllAttr();
                aIdx++;
            }
        }
        pPam->Exchange();
        pPam->DeleteMark();
    }

    // alle Attribute, die schon auf den nachfolgen zeigen auf die neue
    // Box umsetzen !!
    SvxRTFItemStack& rAttrStk = GetAttrStack();
    const SvxRTFItemStackType* pStk;
    for( USHORT n = 0; n < rAttrStk.Count(); ++n )
        if( ( pStk = rAttrStk[ n ])->GetSttNodeIdx() == ULONG(nOldPos) &&
            !pStk->GetSttCnt() )
            ((SvxRTFItemStackType*)pStk)->SetStartPos( SwxPosition( pPam ) );
}

void SwRTFParser::CheckInsNewTblLine()
{
    if( USHRT_MAX != nInsTblRow )
    {
        if( nInsTblRow > GetOpenBrakets() || IsPardTokenRead() )
            nInsTblRow = USHRT_MAX;
        else if( !pTableNode )      // Tabelle nicht mehr vorhanden ?
            NewTblLine();           // evt. Line copieren
    }
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:56  hr
      initial import

      Revision 1.64  2000/09/18 16:04:51  willem.vandorp
      OpenOffice header added.

      Revision 1.63  2000/05/09 17:22:57  jp
      Changes for Unicode

      Revision 1.62  2000/03/15 18:22:45  jp
      Bug #73972#: ReadTable - set distance on the borderlines

      Revision 1.61  2000/03/14 09:55:41  jp
      Bug #73940#: NewTableLine - remove all attributes after copying the row

      Revision 1.60  2000/02/17 13:46:58  jp
      Bug #73098#: Import / Export problems

      Revision 1.59  1999/09/09 17:46:33  jp
      Bug #68573#: InsertTable - dont insert paragraph at start doc

      Revision 1.58  1999/08/11 14:52:54  JP
      Bug #68058#: read/write repeated tableheadlines


      Rev 1.57   11 Aug 1999 16:52:54   JP
   Bug #68058#: read/write repeated tableheadlines

      Rev 1.56   09 Aug 1999 14:34:04   JP
   Bug #68058#: read/write repeated tableheadlines

      Rev 1.55   05 Aug 1999 22:52:38   JP
   Bug #68095#: save tablefmt for optimize the tableborders

      Rev 1.54   03 Aug 1999 19:51:12   JP
   Bug #67984#: read trleft token

      Rev 1.53   22 Jul 1999 19:59:40   JP
   Bug #54824#: read&write tableposition 'from left'

      Rev 1.52   21 Apr 1999 13:31:20   JP
   Bug #65126#: kein VertTop bei Tabellenzellen setzen

      Rev 1.51   01 Apr 1999 16:46:12   JP
   Bug #64291#: Tabellenhoehe lesen/schreiben

      Rev 1.50   29 Mar 1999 16:28:20   JP
   Bug #62847#: Parameter vom DeleteBox haben sich geaendert

      Rev 1.49   11 Feb 1999 23:43:50   JP
   Bug #55590#: Schnittstelle geaendert

      Rev 1.48   18 Dec 1998 18:02:38   JP
   #59571#: Teilfix - Tabelle splitten, wenn die SortBoxes > 64000

      Rev 1.47   15 Dec 1998 17:49:22   JP
   #59571#: Teilfix - statt InsertRow am Doc das neue AppendRow an der Tabelle benutzen

      Rev 1.46   13 Aug 1998 10:33:24   JP
   Bug #53525#: Tabellen - an den Kanten keine doppelte Umrandung mehr

      Rev 1.45   27 May 1998 22:27:10   JP
   Tokens fuer vertikale Ausrichtung in den Boxen beachten

      Rev 1.44   16 Mar 1998 23:21:40   JP
   SplitTable hat neuen Parameter bekommen

      Rev 1.43   20 Feb 1998 13:36:34   MA
   headerfiles gewandert

      Rev 1.42   04 Feb 1998 09:18:10   JP
   DelFrmFmt: bei TabellenBoxen/Lines direkt das FrmFmt loeschen

      Rev 1.41   26 Nov 1997 15:05:34   MA
   headerfiles

      Rev 1.40   25 Nov 1997 15:11:30   JP
   Headerfiles

      Rev 1.39   03 Nov 1997 14:10:34   MA
   precomp entfernt

      Rev 1.38   29 Oct 1997 13:37:50   JP
   Bug #45159#: PageDesc/-Break wird vom InsertTable schon uebernommen

      Rev 1.37   09 Oct 1997 14:27:50   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.36   15 Aug 1997 12:51:50   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.35   11 Aug 1997 17:48:48   OS
   Header-Umstellung

      Rev 1.34   07 Aug 1997 15:06:40   OM
   Headerfile-Umstellung

      Rev 1.33   27 Jun 1997 11:34:36   JP
   Bug #40723#: erst pard oder neue Ebene nach einem row hebt die Tabelle auf

      Rev 1.32   09 Jun 1997 11:22:30   NF
   lcl_DeleteBox richtig als extern declariert

      Rev 1.31   06 Jun 1997 13:25:52   JP
   lcl_DeleteBox: weiterer Parameter

      Rev 1.30   06 May 1997 12:07:14   MA
   swtablehxx aufgeteilt

      Rev 1.29   17 Apr 1997 15:17:10   JP
   segmentc entfernt

      Rev 1.28   17 Apr 1997 15:15:56   JP
   Bug #38923#: beim einfuegen von Zellen, kann die erste eine gemergt sein

      Rev 1.27   20 Jan 1997 19:21:46   JP
   neu: SwTableFmt/-LineFmt/-BoxFmt statt SwFrmFmt

      Rev 1.26   14 Jan 1997 18:41:40   JP
   Tabelle: Umbrueche im Tabellenformat beachten

      Rev 1.25   20 Dec 1996 19:37:36   JP
   Bug #34702#: Tabellentokens hier auswerten, auch unbekannte!

      Rev 1.24   02 Dec 1996 14:51:28   TRI
   bei WTC ist private jetzt public

      Rev 1.23   29 Oct 1996 12:58:48   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.22   18 Sep 1996 13:47:06   JP
   extern Deklaration zum DeleteBox geaendert

      Rev 1.21   20 Aug 1996 14:57:44   TRI
   C40_INSERT statt Insert

      Rev 1.20   22 Jul 1996 11:53:46   JP
   Bug #27445#: Attribute zwischen TabellenDef und ZellenDef erkennen

      Rev 1.19   04 Jul 1996 15:01:56   JP
   Tabellen einlesen korrigiert

      Rev 1.18   28 Jun 1996 15:07:32   MA
   includes

      Rev 1.17   19 Jun 1996 09:38:30   JP
   Tabellen bekommen beim Insert schon einen eindeutigen Namen

      Rev 1.16   25 Apr 1996 13:56:42   MIB
   Verschiebung SvHTMLParser in Gooedies

      Rev 1.15   15 Jan 1996 13:01:22   JP
   Bug #24234#: Tabellen bekommen einen eindeutigen Namen

      Rev 1.14   09 Jan 1996 11:53:48   JP
   Bug 23680: ReadTable - TabelleInTabelle immer abpruefen

      Rev 1.13   13 Dec 1995 10:35:50   MA
   opt: Bessere Defaults fuer Arrays

      Rev 1.12   24 Nov 1995 17:24:04   OM
   PCH->PRECOMPILED

      Rev 1.11   30 Oct 1995 21:18:56   JP
   intbl refernziert auch vorher eingelesene Tabellen

      Rev 1.10   21 Aug 1995 21:30:36   JP
   svxitems-HeaderFile entfernt

      Rev 1.9   27 Jul 1995 09:28:24   mk
   an SCC4.0.1a angepasst (MDA)

      Rev 1.8   20 Jun 1995 16:49:10   MA
   SwFmtBreak -> SvxFmtBreakItem

      Rev 1.7   30 May 1995 08:19:54   SWG
   forward fuer svtool.hxx!!!

      Rev 1.6   03 Apr 1995 20:51:42   JP
   fuer PreComp.Header eingerichtet

      Rev 1.5   08 Feb 1995 09:43:08   JP
   alten RTF-Parser entfernt, Sw_RTF -> SwRTF

      Rev 1.4   02 Feb 1995 18:38:14   JP
   Tabelle: fuege Boxen auch einzeln ein

      Rev 1.3   27 Jan 1995 08:15:50   JP
   sBoxFmtNm kommt jetzt aus Table-Header

      Rev 1.2   15 Jan 1995 20:34:38   JP
   verhinder Tabelle in Tabelle/Fussnote

      Rev 1.1   11 Jan 1995 19:36:28   JP
   RTF-Reader fertiggestellt

      Rev 1.0   06 Jan 1995 12:07:42   JP
   Initial revision.

*************************************************************************/


