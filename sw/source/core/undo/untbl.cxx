/*************************************************************************
 *
 *  $RCSfile: untbl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:28 $
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

#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif

#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDARR_HXX
#include <ndarr.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>           // fuer die UndoIds
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _TABCOL_HXX
#include <tabcol.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _ROWFRM_HXX
#include <rowfrm.hxx>
#endif
#ifndef _CELLFRM_HXX
#include <cellfrm.hxx>
#endif
#ifndef _SWCACHE_HXX
#include <swcache.hxx>
#endif
#ifndef _TBLAFMT_HXX
#include <tblafmt.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
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
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _NODE2LAY_HXX
#include <node2lay.hxx>
#endif
#ifndef _TBLRWCL_HXX
#include <tblrwcl.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif

inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }
extern void ClearFEShellTabCols();

typedef SfxItemSet* SfxItemSetPtr;
SV_DECL_PTRARR_DEL( SfxItemSets, SfxItemSetPtr, 10, 5 )

typedef SwUndoSaveSection* SwUndoSaveSectionPtr;
SV_DECL_PTRARR_DEL( SwUndoSaveSections, SwUndoSaveSectionPtr, 0, 10 )

typedef SwUndoMove* SwUndoMovePtr;
SV_DECL_PTRARR_DEL( SwUndoMoves, SwUndoMovePtr, 0, 10 )

struct SwTblToTxtSave;
typedef SwTblToTxtSave* SwTblToTxtSavePtr;
SV_DECL_PTRARR_DEL( SwTblToTxtSaves, SwTblToTxtSavePtr, 0, 10 )

struct _UndoTblCpyTbl_Entry
{
    ULONG nBoxIdx, nOffset;
    SfxItemSet* pBoxNumAttr;
    SwUndoDelete* pUndo;

    _UndoTblCpyTbl_Entry( const SwTableBox& rBox );
    ~_UndoTblCpyTbl_Entry();
};
typedef _UndoTblCpyTbl_Entry* _UndoTblCpyTbl_EntryPtr;
SV_DECL_PTRARR_DEL( _UndoTblCpyTbl_Entries, _UndoTblCpyTbl_EntryPtr, 0, 10 )

class _SaveBox;
class _SaveLine;

class _SaveTable
{
    friend class _SaveBox;
    friend class _SaveLine;
    _SaveLine* pLine;
    const SwTable* pSwTable;
    SfxItemSets aSets;
    SwFrmFmts aFrmFmts;
    SfxItemSet aTblSet;
    USHORT nLineCount;
    BOOL bModifyBox : 1;
    BOOL bSaveFormula : 1;

public:
    _SaveTable( const SwTable& rTbl, USHORT nLnCnt = USHRT_MAX,
                BOOL bSaveFml = TRUE );
    ~_SaveTable();

    USHORT AddFmt( SwFrmFmt* pFmt );
    void NewFrmFmt( const SwClient* pLnBx, BOOL bIsLine, USHORT nFmtPos,
                    SwFrmFmt* pOldFmt );

    void RestoreAttr( SwTable& rTbl, BOOL bModifyBox = FALSE );
    void SaveCntntAttrs( SwDoc* pDoc );
    void CreateNew( SwTable& rTbl, BOOL bCreateFrms = TRUE,
                    BOOL bRestoreChart = TRUE );
};

class _SaveLine
{
    friend class _SaveTable;
    friend class _SaveBox;

    _SaveLine* pNext;
    _SaveBox* pBox;
    USHORT nItemSet;

public:

    _SaveLine( _SaveLine* pPrev, const SwTableLine& rLine, _SaveTable& rSTbl );
    ~_SaveLine();

    void RestoreAttr( SwTableLine& rLine, _SaveTable& rSTbl );
    void SaveCntntAttrs( SwDoc* pDoc );

    void CreateNew( SwTable& rTbl, SwTableBox& rParent, _SaveTable& rSTbl  );
};

class _SaveBox
{
    friend class _SaveLine;

    _SaveBox* pNext;
    ULONG nSttNode;
    USHORT nItemSet;
    union
    {
        SfxItemSets* pCntntAttrs;
        _SaveLine* pLine;
    } Ptrs;

public:
    _SaveBox( _SaveBox* pPrev, const SwTableBox& rBox, _SaveTable& rSTbl );
    ~_SaveBox();

    void RestoreAttr( SwTableBox& rBox, _SaveTable& rSTbl );
    void SaveCntntAttrs( SwDoc* pDoc );

    void CreateNew( SwTable& rTbl, SwTableLine& rParent, _SaveTable& rSTbl );
};

void InsertSort( SvUShorts& rArr, USHORT nIdx, USHORT* pInsPos = 0 );
void InsertSort( SvULongs& rArr, ULONG nIdx, USHORT* pInsPos = 0 );

#if defined( JP_DEBUG ) && !defined( PRODUCT )
#include "shellio.hxx"
void DumpDoc( SwDoc* pDoc, const String& rFileNm );
void CheckTable( const SwTable& );
#define DUMPDOC(p,s)    DumpDoc( p, s);
#define CHECKTABLE(t) CheckTable( t );
#else
#define DUMPDOC(p,s)
#define CHECKTABLE(t)
#endif

struct SwTblToTxtSave
{
    ULONG nNode;
    xub_StrLen nCntnt;
    SwHistory* pHstry;

    SwTblToTxtSave( SwDoc& rDoc, ULONG nNd, xub_StrLen nCntnt = STRING_MAXLEN );
    ~SwTblToTxtSave() { delete pHstry; }
};

SV_IMPL_PTRARR( SfxItemSets, SfxItemSetPtr )
SV_IMPL_PTRARR( SwUndoSaveSections, SwUndoSaveSectionPtr )
SV_IMPL_PTRARR( SwUndoMoves, SwUndoMovePtr )
SV_IMPL_PTRARR( SwTblToTxtSaves, SwTblToTxtSavePtr )
SV_IMPL_PTRARR( _UndoTblCpyTbl_Entries, _UndoTblCpyTbl_EntryPtr )

USHORT __FAR_DATA aSave_BoxCntntSet[] = {
    RES_CHRATR_COLOR, RES_CHRATR_CROSSEDOUT,
    RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
    RES_CHRATR_POSTURE, RES_CHRATR_POSTURE,
    RES_CHRATR_SHADOWED, RES_CHRATR_WEIGHT,
    RES_PARATR_ADJUST, RES_PARATR_ADJUST,
    0 };



SwUndoInsTbl::SwUndoInsTbl( const SwPosition& rPos, USHORT nCl, USHORT nRw,
                            USHORT nAdj, USHORT nInsert,
                            const SwTableAutoFmt* pTAFmt,
                            const SvUShorts* pColArr )
    : SwUndo( UNDO_INSTABLE ), nSttNode( rPos.nNode.GetIndex() ),
    nRows( nRw ), nCols( nCl ), nAdjust( nAdj ), pDDEFldType( 0 ),
    nInsTblFlags( nInsert ), pColWidth( 0 ), pRedlData( 0 ), pAutoFmt( 0 )
{
    if( pColArr )
    {
        pColWidth = new SvUShorts( 0, 1 );
        pColWidth->Insert( pColArr, 0 );
    }
    if( pTAFmt )
        pAutoFmt = new SwTableAutoFmt( *pTAFmt );

    // Redline beachten
    SwDoc& rDoc = *rPos.nNode.GetNode().GetDoc();
    if( rDoc.IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( REDLINE_INSERT, rDoc.GetRedlineAuthor() );
        SetRedlineMode( rDoc.GetRedlineMode() );
    }
}


SwUndoInsTbl::~SwUndoInsTbl()
{
    delete pDDEFldType;
    delete pColWidth;
    delete pRedlData;
    delete pAutoFmt;
}



void SwUndoInsTbl::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    SwNodeIndex aIdx( rDoc.GetNodes(), nSttNode );

    SwTableNode* pTblNd = aIdx.GetNode().GetTableNode();
    ASSERT( pTblNd, "kein TabellenNode" );
    pTblNd->DelFrms();

    if( IsRedlineOn( GetRedlineMode() ))
        rDoc.DeleteRedline( *pTblNd );
    RemoveIdxFromSection( rDoc, nSttNode );

    // harte SeitenUmbrueche am nachfolgenden Node verschieben
    SwCntntNode* pNextNd = rDoc.GetNodes()[ pTblNd->EndOfSectionIndex()+1 ]->GetCntntNode();
    if( pNextNd )
    {
        SwFrmFmt* pTableFmt = pTblNd->GetTable().GetFrmFmt();
        const SfxPoolItem *pItem;

        if( SFX_ITEM_SET == pTableFmt->GetItemState( RES_PAGEDESC,
            FALSE, &pItem ) )
            pNextNd->SetAttr( *pItem );

        if( SFX_ITEM_SET == pTableFmt->GetItemState( RES_BREAK,
            FALSE, &pItem ) )
            pNextNd->SetAttr( *pItem );
    }


    sTblNm = pTblNd->GetTable().GetFrmFmt()->GetName();
    if( pTblNd->GetTable().IsA( TYPE( SwDDETable )) )
        pDDEFldType = (SwDDEFieldType*)((SwDDETable&)pTblNd->GetTable()).
                                        GetDDEFldType()->Copy();

    rDoc.GetNodes().Delete( aIdx, pTblNd->EndOfSectionIndex() -
                                aIdx.GetIndex() + 1 );

    rUndoIter.pAktPam->DeleteMark();
    rUndoIter.pAktPam->GetPoint()->nNode = aIdx;
    rUndoIter.pAktPam->GetPoint()->nContent.Assign(
                            rUndoIter.pAktPam->GetCntntNode(), 0 );
}


void SwUndoInsTbl::Redo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();

    SwPosition aPos( *rUndoIter.pAktPam->GetPoint() );
    aPos.nNode = nSttNode;
    const SwTable* pTbl = rDoc.InsertTable( aPos, nRows, nCols,
                                    (SwHoriOrient)nAdjust,
                                    nInsTblFlags, pAutoFmt, pColWidth );
    ((SwFrmFmt*)pTbl->GetFrmFmt())->SetName( sTblNm );
    SwTableNode* pTblNode = (SwTableNode*)rDoc.GetNodes()[nSttNode]->GetTableNode();

    if( pDDEFldType )
    {
        SwDDEFieldType* pNewType = (SwDDEFieldType*)rDoc.InsertFldType(
                                                            *pDDEFldType);
        SwDDETable* pDDETbl = new SwDDETable( pTblNode->GetTable(), pNewType );
        pTblNode->SetNewTable( pDDETbl );       // setze die DDE-Tabelle
        delete pDDEFldType, pDDEFldType = 0;
    }

    if( (pRedlData && IsRedlineOn( GetRedlineMode() )) ||
        ( !( REDLINE_IGNORE & GetRedlineMode() ) &&
            rDoc.GetRedlineTbl().Count() ))
    {
        SwPaM aPam( *pTblNode->EndOfSectionNode(), *pTblNode, 1 );
        SwCntntNode* pCNd = aPam.GetCntntNode( FALSE );
        if( pCNd )
            aPam.GetMark()->nContent.Assign( pCNd, 0 );

        if( pRedlData && IsRedlineOn( GetRedlineMode() ) )
        {
            SwRedlineMode eOld = rDoc.GetRedlineMode();
            rDoc.SetRedlineMode_intern( eOld & ~REDLINE_IGNORE );

            rDoc.AppendRedline( new SwRedline( *pRedlData, aPam ));
            rDoc.SetRedlineMode_intern( eOld );
        }
        else
            rDoc.SplitRedline( aPam );
    }
}


void SwUndoInsTbl::Repeat( SwUndoIter& rUndoIter )
{
    // keine Tabelle in Tabelle
    if( !rUndoIter.pAktPam->GetNode()->FindTableNode() )
        rUndoIter.GetDoc().InsertTable( *rUndoIter.pAktPam->GetPoint(),
                                    nRows, nCols, (SwHoriOrient)nAdjust,
                                    nInsTblFlags, pAutoFmt, pColWidth );
}

// -----------------------------------------------------

SwTblToTxtSave::SwTblToTxtSave( SwDoc& rDoc, ULONG nNd, xub_StrLen nCnt )
    : nNode( nNd ), nCntnt( nCnt ), pHstry( 0 )
{
    // Attributierung des gejointen Node merken.
    if( USHRT_MAX != nCnt )
        ++nNd;

    SwTxtNode* pNd = rDoc.GetNodes()[ nNd ]->GetTxtNode();
    if( pNd )
    {
        pHstry = new SwHistory;

        pHstry->Add( pNd->GetTxtColl(), nNd, ND_TEXTNODE );
        if( pNd->GetpSwpHints() )
            pHstry->CopyAttr( pNd->GetpSwpHints(), nNd, 0,
                        pNd->GetTxt().Len(), FALSE );
        if( pNd->GetpSwAttrSet() )
            pHstry->CopyFmtAttr( *pNd->GetpSwAttrSet(), nNd );

        if( !pHstry->Count() )
            delete pHstry, pHstry = 0;
    }
}

SwUndoTblToTxt::SwUndoTblToTxt( const SwTable& rTbl, sal_Unicode cCh )
    : SwUndo( UNDO_TABLETOTEXT ),
    nSttNd( 0 ), nEndNd( 0 ), cTrenner( cCh ), pDDEFldType( 0 ),
    nAdjust( rTbl.GetFrmFmt()->GetHoriOrient().GetHoriOrient() ),
    sTblNm( rTbl.GetFrmFmt()->GetName() ),
    bHdlnRpt( rTbl.IsHeadlineRepeat() ),
    pHistory( 0 )
{
    pTblSave = new _SaveTable( rTbl );
    pBoxSaves = new SwTblToTxtSaves( (BYTE)rTbl.GetTabSortBoxes().Count() );

    if( rTbl.IsA( TYPE( SwDDETable ) ) )
        pDDEFldType = (SwDDEFieldType*)((SwDDETable&)rTbl).GetDDEFldType()->Copy();

    bCheckNumFmt = rTbl.GetFrmFmt()->GetDoc()->IsInsTblFormatNum();

    pHistory = new SwHistory;
    const SwTableNode* pTblNd = rTbl.GetTableNode();
    ULONG nTblStt = pTblNd->GetIndex(), nTblEnd = pTblNd->EndOfSectionIndex();

    const SwSpzFrmFmts& rFrmFmtTbl = *pTblNd->GetDoc()->GetSpzFrmFmts();
    for( USHORT n = 0; n < rFrmFmtTbl.Count(); ++n )
    {
        const SwPosition* pAPos;
        const SwFrmFmt* pFmt = rFrmFmtTbl[ n ];
        const SwFmtAnchor* pAnchor = &pFmt->GetAnchor();
        if( 0 != ( pAPos = pAnchor->GetCntntAnchor()) &&
            ( FLY_AUTO_CNTNT == pAnchor->GetAnchorId() ||
              FLY_AT_CNTNT == pAnchor->GetAnchorId() ) &&
            nTblStt <= pAPos->nNode.GetIndex() &&
            pAPos->nNode.GetIndex() < nTblEnd )
        {
            pHistory->Add( *pFmt );
        }
    }

    if( !pHistory->Count() )
        delete pHistory, pHistory = 0;
}


SwUndoTblToTxt::~SwUndoTblToTxt()
{
    delete pDDEFldType;
    delete pTblSave;
    delete pBoxSaves;
    delete pHistory;
}



void SwUndoTblToTxt::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    SwPaM* pPam = rUndoIter.pAktPam;

    SwNodeIndex aFrmIdx( rDoc.GetNodes(), nSttNd );
    SwNodeIndex aEndIdx( rDoc.GetNodes(), nEndNd );

    pPam->GetPoint()->nNode = aFrmIdx;
    pPam->SetMark();
    pPam->GetPoint()->nNode = aEndIdx;
    rDoc.DelNumRules( *pPam );
    pPam->DeleteMark();
    SwNode2Layout* pNode2Layout = NULL;

    // dann sammel mal alle Uppers ein
    SwNode2Layout aNode2Layout( aFrmIdx.GetNode() );

    // erzeuge die TabelleNode Structur
    SwTableNode* pTblNd = rDoc.GetNodes().UndoTableToText( nSttNd, nEndNd,
                                                            *pBoxSaves );
    SwTableFmt* pTableFmt = rDoc.MakeTblFrmFmt( sTblNm, rDoc.GetDfltFrmFmt() );
    pTableFmt->Add( &pTblNd->GetTable() );      // das Frame-Format setzen
    pTblNd->GetTable().SetHeadlineRepeat( bHdlnRpt );

    // erzeuge die alte Tabellen Struktur
    pTblSave->CreateNew( pTblNd->GetTable() );

    if( pDDEFldType )
    {
        SwDDEFieldType* pNewType = (SwDDEFieldType*)rDoc.InsertFldType(
                                                            *pDDEFldType);
        SwDDETable* pDDETbl = new SwDDETable( pTblNd->GetTable(), pNewType );
        pTblNd->SetNewTable( pDDETbl );     // setze die DDE-Tabelle
        delete pDDEFldType, pDDEFldType = 0;
    }

    if( bCheckNumFmt )
    {
        SwTableSortBoxes& rBxs = pTblNd->GetTable().GetTabSortBoxes();
        for( USHORT nBoxes = rBxs.Count(); nBoxes; )
            rDoc.ChkBoxNumFmt( *rBxs[ --nBoxes ], FALSE );
    }

    if( pHistory )
    {
        USHORT nTmpEnd = pHistory->GetTmpEnd();
        pHistory->TmpRollback( &rDoc, 0 );
        pHistory->SetTmpEnd( nTmpEnd );
    }

    aNode2Layout.RestoreUpperFrms( rDoc.GetNodes(),
                                   pTblNd->GetIndex(), pTblNd->GetIndex()+1 );

    // will man eine TabellenSelektion ??
    pPam->DeleteMark();
    pPam->GetPoint()->nNode = *pTblNd->EndOfSectionNode();
    pPam->SetMark();
    pPam->GetPoint()->nNode = *pPam->GetNode()->StartOfSectionNode();
    pPam->Move( fnMoveForward, fnGoCntnt );
    pPam->Exchange();
    pPam->Move( fnMoveBackward, fnGoCntnt );

    ClearFEShellTabCols();
}

    // steht im untbl.cxx und darf nur vom Undoobject gerufen werden
SwTableNode* SwNodes::UndoTableToText( ULONG nSttNd, ULONG nEndNd,
                                const SwTblToTxtSaves& rSavedData )
{
    SwNodeIndex aSttIdx( *this, nSttNd );
    SwNodeIndex aEndIdx( *this, nEndNd+1 );

    SwTableNode * pTblNd = new SwTableNode( aSttIdx );
    SwEndNode* pEndNd = new SwEndNode( aEndIdx, *pTblNd  );

    aEndIdx = *pEndNd;

    // alle im Bereich liegdenden Nodes den TabellenNode als StartNode setzen
    SwNode* pNd;
    {
        ULONG n, nTmpEnd = aEndIdx.GetIndex();
        for( n = pTblNd->GetIndex() + 1; n < nTmpEnd; ++n )
            if( ( pNd = (*this)[ n ] )->IsCntntNode() )
                ((SwCntntNode*)pNd)->DelFrms();
    }

    // dann die Tabellen Struktur teilweise aufbauen. Erstmal eine Line
    // in der alle Boxen stehen! Die korrekte Struktur kommt dann aus der
    // SaveStruct
    SwTableBoxFmt* pBoxFmt = GetDoc()->MakeTableBoxFmt();
    SwTableLineFmt* pLineFmt = GetDoc()->MakeTableLineFmt();
    SwTableLine* pLine = new SwTableLine( pLineFmt, rSavedData.Count(), 0 );
    pTblNd->GetTable().GetTabLines().C40_INSERT( SwTableLine, pLine, 0 );

    SvULongs aBkmkArr( 0, 4 );
    for( USHORT n = rSavedData.Count(); n; )
    {
        SwTblToTxtSave* pSave = rSavedData[ --n ];
        aSttIdx = pSave->nNode;
        SwTxtNode* pTxtNd = aSttIdx.GetNode().GetTxtNode();
        if( USHRT_MAX != pSave->nCntnt )
        {
            // an der ContentPosition splitten, das vorherige Zeichen
            // loeschen (ist der Trenner!)
            ASSERT( pTxtNd, "Wo ist der TextNode geblieben?" );
            SwIndex aCntPos( pTxtNd, pSave->nCntnt - 1 );

            pTxtNd->Erase( aCntPos, 1 );
            SwCntntNode* pNewNd = pTxtNd->SplitNode(
                                        SwPosition( aSttIdx, aCntPos ));
            if( aBkmkArr.Count() )
                _RestoreCntntIdx( aBkmkArr, *pNewNd, pSave->nCntnt,
                                                    pSave->nCntnt + 1 );
        }
        else
        {
            if( aBkmkArr.Count() )
                aBkmkArr.Remove( 0, aBkmkArr.Count() );
            if( pTxtNd )
                _SaveCntntIdx( GetDoc(), aSttIdx.GetIndex(),
                                pTxtNd->GetTxt().Len(), aBkmkArr );
        }

        if( pTxtNd )
        {
            if( pTxtNd->GetpSwAttrSet() )
                pTxtNd->ResetAllAttr();

            if( pTxtNd->GetpSwpHints() )
                pTxtNd->ClearSwpHintsArr( FALSE, FALSE );
        }

        if( pSave->pHstry )
        {
            USHORT nTmpEnd = pSave->pHstry->GetTmpEnd();
            pSave->pHstry->TmpRollback( GetDoc(), 0 );
            pSave->pHstry->SetTmpEnd( nTmpEnd );
        }

        SwStartNode* pSttNd = new SwStartNode( aSttIdx, ND_STARTNODE,
                                                SwTableBoxStartNode );
        pSttNd->pStartOfSection = pTblNd;
        new SwEndNode( aEndIdx, *pSttNd );

        for( ULONG i = aSttIdx.GetIndex(); i < aEndIdx.GetIndex()-1; ++i )
        {
            pNd = (*this)[ i ];
            pNd->pStartOfSection = pSttNd;
            if( pNd->IsStartNode() )
                i = pNd->EndOfSectionIndex();
        }

        SwTableBox* pBox = new SwTableBox( pBoxFmt, *pSttNd, pLine );
        pLine->GetTabBoxes().C40_INSERT( SwTableBox, pBox, 0 );
        aEndIdx = *pSttNd;
    }
    return pTblNd;
}


void SwUndoTblToTxt::Redo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    SwPaM* pPam = rUndoIter.pAktPam;


    pPam->GetPoint()->nNode = nSttNd;
    pPam->GetPoint()->nContent.Assign( 0, 0 );
    SwNodeIndex aSaveIdx( pPam->GetPoint()->nNode, -1 );

    pPam->SetMark();            // alle Indizies abmelden
    pPam->DeleteMark();

    SwTableNode* pTblNd = pPam->GetNode()->GetTableNode();
    ASSERT( pTblNd, "keinen TableNode gefunden" );

    if( pTblNd->GetTable().IsA( TYPE( SwDDETable )) )
        pDDEFldType = (SwDDEFieldType*)((SwDDETable&)pTblNd->GetTable()).
                                                GetDDEFldType()->Copy();

    rDoc.TableToText( pTblNd, cTrenner );

    aSaveIdx++;
    SwCntntNode* pCNd = aSaveIdx.GetNode().GetCntntNode();
    if( !pCNd && 0 == ( pCNd = rDoc.GetNodes().GoNext( &aSaveIdx ) ) &&
        0 == ( pCNd = rDoc.GetNodes().GoPrevious( &aSaveIdx )) )
        ASSERT( FALSE, "wo steht denn nun der TextNode" );

    pPam->GetPoint()->nNode = aSaveIdx;
    pPam->GetPoint()->nContent.Assign( pCNd, 0 );

    pPam->SetMark();            // alle Indizies abmelden
    pPam->DeleteMark();
}


void SwUndoTblToTxt::Repeat( SwUndoIter& rUndoIter )
{
    SwTableNode* pTblNd = rUndoIter.pAktPam->GetNode()->FindTableNode();
    if( pTblNd )
    {
        // bewege den Cursor aus der Tabelle
        SwPaM* pPam = rUndoIter.pAktPam;
        pPam->GetPoint()->nNode = *pTblNd->EndOfSectionNode();
        pPam->Move( fnMoveForward, fnGoCntnt );
        pPam->SetMark();
        pPam->DeleteMark();

        rUndoIter.GetDoc().TableToText( pTblNd, cTrenner );
    }
}

void SwUndoTblToTxt::SetRange( const SwNodeRange& rRg )
{
    nSttNd = rRg.aStart.GetIndex();
    nEndNd = rRg.aEnd.GetIndex();
}

void SwUndoTblToTxt::AddBoxPos( SwDoc& rDoc, ULONG nNdIdx, xub_StrLen nCntntIdx )
{
    SwTblToTxtSave* pNew = new SwTblToTxtSave( rDoc, nNdIdx, nCntntIdx );
    pBoxSaves->Insert( pNew, pBoxSaves->Count() );
}

// -----------------------------------------------------

SwUndoTxtToTbl::SwUndoTxtToTbl( const SwPaM& rRg, sal_Unicode cCh, USHORT nAdj,
                                USHORT nInsert, const SwTableAutoFmt* pAFmt )
    : SwUndo( UNDO_TEXTTOTABLE ), SwUndRng( rRg ), pAutoFmt( 0 ),
    nAdjust( nAdj ), cTrenner( cCh ), pDelBoxes( 0 ), nInsTblFlags( nInsert ),
    pHistory( 0 )

{
    if( pAFmt )
        pAutoFmt = new SwTableAutoFmt( *pAFmt );

    const SwPosition* pEnd = rRg.End();
    SwNodes& rNds = rRg.GetDoc()->GetNodes();
    bSplitEnd = pEnd->nContent.GetIndex() && ( pEnd->nContent.GetIndex()
                        != pEnd->nNode.GetNode().GetCntntNode()->Len() ||
                pEnd->nNode.GetIndex() >= rNds.GetEndOfContent().GetIndex()-1 );
}

SwUndoTxtToTbl::~SwUndoTxtToTbl()
{
    delete pDelBoxes;
    delete pAutoFmt;
}

void SwUndoTxtToTbl::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    ULONG nTblNd = nSttNode;
    if( nSttCntnt )
        ++nTblNd;       // Node wurde vorher gesplittet
    SwNodeIndex aIdx( rDoc.GetNodes(), nTblNd );
    SwTableNode* pTNd = rDoc.GetNodes()[ aIdx ]->GetTableNode();
    ASSERT( pTNd, "keinen Tabellen-Node gefunden" );

    RemoveIdxFromSection( rDoc, nTblNd );

    sTblNm = pTNd->GetTable().GetFrmFmt()->GetName();

    if( pHistory )
    {
        pHistory->TmpRollback( &rDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }

    if( pDelBoxes )
    {
        SwTable& rTbl = pTNd->GetTable();
        for( USHORT n = pDelBoxes->Count(); n; )
        {
            SwTableBox* pBox = rTbl.GetTblBox( (*pDelBoxes)[ --n ] );
            if( pBox )
                ::_DeleteBox( rTbl, pBox, 0, FALSE, FALSE );
            else
                ASSERT( !this, "Wo ist die Box geblieben?" );
        }
    }

    SwNodeIndex aEndIdx( *pTNd->EndOfSectionNode() );
    rDoc.TableToText( pTNd, 0x0b == cTrenner ? 0x09 : cTrenner );

    // am Start wieder zusammenfuegen ?
    SwPosition* pPos = rUndoIter.pAktPam->GetPoint();
    if( nSttCntnt )
    {
        pPos->nNode = nTblNd;
        pPos->nContent.Assign( rDoc.GetNodes()[ pPos->nNode ]->GetCntntNode(), 0 );
        if( rUndoIter.pAktPam->Move( fnMoveBackward, fnGoCntnt))
        {
            SwNodeIndex& rIdx = rUndoIter.pAktPam->GetPoint()->nNode;

            // dann die Crsr/etc. nochmal relativ verschieben
            RemoveIdxRel( rIdx.GetIndex()+1, *pPos );

            rIdx.GetNode().GetCntntNode()->JoinNext();
        }
    }

    // am Ende wieder zusammenfuegen ?
    if( bSplitEnd )
    {
        SwNodeIndex& rIdx = pPos->nNode;
        rIdx = nEndNode;
        SwTxtNode* pTxtNd = rIdx.GetNode().GetTxtNode();
        if( pTxtNd && pTxtNd->CanJoinNext() )
        {
            rUndoIter.pAktPam->GetMark()->nContent.Assign( 0, 0 );
            rUndoIter.pAktPam->GetPoint()->nContent.Assign( 0, 0 );

            // dann die Crsr/etc. nochmal relativ verschieben
            pPos->nContent.Assign( pTxtNd, pTxtNd->GetTxt().Len() );
            RemoveIdxRel( nEndNode + 1, *pPos );

            pTxtNd->JoinNext();
        }
    }

    SetPaM( rUndoIter );        // manipulierten Bereich selectieren
}


void SwUndoTxtToTbl::Redo( SwUndoIter& rUndoIter )
{
    SetPaM( rUndoIter );
    RemoveIdxFromRange( *rUndoIter.pAktPam, FALSE );
    SetPaM( rUndoIter );

    const SwTable* pTable = rUndoIter.GetDoc().TextToTable(
                *rUndoIter.pAktPam, cTrenner, (SwHoriOrient)nAdjust,
                nInsTblFlags, pAutoFmt );
    ((SwFrmFmt*)pTable->GetFrmFmt())->SetName( sTblNm );
}


void SwUndoTxtToTbl::Repeat( SwUndoIter& rUndoIter )
{
    // keine Tabelle in Tabelle
    if( !rUndoIter.pAktPam->GetNode()->FindTableNode() )
        rUndoIter.GetDoc().TextToTable( *rUndoIter.pAktPam, cTrenner,
                            (SwHoriOrient)nAdjust, nInsTblFlags, pAutoFmt );
}

void SwUndoTxtToTbl::AddFillBox( const SwTableBox& rBox )
{
    if( !pDelBoxes )
        pDelBoxes = new SvULongs;
    pDelBoxes->Insert( rBox.GetSttIdx(), pDelBoxes->Count() );
}

SwHistory& SwUndoTxtToTbl::GetHistory()
{
    if( !pHistory )
        pHistory = new SwHistory;
    return *pHistory;
}

// -----------------------------------------------------

SwUndoTblHeadline::SwUndoTblHeadline( const SwTable& rTbl, BOOL bOldHdl )
    : SwUndo( UNDO_TABLEHEADLINE ),
    bOldHeadline( bOldHdl )
{
    ASSERT( rTbl.GetTabSortBoxes().Count(), "Tabelle ohne Inhalt" );
    const SwStartNode *pSttNd = rTbl.GetTabSortBoxes()[ 0 ]->GetSttNd();
    ASSERT( pSttNd, "Box ohne Inhalt" );

    nTblNd = pSttNd->StartOfSectionIndex();
}


void SwUndoTblHeadline::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    SwTableNode* pTNd = rDoc.GetNodes()[ nTblNd ]->GetTableNode();
    ASSERT( pTNd, "keinen Tabellen-Node gefunden" );

    rDoc.SetHeadlineRepeat( pTNd->GetTable(), bOldHeadline );
}


void SwUndoTblHeadline::Redo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();

    SwTableNode* pTNd = rDoc.GetNodes()[ nTblNd ]->GetTableNode();
    ASSERT( pTNd, "keinen Tabellen-Node gefunden" );

    rDoc.SetHeadlineRepeat( pTNd->GetTable(), !bOldHeadline );
}


void SwUndoTblHeadline::Repeat( SwUndoIter& rUndoIter )
{
    SwTableNode* pTblNd = rUndoIter.pAktPam->GetNode()->FindTableNode();
    if( pTblNd )
        rUndoIter.GetDoc().SetHeadlineRepeat( pTblNd->GetTable(),
                                                !bOldHeadline );
}


/*  */



_SaveTable::_SaveTable( const SwTable& rTbl, USHORT nLnCnt, BOOL bSaveFml )
    : aTblSet( *rTbl.GetFrmFmt()->GetAttrSet().GetPool(), aTableSetRange ),
    nLineCount( nLnCnt ), pSwTable( &rTbl ), bSaveFormula( bSaveFml )
{
    bModifyBox = FALSE;
    aTblSet.Put( rTbl.GetFrmFmt()->GetAttrSet() );
    pLine = new _SaveLine( 0, *rTbl.GetTabLines()[ 0 ], *this );

    _SaveLine* pLn = pLine;
    if( USHRT_MAX == nLnCnt )
        nLnCnt = rTbl.GetTabLines().Count();
    for( USHORT n = 1; n < nLnCnt; ++n )
        pLn = new _SaveLine( pLn, *rTbl.GetTabLines()[ n ], *this );

    aFrmFmts.Remove( 0, aFrmFmts.Count() );
    pSwTable = 0;
}


_SaveTable::~_SaveTable()
{
    delete pLine;
}


USHORT _SaveTable::AddFmt( SwFrmFmt* pFmt )
{
    USHORT nRet = aFrmFmts.GetPos( pFmt );
    if( USHRT_MAX == nRet )
    {
        // Kopie vom ItemSet anlegen
        SfxItemSet* pSet = new SfxItemSet( *pFmt->GetAttrSet().GetPool(),
                                            aTableBoxSetRange );
        pSet->Put( pFmt->GetAttrSet() );
        //JP 20.04.98: Bug 49502 - wenn eine Formel gesetzt ist, nie den
        //              Value mit sichern. Der muss gegebenfalls neu
        //              errechnet werden!
        //JP 30.07.98: Bug 54295 - Formeln immer im Klartext speichern
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pSet->GetItemState( RES_BOXATR_FORMULA, TRUE, &pItem ))
        {
            pSet->ClearItem( RES_BOXATR_VALUE );
            if( pSwTable && bSaveFormula )
            {
                SwTableFmlUpdate aMsgHnt( pSwTable );
                aMsgHnt.eFlags = TBL_BOXNAME;
                ((SwTblBoxFormula*)pItem)->ChgDefinedIn( pFmt );
                ((SwTblBoxFormula*)pItem)->ChangeState( &aMsgHnt );
                ((SwTblBoxFormula*)pItem)->ChgDefinedIn( 0 );
            }
        }
        aSets.Insert( pSet, (nRet = aSets.Count() ) );
        aFrmFmts.Insert( pFmt, nRet );
    }
    return nRet;
}


void _SaveTable::RestoreAttr( SwTable& rTbl, BOOL bMdfyBox )
{
    bModifyBox = bMdfyBox;

    // zuerst die Attribute des TabellenFrmFormates zurueck holen
    SwFrmFmt* pFmt = rTbl.GetFrmFmt();
    SfxItemSet& rFmtSet  = (SfxItemSet&)pFmt->GetAttrSet();
    rFmtSet.ClearItem();
    rFmtSet.Put( aTblSet );

    if( pFmt->IsInCache() )
    {
        SwFrm::GetCache().Delete( pFmt );
        pFmt->SetInCache( FALSE );
    }

    // zur Sicherheit alle Tableframes invalidieren
    SwClientIter aIter( *pFmt );
    for( SwClient* pLast = aIter.First( TYPE( SwFrm ) ); pLast; pLast = aIter.Next() )
        if( ((SwTabFrm*)pLast)->GetTable() == &rTbl )
        {
            ((SwTabFrm*)pLast)->InvalidateAll();
            ((SwTabFrm*)pLast)->SetCompletePaint();
        }

    // FrmFmts mit Defaults (0) fuellen
    pFmt = 0;
    for( USHORT n = aSets.Count(); n; --n )
        aFrmFmts.Insert( pFmt, aFrmFmts.Count() );

    USHORT nLnCnt = nLineCount;
    if( USHRT_MAX == nLnCnt )
        nLnCnt = rTbl.GetTabLines().Count();

    _SaveLine* pLn = pLine;
    for( n = 0; n < nLnCnt; ++n, pLn = pLn->pNext )
    {
        if( !pLn )
        {
            ASSERT( !this, "Anzahl der Lines hat sich veraendert" );
            break;
        }

        pLn->RestoreAttr( *rTbl.GetTabLines()[ n ], *this );
    }

    aFrmFmts.Remove( 0, aFrmFmts.Count() );
    bModifyBox = FALSE;
}


void _SaveTable::SaveCntntAttrs( SwDoc* pDoc )
{
    pLine->SaveCntntAttrs( pDoc );
}


void _SaveTable::CreateNew( SwTable& rTbl, BOOL bCreateFrms,
                            BOOL bRestoreChart )
{
    _FndBox aTmpBox( 0, 0 );
    if( bRestoreChart )
        aTmpBox.SaveChartData( rTbl );
    aTmpBox.DelFrms( rTbl );

    // zuerst die Attribute des TabellenFrmFormates zurueck holen
    SwFrmFmt* pFmt = rTbl.GetFrmFmt();
    SfxItemSet& rFmtSet  = (SfxItemSet&)pFmt->GetAttrSet();
    rFmtSet.ClearItem();
    rFmtSet.Put( aTblSet );

    if( pFmt->IsInCache() )
    {
        SwFrm::GetCache().Delete( pFmt );
        pFmt->SetInCache( FALSE );
    }

    // SwTableBox muss ein Format haben!!
    SwTableBox aParent( (SwTableBoxFmt*)pFmt, rTbl.GetTabLines().Count(), 0 );

    // FrmFmts mit Defaults (0) fuellen
    pFmt = 0;
    for( USHORT n = aSets.Count(); n; --n )
        aFrmFmts.Insert( pFmt, aFrmFmts.Count() );

    pLine->CreateNew( rTbl, aParent, *this );
    aFrmFmts.Remove( 0, aFrmFmts.Count() );

    // die neuen Lines eintragen, die alten loeschen
    USHORT nOldLines = nLineCount;
    if( USHRT_MAX == nLineCount )
        nOldLines = rTbl.GetTabLines().Count();

    for( n = 0; n < aParent.GetTabLines().Count(); ++n )
    {
        SwTableLine* pLn = aParent.GetTabLines()[ n ];
        pLn->SetUpper( 0 );
        if( n < nOldLines )
        {
            SwTableLine* pOld = rTbl.GetTabLines()[ n ];
            rTbl.GetTabLines().C40_REPLACE( SwTableLine, pLn, n );
            delete pOld;
        }
        else
            rTbl.GetTabLines().C40_INSERT( SwTableLine, pLn, n );
    }
    if( n < nOldLines )
        rTbl.GetTabLines().DeleteAndDestroy( n, nOldLines - n );

    aParent.GetTabLines().Remove( 0, n );

    if( bCreateFrms )
        aTmpBox.MakeFrms( rTbl );
    if( bRestoreChart )
        aTmpBox.RestoreChartData( rTbl );
}


void _SaveTable::NewFrmFmt( const SwClient* pLnBx, BOOL bIsLine,
                            USHORT nFmtPos, SwFrmFmt* pOldFmt )
{
    SwDoc* pDoc = pOldFmt->GetDoc();

    SwFrmFmt* pFmt = aFrmFmts[ nFmtPos ];
    if( !pFmt )
    {
        if( bIsLine )
            pFmt = pDoc->MakeTableLineFmt();
        else
            pFmt = pDoc->MakeTableBoxFmt();
        pFmt->SetAttr( *aSets[ nFmtPos ] );
        aFrmFmts.Replace( pFmt, nFmtPos );
    }

    //Erstmal die Frms ummelden.
    SwClientIter aIter( *pOldFmt );
    for( SwClient* pLast = aIter.First( TYPE( SwFrm ) ); pLast; pLast = aIter.Next() )
        if( bIsLine ? pLnBx == ((SwRowFrm*)pLast)->GetTabLine()
                    : pLnBx == ((SwCellFrm*)pLast)->GetTabBox() )
        {
            pFmt->Add( pLast );
            ((SwFrm*)pLast)->InvalidateAll();
            ((SwFrm*)pLast)->ReinitializeFrmSizeAttrFlags();
        }

    //Jetzt noch mich selbst ummelden.
    pFmt->Add( (SwClient*)pLnBx );

    if( bModifyBox && !bIsLine )
    {
        const SfxPoolItem& rOld = pOldFmt->GetAttr( RES_BOXATR_FORMAT ),
                         & rNew = pFmt->GetAttr( RES_BOXATR_FORMAT );
        if( rOld != rNew )
            pFmt->Modify( (SfxPoolItem*)&rOld, (SfxPoolItem*)&rNew );
    }

    if( !pOldFmt->GetDepends() )
        delete pOldFmt;

}


_SaveLine::_SaveLine( _SaveLine* pPrev, const SwTableLine& rLine, _SaveTable& rSTbl )
    : pNext( 0 )
{
    if( pPrev )
        pPrev->pNext = this;

    nItemSet = rSTbl.AddFmt( rLine.GetFrmFmt() );

    pBox = new _SaveBox( 0, *rLine.GetTabBoxes()[ 0 ], rSTbl );
    _SaveBox* pBx = pBox;
    for( USHORT n = 1; n < rLine.GetTabBoxes().Count(); ++n )
        pBx = new _SaveBox( pBx, *rLine.GetTabBoxes()[ n ], rSTbl );
}


_SaveLine::~_SaveLine()
{
    delete pBox;
    delete pNext;
}


void _SaveLine::RestoreAttr( SwTableLine& rLine, _SaveTable& rSTbl )
{
    rSTbl.NewFrmFmt( &rLine, TRUE, nItemSet, rLine.GetFrmFmt() );

    _SaveBox* pBx = pBox;
    for( USHORT n = 0; n < rLine.GetTabBoxes().Count(); ++n, pBx = pBx->pNext )
    {
        if( !pBx )
        {
            ASSERT( !this, "Anzahl der Boxen hat sich veraendert" );
            break;
        }
        pBx->RestoreAttr( *rLine.GetTabBoxes()[ n ], rSTbl );
    }
}


void _SaveLine::SaveCntntAttrs( SwDoc* pDoc )
{
    pBox->SaveCntntAttrs( pDoc );
    if( pNext )
        pNext->SaveCntntAttrs( pDoc );
}


void _SaveLine::CreateNew( SwTable& rTbl, SwTableBox& rParent, _SaveTable& rSTbl )
{
    SwTableLineFmt* pFmt = (SwTableLineFmt*)rSTbl.aFrmFmts[ nItemSet ];
    if( !pFmt )
    {
        SwDoc* pDoc = rTbl.GetFrmFmt()->GetDoc();
        pFmt = pDoc->MakeTableLineFmt();
        pFmt->SetAttr( *rSTbl.aSets[ nItemSet ] );
        rSTbl.aFrmFmts.Replace( pFmt, nItemSet );
    }
    SwTableLine* pNew = new SwTableLine( pFmt, 1, &rParent );
    rParent.GetTabLines().C40_INSERT( SwTableLine, pNew, rParent.GetTabLines().Count() );

    pBox->CreateNew( rTbl, *pNew, rSTbl );

    if( pNext )
        pNext->CreateNew( rTbl, rParent, rSTbl );
}


_SaveBox::_SaveBox( _SaveBox* pPrev, const SwTableBox& rBox, _SaveTable& rSTbl )
    : nSttNode( ULONG_MAX ), pNext( 0 )
{
    Ptrs.pLine = 0;

    if( pPrev )
        pPrev->pNext = this;

    nItemSet = rSTbl.AddFmt( rBox.GetFrmFmt() );

    if( rBox.GetSttNd() )
        nSttNode = rBox.GetSttIdx();
    else
    {
        Ptrs.pLine = new _SaveLine( 0, *rBox.GetTabLines()[ 0 ], rSTbl );

        _SaveLine* pLn = Ptrs.pLine;
        for( USHORT n = 1; n < rBox.GetTabLines().Count(); ++n )
            pLn = new _SaveLine( pLn, *rBox.GetTabLines()[ n ], rSTbl );
    }
}


_SaveBox::~_SaveBox()
{
    if( ULONG_MAX == nSttNode )     // keine EndBox
        delete Ptrs.pLine;
    else
        delete Ptrs.pCntntAttrs;
    delete pNext;
}


void _SaveBox::RestoreAttr( SwTableBox& rBox, _SaveTable& rSTbl )
{
    rSTbl.NewFrmFmt( &rBox, FALSE, nItemSet, rBox.GetFrmFmt() );

    if( ULONG_MAX == nSttNode )     // keine EndBox
    {
        if( !rBox.GetTabLines().Count() )
        {
            ASSERT( !this, "Anzahl der Lines hat sich veraendert" );
        }
        else
        {
            _SaveLine* pLn = Ptrs.pLine;
            for( USHORT n = 0; n < rBox.GetTabLines().Count(); ++n, pLn = pLn->pNext )
            {
                if( !pLn )
                {
                    ASSERT( !this, "Anzahl der Lines hat sich veraendert" );
                    break;
                }

                pLn->RestoreAttr( *rBox.GetTabLines()[ n ], rSTbl );
            }
        }
    }
    else if( rBox.GetSttNd() && rBox.GetSttIdx() == nSttNode )
    {
        if( Ptrs.pCntntAttrs )
        {
            SwNodes& rNds = rBox.GetFrmFmt()->GetDoc()->GetNodes();
            USHORT nSet = 0;
            ULONG nEnd = rBox.GetSttNd()->EndOfSectionIndex();
            for( ULONG n = nSttNode + 1; n < nEnd; ++n )
            {
                SwCntntNode* pCNd = rNds[ n ]->GetCntntNode();
                if( pCNd )
                {
                    SfxItemSet* pSet = (*Ptrs.pCntntAttrs)[ nSet++ ];
                    if( pSet )
                    {
                        USHORT *pRstAttr = aSave_BoxCntntSet;
                        while( *pRstAttr )
                        {
                            pCNd->ResetAttr( *pRstAttr, *(pRstAttr+1) );
                            pRstAttr += 2;
                        }
                        pCNd->SetAttr( *pSet );
                    }
                    else
                        pCNd->ResetAllAttr();
                }
            }
        }
    }
    else
    {
        ASSERT( !this, "Box nicht mehr am gleichen Node" );
    }
}


void _SaveBox::SaveCntntAttrs( SwDoc* pDoc )
{
    if( ULONG_MAX == nSttNode )     // keine EndBox
    {
        // weiter in der Line
        Ptrs.pLine->SaveCntntAttrs( pDoc );
    }
    else
    {
        USHORT nSet = 0;
        ULONG nEnd = pDoc->GetNodes()[ nSttNode ]->EndOfSectionIndex();
        Ptrs.pCntntAttrs = new SfxItemSets( (BYTE)(nEnd - nSttNode - 1 ), 5 );
        for( ULONG n = nSttNode + 1; n < nEnd; ++n )
        {
            SwCntntNode* pCNd = pDoc->GetNodes()[ n ]->GetCntntNode();
            if( pCNd )
            {
                SfxItemSet* pSet = 0;
                if( pCNd->GetpSwAttrSet() )
                {
                    pSet = new SfxItemSet( pDoc->GetAttrPool(),
                                            aSave_BoxCntntSet );
                    pSet->Put( *pCNd->GetpSwAttrSet() );
                }

                Ptrs.pCntntAttrs->Insert( pSet, Ptrs.pCntntAttrs->Count() );
            }
        }
    }
    if( pNext )
        pNext->SaveCntntAttrs( pDoc );
}


void _SaveBox::CreateNew( SwTable& rTbl, SwTableLine& rParent, _SaveTable& rSTbl )
{
    SwTableBoxFmt* pFmt = (SwTableBoxFmt*)rSTbl.aFrmFmts[ nItemSet ];
    if( !pFmt )
    {
        SwDoc* pDoc = rTbl.GetFrmFmt()->GetDoc();
        pFmt = pDoc->MakeTableBoxFmt();
        pFmt->SetAttr( *rSTbl.aSets[ nItemSet ] );
        rSTbl.aFrmFmts.Replace( pFmt, nItemSet );
    }

    if( ULONG_MAX == nSttNode )     // keine EndBox
    {
        SwTableBox* pNew = new SwTableBox( pFmt, 1, &rParent );
        rParent.GetTabBoxes().C40_INSERT( SwTableBox, pNew, rParent.GetTabBoxes().Count() );

        Ptrs.pLine->CreateNew( rTbl, *pNew, rSTbl );
    }
    else
    {
        // Box zum StartNode in der alten Tabelle suchen
        SwTableBox* pBox = rTbl.GetTblBox( nSttNode );
        ASSERT( pBox, "Wo ist meine TabellenBox geblieben?" );

        SwFrmFmt* pOld = pBox->GetFrmFmt();
        pFmt->Add( pBox );
        if( !pOld->GetDepends() )
            delete pOld;

        SwTableBoxes* pTBoxes = &pBox->GetUpper()->GetTabBoxes();
        pTBoxes->Remove( pTBoxes->C40_GETPOS( SwTableBox, pBox ) );

        pBox->SetUpper( &rParent );
        pTBoxes = &rParent.GetTabBoxes();
        pTBoxes->C40_INSERT( SwTableBox, pBox, pTBoxes->Count() );
    }

    if( pNext )
        pNext->CreateNew( rTbl, rParent, rSTbl );
}


/*  */

// UndoObject fuer Attribut Aenderung an der Tabelle


SwUndoAttrTbl::SwUndoAttrTbl( const SwTableNode& rTblNd, BOOL bClearTabCols )
    : SwUndo( UNDO_TABLE_ATTR ),
    nSttNode( rTblNd.GetIndex() )
{
    bClearTabCol = bClearTabCols;
    pSaveTbl = new _SaveTable( rTblNd.GetTable() );
}


SwUndoAttrTbl::~SwUndoAttrTbl()
{
    delete pSaveTbl;
}



void SwUndoAttrTbl::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    SwTableNode* pTblNd = rDoc.GetNodes()[ nSttNode ]->GetTableNode();
    ASSERT( pTblNd, "kein TabellenNode" );

    _SaveTable* pOrig = new _SaveTable( pTblNd->GetTable() );
    pSaveTbl->RestoreAttr( pTblNd->GetTable() );
    delete pSaveTbl;
    pSaveTbl = pOrig;

    if( bClearTabCol )
        ClearFEShellTabCols();
}


void SwUndoAttrTbl::Redo( SwUndoIter& rUndoIter )
{
    Undo( rUndoIter );
}


/*  */

// UndoObject fuer AutoFormat an der Tabelle


SwUndoTblAutoFmt::SwUndoTblAutoFmt( const SwTableNode& rTblNd,
                                    const SwTableAutoFmt& rAFmt )
    : SwUndo( UNDO_TABLE_AUTOFMT ),
    nSttNode( rTblNd.GetIndex() ),
    bSaveCntntAttr( FALSE ), pUndos( 0 )
{
    pSaveTbl = new _SaveTable( rTblNd.GetTable() );

    if( rAFmt.IsFont() || rAFmt.IsJustify() )
    {
        // dann auch noch ueber die ContentNodes der EndBoxen und
        // und alle Absatz-Attribute zusammen sammeln
        pSaveTbl->SaveCntntAttrs( (SwDoc*)rTblNd.GetDoc() );
        bSaveCntntAttr = TRUE;
    }
}


SwUndoTblAutoFmt::~SwUndoTblAutoFmt()
{
    delete pUndos;
    delete pSaveTbl;
}

void SwUndoTblAutoFmt::SaveBoxCntnt( const SwTableBox& rBox )
{
    SwUndoTblNumFmt* p = new SwUndoTblNumFmt( rBox );
    if( !pUndos )
        pUndos = new SwUndos( 8, 8 );
    pUndos->Insert( p, pUndos->Count() );
}


void SwUndoTblAutoFmt::UndoRedo( BOOL bUndo, SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    SwTableNode* pTblNd = rDoc.GetNodes()[ nSttNode ]->GetTableNode();
    ASSERT( pTblNd, "kein TabellenNode" );

    _SaveTable* pOrig = new _SaveTable( pTblNd->GetTable() );
        // dann auch noch ueber die ContentNodes der EndBoxen und
        // und alle Absatz-Attribute zusammen sammeln
    if( bSaveCntntAttr )
        pOrig->SaveCntntAttrs( &rDoc );

    if( pUndos && bUndo )
        for( USHORT n = pUndos->Count(); n; )
            pUndos->GetObject( --n )->Undo( rUndoIter );

    pSaveTbl->RestoreAttr( pTblNd->GetTable(), !bUndo );
    delete pSaveTbl;
    pSaveTbl = pOrig;
}

void SwUndoTblAutoFmt::Undo( SwUndoIter& rUndoIter )
{
    UndoRedo( TRUE, rUndoIter );
}


void SwUndoTblAutoFmt::Redo( SwUndoIter& rUndoIter )
{
    UndoRedo( FALSE, rUndoIter );
}


/*  */


SwUndoTblNdsChg::SwUndoTblNdsChg( USHORT nAction,
                                    const SwSelBoxes& rBoxes,
                                    const SwTableNode& rTblNd,
                                    USHORT nCnt, BOOL bFlg )
    : SwUndo( nAction ),
    nSttNode( rTblNd.GetIndex() ),
    aBoxes( rBoxes.Count() < 255 ? (BYTE)rBoxes.Count() : 255, 10 ),
    nCount( nCnt ), nRelDiff( 0 ), nAbsDiff( 0 ),
    nSetColType( USHRT_MAX ), nCurrBox( 0 ),
    bFlag( bFlg )
{
    Ptrs.pNewSttNds = 0;

    const SwTable& rTbl = rTblNd.GetTable();
    pSaveTbl = new _SaveTable( rTbl );

    // und die Selektion merken
    for( USHORT n = 0; n < rBoxes.Count(); ++n )
        aBoxes.Insert( rBoxes[n]->GetSttIdx(), n );
}


SwUndoTblNdsChg::SwUndoTblNdsChg( USHORT nAction,
                                    const SwSelBoxes& rBoxes,
                                    const SwTableNode& rTblNd )
    : SwUndo( nAction ),
    nSttNode( rTblNd.GetIndex() ),
    aBoxes( rBoxes.Count() < 255 ? (BYTE)rBoxes.Count() : 255, 10 ),
    nCount( 0 ), nRelDiff( 0 ), nAbsDiff( 0 ),
    nSetColType( USHRT_MAX ), nCurrBox( 0 ),
    bFlag( FALSE )
{
    Ptrs.pNewSttNds = 0;

    const SwTable& rTbl = rTblNd.GetTable();
    pSaveTbl = new _SaveTable( rTbl );

    // und die Selektion merken
    for( USHORT n = 0; n < rBoxes.Count(); ++n )
        aBoxes.Insert( rBoxes[n]->GetSttIdx(), n );
}


SwUndoTblNdsChg::~SwUndoTblNdsChg()
{
    delete pSaveTbl;

    if( UNDO_TABLE_DELBOX == GetId() )
        delete Ptrs.pDelSects;
    else
        delete Ptrs.pNewSttNds;
}



void SwUndoTblNdsChg::SaveNewBoxes( const SwTableNode& rTblNd,
                                    const SwTableSortBoxes& rOld )
{
    const SwTable& rTbl = rTblNd.GetTable();
    const SwTableSortBoxes& rTblBoxes = rTbl.GetTabSortBoxes();

    ASSERT( UNDO_TABLE_DELBOX != GetId(), "falsche Action" );
    Ptrs.pNewSttNds = new SvULongs( (BYTE)(rTblBoxes.Count() - rOld.Count()), 5 );

    for( USHORT n = 0, i = 0; n < rOld.Count(); ++i )
    {
        if( rOld[ n ] == rTblBoxes[ i ] )
            ++n;
        else
            // neue Box: sortiert einfuegen!!
            InsertSort( *Ptrs.pNewSttNds, rTblBoxes[ i ]->GetSttIdx() );
    }

    for( ; i < rTblBoxes.Count(); ++i )
        // neue Box: sortiert einfuegen!!
        InsertSort( *Ptrs.pNewSttNds, rTblBoxes[ i ]->GetSttIdx() );
}


void SwUndoTblNdsChg::SaveNewBoxes( const SwTableNode& rTblNd,
                                    const SwTableSortBoxes& rOld,
                                    const SwSelBoxes& rBoxes,
                                    const SvULongs& rNodeCnts )
{
    const SwTable& rTbl = rTblNd.GetTable();
    const SwTableSortBoxes& rTblBoxes = rTbl.GetTabSortBoxes();

    ASSERT( UNDO_TABLE_DELBOX != GetId(), "falsche Action" );
    Ptrs.pNewSttNds = new SvULongs( (BYTE)(rTblBoxes.Count() - rOld.Count()), 5 );

    for( USHORT n = 0, i = 0; n < rOld.Count(); ++i )
    {
        if( rOld[ n ] == rTblBoxes[ i ] )
            ++n;
        else
        {
            // neue Box: sortiert einfuegen!!
            USHORT nInsPos;
            const SwTableBox* pBox = rTblBoxes[ i ];
            InsertSort( *Ptrs.pNewSttNds, pBox->GetSttIdx(), &nInsPos );
            // feststellen, an welcher Position die Line in der Box steht
            const SwTableLine* pLn = pBox->GetUpper();
            USHORT nLinePos = pLn->GetUpper()->GetTabLines().GetPos( pLn );
            // 1. Box der 1. Line besorgen, ist die selektierte Box!
            pBox = pBox->GetUpper()->GetUpper()->GetTabLines()[ 0 ]
                                                        ->GetTabBoxes()[0];
            // stelle fest, wieviele Nodes die Box hatte und ob
            // die aktuelle davon einen bekommen hat
            USHORT nNdsPos = 0;
            while( rBoxes[ nNdsPos ] != pBox )
                ++nNdsPos;
            ULONG nNodes = rNodeCnts[ nNdsPos ];
            // wurden Nodes verschoben und hat die akt. Box davon einen
            // bekommen. Wenn ja, setze das Flag -> beim Undo darf der Inhalt
            // dann nicht geloscht sondern muss verschoben werden!
            BOOL bFlag = nNodes != pBox->GetSttNd()->EndOfSectionIndex() -
                                   pBox->GetSttIdx()
                && nNodes - 1 > nLinePos;
            aMvBoxes.Insert( bFlag, nInsPos );
        }
    }

    for( ; i < rTblBoxes.Count(); ++i )
    {
        // neue Box: sortiert einfuegen!!
        USHORT nInsPos;
        const SwTableBox* pBox = rTblBoxes[ i ];
        InsertSort( *Ptrs.pNewSttNds, pBox->GetSttIdx(), &nInsPos );
        // feststellen, an welcher Position die Line in der Box steht
        const SwTableLine* pLn = pBox->GetUpper();
        USHORT nLinePos = pLn->GetUpper()->GetTabLines().GetPos( pLn );
        // 1. Box der 1. Line besorgen, ist die selektierte Box!
        pBox = pBox->GetUpper()->GetUpper()->GetTabLines()[ 0 ]
                                                    ->GetTabBoxes()[0];
        // stelle fest, wieviele Nodes die Box hatte und ob
        // die aktuelle davon einen bekommen hat
        USHORT nNdsPos = 0;
        while( rBoxes[ nNdsPos ] != pBox )
            ++nNdsPos;
        ULONG nNodes = rNodeCnts[ nNdsPos ];
        // wurden Nodes verschoben und hat die akt. Box davon einen
        // bekommen. Wenn ja, setze das Flag -> beim Undo darf der Inhalt
        // dann nicht geloscht sondern muss verschoben werden!
        BOOL bFlag = nNodes != pBox->GetSttNd()->EndOfSectionIndex() -
                                pBox->GetSttIdx()
            && nNodes - 1 > nLinePos;
        aMvBoxes.Insert( bFlag, nInsPos );
    }
}


void SwUndoTblNdsChg::SaveSection( SwStartNode* pSttNd )
{
    ASSERT( UNDO_TABLE_DELBOX == GetId(), "falsche Action" );
    if( !Ptrs.pDelSects )
        Ptrs.pDelSects = new SwUndoSaveSections( 10, 5 );

    SwTableNode* pTblNd = pSttNd->FindTableNode();
    SwUndoSaveSection* pSave = new SwUndoSaveSection;
    pSave->SaveSection( pSttNd->GetDoc(), SwNodeIndex( *pSttNd ));

    Ptrs.pDelSects->Insert( pSave, Ptrs.pDelSects->Count() );
    nSttNode = pTblNd->GetIndex();
}


void SwUndoTblNdsChg::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    SwNodeIndex aIdx( rDoc.GetNodes(), nSttNode );

    SwTableNode* pTblNd = rDoc.GetNodes()[ aIdx ]->GetTableNode();
    ASSERT( pTblNd, "kein TabellenNode" );

    SwTableFmlUpdate aMsgHnt( &pTblNd->GetTable() );
    aMsgHnt.eFlags = TBL_BOXPTR;
    rDoc.UpdateTblFlds( &aMsgHnt );

    _FndBox aTmpBox( 0, 0 );
    aTmpBox.SaveChartData( pTblNd->GetTable() );

    if( UNDO_TABLE_DELBOX == GetId() )
    {
        // Trick: die fehlenden Boxen in irgendeine Line einfuegen, beim
        // CreateNew werden sie korrekt verbunden.
        SwTableBox* pCpyBox = pTblNd->GetTable().GetTabSortBoxes()[0];
        SwTableBoxes& rLnBoxes = pCpyBox->GetUpper()->GetTabBoxes();

        // die Sections wieder herstellen
        for( USHORT n = Ptrs.pDelSects->Count(); n; )
        {
            SwUndoSaveSection* pSave = (*Ptrs.pDelSects)[ --n ];
            pSave->RestoreSection( &rDoc, &aIdx, SwTableBoxStartNode );
            if( pSave->GetHistory() )
                pSave->GetHistory()->Rollback( &rDoc );
            SwTableBox* pBox = new SwTableBox( (SwTableBoxFmt*)pCpyBox->GetFrmFmt(), aIdx,
                                                pCpyBox->GetUpper() );
            rLnBoxes.C40_INSERT( SwTableBox, pBox, rLnBoxes.Count() );
        }
        Ptrs.pDelSects->DeleteAndDestroy( 0, Ptrs.pDelSects->Count() );
    }
    else if( aMvBoxes.Count() )
    {
        // dann muessen Nodes verschoben und nicht geloescht werden!
        // Dafuer brauchen wir aber ein temp Array
        SvULongs aTmp( 0, 5);
        aTmp.Insert( Ptrs.pNewSttNds, 0 );

        // von hinten anfangen
        for( USHORT n = aTmp.Count(); n; )
        {
            // Box aus der Tabellen-Struktur entfernen
            ULONG nIdx = aTmp[ --n ];
            SwTableBox* pBox = pTblNd->GetTable().GetTblBox( nIdx );
            ASSERT( pBox, "Wo ist meine TabellenBox geblieben?" );

            SwTableBoxes* pTBoxes = &pBox->GetUpper()->GetTabBoxes();
            pTBoxes->Remove( pTBoxes->C40_GETPOS( SwTableBox, pBox ) );

            if( aMvBoxes[ n ] )
            {
                SwNodeRange aRg( *pBox->GetSttNd(), 1,
                            *pBox->GetSttNd()->EndOfSectionNode() );
                SwNodeIndex aInsPos( *pBox->GetUpper()->GetUpper()->GetTabLines()[0]
                                    ->GetTabBoxes()[ 0 ]->GetSttNd(), 2 );

                // alle StartNode Indizies anpassen
                USHORT i = n;
                ULONG nSttIdx = aInsPos.GetIndex() - 2,
                       nNdCnt = aRg.aEnd.GetIndex() - aRg.aStart.GetIndex();
                while( i && aTmp[ --i ] > nSttIdx )
                    aTmp[ i ] += nNdCnt;

                // erst die Box loeschen
                delete pBox;
                // dann die Nodes verschieben,
                rDoc.GetNodes()._MoveNodes( aRg, rDoc.GetNodes(), aInsPos, FALSE );
            }
            else
            {
                delete pBox;
                rDoc.DeleteSection( rDoc.GetNodes()[ nIdx ] );
            }
        }
    }
    else
        // die Nodes loeschen (von Hinten!!)
        for( USHORT n = Ptrs.pNewSttNds->Count(); n; )
        {
            // Box aus der Tabellen-Struktur entfernen
            ULONG nIdx = (*Ptrs.pNewSttNds)[ --n ];
            SwTableBox* pBox = pTblNd->GetTable().GetTblBox( nIdx );
            ASSERT( pBox, "Wo ist meine TabellenBox geblieben?" );

            SwTableBoxes* pTBoxes = &pBox->GetUpper()->GetTabBoxes();
            pTBoxes->Remove( pTBoxes->C40_GETPOS( SwTableBox, pBox ) );

            delete pBox;
            rDoc.DeleteSection( rDoc.GetNodes()[ nIdx ] );
        }

    pSaveTbl->CreateNew( pTblNd->GetTable(), TRUE, FALSE );

    aTmpBox.RestoreChartData( pTblNd->GetTable() );

    if( UNDO_TABLE_DELBOX == GetId() )
        nSttNode = pTblNd->GetIndex();
    ClearFEShellTabCols();
}


void SwUndoTblNdsChg::Redo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();

    SwTableNode* pTblNd = rDoc.GetNodes()[ nSttNode ]->GetTableNode();
    ASSERT( pTblNd, "kein TabellenNode" );

    SwSelBoxes aSelBoxes;
    for( USHORT n = 0; n < aBoxes.Count(); ++n )
    {
        SwTableBox* pBox = pTblNd->GetTable().GetTblBox( aBoxes[ n ] );
        aSelBoxes.Insert( pBox );
    }

    // SelBoxes erzeugen und InsertCell/-Row/SplitTbl aufrufen
    switch( GetId() )
    {
    case UNDO_TABLE_INSCOL:
        if( USHRT_MAX == nSetColType )
            rDoc.InsertCol( aSelBoxes, nCount, bFlag );
        else
        {
            SwTableBox* pBox = pTblNd->GetTable().GetTblBox( nCurrBox );
            rDoc.SetColRowWidthHeight( *pBox, nSetColType, nAbsDiff,
                                        nRelDiff );
        }
        break;

    case UNDO_TABLE_INSROW:
        if( USHRT_MAX == nSetColType )
            rDoc.InsertRow( aSelBoxes, nCount, bFlag );
        else
        {
            SwTable& rTbl = pTblNd->GetTable();
            SwTableBox* pBox = rTbl.GetTblBox( nCurrBox );
            TblChgMode eOldMode = rTbl.GetTblChgMode();
            rTbl.SetTblChgMode( (TblChgMode)nCount );
            rDoc.SetColRowWidthHeight( *pBox, nSetColType, nAbsDiff, nRelDiff );
            rTbl.SetTblChgMode( eOldMode );
        }
        break;

    case UNDO_TABLE_SPLIT:
        rDoc.SplitTbl( aSelBoxes, bFlag, nCount );
        break;
    case UNDO_TABLE_DELBOX:
        if( USHRT_MAX == nSetColType )
        {
            SwTableFmlUpdate aMsgHnt( &pTblNd->GetTable() );
            aMsgHnt.eFlags = TBL_BOXPTR;
            rDoc.UpdateTblFlds( &aMsgHnt );

            pTblNd->GetTable().DeleteSel( &rDoc, aSelBoxes, this, TRUE );
        }
        else
        {
            SwTable& rTbl = pTblNd->GetTable();

            SwTableFmlUpdate aMsgHnt( &rTbl );
            aMsgHnt.eFlags = TBL_BOXPTR;
            rDoc.UpdateTblFlds( &aMsgHnt );

            SwTableBox* pBox = rTbl.GetTblBox( nCurrBox );
            TblChgMode eOldMode = rTbl.GetTblChgMode();
            rTbl.SetTblChgMode( (TblChgMode)nCount );

            rDoc.DoUndo( TRUE );        // wir brauchen die SaveSections!
            SwUndoTblNdsChg* pUndo = 0;

            switch( nSetColType & 0xff )
            {
            case WH_COL_LEFT:
            case WH_COL_RIGHT:
            case WH_CELL_LEFT:
            case WH_CELL_RIGHT:
                 rTbl.SetColWidth( *pBox, nSetColType, nAbsDiff,
                                    nRelDiff, (SwUndo**)&pUndo );
                break;
            case WH_ROW_TOP:
            case WH_ROW_BOTTOM:
            case WH_CELL_TOP:
            case WH_CELL_BOTTOM:
                rTbl.SetRowHeight( *pBox, nSetColType, nAbsDiff,
                                    nRelDiff, (SwUndo**)&pUndo );
                break;
            }

            if( pUndo )
            {
                Ptrs.pDelSects->Insert( pUndo->Ptrs.pDelSects, 0 );
                pUndo->Ptrs.pDelSects->Remove( 0, pUndo->Ptrs.pDelSects->Count() );

                delete pUndo;
            }
            rDoc.DoUndo( FALSE );

            rTbl.SetTblChgMode( eOldMode );
        }
        nSttNode = pTblNd->GetIndex();
        break;
    }
    ClearFEShellTabCols();
}


/*  */


SwUndoTblMerge::SwUndoTblMerge( const SwPaM& rTblSel )
    : SwUndo( UNDO_TABLE_MERGE ), SwUndRng( rTblSel ), pHistory( 0 )
{
    const SwTableNode* pTblNd = rTblSel.GetNode()->FindTableNode();
    ASSERT( pTblNd, "Wo ist TabllenNode" )
    pSaveTbl = new _SaveTable( pTblNd->GetTable() );
    pMoves = new SwUndoMoves;
    nTblNode = pTblNd->GetIndex();
}


SwUndoTblMerge::~SwUndoTblMerge()
{
    delete pSaveTbl;
    delete pMoves;
    delete pHistory;
}


void SwUndoTblMerge::Undo( SwUndoIter& rUndoIter )
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    SwNodeIndex aIdx( rDoc.GetNodes(), nTblNode );

    SwTableNode* pTblNd = rDoc.GetNodes()[ aIdx ]->GetTableNode();
    ASSERT( pTblNd, "kein TabellenNode" );

    SwTableFmlUpdate aMsgHnt( &pTblNd->GetTable() );
    aMsgHnt.eFlags = TBL_BOXPTR;
    rDoc.UpdateTblFlds( &aMsgHnt );

    _FndBox aTmpBox( 0, 0 );
    aTmpBox.SaveChartData( pTblNd->GetTable() );


    // 1. die geloeschten Boxen wiederherstellen:

    // Trick: die fehlenden Boxen in irgendeine Line einfuegen, beim
    // CreateNew werden sie korrekt verbunden.
    SwTableBox *pBox, *pCpyBox = pTblNd->GetTable().GetTabSortBoxes()[0];
    SwTableBoxes& rLnBoxes = pCpyBox->GetUpper()->GetTabBoxes();

DUMPDOC( &rDoc, "d:\\tmp\\tab_a.db" )
CHECKTABLE(pTblNd->GetTable())

    SwSelBoxes aSelBoxes;
    SwTxtFmtColl* pColl = rDoc.GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
    for( USHORT n = 0; n < aBoxes.Count(); ++n )
    {
        aIdx = aBoxes[ n ];
        SwStartNode* pSttNd = rDoc.GetNodes().MakeTextSection( aIdx,
                                            SwTableBoxStartNode, pColl );
        pBox = new SwTableBox( (SwTableBoxFmt*)pCpyBox->GetFrmFmt(), *pSttNd,
                                pCpyBox->GetUpper() );
        rLnBoxes.C40_INSERT( SwTableBox, pBox, rLnBoxes.Count() );

        aSelBoxes.Insert( pBox );
    }

DUMPDOC( &rDoc, "d:\\tmp\\tab_b.db" )
CHECKTABLE(pTblNd->GetTable())

    // 2. die eingefuegten Boxen loeschen
    // die Nodes loeschen (von Hinten!!)
    for( n = aNewSttNds.Count(); n; )
    {
        // Box aus der Tabellen-Struktur entfernen
        ULONG nIdx = aNewSttNds[ --n ];

        if( !nIdx && n )
        {
            nIdx = aNewSttNds[ --n ];
            pBox = pTblNd->GetTable().GetTblBox( nIdx );
            ASSERT( pBox, "Wo ist meine TabellenBox geblieben?" );

            rDoc.GetNodes().MakeTxtNode( SwNodeIndex(
                    *pBox->GetSttNd()->EndOfSectionNode() ), pColl );

            // das war der Trenner, -> die verschobenen herstellen
            for( USHORT i = pMoves->Count(); i; )
            {
                SwTxtNode* pTxtNd;
                USHORT nDelPos;
                SwUndoMove* pUndo = (*pMoves)[ --i ];
                if( !pUndo->IsMoveRange() )
                {
                    pTxtNd = rDoc.GetNodes()[ pUndo->GetDestSttNode() ]->GetTxtNode();
                    nDelPos = pUndo->GetDestSttCntnt() - 1;
                }
                pUndo->Undo( rUndoIter );
                if( pUndo->IsMoveRange() )
                {
                    // den ueberfluessigen Node loeschen
                    aIdx = pUndo->GetEndNode();
                    rDoc.GetNodes().Delete( aIdx, 1 );
                }
                else if( pTxtNd )
                {
                    // evt. noch ueberflussige Attribute loeschen
                    SwIndex aIdx( pTxtNd, nDelPos );
                    if( pTxtNd->GetpSwpHints() && pTxtNd->GetpSwpHints()->Count() )
                        pTxtNd->RstAttr( aIdx, pTxtNd->GetTxt().Len() -
                                                            nDelPos + 1 );
                    // das Trennzeichen loeschen
                    pTxtNd->Erase( aIdx, 1 );
                }
//              delete pUndo;
DUMPDOC( &rDoc, String( "d:\\tmp\\tab_") + String( aNewSttNds.Count() - i ) +
                String(".db") )
            }
//          pMoves->Remove( 0, pMoves->Count() );
            nIdx = pBox->GetSttIdx();
        }
        else
            pBox = pTblNd->GetTable().GetTblBox( nIdx );

        SwTableBoxes* pTBoxes = &pBox->GetUpper()->GetTabBoxes();
        pTBoxes->Remove( pTBoxes->C40_GETPOS( SwTableBox, pBox ) );


        // Indizies aus dem Bereich loeschen
        {
            SwNodeIndex aIdx( *pBox->GetSttNd() );
            rDoc.CorrAbs( SwNodeIndex( aIdx, 1 ),
                        SwNodeIndex( *aIdx.GetNode().EndOfSectionNode() ),
                        SwPosition( aIdx, SwIndex( 0, 0 )), TRUE );
        }

        delete pBox;
        rDoc.DeleteSection( rDoc.GetNodes()[ nIdx ] );
    }
DUMPDOC( &rDoc, "d:\\tmp\\tab_z.db" )
CHECKTABLE(pTblNd->GetTable())


    pSaveTbl->CreateNew( pTblNd->GetTable(), TRUE, FALSE );

    aTmpBox.RestoreChartData( pTblNd->GetTable() );

    if( pHistory )
    {
        pHistory->TmpRollback( &rDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }
//  nTblNode = pTblNd->GetIndex();

    SwPaM* pPam = rUndoIter.pAktPam;
    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nSttNode;
    pPam->GetPoint()->nContent.Assign( pPam->GetCntntNode(), nSttCntnt );
    pPam->SetMark();
    pPam->DeleteMark();

CHECKTABLE(pTblNd->GetTable())
    ClearFEShellTabCols();
}


void SwUndoTblMerge::Redo( SwUndoIter& rUndoIter )
{
    SwPaM* pPam = rUndoIter.pAktPam;
    SwDoc& rDoc = *pPam->GetDoc();

    SetPaM( *pPam );
    rDoc.MergeTbl( *pPam );
}


void SwUndoTblMerge::MoveBoxCntnt( SwPaM& rPam, SwPosition& rPos,
                                    const _SaveFlyArr& rArr )
{
    SwDoc* pDoc = rPam.GetDoc();
    SwNodeIndex aIdx( rPam.GetPoint()->nNode );
    SwNode* pNd = &aIdx.GetNode();

    // alle verschobenen Flys in der History vom Move-Object merken.
    // Hier erstmal zwischenspeichern, damit die alten NodeIdx richtig sind
    SwHistory aHst;
    for( USHORT n = 0; n < rArr.Count(); ++n )
        aHst.Add( *rArr[ n ].pFrmFmt );

    SwUndoMove* pUndoMove;
    if( pDoc->Move( rPam, rPos ) )
        pUndoMove = (SwUndoMove*)pDoc->RemoveLastUndo( UNDO_MOVE );
    else
    {
        // wir muessen das Undo Object haben!
        pUndoMove = new SwUndoMove( rPam, rPos );
        SwPaM aPam( rPos );
        pUndoMove->SetDestRange( aPam, rPos, FALSE, FALSE );
    }
    ASSERT( pUndoMove, "falsches Undo-Object" );

    // alle verschobenen Flys in der History vom Move-Object merken
    if( aHst.Count() )
        pUndoMove->AddTblMrgFlyHstry( aHst );

    pMoves->Insert( pUndoMove, pMoves->Count() );

    const SwStartNode* pBoxNd = pNd->FindTableBoxStartNode();
    ULONG nDelNds = pBoxNd->EndOfSectionIndex() - pBoxNd->GetIndex();
    if( 2 < nDelNds )
    {
        // es darf nur ein Textnode in der Box verbleiben!
        rPam.DeleteMark();

        // Indizies aus dem Bereich loeschen
        {
            pDoc->CorrAbs( SwNodeIndex( *pBoxNd ),
                            SwNodeIndex( *pBoxNd->EndOfSectionNode() ),
                            SwPosition( aIdx,
                                SwIndex( pNd->GetCntntNode(), 0 )), TRUE );
        }

//      rPam.GetBound1().nNode = rPam.GetBound2().nNode = aIdx;
        if( aIdx.GetIndex() - 1  != pBoxNd->GetIndex() )
        {
            SwNodeIndex aTmp( *pBoxNd, 1 );
            pDoc->GetNodes().Delete( aTmp, aIdx.GetIndex() - aTmp.GetIndex() );
        }
        aIdx++;
        pDoc->GetNodes().Delete( aIdx, pBoxNd->EndOfSectionIndex() - aIdx.GetIndex() );
    }
}


void SwUndoTblMerge::MoveBoxCntnt( SwDoc* pDoc, SwNodeRange& rRg, SwNodeIndex& rPos )
{
    SwNodeIndex aTmp( rRg.aStart, -1 ), aTmp2( rPos, -1 );
    SwUndoMove* pUndo = new SwUndoMove( pDoc, rRg, rPos );
    pDoc->Move( rRg, rPos );
    aTmp++;
    aTmp2++;
    pUndo->SetDestRange( aTmp2, rPos, aTmp );

    pMoves->Insert( pUndo, pMoves->Count() );
}


void SwUndoTblMerge::SetSelBoxes( const SwSelBoxes& rBoxes )
{
    // die Selektion merken
    for( USHORT n = 0; n < rBoxes.Count(); ++n )
        InsertSort( aBoxes, rBoxes[n]->GetSttIdx() );

    // als Trennung fuers einfuegen neuer Boxen nach dem Verschieben!
    aNewSttNds.Insert( (ULONG)0, aNewSttNds.Count() );

    nTblNode = rBoxes[ 0 ]->GetSttNd()->FindTableNode()->GetIndex();
}

void SwUndoTblMerge::SaveCollection( const SwTableBox& rBox )
{
    if( !pHistory )
        pHistory = new SwHistory;

    SwNodeIndex aIdx( *rBox.GetSttNd(), 1 );
    SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
    if( !pCNd )
        pCNd = aIdx.GetNodes().GoNext( &aIdx );

    pHistory->Add( pCNd->GetFmtColl(), aIdx.GetIndex(), pCNd->GetNodeType());
    if( pCNd->GetpSwAttrSet() )
        pHistory->CopyFmtAttr( *pCNd->GetpSwAttrSet(), aIdx.GetIndex() );
}

/*  */


SwUndoTblNumFmt::SwUndoTblNumFmt( const SwTableBox& rBox,
                                    const SfxItemSet* pNewSet )
    : SwUndo( UNDO_TBLNUMFMT ),
    pBoxSet( 0 ), pHistory( 0 ), nFmtIdx( NUMBERFORMAT_TEXT )
{
    bNewFmt = bNewFml = bNewValue = FALSE;
    nNode = rBox.GetSttIdx();

    ULONG nNdPos = rBox.IsValidNumTxtNd( 0 == pNewSet );

    if( ULONG_MAX != nNdPos )
    {
        SwDoc* pDoc = rBox.GetFrmFmt()->GetDoc();
        SwTxtNode* pTNd = pDoc->GetNodes()[ nNdPos ]->GetTxtNode();

        pHistory = new SwHistory;
        SwRegHistory aRHst( *rBox.GetSttNd(), pHistory );
        // immer alle TextAttribute sichern; ist fuers Undo mit voll-
        // staendiger Attributierung am besten, wegen den evt.
        // Ueberlappenden Bereichen von An/Aus.
        pHistory->CopyAttr( pTNd->GetpSwpHints(), nNdPos, 0,
                            pTNd->GetTxt().Len(), TRUE );

        if( pTNd->GetpSwAttrSet() )
            pHistory->CopyFmtAttr( *pTNd->GetpSwAttrSet(), nNdPos );

        aStr = pTNd->GetTxt();
        if( pTNd->GetpSwpHints() )
            pTNd->GetpSwpHints()->DeRegister();

        pBoxSet = new SfxItemSet( pDoc->GetAttrPool(), aTableBoxSetRange );
        pBoxSet->Put( rBox.GetFrmFmt()->GetAttrSet() );

        if( pNewSet )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pNewSet->GetItemState( RES_BOXATR_FORMAT,
                    FALSE, &pItem ))
            {
                bNewFmt = TRUE;
                nNewFmtIdx = ((SwTblBoxNumFormat*)pItem)->GetValue();
            }
            if( SFX_ITEM_SET == pNewSet->GetItemState( RES_BOXATR_FORMULA,
                    FALSE, &pItem ))
            {
                bNewFml = TRUE;
                aNewFml = ((SwTblBoxFormula*)pItem)->GetFormula();
            }
            if( SFX_ITEM_SET == pNewSet->GetItemState( RES_BOXATR_VALUE,
                    FALSE, &pItem ))
            {
                bNewValue = TRUE;
                fNewNum = ((SwTblBoxValue*)pItem)->GetValue();
            }
        }
    }

    // wird die History ueberhaupt benoetigt ??
    if( pHistory && !pHistory->Count() )
        DELETEZ( pHistory );
}


SwUndoTblNumFmt::~SwUndoTblNumFmt()
{
    delete pHistory;
    delete pBoxSet;
}

void SwUndoTblNumFmt::Undo( SwUndoIter& rIter )
{
    // konnte die Box veraendert werden ?
    if( !pBoxSet )
        return ;

    SwDoc& rDoc = rIter.GetDoc();
    SwStartNode* pSttNd = rDoc.GetNodes()[ nNode ]->
                            FindSttNodeByType( SwTableBoxStartNode );
    ASSERT( pSttNd, "ohne StartNode kein TabellenBox" );
    SwTableBox* pBox = pSttNd->FindTableNode()->GetTable().GetTblBox(
                                    pSttNd->GetIndex() );
    ASSERT( pBox, "keine TabellenBox gefunden" );

    SwTableBoxFmt* pFmt = rDoc.MakeTableBoxFmt();
    pFmt->SetAttr( *pBoxSet );
    pBox->ChgFrmFmt( pFmt );

    SwTxtNode* pTxtNd = rDoc.GetNodes()[ nNode + 1 ]->GetTxtNode();
    // wenn mehr als ein Node geloescht wurde, dann wurden auch
    // alle "Node"-Attribute gespeichert
    if( pTxtNd->GetpSwAttrSet() )
        pTxtNd->ResetAllAttr();

    if( pTxtNd->GetpSwpHints() )
        pTxtNd->ClearSwpHintsArr( FALSE );

    SwIndex aIdx( pTxtNd, 0 );
    pTxtNd->Erase( aIdx );
    if( aStr.Len() )
        pTxtNd->Insert( aStr, aIdx, INS_NOHINTEXPAND );

    if( pHistory )
    {
        USHORT nTmpEnd = pHistory->GetTmpEnd();
        pHistory->TmpRollback( &rDoc, 0 );
        pHistory->SetTmpEnd( nTmpEnd );
    }

    SwPaM* pPam = rIter.pAktPam;
    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nNode + 1;
    pPam->GetPoint()->nContent.Assign( pTxtNd, 0 );
}


void SwUndoTblNumFmt::Redo( SwUndoIter& rIter )
{
    // konnte die Box veraendert werden ?
    if( !pBoxSet )
        return ;

    SwDoc& rDoc = rIter.GetDoc();

    SwPaM* pPam = rIter.pAktPam;
    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nNode;

    SwNode* pNd = rDoc.GetNodes()[ pPam->GetPoint()->nNode ];
    SwStartNode* pSttNd = pNd->FindSttNodeByType( SwTableBoxStartNode );
    ASSERT( pSttNd, "ohne StartNode kein TabellenBox" );
    SwTableBox* pBox = pSttNd->FindTableNode()->GetTable().GetTblBox(
                                    pSttNd->GetIndex() );
    ASSERT( pBox, "keine TabellenBox gefunden" );

    SwFrmFmt* pBoxFmt = pBox->ClaimFrmFmt();
    if( bNewFmt || bNewFml || bNewValue )
    {
        SfxItemSet aBoxSet( rDoc.GetAttrPool(),
                                RES_BOXATR_FORMAT, RES_BOXATR_VALUE );

        // JP 15.01.99: Nur Attribute zuruecksetzen reicht nicht.
        //              Sorge dafuer, das der Text auch entsprechend
        //              formatiert wird!
        pBoxFmt->LockModify();

        if( bNewFml )
            aBoxSet.Put( SwTblBoxFormula( aNewFml ));
        else
            pBoxFmt->ResetAttr( RES_BOXATR_FORMULA );
        if( bNewFmt )
            aBoxSet.Put( SwTblBoxNumFormat( nNewFmtIdx ));
        else
            pBoxFmt->ResetAttr( RES_BOXATR_FORMAT );
        if( bNewValue )
            aBoxSet.Put( SwTblBoxValue( fNewNum ));
        else
            pBoxFmt->ResetAttr( RES_BOXATR_VALUE );
        pBoxFmt->UnlockModify();

        pBoxFmt->SetAttr( aBoxSet );
    }
    else if( NUMBERFORMAT_TEXT != nFmtIdx )
    {
        SfxItemSet aBoxSet( rDoc.GetAttrPool(),
                            RES_BOXATR_FORMAT, RES_BOXATR_VALUE );

        aBoxSet.Put( SwTblBoxNumFormat( nFmtIdx ));
        aBoxSet.Put( SwTblBoxValue( fNum ));

        // JP 15.01.99: Nur Attribute zuruecksetzen reicht nicht.
        //              Sorge dafuer, das der Text auch entsprechend
        //              formatiert wird!
        pBoxFmt->LockModify();
        pBoxFmt->ResetAttr( RES_BOXATR_FORMULA );
        pBoxFmt->UnlockModify();

        pBoxFmt->SetAttr( aBoxSet );
    }
    else
    {
        // es ist keine Zahl

        // JP 15.01.99: Nur Attribute zuruecksetzen reicht nicht.
        //              Sorge dafuer, das der Text auch entsprechend
        //              formatiert wird!
        pBoxFmt->SetAttr( *GetDfltAttr( RES_BOXATR_FORMAT ));

        pBoxFmt->ResetAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
    }

    if( bNewFml )
    {
        // egal was gesetzt wurde, ein Update der Tabelle macht sich immer gut
        SwTableFmlUpdate aTblUpdate( &pSttNd->FindTableNode()->GetTable() );
        rDoc.UpdateTblFlds( &aTblUpdate );
    }

    if( !pNd->IsCntntNode() )
        pNd = rDoc.GetNodes().GoNext( &pPam->GetPoint()->nNode );
    pPam->GetPoint()->nContent.Assign( (SwCntntNode*)pNd, 0 );
}

void SwUndoTblNumFmt::SetBox( const SwTableBox& rBox )
{
    nNode = rBox.GetSttIdx();
}

/*  */

_UndoTblCpyTbl_Entry::_UndoTblCpyTbl_Entry( const SwTableBox& rBox )
    : nBoxIdx( rBox.GetSttIdx() ), nOffset( 0 ),
    pBoxNumAttr( 0 ), pUndo( 0 )
{
}

_UndoTblCpyTbl_Entry::~_UndoTblCpyTbl_Entry()
{
    delete pUndo;
    delete pBoxNumAttr;
}


SwUndoTblCpyTbl::SwUndoTblCpyTbl()
    : SwUndo( UNDO_TBLCPYTBL ), pInsRowUndo( 0 )
{
    pArr = new _UndoTblCpyTbl_Entries;
}

SwUndoTblCpyTbl::~SwUndoTblCpyTbl()
{
    delete pArr;
    delete pInsRowUndo;
}

void SwUndoTblCpyTbl::Undo( SwUndoIter& rIter )
{
    SwDoc& rDoc = rIter.GetDoc();

    SwTableNode* pTblNd = 0;
    for( USHORT n = pArr->Count(); n; )
    {
        _UndoTblCpyTbl_Entry* pEntry = (*pArr)[ --n ];
        ULONG nSttPos = pEntry->nBoxIdx + pEntry->nOffset;
        SwStartNode* pSNd = rDoc.GetNodes()[ nSttPos ]->FindStartNode();
        if( !pTblNd )
            pTblNd = pSNd->FindTableNode();

        SwTableBox& rBox = *pTblNd->GetTable().GetTblBox( nSttPos );

        SwNodeIndex aInsIdx( *rBox.GetSttNd(), 1 );
        SwTxtNode* pNd = rDoc.GetNodes().MakeTxtNode( aInsIdx,
                                (SwTxtFmtColl*)rDoc.GetDfltTxtFmtColl());
        SwPaM aPam( aInsIdx.GetNode(), *rBox.GetSttNd()->EndOfSectionNode() );
        SwUndoDelete* pUndo = new SwUndoDelete( aPam, TRUE );

        if( pEntry->pUndo )
        {
            pEntry->pUndo->Undo( rIter );
            delete pEntry->pUndo;
        }
        pEntry->pUndo = pUndo;

        aInsIdx = rBox.GetSttIdx() + 1;
        rDoc.GetNodes().Delete( aInsIdx, 1 );

        SfxItemSet aTmpSet( rDoc.GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_VALUE,
                                                RES_VERT_ORIENT, RES_VERT_ORIENT, 0 );
        aTmpSet.Put( rBox.GetFrmFmt()->GetAttrSet() );
        if( aTmpSet.Count() )
        {
            SwFrmFmt* pBoxFmt = rBox.ClaimFrmFmt();
            pBoxFmt->ResetAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
            pBoxFmt->ResetAttr( RES_VERT_ORIENT );
        }

        if( pEntry->pBoxNumAttr )
        {
            rBox.ClaimFrmFmt()->SetAttr( *pEntry->pBoxNumAttr );
            delete pEntry->pBoxNumAttr, pEntry->pBoxNumAttr = 0;
        }

        if( aTmpSet.Count() )
        {
            pEntry->pBoxNumAttr = new SfxItemSet( rDoc.GetAttrPool(),
                                    RES_BOXATR_FORMAT, RES_BOXATR_VALUE,
                                    RES_VERT_ORIENT, RES_VERT_ORIENT, 0 );
            pEntry->pBoxNumAttr->Put( aTmpSet );
        }

        pEntry->nOffset = rBox.GetSttIdx() - pEntry->nBoxIdx;
    }

    if( pInsRowUndo )
        pInsRowUndo->Undo( rIter );
}

void SwUndoTblCpyTbl::Redo( SwUndoIter& rIter )
{
    SwDoc& rDoc = rIter.GetDoc();

    if( pInsRowUndo )
        pInsRowUndo->Redo( rIter );

    SwTableNode* pTblNd = 0;
    for( USHORT n = 0; n < pArr->Count(); ++n )
    {
        _UndoTblCpyTbl_Entry* pEntry = (*pArr)[ n ];
        ULONG nSttPos = pEntry->nBoxIdx + pEntry->nOffset;
        SwStartNode* pSNd = rDoc.GetNodes()[ nSttPos ]->FindStartNode();
        if( !pTblNd )
            pTblNd = pSNd->FindTableNode();

        SwTableBox& rBox = *pTblNd->GetTable().GetTblBox( nSttPos );

        SwNodeIndex aInsIdx( *rBox.GetSttNd(), 1 );
        SwTxtNode* pNd = rDoc.GetNodes().MakeTxtNode( aInsIdx,
                                (SwTxtFmtColl*)rDoc.GetDfltTxtFmtColl());
        SwPaM aPam( aInsIdx.GetNode(), *rBox.GetSttNd()->EndOfSectionNode());
        SwUndoDelete* pUndo = new SwUndoDelete( aPam, TRUE );

        if( pEntry->pUndo )
        {
            pEntry->pUndo->Undo( rIter );
            delete pEntry->pUndo;
        }
        pEntry->pUndo = pUndo;

        aInsIdx = rBox.GetSttIdx() + 1;
        rDoc.GetNodes().Delete( aInsIdx, 1 );

        SfxItemSet aTmpSet( rDoc.GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_VALUE,
                                                RES_VERT_ORIENT, RES_VERT_ORIENT, 0 );
        aTmpSet.Put( rBox.GetFrmFmt()->GetAttrSet() );
        if( aTmpSet.Count() )
        {
            SwFrmFmt* pBoxFmt = rBox.ClaimFrmFmt();
            pBoxFmt->ResetAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
            pBoxFmt->ResetAttr( RES_VERT_ORIENT );
        }
        if( pEntry->pBoxNumAttr )
        {
            rBox.ClaimFrmFmt()->SetAttr( *pEntry->pBoxNumAttr );
            delete pEntry->pBoxNumAttr, pEntry->pBoxNumAttr = 0;
        }

        if( aTmpSet.Count() )
        {
            pEntry->pBoxNumAttr = new SfxItemSet( rDoc.GetAttrPool(),
                                    RES_BOXATR_FORMAT, RES_BOXATR_VALUE,
                                    RES_VERT_ORIENT, RES_VERT_ORIENT, 0 );
            pEntry->pBoxNumAttr->Put( aTmpSet );
        }

        pEntry->nOffset = rBox.GetSttIdx() - pEntry->nBoxIdx;
    }
}

void SwUndoTblCpyTbl::AddBoxBefore( const SwTableBox& rBox, BOOL bDelCntnt )
{
    if( pArr->Count() && !bDelCntnt )
        return;

    _UndoTblCpyTbl_Entry* pEntry = new _UndoTblCpyTbl_Entry( rBox );
    pArr->Insert( pEntry, pArr->Count() );

    SwDoc* pDoc = rBox.GetFrmFmt()->GetDoc();
    if( bDelCntnt )
    {
        SwNodeIndex aInsIdx( *rBox.GetSttNd(), 1 );
        SwTxtNode* pNd = pDoc->GetNodes().MakeTxtNode( aInsIdx,
                                (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl());
        SwPaM aPam( aInsIdx.GetNode(), *rBox.GetSttNd()->EndOfSectionNode() );
        pEntry->pUndo = new SwUndoDelete( aPam, TRUE );

    }

    pEntry->pBoxNumAttr = new SfxItemSet( pDoc->GetAttrPool(),
                                    RES_BOXATR_FORMAT, RES_BOXATR_VALUE,
                                    RES_VERT_ORIENT, RES_VERT_ORIENT, 0 );
    pEntry->pBoxNumAttr->Put( rBox.GetFrmFmt()->GetAttrSet() );
    if( !pEntry->pBoxNumAttr->Count() )
        delete pEntry->pBoxNumAttr, pEntry->pBoxNumAttr = 0;
}

void SwUndoTblCpyTbl::AddBoxAfter( const SwTableBox& rBox, BOOL bDelCntnt )
{
    _UndoTblCpyTbl_Entry* pEntry = (*pArr)[ pArr->Count() - 1 ];

    // wurde der Inhalt geloescht, so loesche jetzt auch noch den temp.
    // erzeugten Node
    if( bDelCntnt && pEntry->pUndo )
    {
        SwNodeIndex aDelIdx( *rBox.GetSttNd(), 1 );
        rBox.GetFrmFmt()->GetDoc()->GetNodes().Delete( aDelIdx, 1 );
    }
    pEntry->nOffset = rBox.GetSttIdx() - pEntry->nBoxIdx;
}

BOOL SwUndoTblCpyTbl::InsertRow( SwTable& rTbl, const SwSelBoxes& rBoxes,
                                USHORT nCnt )
{
    SwTableNode* pTblNd = (SwTableNode*)rTbl.GetTabSortBoxes()[0]->
                                GetSttNd()->FindTableNode();

    SwTableSortBoxes aTmpLst( 0, 5 );
    pInsRowUndo = new SwUndoTblNdsChg( UNDO_TABLE_INSROW, rBoxes, *pTblNd,
                                        nCnt, TRUE );
    aTmpLst.Insert( &rTbl.GetTabSortBoxes(), 0, rTbl.GetTabSortBoxes().Count() );

    BOOL bRet = rTbl.InsertRow( rTbl.GetFrmFmt()->GetDoc(), rBoxes, nCnt, TRUE );
    if( bRet )
        pInsRowUndo->SaveNewBoxes( *pTblNd, aTmpLst );
    else
        delete pInsRowUndo, pInsRowUndo = 0;
    return bRet;
}

BOOL SwUndoTblCpyTbl::IsEmpty() const
{
    return !pInsRowUndo && !pArr->Count();
}

/*  */

SwUndoCpyTbl::SwUndoCpyTbl()
    : SwUndo( UNDO_CPYTBL ), pDel( 0 ), nTblNode( 0 )
{
}

SwUndoCpyTbl::~SwUndoCpyTbl()
{
    delete pDel;
}

void SwUndoCpyTbl::Undo( SwUndoIter& rIter )
{
    SwDoc& rDoc = rIter.GetDoc();
    SwTableNode* pTNd = rDoc.GetNodes()[ nTblNode ]->GetTableNode();

    // harte SeitenUmbrueche am nachfolgenden Node verschieben
    SwCntntNode* pNextNd = rDoc.GetNodes()[ pTNd->EndOfSectionIndex()+1 ]->GetCntntNode();
    if( pNextNd )
    {
        SwFrmFmt* pTableFmt = pTNd->GetTable().GetFrmFmt();
        const SfxPoolItem *pItem;

        if( SFX_ITEM_SET == pTableFmt->GetItemState( RES_PAGEDESC,
            FALSE, &pItem ) )
            pNextNd->SetAttr( *pItem );

        if( SFX_ITEM_SET == pTableFmt->GetItemState( RES_BREAK,
            FALSE, &pItem ) )
            pNextNd->SetAttr( *pItem );
    }

    SwPaM aPam( *pTNd, *pTNd->EndOfSectionNode(), 0 , 1 );
    pDel = new SwUndoDelete( aPam, TRUE );
}

void SwUndoCpyTbl::Redo( SwUndoIter& rIter )
{
    pDel->Undo( rIter );
    delete pDel, pDel = 0;
}


/*  */

SwUndoSplitTbl::SwUndoSplitTbl( const SwTableNode& rTblNd, USHORT eMode,
                                BOOL bNewSize )
    : SwUndo( UNDO_SPLIT_TABLE ), pSavTbl( 0 ),
    nTblNode( rTblNd.GetIndex() ), nMode( eMode ), nOffset( 0 ), nFmlEnd( 0 ),
    pHistory( 0 ), bCalcNewSize( bNewSize )
{
    switch( nMode )
    {
    case HEADLINE_BOXATRCOLLCOPY:
            pHistory = new SwHistory;
            // kein break;
    case HEADLINE_BORDERCOPY:
    case HEADLINE_BOXATTRCOPY:
        pSavTbl = new _SaveTable( rTblNd.GetTable(), 1, FALSE );
        break;
    }
}

SwUndoSplitTbl::~SwUndoSplitTbl()
{
    delete pSavTbl;
    delete pHistory;
}

void SwUndoSplitTbl::Undo( SwUndoIter& rIter )
{
    SwPaM* pPam = rIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();

    pPam->DeleteMark();
    SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
    rIdx = nTblNode + nOffset;

    //Den implizit erzeugten Absatz wieder entfernen.
    pDoc->GetNodes().Delete( rIdx, 1 );

    rIdx = nTblNode + nOffset;
    SwTableNode* pTblNd = rIdx.GetNode().GetTableNode();
    SwTable& rTbl = pTblNd->GetTable();

    SwTableFmlUpdate aMsgHnt( &rTbl );
    aMsgHnt.eFlags = TBL_BOXPTR;
    pDoc->UpdateTblFlds( &aMsgHnt );

    switch( nMode )
    {
    case HEADLINE_BOXATRCOLLCOPY:
        if( pHistory )
            pHistory->TmpRollback( pDoc, nFmlEnd );

        // kein break
    case HEADLINE_BOXATTRCOPY:
    case HEADLINE_BORDERCOPY:
        {
            pSavTbl->CreateNew( rTbl, FALSE );
            pSavTbl->RestoreAttr( rTbl );
        }
        break;

    case HEADLINE_CNTNTCOPY:
        // die erzeugte 1. Line muss wieder entfernt werden
        {
            SwSelBoxes aSelBoxes;
            SwTableBox* pBox = rTbl.GetTblBox( nTblNode + nOffset + 1 );
            rTbl.SelLineFromBox( pBox, aSelBoxes, TRUE );
            rTbl.DeleteSel( pDoc, aSelBoxes, 0, FALSE, FALSE );
        }
        break;
    }

    pDoc->GetNodes().MergeTable( rIdx );

    if( pHistory )
    {
        pHistory->TmpRollback( pDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }

    ClearFEShellTabCols();
}

void SwUndoSplitTbl::Redo( SwUndoIter& rIter )
{
    SwPaM* pPam = rIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();

    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nTblNode;
    pDoc->SplitTable( *pPam->GetPoint(), nMode, bCalcNewSize );

    ClearFEShellTabCols();
}

void SwUndoSplitTbl::Repeat( SwUndoIter& rIter )
{
    SwPaM* pPam = rIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();

    pDoc->SplitTable( *pPam->GetPoint(), nMode, bCalcNewSize );
    ClearFEShellTabCols();
}

void SwUndoSplitTbl::SaveFormula( SwHistory& rHistory )
{
    if( !pHistory )
        pHistory = new SwHistory;

    nFmlEnd = rHistory.Count();
    pHistory->Move( 0, &rHistory );
}

/*  */

SwUndoMergeTbl::SwUndoMergeTbl( const SwTableNode& rTblNd,
                                const SwTableNode& rDelTblNd,
                                BOOL bWithPrv, USHORT nMd )
    : SwUndo( UNDO_MERGE_TABLE ), pSavTbl( 0 ),
    pHistory( 0 ), bWithPrev( bWithPrv ), nMode( nMd )
{
    // Endnode der letzen Tabellenzelle merken, die auf der Position verbleibt
    if( bWithPrev )
        nTblNode = rDelTblNd.EndOfSectionIndex() - 1;
    else
        nTblNode = rTblNd.EndOfSectionIndex() - 1;

    aName = rDelTblNd.GetTable().GetFrmFmt()->GetName();
    pSavTbl = new _SaveTable( rDelTblNd.GetTable() );

    pSavHdl = bWithPrev ? new _SaveTable( rTblNd.GetTable(), 1 ) : 0;
}

SwUndoMergeTbl::~SwUndoMergeTbl()
{
    delete pSavTbl;
    delete pSavHdl;
    delete pHistory;
}

void SwUndoMergeTbl::Undo( SwUndoIter& rIter )
{
    SwPaM* pPam = rIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();

    pPam->DeleteMark();
    SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
    rIdx = nTblNode;

    SwTableNode* pTblNd = rIdx.GetNode().FindTableNode();
    SwTable* pTbl = &pTblNd->GetTable();

    SwTableFmlUpdate aMsgHnt( pTbl );
    aMsgHnt.eFlags = TBL_BOXPTR;
    pDoc->UpdateTblFlds( &aMsgHnt );

    //Lines fuer das Layout-Update herausuchen.
    _FndBox aFndBox( 0, 0 );
    aFndBox.SetTableLines( *pTbl );
    aFndBox.DelFrms( *pTbl );
    aFndBox.SaveChartData( *pTbl );

    SwTableNode* pNew = pDoc->GetNodes().SplitTable( rIdx, TRUE, FALSE );

    //Layout updaten
    aFndBox.MakeFrms( *pTbl );
    aFndBox.RestoreChartData( *pTbl );

    if( bWithPrev )
    {
        // den Namen umsetzen
        pNew->GetTable().GetFrmFmt()->SetName( pTbl->GetFrmFmt()->GetName() );
        pSavHdl->RestoreAttr( pNew->GetTable() );
    }
    else
        pTbl = &pNew->GetTable();
    pTbl->GetFrmFmt()->SetName( aName );

//  pSavTbl->CreateNew( *pTbl, FALSE );
    pSavTbl->RestoreAttr( *pTbl );


    if( pHistory )
    {
        pHistory->TmpRollback( pDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }

    // fuer die neue Tabelle die Frames anlegen
    SwNodeIndex aTmpIdx( *pNew );
    pNew->MakeFrms( &aTmpIdx );

    // Cursor  irgendwo in den Content stellen
    SwCntntNode* pCNd = pDoc->GetNodes().GoNext( &rIdx );
    pPam->GetPoint()->nContent.Assign( pCNd, 0 );

    ClearFEShellTabCols();
}

void SwUndoMergeTbl::Redo( SwUndoIter& rIter )
{
    SwPaM* pPam = rIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();

    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nTblNode;
    if( bWithPrev )
        pPam->GetPoint()->nNode = nTblNode + 3;
    else
        pPam->GetPoint()->nNode = nTblNode;

    pDoc->MergeTable( *pPam->GetPoint(), bWithPrev, nMode );

    ClearFEShellTabCols();
}

void SwUndoMergeTbl::Repeat( SwUndoIter& rIter )
{
    SwPaM* pPam = rIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();

    pDoc->MergeTable( *pPam->GetPoint(), bWithPrev, nMode );
    ClearFEShellTabCols();
}

void SwUndoMergeTbl::SaveFormula( SwHistory& rHistory )
{
    if( !pHistory )
        pHistory = new SwHistory;
    pHistory->Move( 0, &rHistory );
}

/*  */


void InsertSort( SvUShorts& rArr, USHORT nIdx, USHORT* pInsPos )
{
    register USHORT nO  = rArr.Count(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            if( *(rArr.GetData() + nM) == nIdx )
            {
                ASSERT( FALSE, "Index ist schon vorhanden, darf nie sein!" );
                return;
            }
            if( *(rArr.GetData() + nM) < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
                break;
            else
                nO = nM - 1;
        }
    }
    rArr.Insert( nIdx, nU );
    if( pInsPos )
        *pInsPos = nU;
}

void InsertSort( SvULongs& rArr, ULONG nIdx, USHORT* pInsPos )
{
    register USHORT nO  = rArr.Count(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            if( *(rArr.GetData() + nM) == nIdx )
            {
                ASSERT( FALSE, "Index ist schon vorhanden, darf nie sein!" );
                return;
            }
            if( *(rArr.GetData() + nM) < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
                break;
            else
                nO = nM - 1;
        }
    }
    rArr.Insert( nIdx, nU );
    if( pInsPos )
        *pInsPos = nU;
}

#if defined( JP_DEBUG ) && !defined( PRODUCT )


void DumpDoc( SwDoc* pDoc, const String& rFileNm )
{
    Writer* pWrt = SwIoSystem::GetWriter( "DEBUG" );
    if( pWrt )
    {
        SvFileStream aStream( rFileNm, STREAM_STD_WRITE );
        SwPaM* pPam = new SwPaM( pDoc, SwPosition( pDoc->GetNodes().EndOfContent ,
                                                 pDoc->GetNodes().EndOfContent ));
        pPam->Move( fnMoveBackward, fnGoDoc );
        pPam->SetMark();
        pPam->Move( fnMoveForward, fnGoDoc );

        pWrt->Write( pPam, *pDoc, aStream, rFileNm.GetStr() );

        delete pPam;
    }
}
void CheckTable( const SwTable& rTbl )
{
    const SwNodes& rNds = rTbl.GetFrmFmt()->GetDoc()->GetNodes();
    const SwTableSortBoxes& rSrtArr = pTblNd->GetTable().GetTabSortBoxes();
    for( USHORT n = 0; n < rSrtArr.Count(); ++n )
    {
        const SwTableBox* pBox = rSrtArr[ n ];
        const SwNode* pNd = pBox->GetSttNd();
        ASSERT( rNds[ *pBox->GetSttIdx() ] == pNd, "Box mit falchem StartNode"  );
    }
}
#endif

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/untbl.cxx,v 1.1.1.1 2000-09-19 00:08:28 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.117  2000/09/18 16:04:30  willem.vandorp
      OpenOffice header added.

      Revision 1.116  2000/07/11 17:51:56  jp
      Bug #76739#: SwUndoTblToTxt - save the HeadlineRepeat Flag

      Revision 1.115  2000/05/09 10:04:51  jp
      Changes for Unicode

      Revision 1.114  2000/04/10 11:24:56  jp
      Bug #74371#: SwUndoTblMerge:SaveColl - save hard format attributes too

      Revision 1.113  1999/12/15 12:12:20  jp
      Bug #70847#: SwUndoSplitTable - dont store formulas

      Revision 1.112  1999/11/10 12:10:16  jp
      Bug #66833#: save/restore fly by table to text

      Revision 1.111  1999/10/25 12:44:43  jp
      Bug #66833#: Text<->Table - set flys to the right position

      Revision 1.110  1999/07/08 17:18:44  JP
      Bug #67507#: ClearSwpHintsArr - optional dont delete fields/hardblanks/softhyps


      Rev 1.109   08 Jul 1999 19:18:44   JP
   Bug #67507#: ClearSwpHintsArr - optional dont delete fields/hardblanks/softhyps

      Rev 1.108   30 Apr 1999 15:12:12   AMA
   Fix #65184#: Undo von Tabelle<->Text

      Rev 1.107   28 Apr 1999 22:43:46   JP
   Bug #65491#: MoveBoxCntnt: InsPos gesondert merken, kann bei Bereichen verschoben werden

      Rev 1.106   29 Mar 1999 16:28:36   JP
   Bug #62847#: Parameter vom DeleteBox haben sich geaendert

      Rev 1.105   23 Mar 1999 18:26:14   JP
   Bug #63449#: SwUndoTblNumFmt:Redo - ggfs. die Tabellenformel entfernen

      Rev 1.104   26 Feb 1999 15:35:56   JP
   Bug #62500#: Tabellenmerge - bei Prev die Kopfzeile wieder restaurieren

      Rev 1.103   25 Feb 1999 10:53:22   AMA
   Fix #62257#: "Intelligenter" ClientIterator ueber Nodes

      Rev 1.102   17 Feb 1999 21:10:22   JP
   Task #61764#: neu: Tabellen zusammenfassen

      Rev 1.101   17 Feb 1999 16:59:12   JP
   Task #61764#: neue Undo Action fuers Tabellen zusammenfassen

      Rev 1.100   11 Feb 1999 23:43:34   JP
   Bug #55590#: Schnittstelle geaendert

      Rev 1.99   26 Jan 1999 16:00:06   JP
   Task #57749#: spaltige Bereiche - Save-/RestoreTblUpperFrms bekommen jetzt einen Node

      Rev 1.98   15 Jan 1999 11:28:46   JP
   Bug #60794#: Fehlererkennung beim Tabellenrechnen

      Rev 1.97   30 Nov 1998 17:29:30   OM
   #59770# Tabellenoptionen: Ueberschrift nur auf erster Seite

      Rev 1.96   30 Nov 1998 16:16:58   JP
   Bug #59720#: Tabellenoptionen auswerten

      Rev 1.95   10 Nov 1998 19:51:52   JP
   Task #58158#: InsertTable/TextToTable - Flags fuer HdlRepeat/LayoutSplit mitgeben

      Rev 1.94   30 Jul 1998 17:03:24   JP
   Bug #54295#: Formeln immer nur im Klartext sichern

      Rev 1.93   16 Jul 1998 19:19:30   JP
   Bug #52968#: Autoformat - NumFormat kann BoxInhalt veraendern, muss wieder herstellbar sein

      Rev 1.92   30 Jun 1998 15:11:14   JP
   Bug #51844#: SaveChart kann nur erfolgen, wenn die Tabelle mit den entsp. Daten vorliegt

      Rev 1.91   15 Jun 1998 20:03:34   JP
   TextToTable: Undo - vertikal Tab als Tab einfuegen

      Rev 1.90   12 May 1998 23:39:30   JP
   neu: InserTable/TextToTable mit optionalen AutoFormat, TextToTable: ggfs. DefBorder setzen

      Rev 1.89   04 May 1998 15:23:50   JP
   TblMerge: Redo wurde noch einfacher

      Rev 1.88   29 Apr 1998 20:06:00   JP
   SwDoc::MergeTbl: neue Schnittstelle

      Rev 1.87   21 Apr 1998 17:12:28   JP
   Bug #49288# SwUndoSplitTbl - Tabellenformeln sichern

      Rev 1.86   20 Apr 1998 19:12:28   JP
   Bug #49519#: bei Tabellenformeln nie den Wert mit speichern

      Rev 1.85   02 Apr 1998 16:02:00   JP
   Bug #49145#: UndoTblMerge::Undo - Cursor korrigieren vorm loeschen der Box

      Rev 1.84   02 Apr 1998 15:14:14   JP
   Redo: Undo-Flag wird schon von der EditShell abgeschaltet

      Rev 1.83   27 Mar 1998 15:34:14   JP
   UndoTblMerge: SaveCollection fuer leere Zellen einer Line

      Rev 1.82   18 Mar 1998 21:12:38   JP
   Bug #48744#: SplitTable um einen Modus erweitert

      Rev 1.81   16 Mar 1998 23:17:24   JP
   UndoSplitTable: um den Modus erweitert, CopyTable dafuer erweitert

      Rev 1.80   25 Feb 1998 20:56:12   JP
   UndoSplitTable: auch noch den erzeugten TextNode entfernen

      Rev 1.79   25 Feb 1998 12:54:16   MA
   new: SplitTable

      Rev 1.78   16 Feb 1998 22:02:18   JP
   neu: SplitTable - Tabelle an der vorgebenen Grundline aufsplitten (mit Undo)

      Rev 1.77   06 Feb 1998 18:07:58   JP
   UndoTblNum: beim Restaurieren einer Formel die Tabelle neu berechnen

      Rev 1.76   29 Jan 1998 20:13:36   JP
   UndoTblNumFmt: NodeOffset wird nicht benoetigt

      Rev 1.75   28 Jan 1998 19:51:18   JP
   CheckBoxNumFmt: BoxSttNode fuer Undo kann sich bei Redline noch veraendern

      Rev 1.74   27 Jan 1998 22:36:14   JP
   GetNumDepend durch GetDepends ersetzt

      Rev 1.73   27 Jan 1998 15:26:34   JP
   TableLine-/-BoxFormate direkt loeschen

      Rev 1.72   22 Jan 1998 20:53:12   JP
   CTOR des SwPaM umgestellt

      Rev 1.71   08 Jan 1998 20:56:02   JP
   SwDoc::GetRedlineTbl returnt jetzt eine Referenz

      Rev 1.70   19 Dec 1997 12:14:22   JP
   Undo: Redlining beachten

      Rev 1.69   20 Nov 1997 18:26:46   MA
   includes

      Rev 1.68   17 Nov 1997 09:46:24   JP
   Umstellung Numerierung

      Rev 1.67   30 Oct 1997 14:36:04   AMA
   Chg: Kein AutoFlag mehr an Break bzw. PageDesc-Attributen

      Rev 1.66   09 Oct 1997 15:45:50   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.65   12 Sep 1997 10:49:34   OS
   ITEMID_* definiert

      Rev 1.64   10 Sep 1997 10:43:42   JP
   neu: Undo fuers kopieren von Tabellen & in Tabellen

      Rev 1.63   03 Sep 1997 10:29:48   JP
   zusaetzliches include von docary

      Rev 1.62   15 Aug 1997 12:38:28   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.61   12 Aug 1997 12:36:44   OS
   Header-Umstellung

*************************************************************************/


