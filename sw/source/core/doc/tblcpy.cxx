/*************************************************************************
 *
 *  $RCSfile: tblcpy.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:16 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#define _ZFORLIST_DECLARE_TABLE
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif

#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif


BOOL _FndCntntLine( const SwTableLine*& rpLine, void* pPara );
BOOL _FndCntntBox( const SwTableBox*& rpBox, void* pPara );


// ---------------------------------------------------------------

// kopiere die Tabelle in diese.
//  Kopiere alle Boxen einer Line in entsprechenden Boxen. Der alte Inhalt
//  wird dabei geloescht.
//  Ist keine mehr vorhanden, kommt der restliche Inhalt in die letzte
//  Box einer "GrundLine".
//  Ist auch keine Line mehr vorhanden, -> auch in die letzte Box
//  einer "GrundLine"


void lcl_CpyBox( const SwTable& rCpyTbl, const SwTableBox* pCpyBox,
                    SwTable& rDstTbl, SwTableBox* pDstBox,
                    BOOL bDelCntnt, SwUndoTblCpyTbl* pUndo )
{
    ASSERT( pCpyBox->GetSttNd() && pDstBox->GetSttNd(),
            "Keine inhaltstragende Box" );

    SwDoc* pCpyDoc = rCpyTbl.GetFrmFmt()->GetDoc();
    SwDoc* pDoc = rDstTbl.GetFrmFmt()->GetDoc();

    // kopiere erst den neuen und loeschen dann den alten Inhalt
    // (keine leeren Section erzeugen; werden sonst geloescht!)
    SwNodeRange aRg( *pCpyBox->GetSttNd(), 1,
                    *pCpyBox->GetSttNd()->EndOfSectionNode() );

    SwNodeIndex aInsIdx( *pDstBox->GetSttNd(), bDelCntnt ? 1 :
                        pDstBox->GetSttNd()->EndOfSectionIndex() -
                        pDstBox->GetSttIdx() );

    if( pUndo )
        pUndo->AddBoxBefore( *pDstBox, bDelCntnt );

    BOOL bUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );

    SwNodeIndex aSavePos( aInsIdx, -1 );
    pCpyDoc->CopyWithFlyInFly( aRg, aInsIdx, FALSE );
    aSavePos++;

    SwTableLine* pLine = pDstBox->GetUpper();
    while( pLine->GetUpper() )
        pLine = pLine->GetUpper()->GetUpper();

    BOOL bReplaceColl = TRUE;
    if( bDelCntnt )
    {
        // zuerst die Fly loeschen, dann die entsprechenden Nodes
        SwNodeIndex aEndNdIdx( *aInsIdx.GetNode().EndOfSectionNode() );

            // Bookmarks usw. verschieben
        {
            SwPosition aMvPos( aInsIdx );
            SwCntntNode* pCNd = pDoc->GetNodes().GoPrevious( &aMvPos.nNode );
            aMvPos.nContent.Assign( pCNd, pCNd->Len() );
            pDoc->CorrAbs( aInsIdx, aEndNdIdx, aMvPos, /*TRUE*/FALSE );
        }

        // stehen noch FlyFrames rum, loesche auch diese
        const SwPosition* pAPos;
        for( USHORT n = 0; n < pDoc->GetSpzFrmFmts()->Count(); ++n )
        {
            SwFrmFmt* pFly = (*pDoc->GetSpzFrmFmts())[n];
            const SwFmtAnchor* pAnchor = &pFly->GetAnchor();
            if( ( FLY_AT_CNTNT == pAnchor->GetAnchorId() ||
                    FLY_AUTO_CNTNT == pAnchor->GetAnchorId() ) &&
                0 != ( pAPos = pAnchor->GetCntntAnchor() ) &&
                aInsIdx <= pAPos->nNode && pAPos->nNode <= aEndNdIdx )
            {
                pDoc->DelLayoutFmt( pFly );
            }
        }

        // ist DestBox eine Headline-Box und hat Tabellen-Vorlage gesetzt,
        // dann NICHT die TabellenHeadline-Vorlage automatisch setzen
        if( 1 < rDstTbl.GetTabLines().Count() &&
            pLine == rDstTbl.GetTabLines()[0] )
        {
            SwCntntNode* pCNd = aInsIdx.GetNode().GetCntntNode();
            if( !pCNd )
            {
                SwNodeIndex aTmp( aInsIdx );
                pCNd = pDoc->GetNodes().GoNext( &aTmp );
            }

            if( pCNd &&
                /*RES_POOLCOLL_TABLE == */
                RES_POOLCOLL_TABLE_HDLN !=
                    pCNd->GetFmtColl()->GetPoolFmtId() )
                bReplaceColl = FALSE;
        }

        pDoc->GetNodes().Delete( aInsIdx, aEndNdIdx.GetIndex() - aInsIdx.GetIndex() );
    }

    if( pUndo )
        pUndo->AddBoxAfter( *pDstBox, bDelCntnt );

    // Ueberschrift
    SwTxtNode* pTxtNd = pDoc->GetNodes()[ aSavePos ]->GetTxtNode();
    if( pTxtNd )
    {
        USHORT nPoolId = pTxtNd->GetTxtColl()->GetPoolFmtId();
        if( bReplaceColl &&
            (( 1 < rDstTbl.GetTabLines().Count() &&
                pLine == rDstTbl.GetTabLines()[0] )
                // gilt noch die Tabellen-Inhalt ??
                ? RES_POOLCOLL_TABLE == nPoolId
                : RES_POOLCOLL_TABLE_HDLN == nPoolId ) )
        {
            SwTxtFmtColl* pColl = pDoc->GetTxtCollFromPool(
                                    RES_POOLCOLL_TABLE == nPoolId
                                        ? RES_POOLCOLL_TABLE_HDLN
                                        : RES_POOLCOLL_TABLE );
            if( pColl )         // Vorlage umsetzen
            {
                SwPaM aPam( aSavePos );
                aPam.SetMark();
                aPam.Move( fnMoveForward, fnGoSection );
                pDoc->SetTxtFmtColl( aPam, pColl );
            }
        }

        // loesche die akt. Formel/Format/Value Werte
        if( SFX_ITEM_SET == pDstBox->GetFrmFmt()->GetItemState( RES_BOXATR_FORMAT ) ||
            SFX_ITEM_SET == pDstBox->GetFrmFmt()->GetItemState( RES_BOXATR_FORMULA ) ||
            SFX_ITEM_SET == pDstBox->GetFrmFmt()->GetItemState( RES_BOXATR_VALUE ) )
        {
            pDstBox->ClaimFrmFmt()->ResetAttr( RES_BOXATR_FORMAT,
                                                 RES_BOXATR_VALUE );
        }

        // kopiere die TabellenBoxAttribute - Formel/Format/Value
        SfxItemSet aBoxAttrSet( pCpyDoc->GetAttrPool(), RES_BOXATR_FORMAT,
                                                        RES_BOXATR_VALUE );
        aBoxAttrSet.Put( pCpyBox->GetFrmFmt()->GetAttrSet() );
        if( aBoxAttrSet.Count() )
        {
            const SfxPoolItem* pItem;
            SvNumberFormatter* pN = pDoc->GetNumberFormatter( FALSE );
            if( pN && pN->HasMergeFmtTbl() && SFX_ITEM_SET == aBoxAttrSet.
                GetItemState( RES_BOXATR_FORMAT, FALSE, &pItem ) )
            {
                ULONG nOldIdx = ((SwTblBoxNumFormat*)pItem)->GetValue();
                ULONG nNewIdx = pN->GetMergeFmtIndex( nOldIdx );
                if( nNewIdx != nOldIdx )
                    aBoxAttrSet.Put( SwTblBoxNumFormat( nNewIdx ));
            }
            pDstBox->ClaimFrmFmt()->SetAttr( aBoxAttrSet );
        }
    }

    pDoc->DoUndo( bUndo );
}

// ---------------------------------------------------------------

// kopiere die Tabelle in diese.
//  Kopiere alle Boxen einer Line in entsprechenden Boxen. Der alte Inhalt
//  wird dabei geloescht.
//  Ist keine mehr vorhanden, kommt der restliche Inhalt in die letzte
//  Box einer "GrundLine".
//  Ist auch keine Line mehr vorhanden, -> auch in die letzte Box
//  einer "GrundLine"


BOOL SwTable::InsTable( const SwTable& rCpyTbl, const SwNodeIndex& rSttBox,
                        SwUndoTblCpyTbl* pUndo )
{
    SetHTMLTableLayout( 0 );    // MIB 9.7.97: HTML-Layout loeschen

    SwDoc* pDoc = GetFrmFmt()->GetDoc();

    SwTableNode* pTblNd = pDoc->IsIdxInTbl( rSttBox );

    // suche erstmal die Box, in die kopiert werden soll:
    SwTableBox* pMyBox = (SwTableBox*)GetTblBox(
            rSttBox.GetNode().FindTableBoxStartNode()->GetIndex() );

    ASSERT( pMyBox, "Index steht nicht in dieser Tabelle in einer Box" );

    // loesche erstmal die Frames der Tabelle
    _FndBox aFndBox( 0, 0 );
    aFndBox.DelFrms( pTblNd->GetTable() );

    SwDoc* pCpyDoc = rCpyTbl.GetFrmFmt()->GetDoc();

    {
        // Tabellen-Formeln in die relative Darstellung umwandeln
        SwTableFmlUpdate aMsgHnt( &rCpyTbl );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        pCpyDoc->UpdateTblFlds( &aMsgHnt );
    }

    SwTblNumFmtMerge aTNFM( *pCpyDoc, *pDoc );

    BOOL bDelCntnt = TRUE;
    const SwTableBox* pTmp;

    for( USHORT nLines = 0; nLines < rCpyTbl.GetTabLines().Count(); ++nLines )
    {
        // hole die erste Box von der Copy-Line
        const SwTableBox* pCpyBox = rCpyTbl.GetTabLines()[nLines]
                                    ->GetTabBoxes()[0];
        while( pCpyBox->GetTabLines().Count() )
            pCpyBox = pCpyBox->GetTabLines()[0]->GetTabBoxes()[0];

        do {
            // kopiere erst den neuen und loeschen dann den alten Inhalt
            // (keine leeren Section erzeugen, werden sonst geloescht!)
            lcl_CpyBox( rCpyTbl, pCpyBox, *this, pMyBox, bDelCntnt, pUndo );

            if( 0 == (pTmp = pCpyBox->FindNextBox( rCpyTbl, pCpyBox, FALSE )))
                break;      // es folgt keine weitere Box mehr
            pCpyBox = pTmp;

            if( 0 == ( pTmp = pMyBox->FindNextBox( *this, pMyBox, FALSE )))
                bDelCntnt = FALSE;  // kein Platz mehr ??
            else
                pMyBox = (SwTableBox*)pTmp;

        } while( TRUE );

        // suche die oberste Line
        SwTableLine* pNxtLine = pMyBox->GetUpper();
        while( pNxtLine->GetUpper() )
            pNxtLine = pNxtLine->GetUpper()->GetUpper();
        USHORT nPos = GetTabLines().C40_GETPOS( SwTableLine, pNxtLine );
        // gibt es eine naechste ??
        if( nPos + 1 >= GetTabLines().Count() )
            bDelCntnt = FALSE;      // es gibt keine, alles in die letzte Box
        else
        {
            // suche die naechste "Inhaltstragende Box"
            pNxtLine = GetTabLines()[ nPos+1 ];
            pMyBox = pNxtLine->GetTabBoxes()[0];
            while( pMyBox->GetTabLines().Count() )
                pMyBox = pMyBox->GetTabLines()[0]->GetTabBoxes()[0];
            bDelCntnt = TRUE;
        }
    }

    aFndBox.MakeFrms( pTblNd->GetTable() );     // erzeuge die Frames neu
    return TRUE;
}


BOOL SwTable::InsTable( const SwTable& rCpyTbl, const SwSelBoxes& rSelBoxes,
                        SwUndoTblCpyTbl* pUndo )
{
    ASSERT( !rCpyTbl.IsTblComplex() && rSelBoxes.Count(),
            "Tabelle ist zu komplex" );

    SetHTMLTableLayout( 0 );    // MIB 9.7.97: HTML-Layout loeschen

    SwDoc* pDoc = GetFrmFmt()->GetDoc();
    SwDoc* pCpyDoc = rCpyTbl.GetFrmFmt()->GetDoc();

    SwTblNumFmtMerge aTNFM( *pCpyDoc, *pDoc );

    SwTableBox *pTmpBox, *pSttBox = (SwTableBox*)rSelBoxes[0];

    USHORT nLn, nBx;
    _FndLine *pFLine, *pInsFLine = 0;
    _FndBox aFndBox( 0, 0 );
    // suche alle Boxen / Lines
    {
        _FndPara aPara( rSelBoxes, &aFndBox );
        ((SwTableLines&)GetTabLines()).ForEach( &_FndLineCopyCol, &aPara );
    }

    // JP 06.09.96: Sonderfall - eine Box in der Tabelle -> in alle
    //              selektierten Boxen kopieren!
    if( 1 != rCpyTbl.GetTabSortBoxes().Count() )
    {
        SwTableLine* pSttLine = pSttBox->GetUpper();
        USHORT nSttBox = pSttLine->GetTabBoxes().C40_GETPOS( SwTableBox, pSttBox );
        USHORT nSttLine = GetTabLines().C40_GETPOS( SwTableLine, pSttLine );
        _FndBox* pFndBox;

        USHORT nFndCnt = aFndBox.GetLines().Count();
        if( !nFndCnt )
            return FALSE;

        // teste ob genug Platz fuer die einzelnen Lines und Boxen ist:
        USHORT nTstLns = 0;
        pFLine = aFndBox.GetLines()[ 0 ];
        pSttLine = pFLine->GetLine();
        nSttLine = GetTabLines().C40_GETPOS( SwTableLine, pSttLine );
        // sind ueberhaupt soviele Zeilen vorhanden
        if( 1 == nFndCnt )
        {
            // in der Tabelle noch genug Platz ??
            if( (GetTabLines().Count() - nSttLine ) <
                rCpyTbl.GetTabLines().Count() )
            {
                // sollte nicht mehr soviele Lines vorhanden sein, dann
                // teste, ob man durch einfuegen neuer zum Ziel kommt. Aber
                // nur wenn die SSelection eine Box umfasst !!
                if( 1 < rSelBoxes.Count() )
                    return FALSE;

                USHORT nNewLns = rCpyTbl.GetTabLines().Count() -
                                (GetTabLines().Count() - nSttLine );

                // Dann teste mal ob die Anzahl der Boxen fuer die Lines reicht
                SwTableLine* pLastLn = GetTabLines()[ GetTabLines().Count()-1 ];

                pSttBox = pFLine->GetBoxes()[0]->GetBox();
                nSttBox = pFLine->GetLine()->GetTabBoxes().C40_GETPOS( SwTableBox, pSttBox );
                for( USHORT n = rCpyTbl.GetTabLines().Count() - nNewLns;
                        n < rCpyTbl.GetTabLines().Count(); ++n )
                {
                    SwTableLine* pCpyLn = rCpyTbl.GetTabLines()[ n ];

                    if( pLastLn->GetTabBoxes().Count() < nSttBox ||
                        ( pLastLn->GetTabBoxes().Count() - nSttBox ) <
                            pCpyLn->GetTabBoxes().Count() )
                        return FALSE;

                    // Test auf Verschachtelungen
                    for( nBx = 0; nBx < pCpyLn->GetTabBoxes().Count(); ++nBx )
                        if( !( pTmpBox = pLastLn->GetTabBoxes()[ nSttBox + nBx ])
                                    ->GetSttNd() )
                            return FALSE;
                }
                // es ist also Platz fuer das zu kopierende vorhanden, also
                // fuege entsprechend neue Zeilen ein.
                SwTableBox* pInsBox = pLastLn->GetTabBoxes()[ nSttBox ];
                ASSERT( pInsBox && pInsBox->GetSttNd(),
                    "kein CntntBox oder steht nicht in dieser Tabelle" );
                SwSelBoxes aBoxes;

                if( pUndo
                    ? !pUndo->InsertRow( *this, SelLineFromBox( pInsBox,
                                aBoxes, TRUE ), nNewLns )
                    : !InsertRow( pDoc, SelLineFromBox( pInsBox,
                                aBoxes, TRUE ), nNewLns, TRUE ) )
                    return FALSE;
            }

            nTstLns = rCpyTbl.GetTabLines().Count();        // soviele Kopieren
        }
        else if( 0 == (nFndCnt % rCpyTbl.GetTabLines().Count()) )
            nTstLns = nFndCnt;
        else
            return FALSE;       // kein Platz fuer die Zeilen

        for( nLn = 0; nLn < nTstLns; ++nLn )
        {
            // Zeilen sind genug vorhanden, dann ueberpruefe die Boxen
            // je Zeile
            pFLine = aFndBox.GetLines()[ nLn % nFndCnt ];
            SwTableLine* pLine = pFLine->GetLine();
            pSttBox = pFLine->GetBoxes()[0]->GetBox();
            nSttBox = pLine->GetTabBoxes().C40_GETPOS( SwTableBox, pSttBox );
            if( nLn >= nFndCnt )
            {
                // es sind im ClipBoard mehr Zeilen als selectiert wurden
                pInsFLine = new _FndLine( GetTabLines()[ nSttLine + nLn ],
                                        &aFndBox );
                pLine = pInsFLine->GetLine();
            }
            SwTableLine* pCpyLn = rCpyTbl.GetTabLines()[ nLn %
                                        rCpyTbl.GetTabLines().Count() ];

            // zu wenig Zeilen selektiert ?
            if( pInsFLine )
            {
                // eine neue Zeile wird in die FndBox eingefuegt,
                if( pLine->GetTabBoxes().Count() < nSttBox ||
                    ( pLine->GetTabBoxes().Count() - nSttBox ) <
                    pFLine->GetBoxes().Count() )
                    return FALSE;

                // Test auf Verschachtelungen
                for( nBx = 0; nBx < pFLine->GetBoxes().Count(); ++nBx )
                {
                    if( !( pTmpBox = pLine->GetTabBoxes()[ nSttBox + nBx ])
                        ->GetSttNd() )
                        return FALSE;
                    // wenn Ok, fuege die Box in die FndLine zu
                    pFndBox = new _FndBox( pTmpBox, pInsFLine );
                    pInsFLine->GetBoxes().C40_INSERT( _FndBox, pFndBox, nBx );
                }
                aFndBox.GetLines().C40_INSERT( _FndLine, pInsFLine, nLn );
            }
            else if( pFLine->GetBoxes().Count() == 1 )
            {
                if( pLine->GetTabBoxes().Count() < nSttBox  ||
                    ( pLine->GetTabBoxes().Count() - nSttBox ) <
                    pCpyLn->GetTabBoxes().Count() )
                    return FALSE;

                // Test auf Verschachtelungen
                for( nBx = 0; nBx < pCpyLn->GetTabBoxes().Count(); ++nBx )
                {
                    if( !( pTmpBox = pLine->GetTabBoxes()[ nSttBox + nBx ])
                        ->GetSttNd() )
                        return FALSE;
                    // wenn Ok, fuege die Box in die FndLine zu
                    if( nBx == pFLine->GetBoxes().Count() )
                    {
                        pFndBox = new _FndBox( pTmpBox, pFLine );
                        pFLine->GetBoxes().C40_INSERT( _FndBox, pFndBox, nBx );
                    }
                }
            }
            else
            {
                // ueberpruefe die selektierten Boxen mit denen im Clipboard
                // (n-Fach)
                if( 0 != ( pFLine->GetBoxes().Count() %
                            pCpyLn->GetTabBoxes().Count() ))
                    return FALSE;

                // Test auf Verschachtelungen
                for( nBx = 0; nBx < pFLine->GetBoxes().Count(); ++nBx )
                    if( !pFLine->GetBoxes()[ nBx ]->GetBox()->GetSttNd() )
                        return FALSE;
            }
        }

        if( !aFndBox.GetLines().Count() )
            return FALSE;
    }

    {
        // Tabellen-Formeln in die relative Darstellung umwandeln
        SwTableFmlUpdate aMsgHnt( &rCpyTbl );
        aMsgHnt.eFlags = TBL_RELBOXNAME;
        pCpyDoc->UpdateTblFlds( &aMsgHnt );
    }

    // loesche die Frames
    aFndBox.SetTableLines( *this );
    aFndBox.DelFrms( *this );

    if( 1 == rCpyTbl.GetTabSortBoxes().Count() )
    {
        SwTableBox *pTmpBox = rCpyTbl.GetTabSortBoxes()[0];
        for( USHORT n = 0; n < rSelBoxes.Count(); ++n )
            lcl_CpyBox( rCpyTbl, pTmpBox, *this,
                        (SwTableBox*)rSelBoxes[n], TRUE, pUndo );
    }
    else
        for( nLn = 0; nLn < aFndBox.GetLines().Count(); ++nLn )
        {
            pFLine = aFndBox.GetLines()[ nLn ];
            SwTableLine* pCpyLn = rCpyTbl.GetTabLines()[
                                nLn % rCpyTbl.GetTabLines().Count() ];
            for( nBx = 0; nBx < pFLine->GetBoxes().Count(); ++nBx )
            {
                // Kopiere in pMyBox die pCpyBox
                lcl_CpyBox( rCpyTbl, pCpyLn->GetTabBoxes()[
                            nBx % pCpyLn->GetTabBoxes().Count() ],
                    *this, pFLine->GetBoxes()[ nBx ]->GetBox(), TRUE, pUndo );
            }
        }

    aFndBox.MakeFrms( *this );
    return TRUE;
}



BOOL _FndCntntBox( const SwTableBox*& rpBox, void* pPara )
{
    SwTableBox* pBox = (SwTableBox*)rpBox;
    if( rpBox->GetTabLines().Count() )
        pBox->GetTabLines().ForEach( &_FndCntntLine, pPara );
    else
        ((SwSelBoxes*)pPara)->Insert( pBox );
    return TRUE;
}


BOOL _FndCntntLine( const SwTableLine*& rpLine, void* pPara )
{
    ((SwTableLine*)rpLine)->GetTabBoxes().ForEach( &_FndCntntBox, pPara );
    return TRUE;
}


// suche alle Inhaltstragenden-Boxen dieser Box
SwSelBoxes& SwTable::SelLineFromBox( const SwTableBox* pBox,
                                    SwSelBoxes& rBoxes, BOOL bToTop ) const
{
    SwTableLine* pLine = (SwTableLine*)pBox->GetUpper();
    if( bToTop )
        while( pLine->GetUpper() )
            pLine = pLine->GetUpper()->GetUpper();

    // alle alten loeschen
    rBoxes.Remove( USHORT(0), rBoxes.Count() );
    pLine->GetTabBoxes().ForEach( &_FndCntntBox, &rBoxes );
    return rBoxes;
}


