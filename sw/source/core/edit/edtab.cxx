/*************************************************************************
 *
 *  $RCSfile: edtab.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:18 $
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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif

#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _LAYFRM_HXX
#include <layfrm.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _EDIMP_HXX
#include <edimp.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _CELLFRM_HXX
#include <cellfrm.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif
#ifndef _SWDDETBL_HXX
#include <swddetbl.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif

extern void ClearFEShellTabCols();

const SwTable& SwEditShell::InsertTable( USHORT nRows, USHORT nCols,
                                        SwHoriOrient eAdj,
                                        USHORT nInsTblFlags,
                                        const SwTableAutoFmt* pTAFmt )
{
    StartAllAction();
    SwPosition* pPos = GetCrsr()->GetPoint();

    BOOL bEndUndo = 0 != pPos->nContent.GetIndex();
    if( bEndUndo )
    {
        StartUndo( UNDO_START );
        GetDoc()->SplitNode( *pPos );
    }
    const SwTable *pTable = GetDoc()->InsertTable( *pPos, nRows, nCols,
                                                eAdj, nInsTblFlags, pTAFmt );
    if( bEndUndo )
        EndUndo( UNDO_END );

    EndAllAction();
    return *pTable;
}

BOOL SwEditShell::TextToTable( sal_Unicode cCh, SwHoriOrient eAdj,
                                USHORT nInsTblFlags,
                                const SwTableAutoFmt* pTAFmt )
{
    SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
    BOOL bRet = FALSE;
    StartAllAction();
    FOREACHPAM_START(this)
        if( PCURCRSR->HasMark() )
            bRet |= 0 != GetDoc()->TextToTable( *PCURCRSR, cCh, eAdj,
                                                nInsTblFlags, pTAFmt );
    FOREACHPAM_END()
    EndAllAction();
    return bRet;
}

BOOL SwEditShell::TableToText( sal_Unicode cCh )
{
    SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
    BOOL bRet = FALSE;
    SwPaM* pCrsr = GetCrsr();
    const SwTableNode* pTblNd =
            GetDoc()->IsIdxInTbl( pCrsr->GetPoint()->nNode );
    if( IsTableMode() )
    {
        ClearMark();
        pCrsr = GetCrsr();
    }
    else if( !pTblNd || pCrsr->GetNext() != pCrsr )
        return bRet;

    StartAllAction();

    // verschiebe den akt. Cursor aus dem Tabellen Bereich
    // angemeldet ist
#ifdef USED
    SwNodeIndex aTabIdx( pCrsr->GetPoint()->nNode );
#else
    SwNodeIndex aTabIdx( *pTblNd );
#endif
    pCrsr->DeleteMark();
    pCrsr->GetPoint()->nNode = *pTblNd->EndOfSectionNode();
    pCrsr->GetPoint()->nContent.Assign( 0, 0 );
    // SPoint und Mark aus dem Bereich verschieben !!!
    pCrsr->SetMark();
    pCrsr->DeleteMark();

    bRet = GetDoc()->TableToText( pTblNd, cCh );
    pCrsr->GetPoint()->nNode = aTabIdx;
#ifdef USED
    pCrsr->Move( fnMoveBackward, fnGoNode );
#else
    SwCntntNode* pCNd = pCrsr->GetCntntNode();
    if( !pCNd )
        pCrsr->Move( fnMoveForward, fnGoCntnt );
    else
        pCrsr->GetPoint()->nContent.Assign( pCNd, 0 );
#endif

    EndAllAction();
    return bRet;
}

FASTBOOL SwEditShell::IsTextToTableAvailable() const
{
    FASTBOOL bOnlyText = FALSE;
    FOREACHPAM_START(this)
        if( PCURCRSR->HasMark() )
        {
            bOnlyText = TRUE;

            // pruefe ob in der Selection eine Tabelle liegt
            ULONG nStt = PCURCRSR->GetMark()->nNode.GetIndex(),
                  nEnd = PCURCRSR->GetPoint()->nNode.GetIndex();
            if( nStt > nEnd )   { ULONG n = nStt; nStt = nEnd; nEnd = n; }

            for( ; nStt <= nEnd; ++nStt )
                if( !GetDoc()->GetNodes()[ nStt ]->IsTxtNode() )
                {
                    bOnlyText = FALSE;
                    break;
                }

            if( !bOnlyText )
                break;
        }
    FOREACHPAM_END()

    return bOnlyText;
}


void SwEditShell::InsertDDETable( SwDDEFieldType* pDDEType,
                                            USHORT nRows, USHORT nCols,
                                            SwHoriOrient eAdj,
                                            USHORT nInsTblFlags )
{
    SwPosition* pPos = GetCrsr()->GetPoint();

    ASSERT( !GetDoc()->IsIdxInTbl( pPos->nNode ),
            "Tabelle in Tabelle nicht erlaubt" );

    StartAllAction();

    BOOL bEndUndo = 0 != pPos->nContent.GetIndex();
    if( bEndUndo )
    {
        StartUndo( UNDO_START );
        GetDoc()->SplitNode( *pPos );
    }

    SwTable* pTbl = (SwTable*)GetDoc()->InsertTable( *pPos, nRows, nCols, eAdj,
                                                    nInsTblFlags|DEFAULT_BORDER );
    SwTableNode* pTblNode = (SwTableNode*)pTbl->GetTabSortBoxes()[ 0 ]->
                                                GetSttNd()->FindTableNode();
    SwDDETable* pDDETbl = new SwDDETable( *pTbl, pDDEType );
    pTblNode->SetNewTable( pDDETbl );       // setze die DDE-Tabelle

    if( bEndUndo )
        EndUndo( UNDO_END );

    EndAllAction();
}

/*--------------------------------------------------------------------
    Beschreibung: Tabellenfelder einer Tabelle updaten
 --------------------------------------------------------------------*/
void SwEditShell::UpdateTable()
{
    const SwTableNode* pTblNd = IsCrsrInTbl();

    // Keine Arme keine Kekse
    if( pTblNd )
    {
        StartAllAction();
        StartUndo();
        EndAllTblBoxEdit();
        SwTableFmlUpdate aTblUpdate( (SwTable*)&pTblNd->GetTable() );
        GetDoc()->UpdateTblFlds( &aTblUpdate );
        EndUndo();
        EndAllAction();
    }
}

    // Change Modus erfragen/setzen
USHORT SwEditShell::GetTblChgMode() const
{
    USHORT nMode;
    const SwTableNode* pTblNd = IsCrsrInTbl();
    if( pTblNd )
        nMode = pTblNd->GetTable().GetTblChgMode();
    else
        nMode = GetTblChgDefaultMode();
    return nMode;
}

void SwEditShell::SetTblChgMode( USHORT eMode )
{
    const SwTableNode* pTblNd = IsCrsrInTbl();

    // Keine Arme keine Kekse
    if( pTblNd )
    {
        ((SwTable&)pTblNd->GetTable()).SetTblChgMode( (TblChgMode)eMode );
        if( !GetDoc()->IsModified() )   // Bug 57028
            GetDoc()->SetUndoNoResetModified();
        GetDoc()->SetModified();
    }
}

BOOL SwEditShell::GetTblBoxFormulaAttrs( SfxItemSet& rSet ) const
{
    SwSelBoxes aBoxes;
    if( IsTableMode() )
        ::GetTblSelCrs( *this, aBoxes );
    else
    {
        SwPaM *pCrsr = GetCrsr();
        do {
            SwFrm *pFrm = GetCurrFrm();
            do {
                pFrm = pFrm->GetUpper();
            } while ( pFrm && !pFrm->IsCellFrm() );
            if ( pFrm )
            {
                SwTableBox *pBox = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
                aBoxes.Insert( pBox );
            }
        } while( FALSE
                // JP 24.01.97: dann nur die akt. Zelle!!
            /*((SwEditShell*)this)->GoNextCrsr() && pCrsr != GetCrsr()*/ );
    }

    for( USHORT n = 0; n < aBoxes.Count(); ++n )
    {
        const SwTableBox* pSelBox = aBoxes[ n ];
        const SwTableBoxFmt* pTblFmt = (SwTableBoxFmt*)pSelBox->GetFrmFmt();
        if( !n )
        {
            // Formeln in die externe Darstellung bringen!
            const SwTable& rTbl = pSelBox->GetSttNd()->FindTableNode()->GetTable();

            SwTableFmlUpdate aTblUpdate( (SwTable*)&rTbl );
            aTblUpdate.eFlags = TBL_BOXNAME;
            ((SwDoc*)GetDoc())->UpdateTblFlds( &aTblUpdate );

            rSet.Put( pTblFmt->GetAttrSet() );
        }
        else
            rSet.MergeValues( pTblFmt->GetAttrSet() );
    }
    return 0 != rSet.Count();
}

void SwEditShell::SetTblBoxFormulaAttrs( const SfxItemSet& rSet )
{
    SET_CURR_SHELL( this );
    SwSelBoxes aBoxes;
    if( IsTableMode() )
        ::GetTblSelCrs( *this, aBoxes );
    else
    {
        SwPaM *pCrsr = GetCrsr();
        do {
            SwFrm *pFrm = GetCurrFrm();
            do {
                pFrm = pFrm->GetUpper();
            } while ( pFrm && !pFrm->IsCellFrm() );
            if ( pFrm )
            {
                SwTableBox *pBox = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
                aBoxes.Insert( pBox );
            }
        } while( FALSE
                // JP 24.01.97: dann nur die akt. Zelle!!
            /*GoNextCrsr() && pCrsr != GetCrsr()*/ );
    }

    // beim setzen einer Formel keine Ueberpruefung mehr vornehmen!
    if( SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMULA ))
        ClearTblBoxCntnt();

    StartAllAction();
    GetDoc()->StartUndo( UNDO_START );
    for( USHORT n = 0; n < aBoxes.Count(); ++n )
        GetDoc()->SetTblBoxFormulaAttrs( *aBoxes[ n ], rSet );
    GetDoc()->EndUndo( UNDO_END );
    EndAllAction();
}

    // Zellenbreiten ueber Min/Max Berechnung an Tabellenbreite anpassen
void SwEditShell::OptimizeTblBoxWidthMinMax()
{
    SET_CURR_SHELL( this );

    SwPaM *pCrsr = GetCrsr();
    SwTableNode* pTblNd = pCrsr->GetNode()->FindTableNode();
    if( pTblNd && !pTblNd->GetTable().IsTblComplex() )
    {
        SwTabFrm* pTabFrm = 0;
        SvULongs aMinArr( 16, 16 ), aMaxArr( 16, 16 );

        // ueber alle Spalten aller Zeilen und die Min/Max Breiten einsammeln
        SwTableLines& rTblLns = pTblNd->GetTable().GetTabLines();
        for( USHORT n = rTblLns.Count(); n; )
        {
            SwTableBoxes& rTblBxs = rTblLns[ --n ]->GetTabBoxes();
            for( USHORT i = 0; i < rTblBxs.Count(); ++i )
            {
                SwTableBox* pBox = rTblBxs[ i ];

                ULONG nMinCell = 0;
                ULONG nMaxCell = 0;

                // ueber alle Absaetze und die Min/Maxbreiten berechnen
                const SwStartNode* pSttNd = pBox->GetSttNd();
                SwNodeIndex aIdx( *pSttNd, 1 );
                SwNodeIndex aEnd( *pSttNd->EndOfSectionNode() );
                while( aIdx < aEnd )
                {
                    SwTxtNode *pTxtNd = aIdx.GetNode().GetTxtNode();
                    if( pTxtNd )
                    {
                        ULONG nMinCnts;
                        ULONG nMaxCnts;
                        ULONG nAbsMinCnts;
                        pTxtNd->GetMinMaxSize( aIdx.GetIndex(), nMinCnts,
                                               nMaxCnts, nAbsMinCnts );

                        if( nMinCnts > nMinCell )
                            nMinCell = nMinCnts;
                        if( nMaxCnts > nMaxCell )
                            nMaxCell = nMaxCnts;

                        if( !pTabFrm )
                        {
                            SwCntntFrm* pCFrm = pTxtNd->GetFrm( 0, 0, FALSE );
                            if( pCFrm )
                                pTabFrm = pCFrm->FindTabFrm();
                        }
                    }
                    aIdx++;
                }

                // Mindestbreite fuer Inhalt einhalten
                if( nMinCell < MINLAY )
                    nMinCell = MINLAY;
                if( nMaxCell < MINLAY )
                    nMaxCell = MINLAY;

                // Umrandung und Abstand zum Inhalt beachten
                const SvxBoxItem& rBoxItem = pBox->GetFrmFmt()->GetBox();
                USHORT nBrdDist = 0;
                if( rBoxItem.GetLeft() )
                {
                    USHORT nWidth = rBoxItem.GetLeft()->GetOutWidth() +
                                    rBoxItem.GetLeft()->GetInWidth();
                    if( !nBrdDist )
                        nBrdDist = rBoxItem.GetLeft()->GetDistance();

                    nMinCell += nWidth;
                    nMaxCell += nWidth;
                }
                if( rBoxItem.GetRight() )
                {
                    USHORT nWidth = rBoxItem.GetRight()->GetOutWidth() +
                                    rBoxItem.GetRight()->GetInWidth();
                    if( !nBrdDist )
                        nBrdDist = rBoxItem.GetRight()->GetDistance();

                    nMinCell += nWidth;
                    nMaxCell += nWidth;
                }
                if( !nBrdDist )
                    nBrdDist = MIN_BORDER_DIST;
                nMinCell += 2 * nBrdDist;
                nMaxCell += 2 * nBrdDist;

                // Max/Min-Werte in den Arrays merken
                if( i >= aMinArr.Count() )
                    aMinArr.Insert( nMinCell, i );
                else if( nMinCell > aMinArr[ i ] )
                    aMinArr.Replace( nMinCell, i );

                if( i >= aMaxArr.Count() )
                    aMaxArr.Insert( nMaxCell, i );
                else if( nMaxCell > aMaxArr[ i ] )
                    aMaxArr.Replace( nMaxCell, i );
            }
        }

        ASSERT( pTabFrm, "ohne TabFrm kann nichts berechnet werden" );
        if( pTabFrm )
        {
            // 2. Teil die Zellen an die Breiten anpassen
            ULONG nTableMin = 0;
            ULONG nTableMax = 0;
            for( n = aMinArr.Count(); n; )
            {
                nTableMin += aMinArr[ --n ];
                nTableMax += aMaxArr[   n ];
            }

            // Dann errechne mal die Breiten fuer die Spalten. Die Werte
            // werden im MinArray gesammelt!

            // die MinBreite ist schon groesser als der vorgesehene Platz
            ULONG nAbsTabWidth = pTabFrm->Prt().Width();
            if( nTableMin > nAbsTabWidth )
            {
                for( n = aMinArr.Count(); n; )
                {
                    ULONG nColMin = aMinArr[ --n ];
                    nColMin *= nAbsTabWidth;
                    nColMin /= nTableMin;
                    aMinArr.Replace( nColMin, n );
                }
            }
            // die MaxBreite ist kleiner als der vorgesehene Platz
            else if( nTableMax < nAbsTabWidth )
            {
                for( n = aMinArr.Count(); n; )
                {
                    ULONG nColMax = aMaxArr[ --n ];
                    nColMax *= nAbsTabWidth;
                    nColMax /= nTableMax;
                    aMinArr.Replace( nColMax, n );
                }
            }
            else
            {
                double nW = nAbsTabWidth - nTableMin;
                double nD = nTableMax == nTableMin ? 1 : nTableMax - nTableMin;
                for( n = 0; n < aMinArr.Count(); ++n )
                {
                    double nd = aMaxArr[ n ] - aMinArr[ n ];
                    ULONG nAbsColWidth = aMinArr[ n ] + (ULONG)(( nd * nW ) / nD );
                    aMinArr.Replace( nAbsColWidth, n );
                }
            }

            StartAllAction();
            GetDoc()->AppendUndoForAttrTable( pTblNd->GetTable() );

            for( n = 0; n < rTblLns.Count(); ++n )
            {
                SwTableBoxes& rTblBxs = rTblLns[ n ]->GetTabBoxes();
                for( USHORT i = rTblBxs.Count(); i; )
                {
                    SwTableBox* pBox = rTblBxs[ --i ];
                    pBox->ClaimFrmFmt()->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE,
                                                    aMinArr[ i ] ));
                }
            }

            SwTableFmt* pFmt = (SwTableFmt*)pTblNd->GetTable().GetFrmFmt();
            pFmt->LockModify();
            pFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, nAbsTabWidth ));
            pFmt->UnlockModify();

            EndAllAction();
        }
    }
}

BOOL SwEditShell::IsTableBoxTextFormat() const
{
    if( IsTableMode() )
        return FALSE;

    SwTableBox *pBox = 0;
    SwPaM *pCrsr = GetCrsr();
    {
        SwFrm *pFrm = GetCurrFrm();
        do {
            pFrm = pFrm->GetUpper();
        } while ( pFrm && !pFrm->IsCellFrm() );
        if ( pFrm )
            pBox = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
    }

    if( !pBox )
        return FALSE;

    ULONG nFmt;
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == pBox->GetFrmFmt()->GetAttrSet().GetItemState(
        RES_BOXATR_FORMAT, TRUE, &pItem ))
    {
        nFmt = ((SwTblBoxNumFormat*)pItem)->GetValue();
        return GetDoc()->GetNumberFormatter()->IsTextFormat( nFmt ) ||
                NUMBERFORMAT_TEXT == nFmt;
    }

    ULONG nNd = pBox->IsValidNumTxtNd();
    if( ULONG_MAX == nNd )
        return TRUE;

    const String& rTxt = GetDoc()->GetNodes()[ nNd ]->GetTxtNode()->GetTxt();
    if( !rTxt.Len() )
        return FALSE;

    double fVal;
    return !GetDoc()->GetNumberFormatter()->IsNumberFormat( rTxt, nFmt, fVal );
}


BOOL SwEditShell::SplitTable( USHORT eMode )
{
    BOOL bRet = FALSE;
    SwPaM *pCrsr = GetCrsr();
    if( pCrsr->GetNode()->FindTableNode() )
    {
        StartAllAction();
        GetDoc()->StartUndo();

        bRet = GetDoc()->SplitTable( *pCrsr->GetPoint(), eMode, TRUE );

        GetDoc()->EndUndo();
        ClearFEShellTabCols();
        EndAllAction();
    }
    return bRet;
}

BOOL SwEditShell::MergeTable( BOOL bWithPrev, USHORT nMode )
{
    BOOL bRet = FALSE;
    SwPaM *pCrsr = GetCrsr();
    if( pCrsr->GetNode()->FindTableNode() )
    {
        StartAllAction();
        GetDoc()->StartUndo();

        bRet = GetDoc()->MergeTable( *pCrsr->GetPoint(), bWithPrev, nMode );

        GetDoc()->EndUndo();
        ClearFEShellTabCols();
        EndAllAction();
    }
    return bRet;
}

BOOL SwEditShell::CanMergeTable( BOOL bWithPrev, BOOL* pChkNxtPrv ) const
{
    BOOL bRet = FALSE;
    const SwPaM *pCrsr = GetCrsr();
    const SwTableNode* pTblNd = pCrsr->GetNode()->FindTableNode();
    if( pTblNd && !pTblNd->GetTable().ISA( SwDDETable ))
    {
        const SwNodes& rNds = GetDoc()->GetNodes();
        if( pChkNxtPrv )
        {
            const SwTableNode* pChkNd = rNds[ pTblNd->GetIndex() - 1 ]->FindTableNode();
            if( pChkNd && !pChkNd->GetTable().ISA( SwDDETable ) )
                *pChkNxtPrv = TRUE, bRet = TRUE;        // mit Prev ist moeglich
            else
            {
                pChkNd = rNds[ pTblNd->EndOfSectionIndex() + 1 ]->GetTableNode();
                if( pChkNd && !pChkNd->GetTable().ISA( SwDDETable ) )
                    *pChkNxtPrv = FALSE, bRet = TRUE;       // mit Next ist moeglich
            }
        }
        else
        {
            if( bWithPrev )
                pTblNd = rNds[ pTblNd->GetIndex() - 1 ]->FindTableNode();
            else
                pTblNd = rNds[ pTblNd->EndOfSectionIndex() + 1 ]->GetTableNode();

            bRet = pTblNd && !pTblNd->GetTable().ISA( SwDDETable );
        }
    }
    return bRet;
}

