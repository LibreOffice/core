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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif


#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HDL
#include <com/sun/star/i18n/WordType.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_CHARTYPE_HDL
#include <com/sun/star/i18n/CharType.hdl>
#endif



#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>			// ...Percent()
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>			// ResId fuer Statusleiste
#endif
namespace binfilter {

using namespace ::com::sun::star::i18n;

static const USHORT coSrchRplcThreshold = 60000;

struct _PercentHdl
{
    SwDocShell* pDSh;
    ULONG nActPos;
    BOOL bBack, bNodeIdx;

    _PercentHdl( ULONG nStt, ULONG nEnd, SwDocShell* pSh )
        : pDSh( pSh )
    {
        nActPos = nStt;
        if( 0 != ( bBack = (nStt > nEnd )) )
        {
            ULONG n = nStt; nStt = nEnd; nEnd = n;
        }
        ::binfilter::StartProgress( STR_STATSTR_SEARCH, nStt, nEnd, 0 );
    }

    _PercentHdl( const SwPaM& rPam )
        : pDSh( (SwDocShell*)rPam.GetDoc()->GetDocShell() )
    {
        ULONG nStt, nEnd;
        if( rPam.GetPoint()->nNode == rPam.GetMark()->nNode )
        {
            bNodeIdx = FALSE;
            nStt = rPam.GetMark()->nContent.GetIndex();
            nEnd = rPam.GetPoint()->nContent.GetIndex();
        }
        else
        {
            nStt = rPam.GetMark()->nNode.GetIndex();
            nEnd = rPam.GetPoint()->nNode.GetIndex();
        }
        nActPos = nStt;
        if( 0 != ( bBack = (nStt > nEnd )) )
        {
            ULONG n = nStt; nStt = nEnd; nEnd = n;
        }
        ::binfilter::StartProgress( STR_STATSTR_SEARCH, nStt, nEnd, pDSh );
    }

    ~_PercentHdl()  					{ ::binfilter::EndProgress( pDSh ); }

    void NextPos( ULONG nPos ) const
    { ::binfilter::SetProgressState( bBack ? nActPos - nPos : nPos, pDSh ); }

    void NextPos( SwPosition& rPos ) const
        {
            ULONG nPos;
            if( bNodeIdx )
                nPos = rPos.nNode.GetIndex();
            else
                nPos = rPos.nContent.GetIndex();
            ::binfilter::SetProgressState( bBack ? nActPos - nPos : nPos, pDSh );
        }
};

/*N*/ SwCursor::SwCursor( const SwPosition &rPos, SwPaM* pRing )
/*N*/ 	    : SwPaM( rPos, pRing ), pSavePos( 0 ), nCursorBidiLevel( 0 )
/*N*/ {
/*N*/ }


/*N*/ SwCursor::~SwCursor()
/*N*/ {
/*N*/ 	while( pSavePos )
/*N*/ 	{
/*?*/ 		_SwCursor_SavePos* pNext = pSavePos->pNext;
/*?*/ 		delete pSavePos;
/*?*/ 		pSavePos = pNext;
/*N*/ 	}
/*N*/ }


/*N*/ SwCursor::operator SwTableCursor* ()		{ return 0; }
/*N*/ SwCursor::operator SwShellCrsr* ()			{ return 0; }
/*N*/ SwCursor::operator SwShellTableCrsr* ()		{ return 0; }
/*N*/ SwCursor::operator SwUnoCrsr* ()			{ return 0; }
/*N*/ SwCursor::operator SwUnoTableCrsr* ()		{ return 0; }


// Sicher die aktuelle Position, damit ggfs. auf diese zurueck
// gefallen werden kann. Die SavePos Objekte werden als Stack verwaltet,
// damit das auch alles bei verschachtelten Aufrufen funktioniert.
// Das CreateNewSavePos ist virtual, damit abgeleitete Klassen vom Cursor
// gegebenenfalls eigene SaveObjecte anlegen und in den virtuellen
// Check-Routinen verwenden koennen.

/*N*/ void SwCursor::SaveState()
/*N*/ {
/*N*/ 	_SwCursor_SavePos* pNew = CreateNewSavePos();
/*N*/ 	pNew->pNext = pSavePos;
/*N*/ 	pSavePos = pNew;
/*N*/ }

/*N*/ void SwCursor::RestoreState()
/*N*/ {
/*N*/ 	_SwCursor_SavePos* pDel = pSavePos;
/*N*/ 	pSavePos = pSavePos->pNext;
/*N*/ 	delete pDel;
/*N*/ }

/*N*/ _SwCursor_SavePos* SwCursor::CreateNewSavePos() const
/*N*/ {
/*N*/ 	return new _SwCursor_SavePos( *this );
/*N*/ }

// stelle fest, ob sich der Point ausserhalb des Content-Bereichs
// vom Nodes-Array befindet


/*N*/ FASTBOOL SwCursor::IsSelOvr( int eFlags )
/*N*/ {
/*N*/ 	SwTableCursor* pTblCrsr = *this;
/*N*/ 	SwDoc* pDoc = GetDoc();
/*N*/ 	SwNodes& rNds = pDoc->GetNodes();
/*N*/ 
/*N*/ 	BOOL bSkipOverHiddenSections, bSkipOverProtectSections;
/*N*/ 	SwUnoCrsr* pUnoCrsr = *this;
/*N*/ 	if( pUnoCrsr )
/*N*/ 	{
/*N*/ 		bSkipOverHiddenSections = pUnoCrsr->IsSkipOverHiddenSections();
/*N*/ 		bSkipOverProtectSections = pUnoCrsr->IsSkipOverProtectSections();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		bSkipOverHiddenSections = TRUE;
/*?*/ 		bSkipOverProtectSections = !IsReadOnlyAvailable();
/*N*/ 	}

    // Bereiche vom Nodes-Array ueberpruefen
/*N*/ 	if( (SELOVER_CHECKNODESSECTION & eFlags) && pTblCrsr && HasMark() )
/*N*/ 	{
/*?*/ 		SwNodeIndex aOldPos( rNds, pSavePos->nNode );
/*?*/ 		if( !CheckNodesRange( aOldPos, GetPoint()->nNode, TRUE ))
/*?*/ 		{
/*?*/ 			GetPoint()->nNode = aOldPos;
/*?*/ 			GetPoint()->nContent.Assign( GetCntntNode(), pSavePos->nCntnt );
/*?*/ 			return TRUE;
/*?*/ 		}
/*N*/ 	}

// neu: Bereiche ueberpruefen
// Anfang
/*N*/ 	if( pSavePos->nNode != GetPoint()->nNode.GetIndex() &&
/*N*/ 		//JP 28.10.97: Bug 45129 - im UI-ReadOnly ist alles erlaubt
/*N*/ 		( !pDoc->GetDocShell() || !pDoc->GetDocShell()->IsReadOnlyUI() ))
/*N*/ 	{
/*N*/ 		// teste doch mal die neuen Sections:
/*N*/ 		SwNodeIndex& rPtIdx = GetPoint()->nNode;
/*N*/ 		const SwSectionNode* pSectNd = rPtIdx.GetNode().FindSectionNode();
/*N*/ 		if( pSectNd &&
/*N*/ 			((bSkipOverHiddenSections && pSectNd->GetSection().IsHiddenFlag() ) ||
/*N*/ 			 (bSkipOverProtectSections && pSectNd->GetSection().IsProtectFlag() )))
/*N*/ 		{
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 		if( 0 == ( SELOVER_CHANGEPOS & eFlags ) )
/*N*/ 		}

        // oder sollte eine geschuetzte Section innerhalb der Selektion liegen?
/*N*/ 		if( HasMark() && bSkipOverProtectSections)
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ULONG nSttIdx = GetMark()->nNode.GetIndex(),
/*N*/ 		}
/*?*/ 
/*N*/ 	}
// Ende
// neu: Bereiche ueberpruefen

/*N*/ 	const SwNode* pNd = &GetPoint()->nNode.GetNode();
/*N*/ 	if( pNd->IsCntntNode() && 0 == (SwUnoCrsr*)*this )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 const SwCntntFrm* pFrm = ((SwCntntNode*)pNd)->GetFrm();
/*N*/ 	}

    // darf der Cursor in geschuetzen "Nodes" stehen?
/*N*/ 	if( 0 == ( SELOVER_CHANGEPOS & eFlags ) && !IsAtValidPos() )
/*N*/ 	{
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	DeleteMark();
/*N*/ 	}

/*N*/ 	if( !HasMark() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	//JP 19.08.98: teste mal auf ungueltige Selektion - sprich ueber
/*N*/ 	// 				GrundSections:
/*N*/ 	if( !::binfilter::CheckNodesRange( GetMark()->nNode, GetPoint()->nNode, TRUE ))
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 DeleteMark();
/*N*/ 	}

/*N*/ 	const SwTableNode* pPtNd = pNd->FindTableNode();

/*N*/ 	if( (pNd = &GetMark()->nNode.GetNode())->IsCntntNode() &&
/*N*/ 		!((SwCntntNode*)pNd)->GetFrm() && 0 == (SwUnoCrsr*)*this )
/*N*/ 	{
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	DeleteMark();
/*N*/ 	}

/*N*/ 	const SwTableNode* pMrkNd = pNd->FindTableNode();
/*N*/ 
/*N*/ 	// beide in keinem oder beide im gleichen TableNode
/*N*/ 	if( ( !pMrkNd && !pPtNd ) || pPtNd == pMrkNd )
/*N*/ 		return FALSE;

    // in unterschiedlichen Tabellen oder nur Mark in der Tabelle
/*?*/ 	if( ( pPtNd && pMrkNd ) || pMrkNd )
/*?*/ 	{						// dann lasse das nicht zu, alte Pos zurueck
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	RestoreSavePos();
/*?*/ 	}

    // ACHTUNG: dieses kann nicht im TableMode geschehen !!
/*?*/ 	if( pPtNd )		// nur Point in Tabelle, dann gehe hinter/vor diese
/*?*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( SELOVER_CHANGEPOS & eFlags )
/*?*/ 	}
/*?*/ 	return FALSE;		// was bleibt noch ??
/*N*/ }

#if defined( UNX ) || defined( MAC )
#define IDX     (*pCellStt)
#else
#define IDX     aCellStt
#endif


/*N*/ FASTBOOL SwCursor::IsInProtectTable( FASTBOOL bMove, FASTBOOL bChgCrsr )
/*N*/ {
/*N*/ 	// stehe ich in einer Tabelle ??
/*N*/ 	SwDoc* pDoc = GetDoc();
/*N*/ 	SwCntntNode* pCNd = GetCntntNode();
/*N*/ 	if( !pCNd || pSavePos->nNode == GetPoint()->nNode.GetIndex() ||
/*N*/ 		!pCNd->FindTableNode() ||
/*N*/ 		!pCNd->IsProtect() ||
/*N*/ 		IsReadOnlyAvailable() )
/*N*/ 		return FALSE;

/*?*/ 	if( !bMove )
/*?*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( bChgCrsr )
/*?*/ 	}
/*?*/ 
/*?*/ 	// wir stehen in einer geschuetzten TabellenZelle
/*?*/ 	// von Oben nach Unten Traveln ?
/*?*/ 	if( pSavePos->nNode < GetPoint()->nNode.GetIndex() )
/*?*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 // suche die naechste "gueltige" Box
/*?*/ 
/*?*/ 	}
/*?*/ 
/*?*/ 	// suche die vorherige "gueltige" Box
/*?*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 // liegt vor dem StartNode der Zelle ein weiterer EndNode, dann
/*?*/ 		// gibt es auch eine vorherige Zelle
/*?*/ 	}
/*?*/ 
/*?*/ 	ASSERT( FALSE, "sollte nie erreicht werden oder??" );
/*?*/ 	return	FALSE;
/*N*/ }

// TRUE: an die Position kann der Cursor gesetzt werden
/*N*/ FASTBOOL SwCursor::IsAtValidPos( BOOL bPoint ) const
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); return FALSE; //STRIP001 	const SwDoc* pDoc = GetDoc();
/*N*/ }

/*N*/ void SwCursor::SaveTblBoxCntnt( const SwPosition* )	{}

// setze den SRange fuer das Suchen im Dokument








// diese Methode "sucht" fuer alle Anwendungsfaelle, denn in SwFindParas
// steht immer die richtigen Parameter und die entsprechende Find-Methode

/*M*/ ULONG SwCursor::FindAll( SwFindParas& rParas,
/*M*/ 							SwDocPositions nStart, SwDocPositions nEnde,
/*M*/                             FindRanges eFndRngs, BOOL& bCancel )
/*M*/ {DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 
/*M*/ }




/*N*/ FASTBOOL SwCursor::IsStartWord() const
/*N*/ {
/*N*/  	FASTBOOL bRet = FALSE;DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/  	return bRet;
/*N*/ }

/*N*/ FASTBOOL SwCursor::IsEndWord() const
/*N*/ {
     FASTBOOL bRet = FALSE;DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ return bRet;
/*N*/ }


/*N*/ FASTBOOL SwCursor::GoStartWord()
/*N*/ {
/*N*/ 	FASTBOOL bRet = FALSE;DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return bRet;
/*N*/ }

/*N*/ FASTBOOL SwCursor::GoEndWord()
/*N*/ {
/*N*/  	FASTBOOL bRet = FALSE;DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return bRet;
/*N*/ }

/*N*/ FASTBOOL SwCursor::GoNextWord()
/*N*/ {
/*?*/ 	FASTBOOL bRet = FALSE;DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return bRet;
/*N*/ }

/*N*/ FASTBOOL SwCursor::GoPrevWord()
/*N*/ {
/*?*/ 	FASTBOOL bRet = FALSE;DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return bRet;
/*N*/ }


//-----------------------------------------------------------------------------
/*N*/ FASTBOOL SwCursor::GoSentence( SentenceMoveType eMoveType )
/*N*/ {
/*?*/ 	FASTBOOL bRet = FALSE;DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*N*/ }


/*N*/ FASTBOOL SwCursor::LeftRight( BOOL bLeft, USHORT nCnt, USHORT nMode,
/*N*/                               BOOL bVisualAllowed, BOOL bInsertCrsr )
/*N*/ {
/*N*/ 	SwTableCursor* pTblCrsr = (SwTableCursor*)*this;
/*N*/ 	if( pTblCrsr )
/*N*/ 		return bLeft ? pTblCrsr->GoPrevCell( nCnt )
/*N*/ 					 : pTblCrsr->GoNextCell( nCnt );
/*N*/ 
/*N*/     // calculate cursor bidi level
/*N*/     const SwCntntFrm* pSttFrm = NULL;
/*N*/     SwNode& rNode = GetPoint()->nNode.GetNode();
/*N*/     const BOOL bDoNotSetBidiLevel = 0 != (SwUnoCrsr*)*this;
/*N*/ 
/*N*/     if ( ! bDoNotSetBidiLevel )
/*N*/     {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/     }
/*N*/ 
/*N*/ 	// kann der Cursor n-mal weiterverschoben werden ?
/*N*/     SwCrsrSaveState aSave( *this );
/*N*/     SwMoveFn fnMove = bLeft ? fnMoveBackward : fnMoveForward;
/*N*/     SwGoInDoc fnGo = CRSR_SKIP_CELLS == nMode ? fnGoCntntCells : fnGoCntnt;
/*N*/     while( nCnt && Move( fnMove, fnGo ) )
/*N*/         --nCnt;
/*N*/ 
/*N*/     // here come some special rules for visual cursor travelling
/*N*/     if ( pSttFrm )
/*N*/     {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/     }
/*N*/ 
/*N*/ 	return 0 == nCnt && !IsInProtectTable( TRUE ) &&
/*N*/ 			!IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS );
/*N*/ }

/*N*/ FASTBOOL SwCursor::UpDown( BOOL bUp, USHORT nCnt,
/*N*/ 							Point* pPt, long nUpDownX )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 SwTableCursor* pTblCrsr = (SwTableCursor*)*this;
/*N*/ }

/*N*/ FASTBOOL SwCursor::GotoTblBox( const String& rName )
/*N*/ {
/*N*/ 	FASTBOOL bRet = FALSE;DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*N*/ }

/*N*/ FASTBOOL SwCursor::MovePara(SwWhichPara fnWhichPara, SwPosPara fnPosPara )
/*N*/ {
/*N*/ 	//JP 28.8.2001: for optimization test something before
/*N*/ 	const SwNode* pNd;
/*N*/ 	if( fnWhichPara == fnParaCurr ||
/*N*/ 		(( pNd = &GetPoint()->nNode.GetNode())->IsTxtNode() &&
/*N*/ 		  pNd->GetNodes()[ pNd->GetIndex() +
/*N*/ 					(fnWhichPara == fnParaNext ? 1 : -1 ) ]->IsTxtNode() ) )
/*N*/ 	{
/*N*/ 		return (*fnWhichPara)( *this, fnPosPara );
/*N*/ 	}
/*N*/ 	// else we must use the SaveStructure, because the next/prev is not
/*N*/ 	// a same node type.
/*N*/ 	SwCrsrSaveState aSave( *this );
/*N*/ 	return (*fnWhichPara)( *this, fnPosPara ) &&
/*N*/ 			!IsInProtectTable( TRUE ) &&
/*N*/ 			!IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS );
/*N*/ }


/*N*/ FASTBOOL SwCursor::MoveSection( SwWhichSection fnWhichSect,
/*N*/ 								SwPosSection fnPosSect)
/*N*/ {
/*N*/ 	SwCrsrSaveState aSave( *this );
/*N*/ 	return (*fnWhichSect)( *this, fnPosSect ) &&
/*N*/ 			!IsInProtectTable( TRUE ) &&
/*N*/ 			!IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS );
/*N*/ }

/*N*/ SwTableCursor::~SwTableCursor() {}

/*N*/ SwTableCursor::operator SwTableCursor* () { return this; }

}
