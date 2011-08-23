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

#ifdef WTC
#define private public
#endif

#include <ctype.h>
#include <float.h>

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>  	// fuer SwAttrSetChg
#endif

#ifndef _SVXLINKMGR_HXX
#include <bf_svx/linkmgr.hxx>
#endif


#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FRAME_HXX
#include <frame.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _CELLFRM_HXX
#include <cellfrm.hxx>
#endif
#ifndef _ROWFRM_HXX
#include <rowfrm.hxx>
#endif
#ifndef _SWSERV_HXX
#include <swserv.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _HTMLTBL_HXX
#include <htmltbl.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif
#ifndef _TBLRWCL_HXX
#include <tblrwcl.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
namespace binfilter {

/*N*/ TYPEINIT1( SwTable, SwClient );
/*N*/ TYPEINIT1( SwTableBox, SwClient );
/*N*/ TYPEINIT1( SwTableLine, SwClient );
/*N*/ TYPEINIT1( SwTableFmt, SwFrmFmt );
/*N*/ TYPEINIT1( SwTableBoxFmt, SwFrmFmt );
/*N*/ TYPEINIT1( SwTableLineFmt, SwFrmFmt );

/*N*/ SV_IMPL_PTRARR(SwTableLines,SwTableLine*);
/*N*/ SV_IMPL_PTRARR(SwTableBoxes,SwTableBox*);
/*N*/ SV_IMPL_PTRARR_SORT(SwTableSortBoxes,SwTableBoxPtr);

/*N*/ SV_IMPL_REF( SwServerObject )

#define COLFUZZY 20

//----------------------------------

/*N*/ class SwTableBox_Impl
/*N*/ {
/*N*/ 	Color *pUserColor, *pNumFmtColor;
/*N*/
/*N*/ 	void SetNewCol( Color** ppCol, const Color* pNewCol );
/*N*/ public:
/*N*/ 	SwTableBox_Impl() : pUserColor(0), pNumFmtColor(0) {}
/*N*/ 	~SwTableBox_Impl() { delete pUserColor; delete pNumFmtColor; }
/*N*/
/*N*/ 	const Color* GetSaveUserColor()	const		{ return pUserColor; }
/*N*/ 	const Color* GetSaveNumFmtColor() const 	{ return pNumFmtColor; }
/*N*/ 	void SetSaveUserColor(const Color* p )		{ SetNewCol( &pUserColor, p ); }
/*N*/ 	void SetSaveNumFmtColor( const Color* p )	{ SetNewCol( &pNumFmtColor, p ); }
/*N*/ };

// ----------- Inlines -----------------------------





//JP 15.09.98: Bug 55741 - Tabs beibehalten (vorne und hinten)



/*************************************************************************
|*
|*	SwTable::SwTable()
|*
|*	Ersterstellung		MA 09. Mar. 93
|*	Letzte Aenderung	MA 05. May. 93
|*
|*************************************************************************/
/*N*/ SwTable::SwTable( SwTableFmt* pFmt )
/*N*/ 	: SwClient( pFmt ),
/*N*/ 	pHTMLLayout( 0 ),
/*N*/ 	nGrfsThatResize( 0 )
/*N*/ {
/*N*/ 	bModifyLocked = FALSE;
/*N*/ 	bHeadlineRepeat = TRUE;
/*N*/
/*N*/ 	// default Wert aus den Optionen setzen
/*N*/   eTblChgMode = (TblChgMode)0;//STRIP001 GetTblChgDefaultMode();
/*N*/ }

/*N*/ SwTable::SwTable( const SwTable& rTable )
/*N*/ 	: SwClient( rTable.GetFrmFmt() ),
/*N*/ 	pHTMLLayout( 0 ),
/*N*/ 	nGrfsThatResize( 0 )
/*N*/ {
/*N*/ 	bHeadlineRepeat = rTable.IsHeadlineRepeat();
/*N*/ 	bModifyLocked = FALSE;
/*N*/ 	eTblChgMode = rTable.eTblChgMode;
/*N*/ }

/*N*/ void DelBoxNode( SwTableSortBoxes& rSortCntBoxes )
/*N*/ {
/*N*/ 	for( USHORT n = 0; n < rSortCntBoxes.Count(); ++n )
/*N*/ 		rSortCntBoxes[ n ]->pSttNd = 0;
/*N*/ }

/*N*/ SwTable::~SwTable()
/*N*/ {
/*N*/ 	if( refObj.Is() )
/*N*/ 	{
/*?*/ 		SwDoc* pDoc = GetFrmFmt()->GetDoc();
/*?*/ 		if( !pDoc->IsInDtor() )			// dann aus der Liste entfernen
/*?*/ 			pDoc->GetLinkManager().RemoveServer( &refObj );
/*?*/
/*?*/ 		refObj->Closed();
/*N*/ 	}
/*N*/
/*N*/ 	// ist die Tabelle der letzte Client im FrameFormat, kann dieses
/*N*/ 	// geloescht werden
/*N*/ 	SwTableFmt* pFmt = (SwTableFmt*)GetFrmFmt();
/*N*/ 	pFmt->Remove( this );				// austragen,
/*N*/
/*N*/ 	if( !pFmt->GetDepends() )
/*N*/ 		pFmt->GetDoc()->DelTblFrmFmt( pFmt );	// und loeschen
/*N*/
/*N*/ 	// Loesche die Pointer aus dem SortArray der Boxen, die
/*N*/ 	// Objecte bleiben erhalten und werden vom DTOR der Lines/Boxes
/*N*/ 	// Arrays geloescht.
/*N*/ 	//JP: reicht leider nicht, es muessen die Pointer auf den StartNode
/*N*/ 	//	der Section geloescht werden
/*N*/ 	DelBoxNode( aSortCntBoxes );
/*N*/ 	aSortCntBoxes.Remove( (USHORT)0, aSortCntBoxes.Count() );
/*N*/ 	delete pHTMLLayout;
/*N*/ }

/*************************************************************************
|*
|*	SwTable::Modify()
|*
|*	Ersterstellung		JP ??
|*	Letzte Aenderung	MA 06. May. 93
|*
|*************************************************************************/
/*?*/ inline BOOL FmtInArr( SvPtrarr& rFmtArr, SwFmt* pBoxFmt )
/*?*/ {
/*?*/ 	BOOL bRet = USHRT_MAX != rFmtArr.GetPos( (VoidPtr)pBoxFmt );
/*?*/ 	if( !bRet )
/*?*/ 		rFmtArr.Insert( (VoidPtr)pBoxFmt, rFmtArr.Count() );
/*?*/ 	return bRet;
/*?*/ }

/*N*/ void lcl_ModifyBoxes( SwTableBoxes &rBoxes, const long nOld,
/*N*/ 						 const long nNew, SvPtrarr& rFmtArr );
/*N*/
/*N*/ void lcl_ModifyLines( SwTableLines &rLines, const long nOld,
/*N*/ 						 const long nNew, SvPtrarr& rFmtArr )
/*N*/ {
/*N*/ 	for ( USHORT i = 0; i < rLines.Count(); ++i )
/*N*/ 		::binfilter::lcl_ModifyBoxes( rLines[i]->GetTabBoxes(), nOld, nNew, rFmtArr );
/*N*/ }

/*N*/ void lcl_ModifyBoxes( SwTableBoxes &rBoxes, const long nOld,
/*N*/ 						 const long nNew, SvPtrarr& rFmtArr )
/*N*/ {
/*N*/ 	for ( USHORT i = 0; i < rBoxes.Count(); ++i )
/*N*/ 	{
/*N*/ 		SwTableBox &rBox = *rBoxes[i];
/*N*/ 		if ( rBox.GetTabLines().Count() )
/*?*/ 			::binfilter::lcl_ModifyLines( rBox.GetTabLines(), nOld, nNew, rFmtArr );
/*N*/ 		//Die Box anpassen
/*N*/ 		SwFrmFmt *pFmt = rBox.GetFrmFmt();
/*N*/ 		if ( !FmtInArr( rFmtArr, pFmt ) )
/*N*/ 		{
/*N*/ 			long nBox = pFmt->GetFrmSize().GetWidth();
/*N*/ 			nBox *= nNew;
/*N*/ 			nBox /= nOld;
/*N*/ 			SwFmtFrmSize aNewBox( ATT_VAR_SIZE, nBox, 0 );
/*N*/ 			pFmt->LockModify();
/*N*/ 			pFmt->SetAttr( aNewBox );
/*N*/ 			pFmt->UnlockModify();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SwTable::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
/*N*/ {
/*N*/ 	// fange SSize Aenderungen ab, um die Lines/Boxen anzupassen
/*N*/ 	USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
/*N*/ 	const SwFmtFrmSize* pNewSize = 0, *pOldSize = 0;
/*N*/
/*N*/ 	if( RES_ATTRSET_CHG == nWhich )
/*N*/ 	{
/*N*/ 		if( SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState(
/*N*/ 			RES_FRM_SIZE, FALSE, (const SfxPoolItem**)&pNewSize ))
/*N*/ 			pOldSize = &((SwAttrSetChg*)pOld)->GetChgSet()->GetFrmSize();
/*N*/ 	}
/*N*/ 	else if( RES_FRM_SIZE == nWhich )
/*N*/ 	{
/*?*/ 		pOldSize = (const SwFmtFrmSize*)pOld;
/*?*/ 		pNewSize = (const SwFmtFrmSize*)pNew;
/*N*/ 	}

/*N*/ 	if( pOldSize || pNewSize )
/*N*/ 	{
/*N*/ 		if ( !IsModifyLocked() )
/*N*/ 		{
/*N*/ 			ASSERT( pOldSize && pOldSize->Which() == RES_FRM_SIZE &&
/*N*/ 					pNewSize && pNewSize->Which() == RES_FRM_SIZE,
/*N*/ 					"Kein Old oder New fuer FmtFrmSize-Modify der SwTable." );
/*N*/
/*N*/ 			// Array zum Vergleichen der Box-Formatpointer; um zu verhindern,
/*N*/ 			// das das gleiche Attribut immer wieder im selben Format
/*N*/ 			// gesetzt wird. (+1 fuer das Ende-Kennzeichen)
/*N*/ 			SvPtrarr aFmtArr( (BYTE)aLines[0]->GetTabBoxes().Count(), 1 );
/*N*/
/*N*/ 			::binfilter::lcl_ModifyLines( aLines, pOldSize->GetWidth(), pNewSize->GetWidth(),
/*N*/ 						   aFmtArr );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		SwClient::Modify( pOld, pNew );			// fuers ObjectDying
/*N*/ }

/*************************************************************************
|*
|*	SwTable::GetTabCols()
|*
|*	Ersterstellung		MA 04. May. 93
|*	Letzte Aenderung	MA 30. Nov. 95
|*
|*************************************************************************/




// MS: Sonst Absturz auf der DEC-Kiste
//
#if defined(ALPHA) && defined(_MSC_VER)
#pragma optimize("", off)
#endif


/*N*/ void SwTable::GetTabCols( SwTabCols &rToFill, const SwTableBox *pStart,
/*N*/ 						  FASTBOOL bRefreshHidden, BOOL bCurRowOnly ) const
/*N*/ {
/*N*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //MA 30. Nov. 95: Opt: wenn bHidden gesetzt ist, wird neu das Hidden
/*N*/ }

#if defined(ALPHA) && defined(_MSC_VER)
#pragma optimize("", on)
#endif

/*************************************************************************
|*
|*	SwTable::SetTabCols()
|*
|*	Ersterstellung		MA 04. May. 93
|*	Letzte Aenderung	MA 26. Aug. 98
|*
|*************************************************************************/
//Struktur zur Parameteruebergabe









/*************************************************************************
|*
|*	const SwTableBox* SwTable::GetTblBox( const Strn¡ng& rName ) const
|*		gebe den Pointer auf die benannte Box zurueck.
|*
|*	Ersterstellung		JP 30. Jun. 93
|*	Letzte Aenderung	JP 30. Jun. 93
|*
|*************************************************************************/
/*N*/ USHORT SwTable::_GetBoxNum( String& rStr, BOOL bFirst )
/*N*/ {
/*N*/ 	USHORT nRet = 0;
/*N*/ 	xub_StrLen nPos = 0;
/*N*/ 	if( bFirst )
/*N*/ 	{
/*N*/ 		// die 1. ist mit Buchstaben addressiert!
/*N*/ 		sal_Unicode cChar;
/*N*/ 		BOOL bFirst = TRUE;
/*N*/ 		while( 0 != ( cChar = rStr.GetChar( nPos )) &&
/*N*/ 			   ( (cChar >= 'A' && cChar <= 'Z') ||
/*N*/ 			     (cChar >= 'a' && cChar <= 'z') ) )
/*N*/ 		{
/*N*/ 			if( (cChar -= 'A') >= 26 )
/*?*/ 				cChar -= 'a' - '[';
/*N*/ 			if( bFirst )
/*N*/ 				bFirst = FALSE;
/*N*/ 			else
/*?*/ 				++nRet;
/*N*/ 			nRet = nRet * 52 + cChar;
/*N*/ 			++nPos;
/*N*/ 		}
/*N*/ 		rStr.Erase( 0, nPos );		// Zeichen aus dem String loeschen
/*N*/ 	}
/*N*/ 	else if( STRING_NOTFOUND == ( nPos = rStr.Search( aDotStr ) ))
/*N*/ 	{
/*N*/ 		nRet = rStr.ToInt32();
/*N*/ 		rStr.Erase();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		nRet = rStr.Copy( 0, nPos ).ToInt32();
/*?*/ 		rStr.Erase( 0, nPos+1 );
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/*N*/ const SwTableBox* SwTable::GetTblBox( const String& rName ) const
/*N*/ {
/*N*/ 	const SwTableBox* pBox = 0;
/*N*/ 	const SwTableLine* pLine;
/*N*/ 	const SwTableLines* pLines;
/*N*/ 	const SwTableBoxes* pBoxes;
/*N*/
/*N*/ 	USHORT nLine, nBox;
/*N*/ 	String aNm( rName );
/*N*/ 	while( aNm.Len() )
/*N*/ 	{
/*N*/ 		nBox = SwTable::_GetBoxNum( aNm, 0 == pBox );
/*N*/ 		// erste Box ?
/*N*/ 		if( !pBox )
/*N*/ 			pLines = &GetTabLines();
/*N*/ 		else
/*N*/ 		{
/*?*/ 			pLines = &pBox->GetTabLines();
/*?*/ 			if( nBox )
/*?*/ 				--nBox;
/*N*/ 		}

/*N*/ 		nLine = SwTable::_GetBoxNum( aNm );
/*N*/
/*N*/ 		// bestimme die Line
/*N*/ 		if( !nLine || nLine > pLines->Count() )
/*N*/ 			return 0;
/*N*/ 		pLine = (*pLines)[ nLine-1 ];
/*N*/
/*N*/ 		// bestimme die Box
/*N*/ 		pBoxes = &pLine->GetTabBoxes();
/*N*/ 		if( nBox >= pBoxes->Count() )
/*N*/ 			return 0;
/*N*/ 		pBox = (*pBoxes)[ nBox ];
/*N*/ 	}

    // abpruefen, ob die gefundene Box auch wirklich eine Inhaltstragende
    // Box ist ??
/*N*/ 	if( pBox && !pBox->GetSttNd() )
/*N*/ 	{
/*?*/ 		ASSERT( FALSE, "Box ohne Inhalt, suche die naechste !!" );
/*?*/ 		// "herunterfallen lassen" bis zur ersten Box
/*?*/ 		while( pBox->GetTabLines().Count() )
/*?*/ 			pBox = pBox->GetTabLines()[0]->GetTabBoxes()[0];
/*N*/ 	}
/*N*/ 	return pBox;
/*N*/ }

/*M*/ SwTableBox* SwTable::GetTblBox( ULONG nSttIdx )
/*M*/ {
/*M*/ 	//MA: Zur Optimierung nicht immer umstaendlich das ganze SortArray abhuenern.
/*M*/     //OS: #102675# converting text to table tries und certain conditions
/*M*/     // to ask for a table box of a table that is not yet having a format
/*M*/     if(!GetFrmFmt())
/*M*/         return 0;
/*M*/     SwTableBox *pRet = 0;
/*M*/     SwNodes &rNds = GetFrmFmt()->GetDoc()->GetNodes();
/*M*/ 	ULONG nIndex;
/*M*/ 	SwCntntNode *pCNd = 0;
/*M*/
/*M*/ 	for ( nIndex = nSttIdx + 1; nIndex < rNds.Count() &&
/*M*/ 								0 == (pCNd = rNds[nIndex]->GetCntntNode());
/*M*/ 								++nIndex )
/*M*/ 		/* do nothing */;
/*M*/
/*M*/ 	if ( pCNd )
/*M*/ 	{
/*M*/ 		SwClientIter aIter( *pCNd );
/*M*/ 		SwFrm *pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) );
/*M*/ 		while ( pFrm && !pFrm->IsCellFrm() )
/*M*/ 			pFrm = pFrm->GetUpper();
/*M*/ 		if ( pFrm )
/*M*/ 			pRet = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
/*M*/ 	}
/*M*/
/*M*/ 	//Falls es das Layout noch nicht gibt oder sonstwie etwas schieft geht.
/*M*/ 	if ( !pRet )
/*M*/ 	{
/*M*/ 		for( USHORT n = aSortCntBoxes.Count(); n; )
/*M*/ 			if( aSortCntBoxes[ --n ]->GetSttIdx() == nSttIdx )
/*M*/ 				return aSortCntBoxes[ n ];
/*M*/ 	}
/*M*/ 	return pRet;
/*M*/ }

/*N*/ BOOL SwTable::IsTblComplex() const
/*N*/ {
/*N*/ 	// returnt TRUE wenn sich in der Tabelle Verschachtelungen befinden
/*N*/ 	// steht eine Box nicht in der obersten Line, da wurde gesplittet/
/*N*/ 	// gemergt und die Struktur ist komplexer.
/*N*/ 	for( USHORT n = 0; n < aSortCntBoxes.Count(); ++n )
/*N*/ 		if( aSortCntBoxes[ n ]->GetUpper()->GetUpper() )
/*N*/ 			return TRUE;
/*N*/ 	return FALSE;
/*N*/ }



/*************************************************************************
|*
|*	SwTableLine::SwTableLine()
|*
|*	Ersterstellung		MA 09. Mar. 93
|*	Letzte Aenderung	MA 09. Mar. 93
|*
|*************************************************************************/
/*N*/ SwTableLine::SwTableLine( SwTableLineFmt *pFmt, USHORT nBoxes,
/*N*/ 							SwTableBox *pUp )
/*N*/ 	: SwClient( pFmt ),
/*N*/ 	aBoxes( (BYTE)nBoxes, 1 ),
/*N*/ 	pUpper( pUp )
/*N*/ {
/*N*/ }

/*N*/ SwTableLine::~SwTableLine()
/*N*/ {
/*N*/ 	// ist die TabelleLine der letzte Client im FrameFormat, kann dieses
/*N*/ 	// geloescht werden
/*N*/ 	SwModify* pMod = GetFrmFmt();
/*N*/ 	pMod->Remove( this );				// austragen,
/*N*/ 	if( !pMod->GetDepends() )
/*N*/ 		delete pMod;	// und loeschen
/*N*/ }

/*************************************************************************
|*
|*	SwTableLine::ClaimFrmFmt(), ChgFrmFmt()
|*
|*	Ersterstellung		MA 03. May. 93
|*	Letzte Aenderung	MA 07. Feb. 96
|*
|*************************************************************************/
/*N*/ SwFrmFmt* SwTableLine::ClaimFrmFmt()
/*N*/ {
/*N*/ 	//Wenn noch andere TableLines ausser mir selbst an dem FrmFmt haengen,
/*N*/ 	//sehe ich mich leider gezwungen mir ein eingenes zu machen und mich
/*N*/ 	//bei diesem anzumelden.
/*N*/ 	SwTableLineFmt *pOld = (SwTableLineFmt*)GetFrmFmt();
/*N*/ 	SwClientIter aIter( *pOld );
/*N*/
/*N*/ 	SwClient* pLast;
/*N*/
/*N*/ 	for( pLast = aIter.First( TYPE( SwTableLine )); pLast && pLast == this;
/*N*/ 		pLast = aIter.Next() )
/*N*/ 		;
/*N*/
/*N*/ 	if( pLast )
/*N*/ 	{
/*N*/ 		SwTableLineFmt *pNewFmt = pOld->GetDoc()->MakeTableLineFmt();
/*N*/ 		*pNewFmt = *pOld;
/*N*/
/*N*/ 		//Erstmal die Frms ummelden.
/*N*/ 		for( pLast = aIter.First( TYPE( SwFrm ) ); pLast; pLast = aIter.Next() )
/*N*/ 			if( ((SwRowFrm*)pLast)->GetTabLine() == this )
/*?*/ 				pNewFmt->Add( pLast );
/*N*/
/*N*/ 		//Jetzt noch mich selbst ummelden.
/*N*/ 		pNewFmt->Add( this );
/*N*/ 		pOld = pNewFmt;
/*N*/ 	}
/*N*/
/*N*/ 	return pOld;
/*N*/ }

 void SwTableLine::ChgFrmFmt( SwTableLineFmt *pNewFmt )
 {
    SwFrmFmt *pOld = GetFrmFmt();
    SwClientIter aIter( *pOld );
    SwClient* pLast;

    //Erstmal die Frms ummelden.
    for( pLast = aIter.First( TYPE( SwFrm ) ); pLast; pLast = aIter.Next() )
    {
        SwRowFrm *pRow = (SwRowFrm*)pLast;
        if( pRow->GetTabLine() == this )
        {
            pNewFmt->Add( pLast );
            pRow->InvalidateSize();
            pRow->_InvalidatePrt();
            pRow->SetCompletePaint();
            pRow->ReinitializeFrmSizeAttrFlags();
        }
    }

    //Jetzt noch mich selbst ummelden.
    pNewFmt->Add( this );

    if ( !aIter.GoStart() )
        delete pOld;
 }

/*************************************************************************
|*
|*	SwTableBox::SwTableBox()
|*
|*	Ersterstellung		MA 04. May. 93
|*	Letzte Aenderung	MA 04. May. 93
|*
|*************************************************************************/
/*N*/ SwTableBox::SwTableBox( SwTableBoxFmt* pFmt, USHORT nLines, SwTableLine *pUp )
/*N*/ 	: SwClient( 0 ),
/*N*/ 	aLines( (BYTE)nLines, 1 ),
/*N*/ 	pSttNd( 0 ),
/*N*/ 	pUpper( pUp ),
/*N*/ 	pImpl( 0 )
/*N*/ {
/*N*/ 	CheckBoxFmt( pFmt )->Add( this );
/*N*/ }

/*N*/ SwTableBox::SwTableBox( SwTableBoxFmt* pFmt, const SwNodeIndex &rIdx,
/*N*/ 						SwTableLine *pUp )
/*N*/ 	: SwClient( 0 ),
/*N*/ 	aLines( 0, 0 ),
/*N*/ 	pUpper( pUp ),
/*N*/ 	pImpl( 0 )
/*N*/ {
/*N*/ 	SwDoc* pDoc = pFmt->GetDoc();
/*N*/ 	CheckBoxFmt( pFmt )->Add( this );
/*N*/
/*N*/ 	pSttNd = pDoc->GetNodes()[ rIdx ]->GetStartNode();
/*N*/
/*N*/ 	// an der Table eintragen
/*N*/ 	const SwTableNode* pTblNd = pSttNd->FindTableNode();
/*N*/ 	ASSERT( pTblNd, "in welcher Tabelle steht denn die Box?" );
/*N*/ 	SwTableSortBoxes& rSrtArr = (SwTableSortBoxes&)pTblNd->GetTable().
/*N*/ 								GetTabSortBoxes();
/*N*/ 	SwTableBox* p = this;	// error: &this
/*N*/ 	rSrtArr.Insert( p );		// eintragen
/*N*/ }

/*N*/ SwTableBox::SwTableBox( SwTableBoxFmt* pFmt, const SwStartNode& rSttNd, SwTableLine *pUp ) :
/*N*/ 	SwClient( 0 ),
/*N*/ 	aLines( 0, 0 ),
/*N*/ 	pUpper( pUp ),
/*N*/ 	pImpl( 0 ),
/*N*/ 	pSttNd( &rSttNd )
/*N*/ {
/*N*/ 	SwDoc* pDoc = pFmt->GetDoc();
/*N*/ 	CheckBoxFmt( pFmt )->Add( this );
/*N*/
/*N*/ 	// an der Table eintragen
/*N*/ 	const SwTableNode* pTblNd = pSttNd->FindTableNode();
/*N*/ 	ASSERT( pTblNd, "in welcher Tabelle steht denn die Box?" );
/*N*/ 	SwTableSortBoxes& rSrtArr = (SwTableSortBoxes&)pTblNd->GetTable().
/*N*/ 								GetTabSortBoxes();
/*N*/ 	SwTableBox* p = this;	// error: &this
/*N*/ 	rSrtArr.Insert( p );		// eintragen
/*N*/ }

/*N*/ SwTableBox::~SwTableBox()
/*N*/ {
/*N*/ 	// Inhaltstragende Box ?
/*N*/ 	if( !GetFrmFmt()->GetDoc()->IsInDtor() && pSttNd )
/*N*/ 	{
/*N*/ 		// an der Table austragen
/*?*/ 		const SwTableNode* pTblNd = pSttNd->FindTableNode();
/*?*/ 		ASSERT( pTblNd, "in welcher Tabelle steht denn die Box?" );
/*?*/ 		SwTableSortBoxes& rSrtArr = (SwTableSortBoxes&)pTblNd->GetTable().
/*?*/ 									GetTabSortBoxes();
/*?*/ 		SwTableBox *p = this;	// error: &this
/*?*/ 		rSrtArr.Remove( p );		// austragen
/*N*/ 	}
/*N*/
/*N*/ 	// ist die TabelleBox der letzte Client im FrameFormat, kann dieses
/*N*/ 	// geloescht werden
/*N*/ 	SwModify* pMod = GetFrmFmt();
/*N*/ 	pMod->Remove( this );				// austragen,
/*N*/ 	if( !pMod->GetDepends() )
/*N*/ 		delete pMod;	// und loeschen
/*N*/
/*N*/ 	delete pImpl;
/*N*/ }

/*N*/ SwTableBoxFmt* SwTableBox::CheckBoxFmt( SwTableBoxFmt* pFmt )
/*N*/ {
    // sollte das Format eine Formel oder einen Value tragen, dann muss die
    // Box alleine am Format haengen. Ggfs. muss ein neues angelegt werden.
/*N*/ 	if( SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_VALUE, FALSE ) ||
/*N*/ 		SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_FORMULA, FALSE ) )
/*N*/ 	{
/*N*/ 		SwClient* pOther = SwClientIter( *pFmt ).First( TYPE( SwTableBox ));
/*N*/ 		if( pOther )
/*N*/ 		{
/*?*/ 			SwTableBoxFmt* pNewFmt = pFmt->GetDoc()->MakeTableBoxFmt();
/*?*/ 			pNewFmt->LockModify();
/*?*/ 			*pNewFmt = *pFmt;
/*?*/
/*?*/ 			// Values und Formeln entfernen
/*?*/ 			pNewFmt->ResetAttr( RES_BOXATR_FORMULA, RES_BOXATR_VALUE );
/*?*/ 			pNewFmt->UnlockModify();
/*?*/
/*?*/ 			pFmt = pNewFmt;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pFmt;
/*N*/ }

/*************************************************************************
|*
|*	SwTableBox::ClaimFrmFmt(), ChgFrmFmt()
|*
|*	Ersterstellung		MA 04. May. 93
|*	Letzte Aenderung	MA 07. Feb. 96
|*
|*************************************************************************/
/*N*/ SwFrmFmt* SwTableBox::ClaimFrmFmt()
/*N*/ {
/*N*/ 	//Wenn noch andere TableBoxen ausser mir selbst an dem FrmFmt haengen,
/*N*/ 	//sehe ich mich leider gezwungen mir ein eingenes zu machen und mich
/*N*/ 	//bei diesem anzumelden.
/*N*/ 	SwTableBoxFmt *pOld = (SwTableBoxFmt*)GetFrmFmt();
/*N*/ 	SwClientIter aIter( *pOld );
/*N*/ 	SwClient* pLast;
/*N*/
/*N*/ 	for( pLast = aIter.First( TYPE( SwTableBox )); pLast && pLast == this;
/*N*/ 		pLast = aIter.Next() )
/*N*/ 		;
/*N*/
/*N*/ 	if( pLast )
/*N*/ 	{
/*N*/ 		SwTableBoxFmt* pNewFmt = pOld->GetDoc()->MakeTableBoxFmt();
/*N*/
/*N*/ 		pNewFmt->LockModify();
/*N*/ 		*pNewFmt = *pOld;
/*N*/
/*N*/ 		// Values und Formeln nie kopieren
/*N*/ 		pNewFmt->ResetAttr( RES_BOXATR_FORMULA, RES_BOXATR_VALUE );
/*N*/ 		pNewFmt->UnlockModify();
/*N*/
/*N*/ 		//Erstmal die Frms ummelden.
/*N*/ 		for( pLast = aIter.First( TYPE( SwFrm ) ); pLast; pLast = aIter.Next() )
/*N*/ 			if( ((SwCellFrm*)pLast)->GetTabBox() == this )
/*N*/ 				pNewFmt->Add( pLast );
/*N*/
/*N*/ 		//Jetzt noch mich selbst ummelden.
/*N*/ 		pNewFmt->Add( this );
/*N*/ 		pOld = pNewFmt;
/*N*/ 	}
/*N*/ 	return pOld;
/*N*/ }

/*N*/ void SwTableBox::ChgFrmFmt( SwTableBoxFmt* pNewFmt )
/*N*/ {
/*N*/ 	SwFrmFmt *pOld = GetFrmFmt();
/*N*/ 	SwClientIter aIter( *pOld );
/*N*/ 	SwClient* pLast;
/*N*/
/*N*/ 	//Erstmal die Frms ummelden.
/*N*/ 	for( pLast = aIter.First( TYPE( SwFrm ) ); pLast; pLast = aIter.Next() )
/*N*/ 	{
/*N*/ 		SwCellFrm *pCell = (SwCellFrm*)pLast;
/*N*/ 		if( pCell->GetTabBox() == this )
/*N*/ 		{
/*?*/ 			pNewFmt->Add( pLast );
/*?*/ 			pCell->InvalidateSize();
/*?*/ 			pCell->_InvalidatePrt();
/*?*/ 			pCell->SetCompletePaint();
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	//Jetzt noch mich selbst ummelden.
/*N*/ 	pNewFmt->Add( this );
/*N*/
/*N*/ 	if( !aIter.GoStart() )
/*?*/ 		delete pOld;
/*N*/ }

/*************************************************************************
|*
|*	String SwTableBox::GetName() const
|*		gebe den Namen dieser Box zurueck. Dieser wird dynamisch bestimmt
|*		und ergibt sich aus der Position in den Lines/Boxen/Tabelle
|*
|*	Ersterstellung		JP 30. Jun. 93
|*	Letzte Aenderung	JP 30. Jun. 93
|*
|*************************************************************************/
/*N*/ void lcl_GetTblBoxColStr( USHORT nCol, String& rNm )
/*N*/ {
/*N*/ 	const USHORT coDiff = 52; 	// 'A'-'Z' 'a' - 'z'
/*N*/ 	register USHORT nCalc;
/*N*/
/*N*/ 	do {
/*N*/ 		nCalc = nCol % coDiff;
/*N*/ 		if( nCalc >= 26 )
/*N*/ 			rNm.Insert( sal_Unicode('a' - 26 + nCalc ), 0 );
/*N*/ 		else
/*N*/ 			rNm.Insert( sal_Unicode('A' + nCalc ), 0 );
/*N*/
/*N*/ 		if( !(nCol -= nCalc) )
/*N*/ 			break;
/*N*/ 		nCol /= coDiff;
/*N*/ 		--nCol;
/*N*/ 	} while( 1 );
/*N*/ }

/*N*/ String SwTableBox::GetName() const
/*N*/ {
/*N*/ 	if( !pSttNd )		// keine Content Box ??
/*N*/ 	{
/*N*/ 		// die naechste erste Box suchen ??
/*?*/ 		return aEmptyStr;
/*N*/ 	}

/*N*/ 	const SwTable& rTbl = pSttNd->FindTableNode()->GetTable();
/*N*/ 	register USHORT nPos;
/*N*/ 	String sNm, sTmp;
/*N*/ 	const SwTableBox* pBox = this;
/*N*/ 	do {
/*N*/ 		const SwTableBoxes* pBoxes = &pBox->GetUpper()->GetTabBoxes();
/*N*/ 		const SwTableLine* pLine = pBox->GetUpper();
/*N*/ 		// auf oberstere Ebene ?
/*N*/ 		const SwTableLines* pLines = pLine->GetUpper()
/*N*/ 				? &pLine->GetUpper()->GetTabLines() : &rTbl.GetTabLines();
/*N*/
/*N*/ 		sTmp = String::CreateFromInt32( nPos = pLines->GetPos( pLine ) + 1 );
/*N*/ 		if( sNm.Len() )
/*N*/ 			sNm.Insert( aDotStr, 0 ).Insert( sTmp, 0 );
/*N*/ 		else
/*N*/ 			sNm = sTmp;
/*N*/
/*N*/ 		sTmp = String::CreateFromInt32(( nPos = pBoxes->GetPos( pBox )) + 1 );
/*N*/ 		if( 0 != ( pBox = pLine->GetUpper()) )
/*N*/ 			sNm.Insert( aDotStr, 0 ).Insert( sTmp, 0 );
/*N*/ 		else
/*N*/ 			::binfilter::lcl_GetTblBoxColStr( nPos, sNm );
/*N*/
/*N*/ 	} while( pBox );
/*N*/ 	return sNm;
/*N*/ }

        // gebe den Zellnamen zu der angebenen Row/Col zurueck. Das ist
        // nur fuer ausgeglichene Tabellen interessant, weil diese keine
        // "Sub"Boxen kennen. Es wird z.B. aus (0,0) ein "A1".

/*N*/ BOOL SwTableBox::IsInHeadline( const SwTable* pTbl ) const
/*N*/ {
/*N*/ 	if( !GetUpper() )			// sollte nur beim Merge vorkommen.
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	if( !pTbl )
/*?*/ 		pTbl = &pSttNd->FindTableNode()->GetTable();
/*N*/
/*N*/ 	const SwTableLine* pLine = GetUpper();
/*N*/ 	while( pLine->GetUpper() )
/*?*/ 		pLine = pLine->GetUpper()->GetUpper();

    // Headerline?
/*N*/ 	return pTbl->GetTabLines()[ 0 ] == pLine;
/*N*/ }

#ifdef DBG_UTIL

/*N*/ ULONG SwTableBox::GetSttIdx() const
/*N*/ {
/*N*/ 	return pSttNd ? pSttNd->GetIndex() : 0;
/*N*/ }
#endif

    // erfrage vom Client Informationen
/*N*/ BOOL SwTable::GetInfo( SfxPoolItem& rInfo ) const
/*N*/ {
/*N*/ 	switch( rInfo.Which() )
/*N*/ 	{
/*N*/ 	case RES_AUTOFMT_DOCNODE:
/*N*/ 		if( aSortCntBoxes.Count() &&
/*N*/ 			&aSortCntBoxes[ 0 ]->GetSttNd()->GetNodes() ==
/*N*/ 			((SwAutoFmtGetDocNode&)rInfo).pNodes )
/*N*/ 		{
/*N*/ 			SwNodeIndex aIdx( *aSortCntBoxes[ 0 ]->GetSttNd() );
/*N*/ 			((SwAutoFmtGetDocNode&)rInfo).pCntntNode =
/*N*/ 							GetFrmFmt()->GetDoc()->GetNodes().GoNext( &aIdx );
/*N*/ 			return FALSE;
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_FINDNEARESTNODE:
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	if( GetFrmFmt() && ((SwFmtPageDesc&)GetFrmFmt()->GetAttr(
/*?*/ 		break;
/*?*/
/*?*/ 	case RES_CONTENT_VISIBLE:
/*?*/ 		{
/*?*/ 			((SwPtrMsgPoolItem&)rInfo).pObject =
/*?*/ 				SwClientIter( *GetFrmFmt() ).First( TYPE(SwFrm) );
/*?*/ 		}
/*?*/ 		return FALSE;
/*?*/ 	}
/*?*/ 	return TRUE;
/*N*/ }

/*N*/ SwTable* SwTable::FindTable( SwFrmFmt* pFmt )
/*N*/ {
/*N*/ 	return pFmt ? (SwTable*)SwClientIter( *pFmt ).First( TYPE(SwTable) ) : 0;
/*N*/ }

/*N*/ SwTableNode* SwTable::GetTableNode() const
/*N*/ {
/*N*/ 	return GetTabSortBoxes().Count()
/*N*/ 		? (SwTableNode*)GetTabSortBoxes()[ 0 ]->GetSttNd()->FindTableNode()
/*N*/ 		: 0;
/*N*/ }

 void SwTable::SetHTMLTableLayout( SwHTMLTableLayout *p )
 {
    delete pHTMLLayout;
    pHTMLLayout = p;
 }

// zum Erkennen von Veraenderungen (haupts. TableBoxAttribute)
/*N*/ void SwTableBoxFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
/*N*/ {
/*N*/ 	if( !IsModifyLocked() && !IsInDocDTOR() )
/*N*/ 	{
/*N*/ 		const SwTblBoxNumFormat *pNewFmt = 0, *pOldFmt = 0;
/*N*/ 		const SwTblBoxFormula *pNewFml = 0, *pOldFml = 0;
/*N*/ 		const SwTblBoxValue *pNewVal = 0, *pOldVal = 0;
/*N*/ 		double aOldValue = 0;
/*N*/ 		ULONG nOldFmt = NUMBERFORMAT_TEXT;
/*N*/
/*N*/ 		SwAttrSetChg *pNewChgSet = 0,  *pOldChgSet = 0;
/*N*/
/*N*/ 		switch( pNew ? pNew->Which() : 0 )
/*N*/ 		{
/*N*/ 		case RES_ATTRSET_CHG:
/*N*/ 			{
/*N*/ 				const SfxItemSet& rSet = *((SwAttrSetChg*)pNew)->GetChgSet();
/*N*/ 				if( SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMAT,
/*N*/ 									FALSE, (const SfxPoolItem**)&pNewFmt ) )
/*N*/ 					nOldFmt = ((SwTblBoxNumFormat&)((SwAttrSetChg*)pOld)->
/*N*/ 							GetChgSet()->Get( RES_BOXATR_FORMAT )).GetValue();
/*N*/ 				rSet.GetItemState( RES_BOXATR_FORMULA, FALSE,
/*N*/ 									(const SfxPoolItem**)&pNewFml );
/*N*/ 				if( SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_VALUE,
/*N*/ 									FALSE, (const SfxPoolItem**)&pNewVal ) )
/*N*/ 					aOldValue = ((SwTblBoxValue&)((SwAttrSetChg*)pOld)->
/*N*/ 							GetChgSet()->Get( RES_BOXATR_VALUE )).GetValue();
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 		case RES_BOXATR_FORMAT:
/*N*/ 			pNewFmt = (SwTblBoxNumFormat*)pNew;
/*N*/ 			nOldFmt = ((SwTblBoxNumFormat*)pOld)->GetValue();
/*N*/ 			break;
/*N*/ 		case RES_BOXATR_FORMULA:
/*N*/ 			pNewFml = (SwTblBoxFormula*)pNew;
/*N*/ 			break;
/*N*/ 		case RES_BOXATR_VALUE:
/*N*/ 			pNewVal = (SwTblBoxValue*)pNew;
/*N*/ 			aOldValue = ((SwTblBoxValue*)pOld)->GetValue();
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		// es hat sich etwas getan und im Set ist noch irgendein BoxAttribut
/*N*/ 		// vorhanden!
/*N*/ 		if( pNewFmt || pNewFml || pNewVal )
/*N*/ 		{
/*N*/ 			GetDoc()->SetFieldsDirty( TRUE );
/*N*/
/*N*/ 			if( SFX_ITEM_SET == GetItemState( RES_BOXATR_FORMAT, FALSE ) ||
/*N*/ 				SFX_ITEM_SET == GetItemState( RES_BOXATR_VALUE, FALSE ) ||
/*N*/ 				SFX_ITEM_SET == GetItemState( RES_BOXATR_FORMULA, FALSE ) )
/*N*/ 			{
/*N*/ 				// die Box holen
/*N*/ 				SwClientIter aIter( *this );
/*N*/ 				SwTableBox* pBox = (SwTableBox*)aIter.First( TYPE( SwTableBox ) );
/*N*/ 				if( pBox )
/*N*/ 				{
/*?*/ 					ASSERT( !aIter.Next(), "keine Box oder mehrere am Format" );
/*?*/
/*?*/ 					ULONG nNewFmt;
/*?*/ 					if( pNewFmt )
/*?*/ 					{
/*?*/ 						nNewFmt = pNewFmt->GetValue();
/*?*/ 						// neu Formatieren
/*?*/ 						// ist es neuer oder wurde der akt. entfernt?
/*?*/ 						if( SFX_ITEM_SET != GetItemState( RES_BOXATR_VALUE, FALSE ))
/*?*/ 							pNewFmt = 0;
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						// das akt. Item besorgen
/*?*/ 						GetItemState( RES_BOXATR_FORMAT, FALSE,
/*?*/ 											(const SfxPoolItem**)&pNewFmt );
/*?*/ 						nOldFmt = GetTblBoxNumFmt().GetValue();
/*?*/ 						nNewFmt = pNewFmt ? pNewFmt->GetValue() : nOldFmt;
/*?*/ 					}
/*?*/
/*?*/ 					// ist es neuer oder wurde der akt. entfernt?
/*?*/ 					if( pNewVal )
/*?*/ 					{
/*?*/ 						if( NUMBERFORMAT_TEXT != nNewFmt )
/*?*/ 						{
/*?*/ 							if( SFX_ITEM_SET == GetItemState(
/*?*/ 												RES_BOXATR_VALUE, FALSE ))
/*?*/ 								nOldFmt = NUMBERFORMAT_TEXT;
/*?*/ 							else
/*?*/ 								nNewFmt = NUMBERFORMAT_TEXT;
/*?*/ 						}
/*?*/ 						else if( NUMBERFORMAT_TEXT == nNewFmt )
/*?*/ 							nOldFmt = 0;
/*?*/ 					}
/*?*/
/*?*/ 					// Logik:
/*?*/ 					// ValueAenderung:	-> "simuliere" eine FormatAenderung!
/*?*/ 					// FormatAenderung:
/*?*/ 					// Text -> !Text oder FormatAenderung:
/*?*/ 					//			- Ausrichtung auf RECHTS, wenn LINKS oder Blocksatz
/*?*/ 					//			- vertikale Ausrichtung auf UNTEN wenn OBEN oder nicht
/*?*/ 					//				gesetzt ist.
/*?*/ 					//			- Text ersetzen (Farbe?? neg. Zahlen ROT??)
/*?*/ 					// !Text -> Text:
/*?*/ 					//			- Ausrichtung auf LINKS, wenn RECHTS
/*?*/ 					//			- vertikale Ausrichtung auf OEBN, wenn UNTEN gesetzt ist
/*?*/
/*?*/ 					SvNumberFormatter* pNumFmtr = GetDoc()->GetNumberFormatter();
/*?*/ 					BOOL bNewIsTxtFmt = pNumFmtr->IsTextFormat( nNewFmt ) ||
/*?*/ 										NUMBERFORMAT_TEXT == nNewFmt;
/*?*/
/*?*/ 					if( !bNewIsTxtFmt && nOldFmt != nNewFmt || pNewFml )
/*?*/ 					{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 		BOOL bChgTxt = TRUE;
/*?*/ 					}
/*?*/ 					else if( bNewIsTxtFmt && nOldFmt != nNewFmt )
/*?*/ 					{
/*?*/ 						// auf jedenfall muessen jetzt die Formeln/Values
/*?*/ 						// geloescht werden!
/*?*/ 	//					LockModify();
/*?*/ 	//					ResetAttr( RES_BOXATR_FORMULA, RES_BOXATR_VALUE );
/*?*/ 	//					UnlockModify();
/*?*/
/*?*/
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	ChgNumToText( *pBox, nNewFmt );
/*?*/ 					}
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// Und die Basis-Klasse rufen
/*N*/ 	SwFrmFmt::Modify( pOld, pNew );
/*N*/ }



/*N*/ ULONG SwTableBox::IsValidNumTxtNd( BOOL bCheckAttr ) const
/*N*/ {
/*N*/ 	ULONG nPos = ULONG_MAX;
/*N*/ 	if( pSttNd )
/*N*/ 	{
/*N*/ 		SwNodeIndex aIdx( *pSttNd );
/*N*/ 		const SwCntntNode* pCNd = pSttNd->GetNodes().GoNext( &aIdx );
/*N*/ 		if( pCNd && pCNd->IsTxtNode() &&
/*N*/ 			pSttNd->GetNodes()[ aIdx.GetIndex() + 1 ]->IsEndNode() )
/*N*/ 		{
/*N*/ 			if( bCheckAttr )
/*N*/ 			{
/*N*/ 				const SwpHints* pHts = ((SwTxtNode*)pCNd)->GetpSwpHints();
/*N*/ 				const String& rTxt = ((SwTxtNode*)pCNd)->GetTxt();
/*N*/ //				if( rTxt.Len() )
/*N*/ 				{
/*N*/ 					nPos = aIdx.GetIndex();
/*N*/
/*N*/ 					// dann teste doch mal, ob das wirklich nur Text im Node steht!
/*N*/ 					// Flys/Felder/..
/*N*/ 					if( pHts )
/*N*/ 					{
/*?*/ 						for( USHORT n = 0; n < pHts->Count(); ++n )
/*?*/ 						{
/*?*/ 							const SwTxtAttr* pAttr = (*pHts)[ n ];
/*?*/ 							if( RES_TXTATR_NOEND_BEGIN <= pAttr->Which() ||
/*?*/ 								*pAttr->GetStart() ||
/*?*/ 								*pAttr->GetAnyEnd() < rTxt.Len() )
/*?*/ 							{
/*?*/ 								nPos = ULONG_MAX;
/*?*/ 								break;
/*?*/ 							}
/*?*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				nPos = aIdx.GetIndex();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nPos;
/*N*/ }

// ist das eine FormelBox oder eine Box mit numerischen Inhalt (AutoSum)

// JP 12.09.97 - Bug 41223:
// falls an der International Einstellung gedreht wurde, so muss beim Laden
// eine entsprechende Aktualisierung erfolgen.
/*N*/ void SwTableBox::ChgByLanguageSystem()
/*N*/ {
/*N*/ 	const SfxPoolItem *pFmtItem, *pValItem;
/*N*/ 	SwFrmFmt* pFmt = GetFrmFmt();
/*N*/ 	ULONG nFmtId;
/*N*/ 	if( SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_FORMAT, TRUE, &pFmtItem )
/*N*/ 		&& ( nFmtId = ((SwTblBoxNumFormat*)pFmtItem)->GetValue()) < SV_COUNTRY_LANGUAGE_OFFSET
/*N*/ 		&& SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_VALUE, TRUE, &pValItem ))
/*N*/ 	{
/*N*/ 		ULONG nNdPos;
/*N*/ 		SvNumberFormatter* pNumFmtr = pFmt->GetDoc()->GetNumberFormatter();
/*N*/
/*N*/ 		if( !pNumFmtr->IsTextFormat( nFmtId ) &&
/*N*/ 			ULONG_MAX != (nNdPos = IsValidNumTxtNd( TRUE )) )
/*N*/ 		{
/*N*/ 			double fVal = ((SwTblBoxValue*)pValItem)->GetValue();
/*N*/ 			Color* pCol = 0;
/*N*/ 			String sNewTxt;
/*N*/ 			pNumFmtr->GetOutputString( fVal, nFmtId, sNewTxt, &pCol );
/*N*/
/*N*/ 			const String& rTxt = pSttNd->GetNodes()[ nNdPos ]->GetTxtNode()->GetTxt();
/*N*/ 			if( rTxt != sNewTxt )
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 ChgTextToNum( *this, sNewTxt, pCol, FALSE );
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*N*/


}
